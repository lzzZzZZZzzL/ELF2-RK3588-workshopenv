/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_user.h
 * @date 2020/05/14
 * @brief
 */

#ifndef __TM_USER_H__
#define __TM_USER_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "tm_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/
#define CAMERA_STATE_FILE "/home/elf/sensor/data/camera_state.txt"  //摄像头状态文件路径

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
/****************************** Structure type *******************************/

/****************************** Auto Generated *******************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/*************************** Property Func List ******************************/
extern struct tm_prop_tbl_t tm_prop_list[];
extern uint16_t tm_prop_list_size;
/****************************** Auto Generated *******************************/

/**************************** Service Func List ******************************/
extern struct tm_svc_tbl_t tm_svc_list[];
extern uint16_t tm_svc_list_size;
/****************************** Auto Generated *******************************/

/**************************** Property Func Read ****************************/
int32_t tm_prop_humi_rd_cb(void *data);
int32_t tm_prop_lx_rd_cb(void *data);
int32_t tm_prop_pose_recog_rd_cb(void *data);
int32_t tm_prop_temp_rd_cb(void *data);

/****************************** Auto Generated *******************************/

/**************************** Service Func Invoke ****************************/

/****************************** Auto Generated *******************************/

/**************************** Property Func Write ****************************/
int32_t tm_prop_pose_recog_wr_cb(void *data);

/****************************** Auto Generated *******************************/

/**************************** Property Func Notify ***************************/
int32_t tm_prop_humi_notify(void *data, float32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_lx_notify(void *data, float32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_pose_recog_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_temp_notify(void *data, float32_t val, uint64_t timestamp, uint32_t timeout_ms);

/****************************** Auto Generated *******************************/

/***************************** Event Func Notify *****************************/

/****************************** Auto Generated *******************************/

#ifdef __cplusplus
}
#endif

#endif
