/**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: command.h
**
** 创   建   人: 张进科
**
** 文件创建日期: 2017 年 1 月 31 日
**
** 描        述: 与stm32通信的命令集

** 日志:
2017.01.31  创建本文件
*********************************************************************************************************/

#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "user_hspi.h"

#define CMD_RETURN_RECV                   (0)	 /* 直接返回接收到的数据 */
#define CMD_GET_SDK_VERSION               (1)    /* 获取sdk版本号 */
#define CMD_GET_FLASH_SIZE_MAP            (2)    /* 查询Flash size以及Flash map */
#define CMD_WIFI_GET_IP_INFO              (3)    /* 查询IP地址 */
#define CMD_WIFI_GET_MACADDR              (4)    /* 查询mac地址 */
#define CMD_GET_DEVICE_LIST               (5)    /* 获取mesh设备列表 */
#define CMD_SEND_MESH_DATA                (6)    /* 向mesh网络中发送数据 */
#define CMD_SERVER_ADDR_SET               (7)    /* 设置服务器IP与端口 */
#define CMD_MESH_GROUP_ID_SET             (8)    /* 设置MESH组ID */
#define CMD_ROUTER_SET                    (9)    /* 设置路由器信息 */
#define CMD_MESH_WIFI_SET                 (10)   /* 设置MESH网络信息 */
#define CMD_MESH_INIT                     (11)   /* 初始化MESH */

#define CMD_SEND_DATA_TO_MCU              (254)  /* 发送数据到mcu */

extern uint8 send_buffer[1024*5]; /* 发送缓冲区 */

extern void command_execute(void);

#endif
