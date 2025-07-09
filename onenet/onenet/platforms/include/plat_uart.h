/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file plat_uart.h
 * @brief UART Interfaces.
 */

#ifndef __PLAT_PORT_H__
#define __PLAT_PORT_H__

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
//Default log output serial port
#define UART_STDIO_FD 1

//Default module communication serial port，Used by default when communicating with WIFI modules and others
#define UART_MODULE_FD 2

enum port_data_bits_e
{
    PORT_DATABITS_5 = 5,
    PORT_DATABITS_6,
    PORT_DATABITS_7,
    PORT_DATABITS_8,
    PORT_DATABITS_9,
};

enum port_stop_bits_e
{
    PORT_STOPBITS_1 = 1,
    PORT_STOPBITS_2
};

enum port_flow_ctrl_e
{
    PORT_FLOWCTRL_NONE = 0,
    PORT_FLOWCTRL_RTS,
    PORT_FLOWCTRL_CTS,
    PORT_FLOWCTRL_CTS_RTS
};

enum port_parity_e
{
    PORT_PARITY_NONE = 0,
    PORT_PARITY_ODD,
    PORT_PARITY_EVEN
};

struct uart_config_t
{
    uint8_t *             port_name;
    uint32_t              baudrate;
    enum port_data_bits_e data_bits;
    enum port_stop_bits_e stop_bits;
    enum port_flow_ctrl_e flow_ctrl;
    enum port_parity_e    parity;
};

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief Open the serial port for module communication，And configure relevant parameters
 *
 * @param port_config Serial port configuration parameters
 * @return handle_t Communication serial port operation handle，Failure Return -1
 */
handle_t uart_open(struct uart_config_t *port_config);

/**
 * @brief Sending data through communication serial port
 *
 * @param uart Communication serial port operation handle
 * @param data Data buffer address to be sent
 * @param data_len Length of data to be sent
 * @param timeout_ms Data transmission timeout
 * @retval   <0 - Failed to send
 * @retval Other - Length of data actually sent
 */
int32_t uart_send(handle_t uart, uint8_t *data, uint32_t data_len, uint32_t timeout_ms);

/**
 * @brief Receive data through communication serial port
 *
 * @param uart Communication serial port operation handle
 * @param buf Buffer address used to receive data
 * @param buf_len Length of data to be received
 * @param timeout_ms Timeout to receive data
 * @retval   <0 - Receiving failed
 * @retval Other - Actual data length received
 */
int32_t uart_recv(handle_t uart, uint8_t *buf, uint32_t buf_len, uint32_t timeout_ms);

/**
 * @brief Close the communication serial port
 *
 * @param uart Communication serial port operation handle
 */
void uart_close(handle_t uart);

//Initialize printf Output and scanf Input，Redirect to serial port
int32_t uart_stdio_init();

//Platform-supported UART List, g_uart_list[0] Default is left blank
extern char* g_uart_list[];

//Platform-supported UART Quantity
extern int g_uart_num;

#ifdef __cplusplus
}
#endif

#endif
