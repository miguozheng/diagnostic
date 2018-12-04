#ifndef _UDS_NETWORK_LAYER_H_
#define _UDS_NETWORK_LAYER_H_

#include "uds_struct.h"

//network init
uds_int8_t UDS_N_init(void);
//put can message to network
uds_int8_t UDS_N_can_data_put(uds_uint32_t id,
							  uds_uint8_t length,
							  uds_uint8_t *data);
//get network response service
uds_uint8_t UDS_N_service_get(UDS_N_Services_t *res,uds_uint8_t *buf);
//issue a USData.request to network
uds_int8_t UDS_N_service_process_USData_request(void *pdata);
//network time manage handle
void UDS_N_time_manage_handle(void);
//network main process
void uds_network_all(void);

#endif

