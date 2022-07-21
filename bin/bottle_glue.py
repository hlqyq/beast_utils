# -*- coding: utf-8 -*-

"""bottle模块插件实现
"""

import logging
import sys
import functools
from io import (StringIO, BytesIO, BufferedReader)
from typing import Callable
from wsgiref.simple_server import (make_server, WSGIServer, WSGIRequestHandler)
from socketserver import BaseServer
import bottle

######################################## interface ########################################

def handle_http_request(server_user_data: int, raw_head: bytes, raw_body: bytes, response_cb: Callable) -> None:
    """handle the http request

    Args:
        server_user_data: the data of the server
        raw_head: the head of the request
        raw_body: the body of the request
        response_cb: def _(server_user_data: int, response_value: bytes, response_size: int)

    """
    server = _get_mock_server_instance(80)
    inp = BufferedReader(BytesIO(raw_head + raw_body))
    out = BytesIO()
    olderr = sys.stderr
    error = sys.stderr = StringIO()
    try:
        server.finish_request((inp, out), ("127.0.0.1", 8888))
    finally:
        sys.stderr = olderr
    error_message = error.getvalue()
    if error_message.startswith('Traceback'):
        logging.error(error_message)
    response_value = out.getvalue()
    response_cb(server_user_data, response_value, len(response_value))

######################################## implements ########################################

@functools.lru_cache()
def _get_mock_server_instance(port: int):
    """get the instance of server

    Args:
        port: listen port

    Returns:
        return the instance of server

    """
    return make_server("127.0.0.1", port, bottle.default_app(), MockServer, MockHandler)

class MockServer(WSGIServer):
    """Non-socket HTTP server"""

    def __init__(self, server_address, RequestHandlerClass):  #pylint: disable=super-init-not-called
        BaseServer.__init__(self, server_address, RequestHandlerClass)  #pylint: disable=non-parent-init-called
        self.server_bind()

    def server_bind(self):
        host, port = self.server_address
        self.server_name = host
        self.server_port = port
        self.setup_environ()

class MockHandler(WSGIRequestHandler):
    """Non-socket HTTP handler"""
    def setup(self):
        self.connection = self.request
        self.rfile, self.wfile = self.connection

    def finish(self):
        pass
