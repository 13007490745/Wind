#include <stm32f10x.h>
#include <rtthread.h>
//#include "uart4_driver.h"
#include "PwrServer.h"
#include "AudioServer.h"
#include "fwsRelayCtlProcess.h"
#include "fgpio.h"

extern  uint8_t MessageTX[15];
extern unsigned char TOLCDF;

#define DB(x) rt_kprintf x
//#define DB(x)

#define DEBUG_DAT 			0

#define CO_DECT_DELAY			50
#define CO_DELAY_CNT			20


#define QUERY_DATA_LEN		7
#define QUERY_CMD_LEN		8
uint8_t const QueryCmdCo[QUERY_CMD_LEN]={0x02,0x03,0x00,0x00,0x00,0x01,0x84,0x39};
#define QUERY_RX_LEN		16
uint8_t QueryCORxBuf[QUERY_RX_LEN];

uint16_t CoPPM = 0;
//static uint8_t  codbgen = 0;
//static uint16_t codbgdat = 0xffff;
//static uint16_t codbgdat = 100;


#define CO_PPM_LEVEL0					230
#define CO_PPM_LEVEL1					250
#define CO_PPM_LEVEL2					380
#define CO_PPM_LEVEL3					400

#define VOICE_PALY_SET					1
#define VOICE_PALY_RESET				0						

uint16_t FwsGetCOPPM(void)
{
 	return CoPPM;
}

#define LV0					0
#define LV1					1
#define LV2					2
//#define LV3					3
//#define LV4					4
//#define LV5					5


void CoPPM_send_conver(uint16_t  CoPPM)
//{
//	
//	//if(TOLCDF!=2)return;
//  if(CoPPM==0x0000)		 //
//        {
//           MessageTX[10]=0x00;
//        }
//        
//				
//        else if((0<CoPPM)&(CoPPM<=110))
//        {
//           MessageTX[10]=0x01;
//        }
//        else if((180<CoPPM)&(CoPPM<=249))
//        {
//           MessageTX[10]=0x02;	  
//        }
//				
//				
//        else if((249<CoPPM)&(CoPPM<=300))
//        {
//           MessageTX[10]=0x03;	  
//        }
//        else if((300<CoPPM)&(CoPPM<=350))
//        {
//           MessageTX[10]=0x04;	  
//        }
//        else if((350<CoPPM)&(CoPPM<=399))
//        {
//           MessageTX[10]=0x05;	  
//        }
//				
//				
//        else if((399<CoPPM)&(CoPPM<=450))
//        {
//           MessageTX[10]=0x06;	  
//        }
//        
//        else if((450<CoPPM)&(CoPPM<=500))
//        {
//           MessageTX[10]=0x07;	  
//        }
//        else if(500<CoPPM)
//        {
//           MessageTX[10]=0x08;	  
//        }
//				
//				
//        else
//        {
//          MessageTX[10]=0x00;
//        }
//   //TOLCDF=3;
//}
{
	if(CoPPM<=0x0005)
	{
		MessageTX[10]=0x00;
	}
	else if(CoPPM<=30)
	{
		MessageTX[10]=0x01;
	}
	else if(CoPPM<=100)
	{
		MessageTX[10]=0x02;	  
	}
	else if(CoPPM<=150)
	{
		MessageTX[10]=0x03;	  
	}
	else if(CoPPM<=200)
	{
		MessageTX[10]=0x04;	  
	}
	else if(CoPPM<=250)
	{
		MessageTX[10]=0x05;	  
	}
	else if(CoPPM<=300)
	{
		MessageTX[10]=0x06;	  
	}
	else if(CoPPM<=400)
	{
		MessageTX[10]=0x07;	  
	}
	else if(500<CoPPM)
	{
		MessageTX[10]=0x08;	  
	}
}

//计算CRC校验
static uint16_t crc16_modbus(uint8_t *buf, uint16_t length)
{
	uint16_t i, crc;
	uint8_t j;

	crc = 0xffff;
	if (length == 0)
	{
		crc = 0;
	}
	else
	{
		for (i=0; i < length; i++)
		{
			crc = crc^(unsigned int)(buf[i]);
			for( j=0; j < 8; j++)
			{
				if ((crc&1)!=0 )
					crc = (crc>>1)^0xA001;
				else
					crc = crc>>1;
			}
		}
	}
	return crc;
}
///////////////////////////////////
/* UART接收消息结构*/
struct rx_msg
{
	rt_device_t dev;
	rt_size_t size;
};
/* 用于接收消息的消息队列*/
static struct rt_messagequeue rx_mq;
/* 接收线程的接收缓冲区*/
//static char uart_rx_buffer[64];
static char msg_pool[128];
//static unsigned int test = 0;
//static unsigned int test1 = 0;

/* 数据到达回调函数*/
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
	struct rx_msg msg;
	msg.dev = dev;
	msg.size = size;
	/* 发送消息到消息队列中*/
	rt_mq_send(&rx_mq, &msg, sizeof(struct rx_msg));
//	test++;
	return RT_EOK;
}

void FwsCOdectProcess_thread_entry(void* parameter)
{
	struct rx_msg msg;
    rt_device_t device;
	static uint8_t  Cmd;
	static uint32_t Cnt = 0; 
	static uint8_t 	frontLVL = LV0,nowLVL = LV0;
	rt_uint32_t rx_length;

	// 从RT系统中获取串口4设备
    device = rt_device_find("uart4");
    if (device != RT_NULL)
    {
    	/* 设置回调函数及打开设备*/
		rt_device_set_rx_indicate(device, uart_input);
		// 以读写方式打开设备
        rt_device_open(device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }
	rt_thread_delay(50);									//* Release 1 second cpu time solt for other module init function */
	IWDG_ReloadCounter();
	dogtri_init();
	rt_kprintf("\n[CO]UART3 band=9600 init.\n");
	rt_thread_delay(50);

    while (1)
	{
		rt_thread_delay(100);
		while (rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 5) == RT_EOK)
		{
			rt_device_read(msg.dev, 0, &QueryCORxBuf[0], 1);
		}
		rt_device_write(device, 0, &QueryCmdCo[0], QUERY_CMD_LEN);
		if (rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 50) == RT_EOK)
		{
			rt_device_read(msg.dev, 0, &QueryCORxBuf[0], 1);
			if(QueryCORxBuf[0] == '\02')
			{
				rx_length = 1;
				while (rx_length < QUERY_DATA_LEN)
				{
					if (rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 10) == RT_EOK)
					{
						rt_device_read(msg.dev, 0, &QueryCORxBuf[rx_length++], 1);
					}
					else
					{
						rt_kprintf("\n[CO]ERR: not received all the data!\n");
						break;
					}
				}
				if ((rx_length == QUERY_DATA_LEN) && (crc16_modbus(QueryCORxBuf, QUERY_DATA_LEN-2) == 0x100 * QueryCORxBuf[QUERY_DATA_LEN-1] + QueryCORxBuf[QUERY_DATA_LEN-2]))
				{
					QueryCORxBuf[rx_length] = '\0';

					CoPPM = (QueryCORxBuf[3]<<8)|QueryCORxBuf[4];	
					CoPPM_send_conver(CoPPM);
				}
				else
				{
					rt_kprintf("\n[CO]ERR: crc error!\n");
				}
			}
			else
			{
				rt_kprintf("\n[CO]ERR: not received the head!\n");
			}
		}
		else
		{
			rt_kprintf("\n[CO]ERR: not received data!\n");
		}

		/* Query power was Start up */
		if( FwsPwrDect_isStartup() == PWR_OFF )
		{
			//TODO: Close any thing	 about CO audio warning.
			Cnt = 0;
			nowLVL = frontLVL = LV0;
			Cmd = ( CO_MARK | WARNING_NULL );
			Sbuf_write(&Ascb,&Cmd,1);
			fwsRlyWindClose(RLY_WIND_CO_FLAG);
			continue;
		}

		/* If ppm safe */
		if( CoPPM < CO_PPM_LEVEL1 )
		{
			if( nowLVL > LV0  )
			{
				Cnt = 0;
				nowLVL = frontLVL = LV0;
				Cmd = ( CO_MARK | WARNING_NULL );
				Sbuf_write(&Ascb,&Cmd,1);
				fwsRlyWindClose(RLY_WIND_CO_FLAG);
			}	
		}

		/* If ppm warning */
		if( ( CoPPM >= CO_PPM_LEVEL1 ) && ( CoPPM < CO_PPM_LEVEL3 )  )
		{
		 	if( nowLVL == LV0 && frontLVL == LV0  )
			{
				nowLVL = LV1;
				Cnt = 0;
				Cmd = ( CO_MARK | CO_WARNING_SPEAK_MARK  );
				Sbuf_write(&Ascb,&Cmd,1);
				fwsRlyWindOpen(RLY_WIND_CO_FLAG,FELAY_DELAY_FOREVER);	
			}
			if( nowLVL == LV1 && frontLVL == LV0  )
			{
 				if( Cnt++ >= CO_DELAY_CNT )
				{
					Cnt = 0;
					Cmd = ( CO_MARK | WARNING_DIDI_MARK  );
					Sbuf_write(&Ascb,&Cmd,1);
					frontLVL = LV1;					 	
				}
			}

			if( nowLVL == LV2 && frontLVL == LV2 )
			{
				nowLVL = LV1; 	
				Cmd = ( CO_MARK | WARNING_DIDI_MARK  );
				Sbuf_write(&Ascb,&Cmd,1);
				frontLVL = LV1;	
			}

			if( nowLVL == LV1 && frontLVL == LV2 )
			{
				frontLVL = LV1; 		
			}
		}
		/* If ppm dangerous */
		if( CoPPM >= CO_PPM_LEVEL3 ) 
		{
			if( nowLVL < LV2  )
			{
				Cnt = 0;

				if( nowLVL == LV0 && frontLVL == LV0  )
					Cmd = ( CO_MARK | WARNING_DIDIDIDI_MARK |CO_WARNING_SPEAK_MARK  );
				else
					Cmd = ( CO_MARK | WARNING_DIDIDIDI_MARK   );
				Sbuf_write(&Ascb,&Cmd,1);
				nowLVL = frontLVL = LV2;
				fwsRlyWindOpen(RLY_WIND_CO_FLAG,FELAY_DELAY_FOREVER);	
			}
		}			
	}
}
//  static  uint8_t  Cmd;
//	static uint32_t Cnt = 0;
//	static  uint8_t 	frontLVL = LV0,nowLVL = LV0;
//void FwsCOdectProcess_thread_entry(void* parameter)
//{ 
//
//	   uint32_t rxlen; 
//	
//	UART4_init(9600);
//	/* Release 1 second cpu time solt for other module init function */
//	rt_thread_delay(100);
//	dogtri_init();
//	rt_kprintf("[CO]UART4 band=9600 init.\n");
//
//	while(1)
//	{ 
//
//		/* Send query command */
//   		Cycbuf_WriteFrame(&U4SendBuf,QueryCmdCo,QUERY_CMD_LEN);
//		/* Waitting result data */	
//		
//		////DOGTRI();
//	 rt_thread_delay(CO_DECT_DELAY);
////		rt_thread_delay(20);
//    DOGTRI();
//		rxlen = Sbuf_read(&U4RevBuf,QueryCORxBuf,QUERY_RX_LEN);
//	
//		
//		if( rxlen == 7 )
//		{
//			if( QueryCORxBuf[0] == 0x02 )
//			/* Data analysis */
//			CoPPM = (QueryCORxBuf[3]<<8)|QueryCORxBuf[4];	
//			//CoPPM = 402;
//			CoPPM_send_conver(CoPPM);
//			
//		}
//
//
//		/* Query power was Start up */
//		if( FwsPwrDect_isStartup() == PWR_OFF )
//		{
//			//TODO: Close any thing	 about CO audio warning.
//			Cnt = 0;
//			nowLVL = frontLVL = LV0;
//			Cmd = ( CO_MARK | WARNING_NULL );
//			Sbuf_write(&Ascb,&Cmd,1);
//			fwsRlyWindClose(RLY_WIND_CO_FLAG);
//			continue;
////			return;
//		}
//
//
//
//		/* If ppm safe */
//		if( CoPPM < CO_PPM_LEVEL1 )
//		{
//			
//
//			if( nowLVL > LV0  )
//			{
//				Cnt = 0;
//				nowLVL = frontLVL = LV0;
//				Cmd = ( CO_MARK | WARNING_NULL );
//				Sbuf_write(&Ascb,&Cmd,1);
//				fwsRlyWindClose(RLY_WIND_CO_FLAG);
//
//			}	
//		}
//
//		/* If ppm warning */
//		if( ( CoPPM >= CO_PPM_LEVEL1 ) && ( CoPPM < CO_PPM_LEVEL3 )  )
//		{
//		 	if( nowLVL == LV0 && frontLVL == LV0  )
//			{
//				nowLVL = LV1;
//				Cnt = 0;
//				Cmd = ( CO_MARK | CO_WARNING_SPEAK_MARK  );
//				Sbuf_write(&Ascb,&Cmd,1);
//				fwsRlyWindOpen(RLY_WIND_CO_FLAG,FELAY_DELAY_FOREVER);	
//			}
//
//			if( nowLVL == LV1 && frontLVL == LV0  )
//			{
// 				if( Cnt++ >= CO_DELAY_CNT )
//				{
//					Cnt = 0;
//					Cmd = ( CO_MARK | WARNING_DIDI_MARK  );
//					Sbuf_write(&Ascb,&Cmd,1);
//					frontLVL = LV1;					 	
//				}
//			}
//
//			if( nowLVL == LV2 && frontLVL == LV2 )
//			{
//				nowLVL = LV1; 	
//				Cmd = ( CO_MARK | WARNING_DIDI_MARK  );
//				Sbuf_write(&Ascb,&Cmd,1);
//				frontLVL = LV1;	
//			}
//
//			if( nowLVL == LV1 && frontLVL == LV2 )
//			{
//				frontLVL = LV1; 		
//			}
//		}
//		/* If ppm dangerous */
//		if( CoPPM >= CO_PPM_LEVEL3 ) 
//		{
//			if( nowLVL < LV2  )
//			{
//				Cnt = 0;
//
//				if( nowLVL == LV0 && frontLVL == LV0  )
//					Cmd = ( CO_MARK | WARNING_DIDIDIDI_MARK |CO_WARNING_SPEAK_MARK  );
//				else
//					Cmd = ( CO_MARK | WARNING_DIDIDIDI_MARK   );
//				Sbuf_write(&Ascb,&Cmd,1);
//				nowLVL = frontLVL = LV2;
//				fwsRlyWindOpen(RLY_WIND_CO_FLAG,FELAY_DELAY_FOREVER);	
//			}
//		
//		}			
//
//	}	
//				
//}

int FwsCOdectProcessInit(void)
{
	rt_thread_t init_thread;
    rt_err_t result = RT_EOK;


	// 创建消息队列，分配队列存储空间
    result = rt_mq_init(&rx_mq, "mqt", &msg_pool[0], 12 - sizeof(void*), sizeof(msg_pool), RT_IPC_FLAG_FIFO);
   
    if (result != RT_EOK) 
    { 
        rt_kprintf("init message queue failed.\n"); 
        return result; 
    }

	init_thread = rt_thread_create( "CO",
									FwsCOdectProcess_thread_entry,
									RT_NULL,
									2048,20,10);

	if( init_thread != RT_NULL )
	{
		rt_thread_startup(init_thread);
	}

	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(FwsGetCOPPM,Get CO PPM value)

void cotest(void)
{
	FwsCOdectProcessInit();
}

FINSH_FUNCTION_EXPORT(cotest,CO server test)

void codbg(uint8_t op)
{
//	codbgen = op;
}
FINSH_FUNCTION_EXPORT(codbg,op=0 close op=1 enable)

#if DEBUG_DAT == 1
void cosim(uint16_t dat)
{
	codbgdat = dat;	
}
FINSH_FUNCTION_EXPORT(cosim,cosim(dat))
#endif

#endif
