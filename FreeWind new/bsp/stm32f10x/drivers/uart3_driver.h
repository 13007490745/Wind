#ifndef __UART3_DRIVER_H__
#define __UART3_DRIVER_H__
#include "cycbuf.h"
#include "Singlebuf.h"

//------- UART2 TX Config ------------------
#define	U3SNED_MAXFRMLEN			128
#define	U3SNED_MAXFRMNUM			20
extern CBUF    U3SendBuf;

//------- UART1 RX Config ------------------
#define U3REVLEN					256
extern SBUF 	U3RevBuf;

void UART3_init(uint32_t band);








#endif

