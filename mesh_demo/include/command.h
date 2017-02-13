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

#define cmd_return_recv                   (0)	 /* 直接返回接收到的数据 */
#define cmd_get_sdk_version               (1)    /* 获取sdk版本号 */
#define cmd_get_flash_size_map            (2)    /* 查询Flash size以及Flash map */
#define cmd_wifi_get_ip_info              (3)    /* 查询IP地址 */
#define cmd_wifi_get_macaddr              (4)    /* 查询mac地址 */
#define cmd_get_device_list               (5)    /* 获取mesh设备列表 */

#define cmd_send_data_to_mcu              (254)  /* 发送数据到mcu */

extern uint8 send_buffer[1024*5]; /* 发送缓冲区 */

extern void command_execute(void);

#endif
