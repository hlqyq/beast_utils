// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_HTTP_SESSION_HPP_
#define NET_HTTP_SESSION_HPP_

#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/lexical_cast.hpp>
#include "base/memory_utils.hpp"
#include "net/websocket_session_factory.hpp"
#include "net/net_utils.h"
#include "base/utils.h"

template<class Derived>
class http_session : virtual public virtual_enable_shared_from_this_base {
 public:
    typedef Derived                                                                                 derived_type;
    typedef http_session<derived_type>                                                              this_type;
    typedef boost::beast::flat_buffer                                                               flat_buffer_type;
    typedef boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>>    parser_type;
    typedef std::shared_ptr<virtual_enable_shared_from_this_base>                                   object_pointer_type;
    typedef std::function<uint32_t(object_pointer_type)>                                            limit_handle_type;
    typedef std::function<uint32_t(object_pointer_type)>                                            timeout_handle_type;
    typedef std::function<void(uintptr_t, const char*, uint32_t)>                                   response_handle_type;
    typedef std::function<void(object_pointer_type, const char*, const char*, uint32_t, response_handle_type response_cb)>  request_handle_type;
    // This queue is used for HTTP pipelining.
    class queue {
        enum{limit = 8};  // Maximum number of responses we will queue

        // The type-erased, saved work item
        struct work {
            virtual ~work() = default;
            virtual void operator()() = 0;
        };

        http_session& self_;
        std::vector<std::unique_ptr<work>> items_;

     public:
        explicit queue(http_session& self) : self_(self) {
            static_assert(limit > 0, "queue limit must be positive");
            items_.reserve(limit);
        }

        // Returns `true` if we have reached the queue limit
        bool is_full(void) const {
            return items_.size() >= limit;
        }

        // Called when a message finishes sending
        // Returns `true` if the caller should initiate a read
        bool on_write(void) {
            BOOST_ASSERT(!items_.empty());
            auto const was_full = is_full();
            items_.erase(items_.begin());
            if (!items_.empty())
                (*items_.front())();
            return was_full;
        }

        // Called by the HTTP handler to send a response.
        template<bool isRequest, class Body, class Fields>
        void operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg) {
            // This holds a work item
            struct work_impl : work {
                http_session& self_;
                boost::beast::http::message<isRequest, Body, Fields> msg_;
                work_impl(http_session& self, boost::beast::http::message<isRequest, Body, Fields>&& msg) : self_(self), msg_(std::move(msg)) {}
                void operator()() {
                    boost::beast::http::async_write(self_.derived().stream(), msg_, boost::beast::bind_front_handler(&http_session::on_write,
                                                    self_.derived().shared_from_this(), msg_.need_eof()));
                }
            };

            // Allocate and store the work
            items_.push_back(boost::make_unique<work_impl>(self_, std::move(msg)));

            // If there was no previous work, start this one
            if (items_.size() == 1)
                (*items_.front())();
        }
    };

 public:
    http_session(flat_buffer_type buffer, limit_handle_type limit_handle, timeout_handle_type timeout_handle, request_handle_type request_handle):
        queue_(*this), buffer_(std::move(buffer)), limit_handle_(limit_handle), timeout_handle_(timeout_handle), request_handle_(request_handle),
        INSTANCE_LOG_IMPL {}
    ~http_session(void) {}

 public:
    derived_type& derived(void) { return static_cast<derived_type&>(*this);}

 protected:
    void do_read(void) {
        // Construct a new parser for each message
        parser_.emplace();

        // Apply a reasonable limit to the allowed size
        // of the body in bytes to prevent abuse.
        parser_->body_limit(limit_handle_(object_pointer_from(this)));

        // Set the timeout.
        boost::beast::get_lowest_layer(derived().stream()).expires_after(std::chrono::seconds(timeout_handle_(shared_from_this())));

        // Read a request using the parser-oriented interface
        boost::beast::http::async_read(derived().stream(), buffer_, *parser_, boost::beast::bind_front_handler(&http_session::on_read,
                                       derived().shared_from_this()));
    }

    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if (ec == boost::beast::http::error::end_of_stream)
            return derived().do_eof();

        if (ec)
            return handle_error(ec, "http_session.read");

        // See if it is a WebSocket Upgrade
        if (boost::beast::websocket::is_upgrade(parser_->get())) {
            // Disable the timeout.
            // The websocket::stream uses its own timeout settings.
            boost::beast::get_lowest_layer(derived().stream()).expires_never();

            // Create a websocket session, transferring ownership
            // of both the socket and the HTTP request.
            return make_websocket_session(derived().release_stream(), parser_->release());
        }

        // Send the response
        {
            static const char* kCrLF = "\r\n";
            static const char* kDblCrLF = "\r\n\r\n";
            enum{prpDblCrLfSize = 4};

            auto req = parser_->release();
            std::stringstream ss;
            ss << req;
            std::string request_content = ss.str();
            std::string body_content = req.body();
            request_content = request_content.substr(0, request_content.find(kDblCrLF) + prpDblCrLfSize);
            request_handle_(shared_from_this(), request_content.c_str(), body_content.c_str(), static_cast<unsigned int>(body_content.size()),
                std::bind(&this_type::response_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        }

        // If we aren't at the queue limit, try to pipeline another request
        if (!queue_.is_full())
            do_read();
    }

    void on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return handle_error(ec, "http_session.write");

        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return derived().do_eof();
        }

        // Inform the queue that a write completed
        if (queue_.on_write()) {
            // Read another request
            do_read();
        }
    }

    void response_cb(uintptr_t server_data, const char* response_content, unsigned int response_size) {
        LOG(VERBOSE) << "http_session::response_cb(" << boost::lexical_cast<std::string>(std::this_thread::get_id()) << ") called.";

        boost::beast::error_code ec;
        boost::beast::http::response_parser<boost::beast::http::string_body> p;
        p.eager(true);
        p.put(boost::asio::buffer(std::string(response_content, response_content + response_size)), ec);
        (queue_)(std::move(p.release()));
    }

 private:
    queue                                       queue_;
    // The parser is stored in an optional container so we can
    // construct it from scratch it at the beginning of each new message.
    parser_type                                 parser_;
    limit_handle_type                           limit_handle_;
    timeout_handle_type                         timeout_handle_;
    request_handle_type                         request_handle_;
    INSTANCE_LOG_DECLARE;

 protected:
    flat_buffer_type                            buffer_;
};

#endif  // NET_HTTP_SESSION_HPP_
