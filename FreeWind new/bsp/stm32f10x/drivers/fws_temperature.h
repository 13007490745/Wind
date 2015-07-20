#ifndef __FWS_TEMPERATURE__
#define __FWS_TEMPERATURE__

#include "stm32f10x.h"
#define DS0					1
#define DS1					2

int fws_DS18B20_Init( uint32_t id );
int fws_DS18B20_TmpGet( uint32_t id, uint8_t *buf );



#endif

