/**  @UDS_N.c
*     @note 2018 crystal-optech Co., Ltd. All Right Reserved.
*     @brief UDS network deal.
*     @author     ZhengLong
*     @date        2018/09/18
*     @note
*     @warning 
*/

#include "uds_common_include.h"
#include "uds_struct.h"
#include "slist.h"

/*************************1*****************************************************
Private global variables and functions
*******************************************************************************/
//Time parameter config
const  UDS_N_Timing_Parameter_t Time_config = 
{
	.N_As = 100,
	.N_Ar = 100,
	.N_Bs = 1000,
	.N_Br = 800,	
	.N_Cs = 800,
	.N_Cr = 1000,
};
//Rx 
static UDS_N_FC_Para_t 		Rx_FC_para = {50,20};
//Tx 
static UDS_N_FC_Para_t 		Tx_FC_para = {20,20};
//Time control
static UDS_N_Count_Status_t Time_Man[N_TIME_NAME_N_ALL];
//CAN PDU pool
static UDS_N_PDU_Pool_t 	PDU_pool[UDS_N_CAN_PDU_POOL_MAX];
//Service issue pool
static UDS_N_Services_Issue_t	 	Issue_pool[UDS_N_SERVICE_ISSUE_POOL_MAX];

//slist
static Slist 				Put_data_rx_slist; 
static Slist 				Put_data_tx_slist; 
static Slist 				Issue_upper_slist; 
static Slist 				Issue_nework_slist; 
static Slist 				Can_send_slist;


//Network control
static UDS_N_Control_t		Network_ctl = 
{

};
//CAN recive buffer manage
static uds_uint8_t								 USD_can_rx_buf_man[UDS_N_CAN_RX_BUFFER_NUM];
//CAN recive buffer
static UDS_N_Rx_Buffer_t						 USD_can_rx_buffer[UDS_N_CAN_RX_BUFFER_NUM];
//UDS buffer
static uds_uint8_t 						 USD_rx_buffer[UDS_N_RX_BUFFER_MAX];
//Temp data
static UDS_N_Services_t					 Network_service;


//
/*
============================================================================
 Function declear
============================================================================
*/
static uds_uint8_t (*UDS_N_can_send_hook)(uds_uint32_t id,
										  uds_uint8_t length,
										  uds_uint8_t *data);
static void UDS_N_register_can_send_interface(uds_uint8_t (*_handle)(uds_uint32_t id,
																	 uds_uint8_t length,
																	 uds_uint8_t *data));
static void UDS_N_pool_init(void);
static void UDS_N_slist_init(void);
static void UDS_N_time_manage_init(void);
static UDS_N_Count_Status_t *UDS_N_time_pointer_get(uds_uint8_t time_type);
static uds_int8_t UDS_N_time_ctl_stop(UDS_N_Time_Name_e time);
static uds_int8_t UDS_N_time_ctl_run(UDS_N_Time_Name_e time);
static uds_int8_t UDS_N_time_ctl_restart(UDS_N_Time_Name_e time);
static uds_int8_t UDS_N_time_ctl_reset(UDS_N_Time_Name_e time);
static uds_int8_t UDS_N_time_status_get(UDS_N_Time_Name_e time);
static void *UDS_N_pool_pointer_get(UDS_N_Pool_Type_e type);
static void UDS_N_init_can_recive_buf(void);
static uds_int8_t UDS_N_malloc_can_recive_buf(void);
static uds_uint8_t *UDS_N_get_can_recive_buf(uds_uint8_t index);
static uds_int8_t UDS_N_free_can_recive_buf(uds_uint8_t index);
static uds_int8_t UDS_N_pool_pointer_free(UDS_N_Pool_Type_e type,
										  void *pool_pointer);
static uds_int8_t UDS_N_issue_to_upper_put(UDS_N_Service_e type,
										   UDS_N_Services_t *service);
static uds_int8_t UDS_N_issue_to_network_put(UDS_N_Service_e type,
											 UDS_N_Services_t *service);
static uds_int8_t UDS_N_add_node_to_list(Slist *list,void *data);
static uds_int8_t UDS_N_list_data_get(UDS_N_List_Type_e list_type,
									  void **pread);
static uds_int8_t UDS_N_generate_USData_indication(UDS_N_Services_t *service,
												   UDS_N_Result_e result,
												   uds_uint8_t *buf);
static uds_int8_t UDS_N_generate_USData_FF_indication(UDS_N_PDU_t *pdu);
static uds_int8_t UDS_N_generate_USData_confirm(UDS_N_Services_t *service,
												UDS_N_Result_e result);
static uds_int8_t UDS_N_generate_ChangeParameter_confirm(UDS_N_Services_t *service,
														 UDS_N_Change_Parameters_Request_t *request,
														 UDS_N_Result_Change_Parameter_e result);

static uds_int8_t UDS_N_rx_ctl_set_status(UDS_N_Rx_Status_e status);
static uds_int8_t UDS_N_tx_ctl_set_status(UDS_N_Tx_Status_e status);
static uds_uint8_t UDS_N_ctl_service_is_busy(void);
static void UDS_N_ctl_service_set(void);
static void UDS_N_ctl_service_clear(void);
static UDS_N_Rx_Status_e UDS_N_rx_ctl_get_status(void);
static UDS_N_Tx_Status_e UDS_N_tx_ctl_get_status(void);
static uds_uint8_t UDS_N_get_status(void);
static uds_uint16_t UDS_N_rx_ctl_get_current_frame(void);
static uds_uint16_t UDS_N_tx_ctl_get_current_frame(void);
static uds_uint16_t UDS_N_rx_ctl_get_target_frame(void);
static uds_uint16_t UDS_N_tx_ctl_get_target_frame(void);
static uds_int8_t UDS_N_rx_ctl_package_is_finish(void);
static uds_int8_t UDS_N_tx_ctl_package_is_finish(void);
static uds_int8_t UDS_N_rx_ctl_set_target_frame(uds_uint16_t length);
static uds_int8_t UDS_N_rx_ctl_add_current_frame(void);
static uds_int8_t UDS_N_tx_ctl_add_current_frame(void);
static uds_int8_t UDS_N_rx_ctl_pre_next_BS(void);
static uds_int8_t UDS_N_tx_ctl_pre_next_BS(void);
static uds_int8_t UDS_N_rx_ctl_set_target_BS(uds_uint8_t bs);
static uds_int8_t UDS_N_tx_ctl_set_target_BS(uds_uint8_t bs);
static uds_uint16_t UDS_N_rx_ctl_get_target_BS(void);
static uds_uint16_t UDS_N_tx_ctl_get_target_BS(void);
static uds_int8_t UDS_N_rx_ctl_add_current_BS(void);
static uds_int8_t UDS_N_tx_ctl_add_current_BS(void);
static uds_int8_t UDS_N_rx_ctl_BS_is_finish(void);
static uds_int8_t UDS_N_tx_ctl_BS_is_finish(void);
static uds_int8_t UDS_N_tx_ctl_set_target_frame(uds_uint16_t length);
static uds_uint16_t UDS_N_rx_ctl_buffer_pointer_current_get(void);
static uds_uint8_t UDS_N_rx_ctl_buffer_index_get(void);
static uds_uint16_t UDS_N_rx_ctl_buffer_pointer_target_get(void);
static uds_uint16_t UDS_N_tx_ctl_buffer_pointer_target_get(void);
static void UDS_N_rx_ctl_reset_for_new(uds_uint16_t length);
static void UDS_N_tx_ctl_reset_for_new(UDS_N_USData_Request_t *mes);
static uds_uint8_t UDS_N_rx_ctl_get_SN_last(void);
static void UDS_N_rx_ctl_set_SN_last(uds_uint8_t sn);
static uds_uint8_t UDS_N_tx_ctl_get_SN_last(void);
static void UDS_N_tx_ctl_set_SN_last(uds_uint8_t sn);
static void *UDS_N_tx_ctl_sevice_info_get(void);
static UDS_N_Service_Info_Pub_t *UDS_N_rx_ctl_sevice_info_get(void);
static uds_int8_t UDS_N_push_to_can_send_list(void *data);
static uds_int8_t UDS_N_rx_send_FC_frame(UDS_N_FC_Ctl_e fc);
static uds_int8_t UDS_N_rx_push_data_to_buffer(uds_uint8_t *data,
											   uds_uint8_t length);
static uds_int8_t UDS_N_tx_pull_data_from_buffer(uds_uint8_t *data,
												 uds_uint8_t length);
static uds_int8_t UDS_N_rx_SF_deal(void *pdata);
static uds_int8_t UDS_N_rx_unexpected_SF_deal(void *pdata);
static uds_int8_t UDS_N_rx_FF_deal(void *pdata);
static uds_int8_t UDS_N_rx_unexpected_FF_deal(void *pdata);
static uds_int8_t UDS_N_rx_CF_deal(void *pdata);
static uds_uint16_t UDS_N_BS_decode(uds_uint8_t bs);
static uds_uint8_t UDS_N_BS_encode(uds_uint16_t bs);
static uds_uint8_t UDS_N_ST_decode(uds_uint8_t st);
static uds_uint8_t UDS_N_ST_encode(uds_uint8_t st);
static void UDS_N_set_send_para(uds_uint8_t bs,uds_uint8_t st);
static void UDS_N_set_recive_para(uds_uint8_t bs,uds_uint8_t st);
static uds_int8_t UDS_N_tx_FC_CTS_deal(void *pdata);
static uds_int8_t UDS_N_tx_FC_WT_deal(void *pdata);
static uds_int8_t UDS_N_tx_FC_OVLFW_deal(void *pdata);
static uds_int8_t UDS_N_tx_FC_deal(void *pdata);
static uds_int8_t UDS_N_rx_message_process_normal(void *pdata);
static uds_int8_t UDS_N_rx_message_process_muilt_frame(void *pdata);
static uds_int8_t UDS_N_rx_CF_length_check(void *pdata);
static uds_int8_t UDS_N_rx_frame_check(void *pdata);
static uds_int8_t UDS_N_rx_message_process(void *pdata);
static uds_int8_t UDS_N_tx_message_process_muilt_frame(void *pdata);
static uds_int8_t UDS_N_tx_message_process(void *pdata);
static void UDS_N_rx_can_recive_data_deal(void);
static void UDS_N_tx_can_recive_data_deal(void);
static uds_int8_t UDS_N_can_send_process(void *pdata);
static void UDS_N_can_send_data_deal(void);
static uds_int8_t UDS_N_service_process(void *pdata);
static void UDS_N_service_deal(void);
static void UDS_N_rx_FC_send_result_hook(uds_int8_t res);
static void UDS_N_tx_SF_send_result_hook(uds_int8_t res);
static void UDS_N_tx_FF_send_result_hook(uds_int8_t res);
static void UDS_N_tx_CF_send_result_hook(uds_int8_t res);
static uds_int8_t UDS_N_tx_generate_SF_frame(void **res);
static uds_int8_t UDS_N_tx_generate_FF_frame(void **res);
static uds_int8_t UDS_N_tx_generate_CF_frame(void **res);
static void UDS_N_send_manage_deal(void);
static void UDS_N_Ar_time_out_reply(void);
static void UDS_N_As_time_out_reply(void);
static void UDS_N_Br_time_out_reply(void);
static void UDS_N_Bs_time_out_reply(void);
static void UDS_N_Cr_time_out_reply(void);
static void UDS_N_Cs_time_out_reply(void);
static void UDS_N_time_manage_deal(void);
static uds_int8_t UDS_N_service_data_copy(UDS_N_Services_t *des,
										  UDS_N_Services_Issue_t *res,
										  uds_uint8_t *buf);


/*
============================================================================
 User function
============================================================================
*/
//Register can send handle for network frame transmission.
static void UDS_N_register_can_send_interface(uds_uint8_t (*_handle)(uds_uint32_t id,
																	 uds_uint8_t length,
																	 uds_uint8_t *data))
{
	if(_handle){
		UDS_N_can_send_hook = _handle;
	}else{
		UDS_PRINTF("UDS Error::can send inerface register failed!");
	}
}
//recive pool init
static void UDS_N_pool_init(void)
{
	uds_uint8_t *pool;

	pool = (uds_uint8_t *)PDU_pool;
	memset(pool,0,sizeof(PDU_pool));
	pool = (uds_uint8_t *)Issue_pool;
	memset(pool,0,sizeof(Issue_pool));
	memset(USD_can_rx_buffer,0,sizeof(USD_can_rx_buffer));
	memset(USD_rx_buffer,0,sizeof(USD_rx_buffer));
}

//list init
static void UDS_N_slist_init(void)
{
	slist_init(&Put_data_rx_slist,50);
	slist_init(&Put_data_tx_slist,10);
	slist_init(&Issue_upper_slist,5);
	slist_init(&Issue_nework_slist,2);
	slist_init(&Can_send_slist,20);
}

//Time init
static void UDS_N_time_manage_init(void)
{
	UDS_N_Count_Status_t	 *ptime = Time_Man;
	uds_uint16_t *config = (uds_uint16_t *)&Time_config;
	uds_uint8_t i;

	for(i = 0;i < N_TIME_NAME_N_ALL;i++,ptime++){
		ptime->Cnt = 0;
		ptime->Status = N_TIME_CNT_STS_IDLE;
		ptime->Cnt_en = 0;
		ptime->Target = config[i];
	}
}

/**********************************************************************
* @brief	   Network time manage handle.
*
* @param[in]   None.
*
* @return	   None
**********************************************************************/
void UDS_N_time_manage_handle(void)
{
	UDS_N_Count_Status_t	*ptime = Time_Man;
	uds_uint8_t i;

	for(i = 0;i < N_TIME_NAME_N_ALL;i++,ptime++){
		if(ptime->Cnt_en){
			if(ptime->Cnt){
				ptime->Cnt--;
				ptime->Status = N_TIME_CNT_STS_RUN;
			}else{
				ptime->Status = N_TIME_CNT_STS_TIMEOUT;
			}
		}
	}
}
/**********************************************************************
* @brief	   Init network layer
*
* @param[in]   _handle:CAN message send interface. 
*
* @return	   None
**********************************************************************/
uds_int8_t UDS_N_init(void)
{
	UDS_Interface_Ext_t *intf_ext = (UDS_Interface_Ext_t *)get_uds_ext_interface();
	uds_int8_t ret = -1;
	
	if(intf_ext->can_send_hook){
		UDS_N_register_can_send_interface(intf_ext->can_send_hook);
	}else{
		UDS_N_register_can_send_interface(UDS_NULL);
		UDS_PRINTF("UDS Error:Network  regist can_send_hook failed!\n\r");
		return ret;
	}
	ret = 1;
	UDS_N_pool_init();
	UDS_N_slist_init();
	UDS_N_time_manage_init();
	UDS_N_init_can_recive_buf();

	return ret;
}
static UDS_N_Count_Status_t *UDS_N_time_pointer_get(uds_uint8_t time_type)
{
	UDS_N_Count_Status_t *ptime = UDS_NULL;

	if(N_TIME_NAME_N_ALL > time_type){
		ptime = &Time_Man[time_type];
	}

	return ptime;
}

//Stop time count
static uds_int8_t UDS_N_time_ctl_stop(UDS_N_Time_Name_e time)
{
	UDS_N_Count_Status_t	*ptime = UDS_N_time_pointer_get(time);
	uds_int8_t ret = -1;

	if(!ptime){
		return ret;
	}

	ptime->Cnt_en = 0;
	ptime->Status = N_TIME_CNT_STS_STOP;
	ret = 1;
	
	return ret;
}
//Run time count
static uds_int8_t UDS_N_time_ctl_run(UDS_N_Time_Name_e time)
{
	UDS_N_Count_Status_t	*ptime = UDS_N_time_pointer_get(time);
	uds_int8_t ret = -1;

	if(!ptime){
		return ret;
	}

	ptime->Cnt_en = 1;
	ptime->Status = N_TIME_CNT_STS_RUN;
	ret = 1;
	
	return ret;
}
//Stop time count
static uds_int8_t UDS_N_time_ctl_restart(UDS_N_Time_Name_e time)
{
	UDS_N_Count_Status_t	*ptime = UDS_N_time_pointer_get(time);
	uds_int8_t ret = -1;

	if(!ptime){
		return ret;
	}

	ptime->Cnt_en = 0;
	ptime->Status = N_TIME_CNT_STS_IDLE;
	ptime->Cnt = ptime->Target;
	ptime->Cnt_en = 1;
	ret = 1;
	
	return ret;
}
//Reset time count
static uds_int8_t UDS_N_time_ctl_reset(UDS_N_Time_Name_e time)
{
	UDS_N_Count_Status_t	*ptime = UDS_N_time_pointer_get(time);
	uds_int8_t ret = -1;

	if(!ptime){
		return ret;
	}

	ptime->Cnt_en = 0;
	ptime->Status = N_TIME_CNT_STS_IDLE;
	ptime->Cnt = ptime->Target;
	ret = 1;
	
	return ret;
}
//Get time count status
static uds_int8_t UDS_N_time_status_get(UDS_N_Time_Name_e time)
{
	UDS_N_Count_Status_t	*ptime = UDS_N_time_pointer_get(time);
	uds_int8_t ret = -1;

	if(!ptime){
		return ret;
	}
	
	ret = ptime->Status;
	
	return ret;
}
//Init can recive buffer manage
static void UDS_N_init_can_recive_buf(void)
{
	uds_uint8_t i = 0;

	for(i = 0;UDS_N_CAN_RX_BUFFER_NUM > i;i++){
		USD_can_rx_buf_man[i] = 0x00;
	}
}

//Get can recive buffer
static uds_int8_t UDS_N_malloc_can_recive_buf(void)
{
	uds_int8_t ret = -1;
	uds_uint8_t i = 0;

	for(i = 0;UDS_N_CAN_RX_BUFFER_NUM > i;i++){
		if(0 == USD_can_rx_buf_man[i]){
			ret = i;
			USD_can_rx_buf_man[i] = 0xFF;
			break;
		}
	}
	if(-1 == ret){
		UDS_PRINTF("UDS Error:CAN recive buffer pool exceed!\n");
	}
	
	return ret;
}
//Get can recive buffer
static uds_uint8_t *UDS_N_get_can_recive_buf(uds_uint8_t index)
{
	uds_uint8_t *ret = UDS_NULL;

	if(UDS_N_CAN_RX_BUFFER_NUM > index){
		ret = USD_can_rx_buffer[index].buf;
	}

	return ret;
}

//Free can recive buffer
static uds_int8_t UDS_N_free_can_recive_buf(uds_uint8_t index)
{
	uds_int8_t ret = -1;

	if(UDS_N_CAN_RX_BUFFER_NUM > index){
		USD_can_rx_buf_man[index] = 0x00;
		ret = 1;
	}
	return ret;
}

//Get memory pool pointer
static void *UDS_N_pool_pointer_get(UDS_N_Pool_Type_e type)
{
	void *ret = UDS_NULL;
	uds_uint16_t i;

	if(N_POOL_TYPE_END > type){
		switch(type){
			case N_POOL_TYPE_PDU:
				for(i = 0;i < UDS_N_CAN_PDU_POOL_MAX;i++){
					if(0 == PDU_pool[i].Status){
						ret = (void *)(&PDU_pool[i]);
						PDU_pool[i].Status = 0xFF;
						break;
					}
				}
				if(UDS_N_CAN_PDU_POOL_MAX == i){
					UDS_PRINTF("UDS Error:PDU pool is over flow!\n");
				}
				break;
			case N_POOL_TYPE_ISSUE:
				for(i = 0;i < UDS_N_SERVICE_ISSUE_POOL_MAX;i++){
					if(0 == Issue_pool[i].Status){
						ret = (void *)(&Issue_pool[i]);
						Issue_pool[i].Status = 0xFF;
						break;
					}
				}
				if(UDS_N_SERVICE_ISSUE_POOL_MAX == i){
					UDS_PRINTF("UDS Error:Service issue pool is over flow!\n");
				}
				break;
			default:break;
		}	
	}

	return ret;
}
//Free memory pool pointer
static uds_int8_t UDS_N_pool_pointer_free(UDS_N_Pool_Type_e type,
										  void *pool_pointer)
{
	uds_uint8_t	*pool = (uds_uint8_t *)pool_pointer;
	uds_int8_t ret = -1;

	if(!pool){
		return ret;
	}

	if(N_POOL_TYPE_END > type){
		switch(type){
			case N_POOL_TYPE_PDU:
				memset(pool,0,sizeof(UDS_N_PDU_Pool_t));
				ret = 1;
				break;
			case N_POOL_TYPE_ISSUE:
				memset(pool,0,sizeof(UDS_N_Services_Issue_t));
				ret = 1;
				break;
			default:break;
		}	
	}

	return ret;
}

//Put issue to upper pool
static uds_int8_t UDS_N_issue_to_upper_put(UDS_N_Service_e type,
										   UDS_N_Services_t *service)
{
	UDS_N_Services_Issue_t	*tpool = UDS_N_pool_pointer_get(N_POOL_TYPE_ISSUE);
	uds_int8_t ret = -1;
	UDS_N_Services_t *pdata = service;
	Snode *pnode = UDS_NULL;
	Slist *plist = &Issue_upper_slist;

	if((!tpool) || (!pdata) || (UDS_N_RX_BUFFER_MAX < pdata->USData_request.Length)){
		return ret;
	}

	ret = 0;
	switch(type){
		case N_USDATA_CONFIRM:
			tpool->Type = N_USDATA_CONFIRM;
			tpool->Buf_index = 0xFF;
			tpool->Service.USData_confirm = pdata->USData_confirm;
			ret = 1;
			break;
		case N_USDATA_FF_INDICATION:
			tpool->Type = N_USDATA_FF_INDICATION;
			tpool->Buf_index = 0xFF;
			tpool->Service.USData_FF_indication = pdata->USData_FF_indication;
			ret = 1;
			break;
		case N_USDATA_INDICATION:
			tpool->Type = N_USDATA_INDICATION;
			tpool->Buf_index = UDS_N_rx_ctl_buffer_index_get();
			tpool->Service.USData_Indication = pdata->USData_Indication;
			ret = 1;
			break;
		case N_CHANGE_PARAMETER_CONFIRM:
			tpool->Type = N_CHANGE_PARAMETER_CONFIRM;
			tpool->Buf_index = 0xFF;
			tpool->Service.Change_parameters_confirm = pdata->Change_parameters_confirm;
			ret = 1;
			break;
		default:break;
	}
	//make node and insert
	if(ret){
		ret = UDS_N_add_node_to_list(plist,(void *)tpool);
		if(1 != ret){
			UDS_N_pool_pointer_free(N_POOL_TYPE_ISSUE,tpool);
		}
	}else{
		UDS_N_pool_pointer_free(N_POOL_TYPE_ISSUE,tpool);
	}
	
	return ret;
}


//Put issue to network pool
static uds_int8_t UDS_N_issue_to_network_put(UDS_N_Service_e type,
											 UDS_N_Services_t *service)
{
	UDS_N_Services_Issue_t	*tpool = UDS_N_pool_pointer_get(N_POOL_TYPE_ISSUE);
	uds_int8_t ret = -1;
	UDS_N_Services_t *pdata = service;
	uds_uint16_t i;
	Slist *plist = &Issue_nework_slist;

	if((!tpool) || (!pdata) || (UDS_N_RX_BUFFER_MAX < pdata->USData_request.Length)){
		return ret;
	}

	ret = 0;
	if(UDS_N_ctl_service_is_busy()){//network busy
		 
	}else{
		switch(type){
			case N_USDATA_REQUEST:
				tpool->Type = N_USDATA_REQUEST;
				tpool->Service.USData_request = pdata->USData_request;
				tpool->Service.USData_request.MessageData = USD_rx_buffer;
				for(i = 0;i < pdata->USData_request.Length;i++){
					tpool->Service.USData_request.MessageData[i] = pdata->USData_request.MessageData[i];
				}
				UDS_N_ctl_service_set();
				ret = 1;
				break;
			case N_CHANGE_PARAMETER_REQUEST:
				tpool->Type = N_CHANGE_PARAMETER_REQUEST;
				tpool->Service.Change_parameters_request = pdata->Change_parameters_request;
				UDS_N_ctl_service_set();
				ret = 1;
				break;
			default:break;
		}
	}
	//make node and insert
	if(ret){
		ret = UDS_N_add_node_to_list(plist,(void *)tpool);
		if(1 != ret){
			UDS_N_pool_pointer_free(N_POOL_TYPE_ISSUE,tpool);
		}
	}else{
		UDS_N_pool_pointer_free(N_POOL_TYPE_ISSUE,tpool);
	}
	
	return ret;
}

//make node and insert to list
static uds_int8_t UDS_N_add_node_to_list(Slist *list,void *data)
{
	uds_int8_t ret = -1;
	Slist *plist = list;
	void *pdata = data;
	Snode *pnode = UDS_NULL;

	if((!pdata) || (!plist)){
		return ret;
	}

	ret = 0;
	//make node
	pnode = slist_node_make(pdata);
	//insert list
	if(UDS_NULL != pnode){
		if(0 == slist_is_full(plist)){//list not full
			slist_node_insert_queue(plist,pnode);
			ret = 1;
		}else{
			slist_node_free(pnode);
		}
	}

	return ret;
}

/**********************************************************************
* @brief	   Push the recive message to network.
*
* @param[in]   id	 :CAN message identify. 
*			   length:CAN message data length.
*			   data  :CAN message data.
*
* @return	   1    :success 
*			   other:error
**********************************************************************/
uds_int8_t UDS_N_can_data_put(uds_uint32_t id,
							  uds_uint8_t length,
							  uds_uint8_t *data)
{
	UDS_N_PDU_Pool_t	*tpool = UDS_N_pool_pointer_get(N_POOL_TYPE_PDU);
	uds_int8_t ret = -1,i,*pdata = data;
	Slist *plist = &Put_data_rx_slist;

	if((!tpool) || (!pdata)){
		return ret;
	}

	ret = 0;
	tpool->PDU.ID = id;
	tpool->PDU.Length = length;
	for(i = 0;i < length;i++){
		tpool->PDU.Frame.Data[i] = pdata[i];
	}
	tpool->PDU.Length = length;	
	if(N_PDU_NAME_FC < tpool->PDU.Frame.PCI.PCI_Bit.PCItype){//PCI error
		return ret;
	}
	//put in the tx list
	if(N_PDU_NAME_FC == tpool->PDU.Frame.PCI.PCI_Bit.PCItype){
		plist = &Put_data_tx_slist;
	}
	//make node and insert
	ret = UDS_N_add_node_to_list(plist,(void *)tpool);
	if(1 != ret){
		UDS_N_pool_pointer_free(N_POOL_TYPE_PDU,tpool);
	}
	
	return ret;
}

//Get list node data
static uds_int8_t UDS_N_list_data_get(UDS_N_List_Type_e list_type,
									  void **pread)
{
	uds_int8_t ret = -1;
	Snode *pnode = UDS_NULL;
	Slist *plist = UDS_NULL;

	if(N_LIST_TYPE_END <= list_type){
		return ret;
	}

	switch(list_type){
		case N_LIST_TYPE_RX_CAN_RECIVE:
			plist = &Put_data_rx_slist;
			break;
		case N_LIST_TYPE_TX_CAN_RECIVE:
			plist = &Put_data_tx_slist;
			break;
		case N_LIST_TYPE_ISSUE_UPPER:
			plist = &Issue_upper_slist;
			break;
		case N_LIST_TYPE_ISSUE_NETWORK:
			plist = &Issue_nework_slist;
			break;
		case N_LIST_TYPE_CAN_SEND:
			plist = &Can_send_slist;
			break;
		default:break;
	}
	if(!plist){
		return ret;
	}
	//get list data
	if(slist_is_empty(plist)){
		ret = 0;
	}else{
		//get head
		pnode = slist_get_head(plist);
		//get first
		pnode = slist_get_next(pnode);
		//get data
		*pread = slist_node_get_data(pnode);
		//remove node
		slist_node_remove(plist,pnode);
		ret = 1;
	}

	return ret;
}
//Generate USData.indication
static uds_int8_t UDS_N_generate_USData_indication(UDS_N_Services_t *service,
												   UDS_N_Result_e result,
												   uds_uint8_t *buf)
{
	uds_int8_t ret = -1;
	UDS_N_Service_Info_Pub_t *data_pdu = UDS_N_rx_ctl_sevice_info_get();
	UDS_N_Services_t *pindication = service;
	uds_uint16_t length = UDS_N_rx_ctl_buffer_pointer_current_get();

	if((!data_pdu) || (!buf)){
		return ret;
	}

#ifdef ADDRESS_EXTENSION_MODE
	pindication->USData_Indication.Info.N_AE		 = data_pdu->N_AE;
#endif
	pindication->USData_Indication.Info.Mtype		 = data_pdu->Mtype;
	pindication->USData_Indication.Info.N_SA		 = data_pdu->N_SA;
	pindication->USData_Indication.Info.N_TA		 = data_pdu->N_TA;
	pindication->USData_Indication.Info.N_TAtype	 = data_pdu->N_TAtype;
	pindication->USData_Indication.Length 	 	 	 = length;
	pindication->USData_Indication.MessageData 		 = buf;
	pindication->USData_Indication.N_Result	 		 = result;
	ret = 1;

	return ret;
}
//Generate USData_FF.indication
static uds_int8_t UDS_N_generate_USData_FF_indication(UDS_N_PDU_t *pdu)
{
	uds_int8_t ret = -1;
	UDS_N_Service_Info_Pub_t *data_info = UDS_N_rx_ctl_sevice_info_get();
	UDS_N_PDU_t *data_pdu = pdu;
	UDS_N_Services_t *pindication = &Network_service;

	if(!data_pdu){
		return ret;
	}

#ifdef ADDRESS_EXTENSION_MODE
	pindication->USData_FF_indication.Info.N_AE		 = data_info->N_AE;
#endif
	pindication->USData_FF_indication.Info.Mtype	 = data_info->Mtype;
	pindication->USData_FF_indication.Info.N_SA		 = data_info->N_SA;
	pindication->USData_FF_indication.Info.N_TA		 = data_info->N_TA;
	pindication->USData_FF_indication.Info.N_TAtype	 = data_info->N_TAtype;
	pindication->USData_FF_indication.Length 		 = data_pdu->Frame.FF.PCI_Bit.DL_MSB * 256 + data_pdu->Frame.FF.DL_LSB;
	ret = 1;

	return ret;
}

//Generate USData.indication
static uds_int8_t UDS_N_generate_USData_confirm(UDS_N_Services_t *service,
												UDS_N_Result_e result)
{
	uds_int8_t ret = -1;
	UDS_N_Services_t *pconfirm = service;
	UDS_N_Service_Info_Pub_t *pservice_info = (UDS_N_Service_Info_Pub_t *)UDS_N_tx_ctl_sevice_info_get();

	if((!pconfirm) || (!pservice_info)){
	  return ret;
	}

#ifdef ADDRESS_EXTENSION_MODE
	pconfirm->USData_confirm.Info.N_AE	   = pservice_info->N_AE;
#endif
	pconfirm->USData_confirm.Info.Mtype    = pservice_info->Mtype;
	pconfirm->USData_confirm.Info.N_SA	   = pservice_info->N_SA;
	pconfirm->USData_confirm.Info.N_TA	   = pservice_info->N_TA;
	pconfirm->USData_confirm.Info.N_TAtype = pservice_info->N_TAtype;
	pconfirm->USData_confirm.N_Result	   = result;
	ret = 1;

	return ret;
}

//Generate ChangeParameter.confirm
static uds_int8_t UDS_N_generate_ChangeParameter_confirm(UDS_N_Services_t *service,
														 UDS_N_Change_Parameters_Request_t *request,
														 UDS_N_Result_Change_Parameter_e result)
{
	uds_int8_t ret = -1;
	UDS_N_Services_t *pconfirm = service;
	UDS_N_Change_Parameters_Request_t *pservice = request;

	if((!pconfirm) || (!pservice)){
	  return ret;
	}

#ifdef ADDRESS_EXTENSION_MODE
	pconfirm->Change_parameters_confirm.Info.N_AE			   = pservice->Info.N_AE;
#endif
	pconfirm->Change_parameters_confirm.Info.Mtype			   = pservice->Info.Mtype;
	pconfirm->Change_parameters_confirm.Info.N_SA			   = pservice->Info.N_SA;
	pconfirm->Change_parameters_confirm.Info.N_TA			   = pservice->Info.N_TA;
	pconfirm->Change_parameters_confirm.Info.N_TAtype		   = pservice->Info.N_TAtype;
	pconfirm->Change_parameters_confirm.Parameter			   = pservice->Parameter;
	pconfirm->Change_parameters_confirm.Result_ChangeParameter = result;
	ret = 1;

	return ret;
}

static void UDS_N_rx_ctl_clear(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	memset(&pnetwork_ctl->Rx_frame_ctl,0,sizeof(UDS_N_Multi_Ctl_t));
}
static void UDS_N_tx_ctl_clear(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	memset(&pnetwork_ctl->Tx_frame_ctl,0,sizeof(UDS_N_Multi_Ctl_t));
}
static void UDS_N_rx_info_clear(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	memset(&pnetwork_ctl->Rx_service_info,0,sizeof(UDS_N_Service_Info_Pub_t));
}

static void UDS_N_tx_info_clear(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	memset(&pnetwork_ctl->Tx_service_info,0,sizeof(UDS_N_Service_Info_Pub_t));
}

static uds_int8_t UDS_N_rx_ctl_set_status(UDS_N_Rx_Status_e status)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	uds_int8_t ret = -1;

	if(N_STS_RX_ERR >= status){
		pnetwork_ctl->Status.Bit.Rx_status = status;
		ret = 1;
	}

	return ret;
}
static uds_int8_t UDS_N_tx_ctl_set_status(UDS_N_Tx_Status_e status)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	uds_int8_t ret = -1;

	if(N_STS_TX_ERR >= status){
		pnetwork_ctl->Status.Bit.Tx_status = status;
		ret = 1;
	}

	return ret;
}

static uds_uint8_t UDS_N_ctl_service_is_busy(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Service_request_flag;
}

void UDS_N_ctl_service_set(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	pnetwork_ctl->Service_request_flag = 1;
}

static void UDS_N_ctl_service_clear(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	pnetwork_ctl->Service_request_flag = 0;
}


static UDS_N_Rx_Status_e UDS_N_rx_ctl_get_status(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Status.Bit.Rx_status;
}

static UDS_N_Tx_Status_e UDS_N_tx_ctl_get_status(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Status.Bit.Tx_status;
}

static uds_uint8_t UDS_N_get_status(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Status.N_status;
}

static uds_uint16_t UDS_N_rx_ctl_get_current_frame(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Rx_frame_ctl.Frame_cnt_current;
}

static uds_uint16_t UDS_N_tx_ctl_get_current_frame(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Tx_frame_ctl.Frame_cnt_current;
}

static uds_uint16_t UDS_N_rx_ctl_get_target_frame(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Rx_frame_ctl.Frame_cnt_taget;
}

static uds_uint16_t UDS_N_tx_ctl_get_target_frame(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Tx_frame_ctl.Frame_cnt_taget;
}

static uds_int8_t UDS_N_rx_ctl_package_is_finish(void)
{
	uds_int8_t ret = 0;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(pnetwork_ctl->Rx_frame_ctl.Frame_cnt_taget == pnetwork_ctl->Rx_frame_ctl.Frame_cnt_current){
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_tx_ctl_package_is_finish(void)
{
	uds_int8_t ret = 0;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(pnetwork_ctl->Tx_frame_ctl.Frame_cnt_taget == pnetwork_ctl->Tx_frame_ctl.Frame_cnt_current){
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_rx_ctl_set_target_frame(uds_uint16_t length)
{
	uds_int8_t ret = -1;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	uds_uint16_t frame_length,temp;

	if(length > SF_DATA_LENGTH){
		temp = length - FF_DATA_LENGTH;
		frame_length = 1 + temp / CF_DATA_LENGTH;
		if(temp%CF_DATA_LENGTH){
			frame_length++;
		}
		pnetwork_ctl->Rx_frame_ctl.Frame_cnt_taget = frame_length;
		pnetwork_ctl->Rx_frame_ctl.Buffer_pointer_target = length;
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_rx_ctl_add_current_frame(void)
{
	uds_int8_t ret = -1;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(pnetwork_ctl->Rx_frame_ctl.Frame_cnt_taget > pnetwork_ctl->Rx_frame_ctl.Frame_cnt_current){
		pnetwork_ctl->Rx_frame_ctl.Frame_cnt_current++;
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_tx_ctl_add_current_frame(void)
{
	uds_int8_t ret = -1;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(pnetwork_ctl->Tx_frame_ctl.Frame_cnt_taget > pnetwork_ctl->Tx_frame_ctl.Frame_cnt_current){
		pnetwork_ctl->Tx_frame_ctl.Frame_cnt_current++;
		ret = 1;
	}

	return ret;
}
static uds_int8_t UDS_N_rx_ctl_pre_next_BS(void)
{
	uds_int8_t ret = 0;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	UDS_N_FC_Para_t *para = &Rx_FC_para;
	uds_uint8_t wait_rx_frame;
	uds_uint16_t bs = UDS_N_BS_decode(para->BS);
	
	wait_rx_frame = pnetwork_ctl->Rx_frame_ctl.Frame_cnt_taget - pnetwork_ctl->Rx_frame_ctl.Frame_cnt_current;
	if(wait_rx_frame){
		if(wait_rx_frame > bs){
			pnetwork_ctl->Rx_frame_ctl.BS_cnt_taget = bs;
		}else{
			pnetwork_ctl->Rx_frame_ctl.BS_cnt_taget = wait_rx_frame;
		}
		if(0 == bs){
			pnetwork_ctl->Rx_frame_ctl.BS_cnt_taget = wait_rx_frame;
		}
		pnetwork_ctl->Rx_frame_ctl.BS_cnt_current = 0;
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_tx_ctl_pre_next_BS(void)
{
	uds_int8_t ret = 0;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	UDS_N_FC_Para_t *para = &Tx_FC_para;
	uds_uint8_t wait_tx_frame;
	uds_uint16_t bs = UDS_N_BS_decode(para->BS);
	
	wait_tx_frame = pnetwork_ctl->Tx_frame_ctl.Frame_cnt_taget - pnetwork_ctl->Tx_frame_ctl.Frame_cnt_current;
	if(wait_tx_frame){
		if(wait_tx_frame > bs){
			pnetwork_ctl->Tx_frame_ctl.BS_cnt_taget = bs;
		}else{
			pnetwork_ctl->Tx_frame_ctl.BS_cnt_taget = wait_tx_frame;
		}
		if(0 == bs){
			pnetwork_ctl->Tx_frame_ctl.BS_cnt_taget = wait_tx_frame;
		}
		pnetwork_ctl->Tx_frame_ctl.BS_cnt_current = 0;
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_rx_ctl_set_target_BS(uds_uint8_t bs)
{
	uds_int8_t ret = 0;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(bs){
		pnetwork_ctl->Rx_frame_ctl.BS_cnt_taget = UDS_N_BS_decode(bs);
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_tx_ctl_set_target_BS(uds_uint8_t bs)
{
	uds_int8_t ret = 0;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(bs){
		pnetwork_ctl->Tx_frame_ctl.BS_cnt_taget = UDS_N_BS_decode(bs);
		ret = 1;
	}

	return ret;
}
static uds_uint16_t UDS_N_rx_ctl_get_target_BS(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Rx_frame_ctl.BS_cnt_taget;
}

static uds_uint16_t UDS_N_tx_ctl_get_target_BS(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Tx_frame_ctl.BS_cnt_taget;
}

static uds_int8_t UDS_N_rx_ctl_add_current_BS(void)
{
	uds_int8_t ret = -1;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(pnetwork_ctl->Rx_frame_ctl.BS_cnt_taget > pnetwork_ctl->Rx_frame_ctl.BS_cnt_current){
		pnetwork_ctl->Rx_frame_ctl.BS_cnt_current++;
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_tx_ctl_add_current_BS(void)
{
	uds_int8_t ret = -1;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(pnetwork_ctl->Tx_frame_ctl.BS_cnt_taget > pnetwork_ctl->Tx_frame_ctl.BS_cnt_current){
		pnetwork_ctl->Tx_frame_ctl.BS_cnt_current++;
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_rx_ctl_BS_is_finish(void)
{
	uds_int8_t ret = 0;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(pnetwork_ctl->Rx_frame_ctl.BS_cnt_taget == pnetwork_ctl->Rx_frame_ctl.BS_cnt_current){
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_tx_ctl_BS_is_finish(void)
{
	uds_int8_t ret = 0;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(pnetwork_ctl->Tx_frame_ctl.BS_cnt_taget == pnetwork_ctl->Tx_frame_ctl.BS_cnt_current){
		ret = 1;
	}

	return ret;
}

static uds_int8_t UDS_N_tx_ctl_set_target_frame(uds_uint16_t length)
{
	uds_int8_t ret = -1;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	uds_uint16_t frame_length,temp;

	if(length > SF_DATA_LENGTH){
		temp = length - FF_DATA_LENGTH;
		frame_length = 1 + temp / CF_DATA_LENGTH;
		if(temp%CF_DATA_LENGTH){
			frame_length++;
		}
		pnetwork_ctl->Tx_frame_ctl.Frame_cnt_taget = frame_length;
		ret = 1;
	}

	return ret;
}
//Get rx Buffer_pointer_current
static uds_uint16_t UDS_N_rx_ctl_buffer_pointer_current_get(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Rx_frame_ctl.Buffer_pointer_current;
}
//Get rx Buffer index
static uds_uint8_t UDS_N_rx_ctl_buffer_index_get(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Rx_frame_ctl.Index_buf;
}

//Get rx Buffer_pointer_target
static uds_uint16_t UDS_N_rx_ctl_buffer_pointer_target_get(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Rx_frame_ctl.Buffer_pointer_target;
}
//Get tx Buffer_pointer_target
static uds_uint16_t UDS_N_tx_ctl_buffer_pointer_target_get(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Tx_frame_ctl.Buffer_pointer_target;
}


static void UDS_N_rx_ctl_reset_for_new(uds_uint16_t length)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	uds_int8_t index = UDS_N_malloc_can_recive_buf();

	pnetwork_ctl->Rx_frame_ctl.Index_buf = index;
	if(-1 != index){
		pnetwork_ctl->Rx_frame_ctl.Buffer_pointer_target = length;
		pnetwork_ctl->Rx_frame_ctl.Buffer_pointer_current = 0;
		pnetwork_ctl->Rx_frame_ctl.Frame_cnt_current = 0;
		pnetwork_ctl->Rx_frame_ctl.SN_last = 0;
		UDS_N_rx_ctl_set_target_frame(length);
		UDS_N_rx_ctl_set_SN_last(0);
	}
}

static void UDS_N_tx_ctl_reset_for_new(UDS_N_USData_Request_t *mes)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	UDS_N_USData_Request_t *pmes = mes;

	if(!pmes){
		return;
	}

	pnetwork_ctl->Tx_service_info = pmes->Info;
	pnetwork_ctl->Tx_frame_ctl.Buffer_pointer_current = 0;
	pnetwork_ctl->Tx_frame_ctl.BS_cnt_current = 0;
	pnetwork_ctl->Tx_frame_ctl.Frame_cnt_current = 0;
	pnetwork_ctl->Tx_frame_ctl.SN_last = 0;
	pnetwork_ctl->Tx_frame_ctl.Buffer_pointer_target = mes->Length;
	UDS_N_tx_ctl_set_target_frame(mes->Length);
	UDS_N_tx_ctl_set_SN_last(0);
}

static uds_uint8_t UDS_N_rx_ctl_get_SN_last(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Rx_frame_ctl.SN_last;
}
static void UDS_N_rx_ctl_set_SN_last(uds_uint8_t sn)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	pnetwork_ctl->Rx_frame_ctl.SN_last = sn&0x0F;
}

static uds_uint8_t UDS_N_tx_ctl_get_SN_last(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return pnetwork_ctl->Tx_frame_ctl.SN_last;
}
static void UDS_N_tx_ctl_set_SN_last(uds_uint8_t sn)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	pnetwork_ctl->Tx_frame_ctl.SN_last = sn&0x0F;
}
//
static void *UDS_N_tx_ctl_sevice_info_get(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return (void *)&pnetwork_ctl->Tx_service_info;
}
//
static UDS_N_Service_Info_Pub_t *UDS_N_rx_ctl_sevice_info_get(void)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	return (void *)&pnetwork_ctl->Rx_service_info;
}

//Put issue to upper pool
static uds_int8_t UDS_N_push_to_can_send_list(void *data)
{
	uds_int8_t ret = -1;
	UDS_N_Services_t *pdata = data;
	Slist *plist = &Can_send_slist;

	if(!pdata){
		return ret;
	}

	//make node and insert
	ret = UDS_N_add_node_to_list(plist,(void *)pdata);
	if(1 != ret){
		ret = UDS_N_pool_pointer_free(N_POOL_TYPE_PDU,pdata);
	}
	
	return ret;
}

//send fc frame
static uds_int8_t UDS_N_rx_send_FC_frame(UDS_N_FC_Ctl_e fc)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_Pool_t *pdu = UDS_N_pool_pointer_get(N_POOL_TYPE_PDU);
	UDS_N_Frame_t *frame_FC = UDS_NULL;
	UDS_N_FC_Para_t *para = &Rx_FC_para;
	uds_uint8_t i;

	if((!pdu) || (N_FC_NONE < fc)){
		return ret;
	}

	pdu->Hook = UDS_N_rx_FC_send_result_hook;
	pdu->PDU.ID = UDS_DIAGNOSTICS_SELF_ID;
	pdu->PDU.Length = 8;
	frame_FC = &pdu->PDU.Frame;
	for(i = 0;i < FC_DATA_LENGTH;i++){
		frame_FC->FC.Reveser[i] = 0;
	}
	frame_FC->FC.PCI_Bit.PCItype = N_PDU_NAME_FC;
	switch(fc){
		case N_FC_CTS:
			frame_FC->FC.PCI_Bit.FS = N_FC_CTS;
			ret = 1;
			break;
		case N_FC_WT:
			frame_FC->FC.PCI_Bit.FS = N_FC_WT;
			ret = 1;
			break;
		case N_FC_OVLFW:
			frame_FC->FC.PCI_Bit.FS = N_FC_OVLFW;
			ret = 1;
			break;
		default:break;
	}
	//push to send buffer 
	if(1 == ret){
		frame_FC->FC.BS = para->BS;
		frame_FC->FC.STmin = para->STmin;
		ret = UDS_N_push_to_can_send_list((void *)pdu);
	}
	
	return ret;
}

//Push data to can rx buffer
static uds_int8_t UDS_N_rx_push_data_to_buffer(uds_uint8_t *data,
											   uds_uint8_t length)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	uds_uint8_t *rx_buffer = UDS_N_get_can_recive_buf(pnetwork_ctl->Rx_frame_ctl.Index_buf);
	uds_uint8_t i,*pdata = data;
	uds_int8_t ret = -1;
	uds_uint16_t pointer_target = pnetwork_ctl->Rx_frame_ctl.Buffer_pointer_target;

	if((!pdata) || (!length) || (!rx_buffer)){
		return ret;
	}

	for(i = 0;i < length;i++){
		if(pointer_target > pnetwork_ctl->Rx_frame_ctl.Buffer_pointer_current){
			rx_buffer[pnetwork_ctl->Rx_frame_ctl.Buffer_pointer_current++] = pdata[i];
			ret = 1;
		}else{
			break;
		}
	}
	if(1 == ret){
		UDS_N_rx_ctl_add_current_frame();
		UDS_N_rx_ctl_add_current_BS();
	}

	return ret;
}
//Pull data from can service buffer
static uds_int8_t UDS_N_tx_pull_data_from_buffer(uds_uint8_t *data,
												 uds_uint8_t length)
{
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	uds_uint8_t *rx_buffer = USD_rx_buffer,i,*pdata = data;
	uds_int8_t ret = -1;

	if((!pdata) || (!length)){
		return ret;
	}

	for(i = 0;i < length;i++){
		if(UDS_N_RX_BUFFER_MAX > pnetwork_ctl->Tx_frame_ctl.Buffer_pointer_current){
			pdata[i] = rx_buffer[pnetwork_ctl->Tx_frame_ctl.Buffer_pointer_current++];
		}else{
			pdata[i] = 0;
		}
	}
	UDS_N_tx_ctl_add_current_frame();
	UDS_N_tx_ctl_add_current_BS();
	ret = 1;

	return ret;
}
static uds_int8_t UDS_N_rx_SF_deal(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_t *data_pdu = (UDS_N_PDU_t *)pdata;
	uds_uint8_t length,*buf = UDS_NULL;;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(!data_pdu){
		return ret;
	}

	ret = 0;
	length = data_pdu->Frame.SF.PCI_Bit.DL;
	if((0 < length)
		&&(SF_DATA_LENGTH >= length)){//check length
		UDS_N_rx_ctl_clear();
		UDS_N_rx_info_clear();
		//init recive ctl
		UDS_N_rx_ctl_reset_for_new(length);
		//push SF data to buffer
		ret = UDS_N_rx_push_data_to_buffer(data_pdu->Frame.SF.N_Data,length);

		pnetwork_ctl->Rx_service_info.Mtype = MTYPE_DIAGNOSTICS;
		pnetwork_ctl->Rx_service_info.N_SA = data_pdu->ID;
		if(UDS_DIAGNOSTICS_FUNCTIONAL_ID == data_pdu->ID){
			pnetwork_ctl->Rx_service_info.N_TAtype = N_TATYPE_FUNCTIONAL;
			pnetwork_ctl->Rx_service_info.N_TA = (uds_uint8_t)UDS_DIAGNOSTICS_FUNCTIONAL_ID;
		}else{
			pnetwork_ctl->Rx_service_info.N_TAtype = N_TATYPE_PHYSICAL;
			pnetwork_ctl->Rx_service_info.N_TA = (uds_uint8_t)UDS_DIAGNOSTICS_PHYSICAL_ID;
		}
		#ifdef ADDRESS_EXTENSION_MODE	
		pnetwork_ctl->Rx_service_info.N_AE = 0;
		#endif
		buf = UDS_N_get_can_recive_buf(pnetwork_ctl->Rx_frame_ctl.Index_buf);
		//issue upper layer
		if(1 == ret){
			ret = UDS_N_generate_USData_indication(&Network_service,
												   N_OK,
												   buf);
			if(1 == ret){
				ret = UDS_N_issue_to_upper_put(N_USDATA_INDICATION,&Network_service);
			}
		}
	}
	UDS_N_rx_ctl_set_status(N_STS_RX_IDLE);
	UDS_N_rx_ctl_clear();
	UDS_N_rx_info_clear();
	
	return ret;
}
static uds_int8_t UDS_N_rx_unexpected_SF_deal(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_t *data_pdu = (UDS_N_PDU_t *)pdata;
	uds_uint8_t length,*buf = UDS_NULL;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(!data_pdu){
		return ret;
	}

	ret = 0;
	length = data_pdu->Frame.SF.PCI_Bit.DL;
	if((0 < length)
		&&(SF_DATA_LENGTH >= length)){//check length
		//stop timer Cr
		UDS_N_time_ctl_stop(N_TIME_NAME_N_Cr);
		//set status
		UDS_N_rx_ctl_set_status(N_STS_RX_IDLE);
		//get buf
		buf = UDS_N_get_can_recive_buf(pnetwork_ctl->Rx_frame_ctl.Index_buf);
		//issue upper layer
		ret = UDS_N_generate_USData_indication(&Network_service,
											   N_UNEXP_PDU,
											   buf);
		if(1 == ret){
			ret = UDS_N_issue_to_upper_put(N_USDATA_INDICATION,&Network_service);
		}
		UDS_N_rx_ctl_clear();
		UDS_N_rx_info_clear();
	}

	return ret;
}

static uds_int8_t UDS_N_rx_FF_deal(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_t *data_pdu = (UDS_N_PDU_t *)pdata;
	uds_uint16_t length;
	UDS_N_FC_Ctl_e send_FC_flag = N_FC_NONE;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(!data_pdu){
		return ret;
	}

	ret = 0;
	//Restart Br time count
	ret |= UDS_N_time_ctl_restart(N_TIME_NAME_N_Br);
	length = data_pdu->Frame.FF.PCI_Bit.DL_MSB * 256 + data_pdu->Frame.FF.DL_LSB;
	if(SF_DATA_LENGTH < length){//check length
		if(UDS_N_CAN_RX_BUFFER_MAX >= length){//checke recived ablity.
			UDS_N_rx_ctl_clear();
			UDS_N_rx_info_clear();
			//init recive ctl
			UDS_N_rx_ctl_reset_for_new(length);
			pnetwork_ctl->Rx_service_info.Mtype = MTYPE_DIAGNOSTICS;
			pnetwork_ctl->Rx_service_info.N_SA = data_pdu->ID;
			if(UDS_DIAGNOSTICS_FUNCTIONAL_ID == data_pdu->ID){
				pnetwork_ctl->Rx_service_info.N_TAtype = N_TATYPE_FUNCTIONAL;
				pnetwork_ctl->Rx_service_info.N_TA = (uds_uint8_t)UDS_DIAGNOSTICS_FUNCTIONAL_ID;
			}else{
				pnetwork_ctl->Rx_service_info.N_TAtype = N_TATYPE_PHYSICAL;
				pnetwork_ctl->Rx_service_info.N_TA = (uds_uint8_t)UDS_DIAGNOSTICS_PHYSICAL_ID;
			}
			#ifdef ADDRESS_EXTENSION_MODE	
			pnetwork_ctl->Rx_service_info.N_AE = 0;
			#endif
			UDS_N_rx_ctl_set_status(N_STS_RX_MULTI_FRAME);
			//push FF data to buffer
			ret |= UDS_N_rx_push_data_to_buffer(data_pdu->Frame.FF.N_Data,FF_DATA_LENGTH);
			//issue upper layer
			ret |= UDS_N_generate_USData_FF_indication(data_pdu);
			ret |= UDS_N_issue_to_upper_put(N_USDATA_FF_INDICATION,&Network_service);
			//set bs
			UDS_N_rx_ctl_pre_next_BS();
			send_FC_flag = N_FC_CTS;
			//set rx status
			ret |= UDS_N_rx_ctl_set_status(N_STS_RX_MULTI_FRAME);
		}else{//senf FC Overflow
			send_FC_flag = N_FC_OVLFW;
		}	
		//send flow control frame
		if(N_FC_NONE > send_FC_flag){
			ret = UDS_N_rx_send_FC_frame(send_FC_flag);
		}
	}
	
	return ret;
}
static uds_int8_t UDS_N_rx_unexpected_FF_deal(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_t *data_pdu = (UDS_N_PDU_t *)pdata;
	uds_uint8_t length,*buf = UDS_NULL;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(!data_pdu){
		return ret;
	}

	ret = 0;
	length = data_pdu->Frame.FF.PCI_Bit.DL_MSB * 256 + data_pdu->Frame.FF.DL_LSB;
	if(SF_DATA_LENGTH < length)//check length
		if(UDS_N_RX_BUFFER_MAX >= length){//checke recived ablity.
		//stop timer Cr
		UDS_N_time_ctl_stop(N_TIME_NAME_N_Cr);
		//set status
		UDS_N_rx_ctl_set_status(N_STS_RX_IDLE);
		//get buf
		buf = UDS_N_get_can_recive_buf(pnetwork_ctl->Rx_frame_ctl.Index_buf);
		//issue upper layer
		ret = UDS_N_generate_USData_indication(&Network_service,
											   N_UNEXP_PDU,
											   buf);
		if(1 == ret){
			ret = UDS_N_issue_to_upper_put(N_USDATA_INDICATION,&Network_service);
		}
		UDS_N_rx_ctl_clear();
		UDS_N_rx_info_clear();
	}

	return ret;
}

static uds_int8_t UDS_N_rx_CF_deal(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_t *data_pdu = (UDS_N_PDU_t *)pdata;
	uds_uint8_t ctl_sn,r_sn,*buf = UDS_NULL;
	UDS_N_FC_Ctl_e send_FC_flag = N_FC_NONE;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(!data_pdu){
		return ret;
	}

	ret = 0;
	ctl_sn = UDS_N_rx_ctl_get_SN_last();
	r_sn = data_pdu->Frame.CF.PCI_Bit.SN;
	ctl_sn = 0x0F & (ctl_sn +1);
	if(r_sn == ctl_sn){//serial number check.
		UDS_N_rx_ctl_set_SN_last(ctl_sn);
		//Restart Cr time count
		UDS_N_time_ctl_restart(N_TIME_NAME_N_Cr);
		//push CF data to buffer
		ret = UDS_N_rx_push_data_to_buffer(data_pdu->Frame.CF.N_Data,CF_DATA_LENGTH);
		//check BS finish
		if(UDS_N_rx_ctl_BS_is_finish()){//bs full
			//set FC flag
			send_FC_flag = N_FC_CTS;
			//prepare next BS recive
			ret = UDS_N_rx_ctl_pre_next_BS();
		}
		//check recive package finish
		if(UDS_N_rx_ctl_package_is_finish()){
			//set FC flag
			send_FC_flag = N_FC_NONE;
			//generate USData.indication
			//get buf
			buf = UDS_N_get_can_recive_buf(pnetwork_ctl->Rx_frame_ctl.Index_buf);
			ret = UDS_N_generate_USData_indication(&Network_service,
												   N_OK,
												   buf);
			if(1 == ret){
				ret = UDS_N_issue_to_upper_put(N_USDATA_INDICATION,&Network_service);
				if(1 == ret){
					//set Rx contrl status
					ret = UDS_N_rx_ctl_set_status(N_STS_RX_IDLE);
				}
			}
			UDS_N_time_ctl_stop(N_TIME_NAME_N_Cr);
		}
		//send flow control frame
		if(N_FC_NONE > send_FC_flag){
			ret = UDS_N_rx_send_FC_frame(send_FC_flag);
		}
	}else{//error handle
		//get buf
		buf = UDS_N_get_can_recive_buf(pnetwork_ctl->Rx_frame_ctl.Index_buf);
		//generate USData.indication
		ret = UDS_N_generate_USData_indication(&Network_service,
											   N_WRONG_SN,
											   buf);
		if(1 == ret){
			ret = UDS_N_issue_to_upper_put(N_USDATA_INDICATION,&Network_service);
			if(1 == ret){
				//set rx status
				ret = UDS_N_rx_ctl_set_status(N_STS_RX_IDLE);
			}
		}
	}

	return ret;
}
static uds_uint16_t UDS_N_BS_decode(uds_uint8_t bs)
{
	uds_uint16_t ret;

	if(bs){
		ret = bs;
	}else{
		ret = 0xFFFF;
	}

	return ret;
}

static uds_uint8_t UDS_N_BS_encode(uds_uint16_t bs)
{
	uds_uint8_t ret;

	if(0xFFFF == bs){
		ret = 0;
	}else{
		ret = bs;
	}

	return ret;
}

static uds_uint8_t UDS_N_ST_decode(uds_uint8_t st)
{
	uds_uint8_t ret = 127;

	if(127 >= st){
		ret = st;
	}else if((0xF1 <= st) && (0xF9 >= st)){
		ret = 1;
	}

	return ret;
}

static uds_uint8_t UDS_N_ST_encode(uds_uint8_t st)
{
	uds_uint8_t ret = 127;

	if(127 >= st){
		ret = st;
	}

	return ret;
}

static void UDS_N_set_send_para(uds_uint8_t bs,uds_uint8_t st)
{
	UDS_N_FC_Para_t *para = &Tx_FC_para;
	UDS_N_Count_Status_t *pCs = UDS_N_time_pointer_get(N_TIME_NAME_N_Cs);

	para->BS 	= bs;
	para->STmin = st;
	//update to time
	if(pCs){
		pCs->Target = UDS_N_ST_decode(para->STmin);
	}
}

static void UDS_N_set_recive_para(uds_uint8_t bs,uds_uint8_t st)
{
	UDS_N_FC_Para_t *para = &Rx_FC_para;

	para->BS 	= bs;
	para->STmin = st;
}

static uds_int8_t UDS_N_tx_FC_CTS_deal(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_Frame_t *data_pdu = (UDS_N_Frame_t *)pdata;

	if(!data_pdu){
		return ret;
	}

	ret = 0;
	//stop timer Bs
	ret |= UDS_N_time_ctl_stop(N_TIME_NAME_N_Bs);
	//stop timer Cs
	ret |= UDS_N_time_ctl_stop(N_TIME_NAME_N_Cs);
	//set fc parameter
	UDS_N_set_send_para(data_pdu->FC.BS,data_pdu->FC.STmin);
	//set bs para
	ret |= UDS_N_tx_ctl_pre_next_BS();
	//restart timer Cs
	ret |= UDS_N_time_ctl_restart(N_TIME_NAME_N_Cs);
	//set send status
	ret |= UDS_N_tx_ctl_set_status(N_STS_TX_MULTI_FRAME_IN_TRANSMIT);	
	
	return ret;
}

static uds_int8_t UDS_N_tx_FC_WT_deal(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_Frame_t *data_pdu = (UDS_N_Frame_t *)pdata;

	if(!data_pdu){
		return ret;
	}

	ret = 0;
	//restar Bs timer
	ret |= UDS_N_time_ctl_restart(N_TIME_NAME_N_Bs);
	
	return ret;
}

static uds_int8_t UDS_N_tx_FC_OVLFW_deal(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_Frame_t *data_pdu = (UDS_N_Frame_t *)pdata;

	if(!data_pdu){
		return ret;
	}

	ret = 0;
	//Terminal send action
	ret |= UDS_N_time_ctl_stop(N_TIME_NAME_N_Bs);
	//set send status
	UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
	//generate failed USData.confirm
	ret = UDS_N_generate_USData_confirm(&Network_service,N_BUFFER_OVFLW);
	//issue to upper layer
	if(1 == ret){
		UDS_N_issue_to_upper_put(N_USDATA_CONFIRM,&Network_service);
	}

	//
	
	return ret;
}

static uds_int8_t UDS_N_tx_FC_deal(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_t *data_pdu = (UDS_N_PDU_t *)pdata;
	uds_uint8_t fc_type,fs_error = 0;
	if(!data_pdu){
		return ret;
	}

	ret = 0;
	fc_type = data_pdu->Frame.FC.PCI_Bit.FS;
	switch(fc_type){
		case N_FC_CTS:
			ret = UDS_N_tx_FC_CTS_deal(&data_pdu->Frame);
			fs_error = 1;
			break;
		case N_FC_WT:
			ret = UDS_N_tx_FC_WT_deal(&data_pdu->Frame);
			fs_error = 1;
			break;
		case N_FC_OVLFW:
			ret = UDS_N_tx_FC_OVLFW_deal(&data_pdu->Frame);
			fs_error = 1;
			break;
		case N_FC_NONE:
			break;
		default:break;
	}
	if(0 == fs_error){//invalid flow control
		//Terminal send action
		ret |= UDS_N_time_ctl_stop(N_TIME_NAME_N_Bs);
		//set send status
		UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
		//generate failed USData.confirm
		ret = UDS_N_generate_USData_confirm(&Network_service,N_INVALID_FS);
		//issue to upper layer
		if(1 == ret){
			UDS_N_issue_to_upper_put(N_USDATA_CONFIRM,&Network_service);
		}
	}

	return ret;
}

static uds_int8_t UDS_N_rx_message_process_normal(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_t *data_pdu = (UDS_N_PDU_t *)pdata;
	uds_uint8_t pci_type;
	
	if(!data_pdu){
		return ret;
	}

	ret = 0;
	pci_type = data_pdu->Frame.PCI.PCI_Bit.PCItype;
	switch(pci_type){
		case N_PDU_NAME_SF:
			ret = UDS_N_rx_SF_deal(pdata);
			break;
		case N_PDU_NAME_FF:
			ret = UDS_N_rx_FF_deal(pdata);
			break;
		default:break;
	}

	return ret;
}
static uds_int8_t UDS_N_rx_message_process_muilt_frame(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_t *data_pdu = (UDS_N_PDU_t *)pdata;
	uds_uint16_t length;
	
	if(!data_pdu){
		return ret;
	}

	ret = 0;
	switch(data_pdu->Frame.PCI.PCI_Bit.PCItype){
		case N_PDU_NAME_SF:
			ret |= UDS_N_rx_unexpected_SF_deal(data_pdu);
			ret |= UDS_N_rx_SF_deal(data_pdu);
			break;
		case N_PDU_NAME_FF:
			ret |= UDS_N_rx_unexpected_FF_deal(data_pdu);
			ret |= UDS_N_rx_FF_deal(data_pdu);
			break;
		case N_PDU_NAME_CF:
			ret = UDS_N_rx_CF_deal(data_pdu);
			break;
		case N_PDU_NAME_FC:
			
			break;			
		default:break;
	}

	return ret;
}

//for canoe uds test
static uds_int8_t UDS_N_rx_CF_length_check(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_Pool_t *data_pdu = (UDS_N_PDU_Pool_t *)pdata;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;
	uds_uint16_t untransmit_byte;

	untransmit_byte = pnetwork_ctl->Rx_frame_ctl.Buffer_pointer_target - pnetwork_ctl->Rx_frame_ctl.Buffer_pointer_current;

	if(CF_DATA_LENGTH <= untransmit_byte){//not the end frame
		if(8 == data_pdu->PDU.Length){
			ret = 1;
		}
	}else{
		untransmit_byte = untransmit_byte + 8 - CF_DATA_LENGTH;
		if(untransmit_byte < data_pdu->PDU.Length){
			ret = 1;
		}
	}

	return ret;
}

static uds_int8_t UDS_N_rx_frame_check(void *pdata)
{
	UDS_N_PDU_Pool_t *pmessage = (UDS_N_PDU_Pool_t *)pdata;
	uds_int8_t ret = -1;

	switch(pmessage->PDU.Frame.PCI.PCI_Bit.PCItype){
		case N_PDU_NAME_SF:
			if(pmessage->PDU.Length > pmessage->PDU.Frame.SF.PCI_Bit.DL){
				ret = 1;
			}
			break;
		case N_PDU_NAME_FF:
			if(8 == pmessage->PDU.Length){
				ret = 1;
			}
			break;
		case N_PDU_NAME_CF:
			if(1 == UDS_N_rx_CF_length_check(pmessage)){
				ret = 1;
			}
			break;
		case N_PDU_NAME_FC:
			if(pmessage->PDU.Length > (8 - FC_DATA_LENGTH)){
				ret = 1;
			}
			break;			
		default:break;
	}

	return ret;
}

static uds_int8_t UDS_N_rx_message_process(void *pdata)
{
	UDS_N_PDU_Pool_t *pmessage = (UDS_N_PDU_Pool_t *)pdata;
	uds_int8_t ret = -1;
	UDS_N_Rx_Status_e sts = UDS_N_rx_ctl_get_status();

	if(!pmessage){
		return ret;
	}

	if(1 == UDS_N_rx_frame_check(pdata)){
		switch(sts){
			case N_STS_RX_IDLE:
				ret = UDS_N_rx_message_process_normal((void *)&pmessage->PDU);
				break;
			case N_STS_RX_MULTI_FRAME:
				ret = UDS_N_rx_message_process_muilt_frame((void *)&pmessage->PDU);
				break;
			case N_STS_RX_ERR:
		
				break;
			default:break;
		}
	}
	//free pointer
	UDS_N_pool_pointer_free(N_POOL_TYPE_PDU,pmessage);
	ret = 1;

	return ret;
}
static uds_int8_t UDS_N_tx_message_process_muilt_frame(void *pdata)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_t *data_pdu = (UDS_N_PDU_t *)pdata;
	uds_uint16_t length;
	
	if(!data_pdu){
		return ret;
	}

	ret = 0;
	switch(data_pdu->Frame.PCI.PCI_Bit.PCItype){
		case N_PDU_NAME_SF:
			break;
		case N_PDU_NAME_FF:
			break;
		case N_PDU_NAME_CF:
			break;
		case N_PDU_NAME_FC:
			ret = UDS_N_tx_FC_deal(data_pdu);
			break;
		default:break;
	}

	return ret;
}

static uds_int8_t UDS_N_tx_frame_check(void *pdata)
{
	UDS_N_PDU_Pool_t *pmessage = (UDS_N_PDU_Pool_t *)pdata;
	uds_int8_t ret = -1;

	switch(pmessage->PDU.Frame.PCI.PCI_Bit.PCItype){
		case N_PDU_NAME_FC:
			if(pmessage->PDU.Length > (8 - FC_DATA_LENGTH)){
				ret = 1;
			}
			break;			
		default:break;
	}

	return ret;
}

static uds_int8_t UDS_N_tx_message_process(void *pdata)
{
	UDS_N_PDU_Pool_t *pmessage = (UDS_N_PDU_Pool_t *)pdata;
	uds_int8_t ret = -1;
	UDS_N_Rx_Status_e sts = UDS_N_tx_ctl_get_status();

	if(!pmessage){
		return ret;
	}

	if(0 < UDS_N_tx_frame_check(pdata)){
		switch(sts){
			case N_STS_TX_IDLE:
				
				break;
			case N_STS_TX_WAIT_FC:
				ret = UDS_N_tx_message_process_muilt_frame((void *)&pmessage->PDU);
				break;
			case N_STS_TX_MULTI_FRAME_IN_TRANSMIT:
		
				break;
			case N_STS_TX_ERR:
		
				break;
			default:break;
		}
	}
	//free pointer
	UDS_N_pool_pointer_free(N_POOL_TYPE_PDU,pmessage);
	ret = 1;

	return ret;
}

//Deal rx recive data 
static void UDS_N_rx_can_recive_data_deal(void)
{
	void *pdata = UDS_NULL;

	//There are some messages to process
	while(0 < UDS_N_list_data_get(N_LIST_TYPE_RX_CAN_RECIVE,&pdata)){
		UDS_N_rx_message_process(pdata);
	}
}

//Deal tx recive data 
static void UDS_N_tx_can_recive_data_deal(void)
{
	void *pdata = UDS_NULL;

	//There are some messages to process
	while(0 < UDS_N_list_data_get(N_LIST_TYPE_TX_CAN_RECIVE,&pdata)){
		UDS_N_tx_message_process(pdata);
	}
}

//CAN send process
static uds_int8_t UDS_N_can_send_process(void *pdata)
{
	UDS_N_PDU_Pool_t *pmessage = (UDS_N_PDU_Pool_t *)pdata;
	uds_int8_t ret = -1;

	if((!pmessage) || (!UDS_N_can_send_hook)){
		return ret;
	}

	ret = (uds_int8_t)(UDS_N_can_send_hook(pmessage->PDU.ID,
										   pmessage->PDU.Length,
										   pmessage->PDU.Frame.Data));
	//send hook
	if(pmessage->Hook){
		pmessage->Hook(ret);
	}
	//free pointer
	UDS_N_pool_pointer_free(N_POOL_TYPE_PDU,pmessage);
	ret = 1;

	return ret;
}

//Can send data 
static void UDS_N_can_send_data_deal(void)
{
	void *pdata = UDS_NULL;
	
	//There are some messages to process
	while(0 < UDS_N_list_data_get(N_LIST_TYPE_CAN_SEND,&pdata)){
		UDS_N_can_send_process(pdata);
	}
}
/**********************************************************************
* @brief	   Push the USData.resuest service message to network.
*
* @param[in]   pdata:UDS_N_USData_Request_t pointer for transmit.
*
* @return	   1    :success 
*			   other:busy
**********************************************************************/
uds_int8_t UDS_N_service_process_USData_request(void *pdata)
{
	UDS_N_USData_Request_t *pmessage = (UDS_N_USData_Request_t *)pdata;
	uds_int8_t ret = -1;

	if((!pmessage) || (UDS_N_RX_BUFFER_MAX < pmessage->Length) || UDS_N_ctl_service_is_busy()){
		return ret;
	}

	//issue to network
	ret = UDS_N_issue_to_network_put(N_USDATA_REQUEST,(UDS_N_Services_t *)pmessage);
	if(1 == ret){
		//prepare new package send
		UDS_N_tx_ctl_reset_for_new(pmessage);
		if(SF_DATA_LENGTH >= pmessage->Length){
			UDS_N_tx_ctl_set_status(N_STS_TX_SEND_SF);
		}else{
			UDS_N_tx_ctl_set_status(N_STS_TX_SEND_FF);
		}
	}
	
	return ret;
}

/**********************************************************************
* @brief	   Push the UDS_N_service_process_ChangeParameters_request
			   service message to network.
*
* @param[in]   pdata:UDS_N_service_process_ChangeParameters_request
			   pointer for transmit.
*
* @return	   1    :success 
*			   other:busy
**********************************************************************/
uds_int8_t UDS_N_service_process_ChangeParameters_request(void *pdata)
{
	UDS_N_Change_Parameters_Request_t *pmessage = (UDS_N_Change_Parameters_Request_t *)pdata;
	uds_int8_t ret = -1;
	UDS_N_Change_Parameter_e res;
	UDS_N_FC_Para_t *para = &Rx_FC_para;

	if(!pmessage){
		return ret;
	}

	if(N_STS_RX_MULTI_FRAME == UDS_N_rx_ctl_get_status()){
		res = N_PARA_RX_ON;
	}else{
		switch(pmessage->Parameter){
			case N_PARA_CH_STmin:
				if((0x80 > pmessage->Parameter_Value)
					|| ((0xF0 < pmessage->Parameter_Value) && (0xFA > pmessage->Parameter_Value))){
					para->STmin = pmessage->Parameter_Value;
					res = N_PARA_CHANGE_OK;
				}else{
					res = N_PARA_WRONG_VALUE;
				}
				break;
			case N_PARA_CH_BS:
				para->BS = pmessage->Parameter_Value;
				res = N_PARA_CHANGE_OK;
				break;
			default:
				res = N_PARA_WRONG_PARAMETER;
				break;
		}
	}

	ret = UDS_N_generate_ChangeParameter_confirm(&Network_service,pmessage,res);
	if(1 == ret){
		ret = UDS_N_issue_to_upper_put(N_CHANGE_PARAMETER_CONFIRM,&Network_service);
	}
	
	return 1;
}


//Service process
static uds_int8_t UDS_N_service_process(void *pdata)
{
	UDS_N_Services_Issue_t *pmessage = (UDS_N_Services_Issue_t *)pdata;
	uds_int8_t ret = -1;

	if(!pmessage){
		return ret;
	}

	ret = 0;
	switch(pmessage->Type){
		case N_USDATA_REQUEST:
			ret = UDS_N_service_process_USData_request((void *)&pmessage->Service.USData_request);
			break;
		case N_CHANGE_PARAMETER_REQUEST:

			break;
		default:break;
	}
	//free pointer
	UDS_N_pool_pointer_free(N_POOL_TYPE_ISSUE,pmessage);

	return ret;
}

//service send data 
static void UDS_N_service_deal(void)
{
	void *pdata = UDS_NULL;

	//There are some messages to process
	while(0 < UDS_N_list_data_get(N_LIST_TYPE_ISSUE_NETWORK,&pdata)){
		UDS_N_service_process(pdata);
	}
}
static void UDS_N_rx_FC_send_result_hook(uds_int8_t res)
{
	//change rx ctl status
	//UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
	if(1 == res){//send ok
		//stop timer Ar
		UDS_N_time_ctl_stop(N_TIME_NAME_N_Ar);
		//restart timer Cr
		if(N_STS_RX_MULTI_FRAME == UDS_N_rx_ctl_get_status()){
			UDS_N_time_ctl_restart(N_TIME_NAME_N_Cr);
		}		
	}
}

static void UDS_N_tx_SF_send_result_hook(uds_int8_t res)
{
	uds_int8_t ret = -1;
	
	if(1 == res){//send ok
		//stop timer As
		UDS_N_time_ctl_stop(N_TIME_NAME_N_As);
		//change tx ctl status
		UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
		//generate sccuess USData.confirm
		ret = UDS_N_generate_USData_confirm(&Network_service,N_OK);
		//issue to upper layer
		if(1 == ret){
			UDS_N_issue_to_upper_put(N_USDATA_CONFIRM,&Network_service);
		}
	}else{
		//generate failed USData.confirm
		//ret = UDS_N_generate_USData_confirm(&USData_Confirm,N_ERROR);
	}
}
static void UDS_N_tx_FF_send_result_hook(uds_int8_t res)
{
	if(1 == res){//send ok
		//stop timer As
		UDS_N_time_ctl_stop(N_TIME_NAME_N_As);
		//restart timer Bs
		UDS_N_time_ctl_restart(N_TIME_NAME_N_Bs);
		//change tx ctl status
		UDS_N_tx_ctl_set_status(N_STS_TX_WAIT_FC);
	}else{
		/*
		//stop timer As
		UDS_N_time_ctl_stop(N_TIME_NAME_N_As);
		//change tx ctl status
		UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
		//generate USData.confirm
		ret = UDS_N_generate_USData_confirm(&USData_Confirm,N_ERROR);
		//issue to upper layer
		if(1 == ret){
			UDS_N_issue_to_upper_put(N_USDATA_CONFIRM,&USData_Confirm);
		}
		*/
	}
}
static void UDS_N_tx_CF_send_result_hook(uds_int8_t res)
{
	uds_int8_t ret = -1;
	
	if(1 == res){//send ok
		//stop timer As
		UDS_N_time_ctl_stop(N_TIME_NAME_N_As);
		
		if(UDS_N_tx_ctl_package_is_finish()){//package is finish
			//set status
			UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
			//generate USData.confirm
			ret = UDS_N_generate_USData_confirm(&Network_service,N_OK);
			//issue to upper layer
			if(1 == ret){
				UDS_N_issue_to_upper_put(N_USDATA_CONFIRM,&Network_service);
			}
		}else{
			if(UDS_N_tx_ctl_BS_is_finish()){//segment package is finish
				//restart timer 
				UDS_N_time_ctl_restart(N_TIME_NAME_N_Bs);
				//change tx ctl status
				UDS_N_tx_ctl_set_status(N_STS_TX_WAIT_FC);
			}else{
				//restart timer Cs
				UDS_N_time_ctl_restart(N_TIME_NAME_N_Cs);
			}
		}
	}else{
	/*
		//stop timer As
		UDS_N_time_ctl_stop(N_TIME_NAME_N_As);
		//change tx ctl status
		UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
		//generate USData.confirm
		ret = UDS_N_generate_USData_confirm(&USData_Confirm,N_ERROR);
		//issue to upper layer
		if(1 == ret){
			UDS_N_issue_to_upper_put(N_USDATA_CONFIRM,&USData_Confirm);
		}
		*/
	}
}

//generate SF frame
static uds_int8_t UDS_N_tx_generate_SF_frame(void **res)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_Pool_t *pdu = (UDS_N_PDU_Pool_t *)UDS_N_pool_pointer_get(N_POOL_TYPE_PDU);
	uds_uint16_t temp;
	UDS_N_Service_Info_Pub_t *pservice_info = (UDS_N_Service_Info_Pub_t *)UDS_N_tx_ctl_sevice_info_get();
	
	if(!pdu){
		return ret;
	}

	temp = UDS_N_tx_ctl_buffer_pointer_target_get();
	pdu->PDU.ID = UDS_DIAGNOSTICS_SELF_ID;//pservice_info->N_SA;
	pdu->PDU.Length = 8;
	pdu->PDU.Frame.SF.PCI_Bit.PCItype = N_PDU_NAME_SF;
	pdu->PDU.Frame.SF.PCI_Bit.DL = temp;
	pdu->Hook = UDS_N_tx_SF_send_result_hook;
	ret = UDS_N_tx_pull_data_from_buffer(pdu->PDU.Frame.SF.N_Data,temp);
	if(1 == ret){
		*res = pdu;
	}else{
		UDS_N_pool_pointer_free(N_POOL_TYPE_PDU,pdu);
	}
	
	return ret;
}

//generate FF frame
static uds_int8_t UDS_N_tx_generate_FF_frame(void **res)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_Pool_t *pdu = UDS_N_pool_pointer_get(N_POOL_TYPE_PDU);
	uds_uint16_t temp;
	UDS_N_Service_Info_Pub_t *pservice_info = (UDS_N_Service_Info_Pub_t *)UDS_N_tx_ctl_sevice_info_get();

	if(!pdu){
		return ret;
	}

	temp = UDS_N_tx_ctl_buffer_pointer_target_get();
	pdu->PDU.ID = UDS_DIAGNOSTICS_SELF_ID;//pservice_info->N_SA;
	pdu->PDU.Length = 8;
	pdu->PDU.Frame.FF.PCI_Bit.PCItype = N_PDU_NAME_FF;
	pdu->PDU.Frame.FF.DL_LSB = temp & 0x00FF;
	pdu->PDU.Frame.FF.PCI_Bit.DL_MSB = (temp / 256) & 0x000F;
	pdu->Hook = UDS_N_tx_FF_send_result_hook;
	ret = UDS_N_tx_pull_data_from_buffer(pdu->PDU.Frame.FF.N_Data,FF_DATA_LENGTH);
	if(1 == ret){
		*res = pdu;
	}else{
		UDS_N_pool_pointer_free(N_POOL_TYPE_PDU,pdu);
	}
	
	return ret;
}
//generate CF frame
static uds_int8_t UDS_N_tx_generate_CF_frame(void **res)
{
	uds_int8_t ret = -1;
	UDS_N_PDU_Pool_t *pdu = UDS_N_pool_pointer_get(N_POOL_TYPE_PDU);
	uds_uint8_t sn_last = UDS_N_tx_ctl_get_SN_last();
	UDS_N_Service_Info_Pub_t *pservice_info = (UDS_N_Service_Info_Pub_t *)UDS_N_tx_ctl_sevice_info_get();

	if(!pdu){
		return ret;
	}

	pdu->PDU.ID = UDS_DIAGNOSTICS_SELF_ID;//pservice_info->N_SA;
	pdu->PDU.Length = 8;
	pdu->PDU.Frame.CF.PCI_Bit.PCItype = N_PDU_NAME_CF;
	pdu->PDU.Frame.CF.PCI_Bit.SN = sn_last + 1;
	UDS_N_tx_ctl_set_SN_last(sn_last + 1);
	pdu->Hook = UDS_N_tx_CF_send_result_hook;
	ret = UDS_N_tx_pull_data_from_buffer(pdu->PDU.Frame.CF.N_Data,CF_DATA_LENGTH);
	if(1 == ret){
		*res = pdu;
	}else{
		UDS_N_pool_pointer_free(N_POOL_TYPE_PDU,pdu);
	}
	
	return ret;
}

//send manage 
static void UDS_N_send_manage_deal(void)
{
	UDS_N_Rx_Status_e sts = UDS_N_tx_ctl_get_status();
	void *pdu = UDS_NULL;
	uds_int8_t ret;
	
	switch(sts){
		case N_STS_TX_IDLE:
			break;
		case N_STS_TX_WAIT_SEND_OK:
			break;
		case N_STS_TX_SEND_SF:
			//generate SF frame
			ret = UDS_N_tx_generate_SF_frame(&pdu);
			if(1 > ret){
				UDS_PRINTF("UDS Error:generate SF frame fialed!\n\r");
			}
			//push SF to send
			if(1 == ret){
				ret = UDS_N_push_to_can_send_list(pdu);
			}
			if(1 == ret){
				//set status
				UDS_N_tx_ctl_set_status(N_STS_TX_WAIT_SEND_OK);
				//restart As
				UDS_N_time_ctl_restart(N_TIME_NAME_N_As);
			}else{
				UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
			}
			break;
		case N_STS_TX_SEND_FF:
			//generate FF frame
			ret = UDS_N_tx_generate_FF_frame(&pdu);
			if(1 > ret){
				UDS_PRINTF("UDS Error:generate FF frame fialed!\n\r");
			}
			//push FF to send
			if(1 == ret){
				ret = UDS_N_push_to_can_send_list(pdu);
			}
			if(1 == ret){
				//set status
				UDS_N_tx_ctl_set_status(N_STS_TX_WAIT_FC);
				UDS_N_tx_ctl_set_SN_last(0);
				//start timer As
				UDS_N_time_ctl_restart(N_TIME_NAME_N_As);
			}else{
				UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
			}
			if(1 != ret){//prepare failed
				if(UDS_NULL != pdu){
					UDS_N_pool_pointer_free(N_POOL_TYPE_PDU,pdu);
				}
			}
			break;	
		case N_STS_TX_WAIT_FC:
			break;
		case N_STS_TX_MULTI_FRAME_IN_TRANSMIT:
			break;
		case N_STS_TX_TRANSMIT_FINISH:
			//set status
			UDS_N_tx_ctl_set_status(N_STS_TX_IDLE);
			UDS_N_ctl_service_clear();
			//generate USData.confirm service

			//issue service
			
			break;
		case N_STS_TX_ERR:
			break;
		default:break;
	}
}

static void UDS_N_Ar_time_out_reply(void)
{
	UDS_N_Count_Status_t *ptime = UDS_N_time_pointer_get(N_TIME_NAME_N_Ar);
	uds_int8_t ret = -1;
	uds_uint8_t *buf = UDS_NULL;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(!ptime){
		return;
	}

	if(ptime->Cnt_en){
		if(N_TIME_CNT_STS_TIMEOUT == ptime->Status){
			//stop timer Ar
			UDS_N_time_ctl_stop(N_TIME_NAME_N_Ar);
			//set status
			UDS_N_rx_ctl_set_status(N_STS_RX_IDLE);
			//get buf
			buf = UDS_N_get_can_recive_buf(pnetwork_ctl->Rx_frame_ctl.Index_buf);
			//issue upper layer
			ret = UDS_N_generate_USData_indication(&Network_service,
												   N_TIMEOUT_A,
												   buf);
			if(1 == ret){
				UDS_N_issue_to_upper_put(N_USDATA_INDICATION,&Network_service);
			}
		}
	}
}
static void UDS_N_As_time_out_reply(void)
{
	UDS_N_Count_Status_t *ptime = UDS_N_time_pointer_get(N_TIME_NAME_N_As);
	uds_int8_t ret = -1;
	
	if(!ptime){
		return;
	}

	if(ptime->Cnt_en){
		if(N_TIME_CNT_STS_TIMEOUT == ptime->Status){
			//stop timer As
			UDS_N_time_ctl_stop(N_TIME_NAME_N_As);
			//set status
			UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
			//generate failed USData.confirm
			ret = UDS_N_generate_USData_confirm(&Network_service,N_TIMEOUT_A);
			//issue to upper layer
			if(1 == ret){
				UDS_N_issue_to_upper_put(N_USDATA_CONFIRM,&Network_service);
			}
		}
	}
}
static void UDS_N_Br_time_out_reply(void)
{
	UDS_N_Count_Status_t *ptime = UDS_N_time_pointer_get(N_TIME_NAME_N_Br);

	if(!ptime){
		return;
	}

	if(ptime->Cnt_en){
		if(N_TIME_CNT_STS_TIMEOUT == ptime->Status){
			
		}
	}
}
static void UDS_N_Bs_time_out_reply(void)
{
	UDS_N_Count_Status_t *ptime = UDS_N_time_pointer_get(N_TIME_NAME_N_Bs);
	uds_int8_t ret = -1;

	if(!ptime){
		return;
	}

	if(ptime->Cnt_en){
		if(N_TIME_CNT_STS_TIMEOUT == ptime->Status){			
			//stop timer Bs
			UDS_N_time_ctl_stop(N_TIME_NAME_N_Bs);
			//set status
			UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
			//generate failed USData.confirm
			ret = UDS_N_generate_USData_confirm(&Network_service,N_TIMEOUT_Bs);
			//issue to upper layer
			if(1 == ret){
				UDS_N_issue_to_upper_put(N_USDATA_CONFIRM,&Network_service);
			}
		}
	}
}
static void UDS_N_Cr_time_out_reply(void)
{
	UDS_N_Count_Status_t *ptime = UDS_N_time_pointer_get(N_TIME_NAME_N_Cr);
	uds_int8_t ret = -1;
	uds_uint8_t *buf = UDS_NULL;
	UDS_N_Control_t *pnetwork_ctl = &Network_ctl;

	if(!ptime){
		return;
	}

	if(ptime->Cnt_en){
		if(N_TIME_CNT_STS_TIMEOUT == ptime->Status){
			//stop timer Cr
			UDS_N_time_ctl_stop(N_TIME_NAME_N_Cr);
			//set status
			UDS_N_rx_ctl_set_status(N_STS_RX_IDLE);
			//get buf
			buf = UDS_N_get_can_recive_buf(pnetwork_ctl->Rx_frame_ctl.Index_buf);
			//issue upper layer
			ret = UDS_N_generate_USData_indication(&Network_service,
												   N_TIMEOUT_Cr,
												   buf);
			if(1 == ret){
				UDS_N_issue_to_upper_put(N_USDATA_INDICATION,&Network_service);
			}
		}
	}
}
static void UDS_N_Cs_time_out_reply(void)
{
	UDS_N_Count_Status_t *ptime = UDS_N_time_pointer_get(N_TIME_NAME_N_Cs);
	uds_int8_t ret = -1;
	void *pdu = UDS_NULL;
	
	if(!ptime){
		return;
	}

	if(ptime->Cnt_en){
		if(N_TIME_CNT_STS_TIMEOUT == ptime->Status){
			UDS_N_time_ctl_stop(N_TIME_NAME_N_Cs);
			//generate CF frame
			ret = UDS_N_tx_generate_CF_frame(&pdu);
			if(1 > ret){
				UDS_PRINTF("UDS Error:generate CF frame fialed!\n\r");
			}
			//push CF to send
			if(1 == ret){
				ret = UDS_N_push_to_can_send_list(pdu);
			}
			if(1 == ret){
				//start timer As
				UDS_N_time_ctl_restart(N_TIME_NAME_N_As);
			}else{
				UDS_N_tx_ctl_set_status(N_STS_TX_TRANSMIT_FINISH);
			}
			if(1 != ret){//prepare failed
				if(UDS_NULL != pdu){
					UDS_N_pool_pointer_free(N_POOL_TYPE_PDU,pdu);
				}
			}
		}
	}
}

//Time manage
static void UDS_N_time_manage_deal(void)
{
	UDS_N_Ar_time_out_reply();
	UDS_N_As_time_out_reply();
	UDS_N_Br_time_out_reply();
	UDS_N_Bs_time_out_reply();
	UDS_N_Cr_time_out_reply();
	UDS_N_Cs_time_out_reply();
}
static uds_int8_t UDS_N_service_data_copy(UDS_N_Services_t *des,
										  UDS_N_Services_Issue_t *res,
										  uds_uint8_t *buf)
{
	uds_uint8_t *pres,*pdes;
	uds_int8_t ret = -1;
	uds_uint16_t i;
	
	if((!des) || (!res) || (!buf)){
		return ret;
	}

	ret = 1;
	pres = (uds_uint8_t *)&res->Service;
	pdes = (uds_uint8_t *)des;
	for(i = 0;i < sizeof(UDS_N_Services_t);i++){
		pdes[i] = pres[i];
	}
	if(N_USDATA_INDICATION == res->Type){
		for(i = 0;i < res->Service.USData_Indication.Length;i++){
			buf[i] = res->Service.USData_Indication.MessageData[i];
		}
		des->USData_Indication.MessageData = buf;
	}
	if(0xFF != res->Buf_index){
		ret = UDS_N_free_can_recive_buf(res->Buf_index);
	}	
	
	return ret;
}
/**********************************************************************
* @brief	   Pull the service message from network.
*
* @param[in]   res	 :UDS_N_Services_Issue_t pointer for pull service 
*					  data.
*			   buf	 :message data buf.
*
* @return	   UDS_N_Service_e  type 
*			   other:error
*			   N_USDATA_REQUEST				:N_USData.request 
*			   N_USDATA_CONFIRM				:N_USData.confirm
*			   N_USDATA_FF_INDICATION		:N_USData_FF.indication
*			   N_USDATA_INDICATION			:N_USData.indication
*			   N_CHANGE_PARAMETER_REQUEST	:N_ChangeParameters.request
*			   N_CHANGE_PARAMETER_CONFIRM	:N_ChangeParameter.confirm
*			   N_TYPE_ERROR					:error
**********************************************************************/
uds_uint8_t UDS_N_service_get(UDS_N_Services_t *res,uds_uint8_t *buf)
{
	UDS_N_Services_Issue_t *pdata = UDS_NULL;
	UDS_N_Services_t *pres = res;
	uds_uint8_t ret = N_TYPE_ERROR;

	if((!pres) || (!buf)){
		return ret;
	}
	
	//There are some messages to process
	if(0 < UDS_N_list_data_get(N_LIST_TYPE_ISSUE_UPPER,(void *)&pdata)){
		if(1 == UDS_N_service_data_copy(pres,pdata,buf)){
			ret = pdata->Type;
		}
		UDS_N_pool_pointer_free(N_POOL_TYPE_ISSUE,pdata);
	}

	return ret;
}
/**********************************************************************
* @brief	   Network main process.
*
* @param[in]   None.
*
* @return	   None
**********************************************************************/
void uds_network_all(void)
{
	//N_USData 
	UDS_N_service_deal();
	//Send manage
	UDS_N_send_manage_deal();
	//Rx can message handle
	UDS_N_rx_can_recive_data_deal();
	//Tx can message handle
	UDS_N_tx_can_recive_data_deal();
	//Frame send manage
	UDS_N_can_send_data_deal();
	//Timeout handle
	UDS_N_time_manage_deal();
}
