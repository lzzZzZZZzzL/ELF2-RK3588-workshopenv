/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file data_types.h
 * @brief Data type definition.
 */

#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/
#ifndef PLAT_HAVE_STDINT

#define TRUE  1
#define FALSE 0

typedef unsigned char boolean;

typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef signed short       int16_t;
typedef unsigned short     uint16_t;
typedef signed int         int32_t;
typedef unsigned int       uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;
#else
#include <stdbool.h>
#include <stdint.h>

#define TRUE  true
#define FALSE false

typedef bool      boolean;
#endif

typedef float  float32_t;
typedef double float64_t;

#ifndef CONFIG_PLAT_ARCH_64BIT
typedef int handle_t;
typedef int ptr_t;
#else
typedef long long handle_t;
typedef long long ptr_t;
#endif

#ifndef NULL
#define NULL 0
#endif

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
struct buffer_t
{
    uint8_t *buf;
    uint32_t len;
};

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
