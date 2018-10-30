#ifndef _UDS_CFG_H_
#define _UDS_CFG_H_


//#define ADDRESS_EXTENSION_MODE  						//扩展模式
#define UDS_DIAGNOSTICS_PHYSICAL_ID				0x00	//诊断物理ID
#define UDS_DIAGNOSTICS_FUNCTIONAL_ID			0x02	//诊断功能ID
#define UDS_DIAGNOSTICS_SELF_ID					0x706	//断自身ID

#define UDS_N_CAN_PDU_POOL_MAX  				255		//CAN接收池最大帧数
#define UDS_N_RX_BUFFER_MAX						4095	//发送buffer最大值
#define UDS_N_CAN_RX_BUFFER_MAX					4095	//接收buffer最大值
#define UDS_N_CAN_RX_BUFFER_NUM					5		//接收buffer个数

#define UDS_N_SERVICE_ISSUE_POOL_MAX  			20		//服务池大小


#endif

