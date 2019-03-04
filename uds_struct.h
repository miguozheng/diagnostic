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
#define UDS_N_CAN_PDU_POOL_MAX  				80							//CAN数据池最大帧数
#define UDS_N_RX_BUFFER_MAX						UDS_RECIVE_BUFFER_LENGTH	//发送buffer最大值
#define UDS_N_CAN_RX_BUFFER_MAX					UDS_RECIVE_BUFFER_LENGTH	//接收buffer最大值
#define UDS_N_CAN_RX_BUFFER_NUM					5							//接收buffer个数
#define UDS_N_SERVICE_ISSUE_POOL_MAX  			10							//服务池大小

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
	N_PARA_CH_STmin = 0,//Send time min
	N_PARA_CH_BS,		//Block size
	N_PARA_CH_ERROR		//type end
} UDS_N_Change_Parameter_e;

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
	UDS_N_Change_Parameter_e	Parameter;		 //Identifies a parameter of the network layer.
	uds_uint8_t					Parameter_Value; //Parameter Value
} UDS_N_Change_Parameters_Request_t;//N_ChangeParameters.request 

typedef struct
{
	UDS_N_Service_Info_Pub_t		Info;		 			//address information
	UDS_N_Change_Parameter_e		Parameter;		 		//Identifies a parameter of the network layer.
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
	S_TIME_S3_CLIENT,		  //P3CAN_Client
	S_TIME_S3_SERVER,		  //S3Client
	S_TIME_SECURITYDELAY_SERVER,//Security_Delay
	S_TIME_NAME_ALL
} UDS_S_Time_Name_e;
	
typedef enum
{
	S_TIME_CNT_STS_IDLE = 0,//Idle
	S_TIME_CNT_STS_STOP,	//Stop
	S_TIME_CNT_STS_RUN,		//Run
	S_TIME_CNT_STS_TIMEOUT	//Timeout
} UDS_S_TimeCnt_Status_e;



typedef UDS_N_Count_Status_t	UDS_S_Count_Status_t;

typedef struct
{
	uds_uint16_t	P2CAN_Client;
	uds_uint16_t	P2xCAN_Client;
	uds_uint16_t	P2CAN_Server;
	uds_uint16_t	P2xCAN_Server;
	uds_uint16_t	P3CAN_Client_Phys;
	uds_uint16_t	P3CAN_Client_Func;
	uds_uint16_t	S3_Client;
	uds_uint16_t	S3_Server;
	uds_uint16_t	SecurityDelay_Server;
} UDS_S_Timing_Parameter_t;//Session Timing Parameter 

/*********************************************************************************************************************
 * Application layer types definitions
 *********************************************************************************************************************/
#ifndef UDS_FALSE	
#define	UDS_FALSE 0U
#endif

#ifndef UDS_TRUE
#define	UDS_TRUE 1U	
#endif

#ifndef UDS_A_NULL
#define UDS_A_NULL (void*)(0)
#endif

#ifndef NO_TAG
#define NO_TAG 0U
#endif

/*Test variable*/
#define VEHICLESPEED_TEST 73U
#define ENGINERPM_TEST 2498U
#define VOLTAGE_TEST 13501U

/*UUDT-ID*/
#define PERIODICDID_SEND_ID UDS_DIAGNOSTICS_PERIODIC_ID

/*诊断服务*/
#define NUMBEROFSERVICE 21U				//支持的诊断服务数量---暂定
#define MAXKEYERRORPERMITCOUNT 3U		//最大密钥失败允许次数---暂定

#define MAXNUMBEROFDID 10U				//客户端一次请求的DID数目最大值---暂定
#define NUMOFREADDID 50U				//ECU支持的ReadDataByIdentifier与ReadDataByPeriodicIdentifier，DID个数---暂定
#define MAXNUMBEROFPERIODICDID 10U		//一次请求周期数据标识符允许的最大个数---暂定

#define MAX_DTC_NUMBER 3U				//ECU支持的最大DTC数目---暂定-Test
#define MAX_DTCGROUP_NUMBER 5U			//ECU支持的最大DTC组数目---暂定
#define MAX_DTC_EEPROM_BYTES 1U			//每个DTC需要保存的字节数，DTC状态+DTC扩展数据---暂定
#define MAX_RECORDNUMBER_NUMBER 3U		//Snapshot的最大Recordnumber---暂定
#define MAX_DTC_SNAPSHOT_STORE_BYTES 256U//每个DTCSnapshot保存的最大字节数
#define DTC_SNAPSHOT_RECORDNUMBER01 1U	//Recordnumber=1的Snapshot
#define DTC_SNAPSHOT_RECORDNUMBER02 2U	//Recordnumber=2的Snapshot
#define DTC_SNAPSHOT1_EEPROM_BYTES 1U	//DTCSnapshot1需要保存的字节数，根据程序设计变更---暂定
#define DTC_SNAPSHOT2_EEPROM_BYTES 2U	//DTCSnapshot2需要保存的字节数，根据程序设计变更---暂定
#define DTC_SNAPSHOT3_EEPROM_BYTES 2U	//DTCSnapshot3需要保存的字节数，根据程序设计变更---暂定
#define DTC_SNAPSHOT_BYTES (DTC_SNAPSHOT1_EEPROM_BYTES + DTC_SNAPSHOT2_EEPROM_BYTES + DTC_SNAPSHOT3_EEPROM_BYTES) //DTCSnapshot所占字节数
#define DTC_SNAPSHOT1_STARTNUM 0U		//DTCSnapshot1保存数据的起始序号---暂定
#define DTC_SNAPSHOT2_STARTNUM DTC_SNAPSHOT1_EEPROM_BYTES //DTCSnapshot2保存数据的起始序号---暂定
#define DTC_SNAPSHOT3_STARTNUM (DTC_SNAPSHOT1_EEPROM_BYTES + DTC_SNAPSHOT2_EEPROM_BYTES) //DTCSnapshot3保存数据的起始序号---暂定
#define INVALID_DTCEXTDATARECORDNUMBER 0xFFU //无效的DTC扩展数据序号

#define PREFAILEDSTEP 2					//Test Sample测试失败计数器步幅
#define PREPASSEDSTEP 1					//Test Sample测试成功计数器步幅
#define FAILEDFAULTDETECTIONTIMES 20	//测试失败判断阈值
#define PASSEDFAULTDETECTIONTIMES (-20)	//测试成功判断阈值
#define DETECTIMES 100U					//一个测试循环包含的测试次数
#define DTCCONFIRMATIONTHRESHOLD 2U		//DTC确认的循环数
#define MAXDTCAGINGTIMES 10U			//故障老化阈值

#define TRANSMISSIONMODE_SLOWRATE 2000U	//低速发送模式，不能小于函数最小运行周期
#define TRANSMISSIONMODE_MEDIUMRATE 1000U//中速发送模式--暂定
#define TRANSMISSIONMODE_FASTRATE 500U	//快速发送模式--暂定
#define PERIODICDIDDATARUNPERIOD 10U	//周期响应数据函数运行周期10ms,需要与函数调用周期一直
#define DATAPUTCYCLE 1U					//周期响应数据填充一次数据的延时运行周期数
#define MAX_PERIODICDID_DATALENGTH 5U   //Standard周期DID数据最大字节长度

#define MAX_SOURCEDATARECORD_NUMBER 50U //源数据数目
#define MAX_SOURCEDATARECORD_POSITION 100U //源数据最大位置
#define DYNAMICALLY_DEFINE_PDID_COUNTER 1U //0X2C动态定义的PDID，重新定义后为运行状态,周期计数器的状态，0-Clear,1-Keep

#define VIN_NUMBER_BYTES 17U			//VIN字节数

#define INPUTOUTPUTCONTROL_DID0 0x0579U //输入输出控制参数DID0
#define ENGINE_RPM_DEFAULT 800U			//IO控制参数发送机转速默认值800
#define INPUTOUTPUTCONTROL_DID1 0xAA35U	//输入输出控制参数DID1
#define VEHICLE_SPEED_DEFAULT 50U		//IO控制参数车速默认值50
#define VOLTAGE_DEFAULT 1000U			//IO控制参数电压默认值1000mv

#define MAX_ROUTINE_NUMBER 10U			//最大例程数目
	
/*数据发送最大字节数*/
#define MAXDATALENGTH UDS_RECIVE_BUFFER_LENGTH
	
/*诊断源地址与目标地址*/
#define UDS_A_SOURCEADDRESS (UDS_DIAGNOSTICS_SELF_ID & 0xff)
#define UDS_A_TARGETADDRESS 0X05

typedef enum
{
	A_SES_TIME_CTL_STS_IDLE = 0,//Idle
	A_SES_TIME_CTL_STS_P2RUN,	//P2 run
	A_SES_TIME_CTL_STS_P2STOP, 	//P2 stop
	A_SES_TIME_CTL_STS_P2xRUN,	//P2* run
	A_SES_TIME_CTL_STS_P2xSTOP, //P2* stop
	A_SES_TIME_CTL_STS_S3RUN,	//S3 run
	A_SES_TIME_CTL_STS_S3STOP, 	//S3 stop
	A_SES_TIME_CTL_STS_ERROR	//Error
} UDS_Session_TimeCtl_Status_e;//Session time control status enum

typedef enum
{
	A_SES_IND_NONE = 0, //None
	A_SES_IND_NORMAL,	//Normal indication
	A_SES_IND_FF,		//FF indication
	A_SES_IND_TP,		//TP indication
	A_SES_IND_ERROR		//Error
} UDS_Session_Ind_Type_e;//Session recive indication type

typedef enum
{
	A_SES_REQ_NONE = 0, //None
	A_SES_REQ_NORMAL,	//Normal request
	A_SES_REQ_ERROR		//Error
} UDS_Session_Req_Type_e;//Session recive request type

typedef enum
{
	A_SES_CON_NONE = 0,//None
	A_SES_CON_NORMAL,  //Normal confirm
	A_SES_CON_NRC,	   //Negative response code confirm
	A_SES_CON_ERROR    //Error
} UDS_Session_Con_Type_e;//Session recive confirm type

typedef enum
{
	A_SES_STS_DEFAULT = 0,//Default session
	A_SES_STS_NON_DEFAULT,//Non-default session
	A_SES_STS_ERROR		  //Error
} UDS_Session_Status_e;//Session status type

typedef enum
{
	A_SES_SUP_NONE = 0,//none
	A_SES_SUP_POSITIVE,//supress positive response
	A_SES_SUP_ERROR	   //Error
} UDS_Session_Suppress_Type_e;//Session supress positive response type

typedef struct
{
	UDS_Session_TimeCtl_Status_e	Status; //Current control status
	UDS_Session_Status_e			Ses_sts;//Session status
	UDS_Session_Ind_Type_e			Ind_sts;//Indication status
	UDS_Session_Con_Type_e			Con_sts;//Confirm status
	UDS_Session_Req_Type_e			Req_sts;//Request status
	UDS_Session_Suppress_Type_e		Sup_sts;//Supress status
} UDS_A_Session_Time_Control_t;//Session time control; 

/*Indication类型*/
typedef enum {
	Init_Indication = 0,
	Indication = 1,						//Normal Indication
	FF_Indication = 2,					//FirstFrame Indication
	TP_Indication = 3,					//0x3E Service Indication
	Error_Indication					//Error
} IndicationType_e;

/*Response类型*/
typedef enum {
	Init_Response = 0,
	Normal_Response = 1,				//Response
	Error_Response = 2					//Error
} ResponseType_e;

/*Confirm类型*/
typedef enum {
	Init_Confirm = 0,
	Normal_Confirm = 1,					//Confirm
	NRC78_Confirm = 2,					//NRC=0x78 Response_Confirm
	Error_Confirm = 3					//Error
} ConfirmType_e;
	
/*Seesion类型*/
typedef enum {
	Default_Session = 0,				//Default session
	Non_Default_Session = 1,			//Non-default session
	Error_Session = 2		  			//Error
} SessionType_e;

/*正响应抑制类型*/
typedef enum {
	NotSuppressPositive = 0,
	SuppressPositive = 1,
	Error_SupPositive = 2
} SuppressPositiveType_e;

/*诊断服务进程*/
typedef enum {
	DiagProcessInit = 0,
	DiagProcessing = 1,
	Error_DiagProcess = 2
} DiagnosticProcStatus_e;

/*输入输出控制参数*/
typedef enum {
	returnControlToECU = 0,
	resetToDefault,
	freezeCurrentState,
	shortTermAdjustment
} InputOutputControlParameter_e;

typedef enum {
	WAIT_SEED_REQ = 0,
	WAIT_KEY = 1,
	WAIT_DELAY = 2,
	UNLOCKED = 3
} SecurityUnlockStep;

/*NRC-否定响应码*/
typedef enum {
	PR_00 = 0x00,					//positiveresponse
	GR_10 = 0X10,					//generalReject
	SNS_11 = 0X11,					//serviceNotSupported
	SFNS_12 = 0X12,					//sub-functionNotSupported 
	IMLOIF_13 = 0X13,				//incorrectMessageLengthOrInvalidFormat  
	RTL_14 = 0X14,					//responseTooLong
	BRR_21 = 0X21,					//busyRepeatRequest
	CNC_22 = 0X22,					//conditionsNotCorrect
	RSE_24 = 0X24,					//requestSequenceError
	ROOR_31 = 0X31,					//requestOutOfRange
	SAD_33 = 0X33,					//securityAccessDenied
	IK_35 = 0X35,					//invalidKey
	ENOA_36 = 0X36,					//exceedNumberOfAttempts
	RTDNE_37 = 0X37,				//requiredTimeDelayNotExpired
	UDNA_70 = 0X70,					//uploadDownloadNotAccepted
	TDS_71 = 0X71,					//transferDataSuspended
	GPF_72 = 0X72,					//generalProgrammingFailure
	WBSC_73 = 0X73,					//wrongBlockSequenceCounter
	RCRRP_78 = 0X78,				//requestCorrectlyReceived-ResponsePending
	SFNSIAS_7E = 0X7E,				//subfunctionNotSupportinActiveSession
	SNSIAS_7F = 0X7F,				//serviceNotSupportedInActiveSession
	VTH_92 = 0X92,					//voltageTooLow
	VTL_93 = 0X93					//voltageTooHigh
} NegativeResposeCode;

typedef enum {
	DIAGNOSTICSESSIONCONTROL = 0x10,
	ECURESET = 0x11,
	CLEARDIAGNOSTICINFORMATION = 0x14,
	READDTCINFORMATION = 0x19,
	READDATABYIDENTIFIER = 0x22,
	READMEMORYBYADDRESS = 0x23,
	SECURITYACCESS = 0x27,
	COMMUNICATIONCONTROL = 0x28,
	READDATABYPERIODICIDENTIFIER = 0x2A,
	DYNAMICALLYDEFINEDATAIDENTIFIER = 0x2C,
	WRITEDATABYIDENTIFIER = 0x2E,
	INPUTOUTPUTCONTROLBYIDENTIFIER = 0x2F,
	ROUTINECONTROL = 0x31,
	REQUESTDOWNLOAD = 0x34,
	REQUESTUPLOAD = 0x35,
	TRANSFERDATA = 0x36,
	REQUESTTRANSFEREXIT = 0x37,
	WRITEMEMORYBYADDRESS = 0x3D,
	TESTERPRESENT = 0x3E,
	CONTROLDTCSETTING = 0x85,
	LINKCONTROL = 0x87
} DiagnosticService_e;

/*诊断类型*/
typedef enum {
	Diagnostic = 0,						//诊断
	RemoteDiagnostic = 1				//远程诊断
} MessageType_e;

/*寻址类型*/
typedef enum {
	Physical = 0,						//物理寻址
	Functional = 1						//功能寻址
} TargetAddrType_e;

/*服务运行结果*/
typedef enum {
	OK = 0,
	NOK = 1
} Status_e;

/*诊断会话模式*/
typedef enum {
	DefaultSession = 0,
	ExtendSession = 1,
	ProgramingSession = 2,
	FactorySeesion = 3
} DiagnosticSession_e;

/*10h-DiagnosticSessionControl服务*/
typedef enum {
	DefaultSessionType = 1,
	ProgramingSessionType = 2,
	ExtendSessionType = 3,
	FactorySeesionType = 0x5F			//暂定
} DiagnosticSessionType_e;

/*安全等级定义-暂定4个安全等级*/
typedef enum {
	LEVEL_ZERO = 7,						//unlocked
	LEVEL_ONE = 1,						//1 level
	LEVEL_TWO = 2,						//2 level
	LEVEL_THREE = 4,					//3 level
	LEVEL_FOUR = 0x80,					//Factory mode
	LEVEL_UNSUPPORT = 0,				//Service not supported in active session
} SecurityLevel_e;
typedef enum {
	SEED01 = 0x01,
	KEY02 = 0x02,
	SEED03 = 0x03,
	KEY04 = 0x04
} SeedKey_e;

/*复位类型，参考ISO-14229-1中11服务复位类型的定义*/
typedef enum {
	HARD_RESET = 1,						//硬件复位
	KEY_OFF_ON_RESET = 2,				//关开钥匙复位
	SOFT_RESET = 3,						//软件复位
	ENABLE_RAPID_POWER_SHUTDOWN = 4,	//预留
	DISABLE_RAPID_POWER_SHUTDOWN = 5	//预留
} EcuResetType_e;

/*诊断测试结果*/
typedef enum {
	IN_TESTING = 0,
	PASSED = 1,	
	FAILED = 2 
} DTCTestResult_e;

typedef enum {
	PREPASSED = 0,
	PREFAILED = 1,
	NO_RESULT = 2
} DTCTestSampleResult_e;

/*通信控制参数*/
typedef enum {
	ERXTX = 0,							//enableRxAndTx
	ERXDTX = 1,							//enableRxAndDisableTx
	DRXETX = 2,							//disableRxAndEnableTx
	DRXTX = 3,							//disableRxAndTx
	ERXDTXWEAI = 4,						//enableRxAndDisableTxWithEnhancedAddressInformation
	ERXTXWEAI = 5						//enableRxAndTxWithEnhancedAddressInformation
} ControlType_e;
typedef enum {
	NCM = 1,							//normalCommunicationMessages
	NWMCM = 2,							//networkManagementCommunicationMessages 
	NWMCM_NCM = 3						//networkManagementCommunicationMessages and normalCommunicationMessages
} CommulicationType_e;

/*读取DTC信息子服务*/
typedef enum {
	reportNumberOfDTCByStatusMask = 1,
	reportDTCByStatusMask = 2,
	reportDTCSnapshotIdentification = 3,
	reportDTCSnapshotRecordByDTCNumber = 4,
	reportDTCStoredDataByRecordNumber = 5,
	reportDTCExtDataRecordByDTCNumber = 6,
	reportSupportedDTC = 0x0A
} RDTCISubfunc_e;

/*DTC类型定义*/
typedef enum {
	SAE_J2012_DA_DTCFormat_00 = 0,
	ISO_14229_1_DTCFormat = 1,
	SAE_J1939_73_DTCFormat = 2,
	ISO_11992_4_DTCFormat = 3,
	SAE_J2012_DA_DTCFormat_04 = 4
} DTCFormatIdentifier_e;

/*transmissionMode*/
typedef enum {
	SendAtSlowRate = 1,
	SendAtMediumRate = 2,
	SendAtFastRate = 3,
	StopSending = 4
} TransmissionMode_e;

/*DynamicallyDefineDataIdentifier*/
typedef enum {
	DefineByIdentifier = 1,
	DefineByMemoryAddress = 2,
	ClearDynamicallyDefinedDataIdentifier = 3
} DynamicallyDefineDataIdentifier_e;

typedef enum {
	NoDefine = 0,
	StaticDefine = 1,
	DynamicalDefine = 2
} DynamicalDataIdentifierStatus_e;

typedef enum {
	DTCSetting_on = 1,
	DTCSetting_off = 2
} DTCSettingType_e;

typedef enum {
	Data_VehicleSpeed = 0,
	Data_EngineRPM,
	Data_Voltage,
	Data_EngineRPM_IO,
	
	Data_Number
} Send_Data_e;

typedef enum {
	ReadOnly = 0,
	Write_RAM,
	Write_EEPROM,
	Write_RAM_IO
} DataIdentifierType_e;

typedef enum {
	VerifyBaudrateTransitionWithFixedBaudrate = 1,
	VerifyBaudrateTransitionWithSpecificBaudrate,
	TransitionBaudrate
} LinkControlType_e;

typedef enum {
	WAIT_MODE_REQ = 0,
	WAIT_TRANSITION,
	TRANSITION_OK
} LinkControlStep_e;

typedef enum {
	Baudrate_1M = 0,
	Baudrate_500K,
	Baudrate_250K,
	Baudrate_125K,
	Baudrate_57600,
	FixPara_Number
} TransitionWithFixedParameter_e;

typedef enum {
	StartRoutine = 1,
	StopRoutine = 2,
	RequestRoutineResults
} RoutineControlType_e;

typedef enum {
	Routine_Init = 0,
	Routine_start_successfully,
	Routine_start_unsuccessfully,
	Routine_inprogress,
	Routine_Completed_successfully,
	Routine_Completed_unsuccessfully 
} RoutineProcessStatus_e;

typedef enum {
	Transfer_Init = 0,
	Transfer_Wait_Download,
	Transfer_Wait_Upload,
	Transfer_Complete
} DownloadUploadStep_e;
	
typedef void(*ServiceHandler)(void);	
typedef uds_uint8_t (*DetectCondition)(void);
typedef RoutineProcessStatus_e (*RoutineRun)(RoutineControlType_e RoutineControlType);

typedef struct {
	DownloadUploadStep_e DownloadUploadStep;
	uds_uint8_t DataFormatIdentifier;
	uds_uint8_t DataBlockSequenceCounter;
	uds_uint32_t DataDownloadLength;
	uds_uint8_t DataUploadCounter;
	uds_uint32_t DataUploadTimes;
	uds_uint8_t* memoryAddress;
	uds_uint32_t memorySize;
} DataTransfer_t;

typedef struct {
	uds_uint16_t RoutineIdentifier;
	uds_uint8_t RestartRoutineSupport;
	uds_uint8_t StopRoutineSupport;
	uds_uint8_t RequestRoutineResultsSupport;
	uds_uint8_t RoutineControlOptionRecordDataLength;
	uds_uint8_t RoutineStatusRecordDataLength;
	RoutineProcessStatus_e RoutineStatus;
	RoutineRun RoutineRun_Func;
} Routine_t;

typedef struct {
	LinkControlStep_e LinkControlProcess;
	uds_uint32_t TransitionBaudrates;
} LinkStatus_t;

typedef struct {
	uds_uint8_t* DataPointer;
	uds_uint8_t DataLength;
} Initial_SendData_t;

typedef struct {
	uds_uint8_t DataArray[8];
	//uds_uint8_t DataLength;
} Switch_SendData_t;

typedef struct {
	uds_uint8_t Sub_func;					//子服务ID
	ServiceHandler SubServiceHandle;		//子服务执行函数
} Subfunction_t;

typedef struct {
	uds_uint8_t Support;
	DiagnosticService_e ServiceID;			//诊断服务ID
	uds_uint8_t MinDataLength;
	uds_uint8_t NumOfSubfunc;				//子服务数目
	uds_uint8_t PHYDefaultSession_Security;	//security suppport in default session physical address
	uds_uint8_t PHYProgramSeesion_Security;	//security suppport in program session physical address
	uds_uint8_t PHYExtendedSession_Security;//security suppport in extened session physical address
	uds_uint8_t FUNDefaultSession_Security;	//security suppport in default session function address
	uds_uint8_t FUNProgramSeesion_Security;	//security suppport in program session function address
	uds_uint8_t FUNExtendedSession_Security;//security suppport in extened session function address
	ServiceHandler ServiceHandle;
} DiagService;

typedef struct {
	uds_uint16_t DataIdentifier;				//数据ID
	uds_uint8_t DataBytesLength;				//数据字节长度
	uds_uint8_t *DataArray;						//DID数据指针
} DataByIdentifier;

/*DTC状态定义*/
typedef union {
	uds_uint8_t StatusOfDTC;
	struct {
		uds_uint8_t testFailed : 1;							//DTC is no longer failed at the time of the request
		uds_uint8_t testFailedThisOperationCycle : 1;		//DTC never failed on the current operation cycle
		uds_uint8_t pendingDTC : 1;							//DTC failed on the current or previous operation cycle
		uds_uint8_t confirmedDTC : 1;						//DTC is not confirmed at the time of the request
		uds_uint8_t testNotCompletedSinceLastClear : 1;		//DTC test were completed since the last code clear
		uds_uint8_t testFailedSinceLastClear : 1;			//DTC test failed at least once since last code clear
		uds_uint8_t testNotCompletedThisOperationCycle : 1;	//DTC test completed this operation cycle
		uds_uint8_t warningIndicatorRequested : 1;			//Server is not requesting warningIndicator to be active
	} StatusOfDTC_t;
} DTCStatus_u;

/*DTC故障检测条目*/
//typedef union {
//	uds_uint32_t DTCTableByte;
//	struct {
//		uds_uint32_t CANReceiveNodeLost_0x363 : 1;
//		uds_uint32_t CANReceiveNodeLost_0x3C3 : 1;
//		uds_uint32_t CANReceiveNodeLost_0x490 : 1;
//		uds_uint32_t CANReceiveNodeLost_0x581 : 1;
//		uds_uint32_t CANReceiveNodeLost_0x5A0 : 1;
//		uds_uint32_t CANReceiveNodeLost_0x64F : 1;
//		uds_uint32_t LEDTempSensor_GCCShort_GNDOpen : 1;
//		uds_uint32_t LEDTempSensor_GCCOpen_GNDShort : 1;
//		uds_uint32_t TFTTempSensor_GCCShort_GNDOpen : 1;
//		uds_uint32_t TFTTempSensor_GCCOpen_GNDShort : 1;
//		uds_uint32_t MainboardTempSensor_GCCShort_GNDOpen : 1;
//		uds_uint32_t MainboardTempSensor_GCCOpen_GNDShort : 1;
//		uds_uint32_t LightSensorMalfunction : 1;
//		uds_uint32_t SteppingMotorMalfunction : 1;
//	} DTCTable_t;
//} DTCTable_u;

typedef uds_uint8_t (*DTC_test_func)(void);

/*DTC节点*/
typedef struct {
	uds_uint32_t DTCNumber;
	DTC_test_func Test_func;
	DetectCondition EnableDTCDetect;
	uds_uint8_t DetecTimes;
	uds_int8_t PrefailedStep;
	uds_int8_t PrepassedStep;
	uds_int8_t FailedFaultDetectionTimes; //确保(FailedFaultDetectionTimes + PrefailedStep) <= max(uds_int8_t)
	uds_int8_t PassedFaultDetectionTimes; //确保(PassedFaultDetectionTimes - PrepassedStep) >= min(uds_int8_t)
	uds_uint8_t Confirmationthreshold;
	uds_uint8_t MaxDTCAgingTimes;
	uds_uint8_t DTCStatusTag;
	uds_uint8_t SnapShotTag;
} DTCNode_Config;
typedef struct {
	uds_uint8_t DetecCycleCounter;
	uds_int8_t FaultDetectionCounter;
	uds_uint8_t ConfirmationCounter;
	DTCStatus_u DTCstatus;
	uds_uint8_t FaultOccurenceCounter;
	uds_uint8_t FaultPendingCounter;
	uds_uint8_t FaultAgedCounter;
	uds_uint8_t FaultAgingCounter;
} DTCNode_Sts;


typedef struct {
	DTCNode_Config	Config;
	DTCNode_Sts		Sts;
	//uds_int8_t SupportedSnapShotNumber;
	//uds_int8_t SnapShotCounter;
} DTCNode;

/*DTCGroup*/
typedef struct {
	uds_uint32_t GroupID;
}DTCGroup_t;

/*DTCSnapshot-暂定*/
typedef struct {
	uds_uint8_t SnapshotRecordNumber;
	uds_uint16_t SnapshotID;
	uds_uint8_t* SnapshotData;
	uds_uint8_t DataLength;
	uds_uint8_t StartNum;
}DTCSnapshot_t;

/*可读DID-暂定*/
typedef struct {
	uds_uint16_t DataIdentifier;
	uds_uint8_t* DataPointer;
	uds_uint8_t DataLength;
	uds_uint8_t Support;				//支持PDID
	uds_uint8_t TransmitTime;			//PDID设定周期
	uds_uint8_t TransmitCounter;		//PDID周期计数器
	uds_uint8_t DynamicalIdentifierBit;	//动态DID标志位
	DataIdentifierType_e DataIdentifierType; //DID类型
	uds_uint8_t DataIdentifierTag;
	uds_uint8_t FactoryConfigSupport;	//下线配置支持
}ReadDatabyInentifier_t;

typedef struct {
	uds_uint8_t DataIndex[200];
	uds_uint8_t FirstofDataIndex;
	uds_uint16_t NumofData;
} PeriodicDID_t;

typedef struct {
	SecurityUnlockStep UnlockStep;
	uds_uint32_t Key; //秘钥
	uds_uint8_t KeyVerifyErrorCount; //密钥验证失败计数器需保存至EEPROM，用于每次上电或ECU复位时读取
	uds_uint8_t KeyVerifyErrorCountFromEEPROM;
	uds_uint8_t AccessErrorCountTag;
} SecurityAccess_t;

typedef struct {
	uds_uint16_t SourceDataIdentifier;
	uds_uint8_t PositionInSourceDataRecord;
	uds_uint8_t* DataPointer;
	uds_uint8_t MemorySize;
} SourceDataRecord_t;

/*请求数据单元-针对客户端是响应数据单元*/
typedef struct {
	MessageType_e Mtype;				//诊断类型
	uds_uint8_t SourceAddr;				//源地址
	uds_uint8_t TargetAddr;				//目标地址
	TargetAddrType_e TAType;			//目标地址类型
#ifdef ADDRESS_EXTENSION_MODE	
	uds_uint8_t A_ExtendAddr;			//扩展地址
#endif
	uds_uint8_t *MessageData;         	//数据
	uds_uint16_t Length;				//数据长度
} UDS_A_Request_t;

/*确认数据单元*/
typedef struct {
	MessageType_e Mtype;				//诊断类型          
	uds_uint8_t SourceAddr;          	//源地址           
	uds_uint8_t TargetAddr;           	//目标地址          
	TargetAddrType_e TAType;            //目标地址类型 
#ifdef ADDRESS_EXTENSION_MODE	
	uds_uint8_t A_ExtendAddr;			//扩展地址
#endif       
	Status_e Result;                    //数据发送结果           
} UDS_A_Confirm_t;

/*显示数据单元*/
typedef struct {
	MessageType_e Mtype;				//诊断类型          
	uds_uint8_t SourceAddr;           	//源地址           
	uds_uint8_t TargetAddr;           	//目标地址          
	TargetAddrType_e TAType;            //目标地址类型
#ifdef ADDRESS_EXTENSION_MODE	
	uds_uint8_t A_ExtendAddr;			//扩展地址
#endif        
	uds_uint8_t A_PCI;         			//协议控制信息
	uds_uint8_t A_SDU[MAXDATALENGTH];	//数据
	uds_uint16_t Length;                //数据长度          
	Status_e Result;					//数据接收结果
} UDS_A_Indication_t;

/*应用层缓存数据单元*/
typedef struct {
	MessageType_e Mtype;				//诊断类型          
	uds_uint8_t SourceAddr;          	//源地址           
	uds_uint8_t TargetAddr;           	//目标地址          
	TargetAddrType_e TAType;            //目标地址类型
#ifdef ADDRESS_EXTENSION_MODE	
	uds_uint8_t A_ExtendAddr;			//扩展地址
#endif        
	uds_uint8_t *MessageData;         	//数据
	uds_uint16_t Length;                //数据长度          
	Status_e Result;
} ServiceDataUnit_t;
/*********************************************************************************************************************
 * EEPROM types definitions
 *********************************************************************************************************************/
#define EEPROM_BASE_ADDRESS 0//eeprom base address

typedef enum {
	EE_TEST = 0, 						//test
	SECURITYACCESS_ERROR_COUNTER_TAG,
	DTC_CAN_RECEIVENODELOST_0x363_TAG,
	DTC_CAN_RECEIVENODELOST_0x3C3_TAG,
	DTC_CAN_RECEIVENODELOST_0x490_TAG,
	DTC_CAN_RECEIVENODELOST_0x581_TAG,
	DTC_CAN_RECEIVENODELOST_0x5A0_TAG,
	DTC_CAN_RECEIVENODELOST_0x64F_TAG,
	DTC_LED_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG,
	DTC_LED_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG,
	DTC_TFT_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG,
	DTC_TFT_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG,
	DTC_MAINBOARD_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG,
	DTC_MAINBOARD_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG,
	DTC_LIRHTSENSOR_MALFUNCTION_TAG,
	DTC_STEPPINGMOTOR_MALFUNCTION_TAG,
	
	DTCSNAPSHOT_CAN_0x363_TAG,
	DTCSNAPSHOT_CAN_0x3C3_TAG,
	DTCSNAPSHOT_CAN_0x490_TAG,
	DTCSNAPSHOT_CAN_0x581_TAG,
	DTCSNAPSHOT_CAN_0x5A0_TAG,
	DTCSNAPSHOT_CAN_0x64F_TAG,
	DTCSNAPSHOT_LED_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG,
	DTCSNAPSHOT_LED_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG,
	DTCSNAPSHOT_TFT_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG,
	DTCSNAPSHOT_TFT_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG,
	DTCSNAPSHOT_MAINBOARD_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG,
	DTCSNAPSHOT_MAINBOARD_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG,
	DTCSNAPSHOT_LIRHTSENSOR_MALFUNCTION_TAG,
	DTCSNAPSHOT_STEPPINGMOTOR_MALFUNCTION_TAG,

	VIN_NUMNER_TAG,
	
	EE_TAG_ALL
} UDS_EE_Tag_e;//

typedef struct
{
	UDS_EE_Tag_e	Tag;
	uds_uint8_t		Length;
} UDS_EE_Info_t;//UDS network interface 

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
	uds_int8_t  (*ParaChange_request)(void *pdata);
	uds_int8_t  (*can_direct_send)(uds_uint32_t id,
								   uds_uint8_t length,
								   uds_uint8_t *data);
} UDS_Network_Interface_t;//UDS network interface 

typedef struct
{
	void 		 (*time_manage_handle)(void);
	void		 (*main_proc)(void);
	uds_int8_t	 (*init)(void);
	uds_uint8_t  (*service_get)(UDS_N_Services_t *res,
								uds_uint8_t *buf);
	uds_int8_t   (*USData_request)(void *pdata);
	uds_int8_t   (*ParaChange_request)(void *pdata);
	uds_int8_t   (*time_ctl_stop)(UDS_S_Time_Name_e time);
	uds_int8_t   (*time_ctl_run)(UDS_S_Time_Name_e time);
	uds_int8_t   (*time_ctl_restart)(UDS_S_Time_Name_e time);
	uds_int8_t   (*time_ctl_reset)(UDS_S_Time_Name_e time);
	uds_int8_t   (*time_status_get)(UDS_S_Time_Name_e time);
	uds_uint16_t (*time_value_get)(UDS_S_Time_Name_e time);
} UDS_Session_Interface_t;//UDS session interface 


typedef struct
{
	void		(*main_proc)(void);
	uds_int8_t	(*init)(void);
	uds_int8_t	(*DTC_add_iterm)(DTCNode_Config *dtc_iterm);
} UDS_App_Interface_t;//UDS app interface 

typedef struct
{
	uds_int8_t	(*init)(void);
	uds_int8_t	(*read)(UDS_EE_Tag_e tag,uds_uint8_t *buf);
	uds_int8_t	(*write)(UDS_EE_Tag_e tag,uds_uint8_t *buf);
} UDS_EEPROM_Interface_t;//UDS eeprom interface 


typedef struct
{
	UDS_Network_Interface_t	Network;	//Network
	UDS_Session_Interface_t	Session;	//Session
	UDS_App_Interface_t		Application;//Application
	UDS_EEPROM_Interface_t	Eeprom;		//EEPROM
} UDS_Interface_In_t;//UDS interface 

typedef struct
{
	uds_uint8_t (*can_send_hook)(uds_uint32_t id,
								 uds_uint8_t length,
								 uds_uint8_t *data);
	uds_int8_t  (*eeprom_read)(uds_uint32_t offset,
							   uds_uint16_t length,
							   uds_uint8_t *data);
	uds_int8_t  (*eeprom_write)(uds_uint32_t offset,
								uds_uint16_t length,
								uds_uint8_t *data);

} UDS_Interface_Ext_t;//UDS interface

unsigned char m_can_send_hook(unsigned long id,
							  unsigned char length,
							  unsigned char *data);

uds_int8_t UDS_A_set_session_time_ctl_Ses_sts(UDS_Session_Status_e sts);
uds_int8_t UDS_A_set_session_time_ctl_req(UDS_Session_Req_Type_e sts);
uds_int8_t UDS_A_set_session_time_ctl_con(UDS_Session_Con_Type_e sts);
uds_int8_t UDS_A_set_session_time_ctl_ind(UDS_Session_Ind_Type_e sts);
uds_int8_t UDS_A_set_session_time_ctl_sup(UDS_Session_Suppress_Type_e sts);


#endif

