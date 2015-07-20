#include <stm32f10x.h>
#include <rtthread.h>
#include "fgpio.h"
#include "PwrServer.h"
#include "fwsRelayCtlProcess.h"
extern   unsigned int StartupMark;
 extern  uint8_t MessageTX[15];

typedef struct FLYCTL
{
	uint32_t Timeout;	/* 自动关闭时间 */
	uint32_t Flag;		/* 标记 */		
} FLYCTL;
FLYCTL fctl[FELAY_NUM] = {0};




int fwsRlyWindStateGet(void)
{
	return RELAY_Get(RLY_WIND1)?0:1;
}

void fwsRlyWindOpen(uint32_t flag,uint32_t timeout)
{

   	if( flag & RLY_WIND_PHONE_APP )
	{
		fctl[RLY_WIND1].Timeout = timeout;
		fctl[RLY_WIND1].Flag |= flag;		
		RELAY_Set(RLY_WIND1,RLY_ON);
		rt_thread_delay(1);
		fctl[RLY_WIND2].Timeout = timeout;
		fctl[RLY_WIND2].Flag |= flag;		
		RELAY_Set(RLY_WIND2,RLY_ON);
		MessageTX[11]=0x01;
	}
	else
	{
		if( FwsPwrDect_isStartup() == PWR_ON);
		{
			fctl[RLY_WIND1].Flag |= flag;
			RELAY_Set(RLY_WIND1,RLY_ON);
			rt_thread_delay(1);
			fctl[RLY_WIND2].Flag |= flag;
			RELAY_Set(RLY_WIND2,RLY_ON);
			MessageTX[11]=0x01;
		}
	}
		
		
}

void fwsRlyWindClose(uint32_t flag)
{
	fctl[RLY_WIND1].Flag = fctl[RLY_WIND1].Flag&(~flag);
	fctl[RLY_WIND2].Flag = fctl[RLY_WIND2].Flag&(~flag);
	if(( fctl[RLY_WIND1].Flag == 0 )&&( fctl[RLY_WIND2].Flag == 0 ))
	{RELAY_Set(RLY_WIND1,RLY_OFF);	
	 RELAY_Set(RLY_WIND2,RLY_OFF);	
		MessageTX[11]=0x00;
	}
}




int FwsRlySet(uint32_t id,uint32_t op,uint32_t time)
{
	if( ( id >= FELAY_NUM )||( op > RLY_ON) ) 	return -1;
	
	if( op == RLY_ON )
	{
		fctl[id].Timeout = time;
		RELAY_Set(id,RLY_ON);
	}
	else
	{
	 	RELAY_Set(id,RLY_OFF);
	}
	 
	return 0;	
}




//-------------------------------------------------------------------
void FwsRlyPowerDownAction(void)
{
	static uint8_t i;
	for(i=0;i<FELAY_NUM;i++)
		RELAY_Set(i,RLY_OFF);	

}
void FwsRelayCtlProcess_thread_entry(void* parameter)//
{
	uint8_t i;
 	RELAY_Init();
	FwsRlySet(SW0,RLY_OFF,0);
	rt_thread_delay(100);
	while(1)
	{

		
		for(i=0;i<FELAY_NUM;i++)
		{
		 	if( ( fctl[i].Timeout > 0 ) && ( fctl[i].Timeout != FELAY_DELAY_FOREVER ) )
			{
				fctl[i].Timeout--;
				if( fctl[i].Timeout == 0 )
				{
					fctl[i].Flag = RLY_NULL; 
					RELAY_Set(i,RLY_OFF);
				}	
			}	
		}                     
		rt_thread_delay(10);	 	
     
	}
}



int FwsRelayCtlProcessInit(void)
{
	rt_thread_t init_thread;

	init_thread = rt_thread_create( "Rlysvr",
									FwsRelayCtlProcess_thread_entry,
									RT_NULL,
									1024,30,10);

	if( init_thread != RT_NULL )
	{
		rt_thread_startup(init_thread);
	}

	return 0;
}


#ifdef RT_USING_FINSH
#include <finsh.h>

int fwsRlytest(void)
{
	return fwsRlyWindStateGet();
}
FINSH_FUNCTION_EXPORT(fwsRlytest,Get wind control singal)
#endif


