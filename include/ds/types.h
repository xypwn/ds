// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#ifndef __DS_TYPE_H__
#define __DS_TYPE_H__

/* ssize_t */
#ifdef _WIN32
#include <windows.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

#endif
