/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file        common.h
 * @brief       Generic macro definitions
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>

#include "err_def.h"
#include "log.h"

#ifndef AIOT_ASSERT
#define AIOT_ASSERT(x)                                                         \
  do {                                                                         \
    if (!(x)) {                                                                \
      loge("Assertion failed '%s'", #x);                                       \
      while (1) {                                                              \
      }                                                                        \
    }                                                                          \
  } while (0)
#endif

#ifndef CHECK_EXPR_GOTO
#define CHECK_EXPR_GOTO(expr, label, fmt...)                                   \
  do {                                                                         \
    if (expr) {                                                                \
      loge(fmt);                                                               \
      goto label;                                                              \
    }                                                                          \
  } while (0)
#endif

#ifndef SAFE_ALLOC
#define SAFE_ALLOC(ptr, n)                                                     \
  do {                                                                         \
    ptr = malloc(n);                                                           \
    if (!ptr) {                                                                \
      fprintf(stderr, "malloc failed!\n");                                     \
      exit(-1);                                                                \
    }                                                                          \
    memset(ptr, '\0', n);                                                      \
  } while (0)
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(ptr)                                                         \
  do {                                                                         \
    if (ptr) {                                                                 \
      free(ptr);                                                               \
      ptr = NULL;                                                              \
    }                                                                          \
  } while (0)
#endif


#ifndef UNUSED
#define UNUSED(X) (void)X /* To avoid gcc/g++ warnings */
#endif

#ifndef IN_RANGE
#define IN_RANGE(_start, n, _end) ((n >= _start && n <= _end) ? 1 : 0)
#endif


#endif