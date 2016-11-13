#ifndef __MESH_PARSER_H__
#define __MESH_PARSER_H__

#include "mesh_http.h"
#include "mesh_json.h"
#include "mesh_mqtt.h"
#include "mesh_none.h"
#include "mesh_bin.h"
#include "mesh_device.h"
#include "mesh.h"

#define MESH_PARSER_DEBUG
#ifdef MESH_PARSER_DEBUG
#define MESH_PARSER_PRINT ets_printf
#else
#define MESH_PARSER_PRINT
#endif

typedef void (*mesh_proto_parser_handler)(const void *mesh_header, uint8_t *pdata, uint16_t len);

struct mesh_general_parser_type {
    uint8_t proto;
    mesh_proto_parser_handler handler;
};

#endif
