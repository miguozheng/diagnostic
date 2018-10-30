#ifndef _UDS_H_
#define _UDS_H_
#include "uds_struct.h"
//CAN reciver put the messages to the UDS
uds_int8_t uds_can_data_put(uds_uint32_t id,
							uds_uint8_t length,
							uds_uint8_t *data);
//UDS init all
void uds_init(void);
//UDS time handle
void uds_time_handle(void);
//UDS process main
void uds_proc_main(void);

#endif

