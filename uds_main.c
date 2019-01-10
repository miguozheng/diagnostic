/**  @uds_main.c
*     @note 2018 crystal-optech Co., Ltd. All Right Reserved.
*     @brief UDS main deal.
*     @author     ZhengLong
*     @date        2018/09/18
*     @note
*     @warning 
*/
#include "uds_common_include.h"
#include "uds_struct.h"
/*************************1*****************************************************
Private global variables and functions
*******************************************************************************/
static uds_uint8_t UDS_run_enable = 0;//enable uds run.


/*
============================================================================
 Function declear
============================================================================
*/
static void (*uds_network_time_manage_handle)(void);
static void (*uds_session_time_manage_handle)(void);
static void (*uds_network_proc)(void);
static void (*uds_session_proc)(void);
static void (*uds_application_proc)(void);
static uds_int8_t (*_uds_add_DTC)(DTCNode_Config *dtc_iterm);


static uds_int8_t (*uds_can_data_push)(uds_uint32_t id,
									   uds_uint8_t length,
									   uds_uint8_t *data);
static void uds_self_regist(void);


/*
============================================================================
 User function
============================================================================
*/
static void uds_self_regist(void)
{
	uds_int8_t ret = 0;
	UDS_Interface_In_t *inf = (UDS_Interface_In_t *)get_uds_internal_interface();

	if(inf->Network.main_proc){
		uds_network_proc = inf->Network.main_proc;
	}else{
		ret |= -1;
	}
	if(inf->Network.time_manage_handle){
		uds_network_time_manage_handle = inf->Network.time_manage_handle;
	}else{
		ret |= -1;
	}
	if(inf->Network.can_push_data){
		uds_can_data_push = inf->Network.can_push_data;
	}else{
		ret |= -1;
	}
	if(inf->Session.main_proc){
		uds_session_proc = inf->Session.main_proc;
	}else{
		ret |= -1;
	}
	if(inf->Session.time_manage_handle){
		uds_session_time_manage_handle = inf->Session.time_manage_handle;
	}else{
		ret |= -1;
	}
	if(inf->Application.main_proc){
		uds_application_proc = inf->Application.main_proc;
	}else{
		ret |= -1;
	}

	if(-1 == ret){
		UDS_run_enable |= 0x02;
		UDS_PRINTF("UDS Error:UDS self regist failed!\n\r");
	}

}
uds_int8_t uds_add_DTC(DTCNode_Config *dtc_iterm)
{
	uds_int8_t ret = -1;

	if(_uds_add_DTC){
		ret = _uds_add_DTC(dtc_iterm);
	}else{
		UDS_PRINTF("UDS Error:Try again after UDS init complete!\n\r");
	}

	return ret;
}


void uds_init(void)
{
	uds_int8_t ret = 0;
	UDS_Interface_In_t *inf = (UDS_Interface_In_t *)get_uds_internal_interface();
	
	//network layer init
	if(inf->Network.init){
		ret = inf->Network.init();
		if(1 != ret){
			UDS_PRINTF("UDS Error:Network layer init failed!\n\r");
		}
	}
	if(1 != ret){
		//Forbidden run uds
		UDS_run_enable |= 0x01;
	}
	//session layer init
	if(inf->Session.init){
		ret = inf->Session.init();
		if(1 != ret){
			UDS_PRINTF("UDS Error:Session layer init failed!\n\r");
		}
	}
	if(1 != ret){
		//Forbidden run uds
		UDS_run_enable |= 0x01;
	}
	//appliacation service init
	if(inf->Application.init){
		ret = inf->Application.init();
		if(1 != ret){
			UDS_PRINTF("UDS Error:Application layer init failed!\n\r");
		}
	}
	if(inf->Application.DTC_add_iterm){
		_uds_add_DTC = inf->Application.DTC_add_iterm;
	}
	if(1 != ret){
		//Forbidden run uds
		UDS_run_enable |= 0x01;
	}
	//eeprom init
	if(inf->Eeprom.init){
		ret = inf->Eeprom.init();
		if(1 != ret){
			UDS_PRINTF("UDS Error:EEPROM init failed!\n\r");
		}
	}
	//self regist
	uds_self_regist();
}
//UDS time handle
void uds_time_handle(void)
{
   if(!UDS_run_enable){
	   //network layer process
	   uds_network_time_manage_handle();
	   //session layer process
	   uds_session_time_manage_handle();
	   //appliacation service
   }
}
//put can message to network
uds_int8_t uds_can_data_put(uds_uint32_t id,
							uds_uint8_t length,
							uds_uint8_t *data)
{
	return uds_can_data_push(id,length,data);
}

void uds_proc_main(void)
{
	if(!UDS_run_enable){
		//network layer process
		uds_network_proc();
		//session layer process
		uds_session_proc();
		//appliacation service
		uds_application_proc();
	}
}

