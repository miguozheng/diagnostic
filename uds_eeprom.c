/**  @UDS_N.c
*     @note 2018 crystal-optech Co., Ltd. All Right Reserved.
*     @brief UDS eeorom driver.
*     @author     ZhengLong
*     @date        2018/12/07
*     @note
*     @warning 
*/

#include "uds_common_include.h"
#include "uds_struct.h"
/*************************1*****************************************************
Private global variables and functions
*******************************************************************************/
const UDS_EE_Info_t EE_add_table[EE_TAG_ALL] = 
{
	{SECURITYACCESS_ERROR_COUNTER_TAG, 1 + 1},
	{DTC_CAN_RECEIVENODELOST_0x363_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_CAN_RECEIVENODELOST_0x3C3_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_CAN_RECEIVENODELOST_0x490_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_CAN_RECEIVENODELOST_0x581_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_CAN_RECEIVENODELOST_0x5A0_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_CAN_RECEIVENODELOST_0x64F_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_LED_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_LED_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_TFT_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_TFT_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_MAINBOARD_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_MAINBOARD_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_LIRHTSENSOR_MALFUNCTION_TAG, MAX_DTC_EEPROM_BYTES + 1},
	{DTC_STEPPINGMOTOR_MALFUNCTION_TAG, MAX_DTC_EEPROM_BYTES + 1},
	
	{DTCSNAPSHOT_CAN_0x363_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_CAN_0x3C3_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_CAN_0x490_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_CAN_0x581_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_CAN_0x5A0_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_CAN_0x64F_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_LED_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_LED_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_TFT_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_TFT_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_MAINBOARD_TEMPSENSOR_GCCSHORT_GNDOPEN_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_MAINBOARD_TEMPSENSOR_GCCOPEN_GNDSHORT_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_LIRHTSENSOR_MALFUNCTION_TAG, DTC_SNAPSHOT_BYTES + 1},
	{DTCSNAPSHOT_STEPPINGMOTOR_MALFUNCTION_TAG, DTC_SNAPSHOT_BYTES + 1},

	{VIN_NUMNER_TAG, VIN_NUMBER_BYTES + 1},
	{EE_TEST,4},
};

/*
============================================================================
 Function declear
============================================================================
*/
static uds_int8_t (*UDS_EE_add_read)(uds_uint32_t offset,
									 uds_uint16_t length,
									 uds_uint8_t *buf);
static uds_int8_t (*UDS_EE_add_write)(uds_uint32_t offset,
									  uds_uint16_t length,
									  uds_uint8_t *buf);


/*
============================================================================
 User function
============================================================================
*/
uds_int8_t UDS_EE_init(void)
{
	uds_int8_t ret = 1;
	UDS_Interface_Ext_t *inf = (UDS_Interface_Ext_t *)get_uds_ext_interface();

	if(inf->eeprom_read){
		UDS_EE_add_read = inf->eeprom_read;
	}else{
		ret |= -1;
	}
	if(inf->eeprom_write){
		UDS_EE_add_write = inf->eeprom_write;
	}else{
		ret |= -1;
	}

	return ret;
}


uds_int8_t UDS_EE_tag_read(UDS_EE_Tag_e tag,uds_uint8_t *buf)
{
	uds_int8_t ret = -1;
	uds_uint8_t i = 0;
	uds_uint32_t offset = EEPROM_BASE_ADDRESS;
	UDS_EE_Info_t *table = (UDS_EE_Info_t *)&EE_add_table[0];

	if((!UDS_EE_add_read) || (EE_TAG_ALL <= tag) || (!buf)){
		return ret;
	}
	//get offset
	for(i = 0;i < EE_TAG_ALL;i++,table++){
		if(table->Tag == tag){
			break;
		}else{
			offset += table->Length;
		}
	}
	//read data
	ret = UDS_EE_add_read(offset,table->Length,buf);

	return ret;
}

uds_int8_t UDS_EE_tag_write(UDS_EE_Tag_e tag,uds_uint8_t *buf)
{
	uds_int8_t ret = -1;
	uds_uint8_t i = 0;
	uds_uint32_t offset = EEPROM_BASE_ADDRESS;
	UDS_EE_Info_t *table = (UDS_EE_Info_t *)&EE_add_table[0];

	if((!UDS_EE_add_read) || (EE_TAG_ALL <= tag) || (!buf)){
		return ret;
	}
	//get offset
	for(i = 0;i < EE_TAG_ALL;i++,table++){
		if(table->Tag == tag){
			break;
		}else{
			offset += table->Length;
		}
	}
	//read data
	ret = UDS_EE_add_write(offset,table->Length,buf);

	return ret;
}

