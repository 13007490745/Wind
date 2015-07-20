#ifndef __UART4_DRIVER_H__
#define __UART4_DRIVER_H__
#include "cycbuf.h"
#include "Singlebuf.h"

//------- UART2 TX Config ------------------
#define	U4SNED_MAXFRMLEN			128
#define	U4SNED_MAXFRMNUM			20
extern CBUF    U4SendBuf;

//------- UART1 RX Config ------------------
#define U4REVLEN					256
extern SBUF 	U4RevBuf;

void UART4_init(uint32_t band);








#endif

