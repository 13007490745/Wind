#include <stm32f10x.h>
#include "rtthread.h"
#include "fws_audio.h"
#include "common.h"


#define MAX_AUD_NUM			5

#define AUD_BUSY			GPIO_Pin_13
#define AUD_DAT				GPIO_Pin_14
#define AUD_RST				GPIO_Pin_15		

#define DAT_H()				( GPIOD->BSRR = AUD_DAT ) 
#define DAT_L()				( GPIOD->BRR  = AUD_DAT ) 

#define RST_H()				( GPIOD->BSRR = AUD_RST ) 
#define RST_L()				( GPIOD->BRR  = AUD_RST ) 

#define	BUSY_READ()		    ( GPIOD->IDR & AUD_BUSY	)

int AUD_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = AUD_DAT;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = AUD_RST;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = AUD_BUSY;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

	DAT_L();
	RST_L();
	Delay_Ms(1); 
//	rt_thread_delay(1);
	RST_H();
//	rt_thread_delay(1);
	Delay_Ms(1);
	RST_L();

	return 0;
}

int AUD_Play(uint32_t index)
{
	if( index > MAX_AUD_NUM	)
		return -1;

	RST_H();
	//rt_thread_delay(1);
	Delay_Ms(1);
	RST_L();
	//rt_thread_delay(1);
	Delay_Ms(1);
	do
	{
		DAT_H();
		//rt_thread_delay(1); 
   Delay_Ms(1);		
		DAT_L();
		//rt_thread_delay(1); 
		Delay_Ms(1);
	}while(index--);

	return 0;
}

uint32_t AUD_IsEnd(void)
{
	if( BUSY_READ()	 )
		return 1;
	else
		return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
int fws_audio(uint32_t index)
{
 	static uint8_t init = 0;
	if( init == 0 )
	{
		init = 1;
		AUD_Init();
	}
	AUD_Play(index);
 
	return 0;
}
FINSH_FUNCTION_EXPORT(fws_audio,fws_audio(index))
#endif



