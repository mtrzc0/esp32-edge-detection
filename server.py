import socket
import sys
import tensorflow as tf
import time
from io import BytesIO
from PIL import Image

Image.LOAD_TRUNCATED_IMAGES = True

TCP_IP = ""
TCP_PORT = 8765

if sys.argv[1] == "jpeg":
    TCP_BUFFER_EXPECTED_SIZE = 32 * 1024
    TCP_BUFFER_SIZE = TCP_BUFFER_EXPECTED_SIZE
elif sys.argv[1] == "tensor":
    TCP_BUFFER_EXPECTED_SIZE = 3 * 512 * 512
    TCP_BUFFER_SIZE = TCP_BUFFER_EXPECTED_SIZE

SAVE_FOLDER = "received_img"


def start_tcp_server(host, port):
    # Create a TCP socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind the socket to a specific address and port
    server_socket.bind((host, port))

    # Listen for incoming connections (max 1 connection in this example)
    server_socket.listen(1)
    print(f"Server listening on {host}:{port}")

    while True:
        # Accept a connection from a client
        client_socket, client_address = server_socket.accept()
        print(f"Accepted connection from {client_address}")

        try:
            # Receive the data in chunks (adjust buffer size as needed)
            received_data = b""
            total_received = 0

            while total_received < TCP_BUFFER_EXPECTED_SIZE:
                data_chunk = client_socket.recv(TCP_BUFFER_SIZE)
                if not data_chunk:
                    break
                received_data += data_chunk.rstrip()
                total_received += len(data_chunk)
            len_received_data = len(received_data)

            print(f"Received {total_received} bytes of data, len of received_data: {len(received_data)}")
            for _ in range(total_received - len_received_data):
                received_data += b"0"

            timestamp = time.time()

            if sys.argv[1] == "jpeg":
                jpeg_image = Image.open(BytesIO(received_data)).convert("RGB").resize((512, 512))
                jpeg_image.save(f"{SAVE_FOLDER}/raw_img{timestamp}.jpg")
            elif sys.argv[1] == "tensor":
                tensor = tf.io.decode_image(received_data)
                jpeg_image = tf.io.encode_jpeg(tf.image.convert_image_dtype(tensor, tf.uint8))
                tf.io.write_file(f"{SAVE_FOLDER}/tensor_img{timestamp}.jpg", jpeg_image)
            else:
                assert False, "Invalid argument"

            print(f"Saved image to {SAVE_FOLDER}/img{timestamp}.jpg")


        except Exception as e:
            print(f"Server exception: {e}")
        finally:
            # Close the connection
            client_socket.close()


if __name__ == "__main__":
    # Set the host and port to your desired values
    start_tcp_server(TCP_IP, TCP_PORT)
