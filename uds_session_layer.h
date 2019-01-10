#ifndef _UDS_SESSION_LAYER_H_
#define _UDS_SESSION_LAYER_H_

#include "uds_struct.h"

//Session time cnt
void UDS_S_time_manage_handle(void);
//Session init
uds_int8_t UDS_S_init(void);
//Session layer all
void uds_session_all(void);
//App get the session layer timer status
uds_int8_t UDS_S_time_status_get(UDS_S_Time_Name_e time);
//App get the lower layer service
uds_uint8_t UDS_S_service_get(UDS_N_Services_t *res,uds_uint8_t *buf);
//App layer issue a USData.request service
uds_int8_t UDS_S_service_process_USData_request(void *pdata);
//App layer issue a ParaChange.request service
uds_int8_t UDS_S_service_process_ParaChange_request(void *pdata);

uds_int8_t UDS_S_time_ctl_stop(UDS_S_Time_Name_e time);
uds_int8_t UDS_S_time_ctl_run(UDS_S_Time_Name_e time);
uds_int8_t UDS_S_time_ctl_restart(UDS_S_Time_Name_e time);
uds_int8_t UDS_S_time_ctl_reset(UDS_S_Time_Name_e time);
uds_uint16_t UDS_S_time_value_get(UDS_S_Time_Name_e time);



#endif
