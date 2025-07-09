/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file tm_api.h
 * @brief Thing Model API for multi-protocols
 */

#ifndef __TM_API_H__
#define __TM_API_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef ADD_MSG2PAYLOAD
#define ADD_MSG2PAYLOAD(payload, _msg)                                                                                                                        \
    do {                                                                                                                                                       \
        int payload_len = osl_strlen(payload);                                                                                                               \
        int _msg_len     = osl_strlen(_msg);                                                                                                                   \
        if (SDK_PAYLOAD_LEN > payload_len + _msg_len) {                                                                                         \
            osl_strcat(payload, _msg);                                                                                                                        \
        } else {                                                                                                                                               \
            loge(                                                                                                                                              \
                "payload length(%d) more than the "                                                                                                            \
                "SDK_PAYLOAD_LEN(%d)",                                                                                                           \
                payload_len + _msg_len,                                                                                                                       \
                SDK_PAYLOAD_LEN);                                                                                                                \
        }                                                                                                                                                      \
    } while (0)
#endif

#define TM_PROPERTY_RW(x)                                                                                                                                      \
    {                                                                                                                                                          \
#x, tm_prop_##x##_rd_cb, tm_prop_##x##_wr_cb                                                                                                           \
    }

#define TM_PROPERTY_RO(x)                                                                                                                                      \
    {                                                                                                                                                          \
#x, tm_prop_##x##_rd_cb, NULL                                                                                                                          \
    }

#define TM_SERVICE(x)                                                                                                                                          \
    {                                                                                                                                                          \
#x, tm_svc_##x##_cb                                                                                                                                    \
    }

    /*****************************************************************************/
    /* External Structures, Enum and Typedefs                                    */
    /*****************************************************************************/
    typedef int32_t (*tm_prop_read_cb)(void* res);
    typedef int32_t (*tm_prop_write_cb)(void* res);
    typedef int32_t (*tm_event_read_cb)(void* res);
    typedef int32_t (*tm_svc_invoke_cb)(void* in, void* out);

    struct tm_prop_tbl_t
    {
        const uint8_t*   name;
        tm_prop_read_cb  tm_prop_rd_cb;
        tm_prop_write_cb tm_prop_wr_cb;
    };

    struct tm_svc_tbl_t
    {
        const uint8_t*   name;
        tm_svc_invoke_cb tm_svc_cb;
    };

    struct tm_downlink_tbl_t
    {
        struct tm_prop_tbl_t* prop_tbl;
        struct tm_svc_tbl_t*  svc_tbl;
        uint16_t              prop_tbl_size;
        uint16_t              svc_tbl_size;
    };
    /*****************************************************************************/
    /* External Variables and Functions                                          */
    /*****************************************************************************/
    /**
     * @brief Device Initialization
     *
     * @param downlink_tbl Downlink data processing callback table，Processing properties and services calls
     * @return 0 - Succeed；-1 - Failed
     */
    int32_t tm_init(struct tm_downlink_tbl_t* downlink_tbl);
    int32_t tm_deinit(void);

    /**
     * @brief Initiate a device login request to the platform
     *
     * @param product_id Product ID
     * @param dev_name device Name
     * @param access_key Product key or device key
     * @param timeout_ms Login timeout
     * @return 0 - Login successfully；-1 - Failed
     */
    int32_t tm_login(const uint8_t* product_id, const uint8_t* dev_name, const uint8_t* access_key, uint64_t expire_time, uint32_t timeout_ms);

    /**
     * @brief Device logout
     *
     * @param timeout_ms Timeout time
     * @return 0 - Logout successfully；-1 - Failed
     */
    int32_t tm_logout(uint32_t timeout_ms);

    int32_t tm_post_property(void* prop_data, uint32_t timeout_ms);
    int32_t tm_post_event(void* event_data, uint32_t timeout_ms);
    int32_t tm_get_desired_props(uint32_t timeout_ms);
    int32_t tm_delete_desired_props(uint32_t timeout_ms);

    /**
     * @brief Packaging the properties and event data of the device，Available for sub-devices
     *
     * @param data Destination pointer address to be packaged，Used for subsequent calls to the escalation interface. Space is allocated internally by the interface when set to null，And return the address by return value
     * @param product_id Products that require packaged dataid
     * @param dev_name The name of the device that needs to package the data
     * @param prop
     * Incoming attribute data. Support json Format(as_raw set to 1)，Can also construct data with tm_data API like in tm_user.c file（as_rawset to 0）
     * @param event Same definition as prop，For incoming event data
     * @param as_raw Is it raw json Data in format
     * @return Packaged data pointer address
     */
    void*   tm_pack_device_data(void* data, const uint8_t* product_id, const uint8_t* dev_name, void* prop, void* event, int8_t as_raw);
    int32_t tm_post_pack_data(void* pack_data, uint32_t timeout_ms);
    int32_t tm_post_history_data(void* history_data, uint32_t timeout_ms);

#ifdef CONFIG_TM_GATEWAY
    typedef int32_t (*tm_subdev_cb)(const uint8_t* name, void* data, uint32_t data_len);

    int32_t tm_set_subdev_callback(tm_subdev_cb callback);

    int32_t tm_post_raw(const uint8_t* name, uint8_t* raw_data, uint32_t raw_data_len, uint8_t** reply_data, uint32_t* reply_data_len, uint32_t timeout_ms);
    int32_t
    tm_send_request(const uint8_t* name, uint8_t as_raw, void* data, uint32_t data_len, void** reply_data, uint32_t* reply_data_len, uint32_t timeout_ms);
    int32_t
    tm_send_response(const uint8_t* name, uint8_t* msg_id, int32_t msg_code, uint8_t as_raw, void* resp_data, uint32_t resp_data_len, uint32_t timeout_ms);
#endif
    int32_t tm_step(uint32_t timeout_ms);


#ifdef __cplusplus
}
#endif

#endif
