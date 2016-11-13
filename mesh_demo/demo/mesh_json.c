/******************************************************************************
 * Copyright 2015-2016 Espressif Systems
 *
 * FileName: mesh_json.c
 *
 * Description: mesh demo for JSON protocol parser
 *
 * Modification history:
 *     2016/03/15, v1.1 create this file.
*******************************************************************************/
#include "mem.h"
#include "osapi.h"
#include "mesh_parser.h"

extern struct espconn g_ser_conn;

void ICACHE_FLASH_ATTR
mesh_json_proto_parser(const void *mesh_header, uint8_t *pdata, uint16_t len)
{
    MESH_PARSER_PRINT("%s\n", __func__);
    MESH_PARSER_PRINT("len:%u, data:%s\n", len, pdata);
}

void ICACHE_FLASH_ATTR
mesh_json_bcast_test()
{
    char buf[32];
    uint8_t src[6];
    uint8_t dst[6];
    struct mesh_header_format *header = NULL;

    if (!wifi_get_macaddr(STATION_IF, src)) {
        MESH_PARSER_PRINT("bcast get sta mac fail\n");
        return;
    }

    os_memset(buf, 0, sizeof(buf));

    os_sprintf(buf, "%s", "{\"bcast\":\"");
    os_sprintf(buf + os_strlen(buf), MACSTR, MAC2STR(src));
    os_sprintf(buf + os_strlen(buf), "%s", "\"}\r\n");

    os_memset(dst, 0, sizeof(dst));  // use bcast to get all the devices working in mesh from root.
    header = (struct mesh_header_format *)espconn_mesh_create_packet(
                            dst,     // destiny address (bcast)
                            src,     // source address
                            false,   // not p2p packet
                            true,    // piggyback congest request
                            M_PROTO_JSON,   // packe with JSON format
                            os_strlen(buf), // data length
                            false,   // no option
                            0,       // option total len
                            false,   // no frag
                            0,       // frag type, this packet doesn't use frag
                            false,   // more frag
                            0,       // frag index
                            0);      // frag length
    if (!header) {
        MESH_PARSER_PRINT("bcast create packet fail\n");
        return;
    }

    if (!espconn_mesh_set_usr_data(header, buf, os_strlen(buf))) {
        MESH_DEMO_PRINT("bcast set user data fail\n");
        MESH_DEMO_FREE(header);
        return;
    }

    if (espconn_mesh_sent(&g_ser_conn, (uint8_t *)header, header->len)) {
        MESH_DEMO_PRINT("bcast mesh is busy\n");
        MESH_DEMO_FREE(header);
        espconn_mesh_connect(&g_ser_conn);
        return;
    }

    MESH_DEMO_FREE(header);
}

void ICACHE_FLASH_ATTR
mesh_json_p2p_test()
{
    uint16_t idx;
    char buf[32];
    uint8_t src[6];
    uint8_t dst[6];
    uint16_t dev_count = 0;
    struct mesh_header_format *header = NULL;
    struct mesh_device_mac_type *list = NULL;

    if (!wifi_get_macaddr(STATION_IF, src)) {
        MESH_PARSER_PRINT("p2p get sta mac fail\n");
        return;
    }

    if (!mesh_device_get_mac_list((const struct mesh_device_mac_type **)&list, &dev_count)) {
        MESH_PARSER_PRINT("p2p get mac list fail\n");
        return;
    }

    /*
     * if device count in mac list is more than 2,
     * we select one device random as destination device
     * otherwise, we use root device as destination.
     */
    idx = dev_count > 1 ? (os_random() % (dev_count + 1)) : 0;
    if (!idx) {
        if (!mesh_device_get_root((const struct mesh_device_mac_type **)&list))
            return;
        os_memcpy(dst, list, sizeof(dst));
    } else {
        os_memcpy(dst, list + idx - 1, sizeof(dst));
    }

    os_memset(buf, 0, sizeof(buf));

    os_sprintf(buf, "%s", "{\"p2p\":\"");
    os_sprintf(buf + os_strlen(buf), MACSTR, MAC2STR(src));
    os_sprintf(buf + os_strlen(buf), "%s", "\"}\r\n");

    header = (struct mesh_header_format *)espconn_mesh_create_packet(
                            dst,     // destiny address
                            src,     // source address
                            true,    // not p2p packet
                            true,    // piggyback congest request
                            M_PROTO_JSON,        // packe with JSON format
                            os_strlen(buf),// data length
                            false,   // no option
                            0,       // option total len
                            false,   // no frag
                            0,       // frag type, this packet doesn't use frag
                            false,   // more frag
                            0,       // frag index
                            0);      // frag length
    if (!header) {
        MESH_PARSER_PRINT("p2p create packet fail\n");
        return;
    }

    if (!espconn_mesh_set_usr_data(header, buf, os_strlen(buf))) {
        MESH_DEMO_PRINT("p2p set user data fail\n");
        MESH_DEMO_FREE(header);
        return;
    }

    if (espconn_mesh_sent(&g_ser_conn, (uint8_t *)header, header->len)) {
        MESH_DEMO_PRINT("p2p mesh is busy\n");
        MESH_DEMO_FREE(header);
        espconn_mesh_connect(&g_ser_conn);
        return;
    }

    MESH_DEMO_FREE(header);
}

void ICACHE_FLASH_ATTR
mesh_json_mcast_test()
{
    char buf[32];
    uint8_t src[6];
    uint16_t i = 0;
    uint16_t ot_len = 0, op_count = 0;
    uint16_t dev_count = 0, max_count = 0;
    struct mesh_header_format *header = NULL;
    struct mesh_header_option_format *option = NULL;
    uint8_t dst[6] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
    struct mesh_device_mac_type *list = NULL, *root = NULL;

    if (!wifi_get_macaddr(STATION_IF, src)) {
        MESH_PARSER_PRINT("mcast get sta mac fail\n");
        return;
    }

    if (!mesh_device_get_mac_list((const struct mesh_device_mac_type **)&list, &dev_count)) {
        MESH_PARSER_PRINT("mcast get mac list fail\n");
        return;
    }

    if (!mesh_device_get_root((const struct mesh_device_mac_type **)&root)) {
        MESH_PARSER_PRINT("mcast get root fail\n");
        return;
    }

    dev_count ++;

    /*
     * if it is possible, we select all nodes as one mcast group
     */

    os_memset(buf, 0, sizeof(buf));
    os_sprintf(buf, "%s", "{\"mcast\":\"");
    os_sprintf(buf + os_strlen(buf), MACSTR, MAC2STR(src));
    os_sprintf(buf + os_strlen(buf), "%s", "\"}\r\n");

    // rest len for mcast option
    max_count = (ESP_MESH_PKT_LEN_MAX - ESP_MESH_HLEN - os_strlen(buf));
    // max mcast option count
    max_count /= ESP_MESH_OPTION_MAX_LEN;
    // max device count for mcast in current packet
    max_count *= ESP_MESH_DEV_MAX_PER_OP;
    if (dev_count > max_count)
        dev_count = max_count;

    op_count = dev_count / ESP_MESH_DEV_MAX_PER_OP;
    ot_len = ESP_MESH_OT_LEN_LEN
             + op_count * (sizeof(*option) + sizeof(*root) * ESP_MESH_DEV_MAX_PER_OP)
             + sizeof(*option) + (dev_count - op_count * ESP_MESH_DEV_MAX_PER_OP) * sizeof(*root);

    header = (struct mesh_header_format *)espconn_mesh_create_packet(
                            dst,     // destiny address
                            src,     // source address
                            false,   // not p2p packet
                            true,    // piggyback congest request
                            M_PROTO_JSON,    // packe with JSON format
                            os_strlen(buf),  // data length
                            true,    // option exist
                            ot_len,  // option total len
                            false,   // no frag
                            0,       // frag type, this packet doesn't use frag
                            false,   // more frag
                            0,       // frag index
                            0);      // frag length
    if (!header) {
        MESH_PARSER_PRINT("mcast create packet fail\n");
        return;
    }

    while (i < op_count) {
        option = (struct mesh_header_option_format *)espconn_mesh_create_option(
                M_O_MCAST_GRP, (uint8_t *)(list + i * ESP_MESH_DEV_MAX_PER_OP),
                (uint8_t)(sizeof(*root) * ESP_MESH_DEV_MAX_PER_OP));
        if (!option) {
            MESH_PARSER_PRINT("mcast %d create option fail\n", i);
            goto MCAST_FAIL;
        }

        if (!espconn_mesh_add_option(header, option)) {
            MESH_PARSER_PRINT("mcast %d set option fail\n", i);
            goto MCAST_FAIL;
        }

        i ++;
        MESH_DEMO_FREE(option);
        option = NULL;
    }

    {
        char *rest_dev = NULL;
        uint8_t rest = dev_count - op_count * ESP_MESH_DEV_MAX_PER_OP;
        if (rest > 0) {
            rest_dev = (char *)os_zalloc(sizeof(*root) * rest);
            if (!rest_dev) {
                MESH_PARSER_PRINT("mcast alloc the last option buf fail\n");
                goto MCAST_FAIL;
            }
            os_memcpy(rest_dev, root, sizeof(*root));
            if (list && rest > 1)
                os_memcpy(rest_dev + sizeof(*root),
                        list + i * ESP_MESH_DEV_MAX_PER_OP,
                        (rest - 1) * sizeof(*root));
            option = (struct mesh_header_option_format *)espconn_mesh_create_option(
                    M_O_MCAST_GRP, rest_dev, sizeof(*root) * rest);
            MESH_DEMO_FREE(rest_dev);
            if (!option) {
                MESH_PARSER_PRINT("mcast create the last option fail\n");
                goto MCAST_FAIL;
            }

            if (!espconn_mesh_add_option(header, option)) {
                MESH_PARSER_PRINT("mcast set the last option fail\n");
                goto MCAST_FAIL;
            }
        }
    }

    if (!espconn_mesh_set_usr_data(header, buf, os_strlen(buf))) {
        MESH_DEMO_PRINT("mcast set user data fail\n");
        goto MCAST_FAIL;
    }

    if (espconn_mesh_sent(&g_ser_conn, (uint8_t *)header, header->len)) {
        MESH_DEMO_PRINT("mcast mesh is busy\n");
        espconn_mesh_connect(&g_ser_conn);
    }

MCAST_FAIL:
    option ? MESH_DEMO_FREE(option) : 0;
    header ? MESH_DEMO_FREE(header) : 0;
}

void ICACHE_FLASH_ATTR
mesh_json_mcast_test_init()
{
    static os_timer_t mcast_timer;
    os_timer_disarm(&mcast_timer);
    os_timer_setfn(&mcast_timer, (os_timer_func_t *)mesh_json_mcast_test, NULL);
    os_timer_arm(&mcast_timer, 17000, true);
}

void ICACHE_FLASH_ATTR
mesh_json_bcast_test_init()
{
    static os_timer_t bcast_timer;
    os_timer_disarm(&bcast_timer);
    os_timer_setfn(&bcast_timer, (os_timer_func_t *)mesh_json_bcast_test, NULL);
    os_timer_arm(&bcast_timer, 18000, true);
}

void ICACHE_FLASH_ATTR
mesh_json_p2p_test_init()
{
    static os_timer_t p2p_timer;
    os_timer_disarm(&p2p_timer);
    os_timer_setfn(&p2p_timer, (os_timer_func_t *)mesh_json_p2p_test, NULL);
    os_timer_arm(&p2p_timer, 19000, true);
}
