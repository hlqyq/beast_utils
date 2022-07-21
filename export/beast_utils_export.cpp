// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include "include/beast_utils.h"
#include "src/app_resource.h"
#include "base/utils.h"
#include "base/memory_utils.hpp"
#include "base/task_utils.hpp"

//////////////////////////////////////// plugins ////////////////////////////////////////

BU_API bool plugin_initialize(plugin_initialize_result_handler_type handler) {
    std::string error;
    auto success = app_resource_get_instance()->init(&error);
    if (handler)
        handler(success, error.c_str());
    return success;
}

BU_API void plugin_final(void) {
}

//////////////////////////////////////// web server ////////////////////////////////////////

BU_API int run_server(int port, bool ssl, int concurrency_hint) {
    return app_resource_get_instance()->run_server(port, ssl, concurrency_hint);
}

BU_API void shutdown_server(void) {
    app_resource_get_instance()->shutdown_server();
}

BU_API void set_server_shutdown_handler(server_shutdown_handler_type handle_cb, uintptr_t user_data) {
    scaffold_handles_get_instance()->server_shutdown_handler_pair = std::make_pair(handle_cb, user_data);
}

//////////////////////////////////////// tasks ////////////////////////////////////////

BU_API void post_task(task_cb_type task, uintptr_t user_data, unsigned int delay_milliseconds) {
    post_task([task, user_data]() { task(user_data); }, delay_milliseconds);
}

//////////////////////////////////////// log handles ////////////////////////////////////////

BU_API void set_log_handler(log_handler_type handle_cb, uintptr_t user_data, bool multi_threaded) {
    ::set_log_handle(handle_cb, user_data, multi_threaded);
}

BU_API void set_log_reporting_level(int severity_level) {
    log_reporting_level() = severity_level;
}

//////////////////////////////////////// ssl handles ////////////////////////////////////////

BU_API void set_ssl_handler(ssl_certificate_cb_type certificate_cb, ssl_key_cb_type key_cb, ssl_dh_cb_type dh_cb, ssl_password_cb_type password_cb) {
    scaffold_handles_get_instance()->ssl_certificate_handler = certificate_cb;
    scaffold_handles_get_instance()->ssl_key_handler = key_cb;
    scaffold_handles_get_instance()->ssl_db_handller = dh_cb;
    scaffold_handles_get_instance()->ssl_password_handler = password_cb;
}

//////////////////////////////////////// http handles ////////////////////////////////////////

BU_API void set_http_handler(http_handler_type handle_cb, uintptr_t user_data) {
    scaffold_handles_get_instance()->http_handler_pair = std::make_pair(handle_cb, user_data);
}

BU_API void set_http_timeout_handler(http_timeout_handler_type handle_cb, uintptr_t user_data) {
    scaffold_handles_get_instance()->http_timeout_handler_pair = std::make_pair(handle_cb, user_data);
}

BU_API void set_http_body_limit_handler(http_body_limit_handler_type handle_cb, uintptr_t user_data) {
    scaffold_handles_get_instance()->http_body_limit_handler_pair = std::make_pair(handle_cb, user_data);
}

//////////////////////////////////////// ws handles ////////////////////////////////////////

BU_API void ws_connection_send(uintptr_t connection, const char* message) {
    ws_connection_send(object_handle_to_pointer(connection), message);
}

BU_API void ws_set_message_handler(ws_message_handler_type handle_cb, uintptr_t user_data) {
    scaffold_handles_get_instance()->ws_message_handler_pair = std::make_pair(handle_cb, user_data);
}

BU_API void ws_set_open_handler(ws_open_handler_type handle_cb, uintptr_t user_data) {
    scaffold_handles_get_instance()->ws_open_handler_pair = std::make_pair(handle_cb, user_data);
}

BU_API void ws_set_close_handler(ws_close_handler_type handle_cb, uintptr_t user_data) {
    scaffold_handles_get_instance()->ws_close_handler_pair = std::make_pair(handle_cb, user_data);
}
