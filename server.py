import socket
import binascii
import os
import time
from PIL import Image
from io import BytesIO

UDP_IP = "" # Listen on all interfaces
UDP_PORT = 8765
UDP_BUFFER = 32768
SAVE_FOLDER = "received_img/"

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

    # Save the hex dump
    try:
        jpeg_image = Image.open(BytesIO(data)).convert("RGB")
    except OSError:
        print("Received invalid image. Skipping...")
        continue

    # Save the JPEG image
    jpeg_path = os.path.join(SAVE_FOLDER, f"img_{timestamp}.jpg")
    jpeg_image.save(jpeg_path)

    print(f"Received and saved img to: {jpeg_path}")
