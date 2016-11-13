#ifndef __MESH_MQTT_H__
#define __MESH_MQTT_H__

#include "c_types.h"

/*
 * this function is used to parse packet formatted with MQTT
 */
void mesh_mqtt_proto_parser(const void *mesh_header, uint8_t *pdata, uint16_t len);

#endif
