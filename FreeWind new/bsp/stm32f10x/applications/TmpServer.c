#include <stm32f10x.h>
#include <rtthread.h>
#include "fws_temperature.h"

extern  uint8_t MessageTX[15];

 uint8_t tmpval[4]={0};
 uint8_t tmpval2[4]={0};
//static  uint8_t tmpval[2][4]={0};

#define NUL				(void*)0

#define SP_T			100
extern unsigned char TOLCDF;





void TMP_Printf(void)
{
	static uint8_t tp[4];
	fws_DS18B20_TmpGet(DS0,tp);
	rt_kprintf("[TMP]T1=%c%d%d.%d   ",	tp[3]?'-':'+',
								tp[0],
								tp[1],
								tp[2]);
	fws_DS18B20_TmpGet(DS1,tp);
	rt_kprintf("T2=%c%d%d.%d\n",tp[3]?'-':'+',
								tp[0],
								tp[1],
								tp[2]);
}

void TmpSampleProcess_thread_entry(void* parameter)
{
	fws_DS18B20_Init(DS0);
	fws_DS18B20_Init(DS1);
	rt_thread_delay(100);
	rt_kprintf("[TMP]T Server init.\n");
	TMP_Printf();
	while(1)
	{
		// if(!TOLCDF)
		
	 {	
		 tmpval[0]=0; tmpval[1]=0; tmpval[2]=0; tmpval[3]=0;
		 fws_DS18B20_TmpGet(DS0,tmpval);
		if(tmpval[3]>0)MessageTX[1]=0X01;
		else MessageTX[1]=0X00;
		 
			MessageTX[2]=tmpval[0];
		MessageTX[3]=tmpval[1];
		 rt_thread_delay(SP_T);
		 
		 tmpval2[0]=0; tmpval2[1]=0; tmpval2[2]=0; tmpval2[3]=0;
	   fws_DS18B20_TmpGet(DS1,tmpval2);
		if(tmpval2[3]>0)	MessageTX[4]=0X01;
		else MessageTX[4]=0X00;
		MessageTX[5]=tmpval2[0];
    	MessageTX[6]=tmpval2[1];
/*
	rt_kprintf("[TMP]T1=%c%d%d   ",	MessageTX[1]?'-':'+',
								MessageTX[2],
								MessageTX[3]);
	rt_kprintf("[TMP]T2=%c%d%d   ",	MessageTX[4]?'-':'+',
								MessageTX[5],
								MessageTX[6]);
		 */
		rt_thread_delay(SP_T);
		//TMP_Printf();
		
		//TMP_Printf();
		//TOLCDF=1; 
   }
 }
}

int TmpSampleProcessInit(void)
{
	rt_thread_t init_thread;

	init_thread = rt_thread_create( "tmp",
									TmpSampleProcess_thread_entry,
									RT_NULL,
									2048,23,20);

	if( init_thread != RT_NULL )
	{
		rt_thread_startup(init_thread);
	}

	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
//FINSH_FUNCTION_EXPORT(TmpGet,TmpGet(id))
void tmpinit(void){ TmpSampleProcessInit(); }
FINSH_FUNCTION_EXPORT(tmpinit,tmp thread init)

#endif


