#include "mem.h"
#include "mesh.h"
#include "osapi.h"
#include "c_types.h"
#include "espconn.h"
#include "user_config.h"
#include "mesh_parser.h"
#include "esp_touch.h"
#include "smartconfig.h"

ESPTOUCH_PROC esptouch_proc;
static os_timer_t esptouch_tout_t;
static bool esptouch_running = false;

/******************************************************************************
 * FunctionName : esptouch_is_running
 * Description  : to determine whether the esptouch is running
*******************************************************************************/
bool ICACHE_FLASH_ATTR
esptouch_is_running()
{
    return esptouch_running;
}

/******************************************************************************
 * FunctionName : esptouch_success_cb
 * Description  : esp-touch success callback
*******************************************************************************/
static void ICACHE_FLASH_ATTR
esptouch_success_cb(void* data)
{
    wifi_set_opmode(STATIONAP_MODE);
    os_timer_disarm(&esptouch_tout_t);
    MESH_DEMO_PRINT("ESP-TOUCH SUCCESS\r\n");
    MESH_DEMO_PRINT("CONNECTED TO AP...ENABLE MESH AND RUN PLATFORM CODE ...WAIT...\r\n");
    esptouch_running = false;
}

/******************************************************************************
 * FunctionName : esptouch_fail_cb
 * Description  : esp-touch fail callback
*******************************************************************************/
static void ICACHE_FLASH_ATTR
esptouch_fail_cb(void* data)
{
    wifi_station_disconnect();
    smartconfig_stop();
    MESH_DEMO_PRINT("ESP-TOUCH TIMEOUT\r\n");
    os_timer_disarm(&esptouch_tout_t);

    esptouch_running = false;
}

/******************************************************************************
 * FunctionName : esptouch_start_cb
 * Description  : esp-touch start callback
*******************************************************************************/
static void ICACHE_FLASH_ATTR
esptouch_start_cb(void* para)
{
    MESH_DEMO_PRINT("****START ESP-TOUCH****\n");
}

/******************************************************************************
 * FunctionName : esptouch_proc_cb
 * Description  : esp-touch status change callback for smartconfig_start
*******************************************************************************/
static void ICACHE_FLASH_ATTR
esptouch_proc_cb(sc_status status, void *pdata)
{
    switch (status) {
    case SC_STATUS_WAIT:
        MESH_DEMO_PRINT("SC_STATUS_WAIT\n");
        break;
    case SC_STATUS_FIND_CHANNEL:
        MESH_DEMO_PRINT("SC_STATUS_FIND_CHANNEL\n");
        if (esptouch_proc.esptouch_start_cb) {
            esptouch_proc.esptouch_start_cb(NULL);
        }
        break;
    case SC_STATUS_GETTING_SSID_PSWD:
        MESH_DEMO_PRINT("SC_STATUS_GETTING_SSID_PSWD\n");

        if (esptouch_proc.esptouch_fail_cb) {
            os_timer_disarm(&esptouch_tout_t);
            os_timer_setfn(&esptouch_tout_t, esptouch_proc.esptouch_fail_cb, NULL);
            os_timer_arm(&esptouch_tout_t, ESP_TOUCH_TIMEOUT_MS, 0);
        }
        break;
    case SC_STATUS_LINK:
        MESH_DEMO_PRINT("SC_STATUS_LINK\n");
        struct station_config *sta_conf = pdata;
        wifi_station_disconnect();
        espconn_mesh_set_router(sta_conf);
        wifi_station_set_config(sta_conf);
        wifi_station_connect();

        os_timer_disarm(&esptouch_tout_t);
        os_timer_arm(&esptouch_tout_t, ESPTOUCH_CONNECT_TIMEOUT_MS, 0);

        break;
    case SC_STATUS_LINK_OVER:
        os_timer_disarm(&esptouch_tout_t);
        MESH_DEMO_PRINT("SC_STATUS_LINK_OVER\n");
        if (esptouch_proc.esptouch_type == SC_TYPE_ESPTOUCH) {
            uint8 phone_ip[4] = {0};
            os_memcpy(phone_ip, (uint8*)pdata, 4);
            MESH_DEMO_PRINT("Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
        }
        smartconfig_stop();
        if (esptouch_proc.esptouch_suc_cb) {
            esptouch_proc.esptouch_suc_cb(NULL);//run finish cb
        }
        break;
    }
}

/******************************************************************************
 * FunctionName : esptouch_init
 * Description  : Set esp-touch callback and start esp-touch
*******************************************************************************/
void ICACHE_FLASH_ATTR
esptouch_init()
{
    esptouch_running = true;
    wifi_station_disconnect();

    MESH_DEMO_PRINT("ESP-TOUCH FLOW INIT...\r\n");
    esptouch_proc.esptouch_fail_cb = esptouch_fail_cb;
    esptouch_proc.esptouch_start_cb = esptouch_start_cb;
    esptouch_proc.esptouch_suc_cb = esptouch_success_cb;
    esptouch_proc.esptouch_type = SC_TYPE_ESPTOUCH;

    MESH_DEMO_PRINT("ESP-TOUCH SET STATION MODE ...\r\n");
    wifi_set_opmode(STATION_MODE);

    if (esptouch_proc.esptouch_fail_cb) {
        os_timer_disarm(&esptouch_tout_t);
        os_timer_setfn(&esptouch_tout_t, esptouch_proc.esptouch_fail_cb, NULL);
        os_timer_arm(&esptouch_tout_t, ESP_TOUCH_TIME_ENTER, 0);
    }

    smartconfig_start(esptouch_proc_cb);
}
