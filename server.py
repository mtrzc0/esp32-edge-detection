import http.server
import socketserver
import os
from urllib.parse import urlparse
from io import BytesIO
from PIL import Image

# Specify the folder to save the JPEG images
output_folder = 'output_images'

# Create the output folder if it doesn't exist
os.makedirs(output_folder, exist_ok=True)

class RequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)

        # Convert the received data to a PIL Image
        try:
            image = Image.open(BytesIO(post_data))
        except Exception as e:
            self.send_error(400, f"Error: {str(e)}")
            return

        # Generate a unique filename (you may want to implement a more sophisticated naming scheme)
        filename = os.path.join(output_folder, 'image_{}.jpeg'.format(hash(post_data)))

        # Save the image
        image.save(filename, format='JPEG')
        print(f"Image saved as: {filename}")

        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"Image received and saved successfully")

if __name__ == "__main__":
    # Specify the port to run the server on
    port = 8080

    # Set up the server
    with socketserver.TCPServer(("", port), RequestHandler) as httpd:
        print(f"Serving on port {port}")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass
        httpd.server_close()

