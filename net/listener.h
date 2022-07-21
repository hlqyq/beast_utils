// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_LISTENER_H_
#define NET_LISTENER_H_

#include <boost/beast/core.hpp>

//////////////////////////////////////// declarations ////////////////////////////////////////

class listener : public std::enable_shared_from_this<listener> {
 public:
    typedef listener                                                this_type;
    typedef boost::asio::io_context                                 io_context_type;
    typedef boost::asio::ip::tcp::acceptor                          acceptor_type;
    typedef boost::asio::ip::tcp::socket                            socket_type;
    typedef boost::asio::ip::tcp::endpoint                          endpoint_type;
    typedef boost::beast::error_code                                error_code_type;
    typedef std::function<void(boost::asio::ip::tcp::socket&& socket)> handle_type;

 public:
    listener(io_context_type& ioc, unsigned short port, handle_type handle);
    ~listener(void);
    explicit listener(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

 public:
     void run(void) { do_accept(); }

 private:
    void do_accept(void);
    void on_accept(error_code_type ec, socket_type socket);

 private:
    io_context_type&            ioc_;
    acceptor_type               acceptor_;
    handle_type                 handle_;
};

#endif  // NET_LISTENER_H_
