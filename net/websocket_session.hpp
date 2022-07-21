// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_WEBSOCKET_SESSION_HPP_
#define NET_WEBSOCKET_SESSION_HPP_

#include <memory>
#include <utility>
#include <string>
#include <boost/beast/websocket.hpp>
#include "base/memory_utils.hpp"
#include "base/utils.h"
#include "net/net_utils.h"

template<class Derived>
class websocket_session : virtual public virtual_enable_shared_from_this_base {
 public:
    typedef Derived                                             derived_type;
    typedef websocket_session<derived_type>                     this_type;
    typedef boost::beast::flat_buffer                           flat_buffer_type;
    typedef std::function<void(std::shared_ptr<virtual_enable_shared_from_this_base>)>                  open_handle_type;
    typedef std::function<void(virtual_enable_shared_from_this_base*)>                                  close_handle_type;
    typedef std::function<void(std::shared_ptr<virtual_enable_shared_from_this_base>, const char*)>     message_handle_type;

 public:
    websocket_session(open_handle_type open_handle, close_handle_type close_handle, message_handle_type message_handle) : open_handle_(open_handle),
                      close_handle_(close_handle), message_handle_(message_handle), INSTANCE_LOG_IMPL {}
    ~websocket_session(void) {
        virtual_enable_shared_from_this_base* pThis = this;
        if (close_handle_)
            close_handle_(pThis);
    }

 public:
    template<class Body, class Allocator>
    void run(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req) {
        do_accept(std::move(req));
    }

    void send(const char* message) {
        derived().ws().text(true);
        std::shared_ptr<std::string> sp_message = std::make_shared<std::string>(message);
        derived().ws().async_write(boost::asio::buffer(*sp_message), boost::beast::bind_front_handler(&websocket_session::on_write,
                                   derived().shared_from_this(), sp_message));
    }

 private:
    derived_type& derived(void) { return static_cast<derived_type&>(*this);}
    template<class Body, class Allocator>
    void do_accept(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req) {
        // Set suggested timeout settings for the websocket
        derived().ws().set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        derived().ws().set_option(boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::response_type& res) {
            res.set(boost::beast::http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " advanced-server-flex");
        }));

        // Accept the websocket handshake
        derived().ws().async_accept(req, boost::beast::bind_front_handler(&websocket_session::on_accept, derived().shared_from_this()));
    }

    void on_accept(boost::beast::error_code ec) {
        if (ec)
            return handle_error(ec, "websocket_session.on_accept");

        if (open_handle_)
            open_handle_(shared_from_this());

        // Read a message
        do_read();
    }

    void do_read(void) {
        // Read a message into our buffer
        derived().ws().async_read(buffer_, boost::beast::bind_front_handler(&websocket_session::on_read, derived().shared_from_this()));
    }

    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the websocket_session was closed
        if (ec == boost::beast::websocket::error::closed)
            return;

        if (ec) {
            handle_error(ec, "websocket_session.on_read");
        } else {
            const std::string message = boost::beast::buffers_to_string(buffer_.data());
            buffer_.consume(buffer_.size());  // Clear the buffer
            if (message_handle_)
                message_handle_(shared_from_this(), message.c_str());

            do_read();
        }
    }

    void on_write(std::shared_ptr<std::string> sp_message, boost::beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(sp_message);
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return handle_error(ec, "websocket_session.on_write");
    }

 protected:
    flat_buffer_type                buffer_;
    open_handle_type                open_handle_;
    close_handle_type               close_handle_;
    message_handle_type             message_handle_;
    INSTANCE_LOG_DECLARE;
};

#endif  // NET_WEBSOCKET_SESSION_HPP_
