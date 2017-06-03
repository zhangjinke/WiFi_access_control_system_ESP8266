/**
 * \file
 * \brief 配置信息
 * 
 * \internal
 * \par Modification history
 * - 1.00 17-06-03  zhangjinke, first implementation.
 * \endinternal
 */

#include "user_config.h"

/** \brief 服务器端口 */
uint16_t g_server_port  = 0;

/** \brief 服务器IP地址 */
uint8_t  g_server_ip[4] = {0, 0, 0, 0};

/** \brief 串口波特率 */
uint32_t g_uart_baud_ratio = 76800;

/** \brief g_mesh_group_id和MESH_SSID_PREFIX相同即为同一MESH网络 */
uint8_t  g_mesh_group_id[6] = {0,0,0,0,0,0};

/** \brief 路由器MAC地址 */
uint8_t  g_mesh_router_bssid[6] = {0, 0, 0, 0, 0, 0};

/** \brief 路由器名称 */
uint8_t g_mesh_router_ssid[32 + 1] = "default";

/** \brief 路由器密码 */
uint8_t g_mesh_router_passwd[256 + 1] = "default";

/** \brief 路由器加密方式 */
uint8_t g_mesh_auth = AUTH_OPEN;

/** \brief MESH网络名称 */
uint8_t g_mesh_ssid_prefix[32 + 1] = "default";

/** \brief MESH网络密码 */
uint8_t g_mesh_passwd[256 + 1] = "default";

/** \brief MESH网络最大跳数 */
uint8_t g_mesh_max_hop = 4;

/* end of file */
