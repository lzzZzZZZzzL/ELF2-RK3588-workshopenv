/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file tm_onejson.c
 * @brief OneJson process based on cJSON
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "cJSON.h"

#include "common.h"
#include "err_def.h"
#include "plat_osl.h"
#include "tm_onejson.h"

#include <stdio.h>

#define SDK_TM_VERSION "1.0"
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
void *tm_onejson_create_data(void) { return (void *)cJSON_CreateObject(); }

void *tm_onejson_create_array(uint32_t size) {
  return (void *)cJSON_CreateArray();
}

void *tm_onejson_create_struct(void) { return (void *)cJSON_CreateObject(); }

void tm_onejson_delete_data(void *data) { cJSON_Delete((cJSON *)data); }

static float64_t float32_to_float64(float32_t val) {
  uint8_t tmp_val1[16] = {0};
  float64_t tmp_val2 = 0;

  osl_sprintf(tmp_val1, (const uint8_t *)"%lg", val);
  osl_sscanf(tmp_val1, (const uint8_t *)"%lf", &tmp_val2);
  return tmp_val2;
}

static int32_t set_value_with_timestamp(cJSON *data, const int8_t *name,
                                        cJSON *value, int64_t ts_in_ms) {
  cJSON *sub = NULL;
  cJSON *target = cJSON_GetObjectItem(data, (const char *const)name);
  cJSON *value_item, *time_item = NULL;

  if (target) {
    if (ts_in_ms) {
      time_item = cJSON_GetObjectItem(target, "time");
      if (time_item) {
        cJSON_SetIntValue(time_item, ts_in_ms);
      } else {
        cJSON_AddItemToObject(target, "time", cJSON_CreateNumber(ts_in_ms));
      }
    }

    value_item = cJSON_GetObjectItem(target, "value");
    if (cJSON_Array != value_item->type) {
      cJSON *item = cJSON_DetachItemFromObject(target, "value");
      cJSON *array = cJSON_CreateArray();
      cJSON_AddItemToArray(array, item);
      cJSON_AddItemToArray(array, value);
      cJSON_AddItemToObject(target, "value", array);
    } else {
      cJSON_AddItemToArray(value_item, value);
    }
  } else {
    sub = cJSON_CreateObject();
    cJSON_AddItemToObject(sub, "value", value);
    if (ts_in_ms) {
      cJSON_AddItemToObject(sub, "time", cJSON_CreateNumber(ts_in_ms));
    }

    if (cJSON_IsArray(data)) {
      cJSON_AddItemToArray(data, sub);
    } else {
      cJSON_AddItemToObject(data, (const char *)name, sub);
    }
  }

  return ERR_OK;
}

static int32_t set_value(cJSON *data, const int8_t *name, cJSON *value) {
  if (cJSON_IsArray(data)) {
    if (value) {
      cJSON_AddItemToArray(data, value);
    }
  } else {
    if (value) {
      cJSON_AddItemToObject(data, (const char *)name, value);
    } else {
      cJSON_AddObjectToObject(data, (const char *const)name);
    }
  }

  return ERR_OK;
}

int32_t tm_onejson_pack_bool_with_timestamp(void *data, const int8_t *name,
                                            boolean val, int64_t ts_in_ms) {
  return set_value_with_timestamp((cJSON *)data, name, cJSON_CreateBool(val),
                                  ts_in_ms);
}

int32_t tm_onejson_pack_number_with_timestamp(void *data, const int8_t *name,
                                              float64_t val, int64_t ts_in_ms) {
  return set_value_with_timestamp((cJSON *)data, name, cJSON_CreateNumber(val),
                                  ts_in_ms);
}

int32_t tm_onejson_pack_float32_with_timestamp(void *data, const int8_t *name,
                                               float32_t val,
                                               int64_t ts_in_ms) {
  return set_value_with_timestamp((cJSON *)data, name,
                                  cJSON_CreateNumber(float32_to_float64(val)),
                                  ts_in_ms);
}

int32_t tm_onejson_pack_string_with_timestamp(void *data, const int8_t *name,
                                              int8_t *val, int64_t ts_in_ms) {
  return set_value_with_timestamp(
      (cJSON *)data, name, cJSON_CreateString((const char *)val), ts_in_ms);
}

int32_t tm_onejson_pack_struct_with_timestamp(void *data, const int8_t *name,
                                              void *val, int64_t ts_in_ms) {
  return set_value_with_timestamp((cJSON *)data, name, (cJSON *)val, ts_in_ms);
}

int32_t tm_onejson_pack_bool(void *data, const int8_t *name, boolean val) {
  return set_value((cJSON *)data, name, cJSON_CreateBool(val));
}
int32_t tm_onejson_pack_number(void *data, const int8_t *name, float64_t val) {
  return set_value((cJSON *)data, name, cJSON_CreateNumber(val));
}

int32_t tm_onejson_pack_float32(void *data, const int8_t *name, float32_t val) {
  return set_value((cJSON *)data, name,
                   cJSON_CreateNumber(float32_to_float64(val)));
}

int32_t tm_onejson_pack_string(void *data, const int8_t *name, int8_t *val) {
  return set_value((cJSON *)data, name, cJSON_CreateString((const char *)val));
}

int32_t tm_onejson_pack_struct(void *data, const int8_t *name, void *val) {
  return set_value((cJSON *)data, name, (cJSON *)val);
}

int32_t tm_onejson_get_array_size(void *array) {
  return cJSON_GetArraySize((cJSON *)array);
}

void *tm_onejson_get_array_element_by_index(void *array, uint32_t index) {
  return (void *)cJSON_GetArrayItem((cJSON *)array, index);
}

void *tm_onejson_get_data_by_name(void *data, const int8_t *name) {
  return (void *)cJSON_GetObjectItem((cJSON *)data, (const char *const)name);
}

int32_t tm_onejson_list_each(void *data, tm_list_cb callback) {
  cJSON *obj = (cJSON *)data;
  cJSON *item = obj->child;
  int32_t ret = ERR_OK;

  while (item) {
    if (ERR_OK !=
        (ret = callback((const uint8_t *)item->string, (void *)item))) {
      break;
    }
    item = item->next;
  }
  return ret;
}

static cJSON *get_value_item(cJSON *obj) {
  if (cJSON_IsObject(obj)) {
    return cJSON_GetObjectItem(obj, "value");
  } else {
    return obj;
  }
}

int32_t tm_onejson_parse_bool(void *data, boolean *val) {
  cJSON *obj = (cJSON *)data;

  if ((NULL == data) || (NULL == val))
    return ERR_INVALID_PARAM;

  if (cJSON_True == obj->type) {
    *val = 1;
  } else {
    *val = 0;
  }

  return ERR_OK;
}

int32_t tm_onejson_parse_number(void *data, float64_t *val) {
  cJSON *obj = (cJSON *)data;

  if ((NULL == data) || (NULL == val))
    return ERR_INVALID_PARAM;

  *val = get_value_item(obj)->valuedouble;

  return ERR_OK;
}

int32_t tm_onejson_parse_string(void *data, int8_t **val) {
  cJSON *obj = (cJSON *)data;

  if ((NULL == data) || (NULL == val))
    return ERR_INVALID_PARAM;

  *val = (uint8_t *)(get_value_item(obj)->valuestring);

  return ERR_OK;
}

void *tm_onejson_pack_props_and_events(void *data, const uint8_t *product_id,
                                       const uint8_t *dev_name, void *props,
                                       void *events, uint8_t as_raw) {
  cJSON *packed_data = (cJSON *)data;
  cJSON *ele = NULL;
  cJSON *identity = NULL;

  if (NULL == data) {
    packed_data = cJSON_CreateArray();
  }

  if (cJSON_Array != packed_data->type) {
    return NULL;
  }

  ele = cJSON_CreateObject();
  if (as_raw) {
    cJSON_AddRawToObject(ele, "properties", props);
    cJSON_AddRawToObject(ele, "events", events);
  } else {
    cJSON_AddItemToObject(ele, "properties", props);
    cJSON_AddItemToObject(ele, "events", events);
  }

  identity = cJSON_CreateObject();
  cJSON_AddStringToObject(identity, "productID", (const char *const)product_id);
  cJSON_AddStringToObject(identity, "deviceName", (const char *const)dev_name);
  cJSON_AddItemToObject(ele, "identity", identity);

  cJSON_AddItemToArray(packed_data, ele);

  return packed_data;
}

uint32_t tm_onejson_pack_request(uint8_t *payload, int32_t msg_id, void *params,
                                 uint8_t as_raw) {
  cJSON *request = cJSON_CreateObject();
  uint8_t *str = NULL;
  uint8_t temp_id[16] = {0};

  osl_sprintf(temp_id, (const uint8_t *)"%d", msg_id);

  cJSON_AddStringToObject(request, "id", (const char *const)temp_id);

  cJSON_AddStringToObject(request, "version", SDK_TM_VERSION);

  if (params) {
    if (as_raw)
      cJSON_AddRawToObject(request, "params", params);
    else
      cJSON_AddItemToObject(request, "params", (cJSON *)params);
  }

  // Add msg To OneJSON
  if (NULL != (str = (uint8_t *)cJSON_PrintUnformatted(request))) {
    ADD_MSG2PAYLOAD(payload, str);
    SAFE_FREE(str);
  }
  cJSON_Delete(request);
  return osl_strlen(payload);
}

void *tm_onejson_parse_request(uint8_t *payload, uint32_t payload_len,
                               uint8_t *msg_id, uint8_t as_raw) {
  cJSON *root = NULL;
  cJSON *item = NULL;
  void *params = NULL;

  root = cJSON_ParseWithLength((const char *)payload, payload_len);
  item = cJSON_GetObjectItem(root, "id");

  if (root != NULL && item != NULL) {
    osl_strcpy(msg_id, (const uint8_t *)item->valuestring);
    if (as_raw) {
      item = cJSON_GetObjectItem(root, "params");
      params = cJSON_PrintUnformatted(item);
    } else {
      params = cJSON_DetachItemFromObject(root, "params");
    }
  }

  if (root != NULL) {
    cJSON_Delete(root);
  }

  return params;
}

uint32_t tm_onejson_pack_reply(uint8_t *payload, uint8_t *msg_id,
                               int32_t msg_code, void *data, uint8_t as_raw) {
  cJSON *reply = cJSON_CreateObject();
  uint8_t *temp_str = NULL;

  cJSON_AddStringToObject(reply, "id", (const char *const)msg_id);
  cJSON_AddNumberToObject(reply, "code", msg_code);
  // TODO: Add msg Fields and test
  // cJSON_AddStringToObject(reply, "msg", "succ");

  if (data) {
    if (as_raw) {
      cJSON_AddRawToObject(reply, "data", data);
    } else {
      cJSON_AddItemToObject(reply, "data", (cJSON *)data);
    }
  }

  if (NULL != (temp_str = (uint8_t *)cJSON_PrintUnformatted(reply))) {
    osl_strcpy(payload, temp_str);
    osl_free(temp_str);
  }

  cJSON_Delete(reply);

  return osl_strlen(payload);
}

void *tm_onejson_parse_reply(uint8_t *payload, uint32_t payload_len,
                             uint8_t *msg_id, int32_t *msg_code,
                             uint8_t as_raw) {
  cJSON *root = NULL;
  cJSON *item = NULL;
  void *data = NULL;

  root = cJSON_ParseWithLength((const char *)payload, payload_len);
  item = cJSON_GetObjectItem(root, "id");
  *msg_code = cJSON_GetObjectItem(root, "code")->valueint;

  if (root != NULL && item != NULL) {
    osl_strcpy(msg_id, (const uint8_t *)item->valuestring);
    if (as_raw) {
      item = cJSON_GetObjectItem(root, "data");
      if (NULL != item) {
        data = cJSON_PrintUnformatted(item);
      }
    } else {
      data = cJSON_DetachItemFromObject(root, "data");
    }
  }

  if (root != NULL) {
    cJSON_Delete(root);
  }

  return data;
}

int tm_onejson_parse_method(uint8_t *payload, uint32_t payload_len,
                            uint8_t *method) {
  cJSON *root = NULL;
  cJSON *item = NULL;
  int ret = ERR_INVALID_DATA;

  root = cJSON_ParseWithLength((const char *)payload, payload_len);
  CHECK_EXPR_GOTO(root == NULL, _END, "cJSON_ParseWithLength failed");

  item = cJSON_GetObjectItem(root, "method");
  if (item != NULL && item->type == cJSON_String && item->valuestring != NULL) {
    osl_strcpy(method, (const uint8_t *)item->valuestring);
    ret = ERR_OK;
  }

_END:
  cJSON_Delete(root);

  return ret;
}
