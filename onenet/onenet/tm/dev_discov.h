/**
 * Copyright (c), 2012~2021 iot.10086.cn All Rights Reserved
 *
 * @file dev_discov.h
 * @brief device discovery process base on mDNS protocol
 */

#ifndef __DEV_DISCOV_H__
#define __DEV_DISCOV_H__

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

    /*****************************************************************************/
    /* External Structures, Enum and Typedefs                                    */
    /*****************************************************************************/
    struct discovery_info_t
    {
        /** Product ID*/
        const uint8_t* product_id;
        /** device Name*/
        const uint8_t* dev_name;
        /** device IP Address（IPv6 Not supported yet），32 Bit address */
        uint32_t dev_addr;
        /** Active notification interval，Unit is seconds. For 0, do not report */
        uint32_t notify_interval;
    };

    /*****************************************************************************/
    /* External Variables and Functions                                          */
    /*****************************************************************************/

    /// @brief Start Device Discovery，Supports two methods: active notification and passive response
    /// @param info
    /// @return info: device Information
    int32_t discovery_start(struct discovery_info_t* info);

    /// @brief Device Discovery main Cycle，For processing MDNS Data，Repeatedly loop before processing is completed
    /// @param timeout_ms
    /// @return  0 - Processing complete，Other - Timeout
    int32_t discovery_step(uint32_t timeout_ms);

    /// @brief Stop Device Discovery
    /// @param
    /// @return
    int32_t discovery_stop(void);

#ifdef __cplusplus
}
#endif

#endif
