/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2025 Contributors to the Eclipse Foundation */
/* AI-generated with GitHub Copilot */

/* Minimal POSIX unistd shim for FileX regression tests on Windows/x64. */

#ifndef WIN64_COMPAT_UNISTD_H
#define WIN64_COMPAT_UNISTD_H

#include <windows.h>

typedef unsigned int useconds_t;

/* Sleep for `usec` microseconds. Windows resolution is ~1 ms; round up. */
static inline int usleep(useconds_t usec)
{
    DWORD ms = (DWORD)((usec + 999U) / 1000U);
    Sleep((ms > 0U) ? ms : 1U);
    return 0;
}

#endif /* WIN64_COMPAT_UNISTD_H */
