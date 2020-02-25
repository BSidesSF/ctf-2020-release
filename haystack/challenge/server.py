import socketserver

import chaffing


PORT = 3199


class ChaffedServer(socketserver.ThreadingTCPServer):

    allow_reuse_address = True


class ChaffedHandler(socketserver.BaseRequestHandler):

    def handle(self):
        with open('message.txt', 'rb') as fp:
            msg = fp.read()
        with open('key.txt', 'rb') as fp:
            key = fp.read()
        encoded = chaffing.chaff_msg(msg, key)
        self.request.sendall(encoded)


if __name__ == '__main__':
    server = ChaffedServer(('', PORT), ChaffedHandler)
    server.serve_forever()
