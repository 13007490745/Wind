#include "stm32f10x.h"
#include <rtthread.h>
#include "uart5_driver.h"

//------- UART5 RX Config ------------------
//定义接收缓冲区
uint8_t	U5RevStack[U5REVLEN];
SBUF 	U5RevBuf;

//------- UART5 TX Config ------------------
uint8_t U5SendStack[U5SNED_MAXFRMNUM][U5SNED_MAXFRMLEN+FRAME_HEAD];
CBUF    U5SendBuf;


void UART5_NVIC_Config(void);
void UART5_TX_DMAStart(uint8_t* SendBuff,uint32_t SendLen);
void UART5_DMA_init(void);


//-----------------------------------------------------------------------------------
void UART5_init(uint32_t band)
{	
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

    //----------------------------------------------------------------------------
    Sbuf_init(&U5RevBuf,U5RevStack,U5REVLEN);

	if( Cycbuf_Init(&U5SendBuf,
					(uint8_t*)U5SendStack,
					U5SNED_MAXFRMLEN,
					U5SNED_MAXFRMNUM) < 0 )
	{
	 	rt_kprintf("Cycle buffer init fail..\n");
	}


	UART5_NVIC_Config();
    //----------------------------------------------------------------------------
	//IO init
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    //----------------------------------------------------------------------------
	//UART init    
    USART_InitStructure.USART_BaudRate = band;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(UART5, &USART_InitStructure);

	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	USART_Cmd(UART5, ENABLE);
}


void UART5_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

}

//-------------------------------- RX -------------------------------------------------

void UART5_RX_Isr(void)
{
	static uint8_t	RevData;
	void GSM_UART_RX(uint8_t c);
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
	{
		RevData = USART_ReceiveData(UART5);							//取出收到的一字节数据		
		Sbuf_write(&U5RevBuf,&RevData,1);
		GSM_UART_RX(RevData);	
	}
}

//-------------------------  UART5 TX --------------------------------
void UART5_TX_TickHook(void)
{
	static int32_t  len = 0;
	static uint32_t txcnt = 0;
	static uint8_t Txbuf[U5SNED_MAXFRMLEN];

	if( len <= 0 )
	{
		len = Cycbuf_ReadFrame(&U5SendBuf,Txbuf,U5SNED_MAXFRMLEN);
		txcnt = 0;
	}
	else
	{
		if( USART_GetFlagStatus(UART5, USART_FLAG_TXE) != RESET )
		{
		 	USART_SendData(UART5, Txbuf[txcnt]);
			txcnt++;
			len--;
		}

	}

}







/*------------------ UART5 DEBUG ----------------------------*/
#include <finsh.h>
static uint8_t init = 0;
void u5sendtest(int num)
{
	static uint8_t test[30]="0000 abcdefg1234567890\r\n";
	static uint32_t lp;

	if( init == 0 )
	{
	 	init = 1;
		UART5_init(9600);
	}
	while(num-- > 0)
	{
		test[0] = (num/1000)%10 + 48;
		test[1] = (num/100)%10 + 48;
		test[2] = (num/10)%10 + 48;
		test[3] = num%10 + 48;
		lp = 0;
		while( Cycbuf_WriteFrame(&U5SendBuf,test,24) < 0 )
		{
			if(lp++ > 100 )
			{
				lp = 0;
				rt_thread_delay(10);
			}
		}; 
	}	

}
FINSH_FUNCTION_EXPORT(u5sendtest,u5sendtest(int num))

uint8_t Uart5_PutChar(uint8_t ch)
{
	if( init == 0 )
	{
	 	init = 1;
		UART5_init(9600);
	}
	USART_SendData(UART5, (uint8_t) ch);
	while(USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET)
	{
	}
	return ch;
}
FINSH_FUNCTION_EXPORT(Uart5_PutChar,Uart5_PutChar(uint8_t ch))

void u5rxdump(void)
{
	uint8_t u5rxbuf[500],rxlen,i;
	static uint32_t cnt = 0;
	if( init == 0 )
	{
	 	init = 1;
		UART5_init(9600);
	}
	
	while(1)
	{
		rxlen = Sbuf_read(&U5RevBuf,u5rxbuf,500);
		if( rxlen > 0 )
		{
			cnt += rxlen;
			rt_kprintf("UART5 RX: (len=%d,totla=%d)\n",rxlen,cnt);
		 	for(i=0;i<rxlen;i++)
			{
			 	rt_kprintf("%02x ",u5rxbuf[i]);	 	
			}
			rt_kprintf("\n");

		}
		rt_thread_delay(20);
	}
}
FINSH_FUNCTION_EXPORT(u5rxdump,dump uart5 rx data band=9600)


