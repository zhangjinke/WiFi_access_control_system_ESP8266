/**
 * \file
 * \brief 全局变量
 *
 * \internal
 * \par Modification History
 * - 1.00 17-04-22  zhangjinke, first implementation.
 * \endinternal
 */

#ifndef __MESH_GLOBAL_H
#define __MESH_GLOBAL_H

/*
 * \brief 向mesh网络中发送数据
 *
 * \param[in] dst   目标地址
 * \param[in] data  数据首地址 全0为发送到服务器
 * \param[in] lenth 数据长度
 *
 * \return 无
 */
void ICACHE_FLASH_ATTR esp_mesh_data_send (uint8_t *p_dst, 
                                           uint8_t *p_data, 
                                           uint32_t lenth);

#endif /* __MESH_GLOBAL_H */

/* end of file */
