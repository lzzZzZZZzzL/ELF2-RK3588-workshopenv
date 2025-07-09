/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file tm_api.c
 * @brief Thing Model API for multi-protocols
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "tm_api.h"
#include "dev_token.h"
#include "err_def.h"
#include "log.h"
#include "plat_osl.h"
#include "plat_time.h"
#include "tm_data.h"
#include "tm_onejson.h"

#include <stdio.h>
#include <string.h>

#if defined(SDK_USE_MQTTS)
#include "tm_mqtt.h"
#elif defined(SDK_USE_COAP)
#include "tm_coap.h"
#elif defined(SDK_USE_NBIOT)
#include "tm_lwm2m.h"
#endif

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#if defined(SDK_USE_NBIOT)
#define TM_TOPIC_PREFIX "thing"

#define TM_TOPIC_PROP_POST ".property.post"
#define TM_TOPIC_PROP_SET ".property.set"
#define TM_TOPIC_PROP_GET ".property.get"
#define TM_TOPIC_EVENT_POST ".event.post"
#define TM_TOPIC_DESIRED_PROPS_GET  ".property.desired.get"
// Not supportted by LwM2M
#define TM_TOPIC_DESIRED_PROPS_DELETE  ".property.desired.delete"
// Not supportted by LwM2M
#define TM_TOPIC_SERVICE_INVOKE  ".service.%s.invoke"
// Not supportted by LwM2M
#define TM_TOPIC_PACK_DATA_POST ".pack.post"
#define TM_TOPIC_HISTORY_DATA_POST ".history.post"

#define TM_TOPIC_PROP_POST_REPLY ".property.post.reply"
#define TM_TOPIC_PROP_SET_REPLY ".property.set.reply"
#define TM_TOPIC_PROP_GET_REPLY ".property.get.reply"
#define TM_TOPIC_EVENT_POST_REPLY  ".event.post.reply"
// Not supportted by LwM2M
#define TM_TOPIC_DESIRED_PROPS_GET_REPLY  ".property.desired.get.reply"
// Not supportted by LwM2M
#define TM_TOPIC_DESIRED_PROPS_DELETE_REPLY  ".property.desired.delete.reply"
// Not supportted by LwM2M
#define TM_TOPIC_SERVICE_INVOKE_REPLY ".service.%s.reply"
#define TM_TOPIC_PACK_DATA_POST_REPLY  ".pack.post.reply"
// Not supportted by LwM2M
#define TM_TOPIC_HISTORY_DATA_POST_REPLY  ".history.post.reply"
// Not supportted by LwM2M
#else
#define TM_TOPIC_PREFIX "$sys/%s/%s/thing"

#define TM_TOPIC_PROP_POST "/property/post"
#define TM_TOPIC_PROP_SET "/property/set"
#define TM_TOPIC_PROP_GET "/property/get"
#define TM_TOPIC_EVENT_POST "/event/post"
#define TM_TOPIC_DESIRED_PROPS_GET "/property/desired/get"
#define TM_TOPIC_DESIRED_PROPS_DELETE "/property/desired/delete"
#define TM_TOPIC_SERVICE_INVOKE "/service/%s/invoke"
#define TM_TOPIC_PACK_DATA_POST "/pack/post"
#define TM_TOPIC_HISTORY_DATA_POST "/history/post"

#define TM_TOPIC_PROP_POST_REPLY "/property/post/reply"
#define TM_TOPIC_PROP_SET_REPLY "/property/set_reply"
#define TM_TOPIC_PROP_GET_REPLY "/property/get_reply"
#define TM_TOPIC_EVENT_POST_REPLY "/event/post/reply"
#define TM_TOPIC_DESIRED_PROPS_GET_REPLY "/property/desired/get/reply"
#define TM_TOPIC_DESIRED_PROPS_DELETE_REPLY "/property/desired/delete/reply"
#define TM_TOPIC_SERVICE_INVOKE_REPLY "/service/%s/invoke_reply"
#define TM_TOPIC_PACK_DATA_POST_REPLY "/pack/post/reply"
#define TM_TOPIC_HISTORY_DATA_POST_REPLY "/history/post/reply"
#endif
/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
#define REPLY_STATUS_NONE 0
#define REPLY_STATUS_WAIT 1
#define REPLY_STATUS_RECEIVED 2
struct tm_reply_info_t {
  uint8_t reply_id[16];
  int32_t reply_code;
  void *reply_data;
  uint8_t resp_flag;
  uint16_t reply_status;
  uint8_t reply_as_raw;
};

struct tm_obj_t {
  struct tm_downlink_tbl_t downlink_tbl;
  uint8_t *topic_prefix;
  int32_t post_id;
  struct tm_reply_info_t reply_info;
#ifdef CONFIG_TM_GATEWAY 
  tm_subdev_cb subdev_callback;
#endif
};

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
struct tm_obj_t g_tm_obj;

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
static uint8_t *construct_topic(const uint8_t *prefix, const uint8_t *suffix) {
  logd("construct_topic prefix %s, suffix %s\n", prefix, suffix);
  uint32_t topic_len = osl_strlen(prefix) + osl_strlen(suffix) + 1;
  uint8_t *topic_buf = osl_malloc(topic_len);

  if (topic_buf) {
    osl_memset(topic_buf, 0, topic_len);
    osl_strcat(topic_buf, prefix);
    osl_strcat(topic_buf, suffix);
  }

  return topic_buf;
}

static int32_t get_post_id(void) {
  if (0x7FFFFFFF == ++(g_tm_obj.post_id)) {
    g_tm_obj.post_id = 1;
  }

  return g_tm_obj.post_id;
}

int32_t tm_send_response(const uint8_t *name, uint8_t *msg_id, int32_t msg_code,
                         uint8_t as_raw, void *resp_data,
                         uint32_t resp_data_len, uint32_t timeout_ms) {
#if defined(SDK_USE_MQTTS) || defined(SDK_USE_NBIOT)
  uint8_t *topic = NULL;
#endif
  uint8_t *payload = NULL;
  uint32_t payload_len = 0;

  if (NULL == (payload = osl_malloc(SDK_PAYLOAD_LEN))) {
    return ERR_IO;
  }

  osl_memset(payload, 0, SDK_PAYLOAD_LEN);
  payload_len =
      tm_onejson_pack_reply(payload, msg_id, msg_code, resp_data, as_raw);
#if defined(SDK_USE_MQTTS)
  topic = construct_topic(g_tm_obj.topic_prefix, name);
  tm_mqtt_send_packet(topic, payload, payload_len, timeout_ms);
  osl_free(topic);
#elif defined(SDK_USE_COAP)
  tm_coap_send_packet(NULL, payload, payload_len, timeout_ms);
#elif defined(SDK_USE_NBIOT)
  topic = construct_topic(g_tm_obj.topic_prefix, name);
  tm_lwm2m_send_packet(topic, payload, payload_len, timeout_ms);
  osl_free(topic);
#endif

  osl_free(payload);

  return ERR_OK;
}

#if defined(SDK_USE_MQTTS)
static int32_t wait_post_reply(int32_t post_id, handle_t cd_hdl) {
  uint8_t temp_id[16] = {0};
  int32_t ret = ERR_TIMEOUT;

  osl_sprintf(temp_id, (const uint8_t *)"%d", post_id);

  do {
    ret = tm_mqtt_step(countdown_left(cd_hdl));

    if (0 > ret) {
      loge("wait reply error");
      break;
    } else {
      if (REPLY_STATUS_RECEIVED == g_tm_obj.reply_info.reply_status) {
        if (0 == osl_strcmp(temp_id, g_tm_obj.reply_info.reply_id)) {
          g_tm_obj.reply_info.reply_status = REPLY_STATUS_NONE;
          ret = ERR_OK;
          break;
        } else {
          g_tm_obj.reply_info.reply_status = REPLY_STATUS_WAIT;
        }
      }
    }
  } while (0 == countdown_is_expired(cd_hdl));

  return ret;
}
#endif

int32_t tm_send_request(const uint8_t *name, uint8_t as_raw, void *data,
                        uint32_t data_len, void **reply_data,
                        uint32_t *reply_data_len, uint32_t timeout_ms) {
  uint8_t *topic = NULL;
  uint8_t *payload = NULL;
  uint32_t payload_len = 0;
  int32_t post_id = get_post_id();
  handle_t cd_hdl = 0;
  int32_t ret = ERR_OTHERS;

  cd_hdl = countdown_start(timeout_ms);

  if (NULL == (payload = osl_malloc(SDK_PAYLOAD_LEN))) {
    return ERR_IO;
  }

  osl_memset(payload, 0, SDK_PAYLOAD_LEN);
  payload_len = tm_onejson_pack_request(payload, post_id, data, as_raw);
  g_tm_obj.reply_info.reply_as_raw = as_raw;

  topic = construct_topic(g_tm_obj.topic_prefix, name);
#if defined(SDK_USE_MQTTS)
  ret =
      tm_mqtt_send_packet(topic, payload, payload_len, countdown_left(cd_hdl));
  if (ERR_OK == ret) {
    g_tm_obj.reply_info.reply_status = REPLY_STATUS_WAIT;
    if (0 == wait_post_reply(post_id, cd_hdl)) {
      // logd("reply err code: %d", g_tm_obj.reply_info.reply_code);
      if (200 == g_tm_obj.reply_info.reply_code) {
        logd("post data ok");
        if (NULL != reply_data) {
          *reply_data = g_tm_obj.reply_info.reply_data;
        }
        ret = ERR_OK;
      } else {
        ret = ERR_OTHERS;
      }
      g_tm_obj.reply_info.reply_data = NULL;
    }
    g_tm_obj.reply_info.reply_code = 0;
    g_tm_obj.reply_info.reply_as_raw = 0;
  }

#elif defined(SDK_USE_COAP)
  ret =
      tm_coap_send_packet(topic, payload, payload_len, countdown_left(cd_hdl));
  if (ERR_OK == ret) {
    logd("tm_send_request ok.");
  } else {
    logd("tm_send_request failed.");
  }
#elif defined(SDK_USE_NBIOT)
  ret =
      tm_lwm2m_send_packet(topic, payload, payload_len, countdown_left(cd_hdl));
  if (ERR_OK == ret) {
    logd("tm_send_request ok.");
  } else {
    logd("tm_send_request failed.");
  }
#endif

  osl_free(topic);
  osl_free(payload);
  countdown_stop(cd_hdl);

  return ret;
}

static int32_t tm_prop_set_handle(const uint8_t *name, void *res) {
  uint16_t i = 0;
  int32_t ret = 0;

  for (i = 0; i < g_tm_obj.downlink_tbl.prop_tbl_size; i++) {
    if (0 == osl_strcmp(name, g_tm_obj.downlink_tbl.prop_tbl[i].name)) {
      ret = g_tm_obj.downlink_tbl.prop_tbl[i].tm_prop_wr_cb(res);
      break;
    }
  }

  return ret;
}

static void tm_prop_set(uint8_t *payload, uint32_t payload_len) {
  void *props_data = NULL;
  uint8_t id[16] = {0};

  props_data = tm_onejson_parse_request(payload, payload_len, id, 0);

  if (NULL != props_data) {
    if (0 == tm_data_list_each(props_data, tm_prop_set_handle)) {
      tm_send_response((const uint8_t *)TM_TOPIC_PROP_SET_REPLY, id, 200, 0,
                       NULL, 0, SDK_REQUEST_TIMEOUT);
    } else {
      tm_send_response((const uint8_t *)TM_TOPIC_PROP_SET_REPLY, id, 100, 0,
                       NULL, 0, SDK_REQUEST_TIMEOUT);
    }

    tm_data_delete(props_data);
  }
}

static void tm_prop_get(uint8_t *payload, uint32_t payload_len) {
  void *props_data = NULL;
  uint8_t *reply_payload = NULL;
  uint8_t id[16] = {0};

  if (NULL == (reply_payload = osl_malloc(SDK_PAYLOAD_LEN))) {
    loge("prop set error, malloc failed.");
    return;
  }
  osl_memset(reply_payload, 0, SDK_PAYLOAD_LEN);

  props_data = tm_onejson_parse_request(payload, payload_len, id, 0);

  if (NULL != props_data) {
    uint32_t i, j = 0;
    uint32_t prop_cnt = tm_data_array_size(props_data);
    uint8_t *prop_name = NULL;
    void *reply_data = tm_data_create();

    for (i = 0; i < prop_cnt; i++) {
      tm_data_get_string(tm_data_array_get_element(props_data, i), &prop_name);
      for (j = 0; j < g_tm_obj.downlink_tbl.prop_tbl_size; j++) {
        if (0 ==
            osl_strcmp(prop_name, g_tm_obj.downlink_tbl.prop_tbl[j].name)) {
          g_tm_obj.downlink_tbl.prop_tbl[j].tm_prop_rd_cb(reply_data);
          break;
        }
      }
    }

    tm_send_response((const uint8_t *)TM_TOPIC_PROP_GET_REPLY, id, 200, 0,
                     reply_data, 0, SDK_REQUEST_TIMEOUT);

    tm_data_delete(props_data);
  }

  osl_free(reply_payload);
}

static void tm_post_reply(uint8_t *payload, uint32_t payload_len) {
  if (REPLY_STATUS_WAIT == g_tm_obj.reply_info.reply_status) {
    g_tm_obj.reply_info.reply_data = tm_onejson_parse_reply(
        payload, payload_len, g_tm_obj.reply_info.reply_id,
        &g_tm_obj.reply_info.reply_code, g_tm_obj.reply_info.reply_as_raw);
    g_tm_obj.reply_info.reply_status = REPLY_STATUS_RECEIVED;
  }
}

#if 0
static int32_t tm_prop_set_desired_handle(const uint8_t *name, void *res)
{
    uint16_t i   = 0;
    int32_t  ret = 0;

    for (i = 0; i < g_tm_obj.downlink_tbl.prop_tbl_size; i++)
    {
        if (0 == osl_strcmp(name, g_tm_obj.downlink_tbl.prop_tbl[i].name))
        {
            ret = g_tm_obj.downlink_tbl.prop_tbl[i].tm_prop_wr_cb(res);
            break;
        }
    }

    return ret;
}
#endif

static void tm_service_invoke(uint8_t *svc_id, uint8_t *payload,
                              uint32_t payload_len) {
  void *svc_data = NULL;
  uint8_t *topic = NULL;
  uint32_t i = 0;
  uint8_t id[16] = {0};

  svc_data = tm_onejson_parse_request(payload, payload_len, id, 0);

  if (NULL != svc_data) {
    void *reply_data = tm_data_struct_create();

    for (i = 0; i < g_tm_obj.downlink_tbl.svc_tbl_size; i++) {
      if (0 == osl_strcmp(svc_id, g_tm_obj.downlink_tbl.svc_tbl[i].name)) {
        g_tm_obj.downlink_tbl.svc_tbl[i].tm_svc_cb(svc_data, reply_data);

        tm_data_delete(svc_data);
#if defined(SDK_USE_MQTTS) || defined(SDK_USE_NBIOT)
        topic = osl_malloc(
            osl_strlen((const uint8_t *)TM_TOPIC_SERVICE_INVOKE_REPLY) +
            osl_strlen(svc_id));
        osl_sprintf(topic, (const uint8_t *)TM_TOPIC_SERVICE_INVOKE_REPLY,
                    svc_id);
#endif
        tm_send_response(topic, id, 200, 0, reply_data, 0, SDK_REQUEST_TIMEOUT);
#if defined(SDK_USE_MQTTS)
        osl_free(topic);
#endif
        break;
      }
    }
  }
}

static int32_t check_action_by_topic(const uint8_t *action,
                                     const uint8_t *topic) {
  return osl_strncmp(action, topic, osl_strlen(topic));
}

static int32_t tm_data_parse(const uint8_t *res_name, uint8_t *payload,
                             uint32_t payload_len) {
  const uint8_t *action = res_name + osl_strlen(g_tm_obj.topic_prefix);

  if (0 == check_action_by_topic(action, (const uint8_t *)TM_TOPIC_PROP_SET)) {
    tm_prop_set(payload, payload_len);
  } else if (0 == check_action_by_topic(action,
                                        (const uint8_t *)TM_TOPIC_PROP_GET)) {
    tm_prop_get(payload, payload_len);
  } else if ((0 == check_action_by_topic(
                       action, (const uint8_t *)TM_TOPIC_PROP_POST)) ||
             (0 == check_action_by_topic(
                       action, (const uint8_t *)TM_TOPIC_EVENT_POST)) ||
             (0 == check_action_by_topic(
                       action, (const uint8_t *)TM_TOPIC_DESIRED_PROPS_GET)) ||
             (0 ==
              check_action_by_topic(
                  action, (const uint8_t *)TM_TOPIC_DESIRED_PROPS_DELETE)) ||
             (0 == check_action_by_topic(
                       action, (const uint8_t *)TM_TOPIC_PACK_DATA_POST)) ||
             (0 == check_action_by_topic(
                       action, (const uint8_t *)TM_TOPIC_HISTORY_DATA_POST))) {
    tm_post_reply(payload, payload_len);
  }
#if defined(SDK_USE_NBIOT)
  else if (0 == check_action_by_topic(action, (const uint8_t *)".service.")) {
    uint8_t svc_id[32] = {0};
    const uint8_t *svc_id_ptr =
        res_name + osl_strlen(g_tm_obj.topic_prefix) + sizeof(".service.") - 1;

    osl_memcpy(svc_id, svc_id_ptr, strlen(svc_id_ptr));
    logd("Service Invoke [%s]", svc_id);

    tm_service_invoke(svc_id, payload, payload_len);
  }
#else
  else if (0 == check_action_by_topic(action, (const uint8_t *)"/service/")) {
    uint8_t svc_id[32] = {0};
    uint8_t *tmp_ptr = NULL;
    const uint8_t *svc_id_ptr =
        res_name + osl_strlen(g_tm_obj.topic_prefix) + 9;

    tmp_ptr = osl_strstr(svc_id_ptr, (const uint8_t *)"/");
    osl_memcpy(svc_id, svc_id_ptr, tmp_ptr - svc_id_ptr);
    logd("Service Invoke [%s]", svc_id);

    tm_service_invoke(svc_id, payload, payload_len);
  }
#endif
#ifdef CONFIG_TM_GATEWAY
  else if (0 == check_action_by_topic(action, (const uint8_t *)"/sub/")) {
    if (g_tm_obj.subdev_callback) {
      g_tm_obj.subdev_callback(action, payload, payload_len);
    }
    if (NULL != osl_strstr(action, (const uint8_t *)"/reply")) {
      tm_post_reply(payload, payload_len);
    }
  }
#endif
  return 0;
}

int32_t tm_init(struct tm_downlink_tbl_t *downlink_tbl) {
  osl_memset(&g_tm_obj, 0, sizeof(g_tm_obj));
  g_tm_obj.downlink_tbl.prop_tbl = downlink_tbl->prop_tbl;
  g_tm_obj.downlink_tbl.prop_tbl_size = downlink_tbl->prop_tbl_size;
  g_tm_obj.downlink_tbl.svc_tbl = downlink_tbl->svc_tbl;
  g_tm_obj.downlink_tbl.svc_tbl_size = downlink_tbl->svc_tbl_size;

#if defined(SDK_USE_MQTTS)
  return tm_mqtt_init(tm_data_parse);
#elif defined(SDK_USE_COAP)
  return tm_coap_init(tm_data_parse);
#elif defined(SDK_USE_NBIOT)
  return tm_lwm2m_init(tm_data_parse);
#endif
}

int32_t tm_deinit(void) {
#if defined(SDK_USE_MQTTS)
  return tm_mqtt_deinit();
#elif defined(SDK_USE_COAP)
  return tm_coap_deinit();
#elif defined(SDK_USE_NBIOT)
  return tm_lwm2m_deinit();
#endif
}

#ifdef CONFIG_TM_GATEWAY
int32_t tm_set_subdev_callback(tm_subdev_cb callback) {
  g_tm_obj.subdev_callback = callback;

  return ERR_OK;
}
#endif

int32_t tm_login(const uint8_t *product_id, const uint8_t *dev_name,
                 const uint8_t *access_key, uint64_t expire_time,
                 uint32_t timeout_ms) {
  int32_t ret = ERR_OK;
  uint8_t dev_token[256] = {0};

  dev_token_generate(dev_token, SIG_METHOD_SHA1, expire_time, product_id,
                     dev_name, access_key);


#if defined(SDK_USE_MQTTS)

#if defined(CONFIG_NETWORK_TLS) && CONFIG_NETWORK_TLS == 1
  logd("Login Info:"
       "\n\t  Product ID : %s"
       "\n\t    Dev Name : %s"
       "\n\t       Token : %s"
       "\n\t Server Addr : %s:%d",
       product_id, dev_name, dev_token, IOT_MQTT_SERVER_ADDR_TLS,
       IOT_MQTT_SERVER_PORT_TLS);
#else
  logd("Login Info:"
       "\n\t  Product ID : %s"
       "\n\t    Dev Name : %s"
       "\n\t       Token : %s"
       "\n\t Server Addr : %s:%d",
       product_id, dev_name, dev_token, IOT_MQTT_SERVER_ADDR,
       IOT_MQTT_SERVER_PORT);
#endif

  ret = tm_mqtt_login(product_id, dev_name, dev_token, timeout_ms);
#elif defined(SDK_USE_COAP)
  ret = tm_coap_login(product_id, dev_name, dev_token, timeout_ms);
#elif defined(SDK_USE_NBIOT)
  ret = tm_lwm2m_login(); // lwm2m协议只使用IMEI验证
#endif

  if (ERR_OK == ret) {
    uint32_t topic_malloc_len = 0;
    uint32_t topic_prefix_len = osl_strlen((const uint8_t *)TM_TOPIC_PREFIX) +
                                osl_strlen(product_id) + osl_strlen(dev_name);
    topic_malloc_len = topic_prefix_len;
    g_tm_obj.topic_prefix = osl_malloc(topic_malloc_len);
    if (NULL == g_tm_obj.topic_prefix) {
#if defined(SDK_USE_MQTTS)
      tm_mqtt_logout(timeout_ms);
#elif defined(SDK_USE_COAP)
      tm_coap_logout(timeout_ms);
#elif defined(SDK_USE_NBIOT)
      return tm_lwm2m_logout(timeout_ms);
#endif
      return ERR_IO;
    }
    osl_memset(g_tm_obj.topic_prefix, 0, topic_malloc_len);
    osl_sprintf(g_tm_obj.topic_prefix, (const uint8_t *)TM_TOPIC_PREFIX,
                product_id, dev_name);
  }

  return ret;
}

int32_t tm_logout(uint32_t timeout_ms) {

  if (g_tm_obj.topic_prefix) {
    osl_free(g_tm_obj.topic_prefix);
    g_tm_obj.topic_prefix = NULL;
  }
#if defined(SDK_USE_MQTTS)
  return tm_mqtt_logout(timeout_ms);
#elif defined(SDK_USE_COAP)
  return tm_coap_logout(timeout_ms);
#elif defined(SDK_USE_NBIOT)
  return tm_lwm2m_logout(timeout_ms);
#endif
}

int32_t tm_post_raw(const uint8_t *name, uint8_t *raw_data,
                    uint32_t raw_data_len, uint8_t **reply_data,
                    uint32_t *reply_data_len, uint32_t timeout_ms) {
  return tm_send_request(name, 1, raw_data, raw_data_len, (void **)reply_data,
                         reply_data_len, timeout_ms);
}

int32_t tm_post_property(void *prop_data, uint32_t timeout_ms) {
  return tm_send_request((const uint8_t *)TM_TOPIC_PROP_POST, 0, prop_data, 0,
                         NULL, NULL, timeout_ms);
}

int32_t tm_post_event(void *event_data, uint32_t timeout_ms) {
  return tm_send_request((const uint8_t *)TM_TOPIC_EVENT_POST, 0, event_data, 0,
                         NULL, NULL, timeout_ms);
}

int32_t tm_get_desired_props(uint32_t timeout_ms) {
  void *prop_list = tm_data_array_create(g_tm_obj.downlink_tbl.prop_tbl_size);
  uint32_t i = 0;
  int32_t ret = ERR_OTHERS;
  void *reply_data = NULL;

  for (i = 0; i < g_tm_obj.downlink_tbl.prop_tbl_size; i++) {
    tm_data_array_set_string(
        prop_list, (uint8_t *)(g_tm_obj.downlink_tbl.prop_tbl[i].name));
  }

  ret = tm_send_request((const uint8_t *)TM_TOPIC_DESIRED_PROPS_GET, 0,
                        prop_list, 0, &reply_data, NULL, timeout_ms);

  if (ERR_OK == ret) {
    tm_data_list_each(reply_data, tm_prop_set_handle);
    logd("get desired props ok");
  }
  if (NULL != reply_data) {
    tm_data_delete(reply_data);
  }

  return ret;
}

int32_t tm_delete_desired_props(uint32_t timeout_ms) {
  void *prop_list = tm_data_create();
  uint32_t i = 0;
  int32_t ret = ERR_OTHERS;

  for (i = 0; i < g_tm_obj.downlink_tbl.prop_tbl_size; i++) {
    tm_data_struct_set_data(
        prop_list, (uint8_t *)(g_tm_obj.downlink_tbl.prop_tbl[i].name), NULL);
  }

  ret = tm_send_request((const uint8_t *)TM_TOPIC_DESIRED_PROPS_DELETE, 0,
                        prop_list, 0, NULL, NULL, timeout_ms);

//   tm_data_delete(prop_list);
// after call tm_send_request,don't need to call tm_data_delete.
// data will be packed into payload and deleted in tm_send_request

  return ret;
}

void *tm_pack_device_data(void *data, const uint8_t *product_id,
                          const uint8_t *dev_name, void *prop, void *event,
                          int8_t as_raw) {
  return tm_onejson_pack_props_and_events(data, product_id, dev_name, prop,
                                          event, as_raw);
}

int32_t tm_post_pack_data(void *pack_data, uint32_t timeout_ms) {
  return tm_send_request((const uint8_t *)TM_TOPIC_PACK_DATA_POST, 0, pack_data,
                         0, NULL, 0, timeout_ms);
}

int32_t tm_post_history_data(void *history_data, uint32_t timeout_ms) {
  return tm_send_request((const uint8_t *)TM_TOPIC_HISTORY_DATA_POST, 0,
                         history_data, 0, NULL, 0, timeout_ms);
}

int32_t tm_step(uint32_t timeout_ms) {
#if defined(SDK_USE_MQTTS)
  return tm_mqtt_step(timeout_ms);
#elif defined(SDK_USE_COAP)
  return tm_coap_step(timeout_ms);
#elif defined(SDK_USE_NBIOT)
  return tm_lwm2m_step(timeout_ms);
#endif
}
