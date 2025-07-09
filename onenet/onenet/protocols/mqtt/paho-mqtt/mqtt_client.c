/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 *
 * @file mqtt_client.c
 * @brief based on PahoMQTT MQTT Client-C\src\MQTT Client.c
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "mqtt_client.h"
#include "data_types.h"
#include "log.h"
#include "mqtt_api.h"
#include "plat_osl.h"
#include "plat_tcp.h"
#include "plat_time.h"

#include <stdlib.h>
#include <string.h>
/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#define MAX_PACKET_ID 65535 /* according to the MQTT specification - do not change! */

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
typedef struct mqtt_client
{
    unsigned int   next_packetid;
    size_t         buf_size, readbuf_size;
    unsigned char *buf, *readbuf;
    unsigned int   keepAliveInterval;
    char           ping_outstanding;
    int            isconnected;
    int            cleansession;

    struct MessageHandlers
    {
        const char* topic_filter;
        void (*fp)(void*, const uint8_t*, struct mqtt_message_t*);
        void* arg;

    } message_handlers[MAX_MESSAGE_HANDLERS], defaultHandler; /* Message handlers are indexed by subscription topic */

    mqtt_network* ipstack;
    handle_t      keepalive_count;
} mqtt_client;

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
#if 0    // New message handler
static void NewMessageData(mqtt_message_data *md, MQTTString *aTopicName,
                           struct mqtt_message_t *aMessage)
{
    md->topicName = aTopicName;
    md->message = aMessage;
}
#endif

static int getNextPacketId(mqtt_client* c)
{
    return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}

static int sendPacket(mqtt_client* c, int length, handle_t cd_handle)
{
    int rc = FAILURE, sent = 0;

    do {
        rc = c->ipstack->mqttwrite(c->ipstack->handle, &c->buf[sent], length, countdown_left(cd_handle));

        if (rc < 0)    // there was an error writing the data
        {
            break;
        }

        sent += rc;
    } while (sent < length && !countdown_is_expired(cd_handle));

    if (sent == length) {
        rc = SUCCESS;
    } else {
        loge("Mqtt client send packet failed!");
        rc = FAILURE;
    }

    return rc;
}

void* mqtt_client_init(mqtt_network* network, unsigned char* sendbuf, size_t sendbuf_size, unsigned char* readbuf, size_t readbuf_size)
{
    int          i;
    mqtt_client* c = NULL;

    c = (mqtt_client*)osl_malloc(sizeof(*c));

    if (NULL == c) {
        return NULL;
    }

    osl_memset(c, 0, sizeof(*c));

    c->ipstack = network;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        c->message_handlers[i].topic_filter = 0;
    }

    c->buf              = sendbuf;
    c->buf_size         = sendbuf_size;
    c->readbuf          = readbuf;
    c->readbuf_size     = readbuf_size;
    c->isconnected      = 0;
    c->cleansession     = 0;
    c->ping_outstanding = 0;

    c->defaultHandler.fp = NULL;
    c->next_packetid     = 1;
    c->keepalive_count         = countdown_start(0);

    return c;
}

void mqtt_client_deinit(void* client)
{
    if (client) {
        countdown_stop(((mqtt_client*)client)->keepalive_count);
        osl_free(client);
    }
}

static int decodePacket(mqtt_client* c, int* value, int timeout)
{
    unsigned char i;
    int           multiplier                       = 1;
    int           len                              = 0;
    const int     k_max_no_of_remaining_length_bytes = 4;

    *value = 0;

    do {
        int rc = MQTTPACKET_READ_ERROR;

        if (++len > k_max_no_of_remaining_length_bytes) {
            rc = MQTTPACKET_READ_ERROR; /* bad data */
            goto exit;
        }

        rc = c->ipstack->mqttread(c->ipstack->handle, &i, 1, timeout);

        if (rc != 1) {
            goto exit;
        }

        *value += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);

exit:
    return len;
}

static int readPacket(mqtt_client* c, handle_t cd_handle)
{
    MQTTHeader header  = { 0 };
    int        len     = 0;
    int        rem_len = 0;

    /* 1. read the header byte.  This has the packet type in it */
    int rc = c->ipstack->mqttread(c->ipstack->handle, c->readbuf, 1, countdown_left(cd_handle));

    if (rc != 1) {
        goto exit;
    }

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, countdown_left(cd_handle));
    len += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    if (rem_len > (c->readbuf_size - len)) {
        rc = BUFFER_OVERFLOW;
        goto exit;
    }

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if (rem_len > 0 && (rc = c->ipstack->mqttread(c->ipstack->handle, c->readbuf + len, rem_len, countdown_left(cd_handle)) != rem_len)) {
        rc = 0;
        goto exit;
    }

    header.byte = c->readbuf[0];
    rc          = header.bits.type;

    if(c->keepAliveInterval>0) countdown_set(c->keepalive_count, c->keepAliveInterval * 1000);

exit:
    return rc;
}

// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
static char isTopicMatched(char* topic_filter, MQTTString* topicName)
{
    char* curf     = topic_filter;
    char* curn     = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;

    while (*curf && curn < curn_end) {
        if (*curn == '/' && *curf != '/') {
            break;
        }

        if (*curf != '+' && *curf != '#' && *curf != *curn) {
            break;
        }

        if (*curf == '+') {
            // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;

            while (nextpos < curn_end && *nextpos != '/') {
                nextpos = ++curn + 1;
            }
        } else if (*curf == '#') {
            curn = curn_end - 1;    // skip until end of string
        }

        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}

static int deliverMessage(mqtt_client* c, MQTTString* topicName, struct mqtt_message_t* message)
{
    int                  i;
    int                  rc  = FAILURE;
    mqtt_message_handler fp  = NULL;
    void*                arg = NULL;

    // we have to find the right message handler - indexed by topic
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        if (c->message_handlers[i].topic_filter != 0
            && (MQTTPacket_equals(topicName, (char*)c->message_handlers[i].topic_filter)
                || isTopicMatched((char*)c->message_handlers[i].topic_filter, topicName))) {
            if (c->message_handlers[i].fp != NULL) {

                fp  = c->message_handlers[i].fp;
                arg = c->message_handlers[i].arg;
                rc  = SUCCESS;
            }
        }
    }
    if (rc == FAILURE) {
        fp  = c->defaultHandler.fp;
        arg = c->defaultHandler.arg;
    }
    if (fp) {
        int   data_len = topicName->lenstring.len;
        char* data     = NULL;
        if (NULL != (data = malloc(data_len + 1))) {
            memset(data, 0, data_len + 1);
            memcpy(data, topicName->lenstring.data, data_len);
            fp(arg, (const uint8_t*)data, message);
            free(data);
        }
        rc = SUCCESS;
    }

    return rc;
}

static int keepalive(mqtt_client* c)
{
    int rc = SUCCESS;

    if (c->keepAliveInterval == 0) {
        goto exit;
    }

    /**
     * To guarantee Timeliness of Response，Current MQTT yield interval is set to 50ms，Works fine under normal circumstances.
     * In some cases req and respin intervals in heartbeat packet may be hundreds ms,Exceeding scheduled interval,making heartbeat determination to fail.
     * To solve the problem，it is considered exceeding the interval when the last heartbeat resp interval exceeding keepalive time. That is to say, once the server doesn't return heartbeat resp，The device will not determine whether the previous heartbeat was successful sent until the next heartbeat is to be sent.
     * If Received resp normally during this period of time，Then continue a normal heartbeat，Otherwise an error is reported，Prevent delayed resp making heartbeat fail
     */
    if (countdown_is_expired(c->keepalive_count)) {
        if (c->ping_outstanding) {
            rc = FAILURE; /* PINGRESP not received in keepalive interval */
        } else {
            handle_t countdown_hdl = countdown_start(2000);

            int len = MQTTSerialize_pingreq(c->buf, c->buf_size);

            if (len > 0 && (rc = sendPacket(c, len, countdown_hdl)) == SUCCESS)    // send the ping packet
            {
                c->ping_outstanding = 1;
                countdown_set(c->keepalive_count, c->keepAliveInterval * 1000);    // record the fact that we have successfully sent the packet

            }

            countdown_stop(countdown_hdl);
        }
    }

exit:
    return rc;
}

static void MQTTCleanSession(mqtt_client* c)
{
    int i = 0;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        c->message_handlers[i].topic_filter = NULL;
    }
}

static void MQTTCloseSession(mqtt_client* c)
{
    c->ping_outstanding = 0;
    c->isconnected      = 0;

    if (c->cleansession) {
        MQTTCleanSession(c);
    }
}

static int cycle(mqtt_client* c, handle_t cd_handle)
{
    int len = 0, rc = SUCCESS;

    int packet_type = readPacket(c, cd_handle); /* read the socket, see what work is due */

    switch (packet_type) {
        default:
            /* no more data to read, unrecoverable. Or read packet fails due to unexpected network error */
            rc = packet_type;
            goto exit;

        case 0: /* timed out reading packet */
            break;

        case CONNACK:
        case PUBACK:
        case SUBACK:
        case UNSUBACK:
            break;

        case PUBLISH: {
            MQTTString            topicName;
            struct mqtt_message_t msg;
            int                   int_qos;
            msg.payload_len = 0; /* this is a size_t, but deserialize publish sets this as int */

            if (MQTTDeserialize_publish(
                    &msg.dup, &int_qos, &msg.retained, &msg.id, &topicName, (unsigned char**)&msg.payload, (int*)&msg.payload_len, c->readbuf, c->readbuf_size)
                != 1) {
                goto exit;
            }

            msg.qos = (enum mqtt_qos_e)int_qos;
            deliverMessage(c, &topicName, &msg);

            if (msg.qos != MQTT_QOS0) {
                if (msg.qos == MQTT_QOS1) {
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
                } else if (msg.qos == MQTT_QOS2) {
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
                }

                if (len <= 0) {
                    rc = FAILURE;
                } else {
                    rc = sendPacket(c, len, cd_handle);
                }

                if (rc == FAILURE) {
                    goto exit;    // there was a problem
                }
            }

            break;
        }

        case PUBREC:
        case PUBREL: {
            unsigned short mypacketid;
            unsigned char  dup, type;

            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1) {
                rc = FAILURE;
            } else if ((len = MQTTSerialize_ack(c->buf, c->buf_size, (packet_type == PUBREC) ? PUBREL : PUBCOMP, 0, mypacketid)) <= 0) {
                rc = FAILURE;
            } else if ((rc = sendPacket(c, len, cd_handle))!= SUCCESS)    // send the PUBREL packet
            {
                rc = FAILURE;    // there was a problem
            }

            if (rc == FAILURE) {
                goto exit;    // there was a problem
            }

            break;
        }

        case PUBCOMP:
            break;

        case PINGRESP:
            logi("keep alive ok");
            c->ping_outstanding = 0;
            break;
    }

    if (keepalive(c) != SUCCESS) {
        // check only keepalive FAILURE status so that previous FAILURE status can be considered as FAULT
        loge("Mqtt keep alive time out!");
        rc = FAILURE;
    }

exit:

    if (rc == SUCCESS) {
        rc = packet_type;
    } else if (c->isconnected) {
        MQTTCloseSession(c);
    }

    return rc;
}

int32_t mqtt_client_yield(void* client, int32_t timeout_ms)
{
    mqtt_client* c            = (mqtt_client*)client;
    int          rc           = SUCCESS;
    handle_t     yield_cd_hdl = 0;

    yield_cd_hdl = countdown_start(timeout_ms);

    if (0 > cycle(c, yield_cd_hdl)) {
        rc = FAILURE;
    }

    countdown_stop(yield_cd_hdl);

    return rc;
}

static int waitfor(mqtt_client* c, int packet_type, handle_t cd_handle)
{
    int rc = FAILURE;

    do {
        if (countdown_is_expired(cd_handle)) {
            break;    // we timed out
        }

        rc = cycle(c, cd_handle);
    } while (rc != packet_type && rc >= 0);

    return rc;
}

int32_t mqtt_client_connect_with_results(void* client, MQTTPacket_connectData* options, mqtt_conn_ack_data* data, uint32_t timeout_ms)
{
    handle_t               connect_cd_hdl  = 0;
    int                    rc              = FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    mqtt_client*           c               = (mqtt_client*)client;
    int                    len             = 0;

    if (c->isconnected) /* don't send connect packet again if we are already connected */
    {
        goto exit;
    }

    connect_cd_hdl = countdown_start(timeout_ms);

    if (options == 0) {
        options = &default_options; /* set default options if none were supplied */
    }

    c->keepAliveInterval = options->keepAliveInterval;
    c->cleansession      = options->cleansession;

    if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0) {
        goto exit;
    }

    if ((rc = sendPacket(c, len, connect_cd_hdl)) != SUCCESS)    // send the connect packet
    {
        goto exit;    // there was a problem
    }

    // this will be a blocking call, wait for the connack
    if (waitfor(c, CONNACK, connect_cd_hdl) == CONNACK) {
        data->rc             = 0;
        data->sessionPresent = 0;

        if (MQTTDeserialize_connack(&data->sessionPresent, &data->rc, c->readbuf, c->readbuf_size) == 1) {
            rc = data->rc;
        } else {
            loge("Mqtt connect respond deserialize error!");
            rc = FAILURE;
        }
    } else {
        loge("Mqtt connect respond time out!");
        rc = FAILURE;
    }

exit:
    countdown_stop(connect_cd_hdl);

    if (rc == SUCCESS) {
        c->isconnected      = 1;
        c->ping_outstanding = 0;
        countdown_set(c->keepalive_count, c->keepAliveInterval * 1000);
    }

    return rc;
}

int32_t mqtt_client_connect(void* client, MQTTPacket_connectData* options, uint32_t timeout_ms)
{
    mqtt_conn_ack_data data;
    return mqtt_client_connect_with_results(client, options, &data, timeout_ms);
}

int32_t mqtt_client_set_message_handler(void* client, const char* topic_filter, message_handler message_handler, void* arg)
{
    mqtt_client* c  = (mqtt_client*)client;
    int          rc = FAILURE;
    int          i  = -1;

    /* first check for an existing matching slot */
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        if (c->message_handlers[i].topic_filter != NULL && strcmp(c->message_handlers[i].topic_filter, topic_filter) == 0) {
            if (message_handler == NULL) /* remove existing */
            {
                c->message_handlers[i].topic_filter = NULL;
                c->message_handlers[i].fp          = NULL;
                c->message_handlers[i].arg         = arg;
            }

            rc = SUCCESS; /* return i when adding new subscription */
            break;
        }
    }

    /* if no existing, look for empty slot (unless we are removing) */
    if (message_handler != NULL) {
        if (rc == FAILURE) {
            for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
                if (c->message_handlers[i].topic_filter == NULL) {
                    rc = SUCCESS;
                    break;
                }
            }
        }

        if (i < MAX_MESSAGE_HANDLERS) {
            c->message_handlers[i].topic_filter = topic_filter;
            c->message_handlers[i].fp          = message_handler;
            c->message_handlers[i].arg         = arg;
        }
    }

    return rc;
}

int32_t mqtt_client_subscribe_with_results(
    void*              client,
    const char*        topic_filter,
    enum mqtt_qos_e    qos,
    message_handler     message_handler,
    void*              arg,
    mqtt_sub_ack_data* data,
    uint32_t           timeout_ms)
{
    mqtt_client* c          = (mqtt_client*)client;
    int          rc         = FAILURE;
    handle_t     sub_cd_hdl = 0;
    int          len        = 0;
    MQTTString   topic      = MQTTString_initializer;
    topic.cstring           = (char*)topic_filter;

    if (!c->isconnected) {
        goto exit;
    }

    sub_cd_hdl = countdown_start(timeout_ms);

    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic, (int*)&qos);

    if (len <= 0) {
        goto exit;
    }

    if ((rc = sendPacket(c, len, sub_cd_hdl)) != SUCCESS)    // send the subscribe packet
    {
        goto exit;    // there was a problem
    }

    if (waitfor(c, SUBACK, sub_cd_hdl) == SUBACK)    // wait for suback
    {
        int            count = 0;
        unsigned short mypacketid;
        data->grantedQoS = MQTT_QOS0;

        if (MQTTDeserialize_suback(&mypacketid, 1, &count, (int*)&data->grantedQoS, c->readbuf, c->readbuf_size) == 1) {
            if (data->grantedQoS != 0x80) {
                rc = mqtt_client_set_message_handler(c, topic_filter, message_handler, arg);
            }
        }
    } else {
        loge("Mqtt subscribe respond time out!");
        rc = FAILURE;
    }

exit:
    countdown_stop(sub_cd_hdl);

    if (rc == FAILURE) {
        MQTTCloseSession(c);
    }

    return rc;
}

int32_t mqtt_client_subscribe(void* c, const char* topic_filter, enum mqtt_qos_e qos, message_handler message_handler, void* arg, uint32_t timeout_ms)
{
    mqtt_sub_ack_data data;
    return mqtt_client_subscribe_with_results(c, topic_filter, qos, message_handler, arg, &data, timeout_ms);
}

int32_t mqtt_client_unsubscribe(void* client, const char* topic_filter, uint32_t timeout_ms)
{
    mqtt_client* c            = (mqtt_client*)client;
    int          rc           = FAILURE;
    handle_t     unsub_cd_hdl = 0;
    MQTTString   topic        = MQTTString_initializer;
    topic.cstring             = (char*)topic_filter;
    int len                   = 0;

    if (!c->isconnected) {
        goto exit;
    }

    unsub_cd_hdl = countdown_start(timeout_ms);

    if ((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic)) <= 0) {
        goto exit;
    }

    if ((rc = sendPacket(
             c,
             len,
             unsub_cd_hdl))
        != SUCCESS)    // send the subscribe packet
    {
        goto exit;    // there was a problem
    }

    if (waitfor(c, UNSUBACK, unsub_cd_hdl) == UNSUBACK) {
        unsigned short mypacketid;    // should be the same as the packetid above

        if (MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) == 1) {
            /* remove the subscription message handler associated with this topic, if there is one */
            mqtt_client_set_message_handler(c, topic_filter, NULL, NULL);
        }
    } else {
        loge("Mqtt unsubscribe respond time out!");
        rc = FAILURE;
    }

exit:
    countdown_stop(unsub_cd_hdl);

    if (rc == FAILURE) {
        MQTTCloseSession(c);
    }

    return rc;
}

int32_t mqtt_client_publish(void* client, const char* topicName, struct mqtt_message_t* message, uint32_t timeout_ms)
{
    mqtt_client* c          = (mqtt_client*)client;
    int          rc         = FAILURE;
    handle_t     pub_cd_hdl = 0;
    MQTTString   topic      = MQTTString_initializer;
    topic.cstring           = (char*)topicName;
    int len                 = 0;

    if (!c->isconnected) {
        goto exit;
    }

    pub_cd_hdl = countdown_start(timeout_ms);

    if (message->qos == MQTT_QOS1 || message->qos == MQTT_QOS2) {
        message->id = getNextPacketId(c);
    }

    len = MQTTSerialize_publish(
        c->buf, c->buf_size, 0, message->qos, message->retained, message->id, topic, (unsigned char*)message->payload, message->payload_len);

    if (len <= 0) {
        goto exit;
    }

    if ((rc = sendPacket(
             c,
             len,
             pub_cd_hdl))
        != SUCCESS)    // send the subscribe packet
    {
        goto exit;    // there was a problem
    }

    if (message->qos == MQTT_QOS1) {
        if (waitfor(c, PUBACK, pub_cd_hdl) == PUBACK) {
            unsigned short mypacketid;
            unsigned char  dup, type;

            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1) {
                loge("Mqtt publish respond deserialize error!");
                rc = FAILURE;
            }
        } else {
            loge("Mqtt publish respond time out!");
            rc = FAILURE;
        }
    } else if (message->qos == MQTT_QOS2) {
        if (waitfor(c, PUBCOMP, pub_cd_hdl) == PUBCOMP) {
            unsigned short mypacketid;
            unsigned char  dup, type;

            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1) {
                loge("Mqtt publish respond deserialize error!");
                rc = FAILURE;
            }
        } else {
            loge("Mqtt publish respond time out!");
            rc = FAILURE;
        }
    }

exit:
    countdown_stop(pub_cd_hdl);

    if (rc == FAILURE) {
        //        MQTTCloseSession(c);
    }

    return rc;
}

int32_t mqtt_client_disconnect(void* client, uint32_t timeout_ms)
{
    mqtt_client* c             = (mqtt_client*)client;
    int          rc            = FAILURE;
    handle_t     discon_cd_hdl = 0;    // we might wait for incomplete incoming publishes to complete
    int          len           = 0;

    discon_cd_hdl = countdown_start(timeout_ms);

    len = MQTTSerialize_disconnect(c->buf, c->buf_size);

    if (len > 0) {
        rc = sendPacket(c, len, discon_cd_hdl);    // send the disconnect packet
    }

    countdown_stop(discon_cd_hdl);
    MQTTCloseSession(c);

    return rc;
}

int32_t mqtt_client_is_connected(void* client)
{
    return ((mqtt_client*)client)->isconnected;
}

/*****************************************************************************/
// 如果 CONFIG_NETWORK_TLS 被定义且值为 1
#if defined(CONFIG_NETWORK_TLS) && CONFIG_NETWORK_TLS == 1
#include "tls.h"
#else
#include "plat_tcp.h"
#endif

/**
 * @brief Connect to the server.
 *
 * @param client MQTT Client instance action handle
 * @return int32_t Return connection results（CONN_ACK or other errors）
 */
void* mqtt_connect(const uint8_t* remote_addr, uint16_t remote_port, const uint8_t* ca_cert, uint16_t ca_cert_len, struct mqtt_param_t* mqtt_param, uint32_t timeout_ms)
{
    struct mqtt_client*    client = NULL;
    struct mqtt_network*   net_cb = NULL;
    MQTTPacket_connectData conn_data;
    handle_t               cd_hdl = 0;

    if (NULL == (net_cb = osl_malloc(sizeof(struct mqtt_network)))) {
        return NULL;
    }
    osl_memset(net_cb, 0, sizeof(struct mqtt_network));

    if (0 == (cd_hdl = countdown_start(timeout_ms))) {
        osl_free(net_cb);
        return NULL;
    }

    #if defined(CONFIG_NETWORK_TLS) && CONFIG_NETWORK_TLS == 1
    net_cb->handle     = tls_connect(remote_addr, remote_port, ca_cert, ca_cert_len, countdown_left(cd_hdl));
    net_cb->mqttread   = tls_recv;
    net_cb->mqttwrite  = tls_send;
    net_cb->disconnect = tls_disconnect;
#else
    net_cb->handle     = plat_tcp_connect(remote_addr, remote_port, countdown_left(cd_hdl));
    net_cb->mqttread   = plat_tcp_recv;
    net_cb->mqttwrite  = plat_tcp_send;
    net_cb->disconnect = plat_tcp_disconnect;
#endif

    if (-1 == net_cb->handle) {
        goto exit;
    }

    client = mqtt_client_init(net_cb, mqtt_param->send_buf, mqtt_param->send_buf_len, mqtt_param->recv_buf, mqtt_param->recv_buf_len);

    if (NULL == client) {
        goto exit1;
    }

    osl_memset(&conn_data, 0, sizeof(conn_data));
    osl_memcpy(conn_data.struct_id, "MQTC", 4);
    conn_data.MQTTVersion       = 4;
    conn_data.keepAliveInterval = SDK_ACCESS_LIFE_TIME;
    conn_data.clientID.cstring  = (char*)mqtt_param->client_id;

    if (mqtt_param->connect_flag & MQTT_CONNECT_FLAG_USERNAME) {
        conn_data.username.cstring = (char*)mqtt_param->username;
    }

    if (mqtt_param->connect_flag & MQTT_CONNECT_FLAG_PASSWORD) {
        conn_data.password.cstring = (char*)mqtt_param->password;
    }

    if (mqtt_param->connect_flag & MQTT_CONNECT_FLAG_CLEAN_SESSION) {
        conn_data.cleansession = 1;
    }

    if (mqtt_param->connect_flag & MQTT_CONNECT_FLAG_WILL) {
        conn_data.willFlag = 1;
        osl_memcpy(conn_data.will.struct_id, "MQTW", 4);
        conn_data.will.topicName.cstring = (char*)mqtt_param->will_msg.will_topic;

        if (mqtt_param->connect_flag & MQTT_CONNECT_FLAG_WILL_RETAIN) {
            conn_data.will.retained = 1;
        }

        conn_data.will.message.lenstring.data = (char*)mqtt_param->will_msg.will_message;
        conn_data.will.message.lenstring.len  = mqtt_param->will_msg.will_message_len;
        conn_data.will.qos                    = mqtt_param->will_msg.qos;
    }
    int ret = 0;
    if (0 != (ret = mqtt_client_connect(client, &conn_data, countdown_left(cd_hdl)))) {
        logd("mqtt connect %d", ret);
        goto exit2;
    }

    logi("MQTT connect ok");

    countdown_stop(cd_hdl);
    return client;

exit2:
    mqtt_client_deinit(client);
exit1:
    net_cb->disconnect(net_cb->handle);
exit:
    countdown_stop(cd_hdl);
    osl_free(net_cb);
    return NULL;
}

/**
 * @brief MQTT Message processing.
 *
 * @param client MQTT Client instance action handle
 * @param timeout_ms Timeout time
 * @return int32_t
 */
int32_t mqtt_yield(void* client, uint32_t timeout_ms)
{
    if (client) {
        return mqtt_client_yield(client, timeout_ms);
    }

    return -2;
}

/**
 * @brief MQTT Message push.
 *
 * @param client MQTT Client instance action handle
 * @param topic Destination of push messages topic
 * @param message Message content that needs to be pushed
 * @return int32_t According to QOS Level Return PUBACK or other errors
 */
int32_t mqtt_publish(void* client, const uint8_t* topic, struct mqtt_message_t* message, uint32_t timeout_ms)
{
    if (client) {
        return mqtt_client_publish(client, (const char*)topic, message, timeout_ms);
    }

    return -1;
}

int32_t mqtt_set_default_message_handler(void* client, mqtt_message_handler msg_handler, void* arg)
{
    mqtt_client* c = (mqtt_client*)client;

    c->defaultHandler.fp  = msg_handler;
    c->defaultHandler.arg = arg;

    return SUCCESS;
}

/**
 * @brief Subscribe designated topic
 *
 * @param client MQTT Client instance action handle
 * @param topic Target subscription topic
 * @param qos target subscription topic QOS Grade
 * @param msg_handler Assign Message handling callbacks for topic
 * @return int32_t
 */
int32_t mqtt_subscribe(void* client, const uint8_t* topic, enum mqtt_qos_e qos, mqtt_message_handler msg_handler, void* arg, uint32_t timeout_ms)
{
    if (client) {
        return mqtt_client_subscribe(client, (const char*)topic, qos, msg_handler, arg, timeout_ms);
    }

    return -1;
}

/**
 * @brief Cancel topic Subscribe.
 *
 * @param client MQTT Client instance action handle
 * @param topic topic
 * @return int32_t
 */
int32_t mqtt_unsubscribe(void* client, const uint8_t* topic, uint32_t timeout_ms)
{
    if (client) {
        return mqtt_client_unsubscribe(client, (const char*)topic, timeout_ms);
    }

    return -1;
}

/**
 * @brief DisconnectMQTTConnect.
 *
 * @param client MQTT Client instance action handle
 * @return int32_t
 */
int32_t mqtt_disconnect(void* client, uint32_t timeout_ms)
{
    mqtt_client* c = (mqtt_client*)client;

    mqtt_client_disconnect(c, timeout_ms);
    if (c->ipstack) {
        c->ipstack->disconnect(c->ipstack->handle);
        osl_free(c->ipstack);
    }
    mqtt_client_deinit(c);

    return 0;
}
