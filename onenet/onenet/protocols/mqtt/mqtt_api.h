/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file mqtt_api.h
 * @brief MQTT Protocol interface encapsulation.
 */

#ifndef __MQTT_API_H__
#define __MQTT_API_H__

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
/**
 * @brief MQTT QOS Grade.
 *
 */
enum mqtt_qos_e
{
    MQTT_QOS0 = 0, /**<QOS0，Send at most once */
    MQTT_QOS1,     /**<QOS1，Send at least once */
    MQTT_QOS2,     /**<QOS2，Only received once */
    MQTT_QOS_DUMMY = 0x7FFFFFFF
};

/**
 * @brief MQTTLogin flag.
 *
 */
enum mqtt_connect_flag_e
{
    /** Clear session state after connection is disconnected*/
    MQTT_CONNECT_FLAG_CLEAN_SESSION = 0x01,
    /** Wills messages are set when connecting*/
    MQTT_CONNECT_FLAG_WILL          = 0x02,
    /** Set whether a will message is a reserved message，Valid when MQTT_CONNECT_FLAG_WILL is set*/
    MQTT_CONNECT_FLAG_WILL_RETAIN   = 0x04,
    /** Provide username information when connecting. When the flag is not set，MQTT_CONNECT_FLAG_PASSWORD is Invalid*/
    MQTT_CONNECT_FLAG_USERNAME      = 0x08,
    /** Provide user password when connecting，Valid when MQTT_CONNECT_FLAG_USERNAME is set*/
    MQTT_CONNECT_FLAG_PASSWORD      = 0x10
};

/**
 * @brief MQTT will message.
 *
 */
struct mqtt_will_message_t
{
    /** will message topic*/
    const uint8_t * will_topic;
    /** Will message content*/
    uint8_t *       will_message;
    /** Will message length*/
    uint32_t        will_message_len;
    /** will message QOS Grade*/
    enum mqtt_qos_e qos;
};

/**
 * @brief MQTT Parameter definition.
 *
 */
struct mqtt_param_t
{
    /** clientID*/
    const uint8_t *client_id;
    /** username*/
    const uint8_t *username;
    /** password*/
    const uint8_t *password;

    /** Heartbeat retention time interval，Unit is second*/
    /** Login flag，Optional. Refer to mqtt_connect_flag_e，supportssetting multiple flags at the same time（OR operation）*/
    uint32_t                   connect_flag;
    /** Will message settings，Valid when connect_flag includes MQTT_CONNECT_FLAG_WILL*/
    struct mqtt_will_message_t will_msg;

    /** Data sending buffer*/
    uint8_t *send_buf;
    /** Data sending buffer length*/
    uint32_t send_buf_len;
    /** Data receiving buffer*/
    uint8_t *recv_buf;
    /** Data receiving buffer length*/
    uint32_t recv_buf_len;
};

/**
 * @brief MQTT Message definition.
 *
 */
struct mqtt_message_t
{
    /** Message QOS Grade*/
    enum mqtt_qos_e qos;
    /** Message Content*/
    uint8_t *       payload;
    /** Message content length*/
    uint32_t        payload_len;
    /** Message retransmission flag*/
    uint8_t         dup;
    /** Whether the message is reserved for the server*/
    uint8_t         retained;
    /** Message ID*/
    uint16_t        id;
};

/**
 * @brief MQTT Delivering message callbacks
 *
 */
typedef void (*mqtt_message_handler)(void * /*arg*/, const uint8_t * /*topic*/, struct mqtt_message_t * /*message*/);

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief Log in MQTT Server
 *
 * @param remote_addr Server Address，Dot decimal form
 * @param remote_port Server Port
 * @param ca_cert CA Certificate，Null means using an unencrypted connection
 * @param ca_cert_len CA Certificate data length
 * @param mqtt_param MQTT Server login settings
 * @param timeout_ms Timeout time
 * @return void*
 */
void *mqtt_connect(const uint8_t *remote_addr, uint16_t remote_port, const uint8_t *ca_cert, uint16_t ca_cert_len,
                   struct mqtt_param_t *mqtt_param, uint32_t timeout_ms);

/**
 * @brief MQTT Message processing.
 *
 * @param client MQTT Client instance action handle
 * @param timeout Timeout time
 * @return int32_t
 */
int32_t mqtt_yield(void *client, uint32_t timeout_ms);

/**
 * @brief MQTT Message push.
 *
 * @param client MQTT Client instance action handle
 * @param topic Destination of push messages topic
 * @param message Message content that needs to be pushed
 * @return int32_t Return PUBACK or other errors According to QOS Level
 */
int32_t mqtt_publish(void *client, const uint8_t *topic, struct mqtt_message_t *message, uint32_t timeout_ms);

int32_t mqtt_set_default_message_handler(void *client, mqtt_message_handler msg_handler, void *arg);

/**
 * @brief Subscribe designated topic
 *
 * @param client MQTT Client instance action handle
 * @param topic Targets subscription topic
 * @param qos Specify the target subscription topic QOS Grade
 * @param msg_handler Assign Message handling callbacks for topic
 * @param arg Set message processing callback parameters
 * @return int32_t
 */
int32_t mqtt_subscribe(void *client, const uint8_t *topic, enum mqtt_qos_e qos, mqtt_message_handler msg_handler,
                       void *arg, uint32_t timeout_ms);

/**
 * @brief Cancel topic Subscribe.
 *
 * @param client MQTT Client instance action handle
 * @param topic Need to unsubscribetopic
 * @return int32_t
 */
int32_t mqtt_unsubscribe(void *client, const uint8_t *topic, uint32_t timeout_ms);

/**
 * @brief Disconnect MQTT Connection
 *
 * @param client MQTT Client instance action handle
 * @return int32_t
 */
int32_t mqtt_disconnect(void *client, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
