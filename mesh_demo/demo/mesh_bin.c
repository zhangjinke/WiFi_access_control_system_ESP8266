/******************************************************************************
 * Copyright 2015-2016 Espressif Systems
 *
 * FileName: mesh_bin.c
 *
 * Description: mesh demo for binary stream parser
 *
 * Modification history:
 *     2016/03/15, v1.1 create this file.
*******************************************************************************/
#include "mesh_parser.h"

#include "command.h"
#include "user_hspi.h"

void ICACHE_FLASH_ATTR
mesh_bin_proto_parser(const void *mesh_header, uint8_t *pdata, uint16_t len)
{
    MESH_PARSER_PRINT("%s\n", __func__);
	
    int i = 0;
    struct mesh_header_format *header = NULL;

    header = (struct mesh_header_format *)mesh_header;

	for (i = 0; i < header->len; i++)
	{
		send_buffer[i] = ((u8 *)header)[i];
	}
	wifi_pack_send.cmd = cmd_send_data_to_mcu;
	wifi_pack_send.lenth = header->len;
	wifi_pack_send.data = send_buffer;
	system_os_post(HSPI_SEND_TASK_PRIO,HSPI_SEND,0);
}
