/**
 * \file
 * \brief 配置信息
 * 
 * \internal
 * \par Modification history
 * - 1.00 17-06-03  zhangjinke, first implementation.
 * \endinternal
 */
#ifndef __USER_CONFIG_H
#define __USER_CONFIG_H

#include "c_types.h"
#include "user_interface.h"

#define MESH_DEMO_PRINT  ets_printf
#define MESH_DEMO_STRLEN ets_strlen
#define MESH_DEMO_MEMCPY ets_memcpy
#define MESH_DEMO_MEMSET ets_memset
#define MESH_DEMO_FREE   os_free

/** \brief 服务器端口 */
extern uint16_t g_server_port;

/** \brief 服务器IP地址 */
extern uint8_t  g_server_ip[4];

/** \brief 串口波特率 */
extern uint32_t g_uart_baud_ratio;

/** \brief g_mesh_group_id和MESH_SSID_PREFIX相同即为同一MESH网络 */
extern uint8_t  g_mesh_group_id[6];

/** \brief 路由器MAC地址 */
extern uint8_t  g_mesh_router_bssid[6];

/** \brief 路由器名称 */
extern uint8_t g_mesh_router_ssid[32 + 1];

/** \brief 路由器密码 */
extern uint8_t g_mesh_router_passwd[256 + 1];

/** \brief MESH网络名称 */
extern uint8_t g_mesh_ssid_prefix[32 + 1];

/** \brief 路由器加密方式 */
extern uint8_t g_mesh_auth;

/** \brief MESH网络密码 */
extern uint8_t g_mesh_passwd[256 + 1];

/** \brief MESH网络最大跳数 */
extern uint8_t g_mesh_max_hop;

#endif

