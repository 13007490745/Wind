#include <rtthread.h>
#include <stm32f10x.h>
#include <rthw.h>
#include "common.h"
#include "fws_temperature.h"

#define DS0					1
#define DS1					2

#define TMP_OK				0
#define TMP_FAIL			-1
#define TMP_DEV_ERR			-2

#define GPIO_DS0			GPIO_Pin_6 
#define GPIO_DS1			GPIO_Pin_7		

#define	DS0_H()				(GPIOD->BSRR = GPIO_DS0	)
#define	DS0_L()				(GPIOD->BRR  = GPIO_DS0	)

#define	DS1_H()				(GPIOD->BSRR = GPIO_DS1	)	
#define	DS1_L()				(GPIOD->BRR  = GPIO_DS1	)

#define	DS0_READ()			(GPIOD->IDR&GPIO_DS0	)
#define	DS1_READ()			(GPIOD->IDR&GPIO_DS1	)

int DS18B20_ByteWriteCH0(uint8_t dat);
int DS18B20_ByteReadCH0 (uint8_t *dat);
int DS18B20_ByteWriteCH1(uint8_t dat);
int DS18B20_ByteReadCH1 (uint8_t *dat);

int DS18B20_IOinit(void);
void DS0_OutputMode(void);
void DS0_InputMode(void);
void DS1_OutputMode(void);
void DS1_InputMode(void);


#define TMPDBG(x)				rt_kprintf x
//#define TMPDBG(x)
int fws_DS18B20_Init( uint32_t id )
{
	int16_t init = TMP_FAIL;
	static uint8_t starup = 0;
	
	if( starup == 0 )
	{
		starup = 1; 
		DS18B20_IOinit();
	}

	if( id == 1 )
	{
		DS0_OutputMode();
		DS0_L();
		rt_thread_delay(1);
		DS0_H();

	}
	else if( id == 2 )
	{
		DS1_OutputMode();
		DS1_L();
		rt_thread_delay(1);
		DS1_H();
	}

   	return init;

}

//����ID��DS18B20��ѡ�񡣲���BUF�Ƿ��ص��¶�ֵ�����һλΪ����λ������28.5���� 2 8 5 0  ����-10.3���� 1 0 3 1
int fws_DS18B20_TmpGet( uint32_t id, uint8_t *buf )
{
	rt_uint32_t level;
	uint8_t 	hbyte,lbyte;
	static uint16_t 	und,tv,diff;
	static uint8_t StarupFlg = 0,errcnt=0;
	double		tval;			
	if( id == 1 )
	{
		fws_DS18B20_Init(id);
		rt_thread_delay(1);
		level = rt_hw_interrupt_disable();
		DS18B20_ByteWriteCH0(0xCC); // ����������кŵĲ���
		DS18B20_ByteWriteCH0(0x44); // �����¶�ת��
		rt_hw_interrupt_enable(level);

		rt_thread_delay(1);

		fws_DS18B20_Init(id);
		rt_thread_delay(1);
		level = rt_hw_interrupt_disable();
		DS18B20_ByteWriteCH0(0xCC); //����������кŵĲ���
		DS18B20_ByteWriteCH0(0xBE); //��ȡ�¶ȼĴ���
		Delay_Us(1);

		DS18B20_ByteReadCH0(&lbyte);
		DS18B20_ByteReadCH0(&hbyte);
		rt_hw_interrupt_enable(level);
		 	 	
	}
	else if( id == 2 )
	{
		fws_DS18B20_Init(id);

		rt_thread_delay(1);
		level = rt_hw_interrupt_disable();
		DS18B20_ByteWriteCH1(0xCC); // ����������кŵĲ���
		DS18B20_ByteWriteCH1(0x44); // �����¶�ת��
		rt_hw_interrupt_enable(level);

		rt_thread_delay(1);

		fws_DS18B20_Init(id);
		rt_thread_delay(1);
		level = rt_hw_interrupt_disable();
		DS18B20_ByteWriteCH1(0xCC); //����������кŵĲ���
		DS18B20_ByteWriteCH1(0xBE); //��ȡ�¶ȼĴ���

		DS18B20_ByteReadCH1(&lbyte);
		DS18B20_ByteReadCH1(&hbyte);
		rt_hw_interrupt_enable(level);
		
	}

	tv = (hbyte<<8) | lbyte ;
	und = tv;
	/*
	if( StarupFlg == 0 )
	{
		StarupFlg = 1;	
		und = tv;
	}
	else
	{
		if( tv > und )
			diff = tv - und;
		else
			diff = und - tv;

		if( diff > 160 )
		{
			errcnt++;
			if(errcnt > 5 )
			{
			 	errcnt = 0;
				und = tv;
			}
		}
		else
		{
			errcnt = 0;
			und = tv;	
		}	
	}
  */
	if( (und & 0xfe00) == 0xfe00 )	   		//�ж��Ƿ�Ϊ0�����£���
	{
		tval = (~und + 1) * 0.0625;
		//���Ϊ�������
		buf[3] 	= 1;
	}
	else
	{
		tval = und * 0.0625;
		//�����ȸ����
	 	buf[3] 	= 0;
	}
	
	tval = tval * 10 + 0.5; 	//����һλС����
	
	buf[0] =  ( (uint32_t)( tval / 100 ) )%10 ;
	buf[1] =  ( (uint32_t)( tval / 10 ) )%10 ;
	buf[2] =  ( (uint32_t)( tval) )%10;

	return 0;
}


int DS18B20_IOinit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

	DS0_H();
	DS1_H();

	return 0;

}


/******************************************
�������ƣ�GPIO_DQ_Out_Mode
��    �ܣ�����DQ����Ϊ��©���ģʽ
��    ������
����ֵ  ����
*******************************************/
void DS0_OutputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure ;
    

    GPIO_InitStructure.GPIO_Pin = GPIO_DS0 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD ; //��©���
    GPIO_Init(GPIOD ,&GPIO_InitStructure) ;
}
void DS1_OutputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure ;
    

    GPIO_InitStructure.GPIO_Pin = GPIO_DS1 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD ; //��©���
    GPIO_Init(GPIOD ,&GPIO_InitStructure) ;
}
/******************************************
�������ƣ�GPIO_DQ_Input_Mode
��    �ܣ�����DQ����Ϊ��������ģʽ
��    ������
����ֵ  ����
*******************************************/
void DS0_InputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure ;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_DS0 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ; //��������
    GPIO_Init(GPIOD ,&GPIO_InitStructure) ;
}

void DS1_InputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure ;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_DS1 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ; //��������
    GPIO_Init(GPIOD ,&GPIO_InitStructure) ;
}


int DS18B20_ByteWriteCH0(uint8_t dat)
{
	uint32_t i = 0;
	DS0_OutputMode();

	for (i=0; i < 8; i++)
	{
        if(dat&0x01)    //��λ��ǰ
        {
            DS0_L() ; //дʱ���϶���Ǵ����ߵĵ͵�ƽ��ʼ
            Delay_Us(2) ;  //15us������
            DS0_H() ;
            Delay_Us(65) ; //##caai old=80����д1ʱ϶������60us
        }
        else
        {
            DS0_L() ;
            Delay_Us(65) ; //##caai old=110 ������60us��120us֮��
            DS0_H() ;
            Delay_Us(5) ;
        }
        dat >>= 1 ;

	}

	return 0;
}


int DS18B20_ByteReadCH0(uint8_t *dat)
{
	uint32_t i = 0;
	uint8_t  ret = 0;

	for (i=0; i < 8; i++)
	{
		DS0_OutputMode();
        ret >>= 1 ;
        DS0_L() ;
        Delay_Us(2) ;
		DS0_InputMode();
        Delay_Us(1) ;
        if(DS0_READ())
        {
            ret |= 0x80 ;
        }
        Delay_Us(65) ;   //�ȴ���һλ������ɴ���
	}

	*dat = ret;
	DS0_OutputMode();
	return (TMP_OK);
}



int DS18B20_ByteWriteCH1(uint8_t dat)
{
	uint32_t i = 0;
	DS1_OutputMode();

	for (i=0; i < 8; i++)
	{
        if(dat&0x01)    //��λ��ǰ
        {
            DS1_L() ; //дʱ���϶���Ǵ����ߵĵ͵�ƽ��ʼ
            Delay_Us(2) ;  //15us������
            DS1_H() ;
            Delay_Us(65) ; //����д1ʱ϶������60us
        }
        else
        {
            DS1_L() ;
            Delay_Us(65) ; //������60us��120us֮��
            DS1_H() ;
            Delay_Us(5) ;
        }
        dat >>= 1 ;

	}

	return 0;
}


int DS18B20_ByteReadCH1(uint8_t *dat)
{
	uint32_t i = 0;
	uint8_t  ret = 0;

	for (i=0; i < 8; i++)
	{
		DS1_OutputMode();
        ret >>= 1 ;
        DS1_L() ;
        Delay_Us(2) ;
		DS1_InputMode();
        Delay_Us(1) ;
        if(DS1_READ())
        {
            ret |= 0x80 ;
        }
        Delay_Us(65) ;   //�ȴ���һλ������ɴ���
	}

	*dat = ret;
	DS1_OutputMode();
	return (TMP_OK);
}



#ifdef RT_USING_FINSH
#include <finsh.h>
int fws_tmp(uint32_t index)
{
	static uint8_t tbuf[10];

	fws_DS18B20_Init(index);

	fws_DS18B20_TmpGet(index,tbuf);
 
	TMPDBG(("�¶� = %c%d%d.%d",tbuf[3]?'-':'+',tbuf[0],tbuf[1],tbuf[2]);)

	return 0;
}
FINSH_FUNCTION_EXPORT(fws_tmp,fws_tmp(index))

void tmpwrite(uint8_t dat)
{
 	DS18B20_ByteWriteCH0(dat);	
}
FINSH_FUNCTION_EXPORT(tmpwrite,write byte)

#endif
