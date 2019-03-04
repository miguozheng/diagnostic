#include "uds_common_include.h"
#include "uds_struct.h"


/*************************1*****************************************************
Private global variables and functions
******************************************************************************/
static UDS_A_Confirm_t UDS_A_Confirm_SDU;
static UDS_A_Indication_t UDS_A_Indication_SDU;
static ServiceDataUnit_t ServiceDataUnit;
static UDS_A_Request_t UDS_A_Request_SDU;

//session control
static UDS_A_Session_Time_Control_t Session_ctl;
uds_uint8_t SDUReceiveBuffArray[MAXDATALENGTH] = { 0 };
uds_uint8_t SDUSendBuffArray[MAXDATALENGTH] = { 0 };
uds_uint8_t SDUSendBuffArray_0X2A[8] = { 0 };
static uds_uint8_t DataLoad_DefaultValue[10] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };  //暂定

/*服务处理*/
static NegativeResposeCode NRC;
static uds_uint16_t ResponseDataLength;
static uds_uint8_t ResponseDataLength_0x2A;
static uds_uint8_t ServiceSubFunc;
static uds_uint8_t SuppressPosRspMsgIndicationBit;
static uds_uint8_t ECUResetType;
static uds_uint8_t WaitConfimBeforeReset = UDS_FALSE;
static uds_uint8_t P2_ServerRequestFlag = UDS_FALSE;
static uds_uint8_t ServiceIndex = 0;
static uds_uint8_t ConfirmDTCUpdate = UDS_TRUE;
static uds_uint8_t WaitConfirmBeforeDTCUpdateEnable = UDS_FALSE;
static uds_uint8_t WaitConfirmBeforeDTCUpdateDisable = UDS_FALSE;
static LinkControlStep_e LinkControlStep = WAIT_MODE_REQ;
static LinkStatus_t LinkStatus;
static DataTransfer_t DataTransfer;

/*DTC*/
static uds_uint8_t DTCStatusAvailabilityMask; //provides an indication of DTC status bits that are supported by the server for masking purposes
//static uds_uint8_t DTCStatusStoreMask;
static DTCNode DTCTable[MAX_DTC_NUMBER];
static uds_uint8_t NumberofDTC = 0;
static uds_uint8_t StatusOfDTCFromEEPROM;
static DTCGroup_t DTCGroupTable[MAX_DTCGROUP_NUMBER];
static uds_uint8_t NumberofDTCGroup = 0;
static DTCSnapshot_t DTCSnapshotTable[MAX_RECORDNUMBER_NUMBER];
static uds_uint8_t NumberofDTCSnapshot = 0;
static uds_uint8_t FaultOccurenceCounterRecord = INVALID_DTCEXTDATARECORDNUMBER; //范围0x01-0x8F
static uds_uint8_t FaultPendingCounterRecord = INVALID_DTCEXTDATARECORDNUMBER; //范围0x01-0x8F
static uds_uint8_t FaultAgingCounterRecord = INVALID_DTCEXTDATARECORDNUMBER; //范围0x01-0x8F
static uds_uint8_t FaultAgedCounterRecord = INVALID_DTCEXTDATARECORDNUMBER; //范围0x01-0x8F

uds_uint8_t VehicleSpeed = VEHICLESPEED_TEST; //km/h-预留
uds_uint16_t EngineRPM = ENGINERPM_TEST; //rpm-预留
uds_uint16_t Voltage = VOLTAGE_TEST; //mv-预留

/*根据DID写数据*/
uds_uint8_t Test_RAM_2E;
uds_uint8_t Test_Table[5];
uds_uint8_t VIN_Number[VIN_NUMBER_BYTES];

/*根据DID控制输入输出参数*/
struct {
	uds_uint16_t EngineRPM;
	uds_uint8_t VehicleSpeed;
	uds_uint16_t Voltage;	
	uds_uint8_t Array[3];
} IO_ControlPara;

union {
	uds_uint8_t status;
	struct {
		uds_uint8_t EngineRPM_Flag : 1;
		uds_uint8_t VehicleSpeed_Flag : 1;
		uds_uint8_t Voltage_Flag : 1;
	} Flag_t;
} IO_ControlPara_Flag_u;

/*可读DID*/
static uds_uint8_t NumberofStaticReadDID = 0;
static uds_uint8_t NumberofReadDID = 0;
static ReadDatabyInentifier_t ReadDIDTable[NUMOFREADDID];
static uds_uint8_t NumofPeriodicDIDRunning = 0;
static PeriodicDID_t PeriodicDIDArray;
static uds_uint8_t PeriodicDIDCnt = 0;
static uds_uint8_t DataBufferPutCycle = DATAPUTCYCLE;
static uds_uint8_t WaitConfirmBeforePeridDIDSend = UDS_FALSE;
static uds_uint8_t ConfirmPeridDIDSend = UDS_FALSE;
static uds_uint8_t NumberofSourceDataRecord = 0;
static SourceDataRecord_t SourceDataRecordTable[MAX_SOURCEDATARECORD_NUMBER];

/*Routine*/
static uds_uint8_t NumberofRoutine = 0;
static Routine_t RoutineTable[MAX_ROUTINE_NUMBER];

/*安全访问*/
static SecurityAccess_t SecurityAccessPara;

/*系统参数*/
struct {
	DiagnosticSession_e SessionMode;
	SecurityLevel_e SecurityLevel;
	DiagnosticProcStatus_e DiagnosticProcessStatus;
	SessionType_e SessionType;
	IndicationType_e IndicationType;
	ConfirmType_e ConfirmType;
	ResponseType_e ResponseType;
	SuppressPositiveType_e SuppressPositiveType;
} SystemState;

uds_uint8_t ReadDIDArrayA815[] = { 0x11, 0x22, 0x33 }; //待添加
struct {
	uds_uint8_t xxxx;
	uds_uint8_t yyyy;
	uds_uint8_t zzzz;
	uds_uint8_t tttt;
} ReadDIDArrayB128 = { 0x12, 0x34, 0x56, 0x78 }; //待添加-test

/*
============================================================================
 Function declear
============================================================================
*/

//App get the lower layer service
static uds_uint8_t (*_UDS_A_service_get)(UDS_N_Services_t *res,uds_uint8_t *buf);
//App layer issue a USData.request service
static uds_int8_t (*_UDS_A_service_process_USData_request)(void *pdata);
//App layer issue a ParaChange.request service
static uds_int8_t (*_UDS_A_service_process_ParaChange_request)(void *pdata);
//App get the session layer timer status
static uds_int8_t (*_UDS_A_timer_status_get)(UDS_S_Time_Name_e time);
static uds_int8_t (*_UDS_A_timer_ctl_stop)(UDS_S_Time_Name_e time);
static uds_int8_t (*_UDS_A_timer_ctl_run)(UDS_S_Time_Name_e time);
static uds_int8_t (*_UDS_A_timer_ctl_restart)(UDS_S_Time_Name_e time);
static uds_int8_t (*_UDS_A_timer_ctl_reset)(UDS_S_Time_Name_e time);
static uds_uint16_t (*_UDS_A_timer_value_get)(UDS_S_Time_Name_e time);

//App eeprom
static uds_int8_t (*_UDS_A_eeprom_read)(UDS_EE_Tag_e tag,uds_uint8_t *buf);
static uds_int8_t (*_UDS_A_eeprom_write)(UDS_EE_Tag_e tag,uds_uint8_t *buf);

//send frame directly
static uds_int8_t (*_UDS_A_send_frame_direct)(uds_uint32_t id,uds_uint8_t length,uds_uint8_t *data);

//App get the lower layer service
static uds_uint8_t UDS_A_service_get(UDS_N_Services_t *res,uds_uint8_t *buf);
//App layer issue a USData.request service
static uds_int8_t UDS_A_service_process_USData_request(void *pdata);
//App layer issue a ParaChange.request service
static uds_int8_t UDS_A_service_process_ParaChange_request(void *pdata);
//App get the session layer timer status
static uds_int8_t UDS_A_timer_status_get(UDS_S_Time_Name_e time);
static uds_int8_t UDS_A_timer_ctl_stop(UDS_S_Time_Name_e time);
static uds_int8_t UDS_A_timer_ctl_run(UDS_S_Time_Name_e time);
static uds_int8_t UDS_A_timer_ctl_restart(UDS_S_Time_Name_e time);
static uds_int8_t UDS_A_timer_ctl_reset(UDS_S_Time_Name_e time);
static uds_uint16_t UDS_A_timer_value_get(UDS_S_Time_Name_e time);

//App eeprom
static uds_int8_t UDS_A_eeprom_read(UDS_EE_Tag_e tag,uds_uint8_t *buf);
static uds_int8_t UDS_A_eeprom_write(UDS_EE_Tag_e tag,uds_uint8_t *buf);

//send frame directly
static uds_int8_t UDS_A_send_frame_direct(uds_uint32_t id,uds_uint8_t length,uds_uint8_t *data);


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
static void DiagnosticState_Init(DiagnosticSession_e SessionMode, SecurityLevel_e SecurityLevel, DiagnosticProcStatus_e DiagnosticProcessStatus, 
								 SessionType_e SessionType, IndicationType_e IndicationType, ConfirmType_e ConfirmType, 
								 ResponseType_e ResponseType, SuppressPositiveType_e SuppressPositiveType);
static void SecurityAccess_Init(void);

static void Service10Handle(void);
static void DefaultSession_01Proc(void);
static void ProgramingSession_02Proc(void);
static void ExtendSession_03Proc(void);
static void FactorySeesion_04Proc(void);

static void Service11Handle(void);
static void HardReset_01Proc(void);
static void KeyOnOffReset_02Proc(void);
static void SoftReset_03Proc(void);

static void Service14Handle(void);

static void Service19Handle(void);
static void ReportNumberOfDTCByStatusMask_01Proc(void);
static void ReportDTCByStatusMask_02Proc(void);
static void ReportDTCSnapshotID_03Proc(void);
static void ReportDTCSnapshotRecord_04Proc(void);
static void ReportDTCStoredData_05Proc(void);
static void ReportDTCExtDataRecord_06Proc(void);
static void ReportSupportedDTC_0AProc(void);

static void Service22Handle(void);

static void Service23Handle(void);

static void Service27Handle(void);
static void SecurityAccessSeed_01Proc(void);
static void SecurityAccessKey_02Proc(void);
static void SecurityAccessSeed_03Proc(void);
static void SecurityAccessKey_04Proc(void);

static void Service28Handle(void);
static void EnableRxAndTx_00Proc(void);
static void EnableRxAndDisableTx_01Proc(void);
static void DisableRxAndEnableTx_02Proc(void);
static void DisableRxAndTx_03Proc(void);

static void Service2AHandle(void);
static void SendAtSlowRate_01Proc(void);
static void SendAtMediumRate_02Proc(void);
static void SendAtFastRate_03Proc(void);
static void StopSending_04Proc(void);

static void Service2CHandle(void);
static void DefineByIdentifier_01Proc(void);
static void DefineByMemoryAddress_02Proc(void);
static void ClearDynamicallyDefinedDataIdentifier_03Proc(void);

static void Service2EHandle(void);

static void Service2FHandle(void);

static void Service31Handle(void);
static void StartRoutine_01Proc(void);
static void StopRoutine_02Proc(void);
static void RequestRoutineResults_03Proc(void);

static void Service34Handle(void);

static void Service35Handle(void);

static void Service36Handle(void);

static void Service37Handle(void);

static void Service3DHandle(void);

static void Service3EHandle(void);
static void TesterPresent_00Proc(void);

static void Service85Handle(void);
static void DTCSettingOn_01Proc(void);
static void DTCSettingOff_02Proc(void);

static void Service87Handle(void);
static void VBTWFBR_01Proc(void);
static void VBTWSBR_02Proc(void);
static void TransitionBaudrate_03Proc(void);

static void SessionSupportAndSecurityAccess_Init(uds_uint8_t support, uds_uint8_t service, uds_uint8_t Numofsubfunction, 
                                                 uds_uint8_t PHYDefaultSession_Security, uds_uint8_t PHYProgramSeesion_Security, uds_uint8_t PHYExtendedSession_Security, 
			                                     uds_uint8_t FUNDefaultSession_Security, uds_uint8_t FUNProgramSeesion_Security, uds_uint8_t FUNExtendedSession_Security);
static void ADDRoutine_Init(uds_uint16_t RoutineIdentifier, uds_uint8_t RestartRoutineSupport, uds_uint8_t StopRoutineSupport, 
                            uds_uint8_t RequestRoutineResultsSupport, uds_uint8_t RoutineControlOptionRecordDataLength, 
							uds_uint8_t RoutineStatusRecordDataLength, RoutineRun RoutineRun_Func);
static void UDS_A_ServiceParameter_Init(void);
static void UDS_A_Data_Indication_Read(void);
static void UDS_A_Data_Confirm_Read(void);
static void UDS_A_DiagnosticService(uds_uint8_t DiagServiceID);
static void UDS_A_Data_Confirm_Proc(void);
static void UDS_A_PositiveResponseDataUnit(uds_uint16_t DataLength);
static void UDS_A_NegativeResponseDataUnit(uds_uint8_t nrc);
static void UDS_A_Response_Proc(NegativeResposeCode NRC_Para, uds_uint16_t DataLength, uds_uint8_t SuppPosiFlag, uds_uint8_t TAtype);

static void Reset_Func(EcuResetType_e ResetType);
static uds_uint32_t SeedCalc01_Func(uds_uint8_t RequestSeed);
static uds_uint32_t KeyCalc01_Func(uds_uint32_t RequestSeed);
static uds_uint32_t SeedCalc03_Func(uds_uint8_t RequestSeed);
static uds_uint32_t KeyCalc03_Func(uds_uint32_t RequestSeed);
static uds_uint8_t *ReadMemoryByAddress_Func(uds_uint8_t *Address, uds_uint16_t Length);
static uds_uint32_t ReadMemorySize_Func(uds_uint8_t *Address, uds_uint16_t Length);
static void DTC_Snapshot_Read_Func(uds_uint8_t SnapShotTag, DTCSnapshot_t TempDTCSnapshot);
static void DTC_Snapshot_Write_Func(uds_uint8_t SnapShotTag);

static void Diagnostic_DTCProc(void);
static void ReadDatabyPeriodicInentifier_Proc(void);
static void AddDTCGroup_Init(uds_uint32_t DTCGroup);
static void AddDTC_Init(uds_uint32_t DTCNumber, DTC_test_func Test_func, DetectCondition EnableDTCDetect_Func, 
						uds_uint8_t DectecTimes, uds_int8_t PrefailedStep, uds_int8_t PrepassedStep,
						uds_int8_t FailedFaultDetectionTimes, uds_int8_t PassedFaultDetectionTimes, 
						uds_uint8_t ConfirmTimes, uds_uint8_t MaxDTCAgingTimes, uds_uint8_t DTCStatusTag, 
						uds_uint8_t SnapShotTag);
static void AddDTCSnapShot_Init(uds_uint8_t recordNumber, uds_uint16_t ID, uds_uint8_t* data, uds_uint8_t length, uds_uint8_t StartNum);
static void AddDID_Init(uds_uint16_t ID, uds_uint8_t* data, uds_uint8_t length, uds_uint8_t DynamicalIdentifierBit, 
                        DataIdentifierType_e DataIdentifierType, uds_uint8_t DataIdentifierTag, uds_uint8_t FactorySupport);
static void AddSourceDataRecord_Init(uds_uint16_t ID, uds_uint8_t Position, uds_uint8_t* data, uds_uint8_t length);
static uds_uint8_t SearchDTCGroup_Func(uds_uint32_t DTCGroup);
static DTCNode* SearchDTCNode_Func(uds_uint32_t DtcCode);
static void DTCStatusAvailabilityMask_Init(uds_uint8_t DTCAvailabilityMask);
static void FaultAgingCounterRecordNumber_Init(uds_uint8_t RecordNumer);
static void FaultAgedCounterRecordNumber_Init(uds_uint8_t RecordNumer);
static void FaultOccurenceCounterRecordNumber_Init(uds_uint8_t RecordNumer);
static void FaultPendingCounterRecordNumber_Init(uds_uint8_t RecordNumer);
static void ClearDynamicalDataIdentifier_Func(uds_uint16_t DynamicalDID);
static uds_uint8_t Check_ECU_Edian (void);
static void BigLittleSwitch_Init(void);
static ReadDatabyInentifier_t* SearchDID_Func(uds_uint16_t DID);
static void IO_ControlParameter_Test(void);
static void UDS_A_Data_Save(UDS_EE_Tag_e tag, uds_uint8_t *buf, uds_uint8_t size);
static void UDS_A_Data_Load(UDS_EE_Tag_e tag, uds_uint8_t *buf, uds_uint8_t size, uds_uint8_t *defaultValue);
static TransitionWithFixedParameter_e SearchFixPara_Func(uds_uint8_t LinkControlModeIdentifier);
static uds_uint8_t SearchSpecificPara_Func(uds_uint32_t modeParameter);
static Routine_t* SearchRoutineID_Func(uds_uint16_t RoutineID);
static RoutineProcessStatus_e RoutineRun_01Func(RoutineControlType_e RoutineControlType);
static RoutineProcessStatus_e RoutineRun_02Func(RoutineControlType_e RoutineControlType);
static RoutineProcessStatus_e RoutineRun_03Func(RoutineControlType_e RoutineControlType);
static uds_uint8_t ProgramWriteData(uds_uint8_t* address, uds_uint8_t* data, uds_uint16_t length);

Initial_SendData_t Initial_SendData_Table[Data_Number] = {
	{(uds_uint8_t *)&VehicleSpeed, 1},
	{(uds_uint8_t *)&EngineRPM, 2},
	{(uds_uint8_t *)&Voltage, 2},
	{(uds_uint8_t *)&EngineRPM, 2}
};

Switch_SendData_t Switch_SendData_Table[Data_Number];

DiagService DiagnosticServiceTable[NUMBEROFSERVICE] = {
 	{ UDS_FALSE,	DIAGNOSTICSESSIONCONTROL, 		2, 4, LEVEL_ZERO,		LEVEL_ZERO,		 LEVEL_ZERO,	  LEVEL_ZERO,		 LEVEL_ZERO,		LEVEL_ZERO,		 Service10Handle },
 	{ UDS_FALSE,	ECURESET, 						2, 3, LEVEL_UNSUPPORT,	LEVEL_ZERO,		 LEVEL_ZERO,	  LEVEL_UNSUPPORT,	 LEVEL_ZERO,		LEVEL_ZERO,		 Service11Handle },
 	{ UDS_FALSE,	CLEARDIAGNOSTICINFORMATION,		4, 0, LEVEL_ZERO,		LEVEL_UNSUPPORT, LEVEL_ZERO, 	  LEVEL_ZERO,		 LEVEL_UNSUPPORT,	LEVEL_ZERO,		 Service14Handle },
 	{ UDS_FALSE,	READDTCINFORMATION, 			2, 7, LEVEL_ZERO,		LEVEL_UNSUPPORT, LEVEL_ZERO,	  LEVEL_ZERO,	 	 LEVEL_UNSUPPORT,	LEVEL_ZERO, 	 Service19Handle },
 	{ UDS_FALSE,	READDATABYIDENTIFIER, 			3, 0, LEVEL_ZERO,		LEVEL_ZERO, 	 LEVEL_ZERO,	  LEVEL_ZERO,	 	 LEVEL_ZERO,		LEVEL_ZERO, 	 Service22Handle },
 	{ UDS_FALSE,	READMEMORYBYADDRESS, 			4, 0, LEVEL_ONE,		LEVEL_UNSUPPORT, LEVEL_ONE,	 	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service23Handle },
 	{ UDS_FALSE,	SECURITYACCESS, 				2, 4, LEVEL_UNSUPPORT,	LEVEL_ZERO,		 LEVEL_ZERO,	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service27Handle },
 	{ UDS_FALSE,	COMMUNICATIONCONTROL, 			3, 4, LEVEL_UNSUPPORT,	LEVEL_ZERO,		 LEVEL_ZERO,	  LEVEL_UNSUPPORT,	 LEVEL_ZERO,		LEVEL_ZERO, 	 Service28Handle },
 	{ UDS_FALSE,	READDATABYPERIODICIDENTIFIER, 	2, 4, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, LEVEL_ONE,	 	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service2AHandle },
 	{ UDS_FALSE,	DYNAMICALLYDEFINEDATAIDENTIFIER,4, 3, LEVEL_UNSUPPORT,  LEVEL_UNSUPPORT, LEVEL_ZERO, 	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service2CHandle },
 	{ UDS_FALSE,	WRITEDATABYIDENTIFIER,			4, 0, LEVEL_UNSUPPORT,  LEVEL_ONE, 		 LEVEL_ONE, 	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service2EHandle },
 	{ UDS_FALSE,	INPUTOUTPUTCONTROLBYIDENTIFIER, 4, 0, LEVEL_UNSUPPORT,  LEVEL_UNSUPPORT, LEVEL_ONE, 	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service2FHandle },
 	{ UDS_FALSE,	ROUTINECONTROL, 				4, 3, LEVEL_UNSUPPORT,  LEVEL_ONE, 		 LEVEL_ONE, 	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service31Handle }, 	
	{ UDS_FALSE,	REQUESTDOWNLOAD, 				5, 0, LEVEL_UNSUPPORT,	LEVEL_ONE,		 LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service34Handle },
	{ UDS_FALSE,	REQUESTUPLOAD, 					5, 0, LEVEL_UNSUPPORT,	LEVEL_ONE,		 LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service35Handle },
	{ UDS_FALSE,	TRANSFERDATA, 					2, 0, LEVEL_UNSUPPORT,	LEVEL_ONE,		 LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service36Handle },
	{ UDS_FALSE,	REQUESTTRANSFEREXIT, 			1, 0, LEVEL_UNSUPPORT,	LEVEL_ONE,		 LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service37Handle },
	{ UDS_FALSE,	WRITEMEMORYBYADDRESS, 			5, 0, LEVEL_UNSUPPORT,  LEVEL_UNSUPPORT, LEVEL_ONE, 	  LEVEL_UNSUPPORT,	 LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT, Service3DHandle },
 	{ UDS_FALSE,	TESTERPRESENT, 					2, 1, LEVEL_ZERO,		LEVEL_ZERO,		 LEVEL_ZERO,	  LEVEL_ZERO,		 LEVEL_ZERO,		LEVEL_ZERO,		 Service3EHandle },
 	{ UDS_FALSE,	CONTROLDTCSETTING, 				2, 2, LEVEL_UNSUPPORT,	LEVEL_ZERO,		 LEVEL_ZERO,	  LEVEL_UNSUPPORT,	 LEVEL_ZERO,		LEVEL_ZERO,		 Service85Handle },
 	{ UDS_FALSE,	LINKCONTROL, 					2, 3, LEVEL_UNSUPPORT,	LEVEL_ONE,		 LEVEL_ONE,	  	  LEVEL_UNSUPPORT,	 LEVEL_ONE,			LEVEL_ONE,		 Service87Handle },
};
Subfunction_t Service10Table[] = {
	{ DefaultSessionType, DefaultSession_01Proc },
	{ ProgramingSessionType, ProgramingSession_02Proc },
	{ ExtendSessionType, ExtendSession_03Proc },
	{ FactorySeesionType, FactorySeesion_04Proc }
};

Subfunction_t Service11Table[] = {
	{ HARD_RESET, HardReset_01Proc },
	{ KEY_OFF_ON_RESET, KeyOnOffReset_02Proc },
	{ SOFT_RESET, SoftReset_03Proc }
};

Subfunction_t Service19Table[] = {
	{ reportNumberOfDTCByStatusMask, ReportNumberOfDTCByStatusMask_01Proc },
	{ reportDTCByStatusMask, ReportDTCByStatusMask_02Proc },
	{ reportDTCSnapshotIdentification, ReportDTCSnapshotID_03Proc },
	{ reportDTCSnapshotRecordByDTCNumber, ReportDTCSnapshotRecord_04Proc },
	{ reportDTCStoredDataByRecordNumber, ReportDTCStoredData_05Proc },
	{ reportDTCExtDataRecordByDTCNumber, ReportDTCExtDataRecord_06Proc },
	{ reportSupportedDTC, ReportSupportedDTC_0AProc }
};
	
Subfunction_t Service27Table[] = {
	{ SEED01, SecurityAccessSeed_01Proc },
	{ KEY02, SecurityAccessKey_02Proc },
	{ SEED03, SecurityAccessSeed_03Proc },
	{ KEY04, SecurityAccessKey_04Proc }
};

Subfunction_t Service28Table[] = {
	{ ERXTX, EnableRxAndTx_00Proc },
	{ ERXDTX, EnableRxAndDisableTx_01Proc },
	{ DRXETX, DisableRxAndEnableTx_02Proc },
	{ DRXTX, DisableRxAndTx_03Proc }
};

Subfunction_t Service2ATable[] = {
	{ SendAtSlowRate, SendAtSlowRate_01Proc },
	{ SendAtMediumRate, SendAtMediumRate_02Proc },
	{ SendAtFastRate, SendAtFastRate_03Proc },
	{ StopSending, StopSending_04Proc }
};

Subfunction_t Service2CTable[] = {
	{ DefineByIdentifier, DefineByIdentifier_01Proc },
	{ DefineByMemoryAddress, DefineByMemoryAddress_02Proc },
	{ ClearDynamicallyDefinedDataIdentifier, ClearDynamicallyDefinedDataIdentifier_03Proc }
};

Subfunction_t Service31Table[] = {
	{ StartRoutine, StartRoutine_01Proc },
	{ StopRoutine, StopRoutine_02Proc },
	{ RequestRoutineResults, RequestRoutineResults_03Proc }
};

Subfunction_t Service3ETable[] = {
	{ 0x00, TesterPresent_00Proc }
};

Subfunction_t Service85Table[] = {
	{ DTCSetting_on, DTCSettingOn_01Proc },
	{ DTCSetting_off, DTCSettingOff_02Proc }
};

Subfunction_t Service87Table[] = {
	{ VerifyBaudrateTransitionWithFixedBaudrate, VBTWFBR_01Proc },
	{ VerifyBaudrateTransitionWithSpecificBaudrate, VBTWSBR_02Proc },
	{ TransitionBaudrate, TransitionBaudrate_03Proc }
};

const uds_uint32_t TransitionWithFixedParameterTable[FixPara_Number][2] = {
	{ 0x01, 1000000 },
	{ 0x02, 500000 },
	{ 0x03, 250000 },
	{ 0x04, 125000 },
	{ 0x05, 57600 }
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
	uds_int8_t ret = 1;

	if(!intf){
		return ret;
	}

	if(intf->Network.can_direct_send){
		_UDS_A_send_frame_direct = intf->Network.can_direct_send;
	}else{
		_UDS_A_send_frame_direct = 0;
		ret |= -1;
	}
	if(intf->Session.service_get){
		_UDS_A_service_get = intf->Session.service_get;
	}else{
		_UDS_A_service_get = 0;
		ret |= -1;
	}
	if(intf->Session.USData_request){
		_UDS_A_service_process_USData_request = intf->Session.USData_request;
	}else{
		_UDS_A_service_process_USData_request = 0;
		ret |= -1;
	}
	if(intf->Session.ParaChange_request){
		_UDS_A_service_process_ParaChange_request = intf->Session.ParaChange_request;
	}else{
		_UDS_A_service_process_ParaChange_request = 0;
		ret |= -1;
	}
	if(intf->Session.time_value_get){
		_UDS_A_timer_value_get = intf->Session.time_value_get;
	}else{
		_UDS_A_timer_value_get = 0;
		ret |= -1;
	}
	if(intf->Session.time_status_get){
		_UDS_A_timer_status_get = intf->Session.time_status_get;
	}else{
		_UDS_A_timer_status_get = 0;
		ret |= -1;
	}
	if(intf->Session.time_ctl_stop){
		_UDS_A_timer_ctl_stop = intf->Session.time_ctl_stop;
	}else{
		_UDS_A_timer_ctl_stop = 0;
		ret |= -1;
	}
	if(intf->Session.time_ctl_run){
		_UDS_A_timer_ctl_run = intf->Session.time_ctl_run;
	}else{
		_UDS_A_timer_ctl_run = 0;
		ret |= -1;
	}
	if(intf->Session.time_ctl_restart){
		_UDS_A_timer_ctl_restart = intf->Session.time_ctl_restart;
	}else{
		_UDS_A_timer_ctl_restart = 0;
		ret |= -1;
	}
	if(intf->Session.time_ctl_reset){
		_UDS_A_timer_ctl_reset = intf->Session.time_ctl_reset;
	}else{
		_UDS_A_timer_ctl_reset = 0;
		ret |= -1;
	}
	if(intf->Eeprom.read){
		_UDS_A_eeprom_read = intf->Eeprom.read;
	}else{
		_UDS_A_eeprom_read = 0;
		ret |= -1;
	}
	if(intf->Eeprom.write){
		_UDS_A_eeprom_write = intf->Eeprom.write;
	}else{
		_UDS_A_eeprom_write = 0;
		ret |= -1;
	}

	return ret;
}
//App get the lower layer service
static uds_uint8_t UDS_A_service_get(UDS_N_Services_t *res,uds_uint8_t *buf)
{
	uds_int8_t ret = -1;

	if(_UDS_A_service_get){
		ret = _UDS_A_service_get(res,buf);
	}

	return ret;
}
//App layer issue a USData.request service
static uds_int8_t UDS_A_service_process_USData_request(void *pdata)
{
	uds_int8_t ret = -1;

	if(_UDS_A_service_process_USData_request){
		ret = _UDS_A_service_process_USData_request(pdata);
	}

	return ret;
}

//App layer issue a ParaChange.request service
static uds_int8_t UDS_A_service_process_ParaChange_request(void *pdata)
{
	uds_int8_t ret = -1;

	if(_UDS_A_service_process_ParaChange_request){
		ret = _UDS_A_service_process_ParaChange_request(pdata);
	}

	return ret;
}

//App get the session layer timer status
static uds_int8_t UDS_A_timer_status_get(UDS_S_Time_Name_e time)
{
	uds_int8_t ret = -1;

	if(_UDS_A_timer_status_get){
		ret = _UDS_A_timer_status_get(time);
	}

	return ret;
}

static uds_int8_t UDS_A_timer_ctl_stop(UDS_S_Time_Name_e time)
{
	uds_int8_t ret = -1;

	if(_UDS_A_timer_ctl_stop){
		ret = _UDS_A_timer_ctl_stop(time);
	}

	return ret;
}

static uds_int8_t UDS_A_timer_ctl_run(UDS_S_Time_Name_e time)
{
	uds_int8_t ret = -1;

	if(_UDS_A_timer_ctl_run){
		ret = _UDS_A_timer_ctl_run(time);
	}

	return ret;
}

static uds_int8_t UDS_A_timer_ctl_restart(UDS_S_Time_Name_e time)
{
	uds_int8_t ret = -1;

	if(_UDS_A_timer_ctl_restart){
		ret = _UDS_A_timer_ctl_restart(time);
	}

	return ret;
}

static uds_int8_t UDS_A_timer_ctl_reset(UDS_S_Time_Name_e time)
{
	uds_int8_t ret = -1;

	if(_UDS_A_timer_ctl_reset){
		ret = _UDS_A_timer_ctl_reset(time);
	}

	return ret;
}

static uds_uint16_t UDS_A_timer_value_get(UDS_S_Time_Name_e time)
{
	uds_uint16_t ret = 0xFFFF;

	if(_UDS_A_timer_value_get){
		ret = _UDS_A_timer_value_get(time);
	}

	return ret;
}


//App eeprom
static uds_int8_t UDS_A_eeprom_read(UDS_EE_Tag_e tag,uds_uint8_t *buf)
{
	uds_int8_t ret = -1;

	if(_UDS_A_eeprom_read){
		ret = _UDS_A_eeprom_read(tag,buf);
	}

	return ret;
}

static uds_int8_t UDS_A_eeprom_write(UDS_EE_Tag_e tag,uds_uint8_t *buf)
{
	uds_int8_t ret = -1;

	if(_UDS_A_eeprom_write){
		ret = _UDS_A_eeprom_write(tag,buf);
	}

	return ret;
}


//send frame directly
static uds_int8_t UDS_A_send_frame_direct(uds_uint32_t id,uds_uint8_t length,uds_uint8_t *data)
{
	uds_int8_t ret = -1;

	if(_UDS_A_send_frame_direct){
		ret = _UDS_A_send_frame_direct(id,length,data);
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

static void UDS_A_Data_Save(UDS_EE_Tag_e tag, uds_uint8_t *buf, uds_uint8_t size) {
	uds_uint8_t i, buffer[MAXDATALENGTH];
	uds_uint32_t sumofData = 0;
	for (i = 0; i < size; i++) {
		sumofData += *(buf + i);
		buffer[i] = *(buf + i);
	}
	buffer[size] = (uds_uint8_t)0xff - (uds_uint8_t)sumofData;
	
	UDS_A_eeprom_write(tag, buffer);
}

static void UDS_A_Data_Load(UDS_EE_Tag_e tag, uds_uint8_t *buf, uds_uint8_t size, uds_uint8_t *defaultValue) {
	uds_uint8_t i, buffer[MAXDATALENGTH];
	uds_uint32_t sumofData = 0;
	UDS_A_eeprom_read(tag, buffer);
	for (i = 0; i < size; i++) {
		sumofData += *(buffer + i);
	}

	if (0xff == ((uds_uint8_t)sumofData + (uds_uint8_t)*(buffer + size))) {
		memcpy(buf, buffer, size);
	}
	else {
		memcpy(buf, defaultValue, size);
	}
}

static uds_uint8_t Check_ECU_Edian (void) {
	union {
		uds_uint16_t Parameter;
		uds_uint8_t Array[2];
	}Check;

	Check.Parameter = 0x1122;

	return (0x11 == Check.Array[0]); //0-小端，1-大端
}

static void BigLittleSwitch_Init(void) {
	for (uds_uint8_t i = 0; i < Data_Number; i++) {
		if (Check_ECU_Edian()) { //大端
			Switch_SendData_Table[i].DataArray[0] = *Initial_SendData_Table[i].DataPointer;
			Switch_SendData_Table[i].DataArray[1] = *(Initial_SendData_Table[i].DataPointer + 1);
			Switch_SendData_Table[i].DataArray[2] = *(Initial_SendData_Table[i].DataPointer + 2);
			Switch_SendData_Table[i].DataArray[3] = *(Initial_SendData_Table[i].DataPointer + 3);
		}
		else { //小端
			switch(Initial_SendData_Table[i].DataLength) {
				case 1:
					Switch_SendData_Table[i].DataArray[0] = *Initial_SendData_Table[i].DataPointer;
					break;
				case 2:
					Switch_SendData_Table[i].DataArray[0] = *(Initial_SendData_Table[i].DataPointer + 1);
					Switch_SendData_Table[i].DataArray[1] = *Initial_SendData_Table[i].DataPointer;
					break;
				case 4:
					Switch_SendData_Table[i].DataArray[0] = *(Initial_SendData_Table[i].DataPointer + 3);
					Switch_SendData_Table[i].DataArray[1] = *(Initial_SendData_Table[i].DataPointer + 2);
					Switch_SendData_Table[i].DataArray[2] = *(Initial_SendData_Table[i].DataPointer + 1);
					Switch_SendData_Table[i].DataArray[3] = *Initial_SendData_Table[i].DataPointer;
					break;
				default:
					Switch_SendData_Table[i].DataArray[0] = *Initial_SendData_Table[i].DataPointer;
			}		
		}
		//Switch_SendData_Table[i].DataLength = Initial_SendData_Table[i].DataLength;
	}
}

static void DiagnosticState_Init(DiagnosticSession_e SessionMode, SecurityLevel_e SecurityLevel, DiagnosticProcStatus_e DiagnosticProcessStatus, 
										SessionType_e SessionType, IndicationType_e IndicationType, ConfirmType_e ConfirmType, 
										ResponseType_e ResponseType, SuppressPositiveType_e SuppressPositiveType) {
	SystemState.SessionMode = SessionMode;
	SystemState.SecurityLevel = SecurityLevel;
	SystemState.DiagnosticProcessStatus = DiagnosticProcessStatus;
	SystemState.SessionType = SessionType;
	SystemState.IndicationType = IndicationType;
	SystemState.ConfirmType = ConfirmType;
	SystemState.ResponseType = ResponseType;
	SystemState.SuppressPositiveType = SuppressPositiveType;
}

static void SecurityAccess_Init(void) {
	SecurityAccessPara.AccessErrorCountTag = SECURITYACCESS_ERROR_COUNTER_TAG;
	UDS_A_Data_Load(SecurityAccessPara.AccessErrorCountTag, &(SecurityAccessPara.KeyVerifyErrorCountFromEEPROM), 1, DataLoad_DefaultValue);
	//UDS_A_eeprom_read(SecurityAccessPara.AccessErrorCountTag, &(SecurityAccessPara.KeyVerifyErrorCountFromEEPROM));
	if (SecurityAccessPara.KeyVerifyErrorCountFromEEPROM > MAXKEYERRORPERMITCOUNT) {
		SecurityAccessPara.KeyVerifyErrorCount = 0;
		UDS_A_Data_Save(SecurityAccessPara.AccessErrorCountTag, &(SecurityAccessPara.KeyVerifyErrorCount), 1);
		//UDS_A_eeprom_write(SecurityAccessPara.AccessErrorCountTag, &(SecurityAccessPara.KeyVerifyErrorCount));
	}
	else {
		SecurityAccessPara.KeyVerifyErrorCount = SecurityAccessPara.KeyVerifyErrorCountFromEEPROM;
	}
	
	if (MAXKEYERRORPERMITCOUNT == SecurityAccessPara.KeyVerifyErrorCount) {
		UDS_A_timer_ctl_run(S_TIME_SECURITYDELAY_SERVER); //开启延时定时器
		SecurityAccessPara.UnlockStep = WAIT_DELAY;
	}
}

static void DTCStatusAvailabilityMask_Init(uds_uint8_t DTCAvailabilityMask) {
	DTCStatusAvailabilityMask = DTCAvailabilityMask;
}

static void FaultOccurenceCounterRecordNumber_Init(uds_uint8_t RecordNumer) {
	FaultOccurenceCounterRecord = RecordNumer;
}

static void FaultPendingCounterRecordNumber_Init(uds_uint8_t RecordNumer) {
	FaultPendingCounterRecord = RecordNumer;
}

static void FaultAgingCounterRecordNumber_Init(uds_uint8_t RecordNumer) {
	FaultAgingCounterRecord = RecordNumer;
}

static void FaultAgedCounterRecordNumber_Init(uds_uint8_t RecordNumer) {
	FaultAgedCounterRecord = RecordNumer;
}

/*源数据配置初始化-用于0X2C_0x01*/
static void AddSourceDataRecord_Init(uds_uint16_t ID, uds_uint8_t Position, uds_uint8_t* data, uds_uint8_t length) {
	if(NumberofSourceDataRecord < MAX_SOURCEDATARECORD_NUMBER) {
		SourceDataRecordTable[NumberofSourceDataRecord].SourceDataIdentifier = ID;
		SourceDataRecordTable[NumberofSourceDataRecord].PositionInSourceDataRecord = Position;
		SourceDataRecordTable[NumberofSourceDataRecord].DataPointer = data;
		SourceDataRecordTable[NumberofSourceDataRecord].MemorySize = length;
		NumberofSourceDataRecord++;
	}
}

/*可读DID配置初始化*/
static void AddDID_Init(uds_uint16_t ID, uds_uint8_t* data, uds_uint8_t length, uds_uint8_t DynamicalIdentifierBit, 
								DataIdentifierType_e DataIdentifierType, uds_uint8_t DataIdentifierTag, uds_uint8_t FactorySupport) {
	if(NumberofReadDID < NUMOFREADDID) {
		ReadDIDTable[NumberofReadDID].Support = UDS_FALSE; //PDID默认不支持
		ReadDIDTable[NumberofReadDID].DataIdentifier = ID;
		ReadDIDTable[NumberofReadDID].DataPointer = data;
		ReadDIDTable[NumberofReadDID].DataLength = length;
		ReadDIDTable[NumberofReadDID].TransmitTime = 0;
		ReadDIDTable[NumberofReadDID].TransmitCounter = 0;
		ReadDIDTable[NumberofReadDID].DynamicalIdentifierBit = DynamicalIdentifierBit;
		ReadDIDTable[NumberofReadDID].DataIdentifierType = DataIdentifierType;
		ReadDIDTable[NumberofReadDID].DataIdentifierTag = DataIdentifierTag;
		ReadDIDTable[NumberofReadDID].FactoryConfigSupport = FactorySupport;
		NumberofReadDID++;
	}
}

/*DTCSnapshot初始化*/
static void AddDTCSnapShot_Init(uds_uint8_t recordNumber, uds_uint16_t ID, uds_uint8_t* data, uds_uint8_t length, uds_uint8_t StartNum) {
	if(NumberofDTCSnapshot < MAX_RECORDNUMBER_NUMBER) {
		DTCSnapshotTable[NumberofDTCSnapshot].SnapshotRecordNumber = recordNumber;
		DTCSnapshotTable[NumberofDTCSnapshot].SnapshotID = ID;
		DTCSnapshotTable[NumberofDTCSnapshot].SnapshotData = data;
		DTCSnapshotTable[NumberofDTCSnapshot].DataLength = length;
		DTCSnapshotTable[NumberofDTCSnapshot].StartNum = StartNum;
		NumberofDTCSnapshot ++;
	}
}

/*DTCGroup初始化*/
static void AddDTCGroup_Init(uds_uint32_t DTCGroup) {
	if(NumberofDTCGroup < MAX_DTCGROUP_NUMBER) {
		DTCGroupTable[NumberofDTCGroup].GroupID = DTCGroup;
		NumberofDTCGroup ++;
	}
}

uds_int8_t UDS_A_AddDTC_Init(DTCNode_Config *dtc_iterm)
{
	uds_int8_t ret = -1;
	DTCNode_Config *dtc = dtc_iterm;
	if(!dtc){
		return ret;
	}

	if(MAX_DTC_NUMBER > NumberofDTC){
		AddDTC_Init(dtc->DTCNumber,dtc->Test_func,dtc->EnableDTCDetect,dtc->DetecTimes,dtc->PrefailedStep,
					dtc->PrepassedStep,dtc->FailedFaultDetectionTimes,dtc->PassedFaultDetectionTimes,
					dtc->Confirmationthreshold,dtc->MaxDTCAgingTimes,dtc->DTCStatusTag,dtc->SnapShotTag);
		ret = 1;
	}else{
		ret = 0;
	}

	return ret;
}
/*DTC配置初始化*/
static void AddDTC_Init(uds_uint32_t DTCNumber, DTC_test_func Test_func, DetectCondition EnableDTCDetect_Func, 
						uds_uint8_t DectecTimes, uds_int8_t PrefailedStep, uds_int8_t PrepassedStep,
						uds_int8_t FailedFaultDetectionTimes, uds_int8_t PassedFaultDetectionTimes, 
						uds_uint8_t ConfirmTimes, uds_uint8_t MaxDTCAgingTimes, uds_uint8_t DTCStatusTag, 
						uds_uint8_t SnapShotTag)
{
	if (NumberofDTC < MAX_DTC_NUMBER) {
		DTCTable[NumberofDTC].Config.DTCNumber = DTCNumber;
		DTCTable[NumberofDTC].Config.Test_func = Test_func;
		DTCTable[NumberofDTC].Config.EnableDTCDetect = EnableDTCDetect_Func;
		DTCTable[NumberofDTC].Config.DetecTimes = DectecTimes;		
		DTCTable[NumberofDTC].Config.PrefailedStep = PrefailedStep;
		DTCTable[NumberofDTC].Config.PrepassedStep = PrepassedStep;
		DTCTable[NumberofDTC].Config.FailedFaultDetectionTimes = FailedFaultDetectionTimes;
		DTCTable[NumberofDTC].Config.PassedFaultDetectionTimes = PassedFaultDetectionTimes;
		DTCTable[NumberofDTC].Config.Confirmationthreshold = ConfirmTimes - 1;
		DTCTable[NumberofDTC].Config.MaxDTCAgingTimes = MaxDTCAgingTimes;
		DTCTable[NumberofDTC].Config.DTCStatusTag = DTCStatusTag;
		DTCTable[NumberofDTC].Config.SnapShotTag = SnapShotTag;
		DTCTable[NumberofDTC].Sts.DetecCycleCounter = 0;
		DTCTable[NumberofDTC].Sts.FaultDetectionCounter = 0;
		DTCTable[NumberofDTC].Sts.ConfirmationCounter = 0;
		DTCTable[NumberofDTC].Sts.FaultOccurenceCounter = 0;
		DTCTable[NumberofDTC].Sts.FaultPendingCounter = 0;
		DTCTable[NumberofDTC].Sts.FaultAgedCounter = 0;
		DTCTable[NumberofDTC].Sts.FaultAgingCounter = 0;

		UDS_A_Data_Load(DTCTable[NumberofDTC].Config.DTCStatusTag, &(DTCTable[NumberofDTC].Sts.DTCstatus.StatusOfDTC), MAX_DTC_EEPROM_BYTES, DataLoad_DefaultValue);
		//UDS_A_eeprom_read(DTCTable[NumberofDTC].DTCStatusTag, &(DTCTable[NumberofDTC].DTCstatus.StatusOfDTC)); //读取DTCstatus
	#if 1
		DTCTable[NumberofDTC].Sts.DTCstatus.StatusOfDTC = DTCTable[NumberofDTC].Sts.DTCstatus.StatusOfDTC & DTCStatusAvailabilityMask;
	#else
		DTCTable[NumberofDTC].DTCstatus.StatusOfDTC_t.testFailed = 0; //bit0 default 0
		DTCTable[NumberofDTC].DTCstatus.StatusOfDTC_t.testFailedThisOperationCycle = 0; //bit1 default 0
		DTCTable[NumberofDTC].DTCstatus.StatusOfDTC_t.testNotCompletedThisOperationCycle = 1; //bit6 default 1
	#endif

		NumberofDTC++;
	}
	else {
		
	}
}

/*服务会话支持与安全等级初始化*/
static void SessionSupportAndSecurityAccess_Init(uds_uint8_t support, uds_uint8_t service, uds_uint8_t Numofsubfunction, uds_uint8_t PHYDefaultSession_Security, 
															uds_uint8_t PHYProgramSeesion_Security, uds_uint8_t PHYExtendedSession_Security, uds_uint8_t FUNDefaultSession_Security, 
															uds_uint8_t FUNProgramSeesion_Security, uds_uint8_t FUNExtendedSession_Security) {
	uds_uint8_t i;
	for (i = 0; i < NUMBEROFSERVICE; i++) {
		if (DiagnosticServiceTable[i].ServiceID == service) {
			DiagnosticServiceTable[i].Support = support;
			DiagnosticServiceTable[i].NumOfSubfunc = Numofsubfunction;
			DiagnosticServiceTable[i].PHYDefaultSession_Security = PHYDefaultSession_Security;
			DiagnosticServiceTable[i].PHYProgramSeesion_Security = PHYProgramSeesion_Security;
			DiagnosticServiceTable[i].PHYExtendedSession_Security = PHYExtendedSession_Security;
			DiagnosticServiceTable[i].FUNDefaultSession_Security = FUNDefaultSession_Security;
			DiagnosticServiceTable[i].FUNProgramSeesion_Security = FUNProgramSeesion_Security;
			DiagnosticServiceTable[i].FUNExtendedSession_Security = FUNExtendedSession_Security;
		}
	}
}

static void ADDRoutine_Init(uds_uint16_t RoutineIdentifier, uds_uint8_t RestartRoutineSupport, uds_uint8_t StopRoutineSupport, 
                            uds_uint8_t RequestRoutineResultsSupport, uds_uint8_t RoutineControlOptionRecordDataLength, 
							uds_uint8_t RoutineStatusRecordDataLength, RoutineRun RoutineRun_Func) {
	if(NumberofRoutine < MAX_ROUTINE_NUMBER) {
		RoutineTable[NumberofRoutine].RoutineIdentifier = RoutineIdentifier;
		RoutineTable[NumberofRoutine].RestartRoutineSupport = RestartRoutineSupport;
		RoutineTable[NumberofRoutine].StopRoutineSupport = StopRoutineSupport;
		RoutineTable[NumberofRoutine].RequestRoutineResultsSupport = RequestRoutineResultsSupport;
		RoutineTable[NumberofRoutine].RoutineControlOptionRecordDataLength = RoutineControlOptionRecordDataLength;
		RoutineTable[NumberofRoutine].RoutineStatusRecordDataLength = RoutineStatusRecordDataLength;
		RoutineTable[NumberofRoutine].RoutineRun_Func = RoutineRun_Func;
		RoutineTable[NumberofRoutine].RoutineStatus = Routine_Init;
		NumberofRoutine ++;
	}
}

void UDS_A_Diagnostic_Init(void) {

	DiagnosticState_Init(DefaultSession, LEVEL_ZERO, DiagProcessInit, Default_Session, Init_Indication, Init_Confirm, Init_Response, NotSuppressPositive);
	SecurityAccess_Init();

	/*诊断服务配置*/
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x10, 4, LEVEL_ZERO,		 LEVEL_ZERO,		LEVEL_ZERO, 	LEVEL_ZERO,		 LEVEL_ZERO,		LEVEL_ZERO);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x11, 3, LEVEL_UNSUPPORT, LEVEL_ZERO,		LEVEL_ZERO, 	LEVEL_UNSUPPORT, LEVEL_ZERO,		LEVEL_ZERO);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x14, 0, LEVEL_ZERO,		 LEVEL_UNSUPPORT,	LEVEL_ZERO,		LEVEL_ZERO,		 LEVEL_UNSUPPORT,	LEVEL_ZERO);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x19, 7, LEVEL_ZERO,		 LEVEL_UNSUPPORT,	LEVEL_ZERO, 	LEVEL_ZERO,	 	 LEVEL_UNSUPPORT,	LEVEL_ZERO);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x22, 0, LEVEL_ZERO,		 LEVEL_ZERO, 	 	LEVEL_ZERO, 	LEVEL_ZERO,	 	 LEVEL_ZERO,		LEVEL_ZERO);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x23, 0, LEVEL_ONE,		 LEVEL_UNSUPPORT,	LEVEL_ONE, 		LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x27, 4, LEVEL_UNSUPPORT, LEVEL_ZERO,		LEVEL_ZERO, 	LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x28, 4, LEVEL_UNSUPPORT, LEVEL_ZERO,		LEVEL_ZERO, 	LEVEL_UNSUPPORT, LEVEL_ZERO,		LEVEL_ZERO);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x2A, 4, LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_ONE, 		LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x2C, 3, LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_ZERO, 	LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x2E, 0, LEVEL_UNSUPPORT, LEVEL_ONE,			LEVEL_ONE, 		LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x2F, 0, LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_ONE, 		LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x31, 3, LEVEL_UNSUPPORT, LEVEL_ONE,			LEVEL_ONE, 		LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x34, 0, LEVEL_UNSUPPORT, LEVEL_ONE,			LEVEL_UNSUPPORT,LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x35, 0, LEVEL_UNSUPPORT, LEVEL_ONE,			LEVEL_UNSUPPORT,LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x36, 0, LEVEL_UNSUPPORT, LEVEL_ONE,			LEVEL_UNSUPPORT,LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x37, 0, LEVEL_UNSUPPORT, LEVEL_ONE,			LEVEL_UNSUPPORT,LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x3D, 0, LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_ONE, 		LEVEL_UNSUPPORT, LEVEL_UNSUPPORT,	LEVEL_UNSUPPORT);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x3E, 1, LEVEL_ZERO,		 LEVEL_ZERO,		LEVEL_ZERO, 	LEVEL_ZERO,		 LEVEL_ZERO,		LEVEL_ZERO);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x85, 2, LEVEL_UNSUPPORT, LEVEL_ZERO,		LEVEL_ZERO,		LEVEL_UNSUPPORT, LEVEL_ZERO,		LEVEL_ZERO);
	SessionSupportAndSecurityAccess_Init(UDS_TRUE, 0x87, 3, LEVEL_UNSUPPORT, LEVEL_ONE,			LEVEL_ONE,		LEVEL_UNSUPPORT, LEVEL_ONE,			LEVEL_ONE);

	/*ECU大小端判断,响应数据转换为大端存储*/
	//BigLittleSwitch_Init();
	
	/*DTC Parameters*/
	//NumberofDTC = 0; //DTC数
	//NumberofDTCGroup = 0; //DTCGroup数
	//NumberofDTCSnapshot = 0; //DTCSnapshot数
	//DTCStatusAvailabilityMask = 0x7f; //DTC状态可用掩码
	//DTCStatusStoreMask = 0x3c; //需要保存的DTC状态掩码
	DTCStatusAvailabilityMask_Init(0x7f);	

	/*DTCGroup*/
	AddDTCGroup_Init(0x00ffffff);

	/*DTCSnapshot-按照Number顺序排列,即将一个Number中的所有DID分配完成，才可进行下一个Number分配*/
	AddDTCSnapShot_Init(DTC_SNAPSHOT_RECORDNUMBER01, 0x9101, Switch_SendData_Table[Data_VehicleSpeed].DataArray, DTC_SNAPSHOT1_EEPROM_BYTES, DTC_SNAPSHOT1_STARTNUM);
	AddDTCSnapShot_Init(DTC_SNAPSHOT_RECORDNUMBER01, 0x9102, Switch_SendData_Table[Data_EngineRPM].DataArray, DTC_SNAPSHOT2_EEPROM_BYTES, DTC_SNAPSHOT2_STARTNUM);
	AddDTCSnapShot_Init(DTC_SNAPSHOT_RECORDNUMBER02, 0x9102, Switch_SendData_Table[Data_Voltage].DataArray, DTC_SNAPSHOT3_EEPROM_BYTES, DTC_SNAPSHOT3_STARTNUM);

	/*DTCExtendData*/
	FaultOccurenceCounterRecordNumber_Init(1);
	FaultPendingCounterRecordNumber_Init(2);
	FaultAgingCounterRecordNumber_Init(3);
	FaultAgedCounterRecordNumber_Init(4);

	/*ReadDataByIdentifier*/
	AddDID_Init(0xA815, ReadDIDArrayA815, 3, UDS_FALSE, ReadOnly, NO_TAG, UDS_FALSE);
	AddDID_Init(0xB128, &ReadDIDArrayB128, 4, UDS_FALSE, ReadOnly, NO_TAG, UDS_FALSE);
	
	/*ReadDataByPeriodicIdentifier*/
	AddDID_Init(0xF202, Switch_SendData_Table[Data_EngineRPM].DataArray, 2, UDS_FALSE, ReadOnly, NO_TAG, UDS_FALSE);
	AddDID_Init(0xF287, Switch_SendData_Table[Data_Voltage].DataArray, 2, UDS_FALSE, ReadOnly, NO_TAG, UDS_FALSE);
	AddDID_Init(0xF2FF, &ReadDIDArrayB128, 3, UDS_FALSE, ReadOnly, NO_TAG, UDS_FALSE);

	/*WriteDataByIdentifier*/
	AddDID_Init(0xA376, &Test_RAM_2E, 1, UDS_FALSE, Write_RAM, NO_TAG, UDS_FALSE);
	AddDID_Init(0xF190, VIN_Number, VIN_NUMBER_BYTES, UDS_FALSE, Write_EEPROM, VIN_NUMNER_TAG, UDS_TRUE); //VIN
	//AddDID_Init(0xB134, Test_Table, 5, UDS_FALSE, Write_EEPROM, VIN_NUMNER_TAG, UDS_FALSE);

	/*InputOutputControlByIdentifier-Test Data*/
	AddDID_Init(INPUTOUTPUTCONTROL_DID0, Switch_SendData_Table[Data_EngineRPM_IO].DataArray, 2, UDS_FALSE, Write_RAM_IO, NO_TAG, UDS_FALSE); //控制单个参数
	AddDID_Init(INPUTOUTPUTCONTROL_DID1, IO_ControlPara.Array, 3, UDS_FALSE, Write_RAM_IO, NO_TAG, UDS_FALSE); //控制多个参数,1byte车速-2byte电压

	/*Static ReadDID*/
	NumberofStaticReadDID = NumberofReadDID; //需放置在ReadDID配置函数之后

	/*SourceDataRecord*/
	AddSourceDataRecord_Init(0x1234, 1, Switch_SendData_Table[Data_Voltage].DataArray, 2);
	AddSourceDataRecord_Init(0x1234, 3, ReadDIDArrayA815, 3);
	AddSourceDataRecord_Init(0x1234, 6, Switch_SendData_Table[Data_VehicleSpeed].DataArray, 1);
	AddSourceDataRecord_Init(0x5678, 1, Switch_SendData_Table[Data_VehicleSpeed].DataArray, 1);
	AddSourceDataRecord_Init(0x5678, 2, Switch_SendData_Table[Data_EngineRPM].DataArray, 2);
	AddSourceDataRecord_Init(0x9ABC, 1, &ReadDIDArrayB128, 4);

	/*RoutineIdentifier*/
	ADDRoutine_Init(0x2568, UDS_TRUE, UDS_TRUE, UDS_TRUE, 0, 0, RoutineRun_01Func);
	ADDRoutine_Init(0x1389, UDS_TRUE, UDS_TRUE, UDS_FALSE, 2, 2, RoutineRun_02Func);
	ADDRoutine_Init(0xA276, UDS_FALSE, UDS_FALSE, UDS_TRUE, 1, 3, RoutineRun_03Func);
}

static void UDS_A_ServiceParameter_Init(void) {
	/*服务类型Init*/
	SystemState.SessionType = Default_Session;
	SystemState.IndicationType = Init_Indication;
	SystemState.ConfirmType = Init_Confirm;
	SystemState.ResponseType = Init_Response;
	SystemState.SuppressPositiveType = NotSuppressPositive;

	/*响应参数Init*/
	NRC = PR_00;
	SuppressPosRspMsgIndicationBit = UDS_FALSE;
	ResponseDataLength = 0;

	/*安全访问延时定时器状态监测*/
	if (S_TIME_CNT_STS_TIMEOUT == UDS_A_timer_status_get(S_TIME_SECURITYDELAY_SERVER)) { //延时时间到达,错误计数器减1
		SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
		UDS_A_timer_ctl_reset(S_TIME_SECURITYDELAY_SERVER); //计时器复位至idle状态
		SecurityAccessPara.KeyVerifyErrorCount--;
		UDS_A_Data_Save(SecurityAccessPara.AccessErrorCountTag, &(SecurityAccessPara.KeyVerifyErrorCount), 1);
		//UDS_A_eeprom_write(SecurityAccessPara.AccessErrorCountTag, &(SecurityAccessPara.KeyVerifyErrorCount));
	}
	else {
	}
	
	/*单个变量数据大小端转换*/
	BigLittleSwitch_Init();
	
	/*IO控制参数更新-大小端转换*/
	IO_ControlPara.Array[0] = VehicleSpeed;
	IO_ControlPara.Array[1] = (uds_int8_t)(Voltage >> 8);
	IO_ControlPara.Array[2] = (uds_int8_t)Voltage;
}

/*诊断数据读取及有效性判断*/
void UDS_A_Diagnostic_Proc(void) {
	uds_uint8_t Tmp = UDS_A_service_get((UDS_N_Services_t *)&ServiceDataUnit, &SDUReceiveBuffArray[0]); //读取网络层服务数据单元至缓存区，返回值为数据单元类型

	UDS_A_ServiceParameter_Init();

	/*服务处理*/
	if ((N_USDATA_INDICATION == Tmp) 
		|| (S_TIME_CNT_STS_RUN == UDS_A_timer_status_get(S_TIME_P2X_CAN_SERVER))) { //缓存区为Indication数据或P2*Server定时器开启
		if (S_TIME_CNT_STS_IDLE == UDS_A_timer_status_get(S_TIME_P2X_CAN_SERVER)) { //读取数据
			UDS_A_Data_Indication_Read();
		}
		else {
			/*使用上次的服务数据*/
		}

		/*根据Indication反馈结果处理*/
		if (OK == UDS_A_Indication_SDU.Result) {			
			UDS_A_DiagnosticService(UDS_A_Indication_SDU.A_PCI);
			if (ResponseDataLength > UDS_RECIVE_BUFFER_LENGTH) { //超出Server最大数据发送长度
				NRC = RTL_14;
			}
			
			if ((NRC != PR_00) || (ResponseDataLength > 0) || (1 == SuppressPosRspMsgIndicationBit)) { //fail-safe-非抑制正响应，响应长度为0
				UDS_A_Response_Proc(NRC,
								ResponseDataLength,
								SuppressPosRspMsgIndicationBit, 
								UDS_A_Indication_SDU.TAType);
			}

			/*Indication类型*/
			if (TESTERPRESENT == UDS_A_Indication_SDU.A_PCI) {
				SystemState.IndicationType = TP_Indication;
			}
			else {
				SystemState.IndicationType = Indication;
			}
		}
		else {
			/*忽略错误数据*/
		}
	}
	else if (N_USDATA_CONFIRM == Tmp) { //缓存区为Confirm数据
		UDS_A_Data_Confirm_Read(); //读取数据
		UDS_A_Data_Confirm_Proc(); //流程处理
	}
	else if (N_USDATA_FF_INDICATION == Tmp) { //缓存区为FF_Indication数据
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
	
	/*将应用层进程状态传递至会话层*/
	UDS_A_set_session_time_ctl_Ses_sts(SystemState.SessionType);
	UDS_A_set_session_time_ctl_req(SystemState.ResponseType);
	UDS_A_set_session_time_ctl_con(SystemState.ConfirmType);
	UDS_A_set_session_time_ctl_ind(SystemState.IndicationType);
	UDS_A_set_session_time_ctl_sup(SystemState.SuppressPositiveType);
		
	ReadDatabyPeriodicInentifier_Proc();
}

/*0x2A-读取周期DID处理*/
//service 0x2A was also allowed as USDT (single frame only) in UDS 2005, now it is allowed on UUDT only.
static void ReadDatabyPeriodicInentifier_Proc(void)	{
	if ((ExtendSession == SystemState.SessionMode) && (LEVEL_ONE == SystemState.SecurityLevel)) {
		if (UDS_TRUE == ConfirmPeridDIDSend) {
			for(uds_uint8_t i = 0; i < NumberofReadDID; i++) {
				if (UDS_TRUE == ReadDIDTable[i].Support) {
					if (0 == ReadDIDTable[i].TransmitCounter) {
						ReadDIDTable[i].TransmitCounter = ReadDIDTable[i].TransmitTime;

						/*每个PeriodicDID的不同序号只能在缓存数组中存储一次*/
						if (PeriodicDIDArray.NumofData > 0) {
							for (uds_uint8_t j = 0; j < PeriodicDIDArray.NumofData; j++) {
								if (PeriodicDIDArray.DataIndex[j] == i) {
									break;
								}

								if (j == (PeriodicDIDArray.NumofData - 1)) {
									PeriodicDIDArray.DataIndex[PeriodicDIDArray.NumofData++] = i;
								}
							}
						}
						else {
							PeriodicDIDArray.DataIndex[PeriodicDIDArray.NumofData++] = i;
						}					
					}
					else {
						ReadDIDTable[i].TransmitCounter --;
					}
				}
				else { //去除待发送缓存数组中被禁止发送的数据序号
					for (uds_uint8_t j = 0; j < PeriodicDIDArray.NumofData; j++) {
						if (PeriodicDIDArray.DataIndex[j] == i) {
							for (uds_uint8_t k = j; k < PeriodicDIDArray.NumofData; k++) {
								PeriodicDIDArray.DataIndex[k] = PeriodicDIDArray.DataIndex[k + 1];
							}
							PeriodicDIDArray.NumofData--;
							//break; //optional
						}
					}
				}
			}
			
			if (PeriodicDIDArray.NumofData > 0) {		
				if (0 == DataBufferPutCycle) {
					PeriodicDIDArray.FirstofDataIndex = PeriodicDIDArray.DataIndex[0];

					/*去除发送缓存数组中与首元素PDID相同的数据序号*/
					for (uds_uint8_t i = 0; i < PeriodicDIDArray.NumofData;) {
						if (ReadDIDTable[PeriodicDIDArray.FirstofDataIndex].DataIdentifier == ReadDIDTable[PeriodicDIDArray.DataIndex[i]].DataIdentifier) {
							for (uds_uint8_t k = i; k < PeriodicDIDArray.NumofData; k++) {
								PeriodicDIDArray.DataIndex[k] = PeriodicDIDArray.DataIndex[k + 1];
							}
							PeriodicDIDArray.NumofData--;
						}
						else {
							i++;
						}
					}
					
					SDUSendBuffArray_0X2A[0] = (uds_uint8_t)ReadDIDTable[PeriodicDIDArray.FirstofDataIndex].DataIdentifier;
					ResponseDataLength_0x2A = 1;

					/*将ReadDID数组中与选定PDID，DID相同的对应数据填充至发送数组，并将相应计数器重置*/
					for (uds_uint8_t i = 0; i < NumberofReadDID; i++) {
						if (ReadDIDTable[PeriodicDIDArray.FirstofDataIndex].DataIdentifier == ReadDIDTable[i].DataIdentifier) {
							if (Write_EEPROM == ReadDIDTable[i].DataIdentifierType) {
								UDS_A_Data_Load(ReadDIDTable[i].DataIdentifierTag, SDUSendBuffArray_0X2A + ResponseDataLength_0x2A, ReadDIDTable[i].DataLength, DataLoad_DefaultValue);
							}
							else {
								memcpy(SDUSendBuffArray_0X2A + ResponseDataLength_0x2A, ReadDIDTable[i].DataPointer, ReadDIDTable[i].DataLength);
							}
							ResponseDataLength_0x2A += ReadDIDTable[i].DataLength;
							ReadDIDTable[i].TransmitCounter = ReadDIDTable[i].TransmitTime; //重置计数器
						}
					}
					
					/*此处调用CAN报文发送函数*/
					UDS_A_send_frame_direct(PERIODICDID_SEND_ID, ResponseDataLength_0x2A, SDUSendBuffArray_0X2A);
				
					DataBufferPutCycle = DATAPUTCYCLE;
				}
				else {
					DataBufferPutCycle--;
				}				
			}
		}
		else {
			for(uds_uint8_t i = 0; i < NumberofReadDID; i++) {
				ReadDIDTable[i].TransmitCounter = 0;
			}
			DataBufferPutCycle = DATAPUTCYCLE;

			/*清除发送缓存数组*/
			PeriodicDIDArray.NumofData = 0;
			memset(PeriodicDIDArray.DataIndex, 0, 200);
		}		
	}
	else {
		for(uds_uint8_t i = 0; i < NumberofReadDID; i++) {
			ReadDIDTable[i].Support = UDS_FALSE; //禁止PDID
			ReadDIDTable[i].TransmitTime = 0; //清除发送周期
			ReadDIDTable[i].TransmitCounter = 0;
		}
		DataBufferPutCycle = DATAPUTCYCLE;
		NumofPeriodicDIDRunning = 0;

		/*清除发送缓存数组*/
		PeriodicDIDArray.NumofData = 0;
		memset(PeriodicDIDArray.DataIndex, 0, 200);
	}
}


/*读取缓存区Indication数据*/
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
	UDS_A_Indication_SDU.A_PCI = SDUReceiveBuffArray[0]; //数据单元首元素为诊断服务ID
	for (i = 1; i < UDS_A_Indication_SDU.Length; i++) {
		UDS_A_Indication_SDU.A_SDU[i - 1] = SDUReceiveBuffArray[i];
	}
}

/*读取缓存区Confirm数据*/
static void UDS_A_Data_Confirm_Read(void) {
	UDS_A_Confirm_SDU.Mtype = ServiceDataUnit.Mtype;
	UDS_A_Confirm_SDU.SourceAddr = UDS_A_SOURCEADDRESS; //地址待定
	UDS_A_Confirm_SDU.TargetAddr = UDS_A_TARGETADDRESS; //地址待定
	UDS_A_Confirm_SDU.TAType = ServiceDataUnit.TAType;
#ifdef ADDRESS_EXTENSION_MODE
	UDS_A_Confirm_SDU.A_ExtendAddr = ServiceDataUnit.A_ExtendAddr;
#endif
	UDS_A_Confirm_SDU.Result = ServiceDataUnit.Result;
}

/*诊断服务处理*/
static void UDS_A_DiagnosticService(uds_uint8_t DiagServiceID) {
	uds_uint8_t ValidSid = 0;
	ServiceIndex = 0;

	/*服务进程状态判断*/
	if (DiagProcessing == SystemState.DiagnosticProcessStatus) { //正在处理服务中，发送NRC78采用P2*Server
		P2_ServerRequestFlag = UDS_TRUE;
		NRC = RCRRP_78;
		return;
	}
	else {
		SystemState.DiagnosticProcessStatus = DiagProcessing;
	}

	/*服务查询*/
	while ((ServiceIndex < NUMBEROFSERVICE) && (!ValidSid)) {
		if (DiagnosticServiceTable[ServiceIndex].ServiceID == DiagServiceID) {
			if (UDS_TRUE == DiagnosticServiceTable[ServiceIndex].Support) {
				ValidSid = UDS_TRUE;
			}
			else {
				ValidSid = UDS_FALSE;
				break;
			}
		}
		else {
			ServiceIndex++;
		}
	}

	/*最小长度-寻址类型-会话模式-安全等级-服务处理*/
	if (UDS_TRUE == ValidSid) {
		if (UDS_A_Indication_SDU.Length >= DiagnosticServiceTable[ServiceIndex].MinDataLength) {			
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
						|| (ECURESET == DiagnosticServiceTable[ServiceIndex].ServiceID)
						|| (READDATABYIDENTIFIER == DiagnosticServiceTable[ServiceIndex].ServiceID)
						|| (SECURITYACCESS == DiagnosticServiceTable[ServiceIndex].ServiceID)
						|| (WRITEDATABYIDENTIFIER == DiagnosticServiceTable[ServiceIndex].ServiceID)
						|| (TESTERPRESENT == DiagnosticServiceTable[ServiceIndex].ServiceID)) {
						DiagnosticServiceTable[ServiceIndex].ServiceHandle();
					}
					else {
						NRC = SNSIAS_7F;
					}
				}
			}
			else if (Functional == UDS_A_Indication_SDU.TAType) {
				if (DefaultSession == SystemState.SessionMode) {
					if (DiagnosticServiceTable[ServiceIndex].FUNDefaultSession_Security == LEVEL_UNSUPPORT) {
						NRC = SNSIAS_7F;
					}
					else {
						if ((DiagnosticServiceTable[ServiceIndex].FUNDefaultSession_Security & SystemState.SecurityLevel) == SystemState.SecurityLevel) {
							DiagnosticServiceTable[ServiceIndex].ServiceHandle();
						}
						else {
							NRC = SAD_33;
						}
					}
				}
				else if (ExtendSession == SystemState.SessionMode) {
					if (DiagnosticServiceTable[ServiceIndex].FUNExtendedSession_Security == LEVEL_UNSUPPORT) {
						NRC = SNSIAS_7F;
					}
					else {
						if ((DiagnosticServiceTable[ServiceIndex].FUNExtendedSession_Security & SystemState.SecurityLevel) == SystemState.SecurityLevel) {
							DiagnosticServiceTable[ServiceIndex].ServiceHandle();
						}
						else {
							NRC = SAD_33;
						}
					}
				}
				else if (ProgramingSession == SystemState.SessionMode) {
					if (DiagnosticServiceTable[ServiceIndex].FUNProgramSeesion_Security == LEVEL_UNSUPPORT) {
						NRC = SNSIAS_7F;
					}
					else {
						if ((DiagnosticServiceTable[ServiceIndex].FUNProgramSeesion_Security & SystemState.SecurityLevel) == SystemState.SecurityLevel) {
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
			NRC = IMLOIF_13;
		}
	}
	else {
		NRC = SNS_11;
	}
}

/*Confirm数据处理*/
static void UDS_A_Data_Confirm_Proc(void) {

	/*Normal_Confirm*/
	if (OK == UDS_A_Confirm_SDU.Result) {
		SystemState.ConfirmType = Normal_Confirm;
		SystemState.DiagnosticProcessStatus = DiagProcessInit;
	}
	else {
		/*待定*/
	}

	if (UDS_TRUE == P2_ServerRequestFlag) { //NRC78_Confirm
		P2_ServerRequestFlag = UDS_FALSE;
		if (OK == UDS_A_Confirm_SDU.Result) {
			SystemState.ConfirmType = NRC78_Confirm;
		}
		else {
			UDS_A_Response_Proc(RCRRP_78, 3, 0, UDS_A_Indication_SDU.TAType); //重复发送NRC78-暂定
		}
	}
	else if (UDS_TRUE == WaitConfimBeforeReset) { //复位请求确认
		WaitConfimBeforeReset = UDS_FALSE;
		if (OK == UDS_A_Confirm_SDU.Result) {
			Reset_Func(ECUResetType);
		}
		else {

		}
	}
	else if (UDS_TRUE == WaitConfirmBeforePeridDIDSend) { //PDID发送请求确认
		WaitConfirmBeforePeridDIDSend = UDS_FALSE;
		if (OK == UDS_A_Confirm_SDU.Result) {
			ConfirmPeridDIDSend = UDS_TRUE;
		}
		else {

		}
	}
	else if (UDS_TRUE == WaitConfirmBeforeDTCUpdateEnable) { //使能DTC更新请求确认
		WaitConfirmBeforeDTCUpdateEnable = UDS_FALSE;	
		if (OK == UDS_A_Confirm_SDU.Result) {
			ConfirmDTCUpdate = UDS_TRUE;
		}
		else {

		}
	}
	else if (UDS_TRUE == WaitConfirmBeforeDTCUpdateDisable) { //禁止DTC更新请求确认
		WaitConfirmBeforeDTCUpdateDisable = UDS_FALSE;	
		if (OK == UDS_A_Confirm_SDU.Result) {
			ConfirmDTCUpdate = UDS_FALSE;
		}
		else {

		}
	}	
}

/*服务器响应处理进程*/
static void UDS_A_Response_Proc(NegativeResposeCode NRC_Para, uds_uint16_t DataLength, uds_uint8_t SuppPosiFlag, uds_uint8_t TAtype) {

	if ((PR_00 == NRC_Para) && (UDS_FALSE == SuppPosiFlag)) { //正响应&不抑制正响应
		UDS_A_PositiveResponseDataUnit(DataLength);
	}
	else if ((NRC_Para != PR_00) && (!((Functional == TAtype) && ((SNS_11 == NRC_Para) || (SFNS_12 == NRC_Para) || (ROOR_31 == NRC_Para) || (SFNSIAS_7E == NRC_Para) || (SNSIAS_7F == NRC_Para))))) { //负响应&非功能寻址下的特定NRC
		UDS_A_NegativeResponseDataUnit(NRC_Para);
	}
	else {

	}

	/*诊断进程状态*/
	if (((PR_00 == NRC_Para) && (UDS_TRUE == SuppPosiFlag)) || ((NRC_Para != PR_00) && (NRC_Para != RCRRP_78))) {
		SystemState.DiagnosticProcessStatus = DiagProcessInit;
	}

	/*Response类型*/
	SystemState.ResponseType = Normal_Response;

	/*正响应抑制类型*/
	if ((PR_00 == NRC_Para) && (UDS_TRUE == SuppPosiFlag)) {
		SystemState.SuppressPositiveType = SuppressPositive;
	}
	else {
		SystemState.SuppressPositiveType = NotSuppressPositive;
	}
}

/*服务器肯定响应请求函数*/
static void UDS_A_PositiveResponseDataUnit(uds_uint16_t DataLength) {
	UDS_A_Request_SDU.Mtype = UDS_A_Indication_SDU.Mtype;
	UDS_A_Request_SDU.SourceAddr = UDS_A_SOURCEADDRESS; //地址待定
	UDS_A_Request_SDU.TargetAddr = UDS_A_TARGETADDRESS; //地址待定
	UDS_A_Request_SDU.TAType = UDS_A_Indication_SDU.TAType;
#ifdef ADDRESS_EXTENSION_MODE
	UDS_A_Request_SDU.A_ExtendAddr = UDS_A_Indication_SDU.A_ExtendAddr;
#endif

	/*肯定响应固定数据格式*/
	SDUSendBuffArray[0] = UDS_A_Indication_SDU.A_PCI + 0x40;
	UDS_A_Request_SDU.MessageData = SDUSendBuffArray;
	UDS_A_Request_SDU.Length = DataLength;

	/*数据发送至网络层缓存区*/
	UDS_A_service_process_USData_request(&UDS_A_Request_SDU);
	//UDS_PRINTF("Request_Status = %d\n", UDS_A_service_process_USData_request(&UDS_A_Request_SDU));
}

/*服务器否定响应请求函数*/
static void UDS_A_NegativeResponseDataUnit(uds_uint8_t nrc) {
	UDS_A_Request_SDU.Mtype = UDS_A_Indication_SDU.Mtype;
	UDS_A_Request_SDU.SourceAddr = UDS_A_SOURCEADDRESS; //地址待定
	UDS_A_Request_SDU.TargetAddr = UDS_A_TARGETADDRESS; //地址待定
	UDS_A_Request_SDU.TAType = UDS_A_Indication_SDU.TAType;
#ifdef ADDRESS_EXTENSION_MODE
	UDS_A_Request_SDU.A_ExtendAddr = UDS_A_Indication_SDU.A_ExtendAddr;
#endif

	/*否定响应固定数据格式*/
	SDUSendBuffArray[0] = 0x7f;
	SDUSendBuffArray[1] = UDS_A_Indication_SDU.A_PCI;
	SDUSendBuffArray[2] = nrc;
	UDS_A_Request_SDU.MessageData = SDUSendBuffArray;
	UDS_A_Request_SDU.Length = 3;

	/*数据发送至网络层缓存区*/
	UDS_A_service_process_USData_request(&UDS_A_Request_SDU);
}

/*10h-会话控制服务*/
static void Service10Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;
	
	/*服务长度错误*/
	if (UDS_A_Indication_SDU.Length != 2) {
		NRC = IMLOIF_13;
		return;
	}

	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service10Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service10Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

/*10h-默认会话子服务*/
static void DefaultSession_01Proc(void) {

	/*错误条件检查*/
	{

	}

	/*条件检查通过，服务处理*/
	{
		SystemState.SessionMode = DefaultSession;
		SystemState.SecurityLevel = LEVEL_ZERO;
		if (SecurityAccessPara.UnlockStep != WAIT_DELAY) {
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
		}

		/*此处需要EnableRxAndTx NWMCM_NCM*/
		
		NumberofReadDID = NumberofStaticReadDID; //进入默认会话，动态DID失效

		ConfirmDTCUpdate = UDS_TRUE; //使能DTC状态Update

		IO_ControlPara_Flag_u.status = UDS_FALSE; //输入输出控制参数无效

		LinkStatus.LinkControlProcess = WAIT_MODE_REQ;; //ECU恢复默认的通讯模式

		/*响应数组填充*/
		SDUSendBuffArray[1] = DefaultSessionType;
		SDUSendBuffArray[2] = (uds_uint8_t)(UDS_A_timer_value_get(S_TIME_P2_CAN_SERVER) >> 8);
		SDUSendBuffArray[3] = (uds_uint8_t)UDS_A_timer_value_get(S_TIME_P2_CAN_SERVER);
		SDUSendBuffArray[4] = (uds_uint8_t)((UDS_A_timer_value_get(S_TIME_P2X_CAN_SERVER) / 10) >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)(UDS_A_timer_value_get(S_TIME_P2X_CAN_SERVER) / 10);
		ResponseDataLength = 6;
	}
}

/*10h-编程会话子服务*/
static void ProgramingSession_02Proc(void) {

	/*错误条件检查*/
	{

		/*当前会话不支持*/
		if (DefaultSession == SystemState.SessionMode) { //默认会话不能直接切换到编程会话
			NRC = SFNSIAS_7E;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		SystemState.SessionMode = ProgramingSession;
		SystemState.SecurityLevel = LEVEL_ZERO;
		if (SecurityAccessPara.UnlockStep != WAIT_DELAY) {
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
		}

		/*响应数组填充*/
		SDUSendBuffArray[1] = ProgramingSessionType;
		SDUSendBuffArray[2] = (uds_uint8_t)(UDS_A_timer_value_get(S_TIME_P2_CAN_SERVER) >> 8);
		SDUSendBuffArray[3] = (uds_uint8_t)UDS_A_timer_value_get(S_TIME_P2_CAN_SERVER);
		SDUSendBuffArray[4] = (uds_uint8_t)((UDS_A_timer_value_get(S_TIME_P2X_CAN_SERVER) / 10) >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)(UDS_A_timer_value_get(S_TIME_P2X_CAN_SERVER) / 10);
		ResponseDataLength = 6;
	}
}

/*10h-扩展会话子服务*/
static void ExtendSession_03Proc(void) {

	/*错误条件检查*/
	{

		/*当前会话不支持*/
		if (ProgramingSession == SystemState.SessionMode) { //编程会话不能直接切换到扩展会话
			NRC = SFNSIAS_7E;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		SystemState.SessionMode = ExtendSession;
		SystemState.SecurityLevel = LEVEL_ZERO;
		if (SecurityAccessPara.UnlockStep != WAIT_DELAY) {
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
		}

		/*响应数组填充*/
		SDUSendBuffArray[1] = ExtendSessionType;
		SDUSendBuffArray[2] = (uds_uint8_t)(UDS_A_timer_value_get(S_TIME_P2_CAN_SERVER) >> 8);
		SDUSendBuffArray[3] = (uds_uint8_t)UDS_A_timer_value_get(S_TIME_P2_CAN_SERVER);
		SDUSendBuffArray[4] = (uds_uint8_t)((UDS_A_timer_value_get(S_TIME_P2X_CAN_SERVER) / 10) >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)(UDS_A_timer_value_get(S_TIME_P2X_CAN_SERVER) / 10);
		ResponseDataLength = 6;
	}
}

/*10h-出厂会话子服务*/
static void FactorySeesion_04Proc(void) {

	/*错误条件检查*/
	{

		/*不满足切换会话模式的条件-待定*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		SystemState.SessionMode = FactorySeesion;
		SystemState.SecurityLevel = LEVEL_ZERO;
		if (SecurityAccessPara.UnlockStep != WAIT_DELAY) {
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
		}

		/*响应数组填充*/
		SDUSendBuffArray[1] = FactorySeesionType;
		SDUSendBuffArray[2] = (uds_uint8_t)(UDS_A_timer_value_get(S_TIME_P2_CAN_SERVER) >> 8);
		SDUSendBuffArray[3] = (uds_uint8_t)UDS_A_timer_value_get(S_TIME_P2_CAN_SERVER);
		SDUSendBuffArray[4] = (uds_uint8_t)((UDS_A_timer_value_get(S_TIME_P2X_CAN_SERVER) / 10) >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)(UDS_A_timer_value_get(S_TIME_P2X_CAN_SERVER) / 10);
		ResponseDataLength = 6;
	}
}

/*11h-ECU复位服务*/
static void Service11Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;
	
	/*子服务长度错误*/
	if (UDS_A_Indication_SDU.Length != 2) {
		NRC = IMLOIF_13;
		return;
	}

	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	ECUResetType = ServiceSubFunc;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service11Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service11Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

/*11h-硬件复位子服务*/
static void HardReset_01Proc(void) {

	/*错误条件检查*/
	{

		/*复位条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		if (UDS_TRUE == SuppressPosRspMsgIndicationBit) { //抑制正响应直接复位
			Reset_Func(HARD_RESET);
		}
		else { //复位之前先发送Response
			SDUSendBuffArray[1] = HARD_RESET;
			ResponseDataLength = 2;
			WaitConfimBeforeReset = UDS_TRUE;
		}
	}
}

/*11h-钥匙下电上电复位子服务*/
static void KeyOnOffReset_02Proc(void) {

	/*错误条件检查*/
	{

		/*复位条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		if (UDS_TRUE == SuppressPosRspMsgIndicationBit) { //抑制正响应直接复位
			Reset_Func(KEY_OFF_ON_RESET);
		}
		else { //复位之前先发送Response
			SDUSendBuffArray[1] = KEY_OFF_ON_RESET;
			ResponseDataLength = 2;
			WaitConfimBeforeReset = UDS_TRUE;
		}
	}
}

/*11h-软件复位子服务*/
static void SoftReset_03Proc(void) {

	/*错误条件检查*/
	{

		/*复位条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		if (UDS_TRUE == SuppressPosRspMsgIndicationBit) { //抑制正响应直接复位
			Reset_Func(SOFT_RESET);
		}
		else { //复位之前先发送Response
			SDUSendBuffArray[1] = SOFT_RESET;
			ResponseDataLength = 2;
			WaitConfimBeforeReset = UDS_TRUE;
		}
	}
}

/*11h-复位接口函数*/
static void Reset_Func(EcuResetType_e ResetType) {
	if (HARD_RESET == ResetType) {
		//预留
	}
	else if (KEY_OFF_ON_RESET == ResetType) {
		//预留
	}
	else if (SOFT_RESET == ResetType) {
		//预留
	}
}

/*14h-ClearDiagnosticInformation服务*/
static void Service14Handle(void) {
	uds_uint8_t i, buffer[MAX_DTC_SNAPSHOT_STORE_BYTES] = { 0 };
	uds_uint32_t GroupOfDTC = ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[0] << 16) 
								| ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[1] << 8) | (uds_uint32_t)UDS_A_Indication_SDU.A_SDU[2];

	/*错误条件检查*/
	{

		/*服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 4) {
			NRC = IMLOIF_13;
			return;
		}

		/*清除条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}

		/*GroupOfDTC不支持*/
		if (!SearchDTCGroup_Func(GroupOfDTC)) {
			NRC = ROOR_31;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		for(i = 0 ; i < NumberofDTC; i++) {
			if((DTCTable[i].Config.DTCNumber & GroupOfDTC) == DTCTable[i].Config.DTCNumber) {
				DTCTable[i].Sts.DTCstatus.StatusOfDTC = 0x50; //bit4 and bit6 set 1
				DTCTable[NumberofDTC].Sts.DetecCycleCounter = 0;
				DTCTable[NumberofDTC].Sts.FaultDetectionCounter = 0;
				DTCTable[NumberofDTC].Sts.ConfirmationCounter = 0;
				DTCTable[NumberofDTC].Sts.FaultOccurenceCounter = 0;
				DTCTable[NumberofDTC].Sts.FaultPendingCounter = 0;
				DTCTable[NumberofDTC].Sts.FaultAgedCounter = 0;
				DTCTable[NumberofDTC].Sts.FaultAgingCounter = 0;
				UDS_A_Data_Save(DTCTable[NumberofDTC].Config.DTCStatusTag, &(DTCTable[NumberofDTC].Sts.DTCstatus.StatusOfDTC), MAX_DTC_EEPROM_BYTES);
				UDS_A_Data_Save(DTCTable[NumberofDTC].Config.SnapShotTag,  buffer, DTC_SNAPSHOT_BYTES);
				//UDS_A_eeprom_write(DTCTable[NumberofDTC].DTCStatusTag, &(DTCTable[NumberofDTC].DTCstatus.StatusOfDTC));
				//UDS_A_eeprom_write(DTCTable[NumberofDTC].SnapShotTag, buffer); //清除DTC快照
			}
		}				
		ResponseDataLength = 1;
	}
}

/*14h-查询DTC组接口函数*/
static uds_uint8_t SearchDTCGroup_Func(uds_uint32_t DTCGroup) {
	uds_uint8_t i;
	for(i = 0; i < NumberofDTCGroup; i++) {
		if(DTCGroupTable[i].GroupID == DTCGroup) {
			return UDS_TRUE;
		}
	}
	return UDS_FALSE;
}

/*19h-读取DTC信息服务*/
static void Service19Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;
	
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service19Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service19Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

/*19h-01ReportNumberOfDTCByStatusMask子服务*/
static void ReportNumberOfDTCByStatusMask_01Proc(void) {
	uds_uint8_t i, DTCStatusMask = UDS_A_Indication_SDU.A_SDU[1]; //DTC状态掩码
	uds_uint16_t DTCCountByte; //DTC字节数

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 3) {
			NRC = IMLOIF_13;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		for(i = 0; i < NumberofDTC; i++) {
			if ((DTCTable[i].Sts.DTCstatus.StatusOfDTC & DTCStatusMask & DTCStatusAvailabilityMask) != 0) {
				DTCCountByte ++;
			}
		}

		/*响应数组填充*/
		SDUSendBuffArray[1] = reportNumberOfDTCByStatusMask;
		SDUSendBuffArray[2] = DTCStatusAvailabilityMask;
		SDUSendBuffArray[3] = SAE_J2012_DA_DTCFormat_00;
		SDUSendBuffArray[4] = (uds_uint8_t)(DTCCountByte >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)DTCCountByte;
		ResponseDataLength = 6;
	}
}

/*19h-02reportDTCByStatusMask子服务*/
static void ReportDTCByStatusMask_02Proc(void) {
	uds_uint8_t i, DTCStatusMask = UDS_A_Indication_SDU.A_SDU[1]; //DTC状态掩码

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 3) {
			NRC = IMLOIF_13;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{

		/*响应数组填充*/
		SDUSendBuffArray[1] = reportDTCByStatusMask;
		SDUSendBuffArray[2] = DTCStatusAvailabilityMask;
		ResponseDataLength = 3;
		for(i = 0; i < NumberofDTC; i++) {
			if((DTCTable[i].Sts.DTCstatus.StatusOfDTC & DTCStatusMask & DTCStatusAvailabilityMask) != 0) {
				SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCTable[i].Config.DTCNumber >> 16);
				SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCTable[i].Config.DTCNumber >> 8);
				SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)DTCTable[i].Config.DTCNumber;
				SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCTable[i].Sts.DTCstatus.StatusOfDTC & DTCStatusAvailabilityMask);
			}
		}
	}
}

/*19h-03reportDTCSnapshotIdentification子服务*/
static void ReportDTCSnapshotID_03Proc(void) {
	uds_uint8_t i, j;

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{

		/*响应数组填充*/
		SDUSendBuffArray[1] = reportDTCSnapshotIdentification;
		ResponseDataLength = 2;
		for(i = 0; i < NumberofDTC; i++) {
			for(j = 0; j < NumberofDTCSnapshot; j++) {	
				if ((0 == j) || ((j > 0) && (DTCSnapshotTable[j].SnapshotRecordNumber != DTCSnapshotTable[j - 1].SnapshotRecordNumber))) {
					SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCTable[i].Config.DTCNumber >> 16);
					SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCTable[i].Config.DTCNumber >> 8);
					SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)DTCTable[i].Config.DTCNumber;
					SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)DTCSnapshotTable[j].SnapshotRecordNumber;
				}
			}
		}
	}
}

/*19h-04ReportDTCSnapshotRecordByDTCNumber子服务*/
static void ReportDTCSnapshotRecord_04Proc(void) {
	uds_uint32_t DTCNumber = ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[1] << 16) 
								| ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[2] << 8) 
								| (uds_uint32_t)UDS_A_Indication_SDU.A_SDU[3];
	uds_uint8_t i, RecordNumberCounter = 0, DTCSnapshotRecordNumber = UDS_A_Indication_SDU.A_SDU[4]; 
	DTCNode* TempDTCNode = SearchDTCNode_Func(DTCNumber);

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 6) {
			NRC = IMLOIF_13;
			return;
		}

		/*DTCNode不支持*/
		if (!TempDTCNode) {
			NRC = ROOR_31;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{

		/*响应数组填充*/
		SDUSendBuffArray[1] = reportDTCSnapshotRecordByDTCNumber;
		SDUSendBuffArray[2] = (uds_uint8_t)(TempDTCNode->Config.DTCNumber >> 16);
		SDUSendBuffArray[3] = (uds_uint8_t)(TempDTCNode->Config.DTCNumber >> 8);
		SDUSendBuffArray[4] = (uds_uint8_t)TempDTCNode->Config.DTCNumber;
		SDUSendBuffArray[5] = (uds_uint8_t)(TempDTCNode->Sts.DTCstatus.StatusOfDTC & DTCStatusAvailabilityMask);		
		ResponseDataLength = 8;

		if (0xff == DTCSnapshotRecordNumber) {
			uds_uint8_t currentRecord = DTCSnapshotTable[0].SnapshotRecordNumber;
			uds_uint8_t currentRecordNumber = 0;
			uds_uint8_t currentRecordSizeIndex = 6;
			for(i = 0 ; i < NumberofDTCSnapshot; i++) {
				if(DTCSnapshotTable[i].SnapshotRecordNumber == currentRecord) {
					currentRecordNumber++;
					SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCSnapshotTable[i].SnapshotID >> 8);
					SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCSnapshotTable[i].SnapshotID);
					DTC_Snapshot_Read_Func(TempDTCNode->Config.SnapShotTag, DTCSnapshotTable[i]); /*将DTC快照中DID中的数据存入发送数组*/					
				}
				else {
					SDUSendBuffArray[currentRecordSizeIndex] = (uds_uint8_t)currentRecord;
					SDUSendBuffArray[currentRecordSizeIndex + 1] = (uds_uint8_t)currentRecordNumber;
					currentRecordSizeIndex = ResponseDataLength;
					ResponseDataLength += 2;
					currentRecordNumber = 1;
					SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCSnapshotTable[i].SnapshotID >> 8);
					SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCSnapshotTable[i].SnapshotID);
					DTC_Snapshot_Read_Func(TempDTCNode->Config.SnapShotTag, DTCSnapshotTable[i]); /*将DTC快照中DID中的数据存入发送数组*/	
					currentRecord = DTCSnapshotTable[i].SnapshotRecordNumber;
				}
			}
			SDUSendBuffArray[currentRecordSizeIndex]  = (uds_uint8_t)currentRecord;
			SDUSendBuffArray[currentRecordSizeIndex + 1]  = (uds_uint8_t)currentRecordNumber;
		}
		else {
			for (i = 0; i < NumberofDTCSnapshot; i++) {
				if (DTCSnapshotRecordNumber == DTCSnapshotTable[i].SnapshotRecordNumber) {
					RecordNumberCounter ++;
					SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCSnapshotTable[i].SnapshotID >> 8);
					SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)DTCSnapshotTable[i].SnapshotID;
					DTC_Snapshot_Read_Func(TempDTCNode->Config.SnapShotTag, DTCSnapshotTable[i]); /*将DTC快照中DID中的数据存入发送数组*/	
				}
			}
			SDUSendBuffArray[6] = (uds_uint8_t)DTCSnapshotRecordNumber;
			SDUSendBuffArray[7] = (uds_uint8_t)RecordNumberCounter;
		}

		if (0 == SDUSendBuffArray[7]) {
			NRC = ROOR_31;
		}
	}
}

static void DTC_Snapshot_Read_Func(uds_uint8_t SnapShotTag, DTCSnapshot_t TempDTCSnapshot) {
	uds_uint8_t length = 0, i, buffer[MAX_DTC_SNAPSHOT_STORE_BYTES];
	for(i = 0 ; i < NumberofDTCSnapshot ; i++) {
		length += DTCSnapshotTable[i].DataLength;
	}
	UDS_A_Data_Load(SnapShotTag, buffer, length, DataLoad_DefaultValue);
	memcpy(SDUSendBuffArray + ResponseDataLength, buffer + TempDTCSnapshot.StartNum, TempDTCSnapshot.DataLength);
	ResponseDataLength += TempDTCSnapshot.DataLength;
}

/*19h-查询DTC节点接口函数*/
static DTCNode* SearchDTCNode_Func(uds_uint32_t DtcCode) {
	uds_uint8_t i;
	for(i = 0 ; i < NumberofDTC; i++) {	
		if (DTCTable[i].Config.DTCNumber == DtcCode) {
			return DTCTable + i;
		}
	}
	return UDS_A_NULL;
}

/*19h-05reportDTCStoredDataByRecordNumber子服务*/
static void ReportDTCStoredData_05Proc(void) {
	NRC = SFNS_12;
}

/*19h-06reportDTCExtDataRecordByDTCNumber子服务*/
static void ReportDTCExtDataRecord_06Proc(void) {
	uds_uint32_t DTCNumber = ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[1] << 16) 
							| ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[2] << 8) 
							| (uds_uint32_t)UDS_A_Indication_SDU.A_SDU[3];
	uds_uint8_t i, RecordNumberCounter = 0, DTCExtDataRecordNumber = UDS_A_Indication_SDU.A_SDU[4]; 
	DTCNode* TempDTCNode = SearchDTCNode_Func(DTCNumber);
	
	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 6) {
			NRC = IMLOIF_13;
			return;
		}

		/*DTCNode不支持或DTCExtDataRecordNumber不支持*/
		if(UDS_A_NULL == TempDTCNode) {
			NRC = ROOR_31;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{

		/*响应数组填充*/
		SDUSendBuffArray[1] = reportDTCExtDataRecordByDTCNumber;
		SDUSendBuffArray[2] = (uds_uint8_t)(TempDTCNode->Config.DTCNumber >> 16);
		SDUSendBuffArray[3] = (uds_uint8_t)(TempDTCNode->Config.DTCNumber >> 8);
		SDUSendBuffArray[4] = (uds_uint8_t)TempDTCNode->Config.DTCNumber;
		SDUSendBuffArray[5] = (uds_uint8_t)(TempDTCNode->Sts.DTCstatus.StatusOfDTC & DTCStatusAvailabilityMask);		
		ResponseDataLength = 6;

		if (0xff == DTCExtDataRecordNumber) {
			if(FaultOccurenceCounterRecord != INVALID_DTCEXTDATARECORDNUMBER) {
				SDUSendBuffArray[ResponseDataLength++] = FaultOccurenceCounterRecord;
				SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)TempDTCNode->Sts.FaultOccurenceCounter;
			}
		
			if(FaultPendingCounterRecord != INVALID_DTCEXTDATARECORDNUMBER) {
				SDUSendBuffArray[ResponseDataLength++] = FaultPendingCounterRecord;
				SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)TempDTCNode->Sts.FaultPendingCounter;
			}
			if(FaultAgingCounterRecord != INVALID_DTCEXTDATARECORDNUMBER) {
				SDUSendBuffArray[ResponseDataLength++] = FaultAgingCounterRecord;
				SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)TempDTCNode->Sts.FaultAgingCounter;
			}
			
			if(FaultAgedCounterRecord != INVALID_DTCEXTDATARECORDNUMBER) {
				SDUSendBuffArray[ResponseDataLength++] = FaultAgedCounterRecord;
				SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)TempDTCNode->Sts.FaultAgedCounter;
			}
		}
		else if (DTCExtDataRecordNumber == FaultOccurenceCounterRecord) {
			SDUSendBuffArray[ResponseDataLength++] = FaultOccurenceCounterRecord;
			SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)TempDTCNode->Sts.FaultOccurenceCounter;
		}
		else if (DTCExtDataRecordNumber == FaultPendingCounterRecord) {
			SDUSendBuffArray[ResponseDataLength++] = FaultPendingCounterRecord;
			SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)TempDTCNode->Sts.FaultPendingCounter;
		}
		else if (DTCExtDataRecordNumber == FaultAgingCounterRecord) {
			SDUSendBuffArray[ResponseDataLength++] = FaultAgingCounterRecord;
			SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)TempDTCNode->Sts.FaultAgingCounter;
		}
		else if (DTCExtDataRecordNumber == FaultAgedCounterRecord) {
			SDUSendBuffArray[ResponseDataLength++] = FaultAgedCounterRecord;
			SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)TempDTCNode->Sts.FaultAgedCounter;
		}
		else {
			NRC = ROOR_31;
		}
		
	}
}

/*19h-0AReportSupportedDTC子服务*/
static void ReportSupportedDTC_0AProc(void) {
	uds_uint8_t i;
	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{

		/*响应数组填充*/
		SDUSendBuffArray[1] = reportSupportedDTC;
		SDUSendBuffArray[2] = DTCStatusAvailabilityMask; //DTC状态有效掩码-待定
		ResponseDataLength = 3;
		for(i = 0; i < NumberofDTC; i++) {
			SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCTable[i].Config.DTCNumber >> 16);
			SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCTable[i].Config.DTCNumber >> 8);
			SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)DTCTable[i].Config.DTCNumber;
			SDUSendBuffArray[ResponseDataLength++] = (uds_uint8_t)(DTCTable[i].Sts.DTCstatus.StatusOfDTC & DTCStatusAvailabilityMask);			
		}
	}
}

/*22h-ReadDataByIdentifier服务*/
static void Service22Handle(void) {
	uds_uint16_t DataIdentifier, DataIdentifierArray[MAXNUMBEROFDID], Index = 0;
	uds_uint8_t i, j, NumOfDid, sts = UDS_FALSE;

	/*错误条件检查*/
	{

		/*服务长度错误*/
		if (0 == (UDS_A_Indication_SDU.Length % 2)) {
			NRC = IMLOIF_13;
			return;
		}
		else if (UDS_A_Indication_SDU.Length >(2 * MAXNUMBEROFDID + 1)) {
			NRC = ROOR_31;
			return;
		}

		/*操作条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		NumOfDid = (UDS_A_Indication_SDU.Length - 1) / 2;
		ResponseDataLength = 1;
		for (i = 0; i < NumOfDid; i++) {
			sts = UDS_FALSE;
			DataIdentifier = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2 * i] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2 * i + 1];
			memcpy(&SDUSendBuffArray[ResponseDataLength], &UDS_A_Indication_SDU.A_SDU[2 * i], 2);
			ResponseDataLength += 2;
			for (j = 0; j < NumberofReadDID; j++) {
				if (DataIdentifier == ReadDIDTable[j].DataIdentifier) {
					if (Write_EEPROM == ReadDIDTable[j].DataIdentifierType) {
						UDS_A_Data_Load(ReadDIDTable[j].DataIdentifierTag, SDUSendBuffArray + ResponseDataLength, ReadDIDTable[j].DataLength, DataLoad_DefaultValue);
						//UDS_A_eeprom_read(ReadDIDTable[j].DataIdentifierTag, SDUSendBuffArray + ResponseDataLength);
					}
					else {
						memcpy((SDUSendBuffArray + ResponseDataLength), ReadDIDTable[j].DataPointer, ReadDIDTable[j].DataLength);
					}
					ResponseDataLength += ReadDIDTable[j].DataLength;
					sts = UDS_TRUE;
				}

				//if ((j == (NumberofReadDID - 1)) && (UDS_FALSE == sts)) { //请求ID有效性
				//	NRC = ROOR_31;
				//	break;
				//}
			}

			if(UDS_FALSE == sts) {
				ResponseDataLength -= 2;
			}
		}

		/*协议要求，请求的所有DID不支持时，发送0X31*/
		if (ResponseDataLength < 2) {
			NRC = ROOR_31;
		}
		else if (ResponseDataLength > UDS_RECIVE_BUFFER_LENGTH) { //超出Server最大数据发送长度
			NRC = RTL_14;
		}
	}
}

/*23h-ReadMemoryByAddress服务*/
static void Service23Handle(void) {
	uds_uint8_t memorySizeBytes = (UDS_A_Indication_SDU.A_SDU[0] >> 4) & 0x0f, memoryAddressBytes = UDS_A_Indication_SDU.A_SDU[0] & 0x0f;
	uds_uint8_t* memoryAddress;
	uds_uint32_t memorySize;

	/*错误条件检查*/
	{

		/*addressAndLengthFormatIdentifier错误*/
		if ((0 == memorySizeBytes) || (0 == memoryAddressBytes) || (memorySizeBytes > 4) || (memoryAddressBytes > 4)) { //(memorySizeBytes > 服务器支持的最大值)等
			NRC = ROOR_31;
			return;
		}

		/*服务长度错误*/
		if (UDS_A_Indication_SDU.Length != (memorySizeBytes + memoryAddressBytes + 2)) {
			NRC = IMLOIF_13;
			return;
		}

		/*操作条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		memoryAddress = ReadMemoryByAddress_Func(UDS_A_Indication_SDU.A_SDU + 1, memoryAddressBytes);
		memorySize = ReadMemorySize_Func((UDS_A_Indication_SDU.A_SDU + memoryAddressBytes + 1), memorySizeBytes);	
		
		/*此处需要添加Tester请求地址与Server本地地址的映射关系，地址有效性检查，地址操作权限检查等*/
		
		memcpy(SDUSendBuffArray + 1, memoryAddress, memorySize);
		ResponseDataLength = memorySize + 1;
	}
}

/*23h-读取内存地址接口函数*/
static uds_uint8_t *ReadMemoryByAddress_Func(uds_uint8_t *Address, uds_uint16_t Length) {
	uds_uint8_t *MemoryParameter = 0;
	switch (Length) {
	case 1:
		MemoryParameter = (uds_uint8_t *)((uds_uint32_t)(*Address));
		break;
	case 2:
		MemoryParameter = (uds_uint8_t *)(((uds_uint32_t)(*Address) << 8) | (uds_uint32_t)(*(Address + 1)));
		break;
	case 3:
		MemoryParameter = (uds_uint8_t *)(((uds_uint32_t)(*Address) << 16) | ((uds_uint32_t)(*(Address + 1)) << 8) | (uds_uint32_t)(*(Address + 2)));
		break;
	case 4:
		MemoryParameter = (uds_uint8_t *)(((uds_uint32_t)(*Address) << 24) | ((uds_uint32_t)(*(Address + 1)) << 16) | ((uds_uint32_t)(*(Address + 2)) << 8) | (uds_uint32_t)(*(Address + 3)));
		break;
	default:
		break;
	}
	return MemoryParameter;
}

/*23h-读取内存长度接口函数*/
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

/*27h-安全访问服务*/
static void Service27Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;
	
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service27Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service27Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

/*27h-SecurityAccess Seed01子服务*/
static void SecurityAccessSeed_01Proc(void) {
	uds_uint32_t Seed;

	/*错误条件检查*/
	{

		/*延迟定时器处于激活状态*/
		if (WAIT_DELAY == SecurityAccessPara.UnlockStep) {
			NRC = RTDNE_37;
			return;
		}

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*安全访问条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{

		/*种子计算*/
		if (LEVEL_ONE == SystemState.SecurityLevel) { //种子请求安全等级等于当前系统安全等级
			Seed = 0;
		}
		else {
			Seed = SeedCalc01_Func((uds_uint32_t)SEED01); //计算Seed
			SecurityAccessPara.Key = KeyCalc01_Func(Seed); //计算Key
			SecurityAccessPara.UnlockStep = WAIT_KEY;
		}

		/*响应数组填充*/
		SDUSendBuffArray[1] = SEED01;
		SDUSendBuffArray[2] = (uds_uint8_t)(Seed >> 24);
		SDUSendBuffArray[3] = (uds_uint8_t)(Seed >> 16);
		SDUSendBuffArray[4] = (uds_uint8_t)(Seed >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)Seed;
		ResponseDataLength = 6;
	}
}

/*27h-SecurityAccess Key02子服务*/
static void SecurityAccessKey_02Proc(void) {
	uds_uint32_t SeedKey;

	/*错误条件检查*/
	{

		/*延迟定时器处于激活状态*/
		if (WAIT_DELAY == SecurityAccessPara.UnlockStep) {
			NRC = RTDNE_37;
			return;
		}

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 6) {
			NRC = IMLOIF_13;
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*安全访问条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*请求序列错误*/
		if (SecurityAccessPara.UnlockStep != WAIT_KEY) {
			NRC = RSE_24;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		SeedKey = ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[1] << 24) | ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[2] << 16) 
					| ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[3] << 8) | (uds_uint32_t)UDS_A_Indication_SDU.A_SDU[4];
		if (SeedKey == SecurityAccessPara.Key) {
			SecurityAccessPara.KeyVerifyErrorCount = 0; //密钥验证失败计数器清零
			SystemState.SecurityLevel = LEVEL_ONE; //切换安全等级
			SecurityAccessPara.UnlockStep = UNLOCKED;

			/*响应数组填充*/
			SDUSendBuffArray[1] = KEY02;
			ResponseDataLength = 2;
		}
		else {
			if (SecurityAccessPara.KeyVerifyErrorCount < (MAXKEYERRORPERMITCOUNT - 1)) {
				SecurityAccessPara.KeyVerifyErrorCount++;
				NRC = IK_35;
				SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
			}
			else {

				/*超过最大密钥失败允许次数*/
				SecurityAccessPara.KeyVerifyErrorCount = MAXKEYERRORPERMITCOUNT;
				UDS_A_timer_ctl_run(S_TIME_SECURITYDELAY_SERVER); //开启延时定时器
				SecurityAccessPara.UnlockStep = WAIT_DELAY;
				NRC = ENOA_36;
			}
		}
		UDS_A_Data_Save(SecurityAccessPara.AccessErrorCountTag, &(SecurityAccessPara.KeyVerifyErrorCount), 1);
		//UDS_A_eeprom_write(SecurityAccessPara.AccessErrorCountTag, &(SecurityAccessPara.KeyVerifyErrorCount)); //保存KeyVerifyErrorCount
	}
}

/*27h-SecurityAccess Seed03子服务*/
static void SecurityAccessSeed_03Proc(void) {
	uds_uint32_t Seed;

	/*错误条件检查*/
	{

		/*延迟定时器处于激活状态*/
		if (WAIT_DELAY == SecurityAccessPara.UnlockStep) {
			NRC = RTDNE_37;
			return;
		}

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*安全访问条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{

		/*种子计算*/
		if (LEVEL_TWO == SystemState.SecurityLevel) { //种子请求安全等级等于当前系统安全等级
			Seed = 0;
		}
		else {
			Seed = SeedCalc03_Func((uds_uint32_t)SEED03); //计算Seed
			SecurityAccessPara.Key = KeyCalc03_Func(Seed); //计算Key
			SecurityAccessPara.UnlockStep = WAIT_KEY;
		}

		/*响应数组填充*/
		SDUSendBuffArray[1] = SEED03;
		SDUSendBuffArray[2] = (uds_uint8_t)(Seed >> 24);
		SDUSendBuffArray[3] = (uds_uint8_t)(Seed >> 16);
		SDUSendBuffArray[4] = (uds_uint8_t)(Seed >> 8);
		SDUSendBuffArray[5] = (uds_uint8_t)Seed;
		ResponseDataLength = 6;
	}
}

/*27h-SecurityAccess Key04子服务*/
static void SecurityAccessKey_04Proc(void) {
	uds_uint32_t SeedKey;

	/*错误条件检查*/
	{

		/*延迟定时器处于激活状态*/
		if (WAIT_DELAY == SecurityAccessPara.UnlockStep) {
			NRC = RTDNE_37;
			return;
		}

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 6) {
			NRC = IMLOIF_13;
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*安全访问条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
			return;
		}

		/*请求序列错误*/
		if (SecurityAccessPara.UnlockStep != WAIT_KEY) {
			NRC = RSE_24;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		SeedKey = ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[1] << 24) | ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[2] << 16) 
					| ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[3] << 8) | (uds_uint32_t)UDS_A_Indication_SDU.A_SDU[4];
		if (SeedKey == SecurityAccessPara.Key) {
			SecurityAccessPara.KeyVerifyErrorCount = 0; //密钥验证失败计数器清零
			SystemState.SecurityLevel = LEVEL_TWO; //切换安全等级
			SecurityAccessPara.UnlockStep = UNLOCKED; //解锁状态

			/*响应数组填充*/
			SDUSendBuffArray[1] = KEY04;
			ResponseDataLength = 2;
		}
		else {
			if (SecurityAccessPara.KeyVerifyErrorCount < (MAXKEYERRORPERMITCOUNT - 1)) {
				SecurityAccessPara.KeyVerifyErrorCount++;
				NRC = IK_35;
				SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
			}
			else {

				/*超过最大密钥失败允许次数*/
				SecurityAccessPara.KeyVerifyErrorCount = MAXKEYERRORPERMITCOUNT;
				UDS_A_timer_ctl_run(S_TIME_SECURITYDELAY_SERVER); //开启延时定时器
				SecurityAccessPara.UnlockStep = WAIT_DELAY;
				NRC = ENOA_36;
			}
		}
		UDS_A_Data_Save(SecurityAccessPara.AccessErrorCountTag, &(SecurityAccessPara.KeyVerifyErrorCount), 1);
		//UDS_A_eeprom_write(SecurityAccessPara.AccessErrorCountTag, &(SecurityAccessPara.KeyVerifyErrorCount)); //保存KeyVerifyErrorCount
	}
}

/*27h-Seed01计算-待添加*/
static uds_uint32_t SeedCalc01_Func(uds_uint8_t RequestSeed) {
	uds_uint32_t Seed;
	Seed = (uds_uint32_t)RequestSeed + 0x11110000;
	return Seed;
}

/*27h-Key01计算-待添加*/
static uds_uint32_t KeyCalc01_Func(uds_uint32_t RequestSeed) {
	uds_uint32_t Key;
	Key = (uds_uint32_t)RequestSeed + 0x00001111;
	return Key;
}

/*27h-Seed03计算-待添加*/
static uds_uint32_t SeedCalc03_Func(uds_uint8_t RequestSeed) {
	uds_uint32_t Seed;
	Seed = (uds_uint32_t)RequestSeed + 0x33330000;
	return Seed;
}

/*27h-Key03计算-待添加*/
static uds_uint32_t KeyCalc03_Func(uds_uint32_t RequestSeed) {
	uds_uint32_t Key;
	Key = (uds_uint32_t)RequestSeed + 0x00003333;
	return Key;
}

/*28h-通讯控制服务*/
static void Service28Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;

	/*子服务长度错误*/
	if (UDS_A_Indication_SDU.Length != 3) {
		NRC = IMLOIF_13;
		return;
	}

	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service28Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service28Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

/*28h-EnableRxAndTx子服务*/
static void EnableRxAndTx_00Proc(void) {
	uds_uint8_t CommunicationType = UDS_A_Indication_SDU.A_SDU[1];

	/*错误条件检查*/
	{

		/*条件不满足-待添加(一般包括车辆静止，车速为0等)*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		switch(CommunicationType) {
			case NCM:
					/*使能接收，使能发送*/
				break;
			case NWMCM:
					/*使能接收，使能发送*/
				break;
			case NWMCM_NCM:
					/*使能接收，使能发送*/
				break;
			default:
				NRC = ROOR_31;
				return;
		}
		
		/*响应数组填充*/
		SDUSendBuffArray[1] = ERXTX;
		ResponseDataLength = 2;
	}
}

/*28h-EnableRxAndDisableTx子服务*/
static void EnableRxAndDisableTx_01Proc(void) {
	uds_uint8_t CommunicationType = UDS_A_Indication_SDU.A_SDU[1];

	/*错误条件检查*/
	{

		/*条件不满足-待添加(一般包括车辆静止，车速为0等)*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		switch(CommunicationType) {
			case NCM:
					/*使能接收，禁止发送*/
				break;
			case NWMCM:
					/*使能接收，禁止发送*/
				break;
			case NWMCM_NCM:
					/*使能接收，禁止发送*/
				break;
			default:
				NRC = ROOR_31;
				return;
		}
		
		/*响应数组填充*/
		SDUSendBuffArray[1] = ERXDTX;
		ResponseDataLength = 2;
	}
}

/*28h-DisableRxAndEnableTx子服务*/
static void DisableRxAndEnableTx_02Proc(void) {
	uds_uint32_t CommunicationType = UDS_A_Indication_SDU.A_SDU[1];

	/*错误条件检查*/
	{

		/*条件不满足-待添加(一般包括车辆静止，车速为0等)*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		switch(CommunicationType) {
			case NCM:
					/*禁止接收，使能发送*/
				break;
			case NWMCM:
					/*禁止接收，使能发送*/
				break;
			case NWMCM_NCM:
					/*禁止接收，使能发送*/
				break;
			default:
				NRC = ROOR_31;
				return;
		}
		
		/*响应数组填充*/
		SDUSendBuffArray[1] = DRXETX;
		ResponseDataLength = 2;
	}

}

/*28h-DisableRxAndTx子服务*/
static void DisableRxAndTx_03Proc(void) {
	uds_uint32_t CommunicationType = UDS_A_Indication_SDU.A_SDU[1];

	/*错误条件检查*/
	{

		/*条件不满足-待添加(一般包括车辆静止，车速为0等)*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		switch(CommunicationType) {
			case NCM:
					/*禁止接收，禁止发送*/
				break;
			case NWMCM:
					/*禁止接收，禁止发送*/
				break;
			case NWMCM_NCM:
					/*禁止接收，禁止发送*/
				break;
			default:
				NRC = ROOR_31;
				return;
		}
		
		/*响应数组填充*/
		SDUSendBuffArray[1] = DRXTX;
		ResponseDataLength = 2;
	}
}

static void Service2AHandle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;
	
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service2ATable[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service2ATable[SubIndex].SubServiceHandle();
	}
	else {
		NRC = ROOR_31; //14229规范0X2A服务无SFNS_12响应码
	}
}

static void SendAtSlowRate_01Proc(void) {
	uds_uint8_t i, j, k, DIDNumber, Tmp = 0, NewDIDCounter = 0, sts = UDS_FALSE;
	uds_uint8_t TempDIDTable[MAXNUMBEROFPERIODICDID], TmpIndex = 0;

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length < 3) {
			NRC = IMLOIF_13;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		DIDNumber = UDS_A_Indication_SDU.Length - 2;
		for(i = 0; i < DIDNumber; i++) {
			for(j = 0; j < NumberofReadDID; j++) {
				if ((0xF200 | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[i + 1]) == ReadDIDTable[j].DataIdentifier) {
					Tmp++;					
					
					/*重复的周期DID请求判断*/
					for(k = 0; k < TmpIndex; k++) {
						if(TempDIDTable[k] == j) {
							sts = UDS_TRUE;
						}
					}

					if (UDS_FALSE == sts) {
						TempDIDTable[TmpIndex++] = j;
						if (UDS_FALSE == ReadDIDTable[j].Support) {
							NewDIDCounter ++;
						}
					}
					sts = UDS_FALSE;
					break;
				}
			}
		}

		if(0 == Tmp) {
			NRC = ROOR_31;
		}
		else if ((NumofPeriodicDIDRunning + NewDIDCounter) > MAXNUMBEROFPERIODICDID) { //运行的周期DID+请求的周期DID>限定的最大周期DID数
			NRC = ROOR_31;
		}
		else {
			//for循环使用参数TmpIndex-对请求的有效PDID进行激活,周期重置;使用参数NewDIDCounter-对新增的有效PDID进行激活，周期重置，对已经激活的PDID不操作
			for (i = 0; i < TmpIndex; i++) {
				for (j = 0; j < NumberofReadDID; j++) {
					if (ReadDIDTable[TempDIDTable[i]].DataIdentifier == ReadDIDTable[j].DataIdentifier) {
						ReadDIDTable[j].Support = UDS_TRUE;
						ReadDIDTable[j].TransmitTime = TRANSMISSIONMODE_SLOWRATE / PERIODICDIDDATARUNPERIOD;
						ReadDIDTable[j].TransmitCounter = 0;
					}
				}
			}			
			NumofPeriodicDIDRunning += NewDIDCounter;
			ResponseDataLength = 1;
		}
		
		if (UDS_TRUE == SuppressPosRspMsgIndicationBit) { //抑制正响应
			ConfirmPeridDIDSend = UDS_TRUE;
		}
		else {
			WaitConfirmBeforePeridDIDSend = UDS_TRUE;
		}
	}
}

static void SendAtMediumRate_02Proc(void) {
	uds_uint8_t i, j, k, DIDNumber, Tmp = 0, NewDIDCounter = 0, sts = UDS_FALSE;
	uds_uint8_t TempDIDTable[MAXNUMBEROFPERIODICDID], TmpIndex = 0;

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length < 3) {
			NRC = IMLOIF_13;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		DIDNumber = UDS_A_Indication_SDU.Length - 2;
		for(i = 0; i < DIDNumber; i++) {
			for(j = 0; j < NumberofReadDID; j++) {
				if ((0xF200 | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[i + 1]) == ReadDIDTable[j].DataIdentifier) {
					Tmp++;					
					
					/*重复的周期DID请求判断*/
					for(k = 0; k < TmpIndex; k++) {
						if(TempDIDTable[k] == j) {
							sts = UDS_TRUE;
						}
					}

					if (UDS_FALSE == sts) {
						TempDIDTable[TmpIndex++] = j;
						if (UDS_FALSE == ReadDIDTable[j].Support) {
							NewDIDCounter ++;
						}
					}
					sts = UDS_FALSE;
					break;
				}
			}
		}

		if(0 == Tmp) {
			NRC = ROOR_31;
		}
		else if ((NumofPeriodicDIDRunning + NewDIDCounter) > MAXNUMBEROFPERIODICDID) { //运行的周期DID+请求的周期DID>限定的最大周期DID数
			NRC = ROOR_31;
		}
		else {
			//for循环使用参数TmpIndex-对请求的有效PDID进行激活,周期重置;使用参数NewDIDCounter-对新增的有效PDID进行激活，周期重置，对已经激活的PDID不操作
			for (i = 0; i < TmpIndex; i++) {
				for (j = 0; j < NumberofReadDID; j++) {
					if (ReadDIDTable[TempDIDTable[i]].DataIdentifier == ReadDIDTable[j].DataIdentifier) {
						ReadDIDTable[j].Support = UDS_TRUE;
						ReadDIDTable[j].TransmitTime = TRANSMISSIONMODE_MEDIUMRATE / PERIODICDIDDATARUNPERIOD;
						ReadDIDTable[j].TransmitCounter = 0;
					}
				}
			}
			NumofPeriodicDIDRunning += NewDIDCounter;
			ResponseDataLength = 1;
		}
		
		if (UDS_TRUE == SuppressPosRspMsgIndicationBit) { //抑制正响应
			ConfirmPeridDIDSend = UDS_TRUE;
		}
		else {
			WaitConfirmBeforePeridDIDSend = UDS_TRUE;
		}
	}
}

static void SendAtFastRate_03Proc(void) {
	uds_uint8_t i, j, k, DIDNumber, Tmp = 0, NewDIDCounter = 0, sts = UDS_FALSE;
	uds_uint8_t TempDIDTable[MAXNUMBEROFPERIODICDID], TmpIndex = 0;

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length < 3) {
			NRC = IMLOIF_13;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		DIDNumber = UDS_A_Indication_SDU.Length - 2;
		for(i = 0; i < DIDNumber; i++) {
			for(j = 0; j < NumberofReadDID; j++) {
				if ((0xF200 | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[i + 1]) == ReadDIDTable[j].DataIdentifier) {
					Tmp++;					
					
					/*重复的周期DID请求判断*/
					for(k = 0; k < TmpIndex; k++) {
						if(TempDIDTable[k] == j) {
							sts = UDS_TRUE;
						}
					}

					if (UDS_FALSE == sts) {
						TempDIDTable[TmpIndex++] = j;
						if (UDS_FALSE == ReadDIDTable[j].Support) {
							NewDIDCounter ++;
						}
					}
					sts = UDS_FALSE;
					break;
				}
			}
		}

		if(0 == Tmp) {
			NRC = ROOR_31;
		}
		else if ((NumofPeriodicDIDRunning + NewDIDCounter) > MAXNUMBEROFPERIODICDID) { //运行的周期DID+请求的周期DID>限定的最大周期DID数
			NRC = ROOR_31;
		}
		else {
			//for循环使用参数TmpIndex-对请求的有效PDID进行激活,周期重置;使用参数NewDIDCounter-对新增的有效PDID进行激活，周期重置，对已经激活的PDID不操作
			for (i = 0; i < TmpIndex; i++) {
				for (j = 0; j < NumberofReadDID; j++) {
					if (ReadDIDTable[TempDIDTable[i]].DataIdentifier == ReadDIDTable[j].DataIdentifier) {
						ReadDIDTable[j].Support = UDS_TRUE;
						ReadDIDTable[j].TransmitTime = TRANSMISSIONMODE_FASTRATE / PERIODICDIDDATARUNPERIOD;
						ReadDIDTable[j].TransmitCounter = 0;
					}
				}
			}
			NumofPeriodicDIDRunning += NewDIDCounter;
			ResponseDataLength = 1;
		}
		
		if (UDS_TRUE == SuppressPosRspMsgIndicationBit) { //抑制正响应
			ConfirmPeridDIDSend = UDS_TRUE;
		}
		else {
			WaitConfirmBeforePeridDIDSend = UDS_TRUE;
		}
	}
}

static void StopSending_04Proc(void) {
	uds_uint8_t i, j, k, DIDNumber, Tmp = 0, NewDIDCounter = 0, sts = UDS_FALSE;
	uds_uint8_t TempDIDTable[MAXNUMBEROFPERIODICDID], TmpIndex = 0;

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length < 2) {
			NRC = IMLOIF_13;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		DIDNumber = UDS_A_Indication_SDU.Length - 2;
		if (0 == DIDNumber) {
			for(i = 0; i < NumberofReadDID; i++) {
				ReadDIDTable[i].Support = UDS_FALSE;
				ReadDIDTable[i].TransmitTime = 0;
				ReadDIDTable[i].TransmitCounter = 0;
			}
			NumofPeriodicDIDRunning = 0;
			ResponseDataLength = 1;
		}
		else {
			for(i = 0; i < DIDNumber; i++) {
				for(j = 0; j < NumberofReadDID; j++) {
					if ((0xF200 | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[i + 1]) == ReadDIDTable[j].DataIdentifier) {
						Tmp ++;

						/*重复的周期DID请求判断*/
						for(k = 0; k < TmpIndex; k++) {
							if(TempDIDTable[k] == j) {
								sts = UDS_TRUE;
							}
						}

						if (UDS_FALSE == sts) {
							TempDIDTable[TmpIndex++] = j;
							if (UDS_TRUE == ReadDIDTable[j].Support) {
								NewDIDCounter ++;
							}
						}
						sts = UDS_FALSE;
						break;
					}
				}
			}

			if(0 == Tmp) {
				NRC = ROOR_31;
			}
			else {
				//for循环使用参数TmpIndex-对请求的有效PDID进行停止,周期清零;使用参数NewDIDCounter-对新增的有效PDID进行停止,周期清零.对已经停止的PDID不操作
				for (i = 0; i < TmpIndex; i++) {
					for (j = 0; j < NumberofReadDID; j++) {
						if (ReadDIDTable[TempDIDTable[i]].DataIdentifier == ReadDIDTable[j].DataIdentifier) {
							ReadDIDTable[j].Support = UDS_FALSE;
							ReadDIDTable[j].TransmitTime = 0;
							ReadDIDTable[j].TransmitCounter = 0;
						}
					}
				}
				NumofPeriodicDIDRunning -= NewDIDCounter;
				ResponseDataLength = 1;
			}
		}

		if (0 == NumofPeriodicDIDRunning) {
			ConfirmPeridDIDSend = UDS_FALSE;
		}
	}
}

static void Service2CHandle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;
	
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service2CTable[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service2CTable[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}	
}

static void DefineByIdentifier_01Proc(void) {
	uds_uint16_t dynamicallyDefinedDataIdentifier, sourceDataIdentifier, dynamicalDIDLength = 0, DefinedDynamicalDIDDataLength = 0;
	uds_uint8_t sourceDataIdentifiernum = 0, position = 0, memorySize = 0, sts = NoDefine, Tmp = 0;
	uds_uint8_t TempDIDTable[MAX_SOURCEDATARECORD_NUMBER], TmpIndex = 0, ConfigPDIDnum;
	ReadDatabyInentifier_t LastPDID;
	
	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if ((UDS_A_Indication_SDU.Length < 8) || (UDS_A_Indication_SDU.Length % 4 != 0)) {
			NRC = IMLOIF_13;
			return;
		}

		/*DID分配错误*/
		if ((UDS_A_Indication_SDU.A_SDU[1] != 0xf2) && (UDS_A_Indication_SDU.A_SDU[1] != 0xf3)) {
			NRC = ROOR_31;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		dynamicallyDefinedDataIdentifier = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[1] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2];
		sourceDataIdentifiernum = (UDS_A_Indication_SDU.Length - 4) / 4;
		
		/*请求的DID类型区分-未定义、静态、动态*/
		for (uds_uint8_t i = 0; i < NumberofReadDID; i++) {
			if (dynamicallyDefinedDataIdentifier == ReadDIDTable[i].DataIdentifier) {
				if (UDS_FALSE == ReadDIDTable[i].DynamicalIdentifierBit) {
					sts = StaticDefine;
				}
				else {
					sts = DynamicalDefine;
					LastPDID = ReadDIDTable[i];
					DefinedDynamicalDIDDataLength += ReadDIDTable[i].DataLength;
				}
			}
		}

		if (StaticDefine == sts) { //Static DID不可再分配
			NRC = ROOR_31;
		}
		else {

			/*sourceData有效性判断*/
			for (uds_uint8_t i = 0; i < sourceDataIdentifiernum; i++) {				
				position = UDS_A_Indication_SDU.A_SDU[i * 4 + 5];
				if ((position < 1) || (position > MAX_SOURCEDATARECORD_POSITION)) {
					NRC = ROOR_31; //optionl
					break;
				}
				sourceDataIdentifier = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[i * 4 + 3] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[i * 4 + 4];
				memorySize = UDS_A_Indication_SDU.A_SDU[i * 4 + 6];
				for (uds_uint8_t j = 0; j < NumberofSourceDataRecord; j++) {
					if ((SourceDataRecordTable[j].SourceDataIdentifier == sourceDataIdentifier) 
						 && (SourceDataRecordTable[j].PositionInSourceDataRecord == position) 
						 && (SourceDataRecordTable[j].MemorySize == memorySize)) {
						Tmp++;
						TempDIDTable[TmpIndex++] = j;
						dynamicalDIDLength += memorySize;
					}
				}
			}

			if (sourceDataIdentifiernum != Tmp) { //请求的sourceData有效性
				NRC = ROOR_31;
			}
			else if ((3 + DefinedDynamicalDIDDataLength + dynamicalDIDLength) > UDS_RECIVE_BUFFER_LENGTH) { //Server发送数据最大长度,SID+DID = 3
				NRC = ROOR_31;
			}
			else if ((0xF2 == UDS_A_Indication_SDU.A_SDU[1]) && (dynamicalDIDLength + DefinedDynamicalDIDDataLength > MAX_PERIODICDID_DATALENGTH)) { //PDID数据长度
				NRC = ROOR_31;
			}
			else {
				for (uds_uint8_t i = 0; i < TmpIndex; i++) {
					AddDID_Init(dynamicallyDefinedDataIdentifier, SourceDataRecordTable[TempDIDTable[i]].DataPointer, SourceDataRecordTable[TempDIDTable[i]].MemorySize, UDS_TRUE, ReadOnly, NO_TAG, UDS_FALSE);
					if (UDS_TRUE == LastPDID.Support)  {
						ReadDIDTable[NumberofReadDID - 1].Support = LastPDID.Support;
						ReadDIDTable[NumberofReadDID - 1].TransmitTime = LastPDID.TransmitTime;
					#if DYNAMICALLY_DEFINE_PDID_COUNTER
						ReadDIDTable[NumberofReadDID - 1].TransmitCounter = LastPDID.TransmitCounter; //保持之前PDID的计数器状态						
					#else
						ReadDIDTable[NumberofReadDID - 1].TransmitCounter = 0; //PDID的计数器重置
					#endif
					}
				} 	

				/*响应数组填充*/
				SDUSendBuffArray[1] = DefineByIdentifier;
				SDUSendBuffArray[2] = UDS_A_Indication_SDU.A_SDU[1];
				SDUSendBuffArray[3] = UDS_A_Indication_SDU.A_SDU[2];
				ResponseDataLength = 4;
			}
		}	
	}
}

static void DefineByMemoryAddress_02Proc(void) {
	uds_uint8_t memorySizeBytes = (UDS_A_Indication_SDU.A_SDU[3] >> 4) & 0x0f, memoryAddressBytes = UDS_A_Indication_SDU.A_SDU[3] & 0x0f;
	uds_uint8_t* memoryAddress, memorySize;
	uds_uint16_t dynamicallyDefinedDataIdentifier, dynamicalDIDLength = 0, length = 4, DefinedDynamicalDIDDataLength = 0;
	uds_uint8_t Addressnum = 0, sts = NoDefine, Tmp = 0, ConfigPDIDnum;
	ReadDatabyInentifier_t LastPDID;

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if ((UDS_A_Indication_SDU.Length < 7) || ((UDS_A_Indication_SDU.Length - 5) % 2 != 0)) {
			NRC = IMLOIF_13;
			return;
		}

		/*DID分配错误、addressAndLengthFormatIdentifier错误*/
		if ((UDS_A_Indication_SDU.A_SDU[1] != 0xf2) && (UDS_A_Indication_SDU.A_SDU[1] != 0xf3)
			|| (0 == memorySizeBytes) || (0 == memoryAddressBytes) || (memorySizeBytes > 4) || (memoryAddressBytes > 4)) { //(memorySizeBytes > 服务器支持的最大值)，地址有效性检查等待添加
			NRC = ROOR_31;
			return;
		}

		/*安全等级检查*/
		if (SystemState.SecurityLevel != LEVEL_ONE) {
			NRC = SAD_33;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		dynamicallyDefinedDataIdentifier = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[1] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2];
		Addressnum = (UDS_A_Indication_SDU.Length - 5) / (memorySizeBytes + memoryAddressBytes);
		
		/*请求的DID类型区分-未定义、静态、动态*/
		for (uds_uint8_t i = 0; i < NumberofReadDID; i++) {
			if (dynamicallyDefinedDataIdentifier == ReadDIDTable[i].DataIdentifier) {
				if (UDS_FALSE == ReadDIDTable[i].DynamicalIdentifierBit) {
					sts = StaticDefine;
				}
				else {
					sts = DynamicalDefine;
					LastPDID = ReadDIDTable[i];
					DefinedDynamicalDIDDataLength += ReadDIDTable[i].DataLength;
				}
			}
		}

		if (StaticDefine == sts) { //Static DID不可再分配
			NRC = ROOR_31;
		}
		else {

			/*Address有效性判断*/
			for (uds_uint8_t i = 0; i < Addressnum; i++) {	
				memoryAddress = ReadMemoryByAddress_Func((&UDS_A_Indication_SDU.A_SDU[length]), memoryAddressBytes);
				memorySize = ReadMemorySize_Func(&UDS_A_Indication_SDU.A_SDU[memoryAddressBytes + length], memorySizeBytes);		
				length = length + memoryAddressBytes + memorySizeBytes;

				/*此处需要添加Tester请求地址与Server本地地址的映射关系*/

				/*地址有效性检查-待添加*/
				if (1) {
					Tmp++;
					dynamicalDIDLength += memorySize;
				}
			}

			if (Addressnum != Tmp) { //请求的Address有效性
				NRC = ROOR_31;
			}
			else if ((3 + DefinedDynamicalDIDDataLength + dynamicalDIDLength) > UDS_RECIVE_BUFFER_LENGTH) { //Server发送数据最大长度,SID+DID = 3
				NRC = ROOR_31;
			}
			else if ((0xF2 == UDS_A_Indication_SDU.A_SDU[1]) && (dynamicalDIDLength + DefinedDynamicalDIDDataLength > MAX_PERIODICDID_DATALENGTH)) { //PDID数据长度
				NRC = ROOR_31;
			}
			else {
				length = 4;
				for (uds_uint8_t i = 0; i < Addressnum; i++) {
					memoryAddress = ReadMemoryByAddress_Func((&UDS_A_Indication_SDU.A_SDU[length]), memoryAddressBytes);
				
					/*此处需要添加Tester请求地址与Server本地地址的映射关系*/
				
					memorySize = ReadMemorySize_Func(&UDS_A_Indication_SDU.A_SDU[memoryAddressBytes + length], memorySizeBytes);
					length = length + memoryAddressBytes + memorySizeBytes;
					AddDID_Init(dynamicallyDefinedDataIdentifier, memoryAddress, memorySize, UDS_TRUE, ReadOnly, NO_TAG, UDS_FALSE);
					if (UDS_TRUE == LastPDID.Support)  {
						ReadDIDTable[NumberofReadDID - 1].Support = LastPDID.Support;
						ReadDIDTable[NumberofReadDID - 1].TransmitTime = LastPDID.TransmitTime;
					#if DYNAMICALLY_DEFINE_PDID_COUNTER
						ReadDIDTable[NumberofReadDID - 1].TransmitCounter = LastPDID.TransmitCounter; //保持之前PDID的计数器状态						
					#else
						ReadDIDTable[NumberofReadDID - 1].TransmitCounter = 0; //PDID的计数器重置
					#endif				
					}
				}
				
				/*响应数组填充*/
				SDUSendBuffArray[1] = DefineByMemoryAddress;
				SDUSendBuffArray[2] = UDS_A_Indication_SDU.A_SDU[1];
				SDUSendBuffArray[3] = UDS_A_Indication_SDU.A_SDU[2];
				ResponseDataLength = 4;
			}
		}	
	}
}

static void ClearDynamicallyDefinedDataIdentifier_03Proc(void) {
	uds_uint8_t DIDNumber;
	uds_uint16_t TargetDynamicalDID;

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if ((UDS_A_Indication_SDU.Length != 2) && (UDS_A_Indication_SDU.Length != 4)) {
			NRC = IMLOIF_13;
			return;
		}

		/*DID分配错误*/
		if ((UDS_A_Indication_SDU.A_SDU[1] != 0xf2) && (UDS_A_Indication_SDU.A_SDU[1] != 0xf3)) {
			NRC = ROOR_31;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}
	
	/*条件检查通过，服务处理*/
	{
		if (2 == UDS_A_Indication_SDU.Length) { //清除所有动态DID
			NumberofReadDID = NumberofStaticReadDID;
		}
		else { //清除指定动态DID
			TargetDynamicalDID = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[1] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2];

			/*判断指定DID是否为PDID*/
			for (uds_uint8_t i = 0; i < NumberofReadDID; i++) {
				if (TargetDynamicalDID == ReadDIDTable[i].DataIdentifier) {
					if (UDS_TRUE == ReadDIDTable[i].Support) {
						NumofPeriodicDIDRunning--;
						break;
					}
				}
			}
			
			ClearDynamicalDataIdentifier_Func(TargetDynamicalDID); // 清除动态DID

			/*只要指定动态DID格式符合规范，即使ECU不存在此动态DID，也作出正响应*/
			SDUSendBuffArray[2] = UDS_A_Indication_SDU.A_SDU[1];
			SDUSendBuffArray[3] = UDS_A_Indication_SDU.A_SDU[2];
		}

		/*响应数组填充*/
		SDUSendBuffArray[1] = ClearDynamicallyDefinedDataIdentifier;
		ResponseDataLength = UDS_A_Indication_SDU.Length;
	}
	
}

static void ClearDynamicalDataIdentifier_Func(uds_uint16_t DynamicalDID) {
#if 0
	/*首元素为指定元素*/
	while ((ReadDIDTable[0].DataIdentifier == DynamicalDID) && (NumberofReadDID > 0)) {
		for (uds_uint8_t i = 0; i < (NumberofReadDID - 1); i++) {
			ReadDIDTable[i] = ReadDIDTable[i + 1];
		}
		NumberofReadDID--;
	}

	for (uds_uint8_t i = 1; i < NumberofReadDID; i++) {
		if (ReadDIDTable[i].DataIdentifier == DynamicalDID) {
			UDS_PRINTF("DynamicalDID = %#x\n", DynamicalDID);
			for (uds_uint8_t j = i; j < NumberofReadDID; j++) {
				ReadDIDTable[j] = ReadDIDTable[j + 1];
			}
			NumberofReadDID--;
			i--;
		}		
	}
#else
	for (uds_uint8_t i = 0; i < NumberofReadDID;) {
		if (ReadDIDTable[i].DataIdentifier == DynamicalDID) {
			for (uds_uint8_t j = i; j < NumberofReadDID; j++) {
				ReadDIDTable[j] = ReadDIDTable[j + 1];
			}
			NumberofReadDID--;
		}
		else {
			i++;
		}
	}
#endif
}

static void Service2EHandle(void) {
	uds_uint16_t DID = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[0] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[1];
	ReadDatabyInentifier_t* TempDID = SearchDID_Func(DID);

	/*错误条件检查*/
	{
	
		/*请求的DID不支持*/
		if (UDS_A_NULL == TempDID) {
			NRC = ROOR_31;
			return;
		}

		/*服务长度错误*/
		if (UDS_A_Indication_SDU.Length != (TempDID->DataLength + 3)) {
			NRC = IMLOIF_13;
			return;
		}

		/*操作条件不满足-待添加(一般包括车辆静止，车速为0等)*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		if (Write_RAM == TempDID->DataIdentifierType) {
			if (FactorySeesion == SystemState.SessionMode) { //出厂模式不支持写RAM
				NRC = ROOR_31;
			}
			else {
				memcpy(TempDID->DataPointer, UDS_A_Indication_SDU.A_SDU + 2, TempDID->DataLength);
			}
		}
		else if (Write_EEPROM == TempDID->DataIdentifierType){
			if (UDS_TRUE == TempDID->FactoryConfigSupport) { //下线配置
				if (FactorySeesion == SystemState.SessionMode) { //出厂会话才可写入
					if (LEVEL_ONE == SystemState.SecurityLevel) {
						//UDS_A_eeprom_write(TempDID->DataIdentifierTag, UDS_A_Indication_SDU.A_SDU + 2);
						UDS_A_Data_Save(TempDID->DataIdentifierTag, UDS_A_Indication_SDU.A_SDU + 2, TempDID->DataLength);
						
						//test code
						//memcpy(TempDID->DataPointer, UDS_A_Indication_SDU.A_SDU + 2, TempDID->DataLength);
					}
					else {
						NRC = SAD_33;
					}
				}
				else {
					NRC = ROOR_31;
				}
			}
			else { //DID正常写数据
				if (FactorySeesion == SystemState.SessionMode) {
					NRC = ROOR_31;
				}
				else { //非出厂会话下才可写入
					//UDS_A_eeprom_write(TempDID->DataIdentifierTag, UDS_A_Indication_SDU.A_SDU + 2);
					UDS_A_Data_Save(TempDID->DataIdentifierTag, UDS_A_Indication_SDU.A_SDU + 2, TempDID->DataLength);

					//test code
					//memcpy(TempDID->DataPointer, UDS_A_Indication_SDU.A_SDU + 2, TempDID->DataLength);
				}
			}
		}
		else {
			NRC = ROOR_31;
		}

		if (PR_00 == NRC) {
			SDUSendBuffArray[1] = UDS_A_Indication_SDU.A_SDU[0];
			SDUSendBuffArray[2] = UDS_A_Indication_SDU.A_SDU[1];
			ResponseDataLength = 3;
		}
	}	
}

static ReadDatabyInentifier_t* SearchDID_Func(uds_uint16_t DID) {
	uds_uint8_t i;
	for(i = 0; i < NumberofReadDID; i++) {
		if(ReadDIDTable[i].DataIdentifier == DID) {
			return ReadDIDTable + i;
		}
	}
	return UDS_A_NULL;
}

/*2Fh-输入输出控制服务*/
static void Service2FHandle(void) {
	uds_uint16_t DID = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[0] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[1];
	ReadDatabyInentifier_t* TempDID = SearchDID_Func(DID);
	uds_uint8_t InputOutputControlParameter;
	union {
		uds_uint8_t controlMask;
		struct {
		uds_uint8_t : 6;
		uds_uint8_t Voltage_Mask : 1;
		uds_uint8_t VehicleSpeed_Mask : 1;
		} mask_t;
	} mask_u;

	/*错误条件检查*/
	{

		/*请求的DID不支持-请求的InputOutputControlParameter不支持*/
		if ((UDS_A_NULL == TempDID) || (UDS_A_Indication_SDU.A_SDU[2] > shortTermAdjustment)) {
			NRC = ROOR_31;
			return;
		}

		/*操作条件不满足-待添加(一般包括车辆静止，车速为0等)*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}
	
	/*以下数据处理目的在于体现服务处理流程，2F服务取决于系统供应商，自由度比较大*/
	/*条件检查通过，服务处理*/
	{
		InputOutputControlParameter = UDS_A_Indication_SDU.A_SDU[2];
		if (Write_RAM_IO == TempDID->DataIdentifierType) {
			switch (TempDID->DataIdentifier) {
				case INPUTOUTPUTCONTROL_DID0: //DID包含一个参数
					if (shortTermAdjustment == InputOutputControlParameter) {
						if ((TempDID->DataLength + 4) == UDS_A_Indication_SDU.Length) {
							IO_ControlPara.EngineRPM = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[3] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[4];
							if ((IO_ControlPara.EngineRPM >= 0) && (IO_ControlPara.EngineRPM <= 8000)) {
								IO_ControlPara_Flag_u.Flag_t.EngineRPM_Flag = UDS_TRUE;
							}
							else {
								NRC = ROOR_31;
							}
						}
						else {
							NRC = IMLOIF_13;
						}
					}
					else {
						if (4 == UDS_A_Indication_SDU.Length) {
							if (returnControlToECU == InputOutputControlParameter) {
								IO_ControlPara_Flag_u.Flag_t.EngineRPM_Flag = UDS_FALSE;
							}
							else if (resetToDefault == InputOutputControlParameter) {
								IO_ControlPara_Flag_u.Flag_t.EngineRPM_Flag = UDS_TRUE;
								IO_ControlPara.EngineRPM = ENGINE_RPM_DEFAULT;
							}
							else if (freezeCurrentState == InputOutputControlParameter) {
								IO_ControlPara_Flag_u.Flag_t.EngineRPM_Flag = UDS_TRUE;
								IO_ControlPara.EngineRPM = EngineRPM;
							}
							else {
								NRC = ROOR_31; //fail-safe
							}
						}
						else {
							NRC = IMLOIF_13;
						}
					}

					if (PR_00 == NRC) {
						SDUSendBuffArray[1] = UDS_A_Indication_SDU.A_SDU[0]; //DID
						SDUSendBuffArray[2] = UDS_A_Indication_SDU.A_SDU[1]; //DID
						SDUSendBuffArray[3] = UDS_A_Indication_SDU.A_SDU[2]; //Para
						SDUSendBuffArray[4] = (uds_uint8_t)(EngineRPM >> 8); //controlState
						SDUSendBuffArray[5] = (uds_uint8_t)EngineRPM;
						ResponseDataLength = 6;
					}
					break;
				case INPUTOUTPUTCONTROL_DID1: //DID包含多个参数
					if (shortTermAdjustment == InputOutputControlParameter) {
						mask_u.controlMask = UDS_A_Indication_SDU.A_SDU[TempDID->DataLength + 3]; //此DID包含两个参数，controlMask占2bit,1Byte
						if ((TempDID->DataLength + 4 + 1) == UDS_A_Indication_SDU.Length) {
							if (UDS_TRUE == mask_u.mask_t.VehicleSpeed_Mask) {
								IO_ControlPara.VehicleSpeed = UDS_A_Indication_SDU.A_SDU[3];
								if ((IO_ControlPara.VehicleSpeed >= 0) && (IO_ControlPara.VehicleSpeed <= 240)) {
									IO_ControlPara_Flag_u.Flag_t.VehicleSpeed_Flag = UDS_TRUE;
								}
								else {
									NRC = ROOR_31;
								}								
							}

							if (UDS_TRUE == mask_u.mask_t.Voltage_Mask) {
								IO_ControlPara.Voltage = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[4] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[5];
								if ((IO_ControlPara.Voltage >= 0) && (IO_ControlPara.Voltage <= 1800)) {
									IO_ControlPara_Flag_u.Flag_t.Voltage_Flag = UDS_TRUE;
								}
								else {
									NRC = ROOR_31;
								}
							}
						}
						else {
							NRC = IMLOIF_13;
						}
					}
					else {
						mask_u.controlMask = UDS_A_Indication_SDU.A_SDU[3];
						if ((4 + 1) == UDS_A_Indication_SDU.Length) {
							if (returnControlToECU == InputOutputControlParameter) {
								if (UDS_TRUE == mask_u.mask_t.VehicleSpeed_Mask) {
									IO_ControlPara_Flag_u.Flag_t.VehicleSpeed_Flag = UDS_FALSE;
								}

								if (UDS_TRUE == mask_u.mask_t.Voltage_Mask) {
									IO_ControlPara_Flag_u.Flag_t.Voltage_Flag = UDS_FALSE;
								}
							}
							else if (resetToDefault == InputOutputControlParameter) {
								if (UDS_TRUE == mask_u.mask_t.VehicleSpeed_Mask) {
									IO_ControlPara_Flag_u.Flag_t.VehicleSpeed_Flag = UDS_TRUE;
									IO_ControlPara.VehicleSpeed = VEHICLE_SPEED_DEFAULT;
								}

								if (UDS_TRUE == mask_u.mask_t.Voltage_Mask) {
									IO_ControlPara_Flag_u.Flag_t.Voltage_Flag = UDS_TRUE;
									IO_ControlPara.Voltage = VOLTAGE_DEFAULT;
								}
							}
							else if (freezeCurrentState == InputOutputControlParameter) {
								if (UDS_TRUE == mask_u.mask_t.VehicleSpeed_Mask) {
									IO_ControlPara_Flag_u.Flag_t.VehicleSpeed_Flag = UDS_TRUE;
									IO_ControlPara.VehicleSpeed = VehicleSpeed;
								}

								if (UDS_TRUE == mask_u.mask_t.Voltage_Mask) {
									IO_ControlPara_Flag_u.Flag_t.Voltage_Flag = UDS_TRUE;
									IO_ControlPara.Voltage = Voltage;
								}
							}
							else {
								NRC = ROOR_31; //fail-safe
							}
						}
						else {
							NRC = IMLOIF_13;
						}
					}			
					
					if (PR_00 == NRC) {
						SDUSendBuffArray[1] = UDS_A_Indication_SDU.A_SDU[0]; //DID
						SDUSendBuffArray[2] = UDS_A_Indication_SDU.A_SDU[1]; //DID
						SDUSendBuffArray[3] = UDS_A_Indication_SDU.A_SDU[2]; //Para
						SDUSendBuffArray[4] = (uds_uint8_t)VehicleSpeed; //controlState
						SDUSendBuffArray[5] = (uds_uint8_t)(Voltage >> 8);
						SDUSendBuffArray[6] = (uds_uint8_t)Voltage;
						ResponseDataLength = TempDID->DataLength + 4;
					}
					break;
				default:
					NRC = ROOR_31; //fail-safe
			}
		}
		else {
			NRC = ROOR_31;
		}	
	}	
}

static void IO_ControlParameter_Test(void) {
	if (UDS_TRUE == IO_ControlPara_Flag_u.Flag_t.EngineRPM_Flag) {
		EngineRPM = IO_ControlPara.EngineRPM;
	}
	else {
		EngineRPM = ENGINERPM_TEST;
	}
	
	if (UDS_TRUE == IO_ControlPara_Flag_u.Flag_t.VehicleSpeed_Flag) {
		VehicleSpeed = IO_ControlPara.VehicleSpeed;
	}
	else {
		VehicleSpeed = VEHICLESPEED_TEST;
	}

	if (UDS_TRUE == IO_ControlPara_Flag_u.Flag_t.Voltage_Flag) {
		Voltage = IO_ControlPara.Voltage;
	}
	else {
		Voltage = VOLTAGE_TEST;
	}
}

static void Service31Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;
	
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service31Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service31Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

static Routine_t* SearchRoutineID_Func(uds_uint16_t RoutineID) {
	for(uds_uint8_t i = 0; i < NumberofRoutine; i++) {
		if (RoutineTable[i].RoutineIdentifier == RoutineID) {
			return RoutineTable + i;
		}
	}
	return UDS_A_NULL;
}

static void StartRoutine_01Proc(void) {
	uds_uint16_t RoutineID = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[1] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2];
	Routine_t* TempRoutineID = SearchRoutineID_Func(RoutineID);
	
	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != (4 + TempRoutineID->RoutineControlOptionRecordDataLength)) {
			NRC = IMLOIF_13;
			return;
		}

		/*routineIdentifier不存在*/
		if (UDS_A_NULL == TempRoutineID) { //暂未添加RoutineControlOptionRecord有效性检查
			NRC = ROOR_31;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}

		/*请求序列检查-例程不可重启，例程启动进行中*/
		if ((UDS_FALSE == TempRoutineID->RestartRoutineSupport) 
			&& ((Routine_start_successfully == TempRoutineID->RoutineStatus) || (Routine_inprogress == TempRoutineID->RoutineStatus))) {
			NRC = RSE_24;
			return;
		}
	}
	
	/*条件检查通过，服务处理*/
	{
		TempRoutineID->RoutineStatus = TempRoutineID->RoutineRun_Func(StartRoutine);
		SDUSendBuffArray[1] = StartRoutine;
		SDUSendBuffArray[2] = UDS_A_Indication_SDU.A_SDU[1]; //RoutineID
		SDUSendBuffArray[3] = UDS_A_Indication_SDU.A_SDU[2]; //RoutineID

		//Test
		for (uds_uint8_t i = 0; i < TempRoutineID->RoutineStatusRecordDataLength; i++) {
			SDUSendBuffArray[4 + i] = UDS_A_Indication_SDU.A_SDU[3 + i];
		}
		ResponseDataLength = 4 + TempRoutineID->RoutineStatusRecordDataLength;
	}	
}

static void StopRoutine_02Proc(void) {
	uds_uint16_t RoutineID = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[1] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2];
	Routine_t* TempRoutineID = SearchRoutineID_Func(RoutineID);
	
	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != (4 + TempRoutineID->RoutineControlOptionRecordDataLength)) {
			NRC = IMLOIF_13;
			return;
		}

		/*routineIdentifier不存在*/
		if (UDS_A_NULL == TempRoutineID) { //暂未添加RoutineControlOptionRecord有效性检查
			NRC = ROOR_31;
			return;
		}

		/*子服务不支持*/
		if (UDS_FALSE == TempRoutineID->StopRoutineSupport) {
			NRC = SFNS_12;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}

		/*请求序列检查-例程未运行或例程已完成*/
		if ((TempRoutineID->RoutineStatus != Routine_start_successfully) && (TempRoutineID->RoutineStatus != Routine_inprogress)) {
			NRC = RSE_24;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		TempRoutineID->RoutineStatus = TempRoutineID->RoutineRun_Func(StopRoutine);
		SDUSendBuffArray[1] = StopRoutine;
		SDUSendBuffArray[2] = UDS_A_Indication_SDU.A_SDU[1]; //RoutineID
		SDUSendBuffArray[3] = UDS_A_Indication_SDU.A_SDU[2]; //RoutineID

		//Test
		for (uds_uint8_t i = 0; i < TempRoutineID->RoutineStatusRecordDataLength; i++) {
			SDUSendBuffArray[4 + i] = UDS_A_Indication_SDU.A_SDU[3 + i];
		}
		ResponseDataLength = 4 + TempRoutineID->RoutineStatusRecordDataLength;
	}	
}

static void RequestRoutineResults_03Proc(void) {
	uds_uint16_t RoutineID = ((uds_uint16_t)UDS_A_Indication_SDU.A_SDU[1] << 8) | (uds_uint16_t)UDS_A_Indication_SDU.A_SDU[2];
	Routine_t* TempRoutineID = SearchRoutineID_Func(RoutineID);
	
	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 4) {
			NRC = IMLOIF_13;
			return;
		}

		/*routineIdentifier不存在*/
		if (UDS_A_NULL == TempRoutineID) { //暂未添加RoutineControlOptionRecord有效性检查
			NRC = ROOR_31;
			return;
		}

		/*子服务不支持*/
		if (UDS_FALSE == TempRoutineID->RequestRoutineResultsSupport) {
			NRC = SFNS_12;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}

		/*请求序列检查-例程未运行或例程已完成*/
		if (TempRoutineID->RoutineStatus != Routine_Completed_successfully) {
			NRC = RSE_24;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		TempRoutineID->RoutineStatus = TempRoutineID->RoutineRun_Func(RequestRoutineResults);
		SDUSendBuffArray[1] = RequestRoutineResults;
		SDUSendBuffArray[2] = UDS_A_Indication_SDU.A_SDU[1]; //RoutineID
		SDUSendBuffArray[3] = UDS_A_Indication_SDU.A_SDU[2]; //RoutineID

		//Test
		for (uds_uint8_t i = 0; i < TempRoutineID->RoutineStatusRecordDataLength; i++) {
			SDUSendBuffArray[4 + i] = UDS_A_Indication_SDU.A_SDU[3 + i];
		}
		ResponseDataLength = 4 + TempRoutineID->RoutineStatusRecordDataLength;
	}
}

static RoutineProcessStatus_e RoutineRun_01Func(RoutineControlType_e RoutineControlType) {
	//Test
	switch(RoutineControlType) {
		case StartRoutine:
			if (1) {
				return Routine_start_successfully;
			}
			else if (1){
				return Routine_inprogress;
			}
			else {
				return Routine_start_unsuccessfully;
			}
			break;
		case StopRoutine:
			if (0) {
				return Routine_Completed_unsuccessfully;
			}
			else {
				return Routine_Completed_successfully;
			}
			break;
		case RequestRoutineResults:
			
			return Routine_Init;

			break;
		default:
			return Routine_Init;
	}
}

static RoutineProcessStatus_e RoutineRun_02Func(RoutineControlType_e RoutineControlType) {
	//Test
	switch(RoutineControlType) {
		case StartRoutine:
			if (1) {
				return Routine_start_successfully;
			}
			else if (1){
				return Routine_inprogress;
			}
			else {
				return Routine_start_unsuccessfully;
			}
			break;
		case StopRoutine:
			if (0) {
				return Routine_Completed_unsuccessfully;
			}
			else {
				return Routine_Completed_successfully;
			}
			break;
		case RequestRoutineResults:
			
			return Routine_Init;

			break;
		default:
			return Routine_Init;
	}
}

static RoutineProcessStatus_e RoutineRun_03Func(RoutineControlType_e RoutineControlType) {
	//Test
	switch(RoutineControlType) {
		case StartRoutine:
			if (1) {
				return Routine_start_successfully;
			}
			else if (1){
				return Routine_inprogress;
			}
			else {
				return Routine_start_unsuccessfully;
			}
			break;
		case StopRoutine:
			if (0) {
				return Routine_Completed_unsuccessfully;
			}
			else {
				return Routine_Completed_successfully;
			}
			break;
		case RequestRoutineResults:
			
			return Routine_Init;

			break;
		default:
			return Routine_Init;
	}
}

static void Service34Handle(void) {
	uds_uint8_t DataFormatIdentifier = UDS_A_Indication_SDU.A_SDU[0]; //高半字节表示压缩方法，低半字节表示加密方法
	uds_uint8_t memorySizeBytes = (UDS_A_Indication_SDU.A_SDU[1] >> 4) & 0x0f, memoryAddressBytes = UDS_A_Indication_SDU.A_SDU[1] & 0x0f;
	uds_uint8_t* memoryAddress;
	uds_uint32_t memorySize;

	/*错误条件检查*/
	{
	
		/*addressAndLengthFormatIdentifier错误*/
		if ((0 == memorySizeBytes) || (0 == memoryAddressBytes) || (memorySizeBytes > 4) || (memoryAddressBytes > 4)) { //(memorySizeBytes > 服务器支持的最大值)等
			NRC = ROOR_31;
			return;
		}

		/*服务长度错误*/
		if (UDS_A_Indication_SDU.Length < (memorySizeBytes + memoryAddressBytes + 3)) {
			NRC = IMLOIF_13;
			return;
		}

		/*操作条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
		
		/*上传下载未接收-待添加*/
		if (0) {
			NRC = UDNA_70;
			return;
		}
	}
	
	/*条件检查通过，服务处理*/
	{
		memoryAddress = ReadMemoryByAddress_Func(UDS_A_Indication_SDU.A_SDU + 2, memoryAddressBytes);

		for (uds_uint8_t i = 0; i < memoryAddressBytes; i++) {
			UDS_PRINTF("memoryAddress%d = %#x\n", i, UDS_A_Indication_SDU.A_SDU[i + 2]);
		}
		
		memorySize = ReadMemorySize_Func((UDS_A_Indication_SDU.A_SDU + memoryAddressBytes + 2), memorySizeBytes);
		
		/*此处需要添加Tester请求地址与Server本地地址的映射关系，地址有效性检查，地址操作权限检查等*/

		DataTransfer.DownloadUploadStep = Transfer_Wait_Download;
		DataTransfer.DataFormatIdentifier = DataFormatIdentifier;
		DataTransfer.DataBlockSequenceCounter = 0; //数据发送块序号第一次从1开始计数
		DataTransfer.DataDownloadLength = 0; //下载数据长度置0
		DataTransfer.memoryAddress = memoryAddress;
		DataTransfer.memorySize = memorySize;

		/*响应数组填充*/
		SDUSendBuffArray[1] = 0x20; //maxNumberOfBlockLength占据2个字节,低半字节恒为0
		SDUSendBuffArray[2] = (uds_uint8_t)(MAXDATALENGTH >> 8);
		SDUSendBuffArray[3] = (uds_uint8_t)MAXDATALENGTH;
		ResponseDataLength = 4;
	}
}

static void Service35Handle(void) {
	uds_uint8_t DataFormatIdentifier = UDS_A_Indication_SDU.A_SDU[0]; //高半字节表示压缩方法，低半字节表示加密方法
	uds_uint8_t memorySizeBytes = (UDS_A_Indication_SDU.A_SDU[1] >> 4) & 0x0f, memoryAddressBytes = UDS_A_Indication_SDU.A_SDU[1] & 0x0f;
	uds_uint8_t* memoryAddress;
	uds_uint32_t memorySize;

	/*错误条件检查*/
	{
	
		/*addressAndLengthFormatIdentifier错误*/
		if ((0 == memorySizeBytes) || (0 == memoryAddressBytes) || (memorySizeBytes > 4) || (memoryAddressBytes > 4)) { //(memorySizeBytes > 服务器支持的最大值)等
			NRC = ROOR_31;
			return;
		}

		/*服务长度错误*/
		if (UDS_A_Indication_SDU.Length < (memorySizeBytes + memoryAddressBytes + 3)) {
			NRC = IMLOIF_13;
			return;
		}

		/*操作条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
		
		/*上传下载未接收-待添加*/
		if (0) {
			NRC = UDNA_70;
			return;
		}
	}
	
	/*条件检查通过，服务处理*/
	{
		memoryAddress = ReadMemoryByAddress_Func(UDS_A_Indication_SDU.A_SDU + 2, memoryAddressBytes);
		memorySize = ReadMemorySize_Func((UDS_A_Indication_SDU.A_SDU + memoryAddressBytes + 2), memorySizeBytes);

		UDS_PRINTF("memoryAddress = %#x\n", memoryAddress);
		UDS_PRINTF("memorySize = %d\n", memorySize);
		
		/*此处需要添加Tester请求地址与Server本地地址的映射关系，地址有效性检查，地址操作权限检查等*/
		
		DataTransfer.DownloadUploadStep = Transfer_Wait_Upload;
		DataTransfer.DataFormatIdentifier = DataFormatIdentifier;
		DataTransfer.DataBlockSequenceCounter = 0; //数据发送块序号第一次从1开始计数
		DataTransfer.DataUploadTimes = memorySize / (MAXDATALENGTH - 2) + (0 == (memorySize % (MAXDATALENGTH -2)) ? 0 : 1); //数据需上传的总次数
		DataTransfer.DataUploadCounter = 0; //数据上传次数计数器清零
		DataTransfer.memoryAddress = memoryAddress;
		DataTransfer.memorySize = memorySize;

		/*响应数组填充*/
		SDUSendBuffArray[1] = 0x20; //maxNumberOfBlockLength占据2个字节,低半字节恒为0
		SDUSendBuffArray[2] = (uds_uint8_t)(MAXDATALENGTH >> 8);
		SDUSendBuffArray[3] = (uds_uint8_t)MAXDATALENGTH;
		ResponseDataLength = 4;
	}
}

//写内存接口函数
static uds_uint8_t ProgramWriteData(uds_uint8_t* address, uds_uint8_t* data, uds_uint16_t length) {

	//编程ok返回ture,否则返回false
	if(1) {
		return UDS_TRUE;
	}
	else {
		return UDS_FALSE;
	}
}

static void Service36Handle(void) {
	uds_uint8_t blockSequenceCounter = UDS_A_Indication_SDU.A_SDU[0];
	uds_uint8_t transferRequestParameterRecord[MAXDATALENGTH];

	/*错误条件检查*/
	{

		/*服务长度错误*/
		if (((Transfer_Wait_Download == DataTransfer.DownloadUploadStep) && (UDS_A_Indication_SDU.Length < 3)) //下载服务，长度小于3
			|| ((Transfer_Wait_Upload == DataTransfer.DownloadUploadStep) && (UDS_A_Indication_SDU.Length != 2))){ //上传服务，长度不等于3
			NRC = IMLOIF_13;
			return;
		}
	
		/*服务请求序列错误-数据传输进程不是等待发送也不是等待上传*/
		if ((DataTransfer.DownloadUploadStep != Transfer_Wait_Download) 
			&& (DataTransfer.DownloadUploadStep != Transfer_Wait_Upload)) {
			NRC = RSE_24;
			return;
		}

		/*wrongBlockSequenceCounter-块序号不等于上次传输的块序号和上次传输的块序号加1*/
		if (((DataTransfer.DataBlockSequenceCounter & 0xff) != blockSequenceCounter) 
			&& (((DataTransfer.DataBlockSequenceCounter + 1) & 0xff) != blockSequenceCounter))	{
			NRC = WBSC_73;
			return;
		}

		/*wrongBlockSequenceCounter-第一次传输块序号为1*/
		if ((blockSequenceCounter != 1) 
			&& (((Transfer_Wait_Download == DataTransfer.DownloadUploadStep) && (0 == DataTransfer.DataDownloadLength))
				|| ((Transfer_Wait_Upload == DataTransfer.DownloadUploadStep) && (0 == DataTransfer.DataUploadCounter)))) {
			NRC = WBSC_73;
			return;
		}

		/*transferDataSuspended-块序号不等于上次传输的块序号时，下载数据长度超过memorySize*/
		if ((DataTransfer.DataBlockSequenceCounter != blockSequenceCounter) 
			&& (DataTransfer.DataDownloadLength + UDS_A_Indication_SDU.Length - 2) > DataTransfer.memorySize) {
			NRC = TDS_71;
			return;
		}

		/*操作条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
		
		/*requestOutOfRange-待添加*/
		if (0) {
			NRC = ROOR_31;
			return;
		}

		/*一般编程错误-待添加*/
		if (0) {
			NRC = GPF_72;
			return;
		}

		/*电压过高过低-待添加*/
		if (0) {
			NRC = VTH_92;
			return;
		}
		else if (0) {
			NRC = VTL_93;
			return;
		}
	}
	
	/*条件检查通过，服务处理*/
	{
		if (Transfer_Wait_Download == DataTransfer.DownloadUploadStep) { //download
			if (DataTransfer.DataBlockSequenceCounter == blockSequenceCounter) { //blockSequenceCounter equal to the previous one
				//send the positive response without writing the data once again
			}
			else {
				if (UDS_TRUE == ProgramWriteData(DataTransfer.memoryAddress + DataTransfer.DataDownloadLength, UDS_A_Indication_SDU.A_SDU + 1, UDS_A_Indication_SDU.Length - 2)) { //写数据
					DataTransfer.DataDownloadLength += UDS_A_Indication_SDU.Length - 2; //去除SID和块序号，获得有效download数据长度
					DataTransfer.DataBlockSequenceCounter = blockSequenceCounter; //更新块序号
				}
				else {
					NRC = GPF_72;
					DataTransfer.DataBlockSequenceCounter = 0; //optional
				}
			}

			if (PR_00 == NRC) {
				SDUSendBuffArray[1] = blockSequenceCounter;
				ResponseDataLength = 2;
			}
		}
		else if (Transfer_Wait_Upload == DataTransfer.DownloadUploadStep) { //upload		
			SDUSendBuffArray[1] = blockSequenceCounter;
			if (DataTransfer.DataBlockSequenceCounter == blockSequenceCounter) { //blockSequenceCounter equal to the previous one
				DataTransfer.DataUploadCounter--;
			}

			if (DataTransfer.DataUploadCounter < (DataTransfer.DataUploadTimes - 1)) { //非最后一次数据传输
				memcpy(SDUSendBuffArray + 2, DataTransfer.memoryAddress + DataTransfer.DataUploadCounter * (MAXDATALENGTH - 2), (MAXDATALENGTH - 2));
				ResponseDataLength = MAXDATALENGTH; //每次传输字节数为maxNumberOfBlockLength
			
				DataTransfer.DataUploadCounter++;
				DataTransfer.DataBlockSequenceCounter = blockSequenceCounter; //更新块序号
			}
			else if (DataTransfer.DataUploadCounter == (DataTransfer.DataUploadTimes - 1)) { //最后一次数据传输
				memcpy(SDUSendBuffArray + 2, DataTransfer.memoryAddress  + DataTransfer.DataUploadCounter * (MAXDATALENGTH - 2), DataTransfer.memorySize - DataTransfer.DataUploadCounter * (MAXDATALENGTH -2));
				ResponseDataLength = DataTransfer.memorySize - DataTransfer.DataUploadCounter * (MAXDATALENGTH -2) + 2; //剩余字节
			
				DataTransfer.DataUploadCounter++;
				DataTransfer.DataBlockSequenceCounter = blockSequenceCounter; //更新块序号
			}
			else {
				NRC = TDS_71;
			}
			
			UDS_PRINTF("DataUploadCounter = %#x\n", DataTransfer.DataUploadCounter);
		}
		else {
			NRC = RSE_24; //fail-safe
		}
	}
}

static void Service37Handle(void) {

	if (((Transfer_Wait_Download == DataTransfer.DownloadUploadStep) && (DataTransfer.DataDownloadLength == DataTransfer.memorySize))  //传输的数据长度等于Download请求长度，download complete
		|| ((Transfer_Wait_Upload == DataTransfer.DownloadUploadStep) && (DataTransfer.DataUploadCounter == DataTransfer.DataUploadTimes))) {
		DataTransfer.DownloadUploadStep = Transfer_Complete;
	}
	
	UDS_PRINTF("memoryAddress = %#x\n", DataTransfer.memoryAddress);
	UDS_PRINTF("memorySize = %d\n", DataTransfer.memorySize);
	UDS_PRINTF("DataDownloadLength = %d\n", DataTransfer.DataDownloadLength);
	UDS_PRINTF("DownloadUploadStep = %#x\n", DataTransfer.DownloadUploadStep);

	UDS_PRINTF("DataUploadCounter = %#x\n", DataTransfer.DataUploadCounter);
	UDS_PRINTF("DataUploadTimes = %#x\n", DataTransfer.DataUploadTimes);

	/*错误条件检查*/
	{

		/*服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 1) {
			NRC = IMLOIF_13;
			return;
		}

		/*服务请求序列错误-块序号不等于上次传输的块序号时，数据传输进程不是等待发送也不是等待上传*/
		if (DataTransfer.DownloadUploadStep != Transfer_Complete) {
			NRC = RSE_24;
			return;
		}
	}

	ResponseDataLength = 1;
}

/*3Dh-根据地址写内存服务*/
static void Service3DHandle(void) {
	uds_uint8_t memorySizeBytes = (UDS_A_Indication_SDU.A_SDU[0] >> 4) & 0x0f, memoryAddressBytes = UDS_A_Indication_SDU.A_SDU[0] & 0x0f;
	uds_uint8_t* memoryAddress;
	uds_uint32_t memorySize;

	/*错误条件检查*/
	{
	
		/*addressAndLengthFormatIdentifier错误*/
		if ((0 == memorySizeBytes) || (0 == memoryAddressBytes) || (memorySizeBytes > 4) || (memoryAddressBytes > 4)) { //(memorySizeBytes > 服务器支持的最大值)等
			NRC = ROOR_31;
			return;
		}

		/*服务长度错误*/
		if (UDS_A_Indication_SDU.Length <= (memorySizeBytes + memoryAddressBytes + 2)) {
			NRC = IMLOIF_13;
			return;
		}

		/*操作条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
		
		/*一般编程错误-待添加*/
		if (0) {
			NRC = GPF_72;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		memoryAddress = ReadMemoryByAddress_Func(UDS_A_Indication_SDU.A_SDU + 1, memoryAddressBytes);
		memorySize = ReadMemorySize_Func((UDS_A_Indication_SDU.A_SDU + memoryAddressBytes + 1), memorySizeBytes);
		
		/*此处需要添加Tester请求地址与Server本地地址的映射关系，地址有效性检查，地址操作权限检查等*/
		
		if (UDS_A_Indication_SDU.Length == (2 + memorySizeBytes + memoryAddressBytes + memorySize)) {			
			memcpy(memoryAddress, (UDS_A_Indication_SDU.A_SDU + memorySizeBytes + memoryAddressBytes + 1), memorySize);
			memcpy(SDUSendBuffArray + 1, UDS_A_Indication_SDU.A_SDU, memorySizeBytes + memoryAddressBytes + 1);
			ResponseDataLength = memorySizeBytes + memoryAddressBytes + 2;
		}
		else {
			NRC = IMLOIF_13;
		}
	}
}

/*3Eh-会话保持服务*/
static void Service3EHandle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;

	/*子服务长度错误*/
	if (UDS_A_Indication_SDU.Length != 2) {
		NRC = IMLOIF_13;
		return;
	}
	
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service3ETable[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service3ETable[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}	
}

/*3Eh-会话保持无抑制正响应00h子服务*/
static void TesterPresent_00Proc(void) {

	/*错误条件检查*/
	{

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{

		/*响应数组填充*/
		SDUSendBuffArray[1] = 0x00;
		ResponseDataLength = 2;
	}
}

/*85h-控制DTC设置服务*/
static void Service85Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;

	/*子服务长度错误*/
	if (UDS_A_Indication_SDU.Length != 2) {
		NRC = IMLOIF_13;
		return;
	}
	
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service85Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service85Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}	
}

/*85h-DTC设置On子服务*/
static void DTCSettingOn_01Proc(void) {

	/*错误条件检查*/
	{

	}

	/*条件检查通过，服务处理*/
	{
	
		if (UDS_TRUE == SuppressPosRspMsgIndicationBit) { //抑制正响应直接setting on
			ConfirmDTCUpdate = UDS_TRUE;
		}
		else { //发送Response
			SDUSendBuffArray[1] = DTCSetting_on;
			ResponseDataLength = 2;
			WaitConfirmBeforeDTCUpdateEnable = UDS_TRUE;
		}
	}
}

/*85h-DTC设置Off子服务*/
static void DTCSettingOff_02Proc(void) {

	/*错误条件检查*/
	{

	}

	/*条件检查通过，服务处理*/
	{
	
		if (UDS_TRUE == SuppressPosRspMsgIndicationBit) { //抑制正响应直接setting on
			ConfirmDTCUpdate = UDS_FALSE;
		}
		else { //发送Response
			SDUSendBuffArray[1] = DTCSetting_off;
			ResponseDataLength = 2;
			WaitConfirmBeforeDTCUpdateDisable = UDS_TRUE;
		}
	}
}

static void Service87Handle(void) {
	uds_uint8_t SubIndex = 0;
	uds_uint8_t ValidSub = UDS_FALSE;
		
	ServiceSubFunc = UDS_A_Indication_SDU.A_SDU[0] & 0x7f;
	SuppressPosRspMsgIndicationBit = (UDS_A_Indication_SDU.A_SDU[0] >> 7) & 0x01;

	/*子服务查询*/
	while ((SubIndex < DiagnosticServiceTable[ServiceIndex].NumOfSubfunc) && (!ValidSub)) {
		if (Service87Table[SubIndex].Sub_func == ServiceSubFunc) {
			ValidSub = UDS_TRUE;
		}
		else {
			SubIndex++;
		}
	}

	if (UDS_TRUE == ValidSub) {
		Service87Table[SubIndex].SubServiceHandle();
	}
	else {
		NRC = SFNS_12;
	}
}

static void VBTWFBR_01Proc(void) {
	uds_uint8_t LinkControlModeIdentifier = UDS_A_Indication_SDU.A_SDU[1];
	TransitionWithFixedParameter_e ParaIndex = SearchFixPara_Func(LinkControlModeIdentifier);
	
	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 3) {
			NRC = IMLOIF_13;
			return;
		}

		/*LinkControlModeIdentifier不存在*/
		if (FixPara_Number == ParaIndex) {
			NRC = ROOR_31;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		LinkStatus.LinkControlProcess = WAIT_TRANSITION;
		LinkStatus.TransitionBaudrates = TransitionWithFixedParameterTable[ParaIndex][1];
		SDUSendBuffArray[1] = VerifyBaudrateTransitionWithFixedBaudrate;
		ResponseDataLength = 2;
	}
}

static TransitionWithFixedParameter_e SearchFixPara_Func(uds_uint8_t LinkControlModeIdentifier) {
	for(uds_uint8_t i = 0; i < FixPara_Number; i++) {
		if (TransitionWithFixedParameterTable[i][0] == LinkControlModeIdentifier) {
			return i;
		}
	}
	return FixPara_Number;
}

static void VBTWSBR_02Proc(void) {
	uds_uint32_t modeParameter = ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[1] << 16) | ((uds_uint32_t)UDS_A_Indication_SDU.A_SDU[2] << 8)
									|(uds_uint32_t)UDS_A_Indication_SDU.A_SDU[3];
	uds_uint8_t Temp = SearchSpecificPara_Func(modeParameter);
	
	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 5) {
			NRC = IMLOIF_13;
			return;
		}

		/*modeParameter无效*/
		if (UDS_FALSE == Temp) {
			NRC = ROOR_31;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		LinkStatus.LinkControlProcess = WAIT_TRANSITION;
		LinkStatus.TransitionBaudrates = modeParameter;
		SDUSendBuffArray[1] = VerifyBaudrateTransitionWithSpecificBaudrate;
		ResponseDataLength = 2;
	}
}

static uds_uint8_t SearchSpecificPara_Func(uds_uint32_t modeParameter) {
	if (1) {
		return UDS_TRUE;
	}
	else {
		return UDS_FALSE;
	}
}

static void TransitionBaudrate_03Proc(void) {

	/*错误条件检查*/
	{

		/*子服务长度错误*/
		if (UDS_A_Indication_SDU.Length != 2) {
			NRC = IMLOIF_13;
			return;
		}

		/*请求顺序错误*/
		if (LinkStatus.LinkControlProcess != WAIT_TRANSITION) {
			NRC = RSE_24;
			return;
		}

		/*条件不满足-待添加*/
		if (0) {
			NRC = CNC_22;
			return;
		}
	}

	/*条件检查通过，服务处理*/
	{
		LinkStatus.LinkControlProcess = TRANSITION_OK; //平台根据LinkStatus.LinkControlProcess判断是否使用设定的波特率
		SDUSendBuffArray[1] = TransitionBaudrate;
		ResponseDataLength = 2;
	}
}


/*DTC测试结果处理*/
static void DTCHandle(DTCNode* DtcNode) {
	DTCTestSampleResult_e PreTestResult = NO_RESULT;
	DTCTestResult_e CurTestResult = IN_TESTING;
	uds_uint8_t OperateCycleComplete = UDS_FALSE,test_result;
	uds_uint8_t DTCStatusOld = DtcNode->Sts.DTCstatus.StatusOfDTC & DTCStatusAvailabilityMask;

	/*Operation Cycle*/
	if (DtcNode->Sts.DetecCycleCounter < DtcNode->Config.DetecTimes) {
		DtcNode->Sts.DetecCycleCounter ++;
		OperateCycleComplete = UDS_FALSE;
	}
	else {
		DtcNode->Sts.DetecCycleCounter = 0;
		OperateCycleComplete = UDS_TRUE;
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testFailed = 0; //optional
	}

	//UDS_PRINTF("DetecCycleCounter = %d\n\r", DtcNode->DetecCycleCounter);

	/*First Operation Cycle*/
	if (1 == DtcNode->Sts.DetecCycleCounter) {
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testFailed = 0;
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testFailedThisOperationCycle = 0;
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testNotCompletedThisOperationCycle = 1;
		DtcNode->Sts.FaultDetectionCounter = 0;
	}

	/*Test Sample*/
	if (DtcNode->Config.EnableDTCDetect()) {
		if(DtcNode->Config.Test_func){
			test_result = DtcNode->Config.Test_func();
		}else{
			test_result = NO_RESULT;
		}
		if (PREPASSED == test_result) {
		#if 0 //optional
			if (DtcNode->FaultDetectionCounter > 0) {
				DtcNode->FaultDetectionCounter = 0 - DtcNode->Config.PrepassedStep;
			}
			else {
				DtcNode->FaultDetectionCounter -= DtcNode->Config.PrepassedStep;
			}
		#else
			DtcNode->Sts.FaultDetectionCounter -= DtcNode->Config.PrepassedStep;
		#endif
		}
		else if (PREFAILED == test_result) {
			if (DtcNode->Sts.FaultDetectionCounter < 0) {
				DtcNode->Sts.FaultDetectionCounter = DtcNode->Config.PrefailedStep;
			}
			else {		
				DtcNode->Sts.FaultDetectionCounter += DtcNode->Config.PrefailedStep;
			}
		}
		else {

		}
	}
	else {
		//DtcNode->FaultDetectionCounter = 0; //Optional
	}

	/*Test Sample Result Judge*/
	if (DtcNode->Sts.FaultDetectionCounter >= DtcNode->Config.FailedFaultDetectionTimes) {
		DtcNode->Sts.FaultDetectionCounter = DtcNode->Config.FailedFaultDetectionTimes;
		CurTestResult = FAILED;
	}
	else if (DtcNode->Sts.FaultDetectionCounter <= DtcNode->Config.PassedFaultDetectionTimes) {
		DtcNode->Sts.FaultDetectionCounter = DtcNode->Config.PassedFaultDetectionTimes;
		CurTestResult = PASSED;
	}
	else {
		CurTestResult = IN_TESTING;
	}
	
	/*Test Complete*/
	if (PASSED == CurTestResult) {
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testFailed = 0;
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testNotCompletedSinceLastClear = 0;
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testNotCompletedThisOperationCycle = 0;
	}
	else if (FAILED == CurTestResult) {
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testFailed = 1;
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testFailedThisOperationCycle = 1;
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.pendingDTC = 1;
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testNotCompletedSinceLastClear = 0;
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testFailedSinceLastClear = 1;
		DtcNode->Sts.DTCstatus.StatusOfDTC_t.testNotCompletedThisOperationCycle = 0;
		
		if (DtcNode->Sts.ConfirmationCounter >= DtcNode->Config.Confirmationthreshold) {
			DtcNode->Sts.ConfirmationCounter = 0;
			DtcNode->Sts.DTCstatus.StatusOfDTC_t.confirmedDTC = 1;
			DTC_Snapshot_Write_Func(DtcNode->Config.SnapShotTag);		
		}
	}
	else {

	}

	/*Operation Cycle Complete*/
	if (UDS_TRUE == OperateCycleComplete) {

		/*Test Passed in this Operation Cycle*/
		if ((0 == DtcNode->Sts.DTCstatus.StatusOfDTC_t.testNotCompletedThisOperationCycle) 
			&& (0 == DtcNode->Sts.DTCstatus.StatusOfDTC_t.testFailedThisOperationCycle)) {
			DtcNode->Sts.DTCstatus.StatusOfDTC_t.pendingDTC = 0;
			DtcNode->Sts.ConfirmationCounter = 0;

			if (1 == DtcNode->Sts.DTCstatus.StatusOfDTC_t.confirmedDTC) {
				if (DtcNode->Sts.FaultAgingCounter < DtcNode->Config.MaxDTCAgingTimes) {
					DtcNode->Sts.FaultAgingCounter ++;
				}
				else {
					DtcNode->Sts.DTCstatus.StatusOfDTC_t.confirmedDTC = 0;
					//DtcNode->Sts.DTCstatus.StatusOfDTC_t.testNotCompletedSinceLastClear = 1;
					//DtcNode->Sts.DTCstatus.StatusOfDTC_t.testFailedSinceLastClear = 0;
				}
			}
			
		}

		/*Test Failed in this Operation Cycle*/
		if (1 == DtcNode->Sts.DTCstatus.StatusOfDTC_t.testFailedThisOperationCycle) {
			if ((0 == DtcNode->Sts.DTCstatus.StatusOfDTC_t.confirmedDTC) && (DtcNode->Sts.ConfirmationCounter < DtcNode->Config.Confirmationthreshold)) {
				DtcNode->Sts.ConfirmationCounter ++;
			}
		
			if (DtcNode->Sts.FaultAgingCounter >= DtcNode->Config.MaxDTCAgingTimes) {
				if (DtcNode->Sts.FaultAgedCounter < 0xff) {
					DtcNode->Sts.FaultAgedCounter ++;
				}
				else {
					DtcNode->Sts.FaultAgedCounter = 0xff; //fail-safe
				}
			}
			DtcNode->Sts.FaultAgingCounter = 0;
			
			if (DtcNode->Sts.FaultOccurenceCounter < 0xff) {
				DtcNode->Sts.FaultOccurenceCounter ++;
			}
			else {
				DtcNode->Sts.FaultOccurenceCounter = 0xff; //fail-safe
			}
		}

		if (1 == DtcNode->Sts.DTCstatus.StatusOfDTC_t.pendingDTC) {
			if (DtcNode->Sts.FaultPendingCounter < 0xff) {
				DtcNode->Sts.FaultPendingCounter ++;
			}
			else {
				DtcNode->Sts.FaultPendingCounter = 0xff; //fail-safe
			}
		}
	}
	else {

	}

	if (DTCStatusOld != (DtcNode->Sts.DTCstatus.StatusOfDTC & DTCStatusAvailabilityMask)) {
		//UDS_A_eeprom_write(DtcNode->DTCStatusTag, &(DtcNode->DTCstatus.StatusOfDTC));
		UDS_A_Data_Save(DtcNode->Config.DTCStatusTag, &(DtcNode->Sts.DTCstatus.StatusOfDTC), MAX_DTC_EEPROM_BYTES);
	}
}

static void DTC_Snapshot_Write_Func(uds_uint8_t SnapShotTag) {
	uds_uint8_t length = 0, i, buffer[MAX_DTC_SNAPSHOT_STORE_BYTES];
	for(i = 0 ; i < NumberofDTCSnapshot ; i++) {
		memcpy(buffer + length, DTCSnapshotTable[i].SnapshotData, DTCSnapshotTable[i].DataLength);
		length += DTCSnapshotTable[i].DataLength;
	}
	//UDS_A_eeprom_write(SnapShotTag, buffer);
	UDS_A_Data_Save(SnapShotTag, buffer, length);
}

/*诊断DTC处理*/
static void DiagnosticDTC_Proc(void) {
	uds_uint8_t i;

	if (UDS_TRUE == ConfirmDTCUpdate) {
		for(i = 0 ; i < NumberofDTC ; i++) {
			DTCHandle(DTCTable + i);
		}
	}
}

/*应用层定时器超时处理*/
static void UDS_A_Timeout_Proc(void) {

	/*S3_Server超时处理*/
	if (S_TIME_CNT_STS_TIMEOUT == UDS_A_timer_status_get(S_TIME_S3_SERVER)) { //S3超时,进入默认会话
		DiagnosticState_Init(DefaultSession, LEVEL_ZERO, DiagProcessInit, Default_Session, Init_Indication, Init_Confirm, 
							Init_Response, NotSuppressPositive);
		UDS_A_timer_ctl_reset(S_TIME_S3_SERVER);
		if (SecurityAccessPara.UnlockStep != WAIT_DELAY) { //ECU Lock
			SecurityAccessPara.UnlockStep = WAIT_SEED_REQ;
		}
		
		/*此处需要EnableRxAndTx NWMCM_NCM*/ //接收发送所有报文
		
		NumberofReadDID = NumberofStaticReadDID; //动态DID失效

		ConfirmDTCUpdate = UDS_TRUE; //使能DTC状态Update

		IO_ControlPara_Flag_u.status = UDS_FALSE; //输入输出控制参数无效

		LinkStatus.LinkControlProcess = WAIT_MODE_REQ;; //ECU恢复默认的通讯模式

		UDS_PRINTF("S3 timeout!\n\r");		
	}
	else {
		
	}

	/*P2_Server或P2*_Server超时处理*/
	if ((S_TIME_CNT_STS_TIMEOUT == UDS_A_timer_status_get(S_TIME_P2_CAN_SERVER)) 
		|| (S_TIME_CNT_STS_TIMEOUT == UDS_A_timer_status_get(S_TIME_P2X_CAN_SERVER))) {

		/*会话模式&安全等级保持状态不变*/
		DiagnosticState_Init(SystemState.SessionMode, SystemState.SecurityLevel, DiagProcessInit, SystemState.SessionType, 
							Init_Indication, Init_Confirm, Init_Response, NotSuppressPositive);		
		UDS_A_timer_ctl_reset(S_TIME_P2_CAN_SERVER);
		UDS_A_timer_ctl_reset(S_TIME_P2X_CAN_SERVER);
		if(SystemState.SessionMode != DefaultSession) {
			UDS_A_timer_ctl_restart(S_TIME_S3_SERVER);
		}
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
	UDS_A_Diagnostic_Init();
	
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
	DiagnosticDTC_Proc();
	IO_ControlParameter_Test();
	UDS_A_seesion_time_management();
	UDS_A_Timeout_Proc();
}
