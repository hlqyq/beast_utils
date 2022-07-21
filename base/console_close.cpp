// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/console_close.h"
#include <memory>
#include <mutex>
#include <condition_variable>

class console_close : public std::enable_shared_from_this<console_close> {
 public:
    typedef console_close                               this_type;
    typedef std::function<void(void)>                   handle_type;
    typedef std::mutex                                  mutex_type;
    typedef std::condition_variable                     condition_variable_type;
    typedef std::unique_lock<mutex_type>                lock_type;

 public:
    explicit console_close(handle_type handle);
    ~console_close(void);
    explicit console_close(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

 public:
    void handle_console_close(void);

 private:
    mutex_type                          mutex_;
    condition_variable_type             cv_;
    handle_type                         handle_;
    bool                                exited_;
};

console_close::console_close(handle_type handle) : handle_(handle), exited_(false) {
}

console_close::~console_close(void) {
    // This code will be called in the main thread. So it should notify the console thread to exit
    {
        lock_type lock(mutex_);
        exited_ = true;
    }
    cv_.notify_all();
}

// This handler will be called in the console thread. So it should wait for the main thread to exit
void console_close::handle_console_close(void) {
    if (handle_)
        handle_();

    lock_type lock(mutex_);
    cv_.wait(lock, [this]() { return exited_; });
}

void set_console_close_handle(void (*os_set_handle)(std::function<void(void)>), std::function<void(void)> handle_cb) {
    auto sp_console_close = std::make_shared<console_close>(handle_cb);
    os_set_handle([sp_console_close, os_set_handle]() {
        sp_console_close->handle_console_close();
    });
}
