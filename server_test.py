import asyncore
import socket
import logging

import numpy as np
import torch

IMG_BUF = 3 * 3 * 512 * 512 # 3 channels, 512x512 image times 3 bytes per pixel (idk why)
SAVE_FOLDER = "received_img/"

class Server(asyncore.dispatcher):
    def __init__(self, address):
        asyncore.dispatcher.__init__(self)
        self.logger = logging.getLogger('Server')
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind(address)
        self.address = self.socket.getsockname()
        self.logger.debug('binding to %s', self.address)
        self.listen(5)

    def handle_accept(self):
        # Called when a client connects to our socket
        client_info = self.accept()
        if client_info is not None:
            self.logger.debug('handle_accept() -> %s', client_info[1])
            ClientHandler(client_info[0], client_info[1])

class ClientHandler(asyncore.dispatcher):
    def __init__(self, sock, address):
        asyncore.dispatcher.__init__(self, sock)
        self.logger = logging.getLogger('Client ' + str(address))

    def handle_read(self):
        data = self.recv(IMG_BUF)
        self.logger.debug('handle_read() -> (%d) "%s"', len(data), data.rstrip())
        #TODO: Change hex_dump to JPEG image
        try:
            tensor = torch.tensor(np.frombuffer(data, dtype=np.uint8), dtype=torch.uint8).to(torch.device("cpu")).view(3, 512, 512)
            tensor.toPILImage().save(f"{SAVE_FOLDER}/test.jpg")
        except OSError:
            print("Received invalid hex dump. Skipping...")

    def handle_close(self):
        self.logger.debug('handle_close()')
        self.close()

def main():
    logging.basicConfig(level=logging.DEBUG, format='%(name)s:[%(levelname)s]: %(message)s')
    HOST = '0.0.0.0'
    PORT = 8765
    s = Server((HOST,PORT))
    asyncore.loop()


if __name__ == '__main__':
    main()