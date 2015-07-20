#ifndef	__FWS_AD_H__
#define __FWS_AD_H__

#define PWR24V					1
#define PWR12V					0

#define PWR_RESET				2
#define PWR_ON					1
#define PWR_OFF					0

int ADC_PwrInit(void);
int PWR_DectInit(void);
int PWR_IsPwr24V(void);
uint16_t ADC_Converter(void);
void ADC_GetVol(uint8_t *buf);
int ADC_PrintVal(void);








#endif

