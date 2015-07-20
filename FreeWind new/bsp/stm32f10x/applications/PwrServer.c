#include <rtthread.h>
#include <stm32f10x.h>
#include "fws_ad.h"
#include "PwrServer.h"

extern  uint8_t MessageTX[15];
extern unsigned char TOLCDF;
  unsigned int StartupMark;

/*最高检测电压设计为32V,分压系数为8。 
当电瓶为24V时，低压报警电压是21/11=2V   对应转换值是 （2*4096）/5.0 = 1564
当电瓶为12V时，低压报警电压是10/11=1.25V  对应转换值是 （2.5*4096）/5.0 = 1024
*/
/* 进行一次判断所采样的样本数 */
#define SAMPLE_CNT					10
/* 判断是否为启动的电压差阈值 */
#define THRESHOLD_CNT_VAL			0
/* 判断是否为启动的次数阈值 */
#define THRESHOLD_CNT				0 		
/* 判断是否为电瓶低电压-- 24V */
#define THRESHOLD_VAL24_VAL	 		1519
/* 判断是否为电瓶低电压-- 12V */
#define THRESHOLD_VAL12_VAL	  		700

#define FULL_VAL24_VAL	 		1735
/* 判断是否为电瓶低电压-- 12V */
#define FULL_VAL12_VAL	  		844

/* 采样时间间隔平率 */
#define SAMPLE_FRQ					30	// old is 12


/* 系统点火启动标记 */

/* 电瓶低电压报警 */

#include<stdio.h> 

uint32_t AVG(uint16_t * BUF,uint16_t LEN) 
{ 
uint32_t min,i,max,b,sum;
    min=0xffff;
    max=0;
    sum=0;
    for(i=0;i<LEN;i++)
    {
        sum=sum+*(BUF+i);
        if(*(BUF+i)<min)
        min=*(BUF+i);
        if(*(BUF+i)>max)
        max=*(BUF+i);
    }
    sum=sum-min-max;
    b=sum/(LEN-2);
   
    return b;
} 

static uint8_t PowerLowAlarm = PWR_OFF;

int FwsPwrDect_is24V(void)				// 24V电源检测
{
	return PWR_IsPwr24V();
}

int FwsPwrDect_isStartup(void)			// 实际FwsPwrDect_isStartup，FwsPwrDect_isPowerLowAlarm，StartupMark和PowerLowAlarm等效，低电压为0
{
	if( FwsPwrDect_isPowerLowAlarm() == PWR_OFF )
		return PWR_OFF;
    else return StartupMark; 
}

int FwsPwrDect_isPowerLowAlarm(void)
{
	return PowerLowAlarm; 
}



void FwsPwrDectProcess_thread_entry(void* parameter)
{
	static uint16_t vol[SAMPLE_CNT],i,cmp,thres_cnt;
	uint32_t add;

	ADC_PwrInit();
	/* 延时1S，保证低优先级的任务也有机会在正常工作前初始化模块。 */
	rt_thread_delay(50);
	rt_kprintf("[PWR]ADC init.\n");
	ADC_PrintVal();

	while(1)
	{	
		thres_cnt = 0;
		add	= 0;
		/* 采样电压，并保存指定数量的样本 */
		for(i=0;i<SAMPLE_CNT;i++)
		{
			vol[i] = ADC_Converter();
			//add += vol[i];
			rt_thread_delay(SAMPLE_FRQ);
		}
		 add=AVG(vol,SAMPLE_CNT);//求平均值
	//	add=0x6f0;
		 //add=AVG(vol,SAMPLE_CNT);

		/* 根据不同的电瓶规格做出低压判断 */
		if( PWR_IsPwr24V() )
		{
			if( add >= THRESHOLD_VAL24_VAL )
			{StartupMark=	PowerLowAlarm = PWR_ON;}
			else
			{StartupMark=PowerLowAlarm = PWR_OFF;	}	
			
			
      //if( TOLCDF==1)
			{
				if(add>=THRESHOLD_VAL24_VAL)
        MessageTX[8]=(add-THRESHOLD_VAL24_VAL)/27;
				else  MessageTX[8]=0;
				if(add>=FULL_VAL24_VAL)
				 MessageTX[8]=8;
        
				 //TOLCDF=2;
      }
    
		}
		else
		{
			
			if( add >= THRESHOLD_VAL12_VAL )
			{StartupMark=	PowerLowAlarm = PWR_ON;}
			else
			{StartupMark=PowerLowAlarm = PWR_OFF;	}	
			 //if( TOLCDF==1)
			{
				if(add>=THRESHOLD_VAL12_VAL)
        MessageTX[8]=(add-THRESHOLD_VAL12_VAL)/18;
				else  MessageTX[8]=0;
				if(add>=FULL_VAL12_VAL)
				 MessageTX[8]=8;
				 //TOLCDF=2;
      }
			
			
			
		}
   rt_thread_delay(12);
 } 
	
}


int FwsPwrDectProcessInit(void)
{
	rt_thread_t init_thread;

	init_thread = rt_thread_create( "pwr",
									FwsPwrDectProcess_thread_entry,
									RT_NULL,
									2028,24,20);

	if( init_thread != RT_NULL )
	{
		rt_thread_startup(init_thread);
	}

	return 0;
}
