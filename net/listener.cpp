// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/listener.h"
#include <utility>
#include <string>
#include <boost/asio/strand.hpp>
#include <boost/lexical_cast.hpp>
#include "base/utils.h"
#include "net/net_utils.h"

listener::listener(io_context_type& ioc, unsigned short port, handle_type handle) : ioc_(ioc),
                   acceptor_(boost::asio::make_strand(ioc)), handle_(handle) {
    auto const address = boost::asio::ip::make_address("0.0.0.0");
    endpoint_type endpoint_instance{address, static_cast<uint_least16_t>(port)};
    error_code_type ec;
    acceptor_.open(endpoint_instance.protocol(), ec);
    if (ec) {
        handle_error(ec, "Listener.open");
        return;
    }

    acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec) {
        handle_error(ec, "Listener.set_option");
        return;
    }

    acceptor_.bind(endpoint_instance, ec);
    if (ec) {
        handle_error(ec, "Listener.bind");
        return;
    }

    acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec)
        handle_error(ec, "Listener.Listener");
}

listener::~listener(void) {
}

void listener::do_accept(void) {
    LOG(VERBOSE) << "Listener.Listenering(" << boost::lexical_cast<std::string>(acceptor_.local_endpoint()) << ")...";

    // The new connection gets its own strand
    acceptor_.async_accept(boost::asio::make_strand(ioc_), boost::beast::bind_front_handler(&this_type::on_accept, shared_from_this()));
}

void listener::on_accept(error_code_type ec, socket_type socket) {
    if (ec) {
        handle_error(ec, "Listener.accept");
    } else {
        LOG(VERBOSE) << "Listener.received(" << boost::lexical_cast<std::string>(socket.remote_endpoint())
            << " ==> " << boost::lexical_cast<std::string>(socket.local_endpoint()) << ").";
        handle_(std::move(socket));
    }

    // Accept another connection
    do_accept();
}
