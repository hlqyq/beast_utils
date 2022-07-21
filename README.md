# beast_utils

Inspire by [the advanced example of beast](https://www.boost.org/doc/libs/1_79_0/libs/beast/example/advanced/server-flex/advanced_server_flex.cpp) and bring it's abilities to python.
It can be used to develop a web application or website by combining bottle.

## Features(From beast advanced server)

1. Timeouts
1. Multi-threaded
1. HTTP pipelining
1. Dual protocols: HTTP and WebSocket
1. Flexible ports: plain and SSL on the same port

## Start

1. After the [build](BUILD.md) is complete. Copy the bin directory as the start sketch.
1. Run the "beast_web_server.py" as the main program file.

## Hello world

1. Inside the bin directory, create your app subdirectory like this(reference the view_example):

```
 bin
 |__hello
    |___ template
    |___ __init__.py
    |___ view_hello.py

```

The "template" subdirectory can be used to store html pages or templates used by views

2. view_hello.py

```python
from bottle import get

@get('/hello')
def _():
    return f'<html><body>Hello world!</body></html>'
```

3. Run the "beast_web_server.py" and launch your browser and visit the url([windows](http://localhost/hello), [linux](http://localhost:8080/hello)). You will see the expected result.

## Port, ssl and concurrency

Open the "beast_web_server.py" file, locate to the end of the file. You'll see this:

```python

if __name__ == '__main__':
    http_default_port = 80
    port_dict = {'Window': http_default_port, 'Linux': 8080}
    server_port, enable_ssl, concurrency_hint = (port_dict.get(platform.system(), http_default_port), True, 2)
    ...

```

Modify it and relaunch.

## Debug in VSCode

Inside some callback handles, the code maybe can't block even though It is set breakpoint. Then you can do like this:

```python

import debugpy

...

def _handle_http_request(server_user_data: int, raw_head: bytes, raw_body: bytes, response_cb: callable) -> None:
    """process http request

    Args:
        raw_head: headers of request
        raw_body: body of request

    """
    debugpy.debug_this_thread()
    ...

```

But in product environment, Remember to remove it or you'll get failure.

See [reference](https://code.visualstudio.com/docs/python/debugging)