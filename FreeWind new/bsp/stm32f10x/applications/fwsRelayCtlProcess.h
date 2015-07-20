#ifndef	__FWSRELAYCTLPROCESS_H__
#define __FWSRELAYCTLPROCESS_H__

#define RLY_WIND1						0
#define RLY_WIND2						1
#define RLY_WINDOW						2
#define RLY_CHAIRHEAT					3
#define RLY_CHAIRCOOL					4

#define FELAY_DELAY_FOREVER				0xffffffff
#define FELAY_DELAY_AUTO				150	// 15s

#define FELAY_NUM						5


#define RLY_NULL					0x00

#define RLY_WIND_MD_FLAG			0x08
#define RLY_WIND_CO_FLAG			0x04
#define RLY_WIND_CH2O_FLAG			0x02
#define RLY_WIND_PHONE_APP			0x01
#define RLY_WIND_ALLOFF_FLAG			0xFF

#define MIN_DIV  600 		//600

void fwsRlyWindOpen(unsigned int flag,unsigned int timeout);
void fwsRlyWindClose(unsigned int op);
int fwsRlyWindStateGet(void);
int FwsRlySet(unsigned int id,unsigned int op,unsigned int time);
void FwsCOdectProcess_thread_entry(void* parameter);

#endif

