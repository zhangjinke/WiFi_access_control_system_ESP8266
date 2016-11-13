/******************************************************************************
 * Copyright 2015-2016 Espressif Systems
 *
 * FileName: mesh_http.c
 *
 * Description: mesh demo for HTTP protocol parser
 *
 * Modification history:
 *     2016/03/15, v1.1 create this file.
*******************************************************************************/
#include "mesh_parser.h"

void url_test1_handler(void *arg);

static struct mesh_http_url_table g_url_tab[] = {
    {"url_test_1", url_test1_handler, NULL},
    //{"url_test_2", url_test2_handler, NULL},
    {NULL, NULL, NULL},
};

void ICACHE_FLASH_ATTR url_test1_handler(void *arg)
{
    MESH_PARSER_PRINT("%s\n", __func__);
}

void ICACHE_FLASH_ATTR
mesh_http_proto_parser(const void *mesh_header, uint8_t *pdata, uint16_t len)
{
    uint16_t i = 0;
    uint16_t url_count = 0;
    const char *url_req = NULL;

    MESH_PARSER_PRINT("%s, len:%u, data:%s\n", __func__, len, pdata);
    /*
     * check the complete of http packet
     */
    // mesh_http_check_complete(..., &url_req)
    
    /*
     *  parser url according to the url table
     */
    url_count = sizeof(g_url_tab) / sizeof(g_url_tab[0]);

    for (i = 0; i < url_count; i ++) {
        if (g_url_tab[i].url && url_req &&
            os_strcmp(g_url_tab[i].url, url_req) == 0) {
            /*
            if (g_url_tab[i].url_handler)
                g_url_tab[i].url_handler(...);
            */
            break;
        }
    }
}
