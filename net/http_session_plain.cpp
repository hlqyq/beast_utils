// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http_session_plain.h"

plain_http_session::plain_http_session(tcp_stream_type&& stream, flat_buffer_type&& buffer, limit_handle_type limit_handle,
                                       timeout_handle_type timeout_handle, request_handle_type request_handle):
                                       base_type(std::move(buffer), limit_handle, timeout_handle, request_handle), stream_(std::move(stream)) {
}

plain_http_session::~plain_http_session(void) {
}

void plain_http_session::run(void) {
    this->do_read();
}

void plain_http_session::do_eof(void) {
    // Send a TCP shutdown
    boost::beast::error_code ec;
    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    // At this point the connection is closed gracefully
}
