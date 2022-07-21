// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_WEBSOCKET_SESSION_FACTORY_HPP_
#define NET_WEBSOCKET_SESSION_FACTORY_HPP_

#include <memory>
#include <utility>
#include "net/websocket_session_plain.h"
#include "net/websocket_session_ssl.h"
#include "src/scaffold_handles.h"

template<class Body, class Allocator>
inline void make_websocket_session(boost::beast::tcp_stream stream,
                                   boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req) {
    std::make_shared<plain_websocket_session>(std::move(stream), handle_ws_connection_open,
                                        handle_ws_connection_close, handle_ws_message)->run(std::move(req));
}

template<class Body, class Allocator>
inline void make_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream> stream,
                                   boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req) {
    std::make_shared<ssl_websocket_session>(std::move(stream), handle_ws_connection_open,
                                      handle_ws_connection_close, handle_ws_message)->run(std::move(req));
}

#endif  // NET_WEBSOCKET_SESSION_FACTORY_HPP_
