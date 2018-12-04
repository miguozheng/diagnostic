/**  @usd_interface_ext.c
*     @note 2018 crystal-optech Co., Ltd. All Right Reserved.
*     @brief define external interface.
*     @author      zhengl
*     @date        2018/10/29
*     @note
*     @warning 
*/

#include "uds_struct.h"

const UDS_Interface_Ext_t uds_interface_ext_table =
{
	m_can_send_hook
};

void *get_uds_ext_interface(void)
{
	return (void *)&uds_interface_ext_table;
}

