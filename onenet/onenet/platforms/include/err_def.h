/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file err_def.h
 * @brief Error code definition.
 */

#ifndef __ERR_DEF_H__
#define __ERR_DEF_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition（Constant and Macro )                                 */
/*****************************************************************************/
/**
 * @name Operation Successful
 * @{
 */
/** Succeed*/
#define ERR_OK 0
/** @} */

/**
 * @name System error
 * @{
 */
/** Insufficient memory*/
#define ERR_ALLOC         -1
/** Interface parameter error*/
#define ERR_INVALID_PARAM  -2
/** Data error*/
#define ERR_INVALID_DATA   -3
/** Resource busy*/
#define ERR_RESOURCE_BUSY  -4
/** Device not initialized*/
#define ERR_UNINITIALIZED  -5
/** Device initialization not complete*/
#define ERR_INITIALIZING   -6
/** Repeat operation*/
#define ERR_REPETITIVE     -7
/** IO/Internal communication error*/
#define ERR_IO             -8
/** Feature not supported*/
#define ERR_NOT_SUPPORT    -9
/** System call error*/
#define ERR_SYSTEM_CONTROL -10
/** Network error*/
#define ERR_NETWORK        -11
/** Cloud error*/
#define ERR_CLOUD          -12
/** Timeout*/
#define ERR_TIMEOUT        -13
/** Request processing failed*/
#define ERR_REQUEST_FAILED -14
/** Data overflow*/
#define ERR_OVERFLOW       -15
/** Other errors*/
#define ERR_OTHERS         -16
/** @} */

#define ERR_SUCC 1
#define ERR_FAIL 0

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
#include "log.h"
#ifndef CHECK_EXPR_GOTO
#define CHECK_EXPR_GOTO(expr, label, fmt...) \
    do { \
        if (expr) { \
            loge(fmt); \
            goto label; \
        } \
    } while (0)
#endif
#ifdef __cplusplus
}
#endif

#endif
