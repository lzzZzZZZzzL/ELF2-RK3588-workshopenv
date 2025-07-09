/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file tm_data.h
 * @brief Thing Model data process,based on json
 * Data is the subdivided attributes and services of the Thing model device，Divided into Property,Event,Service。
 */

#ifndef _TM_DATA_H_
#define _TM_DATA_H_

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "tm_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
typedef int32_t (*tm_list_cb)(const uint8_t * /**data_name*/,
                              void * /** data*/);

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief Create a data instance
 *
 * @return void* Data instance handle
 */
void *tm_data_create();

/**
 * @brief Create a Structure Data Instance
 *
 * @return
 */
void *tm_data_struct_create(void);

/**
 * @brief Create an array data instance
 *
 * @param size Specify the number of array elements
 * @return
 */
void *tm_data_array_create(uint32_t size);
int32_t tm_data_array_set_bool(void *array, boolean val);
int32_t tm_data_array_set_enum(void *array, int32_t val);
int32_t tm_data_array_set_int32(void *array, int32_t val);
int32_t tm_data_array_set_int64(void *array, int64_t val);
int32_t tm_data_array_set_date(void *array, int64_t val);
int32_t tm_data_array_set_float(void *array, float32_t val);
int32_t tm_data_array_set_double(void *array, float64_t val);
int32_t tm_data_array_set_string(void *array, uint8_t *val);
int32_t tm_data_array_set_bitmap(void *array, int32_t val);
int32_t tm_data_array_set_struct(void *array, void *val);

/**
 * @brief Destroy the created data instance
 *
 * @param res Data instances that need to be destroyed
 */
void tm_data_delete(void *data);

/**
 * @brief Add a Boolean data to a data instance
 *
 * @param res Specify data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @param timestamp Specify data timestamp，For0Is invalid
 * @return int32_t
 */
int32_t tm_data_set_bool(void *data, const int8_t *name, boolean val,
                         uint64_t timestamp);
int32_t tm_data_get_bool(void *data, boolean *val);

/**
 * @brief Add an enumerated type of data to a data instance
 *
 * @param res Specify data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @param timestamp Specify data timestamp，For0Is invalid
 * @return int32_t
 */
int32_t tm_data_set_enum(void *data, const int8_t *name, int32_t val,
                         uint64_t timestamp);
int32_t tm_data_get_enum(void *data, int32_t *val);

/**
 * @brief Add a data instance32Bit integer type data
 *
 * @param res Specify data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @param timestamp Specify data timestamp，For0Is invalid
 * @return int32_t
 */
int32_t tm_data_set_int32(void *data, const int8_t *name, int32_t val,
                          uint64_t timestamp);
int32_t tm_data_get_int32(void *data, int32_t *val);

/**
 * @brief Add a data instance64Bit integer type data
 *
 * @param res Specify data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @param timestamp Specify data timestamp，For0Is invalid
 * @return int32_t
 */
int32_t tm_data_set_int64(void *data, const int8_t *name, int64_t val,
                          uint64_t timestamp);
int32_t tm_data_get_int64(void *data, int64_t *val);

/// @brief Get buffer data Type
/// @param data OneJSON Handle
/// @param name Node name
/// @param val Binary array address
/// @param size Binary array length
/// @return 0:Succeed;Other:Failed
int32_t tm_data_get_buffer(void *data, uint8_t **val, uint32_t *size);
int32_t tm_data_set_buffer(void *data, const uint8_t *name, uint8_t *val,
                           uint32_t size, uint64_t timestamp);

/**
 * @brief Add a time-type data to a data instance
 *
 * @param res Specify data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values，UTCTimestamp，Unitms
 * @param timestamp Specify data timestamp，For0Is invalid
 * @return int32_t
 */
int32_t tm_data_set_date(void *data, const int8_t *name, int64_t val,
                         uint64_t timestamp);
int32_t tm_data_get_date(void *data, int64_t *val);

/**
 * @brief Add a single-precision floating-point type data to a data instance
 *
 * @param res Specify data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @param timestamp Specify data timestamp，For0Is invalid
 * @return int32_t
 */
int32_t tm_data_set_float(void *data, const int8_t *name, float32_t val,
                          uint64_t timestamp);
int32_t tm_data_get_float(void *data, float32_t *val);

/**
 * @brief Add a double-precision floating-point type data to a data instance
 *
 * @param res Specify data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @param timestamp Specify data timestamp，For0Is invalid
 * @return int32_t
 */
int32_t tm_data_set_double(void *data, const int8_t *name, float64_t val,
                           uint64_t timestamp);
int32_t tm_data_get_double(void *data, float64_t *val);

/**
 * @brief Add a bitmap type data to a data instance
 *
 * @param res Specify data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @param timestamp Specify data timestamp，For0Is invalid
 * @return int32_t
 */
int32_t tm_data_set_bitmap(void *data, const int8_t *name, uint32_t val,
                           uint64_t timestamp);
int32_t tm_data_get_bitmap(void *data, uint32_t *val);

/**
 * @brief Add a string type data to a data instance
 *
 * @param res Specify data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values，Need to start with '\0' End
 * @param timestamp Specify data timestamp,For 0 is invalid
 * @return int32_t
 */
int32_t tm_data_set_string(void *data, const int8_t *name, int8_t *val,
                           uint64_t timestamp);
int32_t tm_data_get_string(void *data, int8_t **val);

/**
 * @brief Add a struct or array type data to a data instance
 *
 * @param res Specify data instance
 * @param name Specify the data identity to be added
 * @param val Structure data created by tm_data_struct_create or tm_data_array_create
 * @param timestamp Specify data timestamp,For 0 is invalid
 * @return int32_t
 */
int32_t tm_data_set_data(void *data, const int8_t *name, void *val,
                         uint64_t timestamp);
int32_t tm_data_get_data(void *data, const int8_t *name, void **val);

int32_t tm_data_set_struct(void *data, const int8_t *name, void *val,
                           uint64_t timestamp);

int32_t tm_data_set_array(void *data, const int8_t *name, void *val,
                          uint64_t timestamp);

/**
 * @brief Add a Boolean data to a structure data instance
 *
 * @param structure Specify a structure data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @return int32_t
 */
int32_t tm_data_struct_set_bool(void *structure, const int8_t *name,
                                boolean val);
int32_t tm_data_struct_get_bool(void *structure, const int8_t *name,
                                boolean *val);

/**
 * @brief Add an enumeration type of data to a struct data instance
 *
 * @param structure Specify a structure data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @return int32_t
 */
int32_t tm_data_struct_set_enum(void *structure, const int8_t *name,
                                int32_t val);
int32_t tm_data_struct_get_enum(void *structure, const int8_t *name,
                                int32_t *val);

/**
 * @brief To the structure data instance add a 32Bit integer type data
 *
 * @param structure Specify a structure data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @return int32_t
 */
int32_t tm_data_struct_set_int32(void *structure, const int8_t *name,
                                 int32_t val);
int32_t tm_data_struct_get_int32(void *structure, const int8_t *name,
                                 int32_t *val);

/**
 * @brief To the structure data instance add a 64 bit integer type data
 *
 * @param structure Specify a structure data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @return int32_t
 */
int32_t tm_data_struct_set_int64(void *structure, const int8_t *name,
                                 int64_t val);
int32_t tm_data_struct_get_int64(void *structure, const int8_t *name,
                                 int64_t *val);

/**
 * @brief Add a time-type data to a structure data instance
 *
 * @param structure Specify data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values，UTC Timestamp，Unit is ms
 * @return int32_t
 */
int32_t tm_data_struct_set_date(void *structure, const int8_t *name,
                                int64_t val);
int32_t tm_data_struct_get_date(void *structure, const int8_t *name,
                                int64_t *val);

/**
 * @brief Add a single-precision floating-point number type data to a structure data instance
 *
 * @param structure Specify a structure data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @return int32_t
 */
int32_t tm_data_struct_set_float(void *structure, const int8_t *name,
                                 float32_t val);
int32_t tm_data_struct_get_float(void *structure, const int8_t *name,
                                 float32_t *val);

/**
 * @brief Add a double-precision floating-point type data to a structure data instance
 *
 * @param structure Specify a structure data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @return int32_t
 */
int32_t tm_data_struct_set_double(void *structure, const int8_t *name,
                                  float64_t val);
int32_t tm_data_struct_get_double(void *structure, const int8_t *name,
                                  float64_t *val);

uint8_t *tm_data_to_hexstr(const uint8_t *bin, uint32_t size);

int32_t tm_data_to_bin(const uint8_t *hex_str, uint8_t **bin, uint32_t *size);

/**
 * @brief Add a bitmap type data to a structure data instance
 *
 * @param structure Specify a structure data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @return int32_t
 */
int32_t tm_data_struct_set_bitmap(void *structure, const int8_t *name,
                                  uint32_t val);
int32_t tm_data_struct_get_bitmap(void *structure, const int8_t *name,
                                  uint32_t *val);

/**
 * @brief Add a string type data to a structure data instance
 *
 * @param structure Specify a structure data instance
 * @param name Specify the data identity to be added
 * @param val Specify data values
 * @return int32_t
 */
int32_t tm_data_struct_set_string(void *structure, const int8_t *name,
                                  int8_t *val);
int32_t tm_data_struct_get_string(void *structure, const int8_t *name,
                                  int8_t **val);

int32_t tm_data_struct_set_data(void *structure, const int8_t *name, void *val);
int32_t tm_data_struct_get_data(void *structure, const int8_t *name,
                                void **val);

int32_t tm_data_list_each(void *data, tm_list_cb callback);

int32_t tm_data_array_size(void *array);
void *tm_data_array_get_element(void *array, uint32_t index);

/// @brief Load buffer Data Type
/// @param structure OneJSON Handle
/// @param name Node name
/// @param val Binary array address
/// @param size Binary array length
/// @return 0:Succeed;Other:Failed
int32_t tm_data_struct_set_buffer(void *structure, const uint8_t *name,
                                  uint8_t *val, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
