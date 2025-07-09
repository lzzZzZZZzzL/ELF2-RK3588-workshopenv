/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file tm_subdev.h
 * @brief Sub-device process for mqtt protocol
 */

#ifndef __TM_SUBDEV_H__
#define __TM_SUBDEV_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef CONFIG_TM_GATEWAY

    /*****************************************************************************/
    /* External Definition ( Constant and Macro )                                */
    /*****************************************************************************/

    /*****************************************************************************/
    /* External Structures, Enum and Typedefs                                    */
    /*****************************************************************************/
    struct tm_subdev_cbs
    {
        // Property get callback
        int32_t (*subdev_props_get)(const uint8_t* product_id, const uint8_t* dev_name, const uint8_t* props_list, uint8_t** props_data);

        // Property Settings Callback
        int32_t (*subdev_props_set)(const uint8_t* product_id, const uint8_t* dev_name, uint8_t* props_data);

        // Service call callback
        int32_t (*subdev_service_invoke)(const uint8_t* product_id, const uint8_t* dev_name, const uint8_t* svc_id, uint8_t* in_data, uint8_t** out_data);

        // Topological relationship callback
        int32_t (*subdev_topo)(uint8_t* topo_data);
    };

    /*****************************************************************************/
    /* External Variables and Functions                                          */
    /*****************************************************************************/

    /// @brief Gateway Sub-device attribute setting, attribute get, service invocation, topology relationship callback function initialization
    /// @param callbacks
    /// @return 0：Succeed;Others：Failed
    int32_t tm_subdev_init(struct tm_subdev_cbs callbacks);

    /// @brief Gateway and Sub-device Relationship Binding
    /// @param product_id Sub-device productsID
    /// @param dev_name Sub-device Device Name
    /// @param access_key Sub-device login key
    /// @param timeout_ms Timeout
    /// @return 0：Succeed;Others：Failed
    int32_t tm_subdev_add(const uint8_t* product_id, const uint8_t* dev_name, const uint8_t* access_key, uint32_t timeout_ms);

    /// @brief Gateway and Sub-device Relationship Binding deletion
    /// @param product_id Sub-device productsID
    /// @param dev_name Sub-device Device Name
    /// @param access_key Sub-device login key
    /// @param timeout_ms Timeout
    /// @return 0：Succeed;Others：Failed
    int32_t tm_subdev_delete(const uint8_t* product_id, const uint8_t* dev_name, const uint8_t* access_key, uint32_t timeout_ms);

    /// @brief Topological relation get
    /// @param timeout_ms Timeout
    /// @return 0：Succeed;Others：Failed
    int32_t tm_subdev_topo_get(uint32_t timeout_ms);

    /// @brief Sub-device login
    /// @param product_id Sub-device productsID
    /// @param dev_name Sub-device Device Name
    /// @param timeout_ms Timeout
    /// @return 0：Succeed;Others：Failed
    int32_t tm_subdev_login(const uint8_t* product_id, const uint8_t* dev_name, uint32_t timeout_ms);

    /// @brief Sub-device Logout and Login
    /// @param product_id Sub-device productsID
    /// @param dev_name Sub-device Device Name
    /// @param timeout_ms Timeout
    /// @return 0：Succeed;Others：Failed
    int32_t tm_subdev_logout(const uint8_t* product_id, const uint8_t* dev_name, uint32_t timeout_ms);

    /// @brief Data report for sub-devices
    /// @param product_id ProductID
    /// @param dev_name device Name
    /// @param prop_json Property report
    /// @param event_json Event report
    /// @param timeout_ms Timeout
    /// @return 0：Succeed;Others：Failed
    int32_t tm_subdev_post_data(const uint8_t* product_id, const uint8_t* dev_name, uint8_t* prop_json, uint8_t* event_json, uint32_t timeout_ms);

#endif
#ifdef __cplusplus
}
#endif

#endif
