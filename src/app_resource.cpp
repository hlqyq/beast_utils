// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/app_resource.h"
#include <algorithm>
#include <vector>
#include <thread>
#include "base/utils.h"
#include "base/console_close.h"
#include "src/ssl_certificate.h"
#include "os_glue/os_glue.h"
#include "net/listener.h"
#include "base/task_utils.hpp"

app_resource::app_resource(void) : io_context_(nullptr), ssl_context_(nullptr) {
}

app_resource::~app_resource(void) {
}

app_resource*& app_resource::get_singleton_instance(void) {
    static this_type* k_instance = 0;
    if (!k_instance)
        k_instance = new this_type();
    return k_instance;
}

void app_resource::release_singleton_instance(void) {
    this_type*& pInstance = get_singleton_instance();
    delete pInstance;
    pInstance = 0;
}

bool app_resource::init(std::string* result_error) {
    return true;
}

int app_resource::run_server(unsigned short port, bool ssl, int thread_count) {
    ON_SCOPE_EXIT(release_singleton_instance(););
    {
        auto scope_exit = make_scope_guard([this]() {
            auto handler_pair = scaffold_handles_get_instance().server_shutdown_handler_pair;
            if (handler_pair.first)
                handler_pair.first(handler_pair.second);
        });

        // The io_context is required for all I/O
        thread_count = std::max<int>(1, thread_count);
        boost::asio::io_context ioc{ thread_count };
        io_context_ = &ioc;
        scope_exit += [this]() { io_context_ = nullptr; };

        // The SSL context is required, and holds certificates
        ssl_context_type ssl_context{ ssl_context_type::tlsv12 };
        if (ssl)
            load_server_certificate(&ssl_context);
        ssl_context_ = &ssl_context;
        scope_exit += [this]() { this->ssl_context_ = nullptr; };

        // Create and launch a listening port
        handle_listen(ioc, port);

        // Capture SIGINT and SIGTERM to perform a clean shutdown
# ifdef _WIN32
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM, SIGBREAK);
# else
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
# endif
        signals.async_wait([this, &ioc](boost::beast::error_code const&, int) { ioc.post([this]() { shutdown_server(); }); });
        auto close_handle = [this, &ioc]() { ioc.post([this]() { shutdown_server(); }); };
        set_console_close_handle(os_set_console_close_handle, close_handle);

        // Run the I / O service on the requested number of threads
        std::vector<std::thread> v;
        v.reserve(thread_count - 1);
        for (auto i = thread_count - 1; i > 0; --i)
            v.emplace_back([&ioc](){
                ioc.run();
            });
        ioc.run();

        // (If we get here, it means we got a SIGINT or SIGTERM)

        // Block until all the threads exit
        for (auto& t : v)
            t.join();
    }
    return EXIT_SUCCESS;
}

void app_resource::shutdown_server(void) {
    // Stop the `io_context`. This will cause `run()`
    // to return immediately, eventually destroying the
    // `io_context` and all of the sockets in it.
    if (io_context_ && !io_context_->stopped())
        io_context_->stop();
}

app_resource* app_resource_get_instance(void) {
    return app_resource::get_singleton_instance();
}

scaffold_handles* scaffold_handles_get_instance(void) {
    return &(app_resource_get_instance()->scaffold_handles_get_instance());
}

boost::asio::io_context* get_io_context(void) {
    return app_resource_get_instance()->get_io_context();
}

boost::asio::ssl::context* get_ssl_context(void) {
    return app_resource_get_instance()->get_ssl_context();
}
