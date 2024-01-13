import socket
import binascii
import os
import time
from PIL import Image
from io import BytesIO

TCP_IP = "" # Listen on all interfaces
TCP_PORT = 8765
TCP_BUFFER = 3 * 3 * 512 * 512 # 3 channels, 512x512 image times 3 bytes per pixel (idk why)
SAVE_FOLDER = "received_img/"

# Create the save folder if it doesn't exist
if not os.path.exists(SAVE_FOLDER):
    os.makedirs(SAVE_FOLDER)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((TCP_IP, TCP_PORT))
sock.listen(1000)

while True:
    conn, addr = sock.accept()
    data = conn.recvfrom(TCP_BUFFER)

    print(data[0])

    # Convert the data to a hex dump, should pass data[0] (because of TCP)
    hex_dump = binascii.hexlify(data[0]).decode('utf-8')

    timestamp = time.time()

    # Save the hex dump
    try:
        jpeg_image = Image.open(BytesIO(data[0])).convert("RGB")
    except OSError:
        print("Received invalid image. Skipping...")
        continue

    # Save the JPEG image
    jpeg_path = os.path.join(SAVE_FOLDER, f"img_{timestamp}.jpg")
    jpeg_image.save(jpeg_path)

    print(f"Received and saved img to: {jpeg_path}")
