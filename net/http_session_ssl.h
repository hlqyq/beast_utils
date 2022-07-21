// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_HTTP_SESSION_SSL_H_
#define NET_HTTP_SESSION_SSL_H_

#include <utility>
#include <boost/beast/ssl.hpp>
#include "net/http_session.hpp"

class ssl_http_session : public http_session<ssl_http_session>, public virtual_enable_shared_from_this<ssl_http_session> {
 public:
    typedef ssl_http_session                                            this_type;
    typedef http_session<ssl_http_session>                              base_type;
    typedef boost::beast::ssl_stream<boost::beast::tcp_stream>          tcp_stream_type;
    typedef boost::asio::ssl::context                                   ssl_context_type;
    typedef boost::beast::flat_buffer                                   flat_buffer_type;

 public:
     ssl_http_session(boost::beast::tcp_stream&& stream, ssl_context_type* ctx, flat_buffer_type&& buffer, limit_handle_type limit_handle,
                      timeout_handle_type timeout_handle, request_handle_type request_handle);
     ~ssl_http_session(void);
     explicit ssl_http_session(const this_type&) = delete;
     this_type& operator=(const this_type&) = delete;

 public:
     void run(void);

    // Called by the base class
    tcp_stream_type& stream(void) { return stream_; }

    // Called by the base class
    tcp_stream_type release_stream(void) { return std::move(stream_); }

    // Called by the base class
    void do_eof(void);

 private:
     void on_handshake(boost::beast::error_code ec, std::size_t bytes_used);
     void on_shutdown(boost::beast::error_code ec);

 private:
    tcp_stream_type                        stream_;
};

#endif  //  NET_HTTP_SESSION_SSL_H_
