cmake_minimum_required(VERSION 3.10) # Ubuntu 18.04
project(mqtts_onejson_soc)



# 定义包含目录
set(INCLUDE_DIRS
    # 基础组件
    common
    3rd/cJSON
    onenet/utils
    
    # 协议栈
    onenet/protocols/mqtt
    onenet/protocols/mqtt/paho-mqtt
    3rd/paho-mqtt
    
    # 安全模块
    onenet/security/tls
    3rd/wolfssl/wolfssl-3.15.3
    onenet/security/tls/wolfssl
    
    # 平台实现
    onenet/platforms/include
    
    # 物模型模块（特有）
    onenet/tm
)
include_directories(${INCLUDE_DIRS})

# 定义源文件
set(SOURCE_FILES
    # 通用组件
    common/log.c
    common/utils.c
    common/slist.c
    3rd/cJSON/cJSON.c
    
    # 协议实现
    onenet/protocols/mqtt/paho-mqtt/mqtt_client.c
    
    # 安全模块
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/coding.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/hash.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/hmac.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/logging.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/md5.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/misc.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/sha.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/sha256.c
    onenet/security/tls/tls.c
    
    # 平台适配
    onenet/platforms/linux/osl_linux.c
    onenet/platforms/linux/tcp_linux.c
    onenet/platforms/linux/time_linux.c
    onenet/platforms/linux/udp_linux.c
    onenet/utils/dev_token.c
    
    # 物模型特有模块
    onenet/tm/tm_api.c
    onenet/tm/tm_data.c
    onenet/tm/tm_onejson.c
    examples/things_model/tm_user.c
    examples/things_model/main.c
    onenet/tm/tm_mqtt.c
    onenet/tm/tm_subdev.c
    onenet/tm/dev_discov.c
    
    # WolfSSL扩展模块
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/aes.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/asn.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/integer.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/random.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/rsa.c
    3rd/wolfssl/wolfssl-3.15.3/wolfcrypt/src/wc_port.c
    3rd/wolfssl/wolfssl-3.15.3/src/internal.c
    3rd/wolfssl/wolfssl-3.15.3/src/keys.c
    3rd/wolfssl/wolfssl-3.15.3/src/ssl.c
    3rd/wolfssl/wolfssl-3.15.3/src/tls.c
    3rd/wolfssl/wolfssl-3.15.3/src/wolfio.c
    onenet/security/tls/tls.c
    3rd/paho-mqtt/MQTTConnectClient.c
    3rd/paho-mqtt/MQTTDeserializePublish.c
    3rd/paho-mqtt/MQTTFormat.c
    3rd/paho-mqtt/MQTTPacket.c
    3rd/paho-mqtt/MQTTSerializePublish.c
    3rd/paho-mqtt/MQTTSubscribeClient.c
    3rd/paho-mqtt/MQTTUnsubscribeClient.c
    onenet/protocols/mqtt/paho-mqtt/mqtt_client.c
)

# 定义编译选项
set(COMPILE_OPTIONS
    -w # hide warnings
    -g # add debug info
    -Wno-incompatible-function-pointer-types
    -DPLAT_HAVE_STDINT=1
    -DSDK_PAYLOAD_LEN=4096
    -DSDK_SEND_BUF_LEN=4096
    -DSDK_RECV_BUF_LEN=4096
    -DSDK_REQUEST_TIMEOUT=4096
    -DSDK_ACCESS_LIFE_TIME=120
    -DLOG_LEVEL=LOG_LEVEL_DEBUG
    -DCONFIG_PLAT_ARCH_64BIT=1
    -DCONFIG_TM_GATEWAY=0
    -DSDK_USE_MQTTS
    -DCONFIG_NETWORK_TLS=0
    -DIOT_MQTT_SERVER_ADDR_TLS="mqttstls.heclouds.com"
    -DIOT_MQTT_SERVER_PORT_TLS=8883
    -DIOT_MQTT_SERVER_ADDR="mqtts.heclouds.com"
    -DIOT_MQTT_SERVER_PORT=1883
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${COMPILE_OPTIONS})

target_link_libraries(${CMAKE_PROJECT_NAME} pthread)
