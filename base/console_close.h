// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//
// Usage:
//
//  int main(void)
//  {
//      set_console_close_handle(os_set_console_close_handle, [](){ my_cleanup(); });
//        ...
//      return EXIT_SUCCESS;
//  }
//

#ifndef BASE_CONSOLE_CLOSE_H_
#define BASE_CONSOLE_CLOSE_H_

#include <functional>

void set_console_close_handle(void (*os_set_handle)(std::function<void(void)>), std::function<void(void)> handle_cb);

#endif  // BASE_CONSOLE_CLOSE_H_
