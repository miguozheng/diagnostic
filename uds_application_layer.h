#ifndef _UDS_APPLICATION_LAYER_H_
#define _UDS_APPLICATION_LAYER_H_

#include "uds_struct.h"


//Application init
uds_int8_t UDS_A_init(void);
//Application layer all
void uds_application_all(void);
//DTC test add
uds_int8_t UDS_A_AddDTC_Init(DTCNode_Config *dtc_iterm);


#endif
