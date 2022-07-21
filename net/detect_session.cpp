// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/detect_session.h"
#include <utility>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/asio/dispatch.hpp>
#include "net/net_utils.h"

detect_session::detect_session(socket_type&& socket, handle_type handle) : stream_(std::move(socket)), handle_(handle), INSTANCE_LOG_IMPL {
}

detect_session::~detect_session(void) {
}

void detect_session::run(void) {
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    boost::asio::dispatch(stream_.get_executor(), boost::beast::bind_front_handler(&detect_session::on_run, this->shared_from_this()));
}

void detect_session::on_run(void) {
    // Set the timeout.
    stream_.expires_after(std::chrono::seconds(30));
    boost::beast::async_detect_ssl(stream_, buffer_, boost::beast::bind_front_handler(&detect_session::on_detect,
        this->shared_from_this()));
}

void detect_session::on_detect(error_code_type ec, bool result) {
    if (ec) {
        return handle_error(ec, "detect_session.on_detect");
    } else {
        LOG(VERBOSE) << "detect_session.detected(" << boost::lexical_cast<std::string>(stream_.socket().remote_endpoint()) << " ==> "
            << boost::lexical_cast<std::string>(stream_.socket().local_endpoint()) << "): " << (result ? "SSL http" : "plain http");

        handle_(result, std::move(stream_), std::move(buffer_));
    }
}
