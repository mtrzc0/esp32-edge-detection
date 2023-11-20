import socket
import binascii
import os
import time
from PIL import Image
from io import BytesIO

UDP_IP = ""
UDP_PORT = 8765
UDP_BUFFER = 32768
SAVE_FOLDER = "received_data/"

# Create the save folder if it doesn't exist
if not os.path.exists(SAVE_FOLDER):
    os.makedirs(SAVE_FOLDER)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

while True:
    data, addr = sock.recvfrom(UDP_BUFFER)

    # Convert the data to a hex dump
    hex_dump = binascii.hexlify(data).decode('utf-8')

    timestamp = time.time()
    # Convert Hex dump to JPEG
    jpeg_image = Image.open(BytesIO(data)).convert("RGB")

    # Save the JPEG image
    jpeg_path = os.path.join(SAVE_FOLDER, f"received_hexdump_{timestamp}.jpg")
    jpeg_image.save(jpeg_path)

    print(f"Received and saved hex dump to: {jpeg_path}")
