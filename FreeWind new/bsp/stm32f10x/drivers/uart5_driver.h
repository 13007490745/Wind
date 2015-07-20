#ifndef __UART5_DRIVER_H__
#define __UART5_DRIVER_H__

#include "cycbuf.h"
#include "Singlebuf.h"

//------- UART2 TX Config ------------------
#define	U5SNED_MAXFRMLEN			128
#define	U5SNED_MAXFRMNUM			20
extern CBUF    U5SendBuf;

//------- UART1 RX Config ------------------
#define U5REVLEN					256
extern SBUF 	U5RevBuf;

void UART5_init(uint32_t band);








#endif

