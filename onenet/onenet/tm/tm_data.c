/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file tm_data.c
 * @brief Thing Model data process,based on json
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "tm_data.h"
#include "err_def.h"
#include "tm_onejson.h"

#include "plat_osl.h"
/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/

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
void *tm_data_create() { return tm_onejson_create_data(); }

void *tm_data_struct_create(void) { return tm_onejson_create_struct(); }

void *tm_data_array_create(uint32_t size) {
  return tm_onejson_create_array(size);
}

void tm_data_delete(void *data) { tm_onejson_delete_data(data); }

int32_t tm_data_array_set_bool(void *array, boolean val) {
  return tm_onejson_pack_bool(array, NULL, val);
}

int32_t tm_data_array_set_enum(void *array, int32_t val) {
  return tm_onejson_pack_number(array, NULL, val);
}

int32_t tm_data_array_set_int32(void *array, int32_t val) {
  return tm_onejson_pack_number(array, NULL, val);
}

int32_t tm_data_array_set_int64(void *array, int64_t val) {
  return tm_onejson_pack_number(array, NULL, val);
}

int32_t tm_data_array_set_date(void *array, int64_t val) {
  return tm_onejson_pack_number(array, NULL, val);
}

int32_t tm_data_array_set_float(void *array, float32_t val) {
  return tm_onejson_pack_float32(array, NULL, val);
}

int32_t tm_data_array_set_double(void *array, float64_t val) {
  return tm_onejson_pack_number(array, NULL, val);
}

int32_t tm_data_array_set_string(void *array, uint8_t *val) {
  return tm_onejson_pack_string(array, NULL, val);
}

int32_t tm_data_array_set_bitmap(void *array, int32_t val) {
  return tm_onejson_pack_number(array, NULL, val);
}

int32_t tm_data_array_set_struct(void *array, void *val) {
  return tm_onejson_pack_struct(array, NULL, val);
}

int32_t tm_data_set_bool(void *data, const int8_t *name, boolean val,
                         uint64_t timestamp) {
  return tm_onejson_pack_bool_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_set_enum(void *data, const int8_t *name, int32_t val,
                         uint64_t timestamp) {
  return tm_onejson_pack_number_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_set_int32(void *data, const int8_t *name, int32_t val,
                          uint64_t timestamp) {
  return tm_onejson_pack_number_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_set_int64(void *data, const int8_t *name, int64_t val,
                          uint64_t timestamp) {
  return tm_onejson_pack_number_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_set_date(void *data, const int8_t *name, int64_t val,
                         uint64_t timestamp) {
  return tm_onejson_pack_number_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_set_float(void *data, const int8_t *name, float32_t val,
                          uint64_t timestamp) {
  return tm_onejson_pack_float32_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_set_double(void *data, const int8_t *name, float64_t val,
                           uint64_t timestamp) {
  return tm_onejson_pack_number_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_set_bitmap(void *data, const int8_t *name, uint32_t val,
                           uint64_t timestamp) {
  return tm_onejson_pack_number_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_set_string(void *data, const int8_t *name, int8_t *val,
                           uint64_t timestamp) {
  return tm_onejson_pack_string_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_set_data(void *data, const int8_t *name, void *val,
                         uint64_t timestamp) {
  return tm_onejson_pack_struct_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_get_data(void *data, const int8_t *name, void **val) {
  *val = tm_onejson_get_data_by_name(data, name);
  return 0;
}

int32_t tm_data_set_struct(void *data, const int8_t *name, void *val,
                           uint64_t timestamp) {
  return tm_onejson_pack_struct_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_set_array(void *data, const int8_t *name, void *val,
                          uint64_t timestamp) {
  return tm_onejson_pack_struct_with_timestamp(data, name, val, timestamp);
}

int32_t tm_data_get_bool(void *data, boolean *val) {
  tm_onejson_parse_bool(data, val);
  return 0;
}

int32_t tm_data_get_int32(void *data, int32_t *val) {
  float64_t num = 0;

  tm_onejson_parse_number(data, &num);
  *val = (int32_t)num;
  return 0;
}

int32_t tm_data_get_enum(void *data, int32_t *val) {
  tm_data_get_int32(data, val);
  return 0;
}

int32_t tm_data_get_int64(void *data, int64_t *val) {
  float64_t num = 0;

  tm_onejson_parse_number(data, &num);
  *val = (int64_t)num;
  return 0;
}

int32_t tm_data_get_date(void *data, int64_t *val) {
  return tm_data_get_int64(data, val);
}

int32_t tm_data_get_float(void *data, float32_t *val) {
  float64_t num = 0;

  tm_onejson_parse_number(data, &num);
  *val = (float32_t)num;
  return 0;
}

int32_t tm_data_get_double(void *data, float64_t *val) {
  tm_onejson_parse_number(data, val);
  return 0;
}

int32_t tm_data_get_bitmap(void *data, uint32_t *val) {
  return tm_data_get_int32(data, (int32_t *)val);
}

int32_t tm_data_get_string(void *data, int8_t **val) {
  return tm_onejson_parse_string(data, val);
}

int32_t tm_data_struct_set_bool(void *structure, const int8_t *name,
                                boolean val) {
  return tm_onejson_pack_bool(structure, name, val);
}

int32_t tm_data_struct_set_enum(void *structure, const int8_t *name,
                                int32_t val) {
  return tm_onejson_pack_number(structure, name, val);
}



int32_t tm_data_struct_set_int32(void *structure, const int8_t *name,
                                 int32_t val) {
  return tm_onejson_pack_number(structure, name, val);
}

int32_t tm_data_struct_set_int64(void *structure, const int8_t *name,
                                 int64_t val) {
  return tm_onejson_pack_number(structure, name, val);
}

int32_t tm_data_struct_set_date(void *structure, const int8_t *name,
                                int64_t val) {
  return tm_onejson_pack_number(structure, name, val);
}

int32_t tm_data_struct_set_float(void *structure, const int8_t *name,
                                 float32_t val) {
  return tm_onejson_pack_float32(structure, name, val);
}

int32_t tm_data_struct_set_double(void *structure, const int8_t *name,
                                  float64_t val) {
  return tm_onejson_pack_number(structure, name, val);
}

int32_t tm_data_struct_set_bitmap(void *structure, const int8_t *name,
                                  uint32_t val) {
  return tm_onejson_pack_number(structure, name, val);
}

int32_t tm_data_struct_set_string(void *structure, const int8_t *name,
                                  int8_t *val) {
  return tm_onejson_pack_string(structure, name, val);
}

int32_t tm_data_struct_set_data(void *structure, const int8_t *name,
                                void *val) {
  return tm_onejson_pack_struct(structure, name, val);
}

int32_t tm_data_struct_get_data(void *structure, const int8_t *name,
                                void **val) {
  *val = tm_onejson_get_data_by_name(structure, name);
  return 0;
}

int32_t tm_data_struct_get_bool(void *structure, const int8_t *name,
                                boolean *val) {
  void *data = tm_onejson_get_data_by_name(structure, name);

  if (data) {
    return tm_data_get_bool(data, val);
  } else {
    return ERR_INVALID_DATA;
  }
}

int32_t tm_data_struct_get_int32(void *structure, const int8_t *name,
                                 int32_t *val) {
  void *data = tm_onejson_get_data_by_name(structure, name);

  if (data) {
    return tm_data_get_int32(data, val);
  } else {
    return ERR_INVALID_DATA;
  }
}

int32_t tm_data_struct_get_enum(void *structure, const int8_t *name,
                                int32_t *val) {
  return tm_data_struct_get_int32(structure, name, val);
}

int32_t tm_data_struct_get_int64(void *structure, const int8_t *name,
                                 int64_t *val) {
  void *data = tm_onejson_get_data_by_name(structure, name);

  if (data) {
    return tm_data_get_int64(data, val);
  } else {
    return ERR_INVALID_DATA;
  }
}

int32_t tm_data_struct_get_date(void *structure, const int8_t *name,
                                int64_t *val) {
  return tm_data_struct_get_int64(structure, name, val);
}

int32_t tm_data_struct_get_float(void *structure, const int8_t *name,
                                 float32_t *val) {
  void *data = tm_onejson_get_data_by_name(structure, name);

  if (data) {
    return tm_data_get_float(data, val);
  } else {
    return ERR_INVALID_DATA;
  }
}

int32_t tm_data_struct_get_double(void *structure, const int8_t *name,
                                  float64_t *val) {
  void *data = tm_onejson_get_data_by_name(structure, name);

  if (data) {
    return tm_data_get_double(data, val);
  } else {
    return ERR_INVALID_DATA;
  }
}

int32_t tm_data_struct_get_bitmap(void *structure, const int8_t *name,
                                  uint32_t *val) {
  return tm_data_struct_get_int32(structure, name, (int32_t *)val);
}

int32_t tm_data_struct_get_string(void *structure, const int8_t *name,
                                  int8_t **val) {
  void *data = tm_onejson_get_data_by_name(structure, name);

  if (data) {
    return tm_data_get_string(data, val);
  } else {
    return ERR_INVALID_DATA;
  }
}

int32_t tm_data_list_each(void *data, tm_list_cb callback) {
  return tm_onejson_list_each(data, callback);
}

int32_t tm_data_array_size(void *array) {
  return tm_onejson_get_array_size(array);
}

void *tm_data_array_get_element(void *array, uint32_t index) {
  return tm_onejson_get_array_element_by_index(array, index);
}

int32_t tm_data_get_buffer(void *data, uint8_t **val, uint32_t *size) {
  uint8_t *hex_str = 0;
  int ret = 0;

  tm_data_get_string(data, (int8_t **)&hex_str);

  ret = tm_data_to_bin(hex_str, val, size);

  return ret;
}

int32_t tm_data_set_buffer(void *data, const uint8_t *name, uint8_t *val,
                           uint32_t size, uint64_t timestamp) {
  int ret = 0;
  uint8_t *hex_str = tm_data_to_hexstr(val, size);
  ret = tm_onejson_pack_string_with_timestamp(data, name, hex_str, timestamp);
  osl_free(hex_str);
  return ret;
}

int32_t tm_data_struct_set_buffer(void *structure, const uint8_t *name,
                                  uint8_t *val, uint32_t size) {
  int ret = 0;
  uint8_t *hex_str = tm_data_to_hexstr(val, size);

  ret = tm_onejson_pack_string(structure, name, hex_str);

  osl_free(hex_str);

  return ret;
}

uint8_t *tm_data_to_hexstr(const uint8_t *bin, uint32_t size) {
  uint32_t i = 0, j = 0;
  uint8_t *out = (uint8_t *)osl_malloc(2 * size + 1);
  osl_memset(out, 0, 2 * size + 1);
  for (i = 0; i < size; i++) {
    uint8_t c_temp = bin[i];
    for (j = 0; j < 2; j++) {
      uint8_t c_cur = (c_temp & 0x0f);
      c_cur += ((c_cur < 10) ? '0' : ('A' - 10));
      out[2 * i + 1 - j] = c_cur;
      c_temp >>= 4;
    }
  }
  out[2 * size] = '\0';
  return out;
}

int32_t tm_data_to_bin(const uint8_t *hex_str, uint8_t **bin, uint32_t *size) {
  uint32_t i = 0, j = 0;
  uint32_t hex_len = osl_strlen(hex_str);
  if (hex_len % 2 != 0)
    return ERR_INVALID_PARAM;
  *size = hex_len / 2;

  *bin = (uint8_t *)osl_malloc(*size);

  osl_memset(*bin, 0, *size);

  for (i = 0; i < *size; i++) {

    uint8_t c_temp = 0;
    for (j = 0; j < 2; j++) {
      char c_cur = hex_str[2 * i + j];
      if (c_cur >= '0' && c_cur <= '9')
        c_temp = (c_temp << 4) + (c_cur - '0');
      else if (c_cur >= 'a' && c_cur <= 'f')
        c_temp = (c_temp << 4) + (c_cur - 'a' + 10);
      else if (c_cur >= 'A' && c_cur <= 'F')
        c_temp = (c_temp << 4) + (c_cur - 'A' + 10);
      else {
        osl_free(*bin);
        *bin = NULL;
        return ERR_INVALID_PARAM;
      }
    }
    (*bin)[i] = c_temp;
  }

  return ERR_OK;
}
