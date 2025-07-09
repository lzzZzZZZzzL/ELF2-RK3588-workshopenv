/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file        utils.h
 * @brief       Some common used data processing function.
 */

#ifndef __UTILS_H__
#define __UTILS_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "plat_uart.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*****************************************************************************/
    /* External Definition（Constant and Macro )                                 */
    /*****************************************************************************/
    #define _STRING(x) #x
    #define STRING(x) _STRING(x)

    /*****************************************************************************/
    /* External Structures, Enum and Typedefs                                    */
    /*****************************************************************************/
    struct int_pair_t {
        int arr[2];
    };

    /*****************************************************************************/
    /* External Variables and Functions                                          */
    /*****************************************************************************/
    uint16_t set_16bit_le(uint8_t* buf, uint16_t val);
    uint16_t get_16bit_le(uint8_t* buf, uint16_t* val);

    uint16_t set_16bit_be(uint8_t* buf, uint16_t val);
    uint16_t get_16bit_be(uint8_t* buf, uint16_t* val);

    uint16_t set_32bit_le(uint8_t* buf, uint32_t val);
    uint16_t get_32bit_le(uint8_t* buf, uint32_t* val);

    uint16_t set_32bit_be(uint8_t* buf, uint32_t val);
    uint16_t get_32bit_be(uint8_t* buf, uint32_t* val);

    void    str_to_hex(uint8_t* str_hex, uint8_t* str, uint16_t len);
    int32_t hex_to_str(uint8_t* str_hex, uint8_t* str, uint16_t len);
    int int_pair_get(struct int_pair_t* pair,int pair_len,int in,int out,int in_value,int* out_value);

#ifdef __cplusplus
}
#endif

#endif
