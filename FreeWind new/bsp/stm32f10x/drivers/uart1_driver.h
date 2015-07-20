#ifndef __UART1_DRIVER_H__
#define __UART1_DRIVER_H__

#include "cycbuf.h"
#include "Singlebuf.h"

//------- UART1 TX Config ------------------
#define	U1SNED_MAXFRMLEN			128
#define	U1SNED_MAXFRMNUM			20
extern CBUF    U1SendBuf;

//------- UART1 RX Config ------------------
#define U1REVLEN					256
extern SBUF 	U1RevBuf;

void UART1_init(uint32_t band);





#endif

