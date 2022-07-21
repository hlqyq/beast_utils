// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_WEBSOCKET_SESSION_SSL_H_
#define NET_WEBSOCKET_SESSION_SSL_H_

#include <utility>
#include <boost/beast/ssl.hpp>
#include "net/websocket_session.hpp"

class ssl_websocket_session : public websocket_session<ssl_websocket_session>,
                              public virtual_enable_shared_from_this<ssl_websocket_session> {
 public:
    typedef websocket_session<ssl_websocket_session>                                base_type;
    typedef ssl_websocket_session                                                   this_type;
    typedef boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream> > ws_stream_type;

 public:
     explicit ssl_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream>&& stream, open_handle_type open_handle,
                                    close_handle_type close_handle, message_handle_type message_handle):
                                    base_type(open_handle, close_handle, message_handle), ws_(std::move(stream)) {}
     ~ssl_websocket_session(void) {}

 public:
    // Called by the base class
    ws_stream_type& ws(void) { return ws_; }

 private:
    ws_stream_type                ws_;
};

#endif  // NET_WEBSOCKET_SESSION_SSL_H_
