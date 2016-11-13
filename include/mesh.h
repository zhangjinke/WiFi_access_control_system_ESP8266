#ifndef __LWIP_API_MESH_H__
#define __LWIP_API_MESH_H__

#ifdef ESP_MESH_SUPPORT
#include "ip_addr.h"
#include "user_interface.h"
#include "espconn.h"

//const struct eth_addr mesh_bcast_addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
//const struct eth_addr mesh_mcast_addr = {{0x01, 0x00, 0x5E, 0x00, 0x00, 0x00}};

#define ESP_MESH_VER                    (0)
#define ESP_MESH_GROUP_ID_LEN           (6)
#define ESP_MESH_ADDR_LEN               (6)
#define ESP_MESH_OPTION_MAX_LEN         (255)
#define ESP_MESH_PKT_LEN_MAX            (1300)
#define ESP_MESH_FRAG_ID_MASK           (0xFFFF)
#define ESP_MESH_FRAG_IDX_MASK          (0x3FFF)
#define ESP_MESH_OT_LEN_LEN             (sizeof(uint16_t))
#define ESP_MESH_HLEN                   (sizeof(struct mesh_header_format))
#define ESP_MESH_OPTION_HLEN            (sizeof(struct mesh_header_option_format))
#define ESP_MESH_OP_MAX_PER_PKT         ((ESP_MESH_PKT_LEN_MAX - ESP_MESH_HLEN) / ESP_MESH_OPTION_MAX_LEN)
#define ESP_MESH_DEV_MAX_PER_OP         ((ESP_MESH_OPTION_MAX_LEN - ESP_MESH_OPTION_HLEN) / ESP_MESH_ADDR_LEN)
#define ESP_MESH_DEV_MAX_PER_PKT        (ESP_MESH_OP_MAX_PER_PKT * ESP_MESH_DEV_MAX_PER_OP)
#define ESP_MESH_BCAST_ADDR             (mesh_bcast_addr.addr)
#define ESP_MESH_MCAST_ADDR             (mesh_mcast_addr.addr)

typedef void (* espconn_mesh_callback)(int8_t result);
typedef void (* espconn_mesh_scan_callback)(void *arg, int8_t status);
typedef void (* espconn_mesh_usr_callback)(void *arg);

enum mesh_op_result {
    MESH_ONLINE_SUC = 0,
    MESH_LOCAL_SUC,
    MESH_DISABLE_SUC,
    MESH_SOFTAP_SUC,
    MESH_OP_FAILURE = -1
};
enum mesh_type {
    MESH_CLOSE = 0,
    MESH_LOCAL,
    MESH_ONLINE,
    MESH_SOFTAP,
    MESH_LEAF_LOWPOWER,
    MESH_NONE = 0xFF
};

enum mesh_status {
    MESH_DISABLE = 0,
    MESH_WIFI_CONN,
    MESH_NET_CONN,
    MESH_LOCAL_AVAIL,
    MESH_ONLINE_AVAIL,
    MESH_SOFTAP_AVAIL,
    MESH_SOFTAP_SETUP,
    MESH_LEAF_AVAIL
};

enum mesh_node_type {
    MESH_NODE_PARENT = 0,
    MESH_NODE_CHILD,
    MESH_NODE_ALL
};

struct mesh_sub_node_info {
    uint16_t sub_count;
    uint8_t mac[ESP_MESH_ADDR_LEN];
} __packed;

/* mesh header format:
 * |0 1 2  3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7|
 * __________________________________________________________________
 * |ver|oe|  flags  |    proto      |             len               |
 * ------------------------------------------------------------------
 * |                      dst_addr   _______________________________|
 * |_________________________________|                              |                              |
 * |                                    src_addr                    |
 * |----------------------------------------------------------------|
 * |__________ot_len_________________|      option_list             |
 * |                                                                |
 * |-----------------------------------------------------------------
 *
 * format of flags:
 * | 0 1  2 3 4|
 * -------------
 * |cp|cr| rsv |
 * -------------
 *
 * format of proto:
 * |0  1  2 3 4 5 6 7|
 * -------------------
 * |d|p2p|  protocol |
 * -------------------
 *
 * format of option element:
 * |0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 ......|
 * ------------------------------------------------------------------------
 * |     otype     |     olen      |               ovalue                 |
 * ------------------------------------------------------------------------
 */


struct mesh_header_option_format {
    uint8_t otype;      // option type
    uint8_t olen;       // current option length
    uint8_t ovalue[0];  // option value
} __packed;

struct mesh_header_option_header_type {
    uint16_t ot_len;    // option total length;
    struct mesh_header_option_format olist[0];  // option list
} __packed;

struct mesh_header_option_frag_format {
    uint16_t id;            // identify of fragment
    struct {
        uint16_t resv:1;    // reserve for future
        uint16_t mf:1;      // more fragment
        uint16_t idx:14;    // fragment offset;
    } offset;
} __packed;

struct mesh_header_format {
    uint8_t ver:2;          // version of mesh
    uint8_t oe: 1;          // option exist flag
    uint8_t cp: 1;          // piggyback congest permit in packet
    uint8_t cr: 1;          // piggyback congest request in packet
    uint8_t rsv:3;          // reserve for fulture;
    struct {
        uint8_t d:  1;      // direction, 1:upwards, 0:downwards
        uint8_t p2p:1;      // node to node packet
        uint8_t protocol:6; // protocol used by user data;
    } proto;
    uint16_t len;           // packet total length (include mesh header)
    uint8_t dst_addr[ESP_MESH_ADDR_LEN];  // destiny address
    uint8_t src_addr[ESP_MESH_ADDR_LEN];  // source address
    struct mesh_header_option_header_type option[0];  // mesh option
} __packed;

struct mesh_scan_para_type {
    espconn_mesh_scan_callback usr_scan_cb; // scan done callback
    uint8_t grp_id[ESP_MESH_GROUP_ID_LEN];  // group id
    bool grp_set;                           // group set
};

enum mesh_option_type {
    M_O_CONGEST_REQ = 0,        // congest request option
    M_O_CONGEST_RESP,           // congest response option
    M_O_ROUTER_SPREAD,          // router information spread option
    M_O_ROUTE_ADD,              // route table update (node joins mesh) option
    M_O_ROUTE_DEL,              // route table update (node leaves mesh) option
    M_O_TOPO_REQ,               // topology request option
    M_O_TOPO_RESP,              // topology response option
    M_O_MCAST_GRP,              // group list of mcast
    M_O_MESH_FRAG,              // mesh management fragment option
    M_O_USR_FRAG,               // user data fragment
    M_O_USR_OPTION,             // user option
};

enum mesh_usr_proto_type {
    M_PROTO_NONE = 0,           // used to delivery mesh management packet
    M_PROTO_HTTP,               // user data formated with HTTP protocol
    M_PROTO_JSON,               // user data formated with JSON protocol
    M_PROTO_MQTT,               // user data formated with MQTT protocol
    M_PROTO_BIN,                // user data is binary stream
};

enum mesh_pkt_direct {
    MESH_ROUTE_DOWNLOADS = 0,
    MESH_ROUTE_UPWARDS,
};

enum espnow_dbg_data_type {
    M_FREQ_CAL = 0,     // int16_t
    WIFI_STATUS,        // uint8_t
    FREE_HEAP_SIZE,     // uint16_t
    CHILD_NUM,          // uint8_t
    SUB_DEV_NUM,        // uint16_t
    MESH_STATUS,        // int8_t
    MESH_VERSION,       // string with '\0'
    MESH_ROUTER,        // struct station_config
    MESH_LAYER,         // uint8_t
    MESH_ASSOC,         // uint8_t
    MESH_CHANNEL,       // uint8_t
};

void * espconn_mesh_create_packet(uint8_t *dst_addr, uint8_t *src_addr, bool p2p,
                                  bool piggyback_cr, enum mesh_usr_proto_type proto,
                                  uint16_t data_len, bool option, uint16_t ot_len,
                                  bool frag, enum mesh_option_type frag_type,
                                  bool mf, uint16_t frag_idx, uint16_t frag_id);
void * espconn_mesh_create_option(uint8_t otype, uint8_t *ovalue, uint8_t val_len);
bool espconn_mesh_add_option(struct mesh_header_format *head,
                             struct mesh_header_option_format *option);
bool espconn_mesh_get_option(struct mesh_header_format *head,
                             enum mesh_option_type otype, uint16_t oidx,
                             struct mesh_header_option_format **option);
bool espconn_mesh_get_usr_data(struct mesh_header_format *head,
                               uint8_t **usr_data, uint16_t *data_len);
bool espconn_mesh_set_usr_data(struct mesh_header_format *head,
                               uint8_t *usr_data, uint16_t data_len);
bool espconn_mesh_get_src_addr(struct mesh_header_format *head, uint8_t **src_addr);
bool espconn_mesh_get_dst_addr(struct mesh_header_format *head, uint8_t **dst_addr);
bool espconn_mesh_set_src_addr(struct mesh_header_format *head, uint8_t *src_addr);
bool espconn_mesh_set_dst_addr(struct mesh_header_format *head, uint8_t *dst_addr);
bool espconn_mesh_get_usr_data_proto(struct mesh_header_format *head,
                                     enum mesh_usr_proto_type *proto);
bool espconn_mesh_set_usr_data_proto(struct mesh_header_format *head,
                                     enum mesh_usr_proto_type proto);
bool espconn_mesh_is_root();
bool espconn_mesh_is_root_candidate();
bool espconn_mesh_local_addr(struct ip_addr *ip);
bool espconn_mesh_get_node_info(enum mesh_node_type type,
                                uint8_t **info, uint16_t *count);
bool espconn_mesh_get_router(struct station_config *router);
bool espconn_mesh_set_router(struct station_config *router);
bool espconn_mesh_encrypt_init(AUTH_MODE mode, uint8_t *passwd, uint8_t pw_len);
bool espconn_mesh_group_id_init(uint8_t *grp_id, uint16_t gid_len);
bool espconn_mesh_regist_conn_ready_cb(espconn_mesh_usr_callback cb);
bool espconn_mesh_regist_rebuild_fail_cb(espconn_mesh_usr_callback cb);
bool espconn_mesh_regist_usr_cb(espconn_mesh_usr_callback cb);
bool espconn_mesh_server_init(struct ip_addr *ip, uint16_t port);
bool espconn_mesh_set_max_hops(uint8_t max_hops);
bool espconn_mesh_set_ssid_prefix(uint8_t *prefix, uint8_t prefix_len);

int8_t espconn_mesh_connect(struct espconn *usr_esp);
int8_t espconn_mesh_disconnect(struct espconn *usr_esp);
int8_t espconn_mesh_get_status();
int8_t espconn_mesh_sent(struct espconn *usr_esp, uint8 *pdata, uint16 len);

uint8_t espconn_mesh_get_max_hops();
uint8_t espconn_mesh_layer(struct ip_addr *ip);

uint16_t espconn_mesh_get_sub_dev_count();

void espconn_mesh_enable(espconn_mesh_callback enable_cb, enum mesh_type type);
void espconn_mesh_disable(espconn_mesh_callback disable_cb);
void espconn_mesh_deauth_all();
void espconn_mesh_disp_route_table();
void espconn_mesh_print_ver();
void espconn_mesh_release_congest();
void espconn_mesh_scan(struct mesh_scan_para_type *para);
void espconn_mesh_setup_timer(os_timer_t *timer, uint32_t time,
                              os_timer_func_t cb, void *arg, bool repeat);
#endif

#endif

