# -*- coding: utf-8 -*-

"""simple view"""

from time import sleep
from bottle import (get, request, view)

@get('/')
def _():
    """default page"""
    return """<html>
                <body>
                    <ul>
                        <li><a href="/hello?delay=20" target="_blank">Hello world(block for 20 seconds)</a></li>
                        <li><a href="/hello?delay=10" target="_blank">Hello world(block for 10 seconds)</a></li>
                        <li><a href="/hello?delay=5" target="_blank">Hello world(block for 5 seconds)</a></li>
                        <li><a href="/hello" target="_blank">Hello world(immediately)</a></li>
                        <li><a href="/chat" target="_blank">Chat</a></li>
                    </ul>
                </body>
              </html>"""

@get('/hello')
def _():
    """Hello world"""
    delay_seconds = int(request.query.get('delay', '0'))
    if delay_seconds > 0:
        sleep(delay_seconds)
    return f'<html><body>Hello world: block for {delay_seconds} seconds.</body></html>'

@get('/chat')
@view('chat_client.html')
def _():
    """Simple chat client"""
    is_https = request.urlparts.scheme == "https"
    ws_scheme, ws_default_port = ('wss', 443) if is_https else ("ws", 80)
    ws_url = f'{ws_scheme}://{request.urlparts.hostname}:{request.urlparts.port or ws_default_port}'
    return {'ws_url': ws_url}
