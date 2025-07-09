/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file plat_udp.h
 * @brief UDP Interfaces.
 */

#ifndef __PLAT_UDP_H__
#define __PLAT_UDP_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition (Constant and Macro)                                  */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief Establish UDP Connection 
 *
 * @param host Network data destination address
 * @param port Network data destination port
 * @retval -1 - Operation error
 * @retval Other - Network operation handle
 */
handle_t plat_udp_connect(const uint8_t *host, uint16_t port);

/**
 * @brief Send UDP Data.
 *
 * @param handle Network operation handle
 * @param buf Data buffer address to be sent
 * @param len Length of data to be sent
 * @param timeout_ms Data transmission timeout
 * @retval -1 - Error
 * @retval  0 - Timeout
 * @retval Other - Length of data successfully sent
 */
int32_t plat_udp_send(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms);

/**
 * @brief Receive UDP data
 *
 * @param handle Network operation handle
 * @param buf Buffer address used to receive data
 * @param len Length of data to be received
 * @param timeout_ms Operation timeout
 * @retval -1 - Error
 * @retval  0 - Timeout
 * @retval Other - Actual data length received
 */
int32_t plat_udp_recv(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms);

/**
 * @brief Establish UDP socket
 *
 * @param local_host Specify the host address to be used locally，Can be empty
 * @param local_port Specify the port used locally
 * @retval -1 - Error
 * @retval Other - Network operation handle
 */
handle_t plat_udp_bind(const char *local_host, uint16_t local_port);

/**
 * @brief Send UDP Data for the specified address
 *
 * @param handle Network operation handle
 * @param buf Data buffer address to be sent
 * @param len Length of data to be sent
 * @param host Specify the destination host address
 * @param port Specify the destination port
 * @param timeout_ms Data transmission timeout
 * @retval -1 - Error
 * @retval  0 - Timeout
 * @retval Other - Length of data actually sent
 */
int32_t plat_udp_sendto(handle_t handle, void *buf, uint32_t len, const char *host, uint16_t port, uint32_t timeout_ms);

/**
 * @brief Receive UDP Data for the specified address
 *
 * @param handle Network operation handle
 * @param buf Buffer address used to receive data
 * @param len Length of data to be received
 * @param host Specify the source address for receiving data
 * @param port Specify the source port to receive data
 * @param timeout_ms Operation timeout
 * @retval -1 - Error
 * @retval  0 - Timeout
 * @retval Other -  Actual data length received
 */
int32_t plat_udp_recvfrom(handle_t handle, void *buf, uint32_t len, const char *host, uint16_t port, uint32_t timeout_ms);

/**
 * @brief Close the specified connection
 *
 * @param handle Network operation handle
 * @return int32_t
 */
int32_t plat_udp_disconnect(handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
