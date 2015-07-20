#include <stm32f10x.h>
#include <rtthread.h>
//#include "uart1_driver.h"
#include "PwrServer.h"
#include "AudioServer.h"
#include "fwsRelayCtlProcess.h"
#include "fws_ad.h"
#include "fgpio.h"
#include "cycbuf.h"
#include "string.h"

extern   unsigned int StartupMark;

#define SENDTO_LCD_LEN		15
//const uint8_t QueryCmdLCD[QUERY_LCD_LEN]={0x02,0x03,0x00,0x00,0x00,0x01,0x84,0x39};
#define QUERY_RX_LEN		30
uint8_t QueryLCDRxBuf[QUERY_RX_LEN];
//// ONOFF AUTO,time1,time2,time3,time4, bit,
uint8_t TOLCDF=0;
uint8_t GSM_flag=0;
uint8_t MD=0;
uint8_t  MessageTX[15]={0xaa,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x3B,0xcc};

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
static unsigned int test = 0;
static unsigned int test1 = 0;

/* 数据到达回调函数*/
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
	struct rx_msg msg;
	msg.dev = dev;
	msg.size = size;
	/* 发送消息到消息队列中*/
	rt_mq_send(&rx_mq, &msg, sizeof(struct rx_msg));
	test++;
	return RT_EOK;
}

void LCDPANELProcess_thread_entry(void* parameter)
{
	struct rx_msg msg;
    rt_device_t device;

	// 从RT系统中获取串口1设备
    device = rt_device_find("uart1");
    if (device != RT_NULL)
    {
    	/* 设置回调函数及打开设备*/
		rt_device_set_rx_indicate(device, uart_input);
		// 以读写方式打开设备
        rt_device_open(device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }
	rt_thread_delay(50);									//* Release 1 second cpu time solt for other module init function */
	IWDG_ReloadCounter();
	rt_thread_delay(50);

    while (1)
    {
		IWDG_ReloadCounter();
		MessageTX[7]=0X01;
        rt_thread_delay( RT_TICK_PER_SECOND/20 ); /* sleep 0.5 second and switch to other thread */

		while (rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 50) == RT_EOK)
		{
			rt_uint32_t rx_length;
			test1++;
			rx_length = (sizeof(QueryLCDRxBuf) - 1) > msg.size ? msg.size : sizeof(QueryLCDRxBuf) - 1;
			/* 读取消息*/
			rx_length = rt_device_read(msg.dev, 0, &QueryLCDRxBuf[0], rx_length);
			if(QueryLCDRxBuf[0] != 0xaa)
			{
				break;
			}
			while (rx_length < 10)
			{
				if (rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 5) == RT_EOK)
				{
					rt_device_read(msg.dev, 0, &QueryLCDRxBuf[rx_length++], 1);
				}
				else
				{
					break;
				}
			}
			if ((rx_length == 10) && (QueryLCDRxBuf[9] == 0xcc))
			{
				QueryLCDRxBuf[rx_length] = '\0';

				if((QueryLCDRxBuf[1]==0x01)&&(StartupMark==1))//LCD启动，而且车启动时
				{
					if((QueryLCDRxBuf[2]==0x01)|(GSM_flag==1))//LCD手动模式开,且GSM有信号时
					{
//						MessageTX[12] = rxlen;
						MessageTX[13] = 0x03;
						MD=0; //开风机
						MessageTX[11]=0x01;//风机启动状态
						fwsRlyWindOpen(RLY_WIND_MD_FLAG,FELAY_DELAY_FOREVER);
					}
	    			else if((QueryLCDRxBuf[2]==0x00)&(GSM_flag==0))//LCD手动模式关,且GSM有信号时
    				{
//    					MessageTX[12] = rxlen;
    					MessageTX[13] = 0x04;
    					MD=1;//关风机
						fwsRlyWindClose(RLY_WIND_MD_FLAG);
	    			}
	    			else
	    			{
//						MessageTX[12] = rxlen;
						MessageTX[13] = 0x02;
	    			}
    			}
    			else if((QueryLCDRxBuf[1]==0x00)||(StartupMark==0))//LCD关闭，而且车熄火
    			{
					MessageTX[12] = StartupMark;
    				MessageTX[13] = 0x05;
					MD=1; //关风机
					if(StartupMark==0)
						fwsRlyWindClose(RLY_WIND_ALLOFF_FLAG);
					else
						fwsRlyWindClose(RLY_WIND_MD_FLAG);
    			}
    			else
    			{
//					MessageTX[12] = rxlen;
					MessageTX[13] = 0x06;
				}
//				if(QueryLCDRxBuf[7]==0x01)//自动模式时
//				{
//    			     MessageTX[12]=QueryLCDRxBuf[5];
//    			     MessageTX[13]=QueryLCDRxBuf[6];
//    	    	}
			}
			else
			{
				break;
			}
			rt_device_write(msg.dev, 0, &MessageTX[0], SENDTO_LCD_LEN);
		}
    }
}

int LCDPANELProcessInit(void)
{
	rt_thread_t init_thread;
    rt_err_t result = RT_EOK;

	// 创建消息队列，分配队列存储空间
    result = rt_mq_init(&rx_mq, "mqt", &msg_pool[0], 32 - sizeof(void*), sizeof(msg_pool), RT_IPC_FLAG_FIFO);
   
    if (result != RT_EOK) 
    { 
        rt_kprintf("init message queue failed.\n"); 
        return result; 
    }

	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);			// Enable write access to IWDG_PR and IWDG_RLR registers
	IWDG_SetPrescaler(IWDG_Prescaler_32);					// IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz 
	IWDG_SetReload(1250);									// Set counter reload value to 1250 
	IWDG_ReloadCounter();									// Reload IWDG counter 
//	IWDG_Enable();											// Enable IWDG (the LSI oscillator will be enabled by hardware) 

	init_thread = rt_thread_create( "lcdpanel",
									LCDPANELProcess_thread_entry,
									RT_NULL,
									2048,10,10);

	if( init_thread != RT_NULL )
	{
		rt_thread_startup(init_thread);
	}

	return 0;
}
