// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APP_RESOURCE_H_
#define SRC_APP_RESOURCE_H_

#include <memory>
#include <string>
#include <boost/asio/ssl/context.hpp>
#include "src/scaffold_handles.h"

class app_resource {
 public:
    typedef app_resource                                        this_type;
    typedef scaffold_handles                                    callback_handles_type;
    typedef boost::asio::io_context                             io_context_type;
    typedef boost::asio::ssl::context                           ssl_context_type;

 private:
    app_resource(void);
    ~app_resource(void);
    explicit app_resource(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

 public:
    static this_type*& get_singleton_instance(void);
    static void release_singleton_instance(void);
    callback_handles_type& scaffold_handles_get_instance(void) { return callback_handles_; }
    const callback_handles_type& scaffold_handles_get_instance(void) const { return callback_handles_; }
    io_context_type* get_io_context(void) const { return io_context_; }
    ssl_context_type* get_ssl_context(void) const { return ssl_context_; }

 public:
     bool init(std::string* result_error);
     int run_server(unsigned short port, bool ssl, int thread_count);
     void shutdown_server(void);

 private:
     callback_handles_type                                       callback_handles_;
     io_context_type*                                            io_context_;
     ssl_context_type*                                           ssl_context_;
};

app_resource* app_resource_get_instance(void);
boost::asio::ssl::context* get_ssl_context(void);
boost::asio::io_context* get_io_context(void);

#endif  // SRC_APP_RESOURCE_H_
