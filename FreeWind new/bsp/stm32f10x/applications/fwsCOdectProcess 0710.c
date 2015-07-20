#include <stm32f10x.h>
#include <rtthread.h>
#include "uart4_driver.h"
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


#define QUERY_CMD_LEN		8
uint8_t const QueryCmdCo[QUERY_CMD_LEN]={0x02,0x03,0x00,0x00,0x00,0x01,0x84,0x39};
#define QUERY_RX_LEN		16
uint8_t QueryCORxBuf[QUERY_RX_LEN];

uint16_t CoPPM = 0;
static uint8_t  codbgen = 0;
//static uint16_t codbgdat = 0xffff;
static uint16_t codbgdat = 100;


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
{
	
	//if(TOLCDF!=2)return;
  if(CoPPM==0x0000)		 //
        {
           MessageTX[10]=0x00;
        }
        
				
        else if((0<CoPPM)&(CoPPM<=110))
        {
           MessageTX[10]=0x01;
        }
        else if((180<CoPPM)&(CoPPM<=249))
        {
           MessageTX[10]=0x02;	  
        }
				
				
        else if((249<CoPPM)&(CoPPM<=300))
        {
           MessageTX[10]=0x03;	  
        }
        else if((300<CoPPM)&(CoPPM<=350))
        {
           MessageTX[10]=0x04;	  
        }
        else if((350<CoPPM)&(CoPPM<=399))
        {
           MessageTX[10]=0x05;	  
        }
				
				
        else if((399<CoPPM)&(CoPPM<=450))
        {
           MessageTX[10]=0x06;	  
        }
        
        else if((450<CoPPM)&(CoPPM<=500))
        {
           MessageTX[10]=0x07;	  
        }
        else if(500<CoPPM)
        {
           MessageTX[10]=0x08;	  
        }
				
				
        else
        {
          MessageTX[10]=0x00;
        }
   //TOLCDF=3;
}

  static  uint8_t  Cmd;
	static uint32_t Cnt = 0;
	static  uint8_t 	frontLVL = LV0,nowLVL = LV0;
void FwsCOdectProcess_thread_entry(void* parameter)
{ 

	   uint32_t rxlen; 
	
	UART4_init(9600);
	/* Release 1 second cpu time solt for other module init function */
	rt_thread_delay(100);
	dogtri_init();
	rt_kprintf("[CO]UART4 band=9600 init.\n");

	while(1)
	{ 

		/* Send query command */
   		Cycbuf_WriteFrame(&U4SendBuf,QueryCmdCo,QUERY_CMD_LEN);
		/* Waitting result data */	
		
		////DOGTRI();
	 rt_thread_delay(CO_DECT_DELAY);
//		rt_thread_delay(20);
    DOGTRI();
		rxlen = Sbuf_read(&U4RevBuf,QueryCORxBuf,QUERY_RX_LEN);
	
		
		if( rxlen == 7 )
		{
			if( QueryCORxBuf[0] == 0x02 )
			/* Data analysis */
			CoPPM = (QueryCORxBuf[3]<<8)|QueryCORxBuf[4];	
			//CoPPM = 402;
			CoPPM_send_conver(CoPPM);
			
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
//			return;
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

int FwsCOdectProcessInit(void)
{
	rt_thread_t init_thread;

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
	codbgen = op;
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
