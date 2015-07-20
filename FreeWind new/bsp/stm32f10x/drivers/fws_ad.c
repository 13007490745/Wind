#include <rtthread.h>
#include <stm32f10x.h>
#include <stdio.h>

#include "common.h"
#include "fws_ad.h"

int ADC_IOinit(void);
int PWR_DectInit(void);
int PWR_IsPwr24V(void);

#define ADCDBG(x)				rt_kprintf x
//#define ADCDBG(x)

#define ADC_OK					0
#define ADC_FAIL				-1
#define ADC_DEV_ERR				-2

#define PIN_CLK					GPIO_Pin_0
#define PIN_CONV				GPIO_Pin_1
#define PIN_DAT					GPIO_Pin_2

#define PIN_GROUP				GPIOC

#define CLK_H()					( PIN_GROUP->BSRR = PIN_CLK ) 
#define CLK_L()					( PIN_GROUP->BRR  = PIN_CLK ) 

#define CONV_H()				( PIN_GROUP->BSRR = PIN_CONV ) 
#define CONV_L()				( PIN_GROUP->BRR  = PIN_CONV ) 

#define	DAT_READ()				( PIN_GROUP->IDR & PIN_DAT	)
#define	DECT_24V()				( GPIOD->IDR & GPIO_Pin_0	)


#define SHORT_DELAY				320

#define PWR_12V					0
#define PWR_24V					1


int ADC_PwrInit(void)
{
	ADC_IOinit();
	PWR_DectInit();	
	return 0;
}

int PWR_DectInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

	return 0;
}

int PWR_IsPwr24V(void)
{
 	return ( !DECT_24V() );
}

int ADC_IOinit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = PIN_CLK;
    GPIO_Init(PIN_GROUP, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = PIN_CONV;
    GPIO_Init(PIN_GROUP, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = PIN_DAT;
    GPIO_Init(PIN_GROUP, &GPIO_InitStructure);


	CONV_H();
	CLK_L();




	return 0;

}

uint16_t ADC_Converter(void)
{
    uint32_t i;              	//用于12位数字的计数 
	uint16_t result = 0;
	
	//时序图参见文档的12页。
	CONV_H();				   	
	Delay_nop(SHORT_DELAY);							
	CONV_L();				   	// CONV下降沿表示转换启动。
	Delay_nop(SHORT_DELAY);
	CLK_H();					// 先连续两个时钟脉冲，满足时序要求
	Delay_nop(SHORT_DELAY);
	CLK_L();
	Delay_nop(SHORT_DELAY);
	CLK_H();
	Delay_nop(SHORT_DELAY);
	CLK_L();

	for(i=0;i<12;i++)	   		// 第三个脉冲开始读取12位的转换内容。
	{
		CLK_H(); 
		Delay_nop(SHORT_DELAY);
		if( DAT_READ() )
			result |= 0x0001;		
		else
			result &= 0xfffe;
		CLK_L();
		Delay_nop(SHORT_DELAY);

		result <<= 1;				
	}
	CLK_H();
	Delay_nop(SHORT_DELAY);
	CLK_L();				 	// 最后发两个脉冲满足16位的时钟对齐。
	Delay_nop(SHORT_DELAY);
	CLK_H();
	Delay_nop(SHORT_DELAY);
	CLK_L();
	Delay_nop(SHORT_DELAY);
	CONV_H();				  	// 表示转换结束。

    return(result);        		//返回值    	
}

void ADC_GetVol(uint8_t *buf)
{
	static uint16_t val;
	float vol;

	val = ADC_Converter();
	vol = (val * 5.0)/4096;
	vol = vol*3.0;			//三分之一的分压
	vol = vol * 10 + 0.5; 	//保留一位小数。
	
	buf[0] =  ( (uint32_t)( vol / 100 ) )%10 ;
	buf[1] =  ( (uint32_t)( vol / 10 ) )%10 ;
	buf[2] =  ( (uint32_t)( vol) )%10;
}


int ADC_PrintVal(void)
{
	static uint16_t val;
	float vol;
	char s[60] = {0};

	val = ADC_Converter();
	vol = (val * 5.0)/4096;
	
	sprintf(s, "%6.5f", vol*3.0); 
	////rt_kprintf("[ADC]Val=0x%04x  电压=%sV 24V检测：%s\n",val,s,(PWR_IsPwr24V())?"YES":"NO"); 		

	return 0;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
int fws_ad(uint32_t level)
{
	static int it = 0;
	if( it == 0 )
	{
	 	it = 1;
		ADC_PwrInit();	
	}
	ADC_PrintVal();
 	return 0;
}
FINSH_FUNCTION_EXPORT(fws_ad,fws_ad(level));

#endif
