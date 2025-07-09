# OneNET系统适配层接口移植

- [概述](#概述)
- [接口说明](#接口说明)
  - [系统适配](#系统适配)
  - [时间接口](#时间接口)
  - [tcp通信接口](#tcp通信接口)
  - [udp通信接口](#udp通信接口)
  - [串口通信接口](#串口通信接口)
- [移植说明](#移植说明)

## 概述

OneNET SDK通过对常用系统、硬件平台相关的接口进行封装，定义了一套统一的头文件接口。当底层系统或平台发生变化时，只需要按照统一接口进行适配即可，保证上层应用直接可运行。

## 接口说明

### 系统适配

- 头文件

  ```c
  platforms/include/plat_osl.h
  ```

- 接口说明

  - 内存操作函数

    |    接口     |                  说明                  |
    | :---------: | :------------------------------------: |
    | osl_malloc  |              动态分配内存              |
    | osl_calloc  |        动态分配内存并初始化为0         |
    |  osl_free   |           释放动态分配的内存           |
    | osl_memcpy  |                内存拷贝                |
    | osl_memmove | 内存拷贝，支持目标区域和源区域存在重叠 |
    | osl_memset  |               内存初始化               |
  
- 字符串处理函数
  
    |          接口          |                     说明                     |
    | :--------------------: | :------------------------------------------: |
    | osl_strdup/osl_strndup | 拷贝字符串到新的地址，不使用后需osl_free释放 |
    |       osl_strcpy       |                  拷贝字符串                  |
    |       osl_strlen       |                计算字符串长度                |
    |       osl_strcat       |                  连接字符串                  |
    | osl_strcmp/osl_strncmp |                  比较字符串                  |
    |       osl_strstr       |                  查找字符串                  |
    |      osl_sprintf       |                 格式化字符串                 |
    |       osl_sscanf       |                  解析字符串                  |
  
- 其它
  
    |      接口      |    说明    |
    | :------------: | :--------: |
    | osl_get_random | 获取随机数 |

### 时间接口

- 头文件

  ```c
  platforms/include/plat_time.h
  ```

- 接口说明

  - 时间函数

    |           接口           |          说明           |
    | :----------------------: | :---------------------: |
    | time_count_ms/time_count | 返回时间计数（毫秒/秒） |
    | time_delay_ms/time_delay | 以毫秒/秒级进行休眠延迟 |

  - 计时器接口

    |         接口         |           说明           |
    | :------------------: | :----------------------: |
    |   countdown_start    |      启动一个计时器      |
    |    countdown_set     | 重新设置计时器的到期时间 |
    |    countdown_left    | 返回计时器到期的剩余时间 |
    | countdown_is_expired |   判断计时器是否已到期   |
    |    countdown_stop    |      销毁指定计时器      |

### TCP通信接口

TCP接口主要用于MQTT接入方式。用户只使用NB、CoAP方式时无需移植。

- 头文件

  ```c
  platforms/include/plat_tcp.h
  ```

- 接口说明

  |        接口         |            说明             |
  | :-----------------: | :-------------------------: |
  |  plat_tcp_connect   |      连接指定网络地址       |
  |    plat_tcp_send    | 通过已建立的TCP连接发送数据 |
  |    plat_tcp_recv    | 通过已建立的TCP连接接收数据 |
  | plat_tcp_disconnect |     断开已建立的TCP连接     |

### UDP通信接口

UDP接口主要用于物模型接入的CoAP方式，其它接入方式无需移植。

- 头文件

  ```c
  platforms/include/plat_udp.h
  ```

- 接口说明

  |         接口        |                    说明                     |
  | :-----------------: | :-----------------------------------------: |
  |  plat_udp_connect   |  建立一个UDP句柄，并设置默认的远端收发地址  |
  |    plat_udp_bind    |            绑定UDP句柄的本地端口            |
  |    plat_udp_send    | 通过已建立的UDP句柄发送数据到默认的远端地址 |
  |   plat_udp_sendto   | 通过已建立的UDP句柄发送数据到指定的远端地址 |
  |    plat_udp_recv    |         通过已建立的UDP句柄接收数据         |
  |  plat_udp_recvfrom  |  通过已建立的UDP句柄接收来自指定地址的数据  |
  | plat_udp_disconnect |             销毁已建立的UDP句柄             |

### 串口通信接口

目前仅NB-IoT接入方式会通过串口与NB模组进行通信，其它方式无需移植串口。

- 头文件

  ```c
  platforms/include/plat_uart.h
  ```

- 接口说明

  |    接口    |                 说明                 |
  | :--------: | :----------------------------------: |
  | uart_open  | 以指定参数打开串口，返回串口操作句柄 |
  | uart_send  |       通过已打开的串口发送数据       |
  | uart_recv  |       通过已打开的串口接收数据       |
  | uart_close |           关闭已打开的串口           |

## 移植说明

用户在移植系统接口时可采用如下两种方式：

一. 在保持文件接口命名不变的情况下，直接修改linux目录中源码文件

二. 新增平台类型，按以下步骤：

1. 在platforms下新建目录，命名为平台类型名称，例如"myplat";

2. 修改CMakeLists.txt文件，将其中的linux标识全部替换为新的平台标识；

4. 复制platforms/linux目录下的所有文件到新目录platforms/myplat下，根据需要将xxx_linux.c文件以新平台标识重命名为xxx_myplat.c。

5. 根据实际情况，修改所有xxx_myplat.c内的函数实现，确保所有接口在新平台可用；

6. 重新配置SDK，重新编译即可。