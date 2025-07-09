/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file dev_token.c
 * @brief Generate token for mqtt and coap protocol
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "dev_token.h"
#include "plat_osl.h"
#include "log.h"

#include <wolfssl/wolfcrypt/coding.h>
#include <wolfssl/wolfcrypt/hmac.h>

#ifdef USE_SIG_METHOD_SM2
#include "sm2ipk.h"
#endif

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#define DEV_TOKEN_LEN 256
#define DEV_TOKEN_VERISON_STR "2018-10-31"

#define DEV_TOKEN_SIG_METHOD_MD5 "md5"
#define DEV_TOKEN_SIG_METHOD_SHA1 "sha1"
#define DEV_TOKEN_SIG_METHOD_SHA256 "sha256"
#define DEV_TOKEN_SIG_METHOD_SM2 "sm2"
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
int32_t dev_token_generate(uint8_t* token, enum sig_method_e method, uint32_t exp_time, const uint8_t* product_id, const uint8_t* dev_name, const uint8_t* access_key)
{

    uint8_t  base64_data[128] = { 0 };
    uint8_t  str_for_sig[64] = { 0 };
    uint8_t  sign_buf[128]   = { 0 };
    uint32_t base64_data_len = sizeof(base64_data);
    uint8_t* sig_method_str  = NULL;
    uint32_t sign_len        = 0;
    uint32_t i               = 0;
    uint8_t* tmp             = NULL;
    Hmac     hmac;

    osl_sprintf(token, (const uint8_t*)"version=%s", DEV_TOKEN_VERISON_STR);

    if (dev_name) {
        osl_sprintf(token + osl_strlen(token), (const uint8_t*)"&res=products%%2F%s%%2Fdevices%%2F%s", product_id, dev_name);
    } else {
        osl_sprintf(token + osl_strlen(token), (const uint8_t*)"&res=products%%2F%s", product_id);
    }

    osl_sprintf(token + osl_strlen(token), (const uint8_t*)"&et=%u", exp_time);

    Base64_Decode(access_key, osl_strlen(access_key), base64_data, &base64_data_len);

    if (SIG_METHOD_MD5 == method) {
        wc_HmacSetKey(&hmac, MD5, base64_data, base64_data_len);
        sig_method_str = (uint8_t*)DEV_TOKEN_SIG_METHOD_MD5;
        sign_len       = 16;
    } else if (SIG_METHOD_SHA1 == method) {
        wc_HmacSetKey(&hmac, SHA, base64_data, base64_data_len);
        sig_method_str = (uint8_t*)DEV_TOKEN_SIG_METHOD_SHA1;
        sign_len       = 20;
    } else if (SIG_METHOD_SHA256 == method) {
        wc_HmacSetKey(&hmac, SHA256, base64_data, base64_data_len);
        sig_method_str = (uint8_t*)DEV_TOKEN_SIG_METHOD_SHA256;
        sign_len       = 32;
    } else if (SIG_METHOD_SM2 == method) {
        sig_method_str = (uint8_t*)DEV_TOKEN_SIG_METHOD_SM2;
        sign_len       = 64;
    }

    osl_sprintf(token + osl_strlen(token), (const uint8_t*)"&method=%s", sig_method_str);
    if (dev_name) {
        osl_sprintf(str_for_sig, (const uint8_t*)"%u\n%s\nproducts/%s/devices/%s\n%s", exp_time, sig_method_str, product_id, dev_name, DEV_TOKEN_VERISON_STR);
    } else {
        osl_sprintf(str_for_sig, (const uint8_t*)"%u\n%s\nproducts/%s\n%s", exp_time, sig_method_str, product_id, DEV_TOKEN_VERISON_STR);
    }

    if (SIG_METHOD_SM2 == method) {
        #ifdef USE_SIG_METHOD_SM2
        static void* sm2_handle=NULL;
        if(sm2_handle==NULL) {
            sm2ipk_init();
            sm2ipk_matrix_t matrix={0};
            uint8_t id=0;
            uint8_t R[128]={0};
            osl_memcpy(matrix.prik[0],base64_data,base64_data_len);
            sm2_handle=sm2ipk_open_handle(&matrix, &id, 1, matrix.prik[0], R);
        }
        _Bool sm2_ret;
        sm2ipk_sign_t sign_out;
        sm2ipk_signature(sm2_handle, str_for_sig, osl_strlen(str_for_sig), &sign_out);
        osl_memcpy(sign_buf,sign_out.r,32);
        osl_memcpy(sign_buf+32,sign_out.s,32);
        #endif
    }
    else {
        wc_HmacUpdate(&hmac, str_for_sig, osl_strlen(str_for_sig));
        wc_HmacFinal(&hmac, sign_buf);
    }

    osl_memset(base64_data, 0, sizeof(base64_data));
    base64_data_len = sizeof(base64_data);
    Base64_Encode_NoNl(sign_buf, sign_len, base64_data, &base64_data_len);

    osl_strcat(token, (const uint8_t*)"&sign=");
    tmp = token + osl_strlen(token);

    for (i = 0; i < base64_data_len; i++) {
        switch (base64_data[i]) {
            case '+':
                osl_strcat(tmp, (const uint8_t*)"%2B");
                tmp += 3;
                break;
            case ' ':
                osl_strcat(tmp, (const uint8_t*)"%20");
                tmp += 3;
                break;
            case '/':
                osl_strcat(tmp, (const uint8_t*)"%2F");
                tmp += 3;
                break;
            case '?':
                osl_strcat(tmp, (const uint8_t*)"%3F");
                tmp += 3;
                break;
            case '%':
                osl_strcat(tmp, (const uint8_t*)"%25");
                tmp += 3;
                break;
            case '#':
                osl_strcat(tmp, (const uint8_t*)"%23");
                tmp += 3;
                break;
            case '&':
                osl_strcat(tmp, (const uint8_t*)"%26");
                tmp += 3;
                break;
            case '=':
                osl_strcat(tmp, (const uint8_t*)"%3D");
                tmp += 3;
                break;
            default:
                *tmp = base64_data[i];
                tmp += 1;
                break;
        }
    }

    return 0;
}
