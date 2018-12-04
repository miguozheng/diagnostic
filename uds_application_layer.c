#include "uds_common_include.h"
#include "uds_struct.h"


/*************************1*****************************************************
Private global variables and functions
******************************************************************************/
typedef void(*ServiceHandler)();


//session control
static UDS_A_Session_Time_Control_t Session_ctl;
uds_uint8_t SDUReceiveBuffArray[MAXDATALENGTH] = { 0 };
uds_uint8_t SDUSendBuffArray[MAXDATALENGTH] = { 0 };

/*������*/
static SecurityUnlockStep UnlockStep = WAIT_SEED_REQ;
static NegativeResposeCode NRC;
static uds_uint16_t ResponseDataLength;
static uds_uint8_t ServiceSubFunc;
uds_uint8_t SuppressPosRspMsgIndicationBit;
static uds_uint8_t ECUResetType;
uds_uint8_t WaitConfimBeforeReset = FALSE;
uds_uint8_t P2_ServerRequestFlag;

/*DTC*/
uds_uint8_t DTCBytesTable[DTCBYTELENGTH]; //�洢DTC��Ϣ-�ݶ�����Ϊ10��8Bits����
uds_uint8_t DTCStatusAvailabilityMask; //provides an indication of DTC status bits that are supported by the server for masking purposes
uds_uint32_t DTCCountByte; //DTC�ֽ���-����

/*��ȫ����*/
uds_uint32_t Key = 0; //��Կ
uds_uint8_t KeyVerifyErrorCount; //��Կ��֤ʧ�ܼ������豣����EEPROM������ÿ���ϵ��ECU��λʱ��ȡ
uds_uint8_t KeyVerifyErrorCountFromEEPROM;

/*ϵͳ����*/
struct {
	DiagnosticSession_e SessionMode;
	SessionType_e SessionType;
	uds_uint8_t SecurityLevel;
	uds_uint8_t DiagnosticProcessStatus;
	IndicationType_e IndicationType;
	ConfirmType_e ConfirmType;
	ResponseType_e ResponseType;
	SuppressPositiveType_e SuppressPositiveType;
} SystemState = { DefaultSession, 0, LEVEL_ZERO, 0, 0, 0, 0, 0 };

typedef struct {
	uds_uint8_t Sub_func;						//�ӷ���ID
	ServiceHandler SubServiceHandle;		//�ӷ���ִ�к���
} Subfunction_t;

typedef struct {
	uds_uint8_t Support;
	DiagnosticService_e ServiceID;			//��Ϸ���ID
	uds_uint8_t NumOfSubfunc;					//�ӷ�����Ŀ
	uds_uint8_t PHYDefaultSession_Security: 4;	//security suppport in default session physical address
	uds_uint8_t PHYProgramSeesion_Security: 4;	//security suppport in program session physical address
	uds_uint8_t PHYExtendedSession_Security: 4;//security suppport in extened session physical address
	uds_uint8_t FUNDefaultSession_Security: 4;	//security suppport in default session function address
	uds_uint8_t FUNProgramSeesion_Security: 4;	//security suppport in program session function address
	uds_uint8_t FUNExtendedSession_Security: 4;//security suppport in extened session function address
	ServiceHandler ServiceHandle;
} DiagService;

typedef struct {
	uds_uint16_t DataIdentifier;				//����ID
	uds_uint8_t DataBytesLength;				//�����ֽڳ���
	uds_uint8_t *DataArray;						//DID����ָ��
} DataByIdentifier;

const uds_uint8_t DataArrayF192[] = { 0x11, 0x22, 0x33, 0x44 }; //�����
struct {
	uds_uint8_t xxxx;
	uds_uint8_t yyyy;
	uds_uint8_t zzzz;
	uds_uint8_t tttt;
} DataArrayF193; //�����

/*�ɶ�DID����-�����*/
DataByIdentifier ArrayOfDID[] = {
	{ 0XF192, 4, (uds_uint8_t*)DataArrayF192},
	{ 0XF193, 4, (uds_uint8_t*)&DataArrayF193}
};

UDS_A_Indication_t UDS_A_Indication_SDU;
UDS_A_Confirm_t UDS_A_Confirm_SDU;
ServiceDataUnit_t ServiceDataUnit;
UDS_A_Request_t UDS_A_Request_SDU;

/*
============================================================================
 Function declear
============================================================================
*/

//App get the lower layer service
static uds_uint8_t (*UDS_A_service_get)(UDS_N_Services_t *res,uds_uint8_t *buf);
//App layer issue a USData.request service
static uds_int8_t (*UDS_A_service_process_USData_request)(void *pdata);
//App get the session layer timer status
static uds_int8_t (*UDS_A_timer_status_get)(UDS_S_Time_Name_e time);
static uds_int8_t (*UDS_A_timer_ctl_stop)(UDS_S_Time_Name_e time);
static uds_int8_t (*UDS_A_timer_ctl_run)(UDS_S_Time_Name_e time);
static uds_int8_t (*UDS_A_timer_ctl_restart)(UDS_S_Time_Name_e time);
static uds_int8_t (*UDS_A_timer_ctl_reset)(UDS_S_Time_Name_e time);

static uds_int8_t UDS_A_interface_regist(UDS_Interface_In_t *intf);
static UDS_A_Session_Time_Control_t *UDS_A_get_session_time_ctl(void);
static uds_int8_t UDS_A_set_session_time_ctl_sts(UDS_Session_TimeCtl_Status_e sts);
static UDS_Session_TimeCtl_Status_e UDS_A_get_session_time_ctl_sts(void);
//uds_int8_t UDS_A_set_session_time_ctl_Ses_sts(UDS_Session_Status_e sts);
static UDS_Session_Status_e UDS_A_get_session_time_ctl_Ses_sts(void);
//uds_int8_t UDS_A_set_session_time_ctl_req(UDS_Session_Req_Type_e sts);
static UDS_Session_Req_Type_e UDS_A_get_and_clear_session_time_ctl_req(void);
//uds_int8_t UDS_A_set_session_time_ctl_con(UDS_Session_Con_Type_e sts);
static UDS_Session_Con_Type_e UDS_A_get_and_clear_session_time_ctl_con(void);
// uds_int8_t UDS_A_set_session_time_ctl_ind(UDS_Session_Ind_Type_e sts);
static UDS_Session_Ind_Type_e UDS_A_get_and_clear_session_time_ctl_ind(void);
// uds_int8_t UDS_A_set_session_time_ctl_sup(UDS_Session_Suppress_Type_e sts);
 static UDS_Session_Suppress_Type_e UDS_A_get_and_clear_session_time_ctl_sup(void);
static void UDS_A_seesion_prc_idle(void);
static void UDS_A_seesion_prc_P2_run(void);
static void UDS_A_seesion_prc_P2_stop(void);
static void UDS_A_seesion_prc_P2x_run(void);
static void UDS_A_seesion_prc_P2x_stop(void);
static void UDS_A_seesion_prc_S3_run(void);
static void UDS_A_seesion_prc_S3_stop(void);
static void UDS_A_seesion_time_process(void);
static void UDS_A_reset_seesion_time_ctl(void);



static void Service10Handle(void);
static void DefaultSession_Proc(void);
static void ExtendSession_Proc(void);
static void ProgramingSession_Proc(void);

static void Service11Handle(void);
static void HardReset_Proc(void);
static void KeyOnOffReset_Proc(void);
static void SoftReset_Proc(void);

static void Service14Handle(void);

static void Service19Handle(void);
static void ReportDTCStatusMaskNum01_Proc(void);
static void ReportDTCStatusMask02_Proc(void);
static void ReportDTCSnapshotRecord04_Proc(void);
static void ReportSupportedDTC0A_Proc(void);

static void Service22Handle(void);

static void Service23Handle(void);

static void Service27Handle(void);
static void SecurityAccessSeed01_Proc(void);
static void SecurityAccessKey02_Proc(void);
static void SecurityAccessSeed03_Proc(void);
static void SecurityAccessKey04_Proc(void);

static void Service3EHandle(void);
static void TesterPresent00_Proc(void);
static void TesterPresent80_Proc(void);

static void InitSetSessionSupportAndSecurityAccess(uds_uint8_t support, uds_uint8_t service, uds_uint8_t PHYDefaultSession_Security, uds_uint8_t PHYProgramSeesion_Security, uds_uint8_t PHYExtendedSession_Security, uds_uint8_t FUNDefaultSession_Security, uds_uint8_t FUNProgramSeesion_Security, uds_uint8_t FUNExtendedSession_Security);
static void UDS_A_ParameterInit(void);
static void UDS_A_Data_Indication_Read(void);
static void UDS_A_Data_Confirm_Read(void);
static void UDS_A_DiagnosticService(uds_uint8_t DiagServiceID);
static void UDS_A_Data_Confirm_Prco(void);
static void UDS_A_PositiveResponseDataUnit(uds_uint16_t DataLength);
static void UDS_A_NegativeResponseDataUnit(uds_uint8_t nrc);
static void UDS_A_Response_Proc(NegativeResposeCode NRC_Para, uds_uint16_t DataLength, uds_uint8_t SuppPosiFlag, uds_uint8_t TAtype);

static void FlagSet_Func(uds_uint8_t *Flag);
static void FlagClear_Func(uds_uint8_t *Flag);
static void Reset_Func(EcuResetType_e ResetType);
static uds_uint32_t SeedCalc01_Func(uds_uint8_t RequestSeed);
static uds_uint32_t KeyCalc01_Func(uds_uint32_t RequestSeed);
static uds_uint32_t SeedCalc03_Func(uds_uint8_t RequestSeed);
static uds_uint32_t KeyCalc03_Func(uds_uint32_t RequestSeed);
static uds_uint32_t *ReadMemoryByAddress_Func(uds_uint8_t *Address, uds_uint16_t Length);
static uds_uint32_t ReadMemorySize_Func(uds_uint8_t *Address, uds_uint16_t Length);

DiagService DiagnosticServiceTable[NUMBEROFSERVICE] = {
	{ FALSE,	DIAGNOSTICSESSIONCONTROL, 		3, LEVEL_ZERO,		LEVEL_ZERO,		 LEVEL_ZERO,	  LEVEL_ZERO,		 LEVEL_ZERO,		LEVEL_ZERO,		 Service10Handle },
	{ FALSE,	ECURESET, 						3, LEVEL_UNSUPPORT,	LEVEL_ZERO,		 LEVEL_ZERO,	  LEVEL_UNSUPPORT,	 LEVEL_ZERO,		LEVEL_ZERO,		 Service11Handle },
	{ FALSE,	CLEARDIAGNOSTICINFORMATION,		0, LEVEL_ZERO,		LEVEL_UNSUPPORT, LEVEL_ZERO, 	  LEVEL_ZERO,		 LEVEL_UNSUPPORT,	LEVEL_ZERO,		 Service14Handle },
	{ FALSE,	READDTCINFORMATION, 			5, LEVEL_ZERO,		LEVEL_UNSUPPORT, LEVEL_ZERO,	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service19Handle },
	{ FALSE,	READDATABYIDENTIFIER, 			0, LEVEL_ZERO,		LEVEL_UNSUPPORT, LEVEL_ZERO,	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service22Handle },
	{ FALSE,	READMEMORYBYADDRESS, 			0, LEVEL_UNSUPPORT,	LEVEL_ZERO, 	 LEVEL_ZERO,	  LEVEL_UNSUPPORT,	 LEVEL_ZERO,		LEVEL_ZERO, 	 Service23Handle },
	{ FALSE,	SECURITYACCESS, 				4, LEVEL_UNSUPPORT,	LEVEL_ZERO,		 LEVEL_ZERO,	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service27Handle },
	{ FALSE,	TESTERPRESENT, 					2, LEVEL_ZERO,		LEVEL_ZERO,		 LEVEL_ZERO,	  LEVEL_ZERO,		 LEVEL_ZERO,		LEVEL_ZERO,		 Service3EHandle }
};
Subfunction_t Service10Table[] = {
	{ DefaultSessionType, DefaultSession_Proc },
	{ ExtendSessionType, ExtendSession_Proc },
	{ ProgramingSessionType, ProgramingSession_Proc }
};

Subfunction_t Service11Table[] = {
	{ HARD_RESET, HardReset_Proc },
	{ KEY_OFF_ON_RESET, KeyOnOffReset_Proc },
	{ SOFT_RESET, SoftReset_Proc }
};

Subfunction_t Service19Table[] = {
	{ reportNumberOfDTCByStatusMask, ReportDTCStatusMaskNum01_Proc },
	{ reportDTCByStatusMask, ReportDTCStatusMask02_Proc },
	{ repDTCSnapshotRecordByDTCNum, ReportDTCSnapshotRecord04_Proc },
	{ reportSupportedDTC, ReportSupportedDTC0A_Proc }
};
	
Subfunction_t Service27Table[] = {
	{ SEED01, SecurityAccessSeed01_Proc },
	{ KEY02, SecurityAccessKey02_Proc },
	{ SEED03, SecurityAccessSeed03_Proc },
	{ KEY04, SecurityAccessKey04_Proc }
};

Subfunction_t Service3ETable[] = {
	{ 0x00, TesterPresent00_Proc },
	{ 0x80, TesterPresent80_Proc }
};

/*
============================================================================
 User function
============================================================================
*/
static uds_int8_t UDS_A_platform_regist(void)
{

}

static uds_int8_t UDS_A_interface_regist(UDS_Interface_In_t *intf)
{
	uds_int8_t ret = -1;

	if(!intf){
		return ret;
	}

	ret = 1;
	if(intf->Session.service_get){
		UDS_A_service_get = intf->Session.service_get;
	}else{
		ret |= -1;
	}
	if(intf->Session.USData_request){
		UDS_A_service_process_USData_request = intf->Session.USData_request;
	}else{
		ret |= -1;
	}
	if(intf->Session.time_status_get){
		UDS_A_timer_status_get = intf->Session.time_status_get;
	}else{
		ret |= -1;
	}
	if(intf->Session.time_ctl_stop){
		UDS_A_timer_ctl_stop = intf->Session.time_ctl_stop;
	}else{
		ret |= -1;
	}
	if(intf->Session.time_ctl_run){
		UDS_A_timer_ctl_run = intf->Session.time_ctl_run;
	}else{
		ret |= -1;
	}
	if(intf->Session.time_ctl_restart){
		UDS_A_timer_ctl_restart = intf->Session.time_ctl_restart;
	}else{
		ret |= -1;
	}
	if(intf->Session.time_ctl_reset){
		UDS_A_timer_ctl_reset = intf->Session.time_ctl_reset;
	}else{
		ret |= -1;
	}

	return ret;
}


static UDS_A_Session_Time_Control_t *UDS_A_get_session_time_ctl(void)
{
	return &Session_ctl;
}

static uds_int8_t UDS_A_set_session_time_ctl_sts(UDS_Session_TimeCtl_Status_e sts)
{
	uds_int8_t ret = -1;
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();

	if(A_SES_TIME_CTL_STS_ERROR > sts){
		pctl->Status = sts;
		ret = 1;
	}

	return ret;
}

static UDS_Session_TimeCtl_Status_e UDS_A_get_session_time_ctl_sts(void)
{
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();

	return pctl->Status;
}

uds_int8_t UDS_A_set_session_time_ctl_Ses_sts(UDS_Session_Status_e sts)
{
	uds_int8_t ret = -1;
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();

	if(A_SES_STS_ERROR > sts){
		pctl->Ses_sts = sts;
		ret = 1;
	}

	return ret;
}

static UDS_Session_Status_e UDS_A_get_session_time_ctl_Ses_sts(void)
{
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();

	return pctl->Ses_sts;
}


 uds_int8_t UDS_A_set_session_time_ctl_req(UDS_Session_Req_Type_e sts)
{
	uds_int8_t ret = -1;
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();

	if(A_SES_REQ_ERROR > sts){
		pctl->Req_sts = sts;
		ret = 1;
	}

	return ret;
}

static UDS_Session_Req_Type_e UDS_A_get_and_clear_session_time_ctl_req(void)
{
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();
	UDS_Session_Req_Type_e ret = pctl->Req_sts;

	pctl->Req_sts = A_SES_REQ_NONE;

	return ret;
}

 uds_int8_t UDS_A_set_session_time_ctl_con(UDS_Session_Con_Type_e sts)
{
	uds_int8_t ret = -1;
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();

	if(A_SES_CON_ERROR > sts){
		pctl->Con_sts = sts;
		ret = 1;
	}

	return ret;
}

static UDS_Session_Con_Type_e UDS_A_get_and_clear_session_time_ctl_con(void)
{
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();
	UDS_Session_Con_Type_e ret = pctl->Con_sts;

	pctl->Req_sts = A_SES_CON_NONE;

	return ret;
}

 uds_int8_t UDS_A_set_session_time_ctl_ind(UDS_Session_Ind_Type_e sts)
{
	uds_int8_t ret = -1;
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();

	if(A_SES_IND_ERROR > sts){
		pctl->Ind_sts = sts;
		ret = 1;
	}

	return ret;
}

static UDS_Session_Ind_Type_e UDS_A_get_and_clear_session_time_ctl_ind(void)
{
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();
	UDS_Session_Ind_Type_e ret = pctl->Ind_sts;

	pctl->Ind_sts = A_SES_IND_NONE;

	return ret;
}

 uds_int8_t UDS_A_set_session_time_ctl_sup(UDS_Session_Suppress_Type_e sts)
{
	uds_int8_t ret = -1;
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();

	if(A_SES_SUP_ERROR > sts){
		pctl->Sup_sts = sts;
		ret = 1;
	}

	return ret;
}

static UDS_Session_Suppress_Type_e UDS_A_get_and_clear_session_time_ctl_sup(void)
{
	UDS_A_Session_Time_Control_t *pctl = UDS_A_get_session_time_ctl();
	UDS_Session_Suppress_Type_e ret = pctl->Sup_sts;

	pctl->Sup_sts = A_SES_SUP_NONE;

	return ret;
}

static void UDS_A_seesion_prc_idle(void)
{
	uds_uint8_t ind = UDS_A_get_and_clear_session_time_ctl_ind();

	if(A_SES_IND_NORMAL == ind){
		UDS_A_timer_ctl_restart(S_TIME_P2_CAN_SERVER);
		UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_P2RUN);
		UDS_PRINTF("UDS App: IDLE----Got a normal IND ,start P2.\n\r");
	}
}

static void UDS_A_seesion_prc_P2_run(void)
{
	uds_uint8_t req = UDS_A_get_and_clear_session_time_ctl_req();
	uds_uint8_t sup = UDS_A_get_and_clear_session_time_ctl_sup();
	uds_uint8_t ses = UDS_A_get_session_time_ctl_Ses_sts();

	if(A_SES_SUP_POSITIVE == sup){//suppress positive response
		UDS_A_timer_ctl_reset(S_TIME_P2_CAN_SERVER);
		if(A_SES_STS_NON_DEFAULT == ses){//Non-Default session
			UDS_A_timer_ctl_restart(S_TIME_S3_SERVER);
			UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_S3RUN);
			UDS_PRINTF("UDS App: P2 RUN----Suppress positive response ,reset P2 and restart S3 in non-default mode.\n\r");
		}else{
			UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_IDLE);			
			UDS_PRINTF("UDS App: P2 RUN----Suppress positive response ,reset P2 in default mode.\n\r");
		}
	}else{
		if(A_SES_REQ_NORMAL == req){
			UDS_A_timer_ctl_reset(S_TIME_P2_CAN_SERVER);
			UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_P2STOP);			
			UDS_PRINTF("UDS App: P2 RUN----Got a normal REQ ,reset P2 .\n\r");
		}
	}
}

static void UDS_A_seesion_prc_P2_stop(void)
{
	uds_uint8_t con = UDS_A_get_and_clear_session_time_ctl_con();
	uds_uint8_t ses = UDS_A_get_session_time_ctl_Ses_sts();

	switch(con){
		case A_SES_CON_NORMAL:
			UDS_A_timer_ctl_reset(S_TIME_P2_CAN_SERVER);
			if(A_SES_STS_NON_DEFAULT == ses){//Non-Default session
				UDS_A_timer_ctl_restart(S_TIME_S3_SERVER);
				UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_S3RUN);
				UDS_PRINTF("UDS App: P2/P2* STOP----Got normal CON ,reset P2 and restart S3 in non-default mode.\n\r");
			}else{
				UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_IDLE);
				
				UDS_PRINTF("UDS App: P2/P2* STOP----Got normal CON ,reset P2 in default mode.\n\r");
			}
			break;
		case A_SES_CON_NRC:
			UDS_A_timer_ctl_restart(S_TIME_P2X_CAN_SERVER);
			UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_P2xRUN);
			UDS_PRINTF("UDS App: P2/P2* STOP----Got NRC CON ,reset P2 and restart P2* in default mode.\n\r");
			break;
		default:break;
	}
}

static void UDS_A_seesion_prc_P2x_run(void)
{
	uds_uint8_t req = UDS_A_get_and_clear_session_time_ctl_req();

	if(A_SES_REQ_NORMAL == req){
		UDS_A_timer_ctl_reset(S_TIME_P2X_CAN_SERVER);
		UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_P2xSTOP);
		UDS_PRINTF("UDS App: P2* RUN----Got normal REQ ,reset P2* in default mode.\n\r");
	}
}

static void UDS_A_seesion_prc_P2x_stop(void)
{
	UDS_A_seesion_prc_P2_stop();
}

static void UDS_A_seesion_prc_S3_run(void)
{
	uds_uint8_t ind = UDS_A_get_and_clear_session_time_ctl_ind();
	uds_uint8_t ses = UDS_A_get_session_time_ctl_Ses_sts();

	if(A_SES_STS_NON_DEFAULT == ses){
		switch(ind){
			case A_SES_IND_NORMAL:
				UDS_A_timer_ctl_restart(S_TIME_P2_CAN_SERVER);
				UDS_A_timer_ctl_reset(S_TIME_S3_SERVER);
				UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_P2RUN);
				UDS_PRINTF("UDS App: S3 RUN----Got normal IND ,reset S3 and restart P2 in non-default mode.\n\r");
				break;
			case A_SES_IND_FF:
				UDS_A_timer_ctl_reset(S_TIME_S3_SERVER);
				UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_S3STOP);
			
				UDS_PRINTF("UDS App: S3 RUN----Got FF IND ,reset S3 in non-default mode.\n\r");
				break;
			case A_SES_IND_TP:
				UDS_A_timer_ctl_restart(S_TIME_S3_SERVER);				
				UDS_PRINTF("UDS App: S3 RUN----Got TP IND ,resart S3 in non-default mode.\n\r");
				break;
			default:break;
		}
	}else{
		UDS_A_timer_ctl_reset(S_TIME_S3_SERVER);
		UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_IDLE);
		
		UDS_PRINTF("UDS App: S3 RUN----Reset S3 in default mode.\n\r");
	}
}

static void UDS_A_seesion_prc_S3_stop(void)
{
	uds_uint8_t ind = UDS_A_get_and_clear_session_time_ctl_ind();
	uds_uint8_t ses = UDS_A_get_session_time_ctl_Ses_sts();

	if(A_SES_STS_NON_DEFAULT == ses){
		switch(ind){
			case A_SES_IND_NORMAL:
				UDS_A_timer_ctl_restart(S_TIME_P2_CAN_SERVER);
				UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_P2RUN);
			
				UDS_PRINTF("UDS App: S3 STOP----Got normal IND ,reset S3 and restart P2 in non-default mode.\n\r");
				break;
			case A_SES_IND_FF:
				break;
			case A_SES_IND_TP:
				break;
			default:break;
		}
	}else{
		UDS_A_timer_ctl_reset(S_TIME_S3_SERVER);
		UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_IDLE);
		
		UDS_PRINTF("UDS App: S3 STOP----Reset S3 in default mode.\n\r");
	}
}

static void UDS_A_seesion_time_process(void)
{
	uds_int8_t sts = UDS_A_get_session_time_ctl_sts();

	switch(sts){
		case A_SES_TIME_CTL_STS_IDLE:
			UDS_A_seesion_prc_idle();
			break;
		case A_SES_TIME_CTL_STS_P2RUN:
			UDS_A_seesion_prc_P2_run();
			break;
		case A_SES_TIME_CTL_STS_P2STOP:
			UDS_A_seesion_prc_P2_stop();
			break;
		case A_SES_TIME_CTL_STS_P2xRUN:
			UDS_A_seesion_prc_P2x_run();
			break;
		case A_SES_TIME_CTL_STS_P2xSTOP:
			UDS_A_seesion_prc_P2x_stop();
			break;
		case A_SES_TIME_CTL_STS_S3RUN:
			UDS_A_seesion_prc_S3_run();
			break;
		case A_SES_TIME_CTL_STS_S3STOP:
			UDS_A_seesion_prc_S3_stop();
			break;
		default:break;
	}
}

static void UDS_A_reset_seesion_time_ctl(void)
{
	uds_uint8_t ses = UDS_A_get_session_time_ctl_Ses_sts();

	UDS_A_timer_ctl_reset(S_TIME_P2_CAN_SERVER);
	UDS_A_timer_ctl_reset(S_TIME_P2X_CAN_SERVER);
	switch (ses)
	{
		case A_SES_STS_DEFAULT:
			UDS_A_timer_ctl_reset(S_TIME_S3_SERVER);
			UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_IDLE);
			break;
		case A_SES_STS_NON_DEFAULT:
			UDS_A_timer_ctl_restart(S_TIME_S3_SERVER);
			UDS_A_set_session_time_ctl_sts(A_SES_TIME_CTL_STS_S3RUN);
			break;	
	}

}

static void UDS_A_seesion_time_management(void)
{
	UDS_A_seesion_time_process();
	UDS_A_seesion_time_process();
	UDS_A_seesion_time_process();
}

void UDS_A_DiagnosticInit(void) {

	/*�˴���Ҫ�ȶ�ȡ������EEPROM�е�KeyVerifyErrorCount*/
	if (0xff == KeyVerifyErrorCountFromEEPROM){
		KeyVerifyErrorCount = 0;
	}
	else{
		KeyVerifyErrorCount = KeyVerifyErrorCountFromEEPROM; //EEPROM��ȡ��Կ��֤ʧ�ܼ�������ֵ
	}

	if (MAXKEYERRORPERMITCOUNT == KeyVerifyErrorCount) {
		UDS_A_timer_ctl_run(S_TIME_SECURITYDELAY_SERVER); //������ʱ��ʱ��
		UnlockStep = WAIT_DELAY;
	}

	InitSetSessionSupportAndSecurityAccess(TRUE, 0x10, LEVEL_ZERO, 		LEVEL_ZERO, 	 LEVEL_ZERO,	LEVEL_ZERO, 	 LEVEL_ZERO, 	 	LEVEL_ZERO);
	InitSetSessionSupportAndSecurityAccess(TRUE, 0x11, LEVEL_UNSUPPORT, LEVEL_ZERO, 	 LEVEL_ZERO, 	LEVEL_UNSUPPORT, LEVEL_ZERO, 	 	LEVEL_ZERO);
	InitSetSessionSupportAndSecurityAccess(TRUE, 0x14, LEVEL_ZERO, 		LEVEL_UNSUPPORT, LEVEL_ZERO, 	LEVEL_ZERO, 	 LEVEL_UNSUPPORT, 	LEVEL_ZERO);
	InitSetSessionSupportAndSecurityAccess(TRUE, 0x19, LEVEL_ZERO, 		LEVEL_UNSUPPORT, LEVEL_ZERO, 	LEVEL_ZERO, 	 LEVEL_UNSUPPORT, 	LEVEL_ZERO);
	InitSetSessionSupportAndSecurityAccess(TRUE, 0x22, LEVEL_ZERO, 		LEVEL_ZERO, 	 LEVEL_ZERO, 	LEVEL_ZERO, 	 LEVEL_ZERO, 		LEVEL_ZERO);
	InitSetSessionSupportAndSecurityAccess(TRUE, 0x23, LEVEL_UNSUPPORT, LEVEL_ZERO, 	 LEVEL_ZERO, 	LEVEL_UNSUPPORT, LEVEL_ZERO,	 	LEVEL_ZERO);
	InitSetSessionSupportAndSecurityAccess(TRUE, 0x27, LEVEL_UNSUPPORT, LEVEL_ZERO, 	 LEVEL_ZERO, 	LEVEL_UNSUPPORT, LEVEL_UNSUPPORT, 	LEVEL_UNSUPPORT);
	InitSetSessionSupportAndSecurityAccess(TRUE, 0x3E, LEVEL_ZERO, 		LEVEL_ZERO, 	 LEVEL_ZERO, 	LEVEL_ZERO, 	 LEVEL_ZERO, 		LEVEL_ZERO);
}

/*����Ự֧���밲ȫ�ȼ���ʼ��*/
static void InitSetSessionSupportAndSecurityAccess(uds_uint8_t support, uds_uint8_t service, uds_uint8_t PHYDefaultSession_Security, uds_uint8_t PHYProgramSeesion_Security, uds_uint8_t PHYExtendedSession_Security, uds_uint8_t FUNDefaultSession_Security, uds_uint8_t FUNProgramSeesion_Security, uds_uint8_t FUNExtendedSession_Security) {
	uds_uint8_t i;
	for (i = 0; i < NUMBEROFSERVICE; i++) {
		if (DiagnosticServiceTable[i].ServiceID == service) {
			DiagnosticServiceTable[i].FUNDefaultSession_Security = FUNDefaultSession_Security;
			DiagnosticServiceTable[i].FUNExtendedSession_Security = FUNExtendedSession_Security;
			DiagnosticServiceTable[i].FUNProgramSeesion_Security = FUNProgramSeesion_Security;
			DiagnosticServiceTable[i].PHYDefaultSession_Security = PHYDefaultSession_Security;
			DiagnosticServiceTable[i].PHYExtendedSession_Security = PHYExtendedSession_Security;
			DiagnosticServiceTable[i].PHYProgramSeesion_Security = PHYProgramSeesion_Security;
			DiagnosticServiceTable[i].Support = support;
		}
	}
}

/*������ݶ�ȡ����Ч���ж�*/
void UDS_A_Diagnostic_Proc(void) {
	uds_uint8_t Tmp = UDS_A_service_get((UDS_N_Services_t *)&ServiceDataUnit, &SDUReceiveBuffArray[0]); //��ȡ�����������ݵ�Ԫ��������������ֵΪ���ݵ�Ԫ����

	UDS_A_ParameterInit();
	/*������*/
	if ((N_USDATA_INDICATION == Tmp) 
		|| (S_TIME_CNT_STS_RUN == UDS_A_timer_status_get(S_TIME_P2X_CAN_SERVER))) { //������ΪIndication���ݻ�P2*Server��ʱ������
		if (S_TIME_CNT_STS_IDLE == UDS_A_timer_status_get(S_TIME_P2X_CAN_SERVER)) { //��ȡ����
			UDS_A_Data_Indication_Read();
		}
		else {
			/*ʹ���ϴεķ�������*/
		}

		/*����Indication�����������*/
		if (OK == UDS_A_Indication_SDU.Result) {
			UDS_A_DiagnosticService(UDS_A_Indication_SDU.A_PCI);
			UDS_A_Response_Proc(NRC,
								ResponseDataLength,
								SuppressPosRspMsgIndicationBit, 
								UDS_A_Indication_SDU.TAType);

			/*Indication����*/
			if (TESTERPRESENT == UDS_A_Indication_SDU.A_PCI) {
				SystemState.IndicationType = TP_Indication;
			}
			else {
				SystemState.IndicationType = Indication;
			}
		}
		else {
			/*���Դ�������*/
		}
	}
	else if (N_USDATA_CONFIRM == Tmp) { //������ΪConfirm����
		UDS_A_Data_Confirm_Read(); //��ȡ����
		UDS_A_Data_Confirm_Prco(); //���̴���
	}
	else if (N_USDATA_FF_INDICATION == Tmp) { //������ΪFF_Indication����
		SystemState.IndicationType = FF_Indication;
	}
	else {

	}

	if (DefaultSession == SystemState.SessionMode) {
		SystemState.SessionType = Default_Session;
	}
	else {
		SystemState.SessionType = Non_Default_Session;
	}
	
	/*��Ӧ�ò����״̬�������Ự��*/
	UDS_A_set_session_time_ctl_Ses_sts(SystemState.SessionType);
	UDS_A_set_session_time_ctl_req(SystemState.ResponseType);
	UDS_A_set_session_time_ctl_con(SystemState.ConfirmType);
	UDS_A_set_session_time_ctl_ind(SystemState.IndicationType);
	UDS_A_set_session_time_ctl_sup(SystemState.SuppressPositiveType);
}

static void UDS_A_ParameterInit(void) {
	/*��������Init*/
	SystemState.SessionType = Default_Session;
	SystemState.IndicationType = Init_Indication;
	SystemState.ConfirmType = Init_Confirm;
	SystemState.ResponseType = Init_Response;
	SystemState.SuppressPositiveType = NotSuppressPositive;

	/*��Ӧ����Init*/
	NRC = PR_00;
	FlagClear_Func(&SuppressPosRspMsgIndicationBit);

	/*��ȫ������ʱ��ʱ��״̬���*/
	if (S_TIME_CNT_STS_TIMEOUT == UDS_A_timer_status_get(S_TIME_SECURITYDELAY_SERVER)) { //��ʱʱ�䵽��,�����������1
		UnlockStep = WAIT_SEED_REQ;
		UDS_A_timer_ctl_reset(S_TIME_SECURITYDELAY_SERVER); //��ʱ����λ��idle״̬
		KeyVerifyErrorCount--;
		/*�˴��轫KeyVerifyErrorCount������EEPROM*/
	}
	else {
	}
}

/*��ȡ������Indication����*/
static void UDS_A_Data_Indication_Read(void) {
	uds_uint8_t i;
	UDS_A_Indication_SDU.Mtype = ServiceDataUnit.Mtype;
	UDS_A_Indication_SDU.SourceAddr = ServiceDataUnit.SourceAddr;
	UDS_A_Indication_SDU.TargetAddr = ServiceDataUnit.TargetAddr;
	UDS_A_Indication_SDU.TAType = ServiceDataUnit.TAType;
#ifdef ADDRESS_EXTENSION_MODE
	UDS_A_Indication_SDU.A_ExtendAddr = ServiceDataUnit.A_ExtendAddr;
#endif
	UDS_A_Indication_SDU.Length = ServiceDataUnit.Length;
	UDS_A_Indication_SDU.Result = ServiceDataUnit.Result;
	UDS_A_Indication_SDU.A_PCI = SDUReceiveBuffArray[0]; //���ݵ�Ԫ��Ԫ��Ϊ��Ϸ���ID
	for (i = 1; i < UDS_A_Indication_SDU.Length; i++) {
		UDS_A_Indication_SDU.A_SDU[i - 1] = SDUReceiveBuffArray[i];
	}
}

/*��ȡ������Confirm����*/
static void UDS_A_Data_Confirm_Read(void) {
	UDS_A_Confirm_SDU.Mtype = ServiceDataUnit.Mtype;
	UDS_A_Confirm_SDU.SourceAddr = UDS_A_SOURCEADDRESS; //��ַ����
	UDS_A_Confirm_SDU.TargetAddr = UDS_A_TARGETADDRESS; //��ַ����
	UDS_A_Confirm_SDU.TAType = ServiceDataUnit.TAType;
#ifdef ADDRESS_EXTENSION_MODE
	UDS_A_Confirm_SDU.A_ExtendAddr = ServiceDataUnit.A_ExtendAddr;
#endif
	UDS_A_Confirm_SDU.Result = ServiceDataUnit.Result;
}

/*��Ϸ�����*/
static void UDS_A_DiagnosticService(uds_uint8_t DiagServiceID) {
	uds_uint8_t ServiceIndex = 0;
	uds_uint8_t ValidSid = 0;

	/*�������״̬�ж�*/
	if (1 == SystemState.DiagnosticProcessStatus) { //���ڴ�������У�����NRC78����P2*Server
		P2_ServerRequestFlag = 1;
		NRC = RCRRP_78;
		return;
	}
	else {
		FlagSet_Func(&SystemState.DiagnosticProcessStatus);
	}

	/*�����ѯ*/
	while ((ServiceIndex < NUMBEROFSERVICE) && (!ValidSid)) {
		if (DiagnosticServiceTable[ServiceIndex].ServiceID == DiagServiceID) {
			if (TRUE == DiagnosticServiceTable[ServiceIndex].Support) {
				ValidSid = TRUE;
			}
			else {
				ValidSid = FALSE;
				break;
			}
		}
		else {
			ServiceIndex++;
		}
	}

	/*Ѱַ����-�Ựģʽ-��ȫ�ȼ�-������*/
	if (TRUE == ValidSid) {
		if (Physical == UDS_A_Indication_SDU.TAType) {
			if (DefaultSession == SystemState.SessionMode) {
				if (DiagnosticServiceTable[ServiceIndex].PHYDefaultSession_Security == LEVEL_UNSUPPORT) {
					NRC = SNSIAS_7F;
				}
				else {
					if ((DiagnosticServiceTable[ServiceIndex].PHYDefaultSession_Security & SystemState.SecurityLevel) == SystemState.SecurityLevel) {
						DiagnosticServiceTable[ServiceIndex].ServiceHandle();
					}
					else {
						NRC = SAD_33;
					}
				}
			}
			else if (ExtendSession == SystemState.SessionMode) {
				if (DiagnosticServiceTable[ServiceIndex].PHYExtendedSession_Security == LEVEL_UNSUPPORT) {
					NRC = SNSIAS_7F;
				}
				else {
					if ((DiagnosticServiceTable[ServiceIndex].PHYExtendedSession_Security & SystemState.SecurityLevel) == SystemState.SecurityLevel) {
						DiagnosticServiceTable[ServiceIndex].ServiceHandle();
					}
					else {
						NRC = SAD_33;
					}
				}
			}
			else if (ProgramingSession == SystemState.SessionMode) {
				if (DiagnosticServiceTable[ServiceIndex].PHYProgramSeesion_Security == LEVEL_UNSUPPORT) {
					NRC = SNSIAS_7F;
				}
				else {
					if ((DiagnosticServiceTable[ServiceIndex].PHYProgramSeesion_Security & SystemState.SecurityLevel) == SystemState.SecurityLevel) {
						DiagnosticServiceTable[ServiceIndex].ServiceHandle();
					}
					else {
						NRC = SAD_33;
					}
				}
			}
			else if (FactorySeesion == SystemState.SessionMode) {
				if ((DIAGNOSTICSESSIONCONTROL == DiagnosticServiceTable[ServiceIndex].ServiceID)
					|| (READDATABYIDENTIFIER == DiagnosticServiceTable[ServiceIndex].ServiceID)
					|| (SECURITYACCESS == DiagnosticServiceTable[ServiceIndex].ServiceID)
					//|| (WRITEDATABYIDENTIFIER == DiagnosticServiceTable[ServiceIndex].ServiceID)
					|| (ECURESET == DiagnosticServiceTable[ServiceIndex].ServiceID)) {
					DiagnosticServiceTable[ServiceIndex].ServiceHandle();
				}
				else {
					NRC = SNSIAS_7F;
				}
			}
		}
		else if (Functional == UDS_A_Indication_SDU.TAType) {
			if (DefaultSession == SystemState.SessionMode) {
				if (DiagnosticServiceTable[ServiceIndex].PHYDefaultSession_Security == LEVEL_UNSUPPORT) {
					NRC = SNSIAS_7F;
				}
				else {
					if ((DiagnosticServiceTable[ServiceIndex].PHYDefaultSession_Security & SystemState.SecurityLevel) == SystemState.SecurityLevel) {
						DiagnosticServiceTable[ServiceIndex].ServiceHandle();
					}
					else {
						NRC = SAD_33;
					}
				}
			}
			else if (ExtendSession == SystemState.SessionMode) {
				if (DiagnosticServiceTable[ServiceIndex].PHYExtendedSession_Security == LEVEL_UNSUPPORT) {
					NRC = SNSIAS_7F;
				}
				else {
					if ((DiagnosticServiceTable[ServiceIndex].PHYExtendedSession_Security & SystemState.SecurityLevel) == SystemState.SecurityLevel) {
						DiagnosticServiceTable[ServiceIndex].ServiceHandle();
					}
					else {
						NRC = SAD_33;
					}
				}
			}
			else if (ProgramingSession == SystemState.SessionMode) {
				if (DiagnosticServiceTable[ServiceIndex].PHYProgramSeesion_Security == LEVEL_UNSUPPORT) {
					NRC = SNSIAS_7F;
				}
				else {
					if ((DiagnosticServiceTable[ServiceIndex].PHYProgramSeesion_Security & SystemState.SecurityLevel) == SystemState.SecurityLevel) {
						DiagnosticServiceTable[ServiceIndex].ServiceHandle();
					}
					else {
						NRC = SAD_33;
					}
				}
			}
			else if (FactorySeesion == SystemState.SessionMode) {
				NRC = SNS_11;
			}

		}
	}
	else {
		NRC = SNS_11;
	}
}

/*Confirm���ݴ���*/
static void UDS_A_Data_Confirm_Prco(void) {

	/*Normal_Confirm*/
	if (OK == UDS_A_Confirm_SDU.Result) {
		SystemState.ConfirmType = Normal_Confirm;
		FlagClear_Func(&SystemState.DiagnosticProcessStatus);
	}
	else {
		/*����*/
	}

	if (1 == P2_ServerRequestFlag) { //NRC78_Confirm
		if (OK == UDS_A_Confirm_SDU.Result) {
			SystemState.ConfirmType = NRC78_Confirm;
			FlagClear_Func(&P2_ServerRequestFlag); //��������
		}
		else {
			UDS_A_Response_Proc(RCRRP_78, NRCDATALENGTH, NULLFLAG, UDS_A_Indication_SDU.TAType); //�ظ�����NRC78-�ݶ�
		}
	}
	else if (TRUE == WaitConfimBeforeReset) { //��λ����ȷ��
		if (OK == UDS_A_Confirm_SDU.Result) {
			WaitConfimBeforeReset = FALSE;
			Reset_Func(ECUResetType);
		}
		else {

		}
	}


}

/****��������Ӧ�������***
NRC_Para:����Ӧ��
DataLength:���ݳ��ȣ�����Ӧʱ���ȹ̶�Ϊ3��
SuppPosiFlag������Ӧ���Ʊ�־��FALSEʱ����������Ӧ��
TAtype��Physical��Functional
*/
static void UDS_A_Response_Proc(NegativeResposeCode NRC_Para, uds_uint16_t DataLength, uds_uint8_t SuppPosiFlag, uds_uint8_t TAtype) {

	if ((PR_00 == NRC_Para) && (FALSE == SuppPosiFlag)) { //����Ӧ&����������Ӧ
		UDS_A_PositiveResponseDataUnit(DataLength);
	}
	else if ((NRC_Para != PR_00) && (!((Functional == TAtype) && ((SNS_11 == NRC_Para) || (SFNS_12 == NRC_Para) || (ROOR_31 == NRC_Para))))) { //����Ӧ&�ǹ���Ѱַ�µ��ض�NRC
		UDS_A_NegativeResponseDataUnit(NRC_Para);
	}
	else {

	}

	/*��Ͻ���״̬*/
	if (((PR_00 == NRC_Para) && (TRUE == SuppPosiFlag)) || ((NRC_Para != PR_00) && (NRC_Para != RCRRP_78))) {
		FlagClear_Func(&SystemState.DiagnosticProcessStatus);
	}

	/*Response����*/
	SystemState.ResponseType = Normal_Response;

	/*����Ӧ��������*/
	if ((PR_00 == NRC_Para) && (TRUE == SuppPosiFlag)) {
		SystemState.SuppressPositiveType = SuppressPositive;
	}
	else {
		SystemState.SuppressPositiveType = NotSuppressPositive;
	}
}

/*�������϶���Ӧ������*/
static void UDS_A_PositiveResponseDataUnit(uds_uint16_t DataLength) {
	UDS_A_Request_SDU.Mtype = UDS_A_Indication_SDU.Mtype;
	UDS_A_Request_SDU.SourceAddr = UDS_A_SOURCEADDRESS; //��ַ����
	UDS_A_Request_SDU.TargetAddr = UDS_A_TARGETADDRESS; //��ַ����
	UDS_A_Request_SDU.TAType = UDS_A_Indication_SDU.TAType;
#ifdef ADDRESS_EXTENSION_MODE
	UDS_A_Request_SDU.A_ExtendAddr = UDS_A_Indication_SDU.A_ExtendAddr;
#endif

	/*�϶���Ӧ�̶����ݸ�ʽ*/
	SDUSendBuffArray[0] = UDS_A_Indication_SDU.A_PCI + 0x40;
	UDS_A_Request_SDU.MessageData = SDUSendBuffArray;
	UDS_A_Request_SDU.Length = DataLength;

	/*���ݷ���������㻺����*/
	UDS_A_service_process_USData_request(&UDS_A_Request_SDU);
}

/*����������Ӧ������*/
static void UDS_A_NegativeResponseDataUnit(uds_uint8_t nrc) {
	UDS_A_Request_SDU.Mtype = UDS_A_Indication_SDU.Mtype;
	UDS_A_Request_SDU.SourceAddr = UDS_A_SOURCEADDRESS; //��ַ����
	UDS_A_Request_SDU.TargetAddr = UDS_A_TARGETADDRESS; //��ַ����
	UDS_A_Request_SDU.TAType = UDS_A_Indication_SDU.TAType;
#ifdef ADDRESS_EXTENSION_MODE
	UDS_A_Request_SDU.A_ExtendAddr = UDS_A_Indication_SDU.A_ExtendAddr;
#endif

	/*����Ӧ�̶����ݸ�ʽ*/
	SDUSendBuffArray[0] = 0x7f;
	SDUSendBuffArray[1] = UDS_A_Indication_SDU.A_PCI;
	SDUSendBuffArray[2] = nrc;
	UDS_A_Request_SDU.MessageData = SDUSendBuffArray;
	UDS_A_Request_SDU.Length = 3;

	/*���ݷ���������㻺����*/
	UDS_A_service_process_USData_request(&UDS_A_Request_SDU);
}

/*10h-�Ự���Ʒ���*/
static void Service10Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = FALSE;
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*�ӷ����ѯ*/
	while ((SubIndex < NUMBEROFMAXSUBSERVICE) && (!ValidSub)) {
		if (Service10Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (TRUE == ValidSub) {
		Service10Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

/*10h-Ĭ�ϻỰ�ӷ���*/
static void DefaultSession_Proc(void) {

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		SystemState.SessionMode = DefaultSession;
		SystemState.SecurityLevel = LEVEL_ZERO;
		if (UnlockStep != WAIT_DELAY) {
			UnlockStep = WAIT_SEED_REQ;
		}

		/*��Ӧ�������*/
		SDUSendBuffArray[1] = DefaultSessionType;
		SDUSendBuffArray[2] = (uds_uint8_t)(P2SERVER >> 8);
		SDUSendBuffArray[3] = (uds_uint8_t)P2SERVER;
		SDUSendBuffArray[4] = (uds_uint8_t)(P2_SERVER >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)P2_SERVER;
		ResponseDataLength = 6;
	}
}

/*10h-��չ�Ự�ӷ���*/
static void ExtendSession_Proc(void) {

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}

		/*�������л��Ựģʽ������*/
		if (ProgramingSession == SystemState.SessionMode) { //��̻Ự����ֱ���л�����չ�Ự
			NRC = CNC_22;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		SystemState.SessionMode = ExtendSession;
		SystemState.SecurityLevel = LEVEL_ZERO;
		if (UnlockStep != WAIT_DELAY) {
			UnlockStep = WAIT_SEED_REQ;
		}

		/*��Ӧ�������*/
		SDUSendBuffArray[1] = ExtendSessionType;
		SDUSendBuffArray[2] = (uds_uint8_t)(P2SERVER >> 8);
		SDUSendBuffArray[3] = (uds_uint8_t)P2SERVER;
		SDUSendBuffArray[4] = (uds_uint8_t)(P2_SERVER >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)P2_SERVER;
		ResponseDataLength = 6;
	}
}

/*10h-��̻Ự�ӷ���*/
static void ProgramingSession_Proc(void) {

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}

		/*�������л��Ựģʽ������*/
		if (DefaultSession == SystemState.SessionMode) { //Ĭ�ϻỰ����ֱ���л�����̻Ự
			NRC = CNC_22;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		SystemState.SessionMode = ProgramingSession;
		SystemState.SecurityLevel = LEVEL_ZERO;
		if (UnlockStep != WAIT_DELAY) {
			UnlockStep = WAIT_SEED_REQ;
		}

		/*��Ӧ�������*/
		SDUSendBuffArray[1] = ProgramingSessionType;
		SDUSendBuffArray[2] = (uds_uint8_t)(P2SERVER >> 8);
		SDUSendBuffArray[3] = (uds_uint8_t)P2SERVER;
		SDUSendBuffArray[4] = (uds_uint8_t)(P2_SERVER >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)P2_SERVER;
		ResponseDataLength = 6;
	}
}

/*11h-ECU��λ����*/
static void Service11Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = FALSE;
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	ECUResetType = ServiceSubFunc;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*�ӷ����ѯ*/
	while ((SubIndex < NUMBEROFMAXSUBSERVICE) && (!ValidSub)) {
		if (Service11Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (TRUE == ValidSub) {
		Service11Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

/*11h-Ӳ����λ�ӷ���*/
static void HardReset_Proc(void) {

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}

		/*��λ����������-�����*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		if (TRUE == SuppressPosRspMsgIndicationBit) { //��������Ӧֱ�Ӹ�λ
			Reset_Func(HARD_RESET);
		}
		else { //��λ֮ǰ�ȷ���Response
			SDUSendBuffArray[1] = HARD_RESET;
			ResponseDataLength = 2;
			WaitConfimBeforeReset = TRUE;
		}
	}
}

/*11h-Կ���µ��ϵ縴λ�ӷ���*/
static void KeyOnOffReset_Proc(void) {

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}

		/*��λ����������-�����*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		if (TRUE == SuppressPosRspMsgIndicationBit) { //��������Ӧֱ�Ӹ�λ
			Reset_Func(KEY_OFF_ON_RESET);
		}
		else { //��λ֮ǰ�ȷ���Response
			SDUSendBuffArray[1] = KEY_OFF_ON_RESET;
			ResponseDataLength = 2;
			WaitConfimBeforeReset = TRUE;
		}
	}
}

/*11h-�����λ�ӷ���*/
static void SoftReset_Proc(void) {

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}

		/*��λ����������-�����*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		if (TRUE == SuppressPosRspMsgIndicationBit) { //��������Ӧֱ�Ӹ�λ
			Reset_Func(SOFT_RESET);
		}
		else { //��λ֮ǰ�ȷ���Response
			SDUSendBuffArray[1] = SOFT_RESET;
			ResponseDataLength = 2;
			WaitConfimBeforeReset = TRUE;
		}
	}
}

/*11h-��λ�ӿں���*/
static void Reset_Func(EcuResetType_e ResetType) {
	if (HARD_RESET == ResetType) {
		UDS_PRINTF("MCU Reset!\n\r");
		//Ԥ��
	}
	else if (KEY_OFF_ON_RESET == ResetType) {
		//Ԥ��
	}
	else if (SOFT_RESET == ResetType) {
		//Ԥ��
	}
}

/*14h-ClearDiagnosticInformation����*/
static void Service14Handle(void) {
	uds_uint32_t GroupOfDTC;

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}

		/*�������������-�����*/
		if (0) {
			NRC = CNC_22;
			return;
		}

		/*groupOfDTC��������֧��-�����*/
		if (0) {
			NRC = ROOR_31;
			return;
		}
	}

	/*�������ͨ����������*/
	{

		/*���DTC-�ݶ�*/
		GroupOfDTC = ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[0] << 16) | ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[1] << 8) | (uds_uint32_t)UDS_A_Indication_SDU.A_SDU[2];
		memset(DTCBytesTable, 0, DTCBYTELENGTH); //�������DTC���ݣ���RAM��EEPROM
		ResponseDataLength = 1;
	}
}

/*19h-��ȡDTC��Ϣ����*/
static void Service19Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = FALSE;
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*�ӷ����ѯ*/
	while ((SubIndex < NUMBEROFMAXSUBSERVICE) && (!ValidSub)) {
		if (Service19Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (TRUE == ValidSub) {
		Service19Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

/*19h-01ReportNumberOfDTCByStatusMask�ӷ���*/
static void ReportDTCStatusMaskNum01_Proc(void) {
	uds_uint8_t DTCStatusMask; //DTC״̬����

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 3) {
			NRC = IMLOIF_13;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		DTCStatusMask = UDS_A_Indication_SDU.A_SDU[1];

		/*��Ӧ�������*/
		SDUSendBuffArray[1] = reportNumberOfDTCByStatusMask;
		SDUSendBuffArray[2] = DTCStatusAvailabilityMask; //DTC״̬��Ч����-����
		SDUSendBuffArray[3] = SAE_J2012_DA_DTCFormat_00; //����
		SDUSendBuffArray[4] = (uds_uint8_t)(DTCCountByte >> 8); //DTC�ֽ���-����
		SDUSendBuffArray[5] = (uds_uint8_t)DTCCountByte; //DTC�ֽ���-����
		ResponseDataLength = 6;
	}
}

/*19h-02reportDTCByStatusMask�ӷ���*/
static void ReportDTCStatusMask02_Proc(void) {
	uds_uint8_t DTCStatusMask; //DTC״̬����

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 3) {
			NRC = IMLOIF_13;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		DTCStatusMask = UDS_A_Indication_SDU.A_SDU[1];

		/*��Ӧ�������*/
		SDUSendBuffArray[1] = reportDTCByStatusMask;
		SDUSendBuffArray[2] = DTCStatusAvailabilityMask; //DTC״̬��Ч����-����
#if 0
		SDUSendBuffArray[3] = (uds_uint8_t)(DTCStatusByte >> 16); //DTC״̬�ֽ�-����
		SDUSendBuffArray[4] = (uds_uint8_t)(DTCStatusByte >> 8); //DTC״̬�ֽ�-����
		SDUSendBuffArray[5] = (uds_uint8_t)DTCStatusByte; //DTC״̬�ֽ�-����
		SDUSendBuffArray[6] = (uds_uint8_t)statusOfDTC; //DTC״̬-����

		//�����ֽ���ȡ����DTCStatusMask��DTCStatusAvailabilityMask������Ϊ3 + 4 * N
		ResponseDataLength = 7;
#endif
	}
}

/*19h-04ReportDTCSnapshotRecordByDTCNumber�ӷ���*/
static void ReportDTCSnapshotRecord04_Proc(void) {
	uds_uint32_t DTCMaskRecord; //DTC�����¼
	uds_uint8_t DTCSnapshotRecordNumber; //00-OBD,01~FE��Ӧ�ļ�¼��ţ�FF-�ϴ����еĿ�������

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 6) {
			NRC = IMLOIF_13;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		DTCMaskRecord = ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[1] << 16) | ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[2] << 8) | (uds_uint32_t)UDS_A_Indication_SDU.A_SDU[3];
		DTCSnapshotRecordNumber = UDS_A_Indication_SDU.A_SDU[4];

		/*��Ӧ�������*/
		SDUSendBuffArray[1] = repDTCSnapshotRecordByDTCNum;
#if 0
		SDUSendBuffArray[2] = (uds_uint8_t)(DTCStatusByte >> 16); //DTC״̬�ֽ�-����
		SDUSendBuffArray[3] = (uds_uint8_t)(DTCStatusByte >> 8); //DTC״̬�ֽ�-����
		SDUSendBuffArray[4] = (uds_uint8_t)DTCStatusByte; //DTC״̬�ֽ�-����
		SDUSendBuffArray[5] = (uds_uint8_t)statusOfDTC; //DTC״̬-����

		SDUSendBuffArray[6] = (uds_uint8_t)DTCSnapshotRecordNumber; //DTC���ռ�¼���-����
		SDUSendBuffArray[7] = (uds_uint8_t)DTCSnapshotRecordNumberOfIdentifiers; //DTC���ռ�¼DID��Ŀ-����
		SDUSendBuffArray[8] = (uds_uint8_t)(DataIdentifier >> 8); //DID���ֽ�-����
		SDUSendBuffArray[9] = (uds_uint8_t)DataIdentifier; //DID���ֽ�-����
		SDUSendBuffArray[10] = (uds_uint8_t)SnapshotData; //��������-����

		//DTCSnapshotRecordNumberΪ01h~FEh,DataLengthΪ8+3*DID��Ŀ��ΪFFhʱ��DataLength=6+2x+3(y1+y2+...),x-���ռ�¼�����Ŀ��y1,y2,y3...-����DTC���ռ�¼������DID��Ŀ
		ResponseDataLength = 11;
#endif
	}
}

/*19h-0AReportSupportedDTC�ӷ���*/
static void ReportSupportedDTC0A_Proc(void) {

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}
	}

	/*�������ͨ����������*/
	{

		/*��Ӧ�������*/
		SDUSendBuffArray[1] = reportSupportedDTC;
		SDUSendBuffArray[2] = DTCStatusAvailabilityMask; //DTC״̬��Ч����-����
#if 0
		SDUSendBuffArray[3] = (uds_uint8_t)(DTCStatusByte >> 16); //DTC״̬�ֽ�-����
		SDUSendBuffArray[4] = (uds_uint8_t)(DTCStatusByte >> 8); //DTC״̬�ֽ�-����
		SDUSendBuffArray[5] = (uds_uint8_t)DTCStatusByte; //DTC״̬�ֽ�-����
		SDUSendBuffArray[7] = (uds_uint8_t)statusOfDTC; //DTC״̬-����

													//�����ֽ���ȡ����DTCStatusAvailabilityMask������Ϊ3 + 4 * N
		ResponseDataLength = 7;
#endif
	}
}

/*22h-ReadDataByIdentifier����*/
static void Service22Handle(void) {
	uds_uint16_t DataIdentifier, DIDReverse;
	uds_uint8_t i, j, Tmp = 0, DidDataLength = 1; //��ʼ���ݳ���Ϊ1����һ���ֽ�Ϊ��Ϸ���0x22
	uds_uint16_t NumOfDid;

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if ((UDS_A_Indication_SDU.Length < 3) || (0 == (UDS_A_Indication_SDU.Length % 2))) {
			NRC = IMLOIF_13;
			return;
		}
		else if (UDS_A_Indication_SDU.Length >(2 * MAXNUMBEROFDID + 1)) {
			NRC = ROOR_31;
			return;
		}

		/*��������������-�����*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		NumOfDid = (UDS_A_Indication_SDU.Length - 1) / 2;
		for (i = 0; i < NumOfDid; i++) {
			DataIdentifier = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2 * i] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2 * i + 1];
			DIDReverse = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2 * i + 1] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2 * i];
			for (j = 0; j < NUMOFSUPPORTEDDID; j++) {
				if (DataIdentifier == ArrayOfDID[j].DataIdentifier) {
					memcpy(&SDUSendBuffArray[DidDataLength], &DIDReverse, 2);
					memcpy((SDUSendBuffArray + DidDataLength + 2), ArrayOfDID[j].DataArray, ArrayOfDID[j].DataBytesLength);
					DidDataLength = DidDataLength + 2 + ArrayOfDID[j].DataBytesLength;
					Tmp++;
				}
			}
		}

		/*DID��Ŀ���*/
		if (NumOfDid != Tmp) {
			NRC = ROOR_31;
		}
		else {
			ResponseDataLength = DidDataLength;
		}
	}
}

/*23h-ReadMemoryByAddress����*/
static void Service23Handle(void) {
	uds_uint16_t memorySizeBytes = (uds_uint16_t)((UDS_A_Indication_SDU.A_SDU[0] >> 4) & 0x0f), memoryAddressBytes = (uds_uint16_t)(UDS_A_Indication_SDU.A_SDU[0] & 0x0f);
	uds_uint32_t* memoryAddress;
	uds_uint32_t memorySize;

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if ((UDS_A_Indication_SDU.Length < 4) || (UDS_A_Indication_SDU.Length != (memorySizeBytes + memoryAddressBytes + 2)) || (memorySizeBytes > 4) || (memoryAddressBytes > 4)) {
			NRC = IMLOIF_13;
			return;
		}

		/*�ӷ���addressAndLengthFormatIdentifier����*/
		if ((0 == memorySizeBytes) || (0 == memoryAddressBytes)) { //(memorySizeBytes > ������֧�ֵ����ֵ)����ַ��Ч�Լ��ȴ����
			NRC = ROOR_31;
			return;
		}

		/*��������������-�����*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		memoryAddress = ReadMemoryByAddress_Func((&UDS_A_Indication_SDU.A_SDU[1]), memoryAddressBytes);
		memorySize = ReadMemorySize_Func(&UDS_A_Indication_SDU.A_SDU[memoryAddressBytes + 1], memorySizeBytes);
		memcpy(&SDUSendBuffArray[1], memoryAddress, memorySize);
		ResponseDataLength = memorySize + 1;
	}
}

/*27h-��ȫ���ʷ���*/
static void Service27Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = FALSE;
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*�ӷ����ѯ*/
	while ((SubIndex < NUMBEROFMAXSUBSERVICE) && (!ValidSub)) {
		if (Service27Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (TRUE == ValidSub) {
		Service27Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

/*27h-SecurityAccess Seed01�ӷ���*/
static void SecurityAccessSeed01_Proc(void) {
	uds_uint32_t Seed;

	/*�����������*/
	{

		/*�ӳٶ�ʱ�����ڼ���״̬*/
		if (WAIT_DELAY == UnlockStep) {
			NRC = RTDNE_37;
			return;
		}

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*��ȫ��������������-�����*/
		if (0) {
			NRC = CNC_22;
			UnlockStep = WAIT_SEED_REQ;
			return;
		}
	}

	/*�������ͨ����������*/
	{

		/*���Ӽ���*/
		if (LEVEL_ONE == SystemState.SecurityLevel) { //��������ȫ�ȼ����ڵ�ǰϵͳ��ȫ�ȼ�
			Seed = 0;
		}
		else {
			Seed = SeedCalc01_Func(SEED01); //����Seed
			Key = KeyCalc01_Func(SEED01); //����Key
			UnlockStep = WAIT_KEY;
		}

		/*��Ӧ�������*/
		SDUSendBuffArray[1] = SEED01;
		SDUSendBuffArray[2] = (uds_uint8_t)(Seed >> 24);
		SDUSendBuffArray[3] = (uds_uint8_t)(Seed >> 16);
		SDUSendBuffArray[4] = (uds_uint8_t)(Seed >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)Seed;
		ResponseDataLength = 6;
	}
}

/*27h-SecurityAccess Key02�ӷ���*/
static void SecurityAccessKey02_Proc(void) {
	uds_uint32_t SeedKey;

	/*�����������*/
	{

		/*�ӳٶ�ʱ�����ڼ���״̬*/
		if (WAIT_DELAY == UnlockStep) {
			NRC = RTDNE_37;
			return;
		}

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 6) {
			NRC = IMLOIF_13;
			UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*��ȫ��������������-�����*/
		if (0) {
			NRC = CNC_22;
			UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*�������д���*/
		if (WAIT_SEED_REQ == UnlockStep) {
			NRC = RSE_24;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		SeedKey = ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[1] << 24) | ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[2] << 16) | ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[3] << 8) | (uds_uint32_t)UDS_A_Indication_SDU.A_SDU[4];
		if (SeedKey == Key) {
			KeyVerifyErrorCount = 0; //��Կ��֤ʧ�ܼ���������
			SystemState.SecurityLevel = LEVEL_ONE; //�л���ȫ�ȼ�
			UnlockStep = UNLOCKED;

			/*��Ӧ�������*/
			SDUSendBuffArray[1] = KEY02;
			ResponseDataLength = 2;
		}
		else {
			if (KeyVerifyErrorCount < (MAXKEYERRORPERMITCOUNT - 1)) {
				KeyVerifyErrorCount++;
				NRC = IK_35;
				UnlockStep = WAIT_SEED_REQ;
			}
			else {

				/*���������Կʧ���������*/
				KeyVerifyErrorCount = MAXKEYERRORPERMITCOUNT;
				UDS_A_timer_ctl_run(S_TIME_SECURITYDELAY_SERVER); //������ʱ��ʱ��
				UnlockStep = WAIT_DELAY;
				NRC = ENOA_36;
			}

			/*�˴��轫KeyVerifyErrorCount������EEPROM*/
		}
	}
}

/*27h-SecurityAccess Seed03�ӷ���*/
static void SecurityAccessSeed03_Proc(void) {
	uds_uint32_t Seed;

	/*�����������*/
	{

		/*�ӳٶ�ʱ�����ڼ���״̬*/
		if (WAIT_DELAY == UnlockStep) {
			NRC = RTDNE_37;
			return;
		}

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*��ȫ��������������-�����*/
		if (0) {
			NRC = CNC_22;
			UnlockStep = WAIT_SEED_REQ;
			return;
		}
	}

	/*�������ͨ����������*/
	{

		/*���Ӽ���*/
		if (LEVEL_TWO == SystemState.SecurityLevel) { //��������ȫ�ȼ����ڵ�ǰϵͳ��ȫ�ȼ�
			Seed = 0;
		}
		else {
			Seed = SeedCalc03_Func((uds_uint32_t)SEED03); //����Seed
			Key = KeyCalc03_Func((uds_uint32_t)SEED03); //����Key
			UnlockStep = WAIT_KEY;
		}

		/*��Ӧ�������*/
		SDUSendBuffArray[1] = SEED03;
		SDUSendBuffArray[2] = (uds_uint8_t)(Seed >> 24);
		SDUSendBuffArray[3] = (uds_uint8_t)(Seed >> 16);
		SDUSendBuffArray[4] = (uds_uint8_t)(Seed >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)Seed;
		ResponseDataLength = 6;
	}
}

/*27h-SecurityAccess Key04�ӷ���*/
static void SecurityAccessKey04_Proc(void) {
	uds_uint32_t SeedKey;

	/*�����������*/
	{

		/*�ӳٶ�ʱ�����ڼ���״̬*/
		if (WAIT_DELAY == UnlockStep) {
			NRC = RTDNE_37;
			return;
		}

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 6) {
			NRC = IMLOIF_13;
			UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*��ȫ��������������-�����*/
		if (0) {
			NRC = CNC_22;
			UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*�������д���*/
		if (WAIT_SEED_REQ == UnlockStep) {
			NRC = RSE_24;
			return;
		}
	}

	/*�������ͨ����������*/
	{
		SeedKey = ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[1] << 24) | ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[2] << 16) | ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[3] << 8) | (uds_uint32_t)UDS_A_Indication_SDU.A_SDU[4];
		if (SeedKey == Key) {
			KeyVerifyErrorCount = 0; //��Կ��֤ʧ�ܼ���������
			SystemState.SecurityLevel = LEVEL_TWO; //�л���ȫ�ȼ�
			UnlockStep = UNLOCKED; //����״̬

								   /*��Ӧ�������*/
			SDUSendBuffArray[1] = KEY04;
			ResponseDataLength = 2;
		}
		else {
			if (KeyVerifyErrorCount < (MAXKEYERRORPERMITCOUNT - 1)) {
				KeyVerifyErrorCount++;
				NRC = IK_35;
				UnlockStep = WAIT_SEED_REQ;
			}
			else {

				/*���������Կʧ���������*/
				KeyVerifyErrorCount = MAXKEYERRORPERMITCOUNT;
				UDS_A_timer_ctl_run(S_TIME_SECURITYDELAY_SERVER); //������ʱ��ʱ��
				UnlockStep = WAIT_DELAY;
				NRC = ENOA_36;
			}

			/*�˴��轫KeyVerifyErrorCount������EEPROM*/
		}
	}
}

/*3Eh-�Ự���ַ���*/
static void Service3EHandle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = FALSE;
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*�ӷ����ѯ*/
	while ((SubIndex < NUMBEROFMAXSUBSERVICE) && (!ValidSub)) {
		if (Service3ETable[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (TRUE == ValidSub) {
		Service3ETable[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

/*3Eh-�Ự��������������Ӧ00h�ӷ���*/
static void TesterPresent00_Proc(void) {

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}
	}

	/*�������ͨ����������*/
	{

		/*��Ӧ�������*/
		SDUSendBuffArray[1] = 0x00;
		ResponseDataLength = 2;
	}
}

/*3Eh-�Ự������������Ӧ80h�ӷ���*/
static void TesterPresent80_Proc(void) {

	/*�����������*/
	{

		/*�ӷ��񳤶ȴ���*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}
	}
}

/*Seed����-�����*/
static uds_uint32_t SeedCalc01_Func(uds_uint8_t RequestSeed) {
	uds_uint32_t Seed;
	Seed = (uds_uint32_t)RequestSeed + 0x11110000;
	return Seed;
}

/*Key����-�����*/
static uds_uint32_t KeyCalc01_Func(uds_uint32_t RequestSeed) {
	uds_uint32_t Key;
	Key = (uds_uint32_t)RequestSeed + 0x00001111;
	return Key;
}

/*Seed����-�����*/
static uds_uint32_t SeedCalc03_Func(uds_uint8_t RequestSeed) {
	uds_uint32_t Seed;
	Seed = (uds_uint32_t)RequestSeed + 0x33330000;
	return Seed;
}

/*Key����-�����*/
static uds_uint32_t KeyCalc03_Func(uds_uint32_t RequestSeed) {
	uds_uint32_t Key;
	Key = (uds_uint32_t)RequestSeed + 0x00003333;
	return Key;
}

/*��ȡ�ڴ��ַ*/
static uds_uint32_t *ReadMemoryByAddress_Func(uds_uint8_t *Address, uds_uint16_t Length) {
	uds_uint32_t *MemoryParameter = 0;
	switch (Length) {
	case 1:
		MemoryParameter = (uds_uint32_t *)((uds_uint32_t)(*Address));
		break;
	case 2:
		MemoryParameter = (uds_uint32_t *)(((uds_uint32_t)(*Address) << 8) | (uds_uint32_t)(*(Address + 1)));
		break;
	case 3:
		MemoryParameter = (uds_uint32_t *)(((uds_uint32_t)(*Address) << 16) | ((uds_uint32_t)(*(Address + 1)) << 8) | (uds_uint32_t)(*(Address + 2)));
		break;
	case 4:
		MemoryParameter = (uds_uint32_t *)(((uds_uint32_t)(*Address) << 24) | ((uds_uint32_t)(*(Address + 1)) << 16) | ((uds_uint32_t)(*(Address + 2)) << 8) | (uds_uint32_t)(*(Address + 3)));
		break;
	default:
		break;
	}
	return MemoryParameter;
}

/*��ȡ�ڴ泤��*/
static uds_uint32_t ReadMemorySize_Func(uds_uint8_t *Address, uds_uint16_t Length) {
	uds_uint32_t MemoryParameter = 0;
	switch (Length) {
	case 1:
		MemoryParameter = *Address;
		break;
	case 2:
		MemoryParameter = ((*Address) << 8) | (*(Address + 1));
		break;
	case 3:
		MemoryParameter = ((*Address) << 16) | ((*(Address + 1)) << 8) | (*(Address + 2));
		break;
	case 4:
		MemoryParameter = ((*Address) << 24) | ((*(Address + 1)) << 16) | ((*(Address + 2)) << 8) | (*(Address + 3));
		break;
	default:
		break;
	}
	return MemoryParameter;
}

/*��־����*/
static void FlagClear_Func(uds_uint8_t *Flag) {
	(*Flag) = 0;
}

/*��־��λ*/
static void FlagSet_Func(uds_uint8_t *Flag) {
	(*Flag) = 1;
}

/*S3Server��ʱ����*/
static void UDS_A_S3_Timeout_Proc(void) {
	if (S_TIME_CNT_STS_TIMEOUT == UDS_A_timer_status_get(S_TIME_S3_SERVER)) { //S3��ʱ���»Ự�л�
		UDS_A_reset_seesion_time_ctl();
		UDS_A_timer_ctl_reset(S_TIME_S3_SERVER);
		FlagClear_Func(&SystemState.DiagnosticProcessStatus);
		SystemState.SessionType = Default_Session;
		SystemState.IndicationType = Init_Indication;
		SystemState.ConfirmType = Init_Confirm;
		SystemState.ResponseType = Init_Response;
		SystemState.SuppressPositiveType = NotSuppressPositive;
		SystemState.SessionMode = DefaultSession;
		SystemState.SecurityLevel = LEVEL_ZERO;
		if (UnlockStep != WAIT_DELAY) {
			UnlockStep = WAIT_SEED_REQ;
		}
		UDS_PRINTF("S3 timeout!\n\r");
	}
	else {

	}
}

/**********************************************************************
* @brief	   Init application layer
*
* @param[in]   None.
*
* @return	   None.
**********************************************************************/
uds_int8_t UDS_A_init(void)
{
	UDS_Interface_In_t *intf = (UDS_Interface_In_t *)get_uds_internal_interface();
	uds_int8_t ret = -1;

	ret = UDS_A_interface_regist(intf);
	if(1 != ret){
		UDS_PRINTF("UDS Error:Application layer interface regist failed!\n\r");
	}
	UDS_A_DiagnosticInit();
	
	return ret;
}
/**********************************************************************
* @brief	   Application main process.
*
* @param[in]   None.
*
* @return	   None.
**********************************************************************/
void uds_application_all(void)
{
	UDS_A_Diagnostic_Proc();
	UDS_A_seesion_time_management();
	UDS_A_S3_Timeout_Proc();
}
