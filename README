------------------------------------------------------------------------------------------------------
Description:
1. The mesh_demo is used to connect multiple IOT nodes to local server.
2. Every node sends one packet to server per 7 seconds after it joins mesh network.
3. Every node gets mac address of all devices working in mesh per 14 seconds after it joins mesh
   Root device uses espconn_mesh_get_node_info to get mac address list,
   while non-root devices use topology request with bcast to get mac address of all nodes.
4. When local server receives one packet from node, it just echoes the packet back to the node.
5. When node receives packet from server, it parses packet according to protocol,
   and forwards packet to specified protocol parser.
6. Mesh_demo provides current lib of mesh with non-os-sdk.
7. Mesh_demo provides recommendation to setup general protocol parser. (mesh_parser.c)
8. Mesh_demo provides demo to build mesh packet and response packet according to packet received.
9. Mesh_demo provides demo to parse topology packet (mesh_none.c).
10.Mesh_demo provides management for mac address list (mesh_device.c).
11.Mesh_demo provides demo using communication of ucast/bcast/mcast/p2p (mesh_json.c).
------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------
Please follow below steps to run mesh_demo:
1. Setup local server for mesh node using demo_server.py.
2. After local server works successfully, please update server_ip in user_config.h according to your local server.
3. Config MESH_ROUTER_SSID and MESH_ROUTER_PASSWD according to your router in user_config.h.
4. Build run image using gen_misc.sh.
5. Download run image to device, then restart mesh node.
------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------
How to set up local server:
1. Prepare one computer and make sure python script can run on the computer.
2. Run demo_server.py on the computer
------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------
How to build run image:
1. Obtain IP address of local server.
2. Change server_ip in user_config.h according to the IP address of local server.
3. Run ./gen_misc.h
   boot version: boot_v1.2+
   bin generate: user1.bin
   spi speed: 40MHz
   spi mode: QIO
   spi size and map: 1024 (512KB + 512KB)
------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------
Download image to device:
1. Get download tools from the official website of Esressif at http://espressif.com.
2. Download the following four files in ./bin/ to device:
------------------------------------------------------------------------------------------------------
          file                     download_address
---------------------------------------------------------------
esp_init_data_default.bin             0xfc000
blank.bin                             0xfe000
boot_v1.4(b1).bin                     0x00000
upgrade/user1.1024.new2.bin           0x01000
---------------------------------------------------------------
NOTE: Before starting to download, please make sure you have selected the right flash size (8 Mbit).


------------------------------------------------------------------------------------------------------
You can build run image without boot (optional):
1. Run ./gen_misc.h
   boot none
   bin generate: eagle.flash.bin+eagle.irom0text.bin
   spi speed: 40MHz
   spi mode: QIO
   spi size and map: 1024 (512KB + 512KB)
2. Download image to device:
---------------------------------------------------------------
          file                     download_address
---------------------------------------------------------------
eagle.flash.bin                       0x00000
eagle.irom0text.bin                   0x40000
------------------------------------------------------------------------------------------------------
