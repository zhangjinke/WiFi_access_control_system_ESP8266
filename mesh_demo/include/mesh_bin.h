#ifndef __MESH_BIN_H__
#define __MESH_BIN_H__

#include "c_types.h"

/*
 * this function is used to parse binary stream
 */
void mesh_bin_proto_parser(const void *mesh_header, uint8_t *pdata, uint16_t len);

#endif
