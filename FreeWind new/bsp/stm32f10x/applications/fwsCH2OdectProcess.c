#include <stm32f10x.h>
#include <rtthread.h>
//#include "uart3_driver.h"
//#include "uart4_driver.h"
#include "PwrServer.h"
#include "AudioServer.h"
#include "fwsRelayCtlProcess.h"
#include "fwsRelayCtlProcess.h"
#include "fgpio.h"
extern  uint8_t MessageTX[15];
extern unsigned char TOLCDF;


#define CH2O_DECT_DELAY			150
#define CH2O_DELAY_CNT			20

#define DB1(x) rt_kprintf x
#define DB(x) rt_kprintf x
//#define DB(x)
#define DEBUG_DAT 			0

#define QUERY_DATA_LEN		7
#define QUERY_CMD_LEN		8
uint8_t const QueryCmdCh2o[QUERY_CMD_LEN]={0x01,0x03,0x00,0x00,0x00,0x01,0x84,0x0a};
#define QUERY_RX_LEN		16
uint8_t QueryCH2ORxBuf[QUERY_RX_LEN];

static uint16_t Ch2oPPM = 0;
//static uint8_t  ch2odbgen = 0;
//static uint16_t ch2odbgdat = 0xffff;
//static uint16_t ch2odbgdat = 22;

#define CH2O_PPM_LEVEL0					12
#define CH2O_PPM_LEVEL1					21
#define CH2O_PPM_LEVEL2					35
#define CH2O_PPM_LEVEL3					41


#define VOICE_PALY_SET					1
#define VOICE_PALY_RESET				0						

uint16_t FwsGetCH2OPPM(void)
{
 	return Ch2oPPM;
}

#define LV0					0
#define LV1					1
#define LV2					2

void Ch2oPPM_send_conver(uint16_t  Ch2oPPM)
//{
//	
//	//if(TOLCDF!=3)return;
//  if(Ch2oPPM<=12)
//        {
//           MessageTX[9]=0x00;
//        }
//				
//				
//				 else if((0<Ch2oPPM)&&(Ch2oPPM<=16))	 //	
//        {
//           MessageTX[9]=0x01;
//        }
//        else if((16<Ch2oPPM)&&(Ch2oPPM<=20))	 //
//        {
//           MessageTX[9]=0x02;	  
//        }
//				
//				
//				
//				
//        else if((20<Ch2oPPM)&&(Ch2oPPM<=28))	 // 
//        {
//           MessageTX[9]=0x03;	  
//        }
//        else if((28<Ch2oPPM)&&(Ch2oPPM<=35))	 //	
//        {
//           MessageTX[9]=0x04;	  
//        }
//        else if((35<Ch2oPPM)&&(Ch2oPPM<=40))	 //	0.2-0.25	 5
//        {
//          MessageTX[9]=0x05;	  
//        }
//				
//				
//				
//				
//        else if((40<Ch2oPPM)&&(Ch2oPPM<=48))	 //	 0.25-0.3	 6
//        {
//           MessageTX[9]=0x06;	  
//        }
//				
//        
//        else if((48<Ch2oPPM)&&(Ch2oPPM<=100))	 //	 0.3- 0.35
//        {
//           MessageTX[9]=0x07;	  
//        }
//        else if(100<Ch2oPPM)						    //	 0.35-
//        {
//           MessageTX[9]=0x08;	  
//        }
//        else
//        {
//            MessageTX[9]=0x00;
//        }
//        
//   //TOLCDF=4;
//}
{
	if(Ch2oPPM<=12)
	{
		MessageTX[9]=0x00;
	}
	else if(Ch2oPPM<=16)
	{
		MessageTX[9]=0x01;
	}
	else if(Ch2oPPM<=20)
	{
		MessageTX[9]=0x02;
	}
	else if(Ch2oPPM<=28)
	{
		MessageTX[9]=0x03;
	}
	else if(Ch2oPPM<=35)
	{
		MessageTX[9]=0x04;
	}
	else if(Ch2oPPM<=40)
	{
		MessageTX[9]=0x05;
	}
	else if(Ch2oPPM<=48)
	{
		MessageTX[9]=0x06;
	}
	else if(Ch2oPPM<=100)
	{
		MessageTX[9]=0x07;
	}
	else if(100<Ch2oPPM)						    //	 0.35-
	{
		MessageTX[9]=0x08;	  
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
	if(size == 1)
		msg.size = size;
	else
		msg.size = size;	
	/* 发送消息到消息队列中*/
	rt_mq_send(&rx_mq, &msg, sizeof(struct rx_msg));
//	test++;
	return RT_EOK;
}

void FwsCH2OdectProcess_thread_entry(void* parameter)
{
	struct rx_msg msg;
    rt_device_t device;
	static uint8_t  Cmd;
	static uint32_t Cnt = 0; 
	static uint8_t 	frontLVL = LV0,nowLVL = LV0;
	rt_uint32_t rx_length;

	// 从RT系统中获取串口3设备
    device = rt_device_find("uart3");
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
	rt_kprintf("\n[CH2O]UART3 band=9600 init.\n");
	rt_thread_delay(50);

    while (1)
	{
		rt_thread_delay(100);
		while (rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 5) == RT_EOK)
		{
			rt_device_read(msg.dev, 0, &QueryCH2ORxBuf[0], 1);
		}
		rt_device_write(device, 0, &QueryCmdCh2o[0], QUERY_CMD_LEN);
		if (rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 50) == RT_EOK)
		{
			rt_device_read(msg.dev, 0, &QueryCH2ORxBuf[0], 1);
			if(QueryCH2ORxBuf[0] == '\01')
			{
				rx_length = 1;
				while (rx_length < QUERY_DATA_LEN)
				{
					if (rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 10) == RT_EOK)
					{
						rt_device_read(msg.dev, 0, &QueryCH2ORxBuf[rx_length++], 1);
					}
					else
					{
						rt_kprintf("\n[CH2O]ERR: not received all the data!\n");
						break;
					}
				}
				if ((rx_length == QUERY_DATA_LEN) && (crc16_modbus(QueryCH2ORxBuf, QUERY_DATA_LEN-2) == 0x100 * QueryCH2ORxBuf[QUERY_DATA_LEN-1] + QueryCH2ORxBuf[QUERY_DATA_LEN-2]))
				{
					QueryCH2ORxBuf[rx_length] = '\0';

					Ch2oPPM = (QueryCH2ORxBuf[3]<<8)|QueryCH2ORxBuf[4];	
					rt_kprintf("\n[CH2O]DATA: %d\n", Ch2oPPM);
					Ch2oPPM_send_conver(Ch2oPPM);
				}
				else
				{
					rt_kprintf("\n[CH2O]ERR: crc error!\n");
				}
			}
			else
			{
				rt_kprintf("\n[CH2O]ERR: not received the head!\n");
			}
		}
		else
		{
			rt_kprintf("\n[CH2O]ERR: not received data!\n");
		}

		/* Query power was Start up */
		if( FwsPwrDect_isStartup() == PWR_OFF )
		{
			//TODO: Close any thing	 about ch2o audio warning.
			Cnt = 0;
			Cmd = ( CH2O_MARK | WARNING_NULL );
			Sbuf_write(&Ascb,&Cmd,1);
			fwsRlyWindClose(RLY_WIND_CH2O_FLAG);
			continue;
		}

		/* If ppm safe */	   
		if( Ch2oPPM < CH2O_PPM_LEVEL1 )//
		{
			if( nowLVL > LV0  )
			{
				Cnt = 0;
				nowLVL = frontLVL = LV0;
				Cmd = ( CH2O_MARK | WARNING_NULL );
				Sbuf_write(&Ascb,&Cmd,1);
				fwsRlyWindClose(RLY_WIND_CH2O_FLAG);//关窗
			}	
		}

		/* If ppm warning */
		if( ( Ch2oPPM >= CH2O_PPM_LEVEL1 ) && ( Ch2oPPM < CH2O_PPM_LEVEL3 )  )
		{
		 	if( nowLVL == LV0 && frontLVL == LV0  )
			{
				nowLVL = LV1;
				Cnt = 0;
				Cmd = ( CH2O_MARK | CH2O_WARNING_SPEAK_MARK  );//语音提示报警
				Sbuf_write(&Ascb,&Cmd,1);
				fwsRlyWindOpen(RLY_WIND_CH2O_FLAG,FELAY_DELAY_AUTO);	//开风机
			}

			if( nowLVL == LV1 && frontLVL == LV0  )
			{
 				if( Cnt++ >= CH2O_DELAY_CNT )
				{
					Cnt = 0;
					Cmd = ( CH2O_MARK | WARNING_DIDI_MARK  );//DIDI报警
					Sbuf_write(&Ascb,&Cmd,1);
					frontLVL = LV1;					 	
				}
			}

			if( nowLVL == LV2 && frontLVL == LV2 )
			{
				nowLVL = LV1; 	
				Cmd = ( CH2O_MARK | WARNING_DIDI_MARK  );
				Sbuf_write(&Ascb,&Cmd,1);
				frontLVL = LV1;	
			}

			if( nowLVL == LV1 && frontLVL == LV2 )
			{
				frontLVL = LV1; 		
			}
		}
		/* If ppm dangerous */
		if( Ch2oPPM >= CH2O_PPM_LEVEL3 ) 
		{
			if( nowLVL < LV2  )
			{
				Cnt = 0;

				if( nowLVL == LV0 && frontLVL == LV0  )
					Cmd = ( CH2O_MARK | WARNING_DIDIDIDI_MARK |CH2O_WARNING_SPEAK_MARK  );
				else
					Cmd = ( CH2O_MARK | WARNING_DIDIDIDI_MARK   );
				Sbuf_write(&Ascb,&Cmd,1);
				nowLVL = frontLVL = LV2;
				fwsRlyWindOpen(RLY_WIND_CH2O_FLAG,FELAY_DELAY_AUTO);	//开风机
			}
		}			
	}
}

int FwsCH2OdectProcessInit(void)
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

	init_thread = rt_thread_create( "CH2O",
									FwsCH2OdectProcess_thread_entry,
									RT_NULL,
									2048,21,10);

	if( init_thread != RT_NULL )
	{
		rt_thread_startup(init_thread);
	}

	return 0;
}


#ifdef RT_USING_FINSH
#include <finsh.h>


FINSH_FUNCTION_EXPORT(FwsGetCH2OPPM,Get CH2O value.)

void ch2otest(void)
{
	FwsCH2OdectProcessInit();
}
FINSH_FUNCTION_EXPORT(ch2otest,CH2O server test)

void ch2odbg(uint8_t op)
{
//	ch2odbgen = op;
}
FINSH_FUNCTION_EXPORT(ch2odbg,op=0 close op=1 enable)

#if DEBUG_DAT == 1
void ch2osim(uint16_t dat)
{
//	ch2odbgdat = dat;	
}
FINSH_FUNCTION_EXPORT(ch2osim,ch2odbgsim(dat))
#endif
#endif

