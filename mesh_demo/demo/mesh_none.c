/******************************************************************************
 * Copyright 2015-2016 Espressif Systems
 *
 * FileName: mesh_none.c
 *
 * Description: mesh demo for mesh topology parser
 *
 * Modification history:
 *     2016/03/15, v1.1 create this file.
*******************************************************************************/
#include "mem.h"
#include "mesh.h"
#include "osapi.h"
#include "mesh_parser.h"

extern struct espconn g_ser_conn;

void ICACHE_FLASH_ATTR
mesh_disp_sub_dev_mac(uint8_t *sub_mac, uint16_t sub_count)
{
    uint16_t i = 0;
    const uint8_t mac_len = 6;

    if (!sub_mac || sub_count == 0) {
        MESH_PARSER_PRINT("sub_mac:%p, sub_count:%u\n", sub_mac, sub_count);
        return;
    }

    for (i = 0; i < sub_count; i ++) {
        MESH_PARSER_PRINT("idx: %u, mac:" MACSTR "\n", i, MAC2STR(sub_mac));
        sub_mac += mac_len;
    }
}

void ICACHE_FLASH_ATTR
mesh_none_proto_parser(const void *mesh_header, uint8_t *pdata, uint16_t len)
{
    uint16_t op_idx = 1;
    uint16_t dev_count = 0;
    uint8_t *dev_mac = NULL;
    const uint8_t mac_len = 6;
    struct mesh_header_format *header = NULL;
    struct mesh_header_option_format *option = NULL;

    MESH_PARSER_PRINT("%s\n", __func__);

    if (!pdata)
        return;

    header = (struct mesh_header_format *)pdata;
    MESH_PARSER_PRINT("root's mac:" MACSTR "\n", MAC2STR(header->src_addr));
    mesh_device_set_root((struct mesh_device_mac_type *)header->src_addr);

    while (espconn_mesh_get_option(header, M_O_TOPO_RESP, op_idx++, &option)) {
        dev_count = option->olen / mac_len;
        dev_mac = option->ovalue;
        mesh_device_add((struct mesh_device_mac_type *)dev_mac, dev_count);
        //mesh_disp_sub_dev_mac(dev_mac, dev_count);
    }

    mesh_device_disp_mac_list();
}

void ICACHE_FLASH_ATTR mesh_topo_test()
{
    uint8_t src[6];
    uint8_t dst[6];
    struct mesh_header_format *header = NULL;
    struct mesh_header_option_format *option = NULL;
    uint8_t ot_len = sizeof(struct mesh_header_option_header_type) + sizeof(*option) + sizeof(dst); 

    if (!wifi_get_macaddr(STATION_IF, src)) {
        MESH_PARSER_PRINT("get sta mac fail\n");
        return;
    }

    /*
     * root device uses espconn_mesh_get_node_info to get mac address of all devices
     */
    if (espconn_mesh_is_root()) {
        uint8_t *sub_dev_mac = NULL;
        uint16_t sub_dev_count = 0;
        if (!espconn_mesh_get_node_info(MESH_NODE_ALL, &sub_dev_mac, &sub_dev_count))
            return;
        // the first one is mac address of router
        //mesh_disp_sub_dev_mac(sub_dev_mac, sub_dev_count);
        mesh_device_set_root((struct mesh_device_mac_type *)src);

        if (sub_dev_count > 1) {
            struct mesh_device_mac_type *list = (struct mesh_device_mac_type *)sub_dev_mac;
            mesh_device_add(list + 1, sub_dev_count - 1);
        }
        mesh_device_disp_mac_list();

        // release memory occupied by mac address.
        espconn_mesh_get_node_info(MESH_NODE_ALL, NULL, NULL); 
        return;
    }

    /*
     * non-root device uses topology request with bcast to get mac address of all devices
     */
    os_memset(dst, 0, sizeof(dst));  // use bcast to get all the devices working in mesh from root.
    header = (struct mesh_header_format *)espconn_mesh_create_packet(
                            dst,     // destiny address (bcast)
                            src,     // source address
                            false,   // not p2p packet
                            true,    // piggyback congest request
                            M_PROTO_NONE,  // packe with JSON format
                            0,       // data length
                            true,    // no option
                            ot_len,  // option total len
                            false,   // no frag
                            0,       // frag type, this packet doesn't use frag
                            false,   // more frag
                            0,       // frag index
                            0);      // frag length
    if (!header) {
        MESH_PARSER_PRINT("create packet fail\n");
        return;
    }

    option = (struct mesh_header_option_format *)espconn_mesh_create_option(
            M_O_TOPO_REQ, dst, sizeof(dst));
    if (!option) {
        MESH_PARSER_PRINT("create option fail\n");
        goto TOPO_FAIL;
    }

    if (!espconn_mesh_add_option(header, option)) {
        MESH_PARSER_PRINT("set option fail\n");
        goto TOPO_FAIL;
    }

    if (espconn_mesh_sent(&g_ser_conn, (uint8_t *)header, header->len)) {
        MESH_PARSER_PRINT("topo mesh is busy\n");
        espconn_mesh_connect(&g_ser_conn);
    }
TOPO_FAIL:
    option ? MESH_DEMO_FREE(option) : 0;
    header ? MESH_DEMO_FREE(header) : 0;
}

void ICACHE_FLASH_ATTR mesh_topo_test_init()
{
    static os_timer_t topo_timer;
    os_timer_disarm(&topo_timer);
    os_timer_setfn(&topo_timer, (os_timer_func_t *)mesh_topo_test, NULL);
    os_timer_arm(&topo_timer, 14000, true);
}
