#ifndef __UART2_DRIVER_H__
#define __UART2_DRIVER_H__
#include "cycbuf.h"
#include "Singlebuf.h"

//------- UART2 TX Config ------------------
#define	U2SNED_MAXFRMLEN			128
#define	U2SNED_MAXFRMNUM			20
extern CBUF    U2SendBuf;

//------- UART1 RX Config ------------------
#define U2REVLEN					256
extern SBUF 	U2RevBuf;

void UART2_init(uint32_t band);








#endif

