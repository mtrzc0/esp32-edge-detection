import asyncio
import websockets
from io import BytesIO
from PIL import Image

async def save_image(data):
    try:
        # Convert the binary data to a PIL Image
        image = Image.open(BytesIO(data))

        # Save the image as JPEG
        image.save('images/output.jpg', 'JPEG')

        print('Image saved successfully')
    except Exception as e:
        print(f'Error saving image: {e}')

async def handle_client(websocket, path):
    print('Client connected')

    try:
        async for message in websocket:
            # Assuming the message received is binary data
            await save_image(message)

    except websockets.exceptions.ConnectionClosed:
        print('Client disconnected')

start_server = websockets.serve(handle_client, '192.168.43.170', 8765)

print('Server is running on ws://192.168.43.170:8765')

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()

