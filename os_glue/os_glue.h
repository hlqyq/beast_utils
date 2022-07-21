// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OS_GLUE_OS_GLUE_H_
#define OS_GLUE_OS_GLUE_H_

#include <functional>

void os_set_console_close_handle(std::function<void(void)> close_cb);

#endif  // OS_GLUE_OS_GLUE_H_
