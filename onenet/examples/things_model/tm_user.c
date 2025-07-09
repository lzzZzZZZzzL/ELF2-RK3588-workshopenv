/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_user.c
 * @date 2020/05/14
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "tm_data.h"
#include "tm_api.h"
#include "tm_user.h"

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
/*************************** Property Func List ******************************/
struct tm_prop_tbl_t tm_prop_list[] = {
    TM_PROPERTY_RO(humi),
    TM_PROPERTY_RO(lx),
    TM_PROPERTY_RW(pose_recog),
    TM_PROPERTY_RO(temp)
};
uint16_t tm_prop_list_size = ARRAY_SIZE(tm_prop_list);
/****************************** Auto Generated *******************************/

/***************************** Service Func List *******************************/
struct tm_svc_tbl_t tm_svc_list[] = {0};
uint16_t tm_svc_list_size = 0;
/****************************** Auto Generated *******************************/

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
/**************************** Property Func Read *****************************/
int32_t tm_prop_humi_rd_cb(void *data)
{
    float32_t val = 0.0;
    tm_data_struct_set_float(data, "humi", val);
    return 0;
}

int32_t tm_prop_lx_rd_cb(void *data)
{
    float32_t val = 0.0;
    tm_data_struct_set_float(data, "lx", val);
    return 0;
}

int32_t tm_prop_pose_recog_rd_cb(void *data)
{
    int32_t val = 0;
    tm_data_struct_set_int32(data, "pose_recog", val);
    return 0;
}

int32_t tm_prop_temp_rd_cb(void *data)
{
    float32_t val = 0.0;
    tm_data_struct_set_float(data, "temp", val);
    return 0;
}

/****************************** Auto Generated *******************************/

/**************************** Property Func Write ****************************/
int32_t tm_prop_pose_recog_wr_cb(void *data)
{
    int32_t val = 0;
    tm_data_get_int32(data, &val);

    //接收平台下发的摄像头状态并写入文件
    FILE *fp = fopen(CAMERA_STATE_FILE, "w");
    if(fp)
    {
        fprintf(fp, "%d\n", val);
        fclose(fp);
        //logi("Updated camera state to %d", val);
    }
    else
    {
        loge("Failed to open camera state file for writing");
    }

    return 0;
}

/****************************** Auto Generated *******************************/

/**************************** Property Func Notify ***************************/
int32_t tm_prop_humi_notify(void *data, float32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
	void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_float(resource, "humi", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_lx_notify(void *data, float32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
	void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_float(resource, "lx", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_pose_recog_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
	void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "pose_recog", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_temp_notify(void *data, float32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
	void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_float(resource, "temp", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}


/****************************** Auto Generated *******************************/

/***************************** Event Func Notify *****************************/

/****************************** Auto Generated *******************************/

/**************************** Service Func Invoke ****************************/

/****************************** Auto Generated *******************************/
