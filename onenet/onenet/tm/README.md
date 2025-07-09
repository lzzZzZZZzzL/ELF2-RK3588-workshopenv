# 物模型接入（OneNET Studio）

[![Language](https://img.shields.io/badge/Language-C-brightgreen.svg?style=flat-square "Language")]()


- [概述](#概述)
- [用户接口说明](#用户接口说明)

  - [设备通用接口](#设备通用接口)
  - [子设备接口](#子设备接口)
  - [文件管理接口](#文件管理接口)
  - [功能点操作接口](#功能点操作接口)
- [用户用例](#用户用例)

## 概述

物模型接入主要用于对接[OneNET Studio](https://open.iot.10086.cn/studio/summary)。该接入方式要求用户对自己的产品进行数字化抽象并建立物模型，使用三种基础功能（**属性**、**服务**、**事件**）来描述设备能做什么，能对外提供什么服务

OneNET Studio在设备侧提供了多种接入方式：MQTT、CoAP、LwM2M、HTTP、泛协议，SDK主要支持**MQTT**和**CoAP**两种方式。

## 用户接口说明

### 设备通用接口

物模型接入的MQTT和CoAP协议均使用了相同的数据定义和几乎一致的交互流程，SDK将其封装为同一套操作接口，通过配置进行切换。

- 头文件

  ```c
  onenet/tm/tm_api.h
  ```

- 接口说明

  - 设备初始化/去初始化

    由于平台下发数据的不定性，对于下发数据的处理需要通过回调函数来通知用户。当设备调用初始化接口时，要求配置下发数据的处理回调接口。

    ```c
    /**
     * @brief 设备初始化
     * 
     * @param downlink_tbl 下行数据处理回调接口表，包含属性和服务的处理接口
     * @return 0 - 成功；-1 - 失败
     */
    int32_t tm_init(struct tm_downlink_tbl_t *downlink_tbl);
    int32_t tm_deinit(void);
    ```
  
    > *一般情况下，初始化和去初始化接口只需要调用一次，初始化后只需要通过登录登出接口来维护设备的在线状态即可。*
  
  - 设备登录
  
    ```c
    /**
     * @brief 向平台发起设备登录请求
     * 
     * @param product_id 产品ID
     * @param dev_name 设备名称
     * @param access_key 产品key或设备key
     * @param timeout_ms 登录超时时间
     * @return 0 - 登录成功；-1 - 失败
     */
    int32_t tm_login(const uint8_t *product_id, const uint8_t *dev_name, const uint8_t *access_key, uint32_t timeout_ms);
    ```
  
  - 设备登出
  
    ```c
    /**
     * @brief 设备登出
     * 
     * @param timeout_ms 超时时间
     * @return 0 - 登出成功；-1 - 失败
     */
    int32_t tm_logout(uint32_t timeout_ms);
    ```
  
  - 上报属性/事件
  
    通常情况下，用户不会直接使用上报属性/事件接口，而是调用平台自动生成的功能点操作接口。
  
    ```c
    int32_t tm_post_property(void *prop_data, uint32_t timeout_ms);
    int32_t tm_post_event(void *event_data, uint32_t timeout_ms);
    ```
  
    > *用户需要同时上报多个属性或事件时，需要先调用tm_data.h的接口构造数据，再使用上报接口发送到平台。具体用法可参考用户用例*
    
  - 主循环
  
    用于接收并解析平台下发的数据，解析成功后通过设置的回调接口传递给用户；同时本接口还负责维护连接保活。
  
    ```c
    int32_t tm_step(uint32_t timeout_ms);
    ```
  
  - 属性期望值管理（获取/删除）
  
    当北向应用需要设置设备的属性值，而设备不在线无法下发设置指令时，可以对设备设置属性期望值。当设备重新上线后主动获取期望值，并进行本地生效。
  
    ```c
    int32_t tm_get_desired_props(uint32_t timeout_ms);
    int32_t tm_delete_desired_props(uint32_t timeout_ms);
    ```
  
    > *调用接口后，SDK内部解析平台下发的期望属性值，将自动调用对应的功能点操作接口。*
  
  - 打包/上报设备批量数据
  
    ```c
    /**
     * @brief 打包设备的属性和事件数据，可用于子设备
     * 
     * @param data 需要打包的目标指针地址，用于后续调用上报接口。设置为空时由接口内部分配空间，并通过返回值返回地址
     * @param product_id 需要打包数据的产品id
     * @param dev_name 需要打包数据的设备名称
     * @param prop 传入属性数据。支持json格式(as_raw为1)，也可以仿照tm_user文件的数据上传接口使用tm_data接口构造数据（as_raw为0）
     * @param event 定义同prop，用于传入事件数据
     * @param as_raw 是否为原始json格式的数据
     * @return 打包后的数据指针地址 
     */
    void *  tm_pack_device_data(void *data, const uint8_t *product_id, const uint8_t *dev_name, void *prop, void *event, int8_t as_raw);
    int32_t tm_post_pack_data(void *pack_data, uint32_t timeout_ms);
    ```
  
    > *接口主要用于打包批量数据，适用于以下两种情形：*
    >
    > 1. *同时打包属性和事件*
    >
    >    *分别调用接口打包属性和事件，然后调用本接口，将属性和事件数据进行组合。*
    >
    > 2. *同时打包多个设备的数据*
    >
    >    *主要适用于多个子设备的数据上报。用户可多次调用打包接口，将上一次打包的返回值作为下一次打包的目标地址，实现多个设备的数据组合。*
  
  - 上报设备历史数据
  
    ```c
    int32_t tm_post_history_data(void *history_data, uint32_t timeout_ms);
    ```
  
    > *打包接口使用设备批量数据共用打包接口。*

### 子设备接口

OneNET Studio的网关子设备模型仅用于MQTT协议接入。

- 头文件

  ```c
  onenet/tm/tm_subdev.h
  ```

- 接口说明

  - 功能初始化

    定义与通用接口类似，都需要传入回调接口函数指针，平台下发数据时调用。

    ```c
    int32_t tm_subdev_init(struct tm_subdev_cbs callbacks);
    ```

  - 添加子设备

    将指定子设备绑定到当前的网关设备，所需要的认证信息与通用接口登录时要求一致。

    ```c
    int32_t tm_subdev_add(const uint8_t *product_id, const uint8_t *dev_name, const uint8_t *access_key, uint32_t timeout_ms)
    ```

  - 删除子设备

    将指定子设备从当前网关删除，解除关联的拓扑关系。

    ```c
    int32_t tm_subdev_delete(const uint8_t *product_id, const uint8_t *dev_name, const uint8_t *access_key, uint32_t timeout_ms);
    ```

  - 获取子设备列表

    网关从平台侧同步当前已绑定到本设备的子设备列表。子设备列表通过回调函数返回，json格式。

    ```c
    int32_t tm_subdev_topo_get(uint32_t timeout_ms)
    ```

  - 子设备登录/登出

    ```c
    int32_t tm_subdev_login(const uint8_t *product_id, const uint8_t *dev_name, uint32_t timeout_ms);
    int32_t tm_subdev_logout(const uint8_t *product_id, const uint8_t *dev_name, uint32_t timeout_ms)
    ```

  - 子设备上报数据

    ```c
    int32_t tm_subdev_post_data(const uint8_t *product_id, const uint8_t *dev_name, uint8_t *prop_json, uint8_t *event_json, uint32_t timeout_ms)
    ```

### 文件管理接口

文件管理接口基于HTTP协议，要求设备必须具备TCP传输方式。文件管理相关接口不依赖于设备的登录状态，可随时调用。

- 头文件

  ```c
  onenet/tm/tm_api.h
  ```

- 接口说明

  - 关键定义

    - 文件数据包定义

      ```c
      struct tm_file_data_t
      {
          /** 本次获取的数据在文件数据的起始位置，从0开始*/
          uint32_t seq;
          /** 获取到的数据缓冲区地址*/
          uint8_t *data;
          /** 获取到的数据长度*/
          uint32_t data_len;
      };
      ```

    - 文件管理事件回调

      ```c
      enum tm_file_event_e
      {
          /** 获取文件数据回调，参数为struct tm_file_data_t*/
          TM_FILE_EVENT_GET_DATA = 0,
          /** 上传文件成功，参数为文件在平台保存的唯一ID，字符串型*/
          TM_FILE_EVENT_POST_SUCCESSED,
          /** 上传文件失败，参数为平台返回的错误信息，字符串型*/
          TM_FILE_EVENT_POST_FAILED,
      };
      typedef int32_t tm_file_cb(enum tm_file_event_e event, void *event_data);
      ```

  - 接口说明

    - 文件获取

      ```c
      /**
       * @brief 获取文件
       * 
       * @param callback 获取文件的事件回调接口
       * @param file_id 平台下发的文件唯一ID
       * @param file_size 需要获取的文件大小
       * @param segment_size 为节省终端资源，文件支持分片获取。本参数用于设置分片大小
       * @param timeout_ms 超时时间
       * @return 0 - 成功；其它 - 失败
       */
      int32_t tm_file_get(tm_file_cb callback, uint8_t *file_id, uint32_t file_size, uint32_t segment_size, uint32_t timeout_ms);
      ```

      > *平台首先会通过设备通用接口（tm_svc$AsyncGet_cb、或tm_svc$SyncGetFile_cb）下指令，通知设备需要获取的目标文件唯一 ID、文件名、文件大小， 设备主动使用本接口来获取文件内容数据。 如果接口调用嵌入到服务回调中使用，timeout_ms 参数过大可 能导致后续数据处理超时或丢失。*

    - 文件上传

      ```c
      /**
       * @brief 向平台上传文件
       * 
       * @param callback 文件事件回调接口
       * @param product_id 设备所属产品ID
       * @param dev_name 设备名称
       * @param access_key 产品key
       * @param file_name 文件名，具体限制参考平台说明文档
       * @param file 文件数据地址缓冲区
       * @param file_size 文件大小
       * @param timeout_ms 超时时间
       * @return 0 - 成功；其它 - 失败
       */
      int32_t tm_file_post(tm_file_cb callback, const uint8_t *product_id, const uint8_t *dev_name, const uint8_t *access_key, const uint8_t *file_name, uint8_t *file, uint32_t file_size, uint32_t timeout_ms);
      ```

### 功能点操作接口

请参考[物模型接入用户示例](../../examples/things_model/README.md)。

## 用户用例

请参考[物模型接入用户示例](../../examples/things_model/README.md)。
