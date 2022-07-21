// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_SSL_CERTIFICATE_H_
#define SRC_SSL_CERTIFICATE_H_

#include <boost/asio/ssl/context.hpp>

void load_server_certificate(boost::asio::ssl::context* ctx);

#endif  // SRC_SSL_CERTIFICATE_H_
