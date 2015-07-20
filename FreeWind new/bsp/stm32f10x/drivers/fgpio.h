#ifndef __FGPIO_H__
#define __FGPIO_H__

#define RLY_NUM					5

#define RLY_ON					1
#define	RLY_OFF					0

#define SW0						0
#define SW1						1
#define SW2						2
#define SW3						3
#define SW4						4

void dogtri_init(void);
void DOGTRI(void);
int RELAY_Init(void);
int RELAY_Set(unsigned int index,unsigned int op);
int RELAY_Get(unsigned intindex);
 
#endif

