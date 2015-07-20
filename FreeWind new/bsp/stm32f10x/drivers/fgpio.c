#include <stm32f10x.h>
#include <rtthread.h>
#include "fgpio.h"



#define PLY_SW0					GPIO_Pin_8
#define PLY_SW1					GPIO_Pin_9
#define PLY_SW2					GPIO_Pin_10
#define PLY_SW3					GPIO_Pin_11
#define PLY_SW4					GPIO_Pin_12

#define DOG_TRI_PIN					GPIO_Pin_8

void dogtri_init()
{  
	 GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

   GPIO_InitStructure.GPIO_Pin   = DOG_TRI_PIN;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	   GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	   GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	
	
	   GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
		GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	
}
void DOGTRI()
{
 
GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	   GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	   GPIO_ResetBits(GPIOA,DOG_TRI_PIN);
	
	
	   GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
		GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
	  GPIO_SetBits(GPIOA,DOG_TRI_PIN);
}	

int RELAY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

    GPIO_InitStructure.GPIO_Pin   = PLY_SW0;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = PLY_SW1;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = PLY_SW2;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = PLY_SW3;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = PLY_SW4;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_SetBits(GPIOD,PLY_SW0);
	GPIO_SetBits(GPIOD,PLY_SW1);
	GPIO_SetBits(GPIOD,PLY_SW2);
	GPIO_SetBits(GPIOD,PLY_SW3);
	GPIO_SetBits(GPIOD,PLY_SW4);

	return 0;
}


int RELAY_Set(uint32_t index,uint32_t op)
{
	if( index > SW4 ) return -1;
	if( op == RLY_ON )
	{
		GPIO_ResetBits(GPIOD,(1<<(index+8)));
	}
	else if( op == RLY_OFF )
	{
		GPIO_SetBits(GPIOD,(1<<(index+8)));
	}

	return 0;
}

int RELAY_Get(uint32_t index)
{
	if( index > SW4 ) return 0;
	return GPIO_ReadOutputDataBit(GPIOD,(1<<(index+8)));		

}

#ifdef RT_USING_FINSH
#include <finsh.h>
int fws_ply(uint32_t index,uint32_t op)
{
	static int it = 0;
	if( it == 0 )
	{
	 	it = 1;
		RELAY_Init();
	}
	RELAY_Set(index,op);
 	return 0;
}
FINSH_FUNCTION_EXPORT(fws_ply,fws_ply(index,op));

#endif



