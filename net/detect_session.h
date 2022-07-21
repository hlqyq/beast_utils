// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_DETECT_SESSION_H_
#define NET_DETECT_SESSION_H_

#include <boost/beast/core.hpp>
#include "base/utils.h"

//////////////////////////////////////// declarations ////////////////////////////////////////

class detect_session : public std::enable_shared_from_this<detect_session> {
 public:
    typedef detect_session                                      this_type;
    typedef boost::beast::tcp_stream                            tcp_stream_type;
    typedef boost::beast::flat_buffer                           flat_buffer_type;
    typedef boost::asio::ip::tcp::socket                        socket_type;
    typedef boost::beast::error_code                            error_code_type;
    typedef std::function<void(bool ssl, boost::beast::tcp_stream&& stream, boost::beast::flat_buffer&& buffer)> handle_type;

 public:
     detect_session(socket_type&& socket, handle_type handle);
     ~detect_session(void);
     explicit detect_session(const this_type&) = delete;
     this_type& operator=(const this_type&) = delete;

 public:
    void run(void);

 private:
    void on_run(void);
    void on_detect(error_code_type ec, bool result);

 private:
    tcp_stream_type             stream_;
    flat_buffer_type            buffer_;
    handle_type                 handle_;
    INSTANCE_LOG_DECLARE;
};

#endif  // NET_DETECT_SESSION_H_
