#ifndef __MESH_DEVICE_H__
#define __MESH_DEVICE_H__

#include "c_types.h"

struct mesh_device_mac_type {
    uint8_t mac[6];
} __packed;

struct mesh_device_list_type {
    uint16_t size;
    uint16_t scale;  // include root
    struct mesh_device_mac_type root;
    struct mesh_device_mac_type *list;
};

void mesh_device_list_init();
void mesh_device_disp_mac_list();
void mesh_device_set_root(struct mesh_device_mac_type *root);
bool mesh_search_device(const struct mesh_device_mac_type *node);
bool mesh_device_add(struct mesh_device_mac_type *nodes, uint16_t count);
bool mesh_device_del(struct mesh_device_mac_type *nodes, uint16_t count);
bool mesh_device_get_root(const struct mesh_device_mac_type **root);
bool mesh_device_get_mac_list(const struct mesh_device_mac_type **list,
                              uint16_t *count);
#endif
