#include "mem.h"
#include "mesh.h"
#include "osapi.h"
#include "c_types.h"
#include "espconn.h"
#include "user_config.h"
#include "mesh_parser.h"

#define MESH_DEV_MEMSET os_memset
#define MESH_DEV_MEMCPY os_memcpy
#define MESH_DEV_MALLOC os_malloc
#define MESH_DEV_ZALLOC os_zalloc
#define MESH_DEV_MEMCMP os_memcmp
#define MESH_DEV_FREE   os_free

static bool g_mesh_device_init = false;
static struct mesh_device_list_type g_node_list;

void ICACHE_FLASH_ATTR
mesh_device_disp_mac_list()
{
    uint16_t idx = 0;

    if (g_node_list.scale < 1)
        return;

    MESH_DEMO_PRINT("=====mac list info=====\n");
    MESH_DEMO_PRINT("root: " MACSTR "\n", MAC2STR(g_node_list.root.mac));
    if (g_node_list.scale < 2)
        return;
    for (idx = 0; idx < g_node_list.scale - 1; idx ++)
        MESH_DEMO_PRINT("idx:%d, " MACSTR "\n", idx, MAC2STR(g_node_list.list[idx].mac));
    MESH_DEMO_PRINT("=====mac list end======\n");
}

bool ICACHE_FLASH_ATTR
mesh_device_get_mac_list(const struct mesh_device_mac_type **list,
                         uint16_t *count)
{
    if (!g_mesh_device_init) {
        MESH_DEMO_PRINT("please init mesh device list firstly\n");
        return false;
    }

    if (!list || !count)
        return false;

    if (g_node_list.scale < 2) {
        *list = NULL;
        *count = 0;
        return true;
    }

    *list = g_node_list.list;
    *count = g_node_list.scale - 1;
    return true;
}

bool ICACHE_FLASH_ATTR
mesh_device_get_root(const struct mesh_device_mac_type **root)
{
    if (!g_mesh_device_init) {
        MESH_DEMO_PRINT("please init mesh device list firstly\n");
        return false;
    }

    if (g_node_list.scale == 0) {
        MESH_DEMO_PRINT("no mac in current mac list\n");
        return false;
    }

    if (!root)
        return false;

    *root = &g_node_list.root;

    return true;
}

void ICACHE_FLASH_ATTR mesh_device_list_release()
{
    if (!g_mesh_device_init)
        return;

    if (g_node_list.list != NULL) {
        MESH_DEV_FREE(g_node_list.list);
        g_node_list.list = NULL;
    }
    MESH_DEV_MEMSET(&g_node_list, 0, sizeof(g_node_list));
}

void ICACHE_FLASH_ATTR mesh_device_list_init()
{
    if (g_mesh_device_init)
        return;
    
    MESH_DEV_MEMSET(&g_node_list, 0, sizeof(g_node_list));
    g_mesh_device_init = true;
}

void ICACHE_FLASH_ATTR
mesh_device_set_root(struct mesh_device_mac_type *root)
{
    if (!g_mesh_device_init)
        mesh_device_list_init();
    /*
     * the first time to set root
     */
    if (g_node_list.scale == 0) {
        MESH_DEMO_PRINT("new root:" MACSTR "\n", MAC2STR((uint8_t *)root));
        MESH_DEV_MEMCPY(&g_node_list.root, root, sizeof(*root));
        g_node_list.scale = 1;
        return;
    }
    
    /*
     * root device is the same to the current node,
     * we don't need to modify anything
     */
    if (!MESH_DEV_MEMCMP(&g_node_list.root, root, sizeof(*root)))
        return;

    /*
     * switch root device, so the mac address list is stale
     * we need to free the stale the mac list
     */
    MESH_DEMO_PRINT("switch root:" MACSTR "to root:" MACSTR "\n",
            MAC2STR((uint8_t *)&g_node_list.root), MAC2STR((uint8_t *)root));
    mesh_device_list_release();
    MESH_DEV_MEMCPY(&g_node_list.root, root, sizeof(*root));
    g_node_list.scale = 1;
}

bool ICACHE_FLASH_ATTR
mesh_search_device(const struct mesh_device_mac_type *node)
{
    uint16_t idx = 0;
    uint16_t scale = 0, i = 0;
    struct mesh_device_mac_type *list = NULL;

    if (g_node_list.scale == 0)
        return false;
    if (!MESH_DEV_MEMCMP(&g_node_list.root, node, sizeof(*node)))
        return true;
    if (g_node_list.list == NULL)
        return false;

    scale = g_node_list.scale - 1;
    list = g_node_list.list;

    for (i = 0; i < scale; i ++) {
        if (!MESH_DEV_MEMCMP(list, node, sizeof(*node)))
            return true;
        list ++;
    }
    return false;
}

bool ICACHE_FLASH_ATTR
mesh_device_add(struct mesh_device_mac_type *nodes, uint16_t count)
{
#define MESH_DEV_STEP (10)
    uint16_t idx = 0;
    uint16_t rest = g_node_list.size + 1 - g_node_list.scale;

    if (!g_mesh_device_init)
        mesh_device_list_init();

    if (g_node_list.size == 0)
        rest = 0;

    if (rest < count) {
        /*
         * current list is limited
         * we need to re-allocate buffer for mac list
         */
        uint16_t size = g_node_list.size + rest + MESH_DEV_STEP;
        uint8_t *buf = (uint8_t *)MESH_DEV_ZALLOC(size * sizeof(*nodes));
        if (!buf) {
            MESH_DEMO_PRINT("mesh add alloc buf fail\n");
            return false;
        }
        /*
         * move the current list to new buffer
         */
        if (g_node_list.list && g_node_list.scale > 1)
            MESH_DEV_MEMCPY(buf, g_node_list.list,
                    (g_node_list.scale - 1) * sizeof(*nodes));
        if (g_node_list.list)
            MESH_DEV_FREE(g_node_list.list);
        g_node_list.list = (struct mesh_device_mac_type *)buf;
        g_node_list.size = size;
    }

    while (idx < count) {
        if (!mesh_search_device(nodes + idx)) {  // not in list, add it into list
            MESH_DEV_MEMCPY(g_node_list.list + g_node_list.scale - 1,
                    nodes + idx, sizeof(*nodes));
            g_node_list.scale ++;
        }
        idx ++;
    }
    return true;
}

bool ICACHE_FLASH_ATTR
mesh_device_del(struct mesh_device_mac_type *nodes, uint16_t count)
{
    uint16_t idx = 0, i = 0;
    uint16_t sub_count = g_node_list.scale - 1;

    if (!nodes || count == 0)
        return true;

    if (!g_mesh_device_init)
        mesh_device_list_init();

    if (g_node_list.scale == 0)
        return false;

    while (idx < count) {
        /*
         * node is not in list, do nothing
         */
        if (!mesh_search_device(nodes + idx)) {
            idx ++;
            continue;
        }

        /*
         * root will be delete, so current mac list is stale
         */
        if (!MESH_DEV_MEMCMP(nodes + idx, &g_node_list.root, sizeof(*nodes))) {
            mesh_device_list_release();
            return true;
        }

        /*
         * delete node from mac list
         */
        for (i = 0; i < sub_count; i ++) {
            if (!MESH_DEV_MEMCMP(nodes + idx, &g_node_list.list[i], sizeof(*nodes))) {
                if (sub_count - i  > 1)
                    MESH_DEV_MEMCPY(&g_node_list.list[i], &g_node_list.list[i + 1],
                            (sub_count - i - 1) * sizeof(*nodes));
                sub_count --;
                g_node_list.scale --;
                i --;
                MESH_DEV_MEMSET(&g_node_list.list[g_node_list.scale], 0, sizeof(*nodes));
                break;
            }
        }
        idx ++;
    }
    return true;
}
