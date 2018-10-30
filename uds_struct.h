#ifndef _UDS_STRUCT_LAYER_H_
#define _UDS_STRUCT_LAYER_H_

#include "uds_cfg.h"
/*********************************************************************************************************************
 * standard types definitions
 *********************************************************************************************************************/

typedef signed char         uds_int8_t;
typedef unsigned char       uds_uint8_t;
typedef signed short        uds_int16_t;
typedef unsigned short      uds_uint16_t;
typedef signed long         uds_int32_t;
typedef unsigned long       uds_uint32_t;

/*********************************************************************************************************************
 * Genal types definitions
 *********************************************************************************************************************/
#define UDS_NULL 0
typedef enum
{
	MTYPE_DIAGNOSTICS = 0,		//Diagnostics
	MTYPE_REMOTE_DIAGNOSTICS	//Remote Diagnostics
} UDS_Mtype_e;

/*********************************************************************************************************************
 * Network layer types definitions
 *********************************************************************************************************************/
	
#ifdef ADDRESS_EXTENSION_MODE
#define SF_DATA_LENGTH 6
#define FF_DATA_LENGTH 5
#define CF_DATA_LENGTH 6
#define FC_DATA_LENGTH 4
#else
#define SF_DATA_LENGTH 7
#define FF_DATA_LENGTH 6
#define CF_DATA_LENGTH 7
#define FC_DATA_LENGTH 5
#endif

typedef enum
{
	N_USDATA_REQUEST = 0,		//N_USData.request 
	N_USDATA_CONFIRM,			//N_USData.confirm
	N_USDATA_FF_INDICATION,		//N_USData_FF.indication
	N_USDATA_INDICATION,		//N_USData.indication
	N_CHANGE_PARAMETER_REQUEST,	//N_ChangeParameters.request
	N_CHANGE_PARAMETER_CONFIRM,	//N_ChangeParameter.confirm
	N_TYPE_ERROR				//error
} UDS_N_Service_e;

typedef enum
{
	N_TATYPE_PHYSICAL = 0,	//1-to-1 communication
	N_TATYPE_FUNCTIONAL		//1-to-n communication
} UDS_N_TAtype_e;

typedef enum
{
	N_PARAMETER_STMIN = 0,	//STmin
	N_PARAMETER_BS,			//Block size
} UDS_N_Parameter_e;

typedef enum
{
	N_PDU_NAME_SF = 0,	//Single Frame
	N_PDU_NAME_FF = 1,	//First Frame
	N_PDU_NAME_CF = 2,	//Consecutive Frame
	N_PDU_NAME_FC = 3	//Flow Control Frame
} UDS_N_PDU_Name_e;

typedef enum
{
	N_FC_CTS   = 0,	//Continue To Send 
	N_FC_WT    = 1,	//wait
	N_FC_OVLFW = 2,	//Overflow
	N_FC_NONE  = 3	//none
} UDS_N_FC_Ctl_e;


typedef enum
{
	N_OK = 0,		//Execution has completed successfully
	N_TIMEOUT_A,	//When the timer N_Ar/N_As has passed
	N_TIMEOUT_Bs,	//When the timer N_Bs has passed
	N_TIMEOUT_Cr,	//When the timer N_Cr has passed
	N_WRONG_SN,		//Reception of an unexpected sequence number 
	N_INVALID_FS,	//Unknown FlowStatus value has been received
	N_UNEXP_PDU,	//Reception of an unexpected protocol data unit
	N_WFT_OVRN,		//Flow control WAIT frame that exceeds the maximum counter N_WFTmax. 
	N_BUFFER_OVFLW, //On reception of a flow control (FC) N_PDU with FlowStatus = OVFLW
	N_ERROR			//An error has been detected
} UDS_N_Result_e;
typedef enum
{
	N_PARA_CHANGE_OK = 0,	//Execution has completed successfully
	N_PARA_RX_ON,			//The service did not execute since a reception of the message identified by <AI> was taking place
	N_PARA_WRONG_PARAMETER,	//Undefined <Parameter>
	N_PARA_WRONG_VALUE		//Out of range <Parameter_Value>
} UDS_N_Result_Change_Parameter_e;

typedef enum
{
	N_TIME_NAME_N_As = 0,
	N_TIME_NAME_N_Ar,
	N_TIME_NAME_N_Bs,
	N_TIME_NAME_N_Br,
	N_TIME_NAME_N_Cs,
	N_TIME_NAME_N_Cr,
	N_TIME_NAME_N_ALL
} UDS_N_Time_Name_e;

typedef enum
{
	N_TIME_CNT_STS_IDLE = 0,
	N_TIME_CNT_STS_STOP,
	N_TIME_CNT_STS_RUN,
	N_TIME_CNT_STS_TIMEOUT
} UDS_N_TimeCnt_Status_e;

typedef enum
{
	N_STS_RX_IDLE = 0,		//Idle mode(sigle frame)
	N_STS_RX_MULTI_FRAME,	//Recive multi-frame mode
	N_STS_RX_ERR			//error mode
} UDS_N_Rx_Status_e;

typedef enum
{
	N_STS_TX_IDLE,		 			 //Idle mode
	N_STS_TX_WAIT_SEND_OK,			 //Wait frame send ok
	N_STS_TX_SEND_SF,				 //Send the SF frame
	N_STS_TX_SEND_FF,	 			 //Send the FF frame
	N_STS_TX_WAIT_FC,	 			 //Wait the FC frame mode
	N_STS_TX_MULTI_FRAME_IN_TRANSMIT,//Transmiting multi-frame mode
	N_STS_TX_TRANSMIT_FINISH,		 //Transmiting finish
	N_STS_TX_ERR		 			 //error mode
} UDS_N_Tx_Status_e;
	
typedef enum
{
	N_STS_IDLE 				 = 0,  //Idle mode(sigle frame)
	N_STS_MULTI_FRAME_RX 	 = 0x1,//Network recive multi-frame mode
	N_STS_MULTI_FRAME_TX 	 = 3,  //Network transmit multi-frame mode
	N_STS_MULTI_FRAME_DOUBLE = 4   //Network recive and transmit multi-frame mode
} UDS_N_Status_e;	

typedef enum
{
	N_LIST_TYPE_RX_CAN_RECIVE = 0,//Rx can recive list
	N_LIST_TYPE_TX_CAN_RECIVE,	  //Tx can recive list
	N_LIST_TYPE_ISSUE_UPPER,	  //Issue service to upper list
	N_LIST_TYPE_ISSUE_NETWORK,	  //Issue service to network list
	N_LIST_TYPE_CAN_SEND,		  //CAN send list
	N_LIST_TYPE_END
} UDS_N_List_Type_e;	

typedef enum
{
	N_POOL_TYPE_PDU = 0,//PDU pool
	N_POOL_TYPE_ISSUE,	//Issue pool
	N_POOL_TYPE_END
} UDS_N_Pool_Type_e;	


typedef struct
{
	UDS_Mtype_e					Mtype;		 //Message type
	uds_uint8_t					N_SA;		 //Network Source Address
	uds_uint8_t					N_TA;		 //Network Target Address
	UDS_N_TAtype_e				N_TAtype;	 //Network Target Address type
#ifdef ADDRESS_EXTENSION_MODE	
	uds_uint8_t 				N_AE;		 //Network Address Extension
#endif
} UDS_N_Service_Info_Pub_t;//N_USData.request 

typedef struct
{
	UDS_N_Service_Info_Pub_t	Info;		 //address information
	uds_uint8_t					*MessageData;//Data the higher layer entities exchange.
	uds_uint16_t				Length;		 //Length of data to be transmitted/received
} UDS_N_USData_Request_t;//N_USData.request 


typedef struct
{
	UDS_N_Service_Info_Pub_t	Info;		//address information
	UDS_N_Result_e				N_Result;	//Length of data to be transmitted/received
} UDS_N_USData_Confirm_t;//N_USData.confirm 

typedef struct
{
	UDS_N_Service_Info_Pub_t	Info;		 //address information
	uds_uint16_t				Length;		 //Length of data to be transmitted/received
} UDS_N_USData_FF_Indication_t;//N_USData_FF.indication 

typedef struct
{
	UDS_N_Service_Info_Pub_t	Info;		 //address information
	uds_uint8_t					*MessageData;//Data the higher layer entities exchange.
	uds_uint16_t				Length;		 //Length of data to be transmitted/received,Max 4095
	UDS_N_Result_e				N_Result;	 //Length of data to be transmitted/received
} UDS_N_USData_Indication_t;//N_USData.indication 

typedef struct
{
	UDS_N_Service_Info_Pub_t	Info;		 	 //address information
	uds_uint8_t					Parameter;		 //Identifies a parameter of the network layer.
	uds_uint8_t					Parameter_Value; //Parameter Value
} UDS_N_Change_Parameters_Request_t;//N_ChangeParameters.request 

typedef struct
{
	UDS_N_Service_Info_Pub_t		Info;		 			//address information
	uds_uint8_t						Parameter;		 		//Identifies a parameter of the network layer.
	UDS_N_Result_Change_Parameter_e	Result_ChangeParameter;	//Status relating to the outcome of a service execution
} UDS_N_Change_Parameters_Confirm_t;//N_ChangeParameters.confirm

typedef union
{
	UDS_N_USData_Request_t				USData_request;			  //N_USData.request 
	UDS_N_USData_Confirm_t				USData_confirm;			  //N_USData.confirm
	UDS_N_USData_FF_Indication_t		USData_FF_indication;	  //N_USData_FF.indication
	UDS_N_USData_Indication_t			USData_Indication;		  //N_USData.indication
	UDS_N_Change_Parameters_Request_t	Change_parameters_request;//N_ChangeParameters.request 
	UDS_N_Change_Parameters_Confirm_t	Change_parameters_confirm;//N_ChangeParameter.confirm
} UDS_N_Services_t;//Network services

typedef struct
{
	UDS_N_Services_t Service;  //Network services
	UDS_N_Service_e  Type;	   //Service type
	uds_uint8_t		 Status;   //status
	uds_uint8_t		 Buf_index;//Buffer index
} UDS_N_Services_Issue_t;//Network recive data parameter 

typedef struct
{
	uds_uint8_t		 buf[UDS_N_CAN_RX_BUFFER_MAX];//Buffer index
} UDS_N_Rx_Buffer_t;//Network recive buffer parameter 

typedef struct
{	
#ifdef ADDRESS_EXTENSION_MODE
	uds_uint8_t N_TA;				  //Extension address
#endif
	struct{
		uds_uint8_t  DL 	: 4;//Data length
		uds_uint8_t  PCItype: 4;//PCItype
	}PCI_Bit;
	uds_uint8_t	 N_Data[SF_DATA_LENGTH];//data
} UDS_N_SF_t;//SingleFrame

typedef struct
{	
#ifdef ADDRESS_EXTENSION_MODE
	uds_uint8_t N_TA;					//Extension address
#endif
	struct{
		uds_uint8_t  DL_MSB : 4;		//Data length least significant bit
		uds_uint8_t  PCItype: 4;		//PCItype
	}PCI_Bit;
	uds_uint8_t  DL_LSB;				//Data length most significant bit
	uds_uint8_t	 N_Data[FF_DATA_LENGTH];//data
} UDS_N_FF_t;//FirstFrame

typedef struct
{	
#ifdef ADDRESS_EXTENSION_MODE
	uds_uint8_t N_TA;					//Extension address
#endif
	struct{
		uds_uint8_t  SN 	: 4;		//Sequence Number
		uds_uint8_t  PCItype: 4;		//PCItype
	}PCI_Bit;
	uds_uint8_t	 N_Data[CF_DATA_LENGTH];//data
} UDS_N_CF_t;//ConsecutiveFrame

typedef struct
{	
#ifdef ADDRESS_EXTENSION_MODE
	uds_uint8_t N_TA;					 //Extension address
#endif
	struct{
		uds_uint8_t  FS 	: 4;		 //Flow Status
		uds_uint8_t  PCItype: 4;		 //PCItype
	}PCI_Bit;
	uds_uint8_t	 BS;					 //Block size
	uds_uint8_t	 STmin;					 //send time min
	uds_uint8_t	 Reveser[FC_DATA_LENGTH];//Reveser data
} UDS_N_FC_t;//FlowControl 

typedef struct
{	
#ifdef ADDRESS_EXTENSION_MODE
	uds_uint8_t N_TA;			//Extension address
#endif
	struct{
		uds_uint8_t  Reveser: 4;//Reveser
		uds_uint8_t  PCItype: 4;//PCItype
	}PCI_Bit;
} UDS_N_Frame_PCI_t;//Frame PCI


typedef union
{
	UDS_N_Frame_PCI_t	PCI;	//PCI
	UDS_N_SF_t			SF; 	//SingleFrame
	UDS_N_FF_t			FF; 	//FirstFrame
	UDS_N_CF_t			CF; 	//ConsecutiveFrame
	UDS_N_FC_t			FC; 	//FlowControl
	uds_uint8_t			Data[8];//data
} UDS_N_Frame_t;//Network frame 

typedef struct
{	
	uds_uint32_t	ID;		//id
	uds_uint8_t		Length; //data length
	UDS_N_Frame_t	Frame;  //Frame
} UDS_N_PDU_t;//Network frame 

typedef struct
{	
	uds_uint8_t	 BS;					 //Block size
	uds_uint8_t	 STmin;					 //send time min
} UDS_N_FC_Para_t;//FlowControl parameter


typedef struct
{
	uds_uint16_t	N_As;
	uds_uint16_t	N_Ar;
	uds_uint16_t	N_Bs;
	uds_uint16_t	N_Br;
	uds_uint16_t	N_Cs;
	uds_uint16_t	N_Cr;
} UDS_N_Timing_Parameter_t;//Network Timing Parameter 

typedef struct
{
	UDS_N_TimeCnt_Status_e		Status; //0:idle 1:stop 2:run 3:time out
	uds_uint8_t					Cnt_en; //0:no 1:yes
	uds_uint16_t				Cnt;
	uds_uint16_t				Target;
} UDS_N_Count_Status_t;//Network Timing Parameter 

typedef struct
{
	UDS_N_PDU_t PDU;					 //data pdu
	uds_uint8_t Status; 				 //status
	void		(*Hook)(uds_int8_t res); //send finish hook	
} UDS_N_PDU_Pool_t;//Network recive data parameter 

typedef union
{
	uds_uint8_t	N_status;
	struct{
		UDS_N_Tx_Status_e		Tx_status:4; //Transimite status
		UDS_N_Rx_Status_e		Rx_status:4; //Recive status
	}Bit;
} UDS_N_Status_t;//Network status 

typedef struct
{
	uds_uint16_t	Frame_cnt_taget;   		//Tx/Rx target frame num.
	uds_uint16_t	Frame_cnt_current; 		//Tx/Rx current frame num.
	uds_uint16_t	Buffer_pointer_target;	//Tx/Rx target buffer pointer target.
	uds_uint16_t	Buffer_pointer_current; //Tx/Rx target buffer pointer current.
	uds_uint8_t		SN_last;				//Tx/Rx CF serial number.
	uds_uint16_t	BS_cnt_taget;   		//Tx/Rx block size target frame num.
	uds_uint16_t	BS_cnt_current; 		//Tx/Rx block size current frame num.	
	uds_uint16_t	Index_buf; 				//Tx/Rx buffer index.	
} UDS_N_Multi_Ctl_t;//Network multi-framme control 

typedef struct
{
	UDS_N_Status_t			 Status; 			  //Network status
	UDS_N_Multi_Ctl_t		 Tx_frame_ctl;		  //Tx frame control
	UDS_N_Service_Info_Pub_t Tx_service_info;	  //Tx service information
	UDS_N_Multi_Ctl_t		 Rx_frame_ctl;		  //Rx frame control
	UDS_N_Service_Info_Pub_t Rx_service_info;	  //Rx service information
	uds_uint8_t				 Service_request_flag;//0:idle 1:busy
} UDS_N_Control_t;//Network control data parameter 



/*********************************************************************************************************************
 * Session layer types definitions
 *********************************************************************************************************************/
typedef enum
{
	S_TIME_P2_CAN_CLIENT = 0, //P2CAN_Client
	S_TIME_P2X_CAN_CLIENT,	  //P2*CAN_Client
	S_TIME_P2_CAN_SERVER,	  //P2CAN_Server
	S_TIME_P2X_CAN_SERVER,	  //P2*CAN_Server
	S_TIME_P3_CAN_CLIENT_PHYS,//P3CAN_Client_Phys
	S_TIME_P3_CAN_CLIENT_FUNC,//P3CAN_Client_Func
	S_TIME_P3_CLIENT,		  //P3CAN_Client
	S_TIME_S3_SERVER,		  //S3Client
	S_TIME_NAME_ALL
} UDS_S_Time_Name_e;
	
typedef enum
{
	S_TIME_CNT_STS_IDLE = 0,//Idle
	S_TIME_CNT_STS_STOP,	//Stop
	S_TIME_CNT_STS_RUN,		//Run
	S_TIME_CNT_STS_TIMEOUT	//Timeout
} UDS_S_TimeCnt_Status_e;

typedef enum
{
	S_STS_IDLE = 0,//P2CAN_Client
} UDS_S_Session_Status_e;

typedef enum
{
	S_TYPE_DEFAULT = 0,//Default session
	S_TYPE_NON_DEFAULT//Non-default session
} UDS_S_Session_Type_e;


typedef enum
{
	S_REQ_TYPE_NORMAL = 0,	//NORMAL request
	S_REQ_TYPE_NRC,			//NRC request
	S_REQ_TYPE_DSC			//DSC request
} UDS_S_Request_Type_e;


typedef UDS_N_Count_Status_t	UDS_S_Count_Status_t;

typedef struct
{
	uds_uint16_t	P2CAN_Client;
	uds_uint16_t	P2xCAN_Client;
	uds_uint16_t	P2CAN_Server;
	uds_uint16_t	P2xCAN_Server;
	uds_uint16_t	P3CAN_Client_Phys;
	uds_uint16_t	P3CAN_Client_Func;
	uds_uint16_t	P3CAN_Client;
	uds_uint16_t	S3_Client;
} UDS_S_Timing_Parameter_t;//Session Timing Parameter 

typedef struct
{
	UDS_S_Session_Type_e	Session_sts;//Session status
} UDS_S_Control_t;//Session control 


/*********************************************************************************************************************
 * Application layer types definitions
 *********************************************************************************************************************/


/*********************************************************************************************************************
 * Interface types definitions
 *********************************************************************************************************************/
typedef struct
{
	void 		(*time_manage_handle)(void);
	void		(*main_proc)(void);
	uds_int8_t	(*init)(void);
	uds_int8_t  (*can_push_data)(uds_uint32_t id,
								 uds_uint8_t length,
								 uds_uint8_t *data);
	uds_uint8_t (*service_get)(UDS_N_Services_t *res,
							   uds_uint8_t *buf);
	uds_int8_t  (*USData_request)(void *pdata);
} UDS_Network_Interface_t;//UDS network interface 

typedef struct
{
	void 		(*time_manage_handle)(void);
	void		(*main_proc)(void);
	uds_int8_t	(*init)(void);
	uds_uint8_t (*service_get)(UDS_N_Services_t *res,
							   uds_uint8_t *buf);
	uds_int8_t  (*USData_request)(void *pdata);
	uds_int8_t  (*time_ctl_stop)(UDS_S_Time_Name_e time);
	uds_int8_t  (*time_ctl_run)(UDS_S_Time_Name_e time);
	uds_int8_t  (*time_ctl_restart)(UDS_S_Time_Name_e time);
	uds_int8_t  (*time_ctl_reset)(UDS_S_Time_Name_e time);
	uds_int8_t	(*time_status_get)(UDS_S_Time_Name_e time);
} UDS_Session_Interface_t;//UDS session interface 

typedef struct
{
	UDS_S_Session_Type_e	Session_sts;//Session status
} UDS_App_Interface_t;//UDS app interface 


typedef struct
{
	UDS_Network_Interface_t	Network;	//Network
	UDS_Session_Interface_t	Session;	//Session
	UDS_App_Interface_t		Application;//Application
} UDS_Interface_In_t;//UDS interface 

typedef struct
{
	uds_uint8_t (*can_send_hook)(uds_uint32_t id,
								 uds_uint8_t length,
								 uds_uint8_t *data);

} UDS_Interface_Ext_t;//UDS interface

unsigned char m_can_send_hook(unsigned long id,
							  unsigned char length,
							  unsigned char *data);


#endif

