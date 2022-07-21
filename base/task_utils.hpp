// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_UTILS_HPP_
#define BASE_TASK_UTILS_HPP_

#include <memory>
#include <utility>
#include <boost/asio.hpp>

//////////////////////////////////////// declarations ////////////////////////////////////////

extern boost::asio::io_context* get_io_context(void);;

template<typename HandlerType> void post_task(HandlerType handler, unsigned int delay_milliseconds = 0);

//////////////////////////////////////// implements ////////////////////////////////////////

template<typename ExecutorType, typename HandlerType>
inline void post_task_impl(ExecutorType executor, HandlerType handler, unsigned int delay_milliseconds) {
    if (delay_milliseconds > 0) {
        auto sp_timer = std::make_shared<boost::asio::steady_timer>(executor, boost::asio::chrono::milliseconds(delay_milliseconds));
        sp_timer->async_wait([executor, handler, sp_timer](const boost::system::error_code& /*ec*/) {
            boost::asio::post(executor, std::move(handler));
        });
    } else {
        boost::asio::post(executor, std::move(handler));
    }
}

template<typename HandlerType>
inline void post_task(HandlerType handler, unsigned int delay_milliseconds) {
    post_task_impl(boost::asio::make_strand(*(get_io_context())), std::move(handler), delay_milliseconds);
}

#endif  // BASE_TASK_UTILS_HPP_
