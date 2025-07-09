/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file        utils.c
 * @brief       Some common used data processing function.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "plat_osl.h"
#include "log.h"
#include "utils.h"
#include "err_def.h"
/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
uint16_t set_16bit_le(uint8_t* buf, uint16_t val)
{
    buf[0] = val & 0xFF;
    buf[1] = (val >> 8) & 0xFF;
    return 2;
}

uint16_t get_16bit_le(uint8_t* buf, uint16_t* val)
{
    *val = (buf[1] << 8) | buf[0];
    return 2;
}

uint16_t set_16bit_be(uint8_t* buf, uint16_t val)
{
    buf[0] = (val >> 8) & 0xFF;
    buf[1] = val & 0xFF;
    return 2;
}

uint16_t get_16bit_be(uint8_t* buf, uint16_t* val)
{
    *val = (buf[0] << 8) | buf[1];
    return 2;
}

uint16_t set_32bit_le(uint8_t* buf, uint32_t val)
{
    buf[0] = val & 0xFF;
    buf[1] = (val >> 8) & 0xFF;
    buf[2] = (val >> 16) & 0xFF;
    buf[3] = (val >> 24) & 0xFF;
    return 4;
}

uint16_t get_32bit_le(uint8_t* buf, uint32_t* val)
{
    *val = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
    return 4;
}

uint16_t set_32bit_be(uint8_t* buf, uint32_t val)
{
    buf[0] = (val >> 24) & 0xFF;
    buf[1] = (val >> 16) & 0xFF;
    buf[2] = (val >> 8) & 0xFF;
    buf[3] = val & 0xFF;
    return 4;
}

uint16_t get_32bit_be(uint8_t* buf, uint32_t* val)
{
    *val = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    return 4;
}

// Refer to example_hex_to_str
int32_t hex_to_str(uint8_t* str_hex, uint8_t* str, uint16_t len)
{
    uint16_t i, j, k;
    uint8_t  data_buf[2] = { 0 };

    for (i = 0, j = 0; i < len; i++) {
        for (k = 0; k < 2; k++) {
            if ((str_hex[j] >= '0') && (str_hex[j] <= '9'))
                data_buf[k] = str_hex[j] - '0';
            else if ((str_hex[j] >= 'a') && (str_hex[j] <= 'f'))
                data_buf[k] = str_hex[j] - 'a' + 10;
            else if ((str_hex[j] >= 'A') && (str_hex[j] <= 'F'))
                data_buf[k] = str_hex[j] - 'A' + 10;
            else
                return -1;
            j++;
        }
        str[i] = ((data_buf[0] << 4) | data_buf[1]);
    }

    return 0;
}

int example_hex_to_str() {
    char str_hex[]="4141";
    char str[64]={0};
    hex_to_str(str_hex,str,strlen(str_hex));

    // exit(0);
    return 0;
}

void str_to_hex(uint8_t* str_hex, uint8_t* str, uint16_t len)
{
    char ddl, ddh;
    int  i;

    for (i = 0; i < len; i++) {
        ddh = 48 + str[i] / 16;
        ddl = 48 + str[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        str_hex[i * 2]     = ddh;
        str_hex[i * 2 + 1] = ddl;
    }
    str_hex[len * 2] = '\0';
}


int int_pair_get(struct int_pair_t* pair,int pair_len,int in,int out,int in_value,int* out_value) {
    uint32_t i = 0;
    for (i = 0; i < pair_len; i++) {
        if (in_value == pair[i].arr[in]) {
            *out_value=pair[i].arr[out];
            return ERR_OK;
        }
    }
    return ERR_INVALID_DATA;
}


