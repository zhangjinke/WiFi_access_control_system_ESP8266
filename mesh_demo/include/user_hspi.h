/**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: user_hspi.h
**
** 创   建   人: 张进科
**
** 文件创建日期: 2016 年 11 月 12 日
**
** 描        述: HSPI驱动

** 日志:
2016.11.12  创建本文件
*********************************************************************************************************/

#ifndef __USER_HSPI_H__
#define __USER_HSPI_H__

#include "user_interface.h"

#pragma pack(1)
struct wifi_pack
{
	uint32  crc;				/* crc校验 */
	uint8   cmd;				/* 命令 */
	uint16  lenth;			    /* 数据长度 */
	uint8  *data;				/* 数据 */
};
#pragma pack()

#define HSPI_SEND  0
#define HSPI_RECV  1

#define HSPI_SEND_QUEUE_LEN 8
#define HSPI_SEND_TASK_PRIO USER_TASK_PRIO_2
extern os_event_t hspi_send_Queue[HSPI_SEND_QUEUE_LEN];

#define HSPI_RECV_QUEUE_LEN 8
#define HSPI_RECV_TASK_PRIO USER_TASK_PRIO_1
extern os_event_t hspi_recv_Queue[HSPI_RECV_QUEUE_LEN];

extern void ICACHE_FLASH_ATTR hspi_slave_init();
extern void ICACHE_FLASH_ATTR hspi_task(os_event_t *e);
extern void ICACHE_FLASH_ATTR hspi_send_task(os_event_t *e);
extern void ICACHE_FLASH_ATTR hspi_recv_task(os_event_t *e);

extern s8 wifi_send(u8 cmd, u16 data_lenth, u8 *data);

#endif
