#ifndef	__FWSPWRPROCESS_H__
#define __FWSPWRPROCESS_H__

#define PWR_OFF						0
#define PWR_ON 						1

int FwsPwrDect_isStartup(void);
int FwsPwrDect_is24V(void);
int FwsPwrDect_isPowerLowAlarm(void);
//unsigned int StartupMark=0;








#endif
