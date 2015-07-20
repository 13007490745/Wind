#ifndef __DS18B20_H
#define __DS18B20_H
typedef enum{FAILED = 0 ,PASSED = !FAILED} TestStatus ;
#define DQ_GPIO  GPIOD
#define DQ_GPIO_Pin GPIO_Pin_6
//REGISTER COMMANDS
#define REGISTER_9_BITS  0x1F
#define REGISTER_10_BITS 0x3F
#define REGISTER_11_BITS 0x5F
#define REGISTER_12_BIT2 0x7F
//ROM COMMANDS
#define ROM_Search_Cmd   0xF0
#define ROM_Read_Cmd     0x33
#define ROM_Match_Cmd    0x55
#define ROM_Skip_Cmd     0xCC
#define ROM_AlarmSearch_Cmd 0xEC
//DS18b20 FUNCTION COMMANDS
#define Convert_T          0x44
#define Write_Scratchpad  0x4E
#define Read_Scratchpad   0xBE
#define Copy_Scratchpad   0x48
#define Recall_EEPROM     0x88
#define Read_PowerSupply  0x84
#define DQ_Write_1()     GPIO_SetBits(DQ_GPIO ,DQ_GPIO_Pin)  //д1
#define DQ_Write_0()     GPIO_ResetBits(DQ_GPIO ,DQ_GPIO_Pin)//д0
#define DQ_ReadBit()     GPIO_ReadInputDataBit(DQ_GPIO ,DQ_GPIO_Pin) //��DQ�ϵ�ֵ
extern void GPIO_DQ_Out_Mode(void) ; //DQ���ģʽ
extern void GPIO_DQ_Input_Mode(void) ;  //DQ����ģʽ
extern void Tx_ResetPulse(void) ;    //���͸�λ����
extern void Rx_PresencePulse(void) ; //����Ӧ���ź�
extern void Write_OneByte_ToDS18b20(unsigned char data) ; //дһ���ֽڵ�18b20
extern unsigned char Read_OneByte_FromDS18b20(void) ; //��18b20��һ���ֽ�
extern void Read_Temperature(unsigned char *sign ,unsigned char *interger ,
                      unsigned int *decimal) ; //���¶�
//д���ò���TH��TL�ͳ�ʼ�����üĴ���
extern void Write_EEPROM(unsigned char Th,unsigned char Tl,unsigned char Register_Con );
void DS18B20_Init(void) ; //��ʼ��
#endif /*DS18B20*/
//=================================================================//
//=========================DS18b20.c================================//
#include "stm32f10x.h"
#include "common.h"
/******************************************
�������ƣ�GPIO_DQ_Out_Mode
��    �ܣ�����DQ����Ϊ��©���ģʽ
��    ������
����ֵ  ����
*******************************************/
void GPIO_DQ_Out_Mode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure ;
    

    GPIO_InitStructure.GPIO_Pin = DQ_GPIO_Pin ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD ; //��©���
    GPIO_Init(DQ_GPIO ,&GPIO_InitStructure) ;
}
/******************************************
�������ƣ�GPIO_DQ_Input_Mode
��    �ܣ�����DQ����Ϊ��������ģʽ
��    ������
����ֵ  ����
*******************************************/
void GPIO_DQ_Input_Mode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure ;
    
    GPIO_InitStructure.GPIO_Pin = DQ_GPIO_Pin ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ; //��������
    GPIO_Init(DQ_GPIO ,&GPIO_InitStructure) ;
}
/******************************************
�������ƣ�Tx_ResetPulse
��    �ܣ����͸�λ����
��    ������
����ֵ  ����
*******************************************/
void Tx_ResetPulse(void)
{
    GPIO_DQ_Out_Mode() ;
    DQ_Write_0() ;  //��λ����
    Delay_Us(500) ; //���ٱ���480us
    DQ_Write_1() ;  //�����������ٶ�
    Delay_Us(1) ;
}
/******************************************
�������ƣ�Rx_PresencePulse
��    �ܣ�����Ӧ���ź�
��    ������
����ֵ  ����
*******************************************/
void Rx_PresencePulse(void)
{
    GPIO_DQ_Input_Mode() ;
    while( DQ_ReadBit()) ;  //�ȴ�DS18b20Ӧ��
    while( DQ_ReadBit() == 0) ; //DS18b20����������60~240us ,Ȼ��������������������
    Delay_Us(300) ;
    GPIO_DQ_Out_Mode() ;    //������ɣ��������¿�������
}
/******************************************
�������ƣ�Write_OneByte_ToDS18b20
��    �ܣ�дһ���ֽڵ�DS18b20
��    ������
����ֵ  ����
*******************************************/
void Write_OneByte_ToDS18b20(unsigned char data)
{
    unsigned char i ;
    GPIO_DQ_Out_Mode() ;
    for(i=0 ;i<8 ;i++)
    {
        if(data&0x01)    //��λ��ǰ
        {
            //д1
            DQ_Write_0() ; //дʱ���϶���Ǵ����ߵĵ͵�ƽ��ʼ
            Delay_Us(8) ;  //15us������
            DQ_Write_1() ;
            Delay_Us(80) ; //����д1ʱ϶������60us
        }
        else
        {
            //д0
            DQ_Write_0() ;
            Delay_Us(110) ; //������60us��120us֮��
            DQ_Write_1() ;
            Delay_Us(5) ;
        }
        data >>= 1 ;
    }
}
/******************************************
�������ƣ�Read_OneByte_FromDS18b20
��    �ܣ���DS18b20��һ���ֽ�
��    ������
����ֵ  ������������
*******************************************/
unsigned char Read_OneByte_FromDS18b20(void)
{
    unsigned char i ,data = 0 ;
    
    for(i=0 ;i<8 ;i++)
    {
        GPIO_DQ_Out_Mode() ;
        data >>= 1 ;
        DQ_Write_0() ;
        Delay_Us(2) ;
        GPIO_DQ_Input_Mode() ;
        Delay_Us(1) ;
        if(DQ_ReadBit())
        {
            data |= 0x80 ;
        }
        Delay_Us(70) ;   //�ȴ���һλ������ɴ���
    }
    GPIO_DQ_Out_Mode() ;
    return data ;
}
/******************************************
�������ƣ�Read_Temperature
��    �ܣ���ȡ�¶���Ϣ
��    ����*sign - ������ţ����ϻ����£�
          *integer - ������������
          *decimal - ����С������
����ֵ  ����
*******************************************/
void Read_Temperature(unsigned char *sign ,
                      unsigned char *interger ,
                      unsigned int *decimal)
{
    unsigned char a=0;
    unsigned char b=0;
    //volatile unsigned char c=0;
    //volatile unsigned char d=0;
    //volatile unsigned char e=0;
    
    unsigned int tmp ;
    
    DS18B20_Init();
    Write_OneByte_ToDS18b20(ROM_Read_Cmd);
  
    DS18B20_Init();
    Write_OneByte_ToDS18b20(ROM_Skip_Cmd);//���������кŲ���
    Write_OneByte_ToDS18b20(Convert_T); //�����¶�ת��
    Delay_Ms(780);//�ȴ�DS18b20ת�����
    
    DS18B20_Init();
    Write_OneByte_ToDS18b20(ROM_Skip_Cmd);
    Write_OneByte_ToDS18b20(Read_Scratchpad); //��ȡ�Ĵ������ݣ����ԴӼĴ���0�����Ĵ���8��
    
    a= Read_OneByte_FromDS18b20();     //�¶ȵ�8λ
    b= Read_OneByte_FromDS18b20();     //�¶ȸ�8λ
    //c= Read_OneByte_FromDS18B20();   //TH
    //d= Read_OneByte_FromDS18B20();   //TL
    //e= Read_OneByte_FromDS18B20();   //Configuration Register
    
    Tx_ResetPulse();  //�ж����ݶ�ȡ
    tmp = (b<<8) | a ;
    if(b & 0xF0)
    {
    *sign = 1 ;              //���Ų���
    tmp = ~tmp+1 ;
    }
    else 
    {
    sign = 0 ;
    }
    *interger = (tmp>>4) & 0x00FF;  //��������
    *decimal = (tmp & 0x000F) * 625 ; //С������ 
}
/******************************************
�������ƣ�Write_EEPROM
��    �ܣ�д���ò���
��    ����Th - �����¶�����
          Tl - �����¶�����
          Register_Con - ���ƼĴ�����ֵ
����ֵ  ������������
*******************************************/
void Write_EEPROM(unsigned char Th,unsigned char Tl,unsigned char Register_Con )
{
  
    DS18B20_Init();
    Write_OneByte_ToDS18b20(ROM_Skip_Cmd);
    Write_OneByte_ToDS18b20(Write_Scratchpad);
    
    Write_OneByte_ToDS18b20(Th);//Th=7F
    Write_OneByte_ToDS18b20(Tl);//Tl=FF ���λ����λ
    Write_OneByte_ToDS18b20(Register_Con);//12λģʽ
    
    Delay_Ms(700);
    DS18B20_Init();
    Write_OneByte_ToDS18b20(ROM_Skip_Cmd);
    Write_OneByte_ToDS18b20(Copy_Scratchpad);//���Ĵ���������ֵд��EEPROM
    
    Delay_Ms(300);
 
}
/******************************************
�������ƣ�DS18B20_Init
��    �ܣ���ʼ��DS18b20
��    ������
����ֵ  ����
*******************************************/
void DS18B20_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO,ENABLE);
    Tx_ResetPulse();
    Rx_PresencePulse(); 
}


#include <finsh.h>
int fws_testmp(void)
{
	unsigned char sign ;
    unsigned char interger ;
    unsigned int  decimal;

	DS18B20_Init();

	Read_Temperature(&sign,&interger,&decimal);
 
	rt_kprintf("�¶� = %c%d.%d",sign?'+':'-',interger,decimal);

	return 0;
}
FINSH_FUNCTION_EXPORT(fws_testmp,test tmp)
