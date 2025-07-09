/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file dev_token.h
 * @brief Generate token for mqtt and coap protocol
 */

#ifndef __DEV_TOKEN_H__
#define __DEV_TOKEN_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
enum sig_method_e
{
    SIG_METHOD_MD5,
    SIG_METHOD_SHA1,
    SIG_METHOD_SHA256,
    SIG_METHOD_SM2
};

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief Generate OneNET Login authentication Token
 *
 * @param token Used to store the address of generated authentication token Data buffer
 * @param method Assign token encryption Algorithm
 * @param exp_time UnixForm specification of timestamptokenTime of expiration
 * @param product_id Product ID to which the device belongs to
 * @param dev_name Device Unique Identification，When set to null, the product level key is used to calculated token
 * @param access_key Unique access key for the product to which the device belongs
 * @retval 0 - token build successfully
 */
int32_t dev_token_generate(uint8_t *token, enum sig_method_e method, uint32_t exp_time, const uint8_t *product_id,
                           const uint8_t *dev_name, const uint8_t *access_key);

#ifdef __cplusplus
}
#endif

#endif
