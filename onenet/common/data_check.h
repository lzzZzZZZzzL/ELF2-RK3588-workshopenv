/**
 * Copyright (c), 2012~2021 iot.10086.cn All Rights Reserved
 *
 * @file data_check.h
 * @brief caculate checksum
 */

#ifndef __DATA_CHECK_H__
#define __DATA_CHECK_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition（Constant and Macro )                                 */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
uint16_t check_get_crc16(uint8_t *buf, uint32_t buf_len);
uint32_t check_get_crc32(uint8_t *buf, uint32_t buf_len);

#ifdef __cplusplus
}
#endif

#endif
