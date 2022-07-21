// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of// this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/scaffold_handles.h"
#include <limits>
#include <memory>
#include "net/http_session_plain.h"
#include "net/http_session_ssl.h"
#include "net/listener.h"
#include "net/detect_session.h"
#include "src/app_resource.h"

uint32_t handle_http_body_limit(std::shared_ptr<virtual_enable_shared_from_this_base> sp_session) {
    uint32_t body_limit = std::numeric_limits<std::uint32_t>::max();
    auto handle_pair = scaffold_handles_get_instance()->http_body_limit_handler_pair;
    if (handle_pair.first) {
        body_limit = handle_pair.first(handle_pair.second, object_handle_from_pointer(sp_session));
    }
    return body_limit;
}

uint32_t handle_http_timeout_seconds(std::shared_ptr<virtual_enable_shared_from_this_base> sp_session) {
    uint32_t timeout_seconds = 300;
    auto handle_pair = scaffold_handles_get_instance()->http_timeout_handler_pair;
    if (handle_pair.first) {
        timeout_seconds = handle_pair.first(handle_pair.second, object_handle_from_pointer(sp_session));
    }
    return timeout_seconds;
}
class http_response_wrapper {
 public:
    typedef http_response_wrapper                                       this_type;
    typedef std::function<void(uintptr_t, const char*, uint32_t)>       handle_type;
    typedef std::shared_ptr<virtual_enable_shared_from_this_base>       session_type;

 public:
    http_response_wrapper(session_type session, handle_type handle) : session_(session), handle_(handle) {}

    static void http_respose_cb(uintptr_t this_handle, const char* response_content, uint32_t response_size) {
        handle_to_pointer<this_type>(this_handle)->http_respose_cb(response_content, response_size);
    }

 private:
    void http_respose_cb(const char* response_content, uint32_t response_size) {
        handle_(object_handle_from_pointer(session_), response_content, response_size);
    }

 private:
    session_type            session_;
    handle_type             handle_;
};

void handle_http_request(std::shared_ptr<virtual_enable_shared_from_this_base> sp_session, const char* head, const char* body, uint32_t body_size,
    std::function<void(uintptr_t, const char*, uint32_t)> response_cb) {
    auto handle_pair = scaffold_handles_get_instance()->http_handler_pair;
    if (handle_pair.first) {
        http_response_wrapper my_class(sp_session, response_cb);
        handle_pair.first(handle_pair.second, reinterpret_cast<uintptr_t>(&my_class), head, body, body_size, http_response_wrapper::http_respose_cb);
    }
}

void handle_ws_connection_open(std::shared_ptr<virtual_enable_shared_from_this_base> sp_ws_connection) {
    auto handle_pair = scaffold_handles_get_instance()->ws_open_handler_pair;
    if (handle_pair.first)
        handle_pair.first(handle_pair.second, reinterpret_cast<uintptr_t>(sp_ws_connection.get()));
}

void handle_ws_connection_close(virtual_enable_shared_from_this_base* ws_connection) {
    auto handle_pair = scaffold_handles_get_instance()->ws_close_handler_pair;
    if (handle_pair.first)
        handle_pair.first(handle_pair.second, reinterpret_cast<uintptr_t>(ws_connection));
}

void handle_ws_message(std::shared_ptr<virtual_enable_shared_from_this_base> sp_ws_connection, const char* message) {
    auto handle_pair = scaffold_handles_get_instance()->ws_message_handler_pair;
    if (handle_pair.first)
        handle_pair.first(handle_pair.second, reinterpret_cast<uintptr_t>(sp_ws_connection.get()), message);
}

void ws_connection_send(std::shared_ptr<virtual_enable_shared_from_this_base> sp_connection, const char* message) {
    if (sp_connection) {
        typedef plain_websocket_session plain_session_type;
        typedef ssl_websocket_session ssl_session_type;
        auto* ws_session = sp_connection.get();
        plain_session_type* plain_session = dynamic_cast<plain_session_type*>(ws_session);
        if (plain_session) {
            plain_session->send(message);
        } else {
            ssl_session_type* ssl_session = dynamic_cast<ssl_session_type*>(ws_session);
            if (ssl_session)
                ssl_session->send(message);
        }
    }
}

// this handle will be called when DetectSession parsed the request of client's connection
void handle_ssl_detect(bool ssl, boost::beast::tcp_stream&& stream, boost::beast::flat_buffer&& buffer) {
    if (ssl) {
        std::make_shared<ssl_http_session>(std::move(stream), get_ssl_context(), std::move(buffer),
                                     handle_http_body_limit, handle_http_timeout_seconds, handle_http_request)->run();
    } else {
        std::make_shared<plain_http_session>(std::move(stream), std::move(buffer), handle_http_body_limit,
                                       handle_http_timeout_seconds, handle_http_request)->run();
    }
}

// this handle will be called when listener received the request of client's connection
void handle_accept(boost::asio::ip::tcp::socket&& socket) {
    boost::asio::post(socket.get_executor(), [socket = std::move(socket)]() mutable {
        std::make_shared<detect_session>(std::move(socket), handle_ssl_detect)->run();
    });
}

// this handle will be called when listen thread is going to run
void handle_listen(boost::asio::io_context& ioc, uint16_t listen_port) {
    std::make_shared<listener>(ioc, listen_port, handle_accept)->run();
}
