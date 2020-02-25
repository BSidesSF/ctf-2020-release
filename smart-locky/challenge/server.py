#!/usr/bin/env python3

import logging
import socketserver

KEY = b"eefi0shush9och5Ama"
FLAG = b"CTF{dont_need_esp_to_read_esp}"
PORT = 4141

class FlagServer(socketserver.ThreadingTCPServer):

    allow_reuse_address = True
    request_queue_size = 20


class FlagHandler(socketserver.BaseRequestHandler):

    def handle(self):
        logging.info('Received request from %s', self.client_address)
        data = self.request.recv(1024).strip()
        logging.info('Got %s', data)
        if data == KEY:
            self.request.sendall(FLAG)
            logging.info('Sending flag to %s', self.client_address)
        else:
            self.request.sendall(b"NO")


def main():
    with FlagServer(("", PORT), FlagHandler) as server:
        server.serve_forever()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()


