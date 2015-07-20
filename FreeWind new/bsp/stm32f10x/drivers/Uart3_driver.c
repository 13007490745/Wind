#include "stm32f10x.h"
#include <rtthread.h>
#include "uart3_driver.h"

//------- UART3 RX Config ------------------
//������ջ�����
uint8_t	U3RevStack[U3REVLEN];
SBUF 	U3RevBuf;

//------- UART3 TX Config ------------------
uint8_t U3SendStack[U3SNED_MAXFRMNUM][U3SNED_MAXFRMLEN+FRAME_HEAD];
CBUF    U3SendBuf;

//------- UART3 ��ѯ���ͽ���������� ------------------
static uint8_t U3StarupMark = 0;

void UART3_NVIC_Config(void);
void UART3_TX_DMAStart(uint8_t* SendBuff,uint32_t SendLen);
void UART3_DMA_init(void);


//-----------------------------------------------------------------------------------
void UART3_init(uint32_t band)
{	
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    //----------------------------------------------------------------------------
    Sbuf_init(&U3RevBuf,U3RevStack,U3REVLEN);

	if( Cycbuf_Init(&U3SendBuf,
					(uint8_t*)U3SendStack,
					U3SNED_MAXFRMLEN,
					U3SNED_MAXFRMNUM) < 0 )
	{
	 	rt_kprintf("Cycle buffer init fail..\n");
	}


	UART3_NVIC_Config();
    //----------------------------------------------------------------------------
	//IO init
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //----------------------------------------------------------------------------
	//UART init    
    USART_InitStructure.USART_BaudRate = band;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStructure);

    //----------------------------------------------------------------------------
	//DMA init     

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	UART3_DMA_init();
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART3, ENABLE);

	//����hook�еĲ�ѯ���͹���
	U3StarupMark = 1;
}
void UART3_DMA_init(void)
{
	#define USART3_DR_Base  0x40004804
    DMA_InitTypeDef DMA_InitStructure;
    //DMA���ã�
    //����DMAԴ���ڴ��ַ&�������ݼĴ�����ַ
    //�����ڴ�-->����
    //ÿ�δ���λ��8bit
    //�����СDMA_BufferSize=SENDBUFF_SIZE
    //��ַ����ģʽ�������ַ�������ڴ��ַ����1
    //DMAģʽ��һ�δ��䣬��ѭ��
    //���ȼ�����
    DMA_DeInit(DMA1_Channel2);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);

	DMA_ITConfig(DMA1_Channel2,DMA_IT_TC,ENABLE);
}

void UART3_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	// DMA TX interrupt
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//-------------------------------- RX -------------------------------------------------

void UART3_RX_Isr(void)
{
	static uint8_t	RevData;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		RevData = USART_ReceiveData(USART3);							//ȡ���յ���һ�ֽ�����		
		Sbuf_write(&U3RevBuf,&RevData,1);	
	}
}

//-------------------------  UART3 TX --------------------------------
/* Output FIFO */



/*----------------------------------------------------------*/
void UART3_TX_DMA_Isr(void)
{
	static int len;
	static unsigned char *p;
	
	//DMA������ɺ󴥷��жϣ���������жϱ�ǡ�
	DMA_ClearITPendingBit(DMA1_IT_TC2);
	//��ʹ�ù��Ļ������Ż��ڴ�ء�
	Cycbuf_NextTailFrame(&U3SendBuf);
	//��DMA�رա�
	DMA_Cmd(DMA1_Channel2, DISABLE);
		
	//��ȡ��һ��������
	len = Cycbuf_GetAvailFrmTailp(&U3SendBuf,&p);

	//�����ȡ��������
	if( len > 0 )
		UART3_TX_DMAStart(p,len);		
}

void UART3_TX_TickHook(void)
{
	static int len;
	static unsigned char *p;
	#define	DMA_ENABLE_MASK					0x01

	if( U3StarupMark == 0 )
		return;

	/* Check DMA is runing*/
	if( DMA1_Channel2->CCR & DMA_ENABLE_MASK )			
		return;	 /* If runing , return */	
	
	/* No send data */
	len = Cycbuf_GetAvailFrmTailp(&U3SendBuf,&p);

	/* If buffer empty */
	if( len <= 0)
		return;	
	/* Starting DMA transmit */
	UART3_TX_DMAStart(p,len); 
}

void UART3_TX_DMAStart(uint8_t* SendBuff,uint32_t SendLen)
{
	DMA1_Channel2->CMAR  = (uint32_t)SendBuff;
	DMA1_Channel2->CNDTR = SendLen;
    //Start DMA Transmission 
    DMA_Cmd(DMA1_Channel2, ENABLE);
}







/*------------------ UART3 DEBUG ----------------------------*/
#include <finsh.h>
void u3sendtest(int num)
{
	static uint8_t test[30]="0000 abcdefg1234567890\r\n";
	static uint32_t lp;
	static uint32_t ini = 0;

	if( ini == 0 )
	{
		ini = 1;
		UART3_init(115200);
	}
	while(num-- > 0)
	{
		test[0] = (num/1000)%10 + 48;
		test[1] = (num/100)%10 + 48;
		test[2] = (num/10)%10 + 48;
		test[3] = num%10 + 48;
		lp = 0;
		while( Cycbuf_WriteFrame(&U3SendBuf,test,24) < 0 )
		{
			if(lp++ > 100 )
			{
				lp = 0;
				rt_thread_delay(10);
			}
		}; 
	}	

}
FINSH_FUNCTION_EXPORT(u3sendtest,u3sendtest(int num))

u8 Uart3_PutChar(u8 ch)
{
	static uint32_t ini = 0;

	if( ini == 0 )
	{
		ini = 1;
		UART3_init(115200);
	}
	USART_SendData(USART3, (u8) ch);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
	{
	}
	return ch;
}
FINSH_FUNCTION_EXPORT(Uart3_PutChar,Uart3_PutChar(u8 ch))



