// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_WEBSOCKET_SESSION_PLAIN_H_
#define NET_WEBSOCKET_SESSION_PLAIN_H_

#include <utility>
#include "net/websocket_session.hpp"

class plain_websocket_session : public websocket_session<plain_websocket_session>,
                                public virtual_enable_shared_from_this<plain_websocket_session> {
 public:
    typedef websocket_session<plain_websocket_session>                              base_type;
    typedef plain_websocket_session                                                 this_type;
    typedef boost::beast::websocket::stream<boost::beast::tcp_stream>               ws_stream_type;

 public:
    explicit plain_websocket_session(boost::beast::tcp_stream&& stream, open_handle_type open_handle, close_handle_type close_handle,
                        message_handle_type message_handle) : base_type(open_handle, close_handle, message_handle), ws_(std::move(stream)) {}
    ~plain_websocket_session(void) {}

 public:
    // Called by the base class
    ws_stream_type& ws(void) { return ws_;}

 private:
    ws_stream_type                ws_;
};

#endif  // NET_WEBSOCKET_SESSION_PLAIN_H_
