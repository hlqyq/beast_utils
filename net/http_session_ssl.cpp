// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http_session_ssl.h"

ssl_http_session::ssl_http_session(boost::beast::tcp_stream&& stream, ssl_context_type* ctx, flat_buffer_type&& buffer,
                                   limit_handle_type limit_handle, timeout_handle_type timeout_handle, request_handle_type request_handle):
                                   base_type(std::move(buffer), limit_handle, timeout_handle, request_handle), stream_(std::move(stream), *ctx) {
}

ssl_http_session::~ssl_http_session(void) {
}

void ssl_http_session::run(void) {
    // Set the timeout.
    boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    // Note, this is the buffered version of the handshake.
    stream_.async_handshake(boost::asio::ssl::stream_base::server, base_type::buffer_.data(),
        boost::beast::bind_front_handler(&ssl_http_session::on_handshake, this->shared_from_this()));
}

void ssl_http_session::do_eof(void) {
    // Set the timeout.
    boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Perform the SSL shutdown
    stream_.async_shutdown(boost::beast::bind_front_handler(&ssl_http_session::on_shutdown, this->shared_from_this()));
}

void ssl_http_session::on_handshake(boost::beast::error_code ec, std::size_t bytes_used) {
    if (ec) {
        handle_error(ec, "ssl_http_session.on_handshake");
    }

    // Consume the portion of the buffer used by the handshake
    this->buffer_.consume(bytes_used);

    this->do_read();
}

void ssl_http_session::on_shutdown(boost::beast::error_code ec) {
    if (ec)
        return handle_error(ec, "ssl_http_session.on_shutdown");

    // At this point the connection is closed gracefully
}
