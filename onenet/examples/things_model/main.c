/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file main.c
 * @brief Thing model example with additional features
 */

/*****************************************************************************/
/* 头文件                                                                  */
/*****************************************************************************/

#include "common.h"
#include "log.h"
#include "tm_api.h"
#include "tm_user.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/*****************************************************************************/
/* 宏定义                                                                  */
/*****************************************************************************/
#ifndef PRODUCT_ID
#define PRODUCT_ID "FjM22ntsEG"
#endif

#ifndef DEVICE_NAME
#define DEVICE_NAME "elf2_test"
#endif

#ifndef ACCESS_KEY
#define ACCESS_KEY "aWJoejU3MzN5OXRiVDR6U0pxeUdNZk1KbzhDcDhxQ0g="
#endif

/** token 有效时间，默认为 2035.12.31 23:59:59 */
#define TM_EXPIRE_TIME 2082729599

/*****************************************************************************/
/* 函数声明                                                                  */
/*****************************************************************************/
int read_index_file(const char *index_file, char *date_str);
int read_data_by_date(const char *base_dir, const char *date_str, float *temp, float *humi, float *lx);
int read_camera_state(const char *camera_state_file, int *camera_state);

/*****************************************************************************/
/* 全局变量                                                                  */
/*****************************************************************************/
static int prev_pose_recog = -1; //-1 表示初始状态未知

/*****************************************************************************/
/* 函数实现                                                                  */
/*****************************************************************************/
/* 主函数 */
int main(int argc, char *argv[])
{
    const char *base_dir = "/home/elf/sensor/data";               //环境数据文件所在目录
    const char *index_file = "/home/elf/sensor/data/index.txt";   //索引文件
    char date_str[16];  //存储日期字符串如 "20250706"
    int camera_state = -1; //摄像头状态

    AIOT_ASSERT(PRODUCT_ID != NULL && strlen(PRODUCT_ID) > 0);
    AIOT_ASSERT(DEVICE_NAME != NULL && strlen(DEVICE_NAME) > 0);
    AIOT_ASSERT(ACCESS_KEY != NULL && strlen(ACCESS_KEY) > 0);

    struct tm_downlink_tbl_t dl_tbl;
    int ret = 0;
    handle_t timer_handle = 0;
    uint32_t timer_period = 120;

    dl_tbl.prop_tbl = tm_prop_list;
    dl_tbl.prop_tbl_size = tm_prop_list_size;
    dl_tbl.svc_tbl = tm_svc_list;
    dl_tbl.svc_tbl_size = tm_svc_list_size;

    char *product_id = (uint8_t *)PRODUCT_ID;
    char *device_sn = (uint8_t *)DEVICE_NAME;
    char *auth_code = (uint8_t *)ACCESS_KEY;

    int timeout_ms = 60 * 1000;

    char log_entry[256];

    /* 设备初始化 */
    ret = tm_init(&dl_tbl);
    CHECK_EXPR_GOTO(ERR_OK != ret, _END, "ThingModel init failed!\n");
    logi("ThingModel init ok");

    /* 设备登录 */
    ret = tm_login(product_id, device_sn, auth_code, TM_EXPIRE_TIME, timeout_ms);
    CHECK_EXPR_GOTO(ERR_OK != ret, _END, "ThingModel login failed!");
    logi("ThingModel login ok");

    int32_t pose_recog = 0;
    float temp = 0, humi = 0, lx = 0;

    while (1)
    {
        //读取索引文件获取日期
        if(read_index_file(index_file, date_str) != 0)
        {
            loge("Failed to read index file");
            break;
        }

        //读取温湿度和光强数据
        if(0 == read_data_by_date(base_dir, date_str, &temp, &humi, &lx))
        {
            //上报属性值
            tm_prop_temp_notify(NULL, temp, 0, 3000);
            tm_prop_humi_notify(NULL, humi, 0, 3000);
            tm_prop_lx_notify(NULL, lx, 0, 3000);
        }
        else
        {
            loge("Failed to read sensor data, using last values");
        }

        //非阻塞延时250ms
        struct timeval tv_status;
        tv_status.tv_sec = 0;
        tv_status.tv_usec = 250000;
        select(0, NULL, NULL, NULL, &tv_status);

        //每0.5秒读取并上传摄像头状态
        if(read_camera_state(CAMERA_STATE_FILE, &camera_state) == 0)
        {
            //printf("camera state: %d\n", camera_state);
            tm_prop_pose_recog_notify(NULL, camera_state, 0, 3000);
        }
        else
        {
            loge("Failed to read camera state");
        }

        //非阻塞延时750ms
        struct timeval tv_sensor;
        tv_sensor.tv_sec = 0;
        tv_sensor.tv_usec = 750000;
        select(0, NULL, NULL, NULL, &tv_sensor);
    }

    /* 设备注销 */
    tm_logout(3000);
_END:
    return 0;
}

/* 读取索引文件获取动作识别状态 */
int read_index_file(const char *index_file, char *date_str)
{
    FILE *fp = fopen(index_file, "r");
    if(!fp)
    {
        //只记录一次错误日志
        static int error_logged = 0;
        if(!error_logged)
        {
            loge("Failed to open index file: %s", index_file);
            error_logged = 1;
        }
        return -1;
    }

    //读取日期
    if(fscanf(fp, "%s", date_str) != 1)
    {
        loge("Failed to read date from index file");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

/* 根据日期构建文件路径并读取数据 */
int read_data_by_date(const char *base_dir, const char *date_str, float *temp, float *humi, float *lx)
{
    char year_month[7]; //存储年月部分，如 "202507"
    strncpy(year_month, date_str, 6);
    year_month[6] = '\0'; //确保字符串以 null 结尾

    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/%s/%s.txt", base_dir, year_month, date_str);

    FILE *fp = fopen(file_path, "r");
    if(!fp)
    {
        //只记录一次错误日志
        static int error_logged = 0;
        if (!error_logged)
        {
            loge("Failed to open data file: %s", file_path);
            error_logged = 1;
        }
        return -1;
    }

    char line[256];
    char last_line[256] = {0};

    //读取最后一行
    while(fgets(line, sizeof(line), fp))
    {
        if(strlen(line) > 1) //忽略空行
        {  
            strncpy(last_line, line, sizeof(last_line));
        }
    }
    fclose(fp);

    if(strlen(last_line) == 0)
    {
        loge("No valid data in file");
        return -1;
    }

    //解析数据，跳过时间戳，提取数值部分
    //格式示例：2025-07-06 23:23:23 25.12℃ 34.56%RH 789.01lx
    if(sscanf(last_line, "%*s %*s %f%*[^0-9.] %f%*[^0-9.] %f", temp, humi, lx) != 3)
    {
        loge("Invalid data format in file: %s", last_line);
        return -1;
    }

    return 0;
}

/* 读取摄像头状态 */
int read_camera_state(const char *camera_state_file, int *camera_state)
{
    FILE *fp = fopen(CAMERA_STATE_FILE, "r");
    if(!fp)
    {
        //只记录一次错误日志
        static int error_logged = 0;
        if(!error_logged)
        {
            loge("Failed to open camera state file: %s", CAMERA_STATE_FILE);
            error_logged = 1;
        }
        return -1;
    }

    //读取摄像头状态
    if(fscanf(fp, "%d", camera_state) != 1)
    {
        loge("Failed to read camera state from file");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}
