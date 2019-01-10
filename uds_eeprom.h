#ifndef _UDS_EEPROM_H_
#define _UDS_EEPROM_H_

#include "uds_struct.h"

uds_int8_t UDS_EE_tag_read(UDS_EE_Tag_e tag,uds_uint8_t *buf);
uds_int8_t UDS_EE_tag_write(UDS_EE_Tag_e tag,uds_uint8_t *buf);
uds_int8_t UDS_EE_init(void);

#endif
