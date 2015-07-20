#include <stm32f10x.h>
#include <rtthread.h>
#include "uart1_driver.h"
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


void LCDPANELProcess_thread_entry(void* parameter)
{
//	static uint8_t  Cmd;
	static uint32_t rxlen; 
	uint8_t i;

	UART1_init(9600);
	/* Release 1 second cpu time solt for other module init function */
	rt_thread_delay(100);
	//rt_kprintf("[CO]UART1 band=9600 init.\n");

	MessageTX[12]=0X00;
	MessageTX[13]=0X3B;
	while(1)
	{
		MessageTX[12] = 0x00;
		MessageTX[13] = 0x00;
	  	MessageTX[7]=0X01;
		rt_thread_delay(40);

		memset(QueryLCDRxBuf, 0, QUERY_RX_LEN);
		rxlen = Sbuf_read(&U1RevBuf,QueryLCDRxBuf,25);

//		if( rxlen == 10 )
		if(( rxlen >= 10 ) && (QueryLCDRxBuf[0] == 0xaa) && (QueryLCDRxBuf[9] == 0xcc))
		{
			MessageTX[13] = 0x01;
			if((QueryLCDRxBuf[1]==0x01)&&(StartupMark==1))//LCD启动，而且车启动时
			{
				MessageTX[13] = 0x02;
//				delay(10);
				if((QueryLCDRxBuf[2]==0x01)|(GSM_flag==1))//LCD手动模式开,且GSM有信号时
				{
					MessageTX[12] = rxlen;
					MessageTX[13] = 0x03;
					MD=0; //开风机
					MessageTX[11]=0x01;//风机启动状态
					//delay(100);	
					fwsRlyWindOpen(RLY_WIND_MD_FLAG,FELAY_DELAY_FOREVER);
					//FwsRlySet(SW1,RLY_OFF,0);
				}
    			else if((QueryLCDRxBuf[2]==0x00)&(GSM_flag==0))//LCD手动模式关,且GSM有信号时
    			{
    				MessageTX[12] = GSM_flag;
    				MessageTX[13] = 0x04;
    				MD=1;//关风机
    				//MessageTX[11]=0x00;	
					fwsRlyWindClose(RLY_WIND_MD_FLAG);
    				
    			}
    		}
    		else if((QueryLCDRxBuf[1]==0x00)||(StartupMark==0))//LCD关闭，而且车熄火
    		{
    			MessageTX[12] = StartupMark;
    			MessageTX[13] = 0x05;
				MD=1; //关风机
    			//MessageTX[11]=0x00;	
				if(StartupMark==0)
					fwsRlyWindClose(RLY_WIND_ALLOFF_FLAG);
				else
					fwsRlyWindClose(RLY_WIND_MD_FLAG);
    		}
    
			if(QueryLCDRxBuf[7]==0x01)//自动模式时
			{
//    		     MessageTX[12]=QueryLCDRxBuf[5];
//    		     MessageTX[13]=QueryLCDRxBuf[6];
    	    }
        
			Cycbuf_WriteFrame(&U1SendBuf,MessageTX,SENDTO_LCD_LEN);
		}
		else if(QueryLCDRxBuf[0] != 0xaa)
		{
			for(i=0; i<10; i++)
			{
				MessageTX[i+1] = QueryLCDRxBuf[i];
			}
			MessageTX[12] = rxlen;
			MessageTX[13] = 0x06;
			Cycbuf_WriteFrame(&U1SendBuf,MessageTX,SENDTO_LCD_LEN);
		}
		else if(QueryLCDRxBuf[9] != 0xcc)
		{
			for(i=0; i<10; i++)
			{
				MessageTX[i+1] = QueryLCDRxBuf[i];
			}
			MessageTX[12] = rxlen;
			MessageTX[13] = 0x07;
			Cycbuf_WriteFrame(&U1SendBuf,MessageTX,SENDTO_LCD_LEN);
		}
		else
		{
			MessageTX[12] = rxlen;
			MessageTX[13] = 0x08;
			Cycbuf_WriteFrame(&U1SendBuf,MessageTX,SENDTO_LCD_LEN);			
		}
	}
}

int LCDPANELProcessInit(void)
{
	rt_thread_t init_thread;

	init_thread = rt_thread_create( "lcdpanel",
									LCDPANELProcess_thread_entry,
									RT_NULL,
									4096,20,10);

	if( init_thread != RT_NULL )
	{
		rt_thread_startup(init_thread);
	}

	return 0;
}
