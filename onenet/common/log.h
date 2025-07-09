/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file        log.h
 * @brief       Log library
 */
#ifndef _LOG_H_
#define _LOG_H_

/*
 * log is thread-safe,
 * 2021/01/25 suchangwei
 */

#include "plat_osl.h"
#include <string.h>

#ifndef DIR_SEPARATOR
#define DIR_SEPARATOR '/'
#endif // DIR_SEPARATOR

#ifndef DIR_SEPARATOR_STR
#define DIR_SEPARATOR_STR "/"
#endif // DIR_SEPARATOR_STR

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

//3KB
#ifndef LOG_MAX_BUFSIZE
#define LOG_MAX_BUFSIZE 1*1024
#endif

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(DIR_SEPARATOR_STR __FILE__, DIR_SEPARATOR) + 1)
#endif // __FILENAME__

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        LOG_LEVEL_VERBOSE = 0,
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_INFO,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_SILENT
    } log_level_e;

    /**
     * @brief Custom log output，Instead of stdout
     * @param level Log level
     * @param file The file where the log is located
     * @param line Number of lines in the file where the log is located
     * @param func Function where the log is located
     * @param fmt Log
     * @return Without
     */
    typedef void (*logger_handler)(int level,const char* file,int line,const char* func,const char* fmt,int len);

    /**
     * @brief Setting OneNET Log level
     * @param level Log level
     * @return Without
     */
    void logger_set_level(int level);
    void logger_set_level_by_str(const char* level);

    /**
     * @brief Close OneNET Log Output
     * @return Without
     */
    void logger_disable();

    /**
     * @brief Enable Logger Log Color Rendering
     * @param on 0/1
     * @return Without
     */
    void logger_color(int on);

#define logger_enable_color() logger_color(1)
#define logger_disable_color() logger_color(0)

    /// @brief User-defined logger
    /// @param handler Log processing callback function
    /// @attention After user-defined log output，Other functions for OneNET Logger are invalid
    void logger_set_handler(logger_handler handler);

    /// @brief Log Output
    /// @param level Log level
    /// @param file The file where the log is located
    /// @param line Number of lines in the file where the log is located
    /// @param func Function where the log is located
    /// @param fmt Log
    /// @param suffix Suffix，Line breaks or other literals
    /// @return
    int logger_print(int level, const char* file, int line, const char* func, const char* suffix, const char* fmt, ...);

    int logger_print_buf(int level, const char* file, int line, const char* func, const char* name, const char* buf,int len);


#define logd(...) logger_print(LOG_LEVEL_DEBUG, __FILENAME__, __LINE__, __FUNCTION__, "\n",__VA_ARGS__)
#define logi(...) logger_print(LOG_LEVEL_INFO, __FILENAME__, __LINE__, __FUNCTION__, "\n",__VA_ARGS__)
#define loge(...) logger_print(LOG_LEVEL_ERROR, __FILENAME__, __LINE__, __FUNCTION__, "\n",__VA_ARGS__)





#ifdef __cplusplus
}    // extern "C"
#endif

#endif    // _LOG_H_
