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

#define HSPI_SEND  0
#define HSPI_RECV  1
#define HSPI_QUEUE_LEN 8
#define HSPI_TASK_PRIO USER_TASK_PRIO_1

extern os_event_t hspiQueue[HSPI_QUEUE_LEN];

extern void ICACHE_FLASH_ATTR hspi_slave_init();
extern void ICACHE_FLASH_ATTR hspi_task(os_event_t *e);

#endif
