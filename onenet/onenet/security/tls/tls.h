/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file tls.h
 * @brief TCP/IP protocol process with TLS
 */

#ifndef __TLS_H__
#define __TLS_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definitionï¼ˆConstant and Macro )                                 */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief Create TLS Secure connection.
 *
 * @param host TLS Connection destination address
 * @param port TLS Connect Target Port
 * @param ca_cert Safety Certificate
 * @param ca_cert_len Certificate length
 * @param timeout Timeout to create connection
 * @retval -1 - Operation failed
 * @retval Other - Network connection handle
 */
handle_t tls_connect(const uint8_t *host, uint16_t port, const uint8_t *ca_cert, uint16_t ca_cert_len, uint32_t timeout);

/**
 * @brief Send data with TLS Secure connection
 *
 * @param handle TLS Connection operation handle
 * @param buf Data buffer address to be sent
 * @param len Length of data to be sent
 * @param timeout Data transmission timeout
 * @retval -1 - Error
 * @retval  0 - Timeout
 * @retval Other - Length of data actually sent successfully
 */
int32_t tls_send(handle_t handle, void *buf, uint32_t len, uint32_t timeout);
/**
 * @brief Receive data with TLS Secure connection
 *
 * @param handle TLS Connection operation handle
 * @param buf Buffer address used to receive data
 * @param len Length of data to be received
 * @param timeout Data reception timeout
 * @retval -1 - Error
 * @retval  0 - Timeout
 * @retval Other - Actual data length received
 */
int32_t tls_recv(handle_t handle, void *buf, uint32_t len, uint32_t timeout);

/**
 * @brief Close assignmentTLSSecure connection.
 *
 * @param handle TLS Connection operation handle Need to be closed 
 * @retval 0 - Succeed
 */
int32_t tls_disconnect(handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
