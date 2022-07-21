// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef SRC_SCAFFOLD_HANDLES_H_
#define SRC_SCAFFOLD_HANDLES_H_

#include <utility>
#include <memory>
#include <boost/asio/io_context.hpp>
#include "include/beast_utils.h"
#include "base/memory_utils_base.hpp"

struct scaffold_handles {
 public:
    typedef uintptr_t                                                       user_data_type;
    typedef std::pair<http_handler_type, user_data_type>                    http_handler_pair_type;
    typedef std::pair<http_timeout_handler_type, user_data_type>            http_timeout_handler_pair_type;
    typedef std::pair<http_body_limit_handler_type, user_data_type>         http_body_limit_handler_pair_type;
    typedef std::pair<ws_open_handler_type, user_data_type>                 ws_open_handler_pair_type;
    typedef std::pair<ws_close_handler_type, user_data_type>                ws_close_handler_pair_type;
    typedef std::pair<ws_message_handler_type, user_data_type>              ws_message_handler_pair_type;
    typedef std::pair<server_shutdown_handler_type, user_data_type>         server_shutdown_handler_pair_type;

 public:
    scaffold_handles(void) : ssl_certificate_handler(nullptr), ssl_key_handler(nullptr), ssl_db_handller(nullptr), ssl_password_handler(nullptr) {}

 public:
    ssl_certificate_cb_type                     ssl_certificate_handler;
    ssl_key_cb_type                             ssl_key_handler;
    ssl_dh_cb_type                              ssl_db_handller;
    ssl_password_cb_type                        ssl_password_handler;
    http_handler_pair_type                      http_handler_pair;
    http_timeout_handler_pair_type              http_timeout_handler_pair;
    http_body_limit_handler_pair_type           http_body_limit_handler_pair;
    ws_open_handler_pair_type                   ws_open_handler_pair;
    ws_close_handler_pair_type                  ws_close_handler_pair;
    ws_message_handler_pair_type                ws_message_handler_pair;
    server_shutdown_handler_pair_type           server_shutdown_handler_pair;
};

extern scaffold_handles* scaffold_handles_get_instance(void);
void handle_listen(boost::asio::io_context& ioc, uint16_t listen_port);
void ws_connection_send(std::shared_ptr<virtual_enable_shared_from_this_base> sp_connection, const char* message);
void handle_ws_connection_open(std::shared_ptr<virtual_enable_shared_from_this_base> sp_ws_connection);
void handle_ws_connection_close(virtual_enable_shared_from_this_base* ws_connection);
void handle_ws_message(std::shared_ptr<virtual_enable_shared_from_this_base> sp_ws_connection, const char* message);

#endif  // SRC_SCAFFOLD_HANDLES_H_
