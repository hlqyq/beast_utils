// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_NET_UTILS_H_
#define NET_NET_UTILS_H_

#include <boost/beast/core.hpp>

void handle_error(boost::beast::error_code ec, char const* what);

#endif  // NET_NET_UTILS_H_
