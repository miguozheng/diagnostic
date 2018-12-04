/**  @UDS_S.c
*     @note 2018 crystal-optech Co., Ltd. All Right Reserved.
*     @brief UDS session deal.
*     @author     
*     @date        2018/10/15
*     @note
*     @warning 
*/
#include "uds_common_include.h"
#include "uds_struct.h"
#include "slist.h"
/*************************1*****************************************************
Private global variables and functions
******************************************************************************/
//Time parameter config
const  UDS_S_Timing_Parameter_t Session_Time_config = 
{
	0,
	0,
	2000,
	2000,
	0,
	0,
	0,
	10000,
	10000
};
//Time control
static UDS_S_Count_Status_t Session_Time_Man[S_TIME_NAME_ALL];
/*
============================================================================
 Function declear
============================================================================
*/
static UDS_S_Count_Status_t *UDS_S_time_pointer_get(uds_uint8_t time_type);
static void UDS_S_time_manage_init(void);
uds_uint8_t (*S_service_get)(UDS_N_Services_t *res,uds_uint8_t *buf);
uds_int8_t (*S_USData_request)(void *pdata);

/*
============================================================================
 User function
============================================================================
*/

static UDS_S_Count_Status_t *UDS_S_time_pointer_get(uds_uint8_t time_type)
{
	UDS_S_Count_Status_t *ptime = UDS_NULL;

	if(S_TIME_NAME_ALL > time_type){
		ptime = &Session_Time_Man[time_type];
	}

	return ptime;
}

//Stop time count
uds_int8_t UDS_S_time_ctl_stop(UDS_S_Time_Name_e time)
{
	UDS_S_Count_Status_t	*ptime = UDS_S_time_pointer_get(time);
	uds_int8_t ret = -1;

	if(!ptime){
		return ret;
	}

	ptime->Cnt_en = 0;
	ptime->Status = S_TIME_CNT_STS_STOP;
	ret = 1;
	
	return ret;
}
//Run time count
uds_int8_t UDS_S_time_ctl_run(UDS_S_Time_Name_e time)
{
	UDS_S_Count_Status_t	*ptime = UDS_S_time_pointer_get(time);
	uds_int8_t ret = -1;

	if(!ptime){
		return ret;
	}

	ptime->Cnt_en = 1;
	ptime->Status = S_TIME_CNT_STS_RUN;
	ret = 1;
	
	return ret;
}
//Stop time count
uds_int8_t UDS_S_time_ctl_restart(UDS_S_Time_Name_e time)
{
	UDS_S_Count_Status_t	*ptime = UDS_S_time_pointer_get(time);
	uds_int8_t ret = -1;

	if(!ptime){
		return ret;
	}

	ptime->Cnt_en = 0;
	ptime->Status = S_TIME_CNT_STS_IDLE;
	ptime->Cnt = ptime->Target;
	ptime->Cnt_en = 1;
	ret = 1;
	
	return ret;
}
//Reset time count
uds_int8_t UDS_S_time_ctl_reset(UDS_S_Time_Name_e time)
{
	UDS_S_Count_Status_t	*ptime = UDS_S_time_pointer_get(time);
	uds_int8_t ret = -1;

	if(!ptime){
		return ret;
	}

	ptime->Cnt_en = 0;
	ptime->Status = S_TIME_CNT_STS_IDLE;
	ptime->Cnt = ptime->Target;
	ret = 1;
	
	return ret;
}
//Get time count status
uds_int8_t UDS_S_time_status_get(UDS_S_Time_Name_e time)
{
	UDS_S_Count_Status_t	*ptime = UDS_S_time_pointer_get(time);
	uds_int8_t ret = -1;

	if(!ptime){
		return ret;
	}
	
	ret = ptime->Status;
	
	return ret;
}
//Time init
static void UDS_S_time_manage_init(void)
{
	UDS_S_Count_Status_t	 *ptime = Session_Time_Man;
	uds_uint16_t *config = (uds_uint16_t *)&Session_Time_config;
	uds_uint8_t i;

	for(i = 0;i < S_TIME_NAME_ALL;i++,ptime++){
		ptime->Cnt = 0;
		ptime->Status = S_TIME_CNT_STS_IDLE;
		ptime->Cnt_en = 0;
		ptime->Target = config[i];
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
uds_int8_t UDS_S_service_process_USData_request(void *pdata)
{
	void *pmessage = pdata;
	uds_int8_t ret = -1;

	if(!pmessage){
		return ret;
	}

	if(S_USData_request){
		ret = S_USData_request(pmessage);
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
uds_uint8_t UDS_S_service_get(UDS_N_Services_t *res,uds_uint8_t *buf)
{
	UDS_N_Services_t *pres = res;
	uds_uint8_t ret = N_TYPE_ERROR;

	if((!pres) || (!buf)){
		return ret;
	}

	if(S_service_get){
		ret = S_service_get(pres,buf);
	}
	
	return ret;
}

/**********************************************************************
* @brief	   Session layer time manage handle.
*
* @param[in]   None.
*
* @return	   None.
**********************************************************************/
void UDS_S_time_manage_handle(void)
{
	UDS_N_Count_Status_t	*ptime = Session_Time_Man;
	uds_uint8_t i;

	for(i = 0;i < S_TIME_NAME_ALL;i++,ptime++){
		if(ptime->Cnt_en){
			if(ptime->Cnt){
				ptime->Cnt--;
				ptime->Status = S_TIME_CNT_STS_RUN;
			}else{
				ptime->Status = S_TIME_CNT_STS_TIMEOUT;
			}
		}
	}
}
/**********************************************************************
* @brief	   Init seesion layer
*
* @param[in]   None.
*
* @return	   None.
**********************************************************************/
uds_int8_t UDS_S_init(void)
{
	UDS_Interface_In_t *intf = (UDS_Interface_In_t *)get_uds_internal_interface();
	uds_int8_t ret = 1;

	UDS_S_time_manage_init();
	if(intf->Network.service_get){
		S_service_get = intf->Network.service_get;
	}else{
		ret = -1;
		S_service_get = UDS_NULL;
		UDS_PRINTF("UDS Error:Session layer service_get regist failed£¡\n\r");
	}
	if(intf->Network.USData_request){
		S_USData_request = intf->Network.USData_request;
	}else{
		ret = -1;
		S_USData_request = UDS_NULL;
		UDS_PRINTF("UDS Error:Session layer USData_request regist failed£¡\n\r");
	}
	
	return ret;
}
/**********************************************************************
* @brief	   Session main process.
*
* @param[in]   None.
*
* @return	   None.
**********************************************************************/
void uds_session_all(void)
{

}


