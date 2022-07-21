#!/usr/bin/python3.8
# -*- coding: utf-8 -*-

"""web server with beast and bottle

inspiring from [here](https://www.boost.org/doc/libs/1_79_0/libs/beast/doc/html/beast/examples.html#beast.examples.servers_advanced)
Advanced, flex (plain + SSL): advanced_server_flex.cpp

Features:
    *. Timeouts
    *. Multi-threaded
    *. HTTP pipelining
    *. Parser-oriented HTTP reading
    *. Dual protocols: HTTP and WebSocket
    *. Flexible ports: plain and SSL on the same port
    *. Clean exit via SIGINT (CTRL+C) or SIGTERM (kill)

"""

from contextlib import ContextDecorator
import logging
import os
import functools
import importlib
import time
from typing import Callable
import platform
import beast_utils as model

logging.basicConfig(level=logging.DEBUG, format='%(levelname)s - %(module)s - %(threadName)s:%(message)s')
log = logging.getLogger(__name__)  #pylint: disable=invalid-name

######################################## main route ########################################

def _start_simple_web_server(server_port: int, enable_ssl: bool, concurrency_hint: int):
    """start simple server: hello world

    Args:
        server_port: Listening port
        enable_ssl: Is ssl enable
        concurrency_hint: Number of concurrent threads

    Returns:
        return (True, '') or (False, {error_message})

    """
    # 1. initialize
    success, result_or_error = model.plugin_initialize()
    if not success:
        return (success, result_or_error)
    # 2. set up http handler
    def _http_hello_world_cb(server_user_data: int, raw_head: bytes, raw_body: bytes, response_cb: callable) -> None:
        cr_lf, response_body = ('\r\n', 'Hello world!')
        response_value = f'HTTP/1.0 200 OK{cr_lf}Content-Length: {len(response_body)}{cr_lf}{cr_lf}{response_body}'.encode()
        response_cb(server_user_data, response_value, len(response_value))
    model.set_http_handler(_http_hello_world_cb)
    # 3. run server
    log.info('Run web server(post: %s)...', server_port)
    return model.run_server(server_port, enable_ssl, concurrency_hint)

def _start_bottle_web_server(server_port: int, enable_ssl: bool, concurrency_hint: int):
    """start the web server

    Args:
        server_port: Listening port
        enable_ssl: Is ssl enable
        concurrency_hint: Number of concurrent threads

    Returns:
        return (True, '') or (False, {error_message})

    """
    # 1. initialize
    success, result_or_error = model.plugin_initialize()
    if not success:
        return (success, result_or_error)
    # 2. set up handles
    model.set_server_shutdown_handler(_server_shutdown_handler)
    if enable_ssl:
        success, result_or_error = _ssl_is_valid()
        if not success:
            log.warning(result_or_error)
        else:
            ssl_file_bind_handles = (_ssl_certificate_handler, _ssl_private_key_handler, _ssl_dh_handler)
            server_ssl_root_path = os.path.join(os.path.dirname(__file__), 'server_ssl')
            ssl_file_handles = list(map(lambda handle_cb: functools.partial(handle_cb, server_ssl_root_path), ssl_file_bind_handles))
            model.set_ssl_handler(*ssl_file_handles, _ssl_password_handler)
    model.set_log_handler(_handle_log)
    model.set_log_reporting_level(0)
    model.set_http_handler(_handle_http_request)
    model.set_http_timeout_handler(_http_timeout_handle)
    model.set_http_body_limit_handler(_http_body_limit_handle)
    model.ws_set_message_handler(_handle_ws_message)
    model.ws_set_open_handler(_ws_open_handler)
    model.ws_set_close_handler(_ws_close_handler)
    # 3. load web views
    application_path, views_relative_path, view_file_name_prefix = (os.path.dirname(__file__), 'view_example', 'view_')
    views_path = os.path.abspath(os.path.join(application_path, views_relative_path))
    success, result_or_error = _scan_and_load_sub_views(application_path, views_path, view_file_name_prefix)
    if not success:
        log.warning(result_or_error)
    # 4. add bottle template path support
    import bottle  #pylint: disable=import-outside-toplevel
    bottle.TEMPLATE_PATH.append(os.path.join(views_path, 'template'))
    # 5. run server
    log.info('Run web server(post: %s)...', server_port)
    return model.run_server(server_port, enable_ssl, concurrency_hint)

######################################## implement ########################################

def _ssl_is_valid() -> tuple:
    """check ssl files exists

    server_ssl
        |____  server.csr
        |____  server.key
        |____  server.dh

    Returns:
        return (True, '') or (False, {error_message})

    """
    ssl_subdirectory, ssl_file_name_list = ('server_ssl', ('server.csr', 'server.key', 'server.dh'))
    ssl_path = os.path.join(os.path.dirname(__file__), ssl_subdirectory)
    for ssl_file_name in ssl_file_name_list:
        ssl_path_name = os.path.join(ssl_path, ssl_file_name)
        if not os.path.exists(ssl_path_name):
            return (False, f'The requested ssl file could not be found: "{ssl_path_name}".')
    return (True, '')

def _scan_and_load_sub_views(application_path, views_path: str, view_file_name_prefix: str = None) -> tuple:
    """scan load load views("view_xxx.py")

    Args:
        application_path: application path
        views_path: views root path
        view_file_name_prefix: View filename prefix

    Returns:
        return (True, '') or (False, {error_message})

    """
    sub_view_prefix = view_file_name_prefix or 'view_'
    def _filter_cb(sub_view_prefix: str, root: str, file: str) -> bool:
        return file.lower().startswith(sub_view_prefix)
    return _scan_and_load_modules(application_path, views_path, 'views', functools.partial(_filter_cb, sub_view_prefix))

def _scan_and_load_modules(application_path: str, root_path: str, module_name: str, filter_function: callable) -> tuple:
    """Scan and load modules from specific directory

    Args:
        application_path: application path
        root_path: Module root path
        module_name: module category
        filter_function: def _filter_cb(file_path: str, file_name: str) -> bool

    Returns:
        return (True, [{module_instance}, ...]) or (False, {error_message})

    """
    # Check parameters
    if log:
        log.info('Scan and loading %s from "%s"...', module_name, root_path)
    if not os.path.exists(root_path):
        return (False, f'The directory("{root_path}") of {module_name} does not exists!')
    # find module files
    ignore_dirs = ('.vscode', 'test', 'docs', '__pycache__')
    path_name_list = []
    for root, dirs, files in os.walk(root_path):
        for ignore_dir in ignore_dirs:
            if ignore_dir in dirs:
                dirs.remove(ignore_dir)
        path_name_list += [os.path.join(root, file) for file in files if filter_function(root, file)]
    # Iterate over each module file
    service_module_list, application_path_size = ([], len(application_path))
    for service_index, service_path_name in enumerate(path_name_list):
        if log:
            log.info('    %s. Load %s module: %s...', service_index + 1, module_name, service_path_name[application_path_size + 1:])
        success, result_or_error = _load_module(application_path, service_path_name)
        if not success:
            if log:
                log.error('    %s. Load %s module failure(%s): %s', service_index + 1, module_name, service_path_name[application_path_size + 1:], result_or_error)
            return (success, result_or_error)
        service_module = result_or_error
        service_module_list.append(service_module)
    return (True, service_module_list)

def _load_module(application_path: str, module_path_name: str) -> tuple:
    """Load module

    Args:
        application_path: application path
        module_path_name: Module full filename

    Returns:
        return (True, {module_instance}) or (False, {error_message})

    """
    relative_path_name = module_path_name[len(application_path) + 1:]
    relative_path_base_name = os.path.splitext(relative_path_name)[0]
    module_name = relative_path_base_name.replace('\\', '.').replace('/', '.')
    try:
        imported_module = importlib.import_module(module_name)
    except ImportError as error:
        return (False, error.msg)
    except TypeError as error:
        return (False, error)
    return (True, imported_module)

def _read_ssl_file_content(server_ssl_root_path: str, ssl_file_name: str) -> bytes:
    """ read ssl file content

    Args:
        server_ssl_root_path: Server ssl authentication file root directory
        ssl_file_name: full filename

    Returns:
        return content

    """
    target_path_name = os.path.join(server_ssl_root_path, ssl_file_name)
    if os.path.exists(target_path_name):
        with open(target_path_name, 'rb') as file:
            return file.read()
    return b''

######################################## beast_utils handlers ########################################

def _server_shutdown_handler() -> None:
    """The shutdown handler is called when the server is going to to be shutdown"""
    log.info('Server will shutdown!')

def _handle_log(severity_level: int, message: str, content_offset: int) -> None:
    """log handle

    Args:
        severity_level: log level
        message: log message

    """
    log_dict = {-1: log.debug, 0: log.info, 1: log.warning, 2: log.error}
    log_func = log_dict.get(severity_level, log.error)
    log_func(message[content_offset:])

def _ssl_certificate_handler(server_ssl_root_path: str) -> bytes:
    """SSL certificate callback

    openssl req -newkey rsa:2048 -nodes -keyout server.key -x509 -days 10000 -out server.csr -subj "/C=CN/ST=HN/L=ChangSha/O=csew/CN=localhost"

    Args:
        server_ssl_root_path: Server ssl authentication file root directory

    Returns:
        return content

    """
    return _read_ssl_file_content(server_ssl_root_path, 'server.csr')

def _ssl_private_key_handler(server_ssl_root_path: str) -> bytes:
    """SSL private key callback

    Returns:
        return content

    """
    return _read_ssl_file_content(server_ssl_root_path, 'server.key')

def _ssl_dh_handler(server_ssl_root_path: str) -> bytes:
    """SSL DH callback

    openssl dhparam -out server.dh 2048

    Returns:
        return content

    """
    return _read_ssl_file_content(server_ssl_root_path, 'server.dh')

def _ssl_password_handler(is_writing: bool) -> str:  #pylint: disable=unused-argument
    """SSL password callback

    Returns:
        return SSL password

    """
    return "test"

def _ws_open_handler(connection_handle: int) -> None:
    """websocket open callback

    Args:
        connection_handle: websocket connection handle

    """
    log.info('    ws(%s) open...', connection_handle)

def _ws_close_handler(connection_handle: int) -> None:
    """websocket close callback

    Args:
        connection_handle: websocket connection handle

    """
    connection_name = model.ws_connection_get_name(connection_handle)
    log.info('    ws(%s: %s) closed!', connection_name or '', connection_handle)
    model.ws_connections_broadcast(connection_handle, f'{connection_name} logout.', False)

def _handle_ws_message(connection_handle: int, message: bytes):
    """ws message callback

    Args:
        connection_handle: websocket connection handle
        message: message content

    """
    if not model.ws_connection_get_name(connection_handle):  # Initialize connection name(The first message)
        model.ws_connection_set_name(connection_handle, message)
        log.info('    ws(%s: %s)login!', message.decode(), connection_handle)
        model.ws_connections_broadcast(connection_handle, f'{message.decode()} login.', True)
    else:  # other message
        log.info('    ws receive (%s) message: %s', model.ws_connection_get_name(connection_handle), message.decode())
        model.ws_connections_broadcast(connection_handle, message, False)

def _http_timeout_handle(connection_handle: int) -> int:
    """The timeout handler is called when an HTTP request is be receiving

    Args:
        connection_handle: websocket connection handle

    Returns:
        return the timeout in seconds

    """
    return 300

def _http_body_limit_handle(connection_handle: int) -> int:
    """The body limit handler is called when an HTTP request is be receiving.

    Args:
        connection_handle: websocket connection handle

    Returns:
        return the max size of body

    """
    return 1024 * 1024 * 1024

def _handle_http_request(server_user_data: int, raw_head: bytes, raw_body: bytes, response_cb: callable) -> None:
    """process http request

    Args:
        raw_head: headers of request
        raw_body: body of request

    """
    from bottle_glue import handle_http_request
    enter_handle = lambda name, url: log.info('%s(%s, ...) starting...', name, url)
    exit_handle = lambda name, url, elapsed_time: log.info('%s(%s, ...) elapsed: %.5s s', name, url, elapsed_time)
    with _create_http_request_profile_guard('handle_http_request', raw_head, enter_handle, exit_handle):
        return handle_http_request(server_user_data, raw_head, raw_body, response_cb)

class _HttpRequestProfileGuard(ContextDecorator):
    """http request profile guard"""
    def __init__(self, function_name, raw_head: bytes, enter_handle: Callable, exit_handle: Callable) -> None:
        self._function_name = function_name
        self._http_url = raw_head[:raw_head.find(b'\r')].decode()
        self._enter_handle = enter_handle
        self._exit_handle = exit_handle
    def __enter__(self):
        self._start_time = time.time()
        self._enter_handle(self._function_name, self._http_url)

        return self
    def __exit__(self, *exc):
        elapsed_time = time.time() - self._start_time
        self._exit_handle(self._function_name, self._http_url, elapsed_time)
        return False

def _create_http_request_profile_guard(function_name: str, raw_head: bytes, enter_handle: Callable, exit_handle: Callable) -> _HttpRequestProfileGuard:
    """create http request profile guard"""
    return _HttpRequestProfileGuard(function_name, raw_head, enter_handle, exit_handle)

########################################  main entry ########################################

if __name__ == '__main__':
    http_default_port = 80
    port_dict = {'Window': http_default_port, 'Linux': 8080}
    server_port, enable_ssl, concurrency_hint = (port_dict.get(platform.system(), http_default_port), True, 2)
    start_web_server = (_start_simple_web_server, _start_bottle_web_server)[-1]
    start_web_server(server_port, enable_ssl, concurrency_hint)
