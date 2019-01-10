/**  @usd_interface_internal.c
*     @note 2018 crystal-optech Co., Ltd. All Right Reserved.
*     @brief define internal interface.
*     @author      zhengl
*     @date        2018/10/29
*     @note
*     @warning 
*/

#include "uds_network_layer.h"
#include "uds_session_layer.h"
#include "uds_application_layer.h"
#include "uds_eeprom.h"


#ifdef C99_COMPILER_SUPPOTR
const UDS_Interface_In_t uds_interface_in_table = 
{

	{
		.time_manage_handle = UDS_N_time_manage_handle,
		.main_proc			= uds_network_all,
		.init				= UDS_N_init,
		.can_push_data		= UDS_N_can_data_put,
		.service_get		= UDS_N_service_get,
		.USData_request		= UDS_N_service_process_USData_request,
		.ParaChange_request = UDS_N_service_process_ChangeParameters_request
	},

	{
		.time_manage_handle = UDS_S_time_manage_handle,
		.main_proc			= uds_session_all,
		.init				= UDS_S_init,
		.service_get		= UDS_S_service_get,
		.USData_request		= UDS_S_service_process_USData_request,
		.USData_request		= UDS_S_service_process_ParaChange_request,
		.time_ctl_stop		= UDS_S_time_ctl_stop,
		.time_ctl_run		= UDS_S_time_ctl_run,
		.time_ctl_restart	= UDS_S_time_ctl_restart,
		.time_ctl_reset		= UDS_S_time_ctl_reset,
		.time_status_get	= UDS_S_time_status_get,
		.time_value_get		= UDS_S_time_value_get
	},

	{
		.main_proc			= uds_application_all,
		.init				= UDS_A_init,
		.DTC_add_iterm		= UDS_A_AddDTC_Init
	},
	
	{
		.init				= UDS_EE_init,
		.read				= UDS_EE_tag_read,
		.write				= UDS_EE_tag_write
	}

};
#else
const UDS_Interface_In_t uds_interface_in_table =
{

	{
		UDS_N_time_manage_handle,
		uds_network_all,
		UDS_N_init,
		UDS_N_can_data_put,
		UDS_N_service_get,
		UDS_N_service_process_USData_request,
		UDS_N_service_process_ChangeParameters_request
	},

	{
		UDS_S_time_manage_handle,
		uds_session_all,
		UDS_S_init,
		UDS_S_service_get,
		UDS_S_service_process_USData_request,
		UDS_S_service_process_ParaChange_request,
		UDS_S_time_ctl_stop,
		UDS_S_time_ctl_run,
		UDS_S_time_ctl_restart,
		UDS_S_time_ctl_reset,
		UDS_S_time_status_get,
		UDS_S_time_value_get
	},
	
	{
		uds_application_all,
		UDS_A_init,
		UDS_A_AddDTC_Init
	},

	{
		UDS_EE_init,
		UDS_EE_tag_read,
		UDS_EE_tag_write
	}
}; 

#endif
void *get_uds_internal_interface(void)
{
	return (void *)&uds_interface_in_table;
}


