// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_HTTP_SESSION_PLAIN_H_
#define NET_HTTP_SESSION_PLAIN_H_

#include <utility>
#include "net/http_session.hpp"

class plain_http_session : public http_session<plain_http_session>, public virtual_enable_shared_from_this<plain_http_session> {
 public:
     typedef plain_http_session                                         this_type;
     typedef http_session<this_type>                                    base_type;
    typedef boost::beast::tcp_stream                                    tcp_stream_type;
    typedef boost::beast::flat_buffer                                   flat_buffer_type;

 public:
     plain_http_session(tcp_stream_type&& stream, flat_buffer_type&& buffer, limit_handle_type limit_handle,
                        timeout_handle_type timeout_handle, request_handle_type request_handle);
     ~plain_http_session(void);
    explicit plain_http_session(const this_type&) = delete;
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
    tcp_stream_type                        stream_;
};

#endif  // NET_HTTP_SESSION_PLAIN_H_
