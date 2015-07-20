#ifndef __AUDIOSERVER_H__
#define __AUDIOSERVER_H__
#include "Singlebuf.h"

#define CH2O_MARK						0x80
#define CO_MARK							0x00

#define CH2O_WARNING_SPEAK_MARK			0x40
#define CO_WARNING_SPEAK_MARK 			0x20
#define SPEAK_MARK						0x60

#define WARNING_DIDIDIDI_MARK	  		0x02
#define WARNING_DIDI_MARK				0x01
#define DIDI_WARNING_MARK				0x03

#define WARNING_NULL					0x00

extern SBUF 	Ascb; //Audio server cmd buf








#endif
