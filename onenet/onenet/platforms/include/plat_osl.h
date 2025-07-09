/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file plat_osl.h
 * @brief Standard Library Interface
 */

#ifndef __PLAT_OSL_H__
#define __PLAT_OSL_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "plat_time.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition（Constant and Macro )                                 */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
extern char* g_server_ip;
extern char* g_server_port;

void *osl_malloc(size_t size);
void *osl_calloc(size_t num, size_t size);
void  osl_free(void *ptr);

void *osl_memcpy(void *dst, const void *src, size_t n);
void *osl_memmove(void *dst, const void *src, size_t n);
void *osl_memset(void *dst, int32_t val, size_t n);

uint8_t *osl_strdup(const uint8_t *s);
uint8_t *osl_strndup(const uint8_t *s, size_t n);
uint8_t *osl_strcpy(uint8_t *s1, const uint8_t *s2);
uint8_t *osl_strncpy(uint8_t *s1, const uint8_t *s2, size_t n);
uint8_t *osl_strcat(uint8_t *dst, const uint8_t *src);
uint8_t *osl_strstr(const uint8_t *s1, const uint8_t *s2);

uint32_t osl_strlen(const uint8_t *s);
int32_t  osl_strcmp(const uint8_t *s1, const uint8_t *s2);
int32_t  osl_strncmp(const uint8_t *s1, const uint8_t *s2, size_t n);
int32_t  osl_sprintf(uint8_t *str, const uint8_t *format, ...);
int32_t  osl_sscanf(const uint8_t *str, const uint8_t *format, ...);
void     osl_assert(boolean expression);
int32_t  osl_get_random(unsigned char *buf, size_t len);
int32_t osl_atoi(const uint8_t *nptr);

/// @brief  Generation a ramdom number located in a closed interval[min,max]
/// @param  min Minimum Value
/// @param  max Maximum value
/// @return  Random number
int32_t osl_rand(int32_t min, int32_t max);

/// @brief  Generation a ramdom string with length len
/// @param  buf String cached address
/// @param  len Length
/// @return  Random String
uint8_t *osl_random_string(uint8_t *buf, int len);


int32_t module_init(void *arg,void* callback);
int32_t module_deinit(void);
#ifdef __cplusplus
}
#endif

#endif
