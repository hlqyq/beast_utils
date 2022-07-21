// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "os_glue/os_glue.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static std::function<void(void)> kConsoleCloseHandle;

BOOL WINAPI MyCtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
    case CTRL_C_EVENT:  // Handle the CTRL-C signal.
        return TRUE;

    case CTRL_CLOSE_EVENT:  // CTRL-CLOSE: confirm that the user wants to exit.
        if (kConsoleCloseHandle)
            kConsoleCloseHandle();
        return TRUE;

    default:
        return FALSE;
    }
}

void os_set_console_close_handle(std::function<void(void)> close_cb) {
    kConsoleCloseHandle = close_cb;
    if (close_cb)
        SetConsoleCtrlHandler(MyCtrlHandler, TRUE);
    else
        SetConsoleCtrlHandler(NULL, FALSE);
}

BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
