#ifndef __esp_touch_H__
#define __esp_touch_H__
#include "user_config.h"
#include "smartconfig.h"

typedef void (*esptouch_StartAction)(void *para);
typedef void (*esptouch_FailCallback)(void *para);
typedef void (*esptouch_SuccessCallback)(void *para);
#define ESPTOUCH_CONNECT_TIMEOUT_MS  (20000)   /*Time limit for connecting WiFi after ESP-TOUCH figured out the SSID&PWD*/
#define ESP_TOUCH_TIME_ENTER         (2000)    /*Time limit for ESP-TOUCH to receive config packets*/
#define ESP_TOUCH_TIMEOUT_MS         (40000)   /*Total time limit for ESP-TOUCH*/
#define ESP_TOUCH_TIME_LIMIT         (2)       /*Get into ESPTOUCH MODE only twice at most, then scan mesh if not connected*/

typedef struct  {
    sc_type esptouch_type;
    esptouch_StartAction esptouch_start_cb;
    esptouch_FailCallback esptouch_fail_cb;
    esptouch_SuccessCallback esptouch_suc_cb;
} ESPTOUCH_PROC;

/******************************************************************************
 * FunctionName : esptouch_init
 * Description  : Set esp-touch callback and start esp-touch
*******************************************************************************/
void ICACHE_FLASH_ATTR esptouch_init();

/******************************************************************************
 * FunctionName : esptouch_is_running
 * Description  : to determine whether the esptouch is running
*******************************************************************************/
bool ICACHE_FLASH_ATTR esptouch_is_running();

#endif
