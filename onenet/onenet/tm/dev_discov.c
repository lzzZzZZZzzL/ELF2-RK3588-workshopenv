/**
 * Copyright (c), 2012~2021 iot.10086.cn All Rights Reserved
 *
 * @file dev_discov.c
 * @brief device discovery process base on mDNS protocol
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <string.h>

#include "err_def.h"
#include "log.h"
#include "plat_osl.h"
#include "plat_time.h"
#include "plat_udp.h"
#include "utils.h"

#include "dev_discov.h"

#if !(defined ( __CC_ARM ) || defined (__ARMCC_VERSION))

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#define MDNS_DEFAULT_ADDR "224.0.0.251"
#define MDNS_DEFAULT_PORT 5353

/** Host Name: {productID.hex}_{dev_name}.aiot_dev.local*/
/** Service Name: {productID.hex}_{dev_name}._aiot._tcp.local*/
/** DevDiscovery：aiot_dd._udp.local*/
#define DEFAULT_DEV_TYPE "aiot_dev"

#define DD_SRVNAME "_aiot_dd"
#define DD_PROTO "_udp"

#define DEFAULT_SRVNAME "_aiot"
#define DEFAULT_PROTO "_tcp"
#define DEFAULT_DOMAIN "local"
#define DEFAULT_RR_TTL 600

#define MDNS_RRTYPE_A 1
#define MDNS_RRTYPE_PTR 12
#define MDNS_RRTYPE_TXT 16
#define MDNS_RRTYPE_SRV 33
#define MDNS_RRTYPE_ANY 255

#define MDNS_RRCLASS_IN 1

// Standard query
#define MDNS_SET_FLAGS(QR) (QR << 15)

#define HOSTNAME_LABEL_COUNT_MAX 3
#define SERVICE_INSTANCE_LABEL_COUNT_MAX 4

#define NETWORK_SENDRECV_BUF_LEN 512
/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
struct mdns_header_t
{
    uint16_t transaction_id;
#if 0 /* Bit define for Flags*/
    struct
    {
        /** Response code*/
        uint16_t rcode   : 4;
        /** Reserved*/
        uint16_t         : 3;
        /** Recursion Availabe*/
        uint16_t ra      : 1;
        /** Recursion Desired*/
        uint16_t rd      : 1;
        /** Trun Cation*/
        uint16_t tc      : 1;
        /** Authoritative Answer*/
        uint16_t aa      : 1;
        /** Kind of query*/
        uint16_t op_code : 4;
        /** Query(0) or Response(1)*/
        uint16_t qr      : 1;
    } flags_value;
#endif
    uint16_t flags;

    /** Number of question entries*/
    uint16_t qd_count;
    /** Number of answer entries*/
    uint16_t an_count;
    /** Number of name server rr*/
    uint16_t ns_count;
    /** Number of additional rr*/
    uint16_t ar_count;
};

struct devdiscov_t
{
    handle_t net_handle;
    uint8_t* net_buf;
    uint8_t* host_name;
    uint32_t s_addr;
    uint32_t notify_interval;
    uint32_t last_notify_time;
};

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
static struct devdiscov_t* g_discov;

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
static uint32_t mdns_pack_header(uint8_t* buf, struct mdns_header_t* mdns_hdr)
{
    uint32_t offset = 0;

    offset += set_16bit_be(buf + offset, mdns_hdr->transaction_id);
    offset += set_16bit_be(buf + offset, mdns_hdr->flags);
    offset += set_16bit_be(buf + offset, mdns_hdr->qd_count);
    offset += set_16bit_be(buf + offset, mdns_hdr->an_count);
    offset += set_16bit_be(buf + offset, mdns_hdr->ns_count);
    offset += set_16bit_be(buf + offset, mdns_hdr->ar_count);

    return offset;
}

static uint32_t mdns_parse_header(uint8_t* buf, struct mdns_header_t* mdns_hdr)
{
    uint32_t offset = 0;

    offset += get_16bit_be(buf + offset, &(mdns_hdr->transaction_id));
    offset += get_16bit_be(buf + offset, &(mdns_hdr->flags));
    offset += get_16bit_be(buf + offset, &(mdns_hdr->qd_count));
    offset += get_16bit_be(buf + offset, &(mdns_hdr->an_count));
    offset += get_16bit_be(buf + offset, &(mdns_hdr->ns_count));
    offset += get_16bit_be(buf + offset, &(mdns_hdr->ar_count));

    return offset;
}

/**
 * @brief Check the specified flash address(buf) if the specified domain name is encoded in the address (name_labels[0].xxxx.name_labels[label_cnt - 1])
 *
 * @param buf
 * @param buf_len
 * @param name_labels
 * @param label_cnt
 * @return  1 - Can match a specified domain name
 *          0 - Cannot match the specified domain name
 */
static uint32_t check_name_labels(uint8_t* buf, int32_t buf_len, uint8_t* name_labels[], uint32_t label_cnt)
{
    uint8_t label_len = 0;

    label_len = osl_strlen(name_labels[0]);
    if (buf_len < (label_len + 2)) {
        return 0;
    }
    if ((*buf == label_len) && (0 == osl_strncmp(buf + 1, name_labels[0], label_len))) {
        if (label_cnt == 1) {
            if (*(buf + 1 + label_len) == '\0') {
                return 1;
            }
        } else {
            return check_name_labels(buf + 1 + label_len, buf_len - 1 - label_len, &name_labels[1], label_cnt - 1);
        }
    }
    return 0;
}

static uint32_t mdns_parse_name(uint8_t* data, uint32_t offset, uint8_t* name_labels[], uint32_t* label_cnt)
{
    uint32_t tmp_offset       = offset;
    uint32_t parsed_label_cnt = 0;
    uint8_t  label_len        = 0;

    do {
        label_len = data[tmp_offset++];
        if (0x3 == ((label_len >> 6) & 0x3)) {
            uint16_t compressed_label_offset = 0;
            uint32_t tmp_label_cnt           = ((*label_cnt > parsed_label_cnt) ? (*label_cnt - parsed_label_cnt) : 0);

            /** Compressed label*/
            compressed_label_offset = ((label_len & 0x3F) << 8) | data[tmp_offset++];
            mdns_parse_name(data, compressed_label_offset, &name_labels[*label_cnt - tmp_label_cnt], &tmp_label_cnt);
            parsed_label_cnt += tmp_label_cnt;
            tmp_offset += 2;
            break;
        } else {
            if (parsed_label_cnt < *label_cnt) {
                name_labels[parsed_label_cnt] = osl_strndup(data + tmp_offset, label_len);
            }
            parsed_label_cnt++;
            tmp_offset += label_len;
            if ('\0' == data[tmp_offset]) {
                tmp_offset++;
                break;
            }
        }
    } while (1);

    *label_cnt = parsed_label_cnt;

    return (tmp_offset - offset);
}

static uint32_t mdns_pack_name(uint8_t* buf, uint32_t offset, uint8_t* name_labels[], uint32_t label_cnt)
{
    uint8_t  i          = 0;
    uint32_t tmp_offset = 0;
    uint8_t* tmp_ptr    = NULL;
    uint8_t  label_len  = 0;
    uint32_t packed_len = 0;

    for (i = 0; i < label_cnt; i++) {
        /** Lookup backwards for each label*/
        tmp_offset = 0;
        do {
            label_len = osl_strlen(name_labels[i]);
            tmp_ptr   = memchr(buf + tmp_offset, label_len, offset - tmp_offset);
            if (tmp_ptr) {
                tmp_offset = tmp_ptr - buf;
                if (1 == check_name_labels(tmp_ptr, buf + offset - tmp_ptr, &name_labels[i], label_cnt - i)) {
                    /** Compress*/
                    packed_len += set_16bit_be(buf + offset + packed_len, (3 << 14 | tmp_offset));
                    return packed_len;
                } else {
                    tmp_offset += 1;
                }
            } else {
                buf[offset + packed_len++] = label_len;
                osl_memcpy(buf + offset + packed_len, name_labels[i], label_len);
                packed_len += label_len;
                if (i == (label_cnt - 1)) {
                    buf[offset + packed_len++] = '\0';
                }
                break;
            }
        } while (1);
    }

    return packed_len;
}

/**
 * @brief Domain name -> IP
 *
 * @param buf
 * @param name
 * @param s_addr
 * @return
 */
static uint32_t mdns_pack_rr_a(uint8_t* buf, uint32_t offset, uint8_t* name_labels[], uint32_t label_cnt, uint32_t s_addr)
{
    uint32_t packed_len = 0;

    packed_len += mdns_pack_name(buf, offset + packed_len, name_labels, label_cnt);
    packed_len += set_16bit_be(buf + offset + packed_len, MDNS_RRTYPE_A);
    packed_len += set_16bit_be(buf + offset + packed_len, MDNS_RRCLASS_IN);
    /** TTL*/
    packed_len += set_32bit_be(buf + offset + packed_len, DEFAULT_RR_TTL);
    /** IP Address Length*/
    packed_len += set_16bit_be(buf + offset + packed_len, sizeof(s_addr));
    /** IP Address*/
    packed_len += set_32bit_be(buf + offset + packed_len, s_addr);

    return packed_len;
}

static uint32_t
mdns_pack_rr_srv(uint8_t* buf, uint32_t offset, uint8_t* srv_name_labels[], uint32_t srv_label_cnt, uint8_t* host_name_labels[], uint32_t host_label_cnt)
{
    uint32_t packed_len       = 0;
    uint32_t rdata_len_offset = 0;
    uint16_t rdata_len        = 0;

    packed_len += mdns_pack_name(buf, offset + packed_len, srv_name_labels, srv_label_cnt);
    packed_len += set_16bit_be(buf + offset + packed_len, MDNS_RRTYPE_SRV);
    packed_len += set_16bit_be(buf + offset + packed_len, MDNS_RRCLASS_IN);

    packed_len += set_32bit_be(buf + offset + packed_len, DEFAULT_RR_TTL);

    rdata_len_offset = packed_len;
    packed_len += 2;
    /** Priority*/
    packed_len += set_16bit_be(buf + offset + packed_len, 0);
    /** Weight*/
    packed_len += set_16bit_be(buf + offset + packed_len, 0);
    /** Port*/
    packed_len += set_16bit_be(buf + offset + packed_len, 0);
    /** Target*/
    packed_len += mdns_pack_name(buf, offset + packed_len, host_name_labels, host_label_cnt);

    /** RData Length*/
    rdata_len = packed_len - rdata_len_offset - 2;
    set_16bit_be(buf + offset + rdata_len_offset, rdata_len);

    return packed_len;
}

static uint32_t mdns_pack_rr_ptr(uint8_t* buf, uint32_t offset, uint8_t* name_labels[], uint32_t label_cnt, uint8_t* domain_labels[], uint32_t domain_label_cnt)
{
    uint32_t packed_len       = 0;
    uint32_t rdata_len_offset = 0;
    uint16_t rdata_len        = 0;

    packed_len += mdns_pack_name(buf, offset + packed_len, name_labels, label_cnt);
    packed_len += set_16bit_be(buf + offset + packed_len, MDNS_RRTYPE_PTR);
    packed_len += set_16bit_be(buf + offset + packed_len, MDNS_RRCLASS_IN);

    packed_len += set_32bit_be(buf + offset + packed_len, DEFAULT_RR_TTL);

    rdata_len_offset = packed_len;
    packed_len += 2;
    packed_len += mdns_pack_name(buf, offset + packed_len, domain_labels, domain_label_cnt);

    /** RData Length*/
    rdata_len = packed_len - rdata_len_offset - 2;
    set_16bit_be(buf + offset + rdata_len_offset, rdata_len);

    return packed_len;
}

static uint32_t mdns_pack_question(uint8_t* buf, uint32_t offset, uint8_t* qname_labels[], uint32_t qname_label_cnt, uint16_t qtype, uint16_t qclass)
{
    uint32_t packed_len = 0;

    packed_len += mdns_pack_name(buf, offset + packed_len, qname_labels, qname_label_cnt);
    packed_len += set_16bit_be(buf + offset + packed_len, qtype);
    packed_len += set_16bit_be(buf + offset + packed_len, qclass);

    return packed_len;
}

static uint32_t mdns_parse_question(uint8_t* buf, uint32_t offset, uint8_t* qname_labels[], uint32_t* qname_label_cnt, uint16_t* qtype, uint16_t* qclass)
{
    uint32_t parsed_len = 0;

    parsed_len += mdns_parse_name(buf, offset, qname_labels, qname_label_cnt);
    parsed_len += get_16bit_be(buf + offset + parsed_len, qtype);
    parsed_len += get_16bit_be(buf + offset + parsed_len, qclass);

    return parsed_len;
}

static uint32_t pack_rdata_a(uint8_t* buf, uint32_t offset, uint32_t s_addr)
{
    uint32_t packed_len = 0;

    /** IP Address Length*/
    packed_len += set_16bit_be(buf + offset + packed_len, sizeof(s_addr));
    /** IP Address*/
    packed_len += set_32bit_be(buf + offset + packed_len, s_addr);

    return packed_len;
}

static uint32_t pack_rdata_ptr(uint8_t* buf, uint32_t offset, uint8_t* domain_labels[], uint32_t domain_label_cnt)
{
    uint32_t packed_len = 2;

    packed_len += mdns_pack_name(buf, offset + packed_len, domain_labels, domain_label_cnt);
    // logd("%d %d", packed_len, domain_label_cnt);

    set_16bit_be(buf + offset, packed_len - 2);

    return packed_len;
}

static uint32_t
pack_rdata_srv(uint8_t* buf, uint32_t offset, uint16_t priority, uint16_t weight, uint16_t port, uint8_t* target_labels[], uint32_t target_label_cnt)
{
    uint32_t packed_len = 2;

    packed_len += set_16bit_be(buf + offset + packed_len, priority);
    packed_len += set_16bit_be(buf + offset + packed_len, weight);
    packed_len += set_16bit_be(buf + offset + packed_len, port);
    packed_len += mdns_pack_name(buf, offset + packed_len, target_labels, target_label_cnt);

    set_16bit_be(buf + offset, packed_len - 2);

    return packed_len;
}

static uint32_t
mdns_pack_rr_header(uint8_t* buf, uint32_t offset, uint8_t* rr_name_labels[], uint32_t label_cnt, uint16_t rr_type, uint16_t rr_class, uint32_t ttl)
{
    uint32_t packed_len = 0;

    packed_len += mdns_pack_question(buf, offset, rr_name_labels, label_cnt, rr_type, rr_class);
    packed_len += set_32bit_be(buf + offset + packed_len, ttl);

    return packed_len;
}

/** Multicast content：RRTYPE_A*/
static int32_t mdns_answer(void)
{
    struct mdns_header_t mdns_hdr    = { 0 };
    uint32_t             packet_len  = 0;
    uint8_t*             srv_name[4] = { 0 };

    srv_name[0] = g_discov->host_name;
    srv_name[1] = DD_SRVNAME;
    srv_name[2] = DD_PROTO;
    srv_name[3] = DEFAULT_DOMAIN;

    mdns_hdr.flags    = (1 << 15);
    mdns_hdr.an_count = 1;

    packet_len += mdns_pack_header(g_discov->net_buf, &mdns_hdr);
    // packet_len += mdns_pack_rr_a(g_discov->net_buf, packet_len, hostname, 3, g_discov->s_addr);
    // packet_len += mdns_pack_rr_srv(g_discov->net_buf, packet_len, srv_name, 4, hostname, 3);
    // packet_len += mdns_pack_rr_ptr(g_discov->net_buf, packet_len, &srv_name[1], 3, srv_name, 4);
    packet_len += mdns_pack_rr_header(g_discov->net_buf, packet_len, &srv_name[1], 3, MDNS_RRTYPE_PTR, MDNS_RRCLASS_IN, DEFAULT_RR_TTL);
    packet_len += pack_rdata_ptr(g_discov->net_buf, packet_len, srv_name, 4);

    plat_udp_send(g_discov->net_handle, g_discov->net_buf, packet_len, 3000);

    return 0;
}

static void clean_name_buf(uint8_t* name[], uint32_t name_cnt)
{
    uint32_t i = 0;

    for (i = 0; i < name_cnt; i++) {
        if (name[i]) {
            osl_free(name[i]);
            name[i] = NULL;
        }
    }
}

static int32_t parse_and_process_query(uint8_t* data, uint32_t data_len)
{
    struct mdns_header_t mdns_hdr = { 0 };

    uint32_t offset          = 0;
    uint32_t i               = 0;
    uint32_t answer_cnt      = 0;
    uint8_t* qname[4]        = { 0 };
    uint32_t qname_label_cnt = 4;
    uint16_t qtype           = 0;
    uint16_t qclass          = 0;
    uint8_t* answers         = NULL;
    uint32_t answer_len      = sizeof(mdns_hdr);
    int32_t  ret             = 0;

    offset += mdns_parse_header(data, &mdns_hdr);

    /** Process query only*/
    if (0 == ((mdns_hdr.flags >> 15) & 0x1)) {
        if (NULL == (answers = osl_calloc(1, 160))) {
            return ERR_ALLOC;
        }

        for (i = 0; i < mdns_hdr.qd_count; i++) {
            qname_label_cnt = 4;
            offset          = mdns_parse_question(data, offset, qname, &qname_label_cnt, &qtype, &qclass);
            if (qname_label_cnt > 4) {
                clean_name_buf(qname, ((qname_label_cnt > 4) ? 4 : qname_label_cnt));
                continue;
            }
            switch (qtype) {
                case MDNS_RRTYPE_A:
                    if ((2 == qname_label_cnt) && (0 == osl_strcmp(qname[0], DEFAULT_DEV_TYPE)) && (0 == osl_strcmp(qname[1], DEFAULT_DOMAIN))) {
                        logd("[ MDNS_RRTYPE_A ] qname is %s%s% s%s",qname[0],qname[1],qname[2],qname[3]);
                        answer_len += mdns_pack_rr_header(answers, answer_len, qname, qname_label_cnt, qtype, qclass, DEFAULT_RR_TTL);
                        answer_len += pack_rdata_a(answers, answer_len, g_discov->s_addr);
                        answer_cnt++;
                    }
                    break;
                case MDNS_RRTYPE_PTR:
                    if ((3 == qname_label_cnt) && (0 == osl_strcmp(qname[0], DD_SRVNAME)) && (0 == osl_strcmp(qname[1], DD_PROTO))
                        && (0 == osl_strcmp(qname[2], DEFAULT_DOMAIN))) {
                        logd("[MDNS_RRTYPE_PTR] qname is %s.%s.%s.%s",qname[0],qname[1],qname[2],qname[3]);
                        uint8_t* domain[4] = { 0 };

                        domain[0] = g_discov->host_name;
                        domain[1] = DD_SRVNAME;
                        domain[2] = DD_PROTO;
                        domain[3] = DEFAULT_DOMAIN;

                        answer_len += mdns_pack_rr_header(answers, answer_len, qname, qname_label_cnt, qtype, qclass, DEFAULT_RR_TTL);
                        answer_len += pack_rdata_ptr(answers, answer_len, domain, 4);
                        answer_cnt++;
                    }
                    break;
                case MDNS_RRTYPE_SRV:
                    if ((4 == qname_label_cnt) && (0 == osl_strcmp(qname[0], g_discov->host_name)) && (0 == osl_strcmp(qname[1], DD_SRVNAME))
                        && (0 == osl_strcmp(qname[2], DD_PROTO)) && (0 == osl_strcmp(qname[3], DEFAULT_DOMAIN))) {
                        logd("[MDNS_RRTYPE_SRV] qname is %s.%s.%s.%s",qname[0],qname[1],qname[2],qname[3]);
                        uint8_t* target[3] = { 0 };

                        target[0] = g_discov->host_name;
                        target[1] = DEFAULT_DEV_TYPE;
                        target[2] = DEFAULT_DOMAIN;
                        answer_len += mdns_pack_rr_header(answers, answer_len, qname, qname_label_cnt, qtype, qclass, DEFAULT_RR_TTL);
                        answer_len += pack_rdata_srv(answers, answer_len, 0, 0, 0, target, 3);
                        answer_cnt++;
                        ret = 1;
                    }
                    break;
                default:
                    break;
            }
            clean_name_buf(qname, ((qname_label_cnt > 4) ? 4 : qname_label_cnt));
        }
        if (answer_cnt) {
            osl_memset(&mdns_hdr, 0, sizeof(mdns_hdr));
            mdns_hdr.flags    = (1 << 15);
            mdns_hdr.an_count = answer_cnt;
            mdns_pack_header(answers, &mdns_hdr);
            // logd("send answers(%d)",answer_len);
            plat_udp_send(g_discov->net_handle, answers, answer_len, 3000);
        }
    }

    if (answers) {
        osl_free(answers);
    }
    return ret;
}

int32_t discovery_start(struct discovery_info_t* info)
{
    int32_t ret = ERR_OK;

    if (!info || !info->product_id || !info->dev_name || !info->dev_addr) {
        return ERR_INVALID_PARAM;
    }
    g_discov = osl_calloc(1, sizeof(*g_discov));
    if (!g_discov) {
        return ERR_ALLOC;
    }
    g_discov->host_name = osl_calloc(1, 2 + osl_strlen(info->product_id) + osl_strlen(info->dev_name));
    if (!g_discov->host_name) {
        ret = ERR_ALLOC;
        goto exit;
    }
    osl_sprintf(g_discov->host_name, "%s_%s", info->product_id, info->dev_name);
    g_discov->notify_interval = info->notify_interval;
    g_discov->s_addr          = info->dev_addr;

    g_discov->net_handle = plat_udp_connect(MDNS_DEFAULT_ADDR, MDNS_DEFAULT_PORT);
    if (g_discov->net_handle < 0) {
        ret = ERR_NETWORK;
        goto exit1;
    }
    g_discov->net_buf = osl_calloc(1, NETWORK_SENDRECV_BUF_LEN);
    if (!g_discov->net_buf) {
        ret = ERR_ALLOC;
        goto exit2;
    }

    return ERR_OK;

exit2:
    plat_udp_disconnect(g_discov->net_handle);
exit1:
    osl_free(g_discov->host_name);
exit:
    osl_free(g_discov);
    return ret;
}

int32_t discovery_step(uint32_t timeout_ms)
{
    handle_t cd_hdl     = countdown_start(timeout_ms);
    uint32_t recved_len = 0;
    int32_t  ret        = ERR_TIMEOUT;

    do {
        /** Attempt to receive UDP Message*/
        recved_len = plat_udp_recv(g_discov->net_handle, g_discov->net_buf, NETWORK_SENDRECV_BUF_LEN, countdown_left(cd_hdl));
        if (0 < recved_len) {
            if (1 == parse_and_process_query(g_discov->net_buf, recved_len)) {
                ret = ERR_OK;
                break;
            }
        }

        /** Proactive notification*/
        if (g_discov->notify_interval) {
            uint64_t now = time_count();
            if (g_discov->notify_interval <= (now - g_discov->last_notify_time)) {
                logd("Notify mdns");
                mdns_answer();
                g_discov->last_notify_time = now;
            }
        }
        time_delay_ms(10);
    } while (0 == countdown_is_expired(cd_hdl));
    countdown_stop(cd_hdl);

    return ret;
}

int32_t discovery_stop(void)
{
    if (g_discov) {
        if (0 <= g_discov->net_handle) {
            plat_udp_disconnect(g_discov->net_handle);
        }
        if (g_discov->net_buf) {
            osl_free(g_discov->net_buf);
        }
        if (g_discov->host_name) {
            osl_free(g_discov->host_name);
        }
        osl_free(g_discov);
    }
    return ERR_OK;
}

#endif