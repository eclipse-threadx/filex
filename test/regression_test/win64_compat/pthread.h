/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2025 Contributors to the Eclipse Foundation */
/* AI-generated with GitHub Copilot */

/* Minimal pthreads shim for FileX regression tests on Windows/x64. */

#ifndef WIN64_COMPAT_PTHREAD_H
#define WIN64_COMPAT_PTHREAD_H

#include <windows.h>

typedef HANDLE pthread_t;
typedef void   pthread_attr_t;

#define PTHREAD_CANCEL_ENABLE       0
#define PTHREAD_CANCEL_DISABLE      1
#define PTHREAD_CANCEL_DEFERRED     0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1

static inline int pthread_create(pthread_t *tid, const pthread_attr_t *attr,
                                  void *(*start_routine)(void *), void *arg)
{
    (void)attr;
    *tid = CreateThread(NULL, 0U, (LPTHREAD_START_ROUTINE)(void *)start_routine, arg, 0U, NULL);
    return (*tid == NULL) ? -1 : 0;
}

static inline int pthread_join(pthread_t tid, void **retval)
{
    (void)retval;
    WaitForSingleObject(tid, INFINITE);
    CloseHandle(tid);
    return 0;
}

static inline int pthread_cancel(pthread_t tid)
{
    TerminateThread(tid, 0U);
    return 0;
}

/* No-op: allows test_control_thread_entry to reach exit(test_control_failed_tests). */
static inline void pthread_exit(void *retval)
{
    (void)retval;
}

static inline int pthread_setcancelstate(int state, int *oldstate)
{
    (void)state;
    if (oldstate != NULL)
    {
        *oldstate = PTHREAD_CANCEL_ENABLE;
    }
    return 0;
}

static inline int pthread_setcanceltype(int type, int *oldtype)
{
    (void)type;
    if (oldtype != NULL)
    {
        *oldtype = PTHREAD_CANCEL_DEFERRED;
    }
    return 0;
}

#endif /* WIN64_COMPAT_PTHREAD_H */
