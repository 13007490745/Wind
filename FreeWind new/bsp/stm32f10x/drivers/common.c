#include <stm32f10x.h>
#include "common.h"
#include <finsh.h>

void SystemTickHook(void)
{
  	void UART1_TX_TickHook(void);
  	void UART2_TX_TickHook(void);
  	void UART3_TX_TickHook(void);
  	void UART4_TX_TickHook(void);
	  void UART5_TX_TickHook(void);

	UART1_TX_TickHook();
	UART3_TX_TickHook();
	UART4_TX_TickHook();
	UART5_TX_TickHook();
}

int UART_DriverSet(uint32_t op )
{
    GPIO_InitTypeDef GPIO_InitStructure;
	static uint8_t drmark = 0;
	if( drmark == 0 )
	{
		drmark = 1;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_AFIO,ENABLE);
		
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
		GPIO_Init(GPIOE, &GPIO_InitStructure);
	}

	if( op == 1 )
	   	GPIO_ResetBits(GPIOE,GPIO_Pin_13);
	else
		GPIO_SetBits(GPIOE,GPIO_Pin_13);
	return 0;
}		 
FINSH_FUNCTION_EXPORT(UART_DriverSet,UART_DriverSet( op ))


void Delay_nop(uint32_t myUs)   
{
  while(myUs--)
  {
  }
}

void Delay_Us(uint32_t myUs)   
{
  u16 i;
  while(myUs--)
  {
    i=6;
    while(i--);
  }
}
 
void Delay_Ms(uint32_t myMs)
{
  u16 i;
  while(myMs--)
  {

    i=8000;
    while(i--);
  }
}



