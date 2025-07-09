/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file plat_tcp.h
 * @brief TCP Interfaces.
 */

#ifndef __PLAT_TCP_H__
#define __PLAT_TCP_H__

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

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief EstablishTCP Connect
 *
 * @param host TCP Connection destination host address
 * @param port TCP Connection destination host port
 * @param timeout_ms Connection establishment timeout
 * @retval -1 - Failed
 * @retval Other - TCP Connection operation handle
 */
handle_t plat_tcp_connect(const uint8_t *host, uint16_t port, uint32_t timeout_ms);

/**
 * @brief send TCP data
 *
 * @param handle TCP Connection operation handle
 * @param buf Data buffer address to be sent
 * @param len Length of data to be sent
 * @param timeout_ms Data transmission timeout
 * @retval -1 - Error
 * @retval  0 - Timeout
 * @retval Other - Length of data actually sent successfully
 */
int32_t plat_tcp_send(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms);

/**
 * @brief receive TCP data
 *
 * @param handle TCP Connection operation handle
 * @param buf Buffer address used to receive data
 * @param len Length of data you want to receive
 * @param timeout_ms Timeout time
 * @retval -1 - Error
 * @retval  0 - Timeout
 * @retval Other - Length of data actually received successfully
 */
int32_t plat_tcp_recv(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms);

/**
 * @brief Disconnect assigned TCP Connection
 *
 * @param handle TCP Connection operation handle
 * @retval 0 - Succeed
 */
int32_t plat_tcp_disconnect(handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
