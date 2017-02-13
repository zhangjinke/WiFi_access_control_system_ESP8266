/**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: command.c
**
** 创   建   人: 张进科
**
** 文件创建日期: 2017 年 1 月 31 日
**
** 描        述: 与stm32通信的命令集

** 日志:
2017.01.31  创建本文件
*********************************************************************************************************/
#include "command.h"
#include "user_hspi.h"

#include "osapi.h"
#include "mesh_device.h"

char sdk_version[10];
int sdk_version_len = 0;

uint8  send_buffer[1024*5] = {0}; /* 发送缓冲区 */
char *err_str;
void command_execute(void)
{
//	os_printf("cmd: %d, lenth: %d, crc: %08X\r\n", wifi_pack_recv.cmd, wifi_pack_recv.lenth, wifi_pack_recv.crc);
	switch(wifi_pack_recv.cmd)
	{
		case cmd_return_recv:
		{
			wifi_pack_send.lenth = wifi_pack_recv.lenth;
			wifi_pack_send.data = wifi_pack_recv.data;
		} break;
		case cmd_get_sdk_version:
		{
			if (sdk_version_len == 0)
			{
				char *recv = (char *)system_get_sdk_version();
				sdk_version_len = os_strlen(recv);
				os_memcpy(sdk_version, recv, sdk_version_len);
			}
			wifi_pack_send.lenth = sdk_version_len;
			wifi_pack_send.data = sdk_version;
		} break;
		case cmd_get_flash_size_map:
		{
			send_buffer[0] = system_get_flash_size_map();
			wifi_pack_send.lenth = 1;
			wifi_pack_send.data = send_buffer;
		} break;
		case cmd_wifi_get_ip_info:
		{
			struct ip_info info;
			if(wifi_get_ip_info(STATION_IF, &info) == false)
			{
				err_str = "err: wifi_get_ip_info failed\r\n";
				wifi_pack_send.lenth = os_strlen(err_str);
				wifi_pack_send.data = err_str;
				break;
			}
			os_memcpy(send_buffer, &info, sizeof(struct ip_info));
			if(wifi_get_ip_info(SOFTAP_IF, &info) == false)
			{
				err_str = "err: wifi_get_ip_info failed\r\n";
				wifi_pack_send.lenth = os_strlen(err_str);
				wifi_pack_send.data = err_str;
				break;
			}
			os_memcpy(send_buffer + sizeof(struct ip_info), &info, sizeof(struct ip_info));
			wifi_pack_send.lenth = sizeof(struct ip_info)*2;
			wifi_pack_send.data = send_buffer;
		} break;
		case cmd_wifi_get_macaddr:
		{
			char *recv = (char *)system_get_sdk_version();
			if(wifi_get_macaddr(STATION_IF, recv) == false)
			{
				err_str = "err: wifi_get_macaddr failed\r\n";
				wifi_pack_send.lenth = os_strlen(err_str);
				wifi_pack_send.data = err_str;
				break;
			}
			os_memcpy(send_buffer, recv, os_strlen(recv));
			if(wifi_get_macaddr(SOFTAP_IF, recv) == false)
			{
				err_str = "err: wifi_get_macaddr failed\r\n";
				wifi_pack_send.lenth = os_strlen(err_str);
				wifi_pack_send.data = err_str;
				break;
			}
			os_memcpy(send_buffer+os_strlen(recv), recv, os_strlen(recv));
			wifi_pack_send.lenth = os_strlen(recv)*2;
			wifi_pack_send.data = send_buffer;
		} break;
		case cmd_get_device_list:
		{
			if ((g_mesh_device_init == false)||(g_node_list.scale < 1))
			{
				err_str = "err: mesh device not init\r\n";
				wifi_pack_send.lenth = os_strlen(err_str);
				wifi_pack_send.data = err_str;
				break;
			}
			if (g_node_list.scale < 2)
			{
				os_memcpy(send_buffer, g_node_list.root.mac, 6);
				wifi_pack_send.lenth = 6;
				wifi_pack_send.data = send_buffer;
				break;
			}
			else
			{
				os_memcpy(send_buffer, g_node_list.root.mac, 6);
				os_memcpy(send_buffer + 6, g_node_list.list, 6 * (g_node_list.scale - 1));
				wifi_pack_send.lenth = 6 * g_node_list.scale;
				wifi_pack_send.data = send_buffer;
				break;
			}
		} break;
		default:
		{
			err_str = "err: unknown command\r\n";
			wifi_pack_send.cmd = wifi_pack_recv.cmd;
			wifi_pack_send.lenth = os_strlen(err_str);
			wifi_pack_send.data = err_str;
		} break;
	}
	wifi_pack_send.cmd = wifi_pack_recv.cmd;
	system_os_post(HSPI_SEND_TASK_PRIO,HSPI_SEND,0);
}
