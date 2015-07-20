#include <stm32f10x.h>
#include <rtthread.h>
#include "uart3_driver.h"
#include "uart4_driver.h"
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


#define QUERY_CMD_LEN		8
uint8_t const QueryCmdCh2o[QUERY_CMD_LEN]={0x01,0x03,0x00,0x00,0x00,0x01,0x84,0x0a};
#define QUERY_RX_LEN		16
uint8_t QueryCH2ORxBuf[QUERY_RX_LEN];

static uint16_t Ch2oPPM = 0;
static uint8_t  ch2odbgen = 0;
//static uint16_t ch2odbgdat = 0xffff;
static uint16_t ch2odbgdat = 22;

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
//#define LV3					3
//#define LV4					4
//#define LV5					5


void Ch2oPPM_send_conver(uint16_t  Ch2oPPM)
{
	
	//if(TOLCDF!=3)return;
  if(Ch2oPPM<=12)
        {
           MessageTX[9]=0x00;
        }
				
				
				 else if((0<Ch2oPPM)&&(Ch2oPPM<=16))	 //	
        {
           MessageTX[9]=0x01;
        }
        else if((16<Ch2oPPM)&&(Ch2oPPM<=20))	 //
        {
           MessageTX[9]=0x02;	  
        }
				
				
				
				
        else if((20<Ch2oPPM)&&(Ch2oPPM<=28))	 // 
        {
           MessageTX[9]=0x03;	  
        }
        else if((28<Ch2oPPM)&&(Ch2oPPM<=35))	 //	
        {
           MessageTX[9]=0x04;	  
        }
        else if((35<Ch2oPPM)&&(Ch2oPPM<=40))	 //	0.2-0.25	 5
        {
          MessageTX[9]=0x05;	  
        }
				
				
				
				
        else if((40<Ch2oPPM)&&(Ch2oPPM<=48))	 //	 0.25-0.3	 6
        {
           MessageTX[9]=0x06;	  
        }
				
        
        else if((48<Ch2oPPM)&&(Ch2oPPM<=100))	 //	 0.3- 0.35
        {
           MessageTX[9]=0x07;	  
        }
        else if(100<Ch2oPPM)						    //	 0.35-
        {
           MessageTX[9]=0x08;	  
        }
        else
        {
            MessageTX[9]=0x00;
        }
        
   //TOLCDF=4;
}


void FwsCH2OdectProcess_thread_entry(void* parameter)
{
	static uint8_t  Cmd;
	static uint32_t Cnt = 0,rxlen,i; 
	static uint8_t 	frontLVL = LV0,nowLVL = LV0;
	UART3_init(9600);
//	UART4_init(9600);
	
	/* Release 1 second cpu time solt for other module init function */
	rt_thread_delay(100);
	dogtri_init();
	rt_kprintf("\n[CH2O]UART3 band=9600 init.\n");

	while(1)
	{
	#if DEBUG_DAT == 0
		/* Send query command */
		
//		FwsCOdectProcess_thread_entry();
		
   		Cycbuf_WriteFrame(&U3SendBuf,QueryCmdCh2o,QUERY_CMD_LEN);
		/* Waitting result data */
		rt_thread_delay(CH2O_DECT_DELAY);
								
		/* Get return data in rx buffer */
		rxlen = Sbuf_read(&U3RevBuf,QueryCH2ORxBuf,QUERY_RX_LEN);
	#else
	 	if( ch2odbgdat != 0xffff )
		{
			QueryCH2ORxBuf[4] = ch2odbgdat&0xff;
			QueryCH2ORxBuf[3] = ch2odbgdat>>8;
			QueryCH2ORxBuf[0] = 0x01;
			ch2odbgdat = 0xffff;	
			rxlen = 7 ;
		}
		else
		{
		 	rxlen = 0 ;
		}
		rt_thread_delay(CH2O_DECT_DELAY);
	#endif
		if( rxlen == 7 )
		{
			if( QueryCH2ORxBuf[0] == 0x01 )
			/* Data analysis */
			{ Ch2oPPM = (QueryCH2ORxBuf[3]<<8)|QueryCH2ORxBuf[4];	
			 //Ch2oPPM = 21;
			  Ch2oPPM_send_conver(Ch2oPPM);
			}
			
		}
		/*
		if( ( rxlen != 0 ) && ( ch2odbgen != 0 ) )
		{
			DB(("UART3 RX: (len = %d)\n",rxlen); )
			for(i=0;i<rxlen;i++)
			{
		   		DB(("0x%02x ",QueryCH2ORxBuf[i]); )	
			}
			DB(("\n"); )	
		}
		*/

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
				fwsRlyWindOpen(RLY_WIND_CH2O_FLAG,FELAY_DELAY_FOREVER);	//开风机
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
				fwsRlyWindOpen(RLY_WIND_CH2O_FLAG,FELAY_DELAY_FOREVER);	//开风机
			}
		

		}			


	}

	
				
}

int FwsCH2OdectProcessInit(void)
{
	rt_thread_t init_thread;

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
	ch2odbgen = op;
}
FINSH_FUNCTION_EXPORT(ch2odbg,op=0 close op=1 enable)

#if DEBUG_DAT == 1
void ch2osim(uint16_t dat)
{
	ch2odbgdat = dat;	
}
FINSH_FUNCTION_EXPORT(ch2osim,ch2odbgsim(dat))
#endif
#endif

