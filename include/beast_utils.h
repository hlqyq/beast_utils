// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef INCLUDE_BEAST_UTILS_H_
#define INCLUDE_BEAST_UTILS_H_

#ifdef BEASTUTILS_EXPORTS
    // Building the library
    #ifdef _WIN32
        // Use the Windows-specific export attribute
        #define BU_API extern "C" __declspec(dllexport)
    #elif __GNUC__ >= 4
        // Use the GCC-specific export attribute
        #define BU_API extern "C" __attribute__((visibility("default")))
    #else
        // Assume that no export attributes are needed
        #define BU_API extern "C"
    #endif
#else
    // Using (including) the library
    #ifdef _WIN32
        // Use the Windows-specific import attribute
        #define BU_API extern "C" __declspec(dllimport)
    #else
        // Assume that no import attributes are needed
        #define BU_API extern "C"
    #endif
#endif

//////////////////////////////////////// plugins ////////////////////////////////////////

// This function must be called first
typedef void (*plugin_initialize_result_handler_type)(bool success, const char* fail_error);
BU_API bool plugin_initialize(plugin_initialize_result_handler_type handler);
// This function should be called finally
BU_API void plugin_final(void);

//////////////////////////////////////// web server ////////////////////////////////////////

BU_API int run_server(int port, bool ssl, int concurrency_hint);
BU_API void shutdown_server(void);

// The shutdown handler is called when the server is going to to be shutdown.
typedef void (*server_shutdown_handler_type)(uintptr_t user_data);
BU_API void set_server_shutdown_handler(server_shutdown_handler_type handle_cb, uintptr_t user_data);

//////////////////////////////////////// tasks ////////////////////////////////////////

typedef void (*task_cb_type)(uintptr_t user_data);
BU_API void post_task(task_cb_type task, uintptr_t user_data, unsigned int delay_milliseconds);

//////////////////////////////////////// log handles ////////////////////////////////////////

typedef void(*log_handler_type)(uintptr_t user_data, int severity_level, const char* message, uint32_t message_content_offset);
BU_API void set_log_handler(log_handler_type handle_cb, uintptr_t user_data, bool multi_threaded);
BU_API void set_log_reporting_level(int severity_level);

//////////////////////////////////////// ssl handles ////////////////////////////////////////

// The ssl handler is called when a ssl handshake is establishing.
typedef unsigned int (*ssl_certificate_cb_type)(char* buffer, unsigned int buffer_size);
typedef unsigned int (*ssl_key_cb_type)(char* buffer, unsigned int buffer_size);
typedef unsigned int (*ssl_dh_cb_type)(char* buffer, unsigned int buffer_size);
typedef unsigned int (*ssl_password_cb_type)(bool is_write, char* buffer, unsigned int buffer_size);
BU_API void set_ssl_handler(ssl_certificate_cb_type certificate_cb, ssl_key_cb_type key_cb, ssl_dh_cb_type dh_cb, ssl_password_cb_type password_cb);

//////////////////////////////////////// http handles ////////////////////////////////////////

// The http handler is called after an HTTP request is received.
typedef void (*http_respose_cb_type)(uintptr_t session_handle, const char* response_content, uint32_t response_size);
typedef void (*http_handler_type)(uintptr_t user_data, uintptr_t session_handle, const char* http_head, const char* http_body,
    unsigned int http_body_size, http_respose_cb_type response_cb);
BU_API void set_http_handler(http_handler_type handle_cb, uintptr_t user_data);

// The timeout handler is called when an HTTP request is be receiving.
typedef uint32_t(*http_timeout_handler_type)(uintptr_t user_data, uintptr_t session_handle);
BU_API void set_http_timeout_handler(http_timeout_handler_type handle_cb, uintptr_t user_data);

// The body limit handler is called when an HTTP request is be receiving.
typedef uint32_t(*http_body_limit_handler_type)(uintptr_t user_data, uintptr_t session_handle);
BU_API void set_http_body_limit_handler(http_body_limit_handler_type handle_cb, uintptr_t user_data);

//////////////////////////////////////// ws handles ////////////////////////////////////////

BU_API void ws_connection_send(uintptr_t connection, const char* message);

// The message handler is called after a new message has been received.
typedef void (*ws_message_handler_type)(uintptr_t user_data, uintptr_t session_handle, const char* message_content);
BU_API void ws_set_message_handler(ws_message_handler_type handle_cb, uintptr_t user_data);

// The open handler is called after the WebSocket handshake is complete and the connection is considered OPEN.
typedef void (*ws_open_handler_type)(uintptr_t user_data, uintptr_t session_handle);
BU_API void ws_set_open_handler(ws_open_handler_type handle_cb, uintptr_t user_data);

// The close handler is called immediately after the connection is closed.
typedef void (*ws_close_handler_type)(uintptr_t user_data, uintptr_t session_handle);
BU_API void ws_set_close_handler(ws_close_handler_type handle_cb, uintptr_t user_data);

#endif  // INCLUDE_BEAST_UTILS_H_
