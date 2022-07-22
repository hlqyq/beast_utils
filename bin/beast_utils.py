# -*- coding: utf-8 -*-

"""Advanced server flext of boost.beast for python

see: https://www.boost.org/doc/libs/1_79_0/libs/beast/doc/html/beast/examples.html#beast.examples.servers_advanced

"""

import ctypes
import os
import sys
import functools
import platform
import threading

beast_utils_dll_base_name, beast_utils_dll_version, os_system = ('beast_utils', '1.0.0', platform.system())
beast_utils_py_path = os.path.dirname(os.path.abspath(__file__))
if os_system == 'Linux':
    beast_utils_dll_file_name = f'lib{beast_utils_dll_base_name}.so.{beast_utils_dll_version}'
    c_uint = ctypes.c_uint64 if '64' in platform.architecture()[0] else ctypes.c_uint32
    beast_utils_dll_path = beast_utils_py_path
    _CURRENT_ENVIRON_PATH = os.environ['PATH']
elif os_system == 'Windows':
    AMD64 = '_64' if 'AMD64' in sys.version else ''
    beast_utils_dll_file_name = f'{beast_utils_dll_base_name}{AMD64}.dll'
    beast_utils_dll_path = beast_utils_py_path
    c_uint = ctypes.c_uint64 if AMD64 else ctypes.c_uint32
    _CURRENT_ENVIRON_PATH = os.environ['PATH']
else:
    raise OSError(f'Unkonwn os: {os_system}')
beast_utils_dll = ctypes.cdll.LoadLibrary(os.path.join(beast_utils_dll_path, beast_utils_dll_file_name))  #pylint: disable=invalid-name
os.environ['PATH'] = '{};{}'.format(_CURRENT_ENVIRON_PATH, os.path.dirname(__file__))

######################################## plugins ########################################

def plugin_initialize() -> tuple:
    """Plugin initialize: This function must be called first

    Returns:
        return (True, '') or (False, {error_message})

    """
    ERROR_CB_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_bool, ctypes.c_char_p)
    def _error_cb(error_list: list, success: bool, fail_error: bytes) -> None:
        if not success and fail_error:
            error_list.append(fail_error.decode())
    error_list = []
    error_handler = ERROR_CB_TYPE(functools.partial(_error_cb, error_list))
    func = beast_utils_dll.plugin_initialize
    func.restype = ctypes.c_bool
    success = func(error_handler)
    return (success, error_list[0] if error_list else '')

def plugin_final() -> None:
    """plugin release: This function should be called finally"""
    beast_utils_dll.plugin_final()

######################################## web server ########################################

def run_server(server_port: int = 80, ssl_support: bool = True, concurrency_hint: int = 1) -> int:
    """run server

    Args:
        server_port: server port
        ssl_support: enable ssl support
        concurrency_hint: the initial amount of thread

    Returns:
        return the exit code

    """
    func = beast_utils_dll.run_server
    func.restype = ctypes.c_int
    func.argtypes = [ctypes.c_uint16, ctypes.c_bool, ctypes.c_int32]
    func(server_port, ssl_support, concurrency_hint)

def shutdown_server() -> None:
    """shutdown server"""
    beast_utils_dll.shutdown_server()

SERVER_SHUTDOWN_HANDLER = ctypes.CFUNCTYPE(None, c_uint)
def set_server_shutdown_handler(handler) -> None:
    """set the notify handler when server will be shutdown

    Args:
        handler: def _() -> None

    """
    current_function, handler_type = set_server_shutdown_handler, SERVER_SHUTDOWN_HANDLER
    def _handler_wrapper(handler, user_data):  #pylint: disable=unused-argument
        handler()
    current_function.handler = handler_type(functools.partial(_handler_wrapper, handler))
    beast_utils_dll.set_server_shutdown_handler(current_function.handler, c_uint(0))

######################################## tasks ########################################

TASK_CB_TYPE = ctypes.CFUNCTYPE(None, c_uint)
def post_task(task_functor, delay_milliseconds, *args, **kwargs) -> None:
    """post a task in the special thread

    Args:
        task_functor: def _() -> None
        delay_milliseconds: delay execute milliseconds: 0 mean execute immediately

    """
    current_function, handler_type = post_task, TASK_CB_TYPE
    def _task_wrapper(task_functor, user_data):  #pylint: disable=unused-argument
        task_functor(*args, **kwargs)
    current_function.task = handler_type(functools.partial(_task_wrapper, task_functor))
    beast_utils_dll.post_task(current_function.task, c_uint(0), delay_milliseconds)

######################################## log handles ########################################

LOG_HANDLER = ctypes.CFUNCTYPE(None, c_uint, ctypes.c_int32, ctypes.c_char_p, ctypes.c_uint32)
def set_log_handler(handler) -> None:
    """Set the log handler

    Args:
        handler: def log_handler(severity_level: int, message: str, content_offset: int) -> None

    """
    current_function, handler_type = set_log_handler, LOG_HANDLER
    def _handler_wrapper(handler, user_data, severity_level: int, message: bytes, content_offset: int):  #pylint: disable=unused-argument
        try:
            message = message.decode()
        except UnicodeDecodeError:
            message = message.decode('GBK')
        handler(severity_level, message, content_offset)
    current_function.handler = handler_type(functools.partial(_handler_wrapper, handler))
    beast_utils_dll.set_log_handler(current_function.handler, c_uint(0))

def set_log_reporting_level(report_level: int) -> None:
    """ Set the report level of the log

    Args:
        report_level: the report level of the log

    """
    func = beast_utils_dll.set_log_reporting_level
    func.argtypes = [ctypes.c_int32,]
    func(report_level)

######################################## ssl handles ########################################

SSL_CERTIFICATE_HANDLER = ctypes.CFUNCTYPE(ctypes.c_uint32, ctypes.POINTER(ctypes.c_char), ctypes.c_uint32)
SSL_KEY_HANDLER = ctypes.CFUNCTYPE(ctypes.c_uint32, ctypes.POINTER(ctypes.c_char), ctypes.c_uint32)
SSL_DH_HANDLER = ctypes.CFUNCTYPE(ctypes.c_uint32, ctypes.POINTER(ctypes.c_char), ctypes.c_uint32)
SSL_PASSWORD_HANDLER = ctypes.CFUNCTYPE(ctypes.c_uint32, ctypes.c_bool, ctypes.POINTER(ctypes.c_char), ctypes.c_uint32)
def set_ssl_handler(certificate_handler: callable, key_handler: callable, db_handler: callable, password_handler: callable) -> None:
    """set SSL handlers

    Args:
        certificate_handler: def _certificate_handler() -> bytes
        key_handler: def _private_key_handler() -> bytes
        dh_handler: def _dh_handler() -> bytes
        password_handler: def _password_handler(is_writing: bool) -> bytes

    """
    current_function = set_ssl_handler
    def _certificate_handler_wrapper(buffer: bytes, buffer_max_size: int) -> int:
        result_buffer = certificate_handler()
        valid_size = min(buffer_max_size, len(result_buffer))
        ctypes.memmove(buffer, result_buffer, valid_size)
        return valid_size
    def _key_handler_wrapper(buffer: bytes, buffer_max_size: int) -> int:
        result_buffer = key_handler()
        valid_size = min(buffer_max_size, len(result_buffer))
        ctypes.memmove(buffer, result_buffer, valid_size)
        return valid_size
    def _db_handler_wrapper(buffer: bytes, buffer_max_size: int) -> int:
        result_buffer = db_handler()
        valid_size = min(buffer_max_size, len(result_buffer))
        ctypes.memmove(buffer, result_buffer, valid_size)
        return valid_size
    def _password_handler_wrapper(is_writing: bool, buffer: bytes, buffer_max_size: int) -> int:
        result_buffer = password_handler(is_writing)
        valid_size = min(buffer_max_size, len(result_buffer))
        ctypes.memmove(buffer, result_buffer, valid_size)
        return valid_size
    current_function.handlers = (SSL_CERTIFICATE_HANDLER(_certificate_handler_wrapper),
                                 SSL_KEY_HANDLER(_key_handler_wrapper),
                                 SSL_DH_HANDLER(_db_handler_wrapper),
                                 SSL_PASSWORD_HANDLER(_password_handler_wrapper))
    beast_utils_dll.set_ssl_handler(*current_function.handlers)

######################################## http handles ########################################

HTTP_HANDLER_CB = ctypes.CFUNCTYPE(None, c_uint, ctypes.c_char_p, ctypes.c_uint32)
HTTP_HANDLER = ctypes.CFUNCTYPE(None, c_uint, c_uint, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_uint32, HTTP_HANDLER_CB)  #pylint: disable=line-too-long
def set_http_handler(handler) -> None:
    """set http handler

    Args:
        handler: def http_handler(server_user_data: int, raw_head: bytes, raw_body: bytes, response_cb: HTTP_HANDLER_CB) -> None

    """
    current_function, handler_type = set_http_handler, HTTP_HANDLER
    def _handler_wrapper(user_data, server_user_data, raw_head: bytes, raw_body: bytes, body_size: int, response_cb) -> None:  #pylint: disable=unused-argument, too-many-arguments
        handler(server_user_data, raw_head, raw_body[:body_size], response_cb)
    current_function.handler = handler_type(_handler_wrapper)
    beast_utils_dll.set_http_handler(current_function.handler, c_uint(0))

HTTP_TIMEOUT_HANDLER = ctypes.CFUNCTYPE(ctypes.c_uint32, c_uint, c_uint)
def set_http_timeout_handler(handler) -> int:
    """set http timeout handler

    Args:
        handler: def _(connection_handle: int) -> int

    """
    current_function, handler_type = set_http_timeout_handler, HTTP_TIMEOUT_HANDLER
    def _handler_wrapper(user_data, server_user_data) -> None:  #pylint: disable=unused-argument
        return handler(server_user_data)
    current_function.handler = handler_type(_handler_wrapper)
    beast_utils_dll.set_http_timeout_handler(current_function.handler, c_uint(0))

HTTP_BODY_LIMIT_HANDLER = ctypes.CFUNCTYPE(c_uint, c_uint, c_uint)
def set_http_body_limit_handler(handler) -> int:
    """set http body limit handler

    Args:
        handler: def _(connection_handle: int) -> int

    """
    current_function, handler_type = set_http_body_limit_handler, HTTP_BODY_LIMIT_HANDLER
    def _handler_wrapper(user_data, server_user_data) -> None:  #pylint: disable=unused-argument
        return handler(server_user_data)
    current_function.handler = handler_type(_handler_wrapper)
    beast_utils_dll.set_http_body_limit_handler(current_function.handler, c_uint(0))

######################################## ws handles ########################################

def ws_connection_send(connection_handle: int, message: str) -> None:
    """Send message to ws connection

    Args:
        connection_handle: ws connection handle
        message: content

    """
    func = beast_utils_dll.ws_connection_send
    func.argtypes = [c_uint, ctypes.c_char_p]
    func(connection_handle, message.encode() if isinstance(message, str) else message)

WS_MESSAGE_HANDLER = ctypes.CFUNCTYPE(None, c_uint, c_uint, ctypes.c_char_p)
def ws_set_message_handler(handler) -> None:
    """set ws message handler

    Args:
        handler: def message_handler(connection_handle: int, message: bytes) -> None

    """
    current_function, handler_type = ws_set_message_handler, WS_MESSAGE_HANDLER
    def _handler_wrapper(user_data, connection_handle: int, message: bytes):  #pylint: disable=unused-argument
        handler(connection_handle, message)
    current_function.handler = handler_type(_handler_wrapper)
    beast_utils_dll.ws_set_message_handler(current_function.handler, c_uint(0))

WS_OPEN_HANDLER = ctypes.CFUNCTYPE(None, c_uint, c_uint)
def ws_set_open_handler(handler) -> None:
    """set ws open handler

    Args:
        handler: def _ws_open_handler(connection_handle: int) -> None

    """
    current_function, handler_type = ws_set_open_handler, WS_OPEN_HANDLER
    def _handler_wrapper(user_data, connection_handle: int):  #pylint: disable=unused-argument
        ws_connection_register(connection_handle)
        handler(connection_handle)
    current_function.handler = handler_type(_handler_wrapper)
    beast_utils_dll.ws_set_open_handler(current_function.handler, c_uint(0))

WS_CLOSE_HANDLER = ctypes.CFUNCTYPE(None, c_uint, c_uint)
def ws_set_close_handler(handler) -> None:
    """set ws close handler

    Args:
        handler: def _ws_close_handler(connection_handle: int) -> None

    """
    current_function, handler_type = ws_set_close_handler, WS_CLOSE_HANDLER
    def _handler_wrapper(user_data, connection_handle: int):  #pylint: disable=unused-argument
        handler(connection_handle)
        ws_connection_unregister(connection_handle)
    current_function.handler = handler_type(_handler_wrapper)
    beast_utils_dll.ws_set_close_handler(current_function.handler, c_uint(0))

######################################## ws extension utils ########################################

def ws_connections_visit(visit_cb):
    """visit all ws connection

    Args:
        visit_cb: visit callback: def _(connection_handle)

    """
    lock_instance, ws_connection_dict = _get_ws_connections_pair()
    with lock_instance:
        for connection_handle in ws_connection_dict:
            result = visit_cb(connection_handle)
            if result:
                return result
        return None

def ws_connection_has(connection_handle: int) -> bool:
    """Check if the connection exists

    Args:
        connection_handle: connection's handle

    Returns:
        return whether the connection exists

    """
    lock_instance, ws_connection_dict = _get_ws_connections_pair()
    with lock_instance:
        return ws_connection_dict.get(connection_handle)

def ws_connection_set_name(connection_handle: int, name: str) -> None:
    """set the connection's name

    Args:
        connection_handle: connection's handle
        name:  connection's name

    """
    lock_instance, ws_connection_dict = _get_ws_connections_pair()
    with lock_instance:
        ws_connection_dict[connection_handle] = name.decode() if isinstance(name, bytes) else name

def ws_connection_register(connection_handle: int) -> None:
    """register the connection

    Args:
        connection_handle: connection's handle

    """
    lock_instance, ws_connection_dict = _get_ws_connections_pair()
    with lock_instance:
        ws_connection_dict[connection_handle] = ''

def ws_connection_unregister(connection_handle: int) -> None:
    """Remove the connection

    Args:
        connection_handle: connection's handle

    """
    lock_instance, ws_connection_dict = _get_ws_connections_pair()
    with lock_instance:
        if connection_handle in ws_connection_dict:
            del ws_connection_dict[connection_handle]

def ws_connection_get_name(connection_handle: int) -> str:
    """Get the connection's name

    Args:
        connection_handle: connection's handle

    Returns:
        return the connection's name

    """
    lock_instance, ws_connection_dict = _get_ws_connections_pair()
    with lock_instance:
        return ws_connection_dict.get(connection_handle)

def ws_connections_broadcast(origin_connection_handle: int, message: bytes, broadcast_origin: bool = False):
    """broadcast message

    Args:
        origin_connection_handle: origin connection's handle
        message: content
        broadcast_origin: whether the broadcast contain itself

    """
    def _visit_cb(filter_cb, message: bytes, connection_handle: int) -> None:
        if filter_cb(connection_handle):
            ws_connection_send(connection_handle, message)
    filter_cb = lambda connection_handle: broadcast_origin or (connection_handle != origin_connection_handle)
    ws_connections_visit(functools.partial(_visit_cb, filter_cb, message))

######################################## implements ########################################

def _get_ws_connections_pair() -> tuple:
    """ get ws connection

    Returns:
        return ({lock_instance: threading.Lock}, {ws_connection_dict: dict})
    """
    func, tag_name = (_get_ws_connections_pair, '__cached')
    if not hasattr(func, tag_name):
        setattr(func, tag_name, (threading.Lock(), {}))
    return getattr(func, tag_name)
