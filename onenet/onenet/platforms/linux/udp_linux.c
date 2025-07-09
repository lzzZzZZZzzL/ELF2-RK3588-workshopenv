/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file udp_linux.c
 * @brief UDP API for linux
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "log.h"
#include "plat_osl.h"
#include "plat_time.h"
#include "plat_udp.h"
#include "slist.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
typedef struct udp_packet_list_node
{
    struct slist_node node;
    uint8_t*          buffer;
    uint32_t          length;
} udp_packet_list_node_t;

struct udp_handle_t
{
    handle_t           fd;
    struct sockaddr_in remote;
};

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/
static void* udp_recv_thread(void* socket_handle);

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
static int8_t             g_udp_close_flag  = 0;
static struct slist_head* g_udp_packet_list = NULL;

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
handle_t plat_udp_connect(const uint8_t* host, uint16_t port)
{
    struct udp_handle_t* net_handle = NULL;
    int32_t              flags      = -1;
    struct hostent*      ip_addr    = NULL;

    if (NULL == (ip_addr = gethostbyname((const char*)host))) {
        loge("Failed to resolve hostname: %s", host);
        return -1;
    }

    if (NULL == (net_handle = osl_calloc(1, sizeof(*net_handle)))) {
        loge("Memory allocation failed for UDP handle");
        return -1;
    }

    if (0 > (net_handle->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))) {
        loge("Socket creation failed: %s", strerror(errno));
        goto exit;
    }
  
    if (0 > (flags = fcntl(net_handle->fd, F_GETFL, 0)) || 0 > fcntl(net_handle->fd, F_SETFL, flags | O_NONBLOCK)) {
        loge("Failed to set non-blocking mode: %s", strerror(errno));
        goto exit1;
    }
  
    net_handle->remote.sin_family = AF_INET;
    net_handle->remote.sin_addr   = *((struct in_addr*)ip_addr->h_addr_list[0]);
    net_handle->remote.sin_port   = htons(port);

    if (7 == (net_handle->remote.sin_addr.s_addr >> 29)) {
        /** MultiCast*/
        struct ip_mreq mreq = { 0 };

        flags = 1;
        if (0 > setsockopt(net_handle->fd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags))) {
            goto exit1;
        }

        if (0 > bind(net_handle->fd, (struct sockaddr*)&(net_handle->remote), sizeof(net_handle->remote))) {
            loge("Bind failed  [%s]", strerror(errno));
            goto exit1;
        }

        mreq.imr_multiaddr.s_addr = net_handle->remote.sin_addr.s_addr;
        mreq.imr_interface.s_addr = 0;    // INADDR_ANY
        if (0 > setsockopt(net_handle->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))) {
            goto exit1;
        }
    }

    if (g_udp_packet_list == NULL) {
        g_udp_packet_list = (struct slist_head*)osl_malloc(sizeof(struct slist_head));
        osl_assert(g_udp_packet_list != NULL);
        slist_init(g_udp_packet_list);
    }

    pthread_t udp_recv_handle = 0;
    pthread_create(&udp_recv_handle, NULL, udp_recv_thread, (void*)net_handle);

    return (handle_t)net_handle;

exit1:
    if (net_handle->fd > 0) {
        close(net_handle->fd);
    }
exit:
    if (net_handle) {
        osl_free(net_handle);
    }
    return -1;
}

int32_t plat_udp_send(handle_t handle, void* buf, uint32_t len, uint32_t timeout_ms)
{
    struct udp_handle_t* net_handle    = (struct udp_handle_t*)handle;
    handle_t             countdown_tmr = 0;
    struct timeval       tv;
    fd_set               fs;
    uint32_t             left_time = 0;
    int32_t              sent_len  = 0;
    int32_t              ret       = 0;

    if ((0 > handle) || (0 == (countdown_tmr = countdown_start(timeout_ms)))) {
        return -1;
    }

    do {
        left_time  = countdown_left(countdown_tmr);
        tv.tv_sec  = left_time / 1000;
        tv.tv_usec = (left_time % 1000) * 1000;

        FD_ZERO(&fs);
        FD_SET(net_handle->fd, &fs);

        ret = select(net_handle->fd + 1, NULL, &fs, NULL, &tv);
        if (0 < ret) {
            if (FD_ISSET(net_handle->fd, &fs)) {
                ret = sendto(net_handle->fd, buf + sent_len, len - sent_len, MSG_DONTWAIT, (struct sockaddr*)&(net_handle->remote), sizeof(net_handle->remote));
                if (0 < ret) {
                    sent_len += ret;
                } else if (0 > ret) {
                    sent_len = ret;
                    break;
                }
            }
        } else if (0 > ret) {
            sent_len = ret;
            break;
        }
    } while ((sent_len < len) && (0 == countdown_is_expired(countdown_tmr)));
    countdown_stop(countdown_tmr);

    // logd("%d bytes sent", sent_len);
    return sent_len;
}

int32_t plat_udp_recv(handle_t handle, void* buf, uint32_t len, uint32_t timeout_ms)
{
    handle_t countdown_tmr = 0;

    if ((0 > handle) || (0 == (countdown_tmr = countdown_start(timeout_ms)))) {
        return -1;
    }

    while (0 == countdown_is_expired(countdown_tmr)) {
        struct slist_node* node = slist_get_head(g_udp_packet_list);
        if (node == NULL) {
            time_delay_ms(100);
            continue;
        } else {
            uint32_t                recv_len = 0;
            udp_packet_list_node_t* packet   = (udp_packet_list_node_t*)node;

            osl_memset(buf, 0, len);
            recv_len = packet->length > len ? len : packet->length;
            osl_memcpy(buf, packet->buffer, recv_len);

            slist_remove_head(g_udp_packet_list);
            osl_free(packet->buffer);
            osl_free(packet);

            countdown_stop(countdown_tmr);
        
            return recv_len;
        }
    }
    countdown_stop(countdown_tmr);
    return 0;
}

int32_t plat_udp_disconnect(handle_t handle)
{
    struct udp_handle_t* net_handle    = (struct udp_handle_t*)handle;
    handle_t             countdown_tmr = 0;
    countdown_tmr                      = countdown_start(5000);

    g_udp_close_flag = 2;
    while (g_udp_close_flag != 0) {
        time_delay_ms(50);
        if (countdown_is_expired(countdown_tmr) == 1) {
            break;
        }
    }

    countdown_stop(countdown_tmr);
    if (0 < net_handle) {
        if (7 == (net_handle->remote.sin_addr.s_addr >> 29)) {
            struct ip_mreq mreq       = { 0 };
            mreq.imr_multiaddr.s_addr = net_handle->remote.sin_addr.s_addr;
            mreq.imr_interface.s_addr = 0;    // INADDR_ANY

            setsockopt(net_handle->fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
        }
        if (0 < net_handle->fd) {
            close(net_handle->fd);
            return 0;
        }
        osl_free(net_handle);
    }

    while (g_udp_packet_list->cnt > 0) {
        struct slist_node*           slist_node_deinit           = NULL;
        struct udp_packet_list_node* udp_packet_list_node_deinit = NULL;

        slist_node_deinit = slist_get_head(g_udp_packet_list);
        slist_remove_head(g_udp_packet_list);

        udp_packet_list_node_deinit = (struct udp_packet_list_node*)slist_node_deinit;

        if (udp_packet_list_node_deinit->buffer != NULL) {
            osl_free(udp_packet_list_node_deinit->buffer);
        }
        osl_free(slist_node_deinit);
    }
    osl_free(g_udp_packet_list);
    g_udp_packet_list = NULL;

    return 0;
}

static void* udp_recv_thread(void* handle)
{
    struct udp_handle_t* net_handle = (struct udp_handle_t*)handle;
    fd_set               fs;
    int                  recv_len        = 0;
    int                  ret             = 0;
    struct sockaddr_in   recved_addr     = { 0 };
    socklen_t            recved_addr_len = sizeof(recved_addr);

    g_udp_close_flag = 1;

    pthread_detach(pthread_self());

    while (1) {
        if (g_udp_close_flag == 2) {
            g_udp_close_flag = 0;
            logd("close socket recv thread");
            return NULL;
        }
        struct timeval tv = { 2, 0 };
        FD_ZERO(&fs);
        FD_SET(net_handle->fd, &fs);
        ret = select(net_handle->fd + 1, &fs, NULL, NULL, &tv);
        // printf("Select ret %d\n", ret);
        if (0 < ret) {
            uint8_t buffer[1024] = { 0 };

            if (FD_ISSET(net_handle->fd, &fs)) {
#if 1
                recv_len = recvfrom(net_handle->fd, buffer, 1024, MSG_DONTWAIT, (struct sockaddr*)&recved_addr, &recved_addr_len);
#else
                recv_len = recv(net_handle->fd, buffer, 1024, MSG_DONTWAIT);
#endif
                if (0 < recv_len) {
                    // logd("%d bytes received [%s:%d]", recv_len, inet_ntoa(recved_addr.sin_addr), ntohs(recved_addr.sin_port));

                    udp_packet_list_node_t* udp_packet = (udp_packet_list_node_t*)osl_malloc(sizeof(udp_packet_list_node_t));

                    if (udp_packet == NULL) {
                        loge("udp rcv no mem");
                        continue;
                    }

                    osl_memset(udp_packet, 0, sizeof(udp_packet_list_node_t));

                    udp_packet->buffer = (uint8_t*)osl_malloc(recv_len + 1);
                    if (udp_packet->buffer == NULL) {
                        loge("udp rcv malloc error");
                    }
                    osl_memset(udp_packet->buffer, 0, recv_len + 1);
                    osl_memcpy(udp_packet->buffer, buffer, recv_len);

                    udp_packet->length = recv_len;

                    slist_insert_tail(g_udp_packet_list, &(udp_packet->node));
                    // logd("g_udp_packet_list cnt %d", g_udp_packet_list->cnt);
                } else if (0 > recv_len) {
                    loge("Error in recvfrom(): %d , %s", errno, strerror(errno));
                }
            }
        } else if (0 > ret) {
            goto TAG_END;
        }
    }
TAG_END:
    loge("Error in socket,recv thread exit..");
    return NULL;
}
