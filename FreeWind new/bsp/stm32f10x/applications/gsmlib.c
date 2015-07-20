#include <rtthread.h>
#include <stm32f10x.h>
#include "gsmlib.h"
#include "fws_ad.h"
#include "ctype.h"  
#include "stdio.h" 
#include "stdlib.h" 
#include "string.h" 
#include "PwrServer.h"
#include "uart5_driver.h"
#include "fws_temperature.h"
#include "fwsCOdectProcess.h"
#include "fwsCH2OdectProcess.h"
#include "fwsRelayCtlProcess.h"

//#define GDB(x) rt_kprintf x
#define GDB(x)

// �����ַ���ǰ��Ŀո�.
#define SKIP_SPACE(ptr)		while(*ptr==' '||*ptr=='\t') ++ptr;
// �����ַ���ǰ��ָ�����ַ���
#define SKIP_CHAR(ptr, c)	while(*ptr==c) ++ptr;





char RBuf0_Send[4]={0xAA,0x00,0x00,0xCC};  //֮ǰ�İ��ӵ�Ƭ��������ͨѶ�ķ��ͻ���   
           
// �·�ϵͳӦ��ĳ�ʱֵ, ��ֵΪִ��DelaySec�Ĵ�����
#define XFSYS_RESP_TIMEOUT		80	
#define MAX_BUFFER_SIZE	    	800
char gsm_rxbuf[MAX_BUFFER_SIZE + 1]; 	//GSM���ջ�����
char gsm_txbuf[MAX_BUFFER_SIZE + 1]; 	//GSM���ͻ�����
char resp_buf [MAX_BUFFER_SIZE + 1];	
uint32_t  	gsm_rxbuflen;  				//GSM����ģ��Ļ�����ָ��

void GSM_UART_RX(uint8_t c)  
{		
	if( gsm_rxbuflen == MAX_BUFFER_SIZE - 1 )
	{
		gsm_rxbuflen = 0;
	}
	GDB(("%c",c));
	gsm_rxbuf[gsm_rxbuflen++] = c;
	gsm_rxbuf[gsm_rxbuflen] = '\0';
}

int UART_SendString(char *s)
{
	int32_t len,ret = -1;

	GDB( ("%s",s); )

	len = strlen(s);
	if( len > 0)
	{
		len = Cycbuf_WriteFrame(&U5SendBuf,(uint8_t *)s,len);
		if( len > 0 )
			ret = len;
	}
 	return ret;
}

int UART_SendChar(char c)
{
	GDB( ("%c",c); )
	return Cycbuf_WriteFrame(&U5SendBuf,(uint8_t *)&c,1);
}
/////////////////////////////////////////////////////////////////////
// �������л�ȡ�ַ���ֵ
// ��"REC READ", ����REC READ.
// 40, ����40
// Test\r\n����Test.
char * GetNextValue(char ** pBuf)
{
	char * ptr = *pBuf;
	char * pv = NULL;
	SKIP_SPACE(ptr); 

	if( *ptr == '\"' )
	{
		pv = ++ptr;
		while( (*ptr != '\0') && (*ptr != '\"') ) ++ptr;

		if( *ptr == '\"' )
		{
			*ptr = '\0';
			ptr += 1;
			SKIP_SPACE(ptr); 
			if( *ptr == ',' )
			{
				ptr += 1;
			}
			else if( *ptr == '\r' )
			{
				ptr += 1;
				if( *ptr == '\n' )
					ptr += 1;
			}
		}
	}
	else
	{
		pv = ptr;
//		while( (*ptr != '\0') && (*ptr != ',') && (*ptr != '\r') && (*ptr != '\n') )  ++ptr;
//
//		if( *ptr == ',' || *ptr == '\n' )
//		{
//			*ptr = '\0';
//			ptr += 1;
//		}
		//ɾ���� (*ptr != ',') Ϊ�˲���������
		while( (*ptr != '\0') && (*ptr != '\r') && (*ptr != '\n') )  ++ptr;

		if(  *ptr == '\n' )
		{
			*ptr = '\0';
			ptr += 1;
		}
		else if( *ptr == '\r' )
		{
			*ptr = '\0';
			ptr += 1;
			if( *ptr == '\n' )
				ptr += 1;
		}
	}

	*pBuf = ptr;
	return pv;
}


// Wait command prompt.
uint8_t WaitPrompt()
{    
	uint8_t tm = 0; 
	do
	{
		rt_thread_delay(100);//��ʱ20��

        if( gsm_rxbuflen > 0 )
        {
    		if( strchr(gsm_rxbuf, '>') != NULL )
    		{
    			return true;
    		}
    		else
    		{
				rt_kprintf("[MSG] Don`t receive '>' in ATSendCmd.\r\n");
                return false;
    		}
        }
	}
	while( ++tm < XFSYS_RESP_TIMEOUT );
	
	return false;  //Timeout.
}


/////////////////////////////////////////////////////////////////////
// ��GPSģ�鷢������ȴ����ؽ����
// ���͵�������gsm_txbuf��
// ���صĽ��������.gsm_rxbuf
// p1, endChar��ʾparameters.
char * SendAtCmd(char * p1, char endChar, uint8_t bHasResp ,char *txbuf)
{
	uint8_t tm;
	char * pcmd;

	// ��ʼ�������ַ�������'AT+'
	/*if( gsm_txbuf[0] != 'A' && gsm_txbuf[1] != 'T' && gsm_txbuf[2] != '+' )
	{
		return NULL; //���ش���.
	} */

    //Uart1Sends("AT+CMGF=1\r\n");
    //DelaySec(1);

	memset(gsm_rxbuf, sizeof(gsm_rxbuf), 0);
	gsm_rxbuflen = 0; 
	
	//Uart1Sends(gsm_txbuf); 
	UART_SendString(txbuf);
	
	// ��ȡ����ؼ��֡�

	if( NULL == (pcmd = strchr(txbuf, '?')) )
	{
		if( NULL == (pcmd = strchr(txbuf, '=')) )
		{
			if( NULL == (pcmd = strchr(txbuf, '\r')) )
				return NULL; //��ô�����ء�
		}
	}
	pcmd[0] = ':';	 
	pcmd[1] = '\0';
	pcmd = txbuf + 2;  //pcmd��Ϊ"+XXXX:"����ʽ.

    if( p1 != NULL )
    {
        if( !WaitPrompt() )
            return NULL;
        gsm_rxbuflen = 0; 
        UART_SendString(p1);
    }
       
    if( endChar != '\0' )
    {
        if( !WaitPrompt() )
            return NULL;
        gsm_rxbuflen = 0; 
        UART_SendChar(endChar);
    }

	tm = 0; 
	do
	{
		char * pb;
		char * pe;

		rt_thread_delay(80);//��ʱ20��

		pe = strstr(gsm_rxbuf, "\nOK");

		// �����Ƿ����OK.
		if( pe != NULL )
		{
            if( bHasResp )
		    	pb = strstr(gsm_rxbuf, pcmd);
            else 
                pb = gsm_rxbuf;
			if( NULL == pb )
			{
				rt_kprintf("[MSG] Can`t find 'OK'.\r\n");
				return NULL; // ��������.
			}
			pb += strlen(pcmd);

			*pe = '\0';
			return pb;
		}
		else
		{
			pe = strstr(gsm_rxbuf, "\nERROR");
			if( pe != NULL )
				return NULL; //error.
		}
	}
	while( ++tm < XFSYS_RESP_TIMEOUT );
	
	return NULL;  //Timeout.
}

/////////////////////////////////////////////////////////////////////
// ��GPSģ�鷢�ͷ��ض��ţ����͵�������resp_buf��.
// idΪҪ���͵ĺ��롣
uint8_t SendResponse(char * id)
{
	//strcpy(gsm_txbuf, "AT+CSCS=\"GSM\"\r\n");
	//if( NULL == SendAtCmd() )
	//	return false;
    static char respbuf[40];                   
    //static const char end[2] = {0x1a, '\0'};
	sprintf(respbuf, "AT+CMGS=\"%s\"\r\n", id);

	if( NULL == SendAtCmd(resp_buf,0x1a,false,respbuf) )
    {                              
        //TraceChars("send failed : ");  
        //TraceChars(resp_buf);
        //TraceChars("\r\n");
		return false;
    }

	return true;
}


/////////////////////////////////////////////////////////////////////
// ��GSM�л�ȡ��������.
int GetMsgNum()
{
	char * ptr;
	char * pv;
	static char respbuf[40]; 

	strcpy(respbuf, "AT+CPMS?\r\n");	//��ѯ��������
	if( NULL == (ptr=SendAtCmd(NULL, '\0', true,respbuf)) )
	{
		return 0;
	}

	pv = GetNextValue(&ptr);  
	if( pv[0]!='S'|| pv[1]!='M'|| pv[2]!='\0')//   strcmp(pValue, "SM") != 0 )
		return -1; // ��������.

	pv = GetNextValue(&ptr);  
	return atoi(pv);
}



/////////////////////////////////////////////////////////////////////
// ��GSM�л�ȡ���һ����������.
uint8_t GetLastMsg(MSG_DATA * pmd)
{
	char * ptr;
	char * pValue;
	char msgbuf[160];
    int num;

	// ��GSM�л�ȡ��������
	num = GetMsgNum();
	if( num <= 0 )
		return false;

	sprintf(msgbuf, "%d", num); //caai Ӧ���Ƕ���ġ�

	// ��ȡ���һ�����ŵ����ݡ�
	sprintf(msgbuf,"AT+CMGR=%d\r\n",num);
	if( NULL == (ptr=SendAtCmd(NULL,'\0', true,msgbuf)) )
		return false;


	// ��ȡ���ű��, "REC UNREAD", "REC READ"
	pValue = GetNextValue(&ptr);
	//if( strcmp(pValue, "REC UNREAD") != 0 )
	//	return false;

	// ������ӵ绰���롣
	pValue = GetNextValue(&ptr);
	if( *pValue == '\0' )
		return false;
                

	pmd->id = pValue; //�绰���롣

    while( NULL != (pValue = GetNextValue(&ptr)) )
    {
        if( strchr( pValue, ':' ) != NULL )
        {
            //time found.
            break;
        }
    }
	pValue = GetNextValue(&ptr); //���ݡ�

	if( *pValue == '\0' )
		return false;

	pmd->text = pValue; //���ݡ�
	return true;
}

// ɾ��GSM�е�������Ϣ.
uint8_t DeleteAllMsg(void)
{    
	       
	strcpy(gsm_txbuf, "AT+CMGDA=\"DEL ALL\"\r\n");	//��ѯ��������
	if( NULL == SendAtCmd(NULL, '\0', false,gsm_txbuf) )
    {
		return false;
    }
	return true;
}


uint8_t GSM_ATisOK(void)
{    
	static char sbuf[10];      
	strcpy(sbuf,"AT\r\n");	//��ѯ��������
	if( NULL == SendAtCmd(NULL, '\0', false,sbuf) )
    {
		return false;
    }
	return true;
}
/////////////////////////////////////////////////////////////////////
// ��������·��.
uint8_t cmdQuery(char * id, int seq)
{
/*
	sprintf(resp_buf, "XF%d:T1=%d,T2=%d,JQ1=%x%x.%x%x,CO=%d,WD=%d\r\n", seq, 
        temp1, temp2, (parameter1 >> 24) & 0xf,(parameter1 >> 16) & 0xf,
        (parameter1 >> 8) & 0xf,(parameter1 >> 0) & 0xf, parameter2, 
        WD_NUM);
*/
	static unsigned char tmp0[10],tmp1[10],val[10],wd_state;
	static unsigned short ValCO;
	static float cal;
	char * p =   resp_buf;
	int len;
	//int len = sprintf(resp_buf, "XF%d:T1=%d,T2=%d,JQ=%d.%dppm,CO=%d,WD=%d\r\n", seq, 
     //   temp1, temp2, (jq_value>>8), jq_value&0xff, co_value,wd_state);
    fws_DS18B20_TmpGet(DS0,tmp0);
	fws_DS18B20_TmpGet(DS1,tmp1);
	ADC_GetVol(val);
	ValCO = FwsGetCOPPM();
	cal = FwsGetCH2OPPM()*0.01; 
	wd_state = fwsRlyWindStateGet();


    len = sprintf(p, "XF%d:", seq);    if(len<=0 ) return false;
    p += len;	
	if( tmp0[3] == 1 )	
    	len = sprintf(p, "T1=-%d%d.%d,",tmp0[0],tmp0[1],tmp0[2] );    
	else
    	len = sprintf(p, "T1=%d%d.%d," ,tmp0[0],tmp0[1],tmp0[2] );    
	if(len<=0 ) return false;
	p += len; 
	 
	if( tmp1[3] == 1 )	
    	len = sprintf(p, "T2=-%d%d.%d,",tmp1[0],tmp1[1],tmp1[2] );    
	else
    	len = sprintf(p, "T2=%d%d.%d," ,tmp1[0],tmp1[1],tmp1[2] );    
	if(len<=0 ) return false;
	p += len;
	  
    len = sprintf(p, "JQ=%.2fppm,", cal);    if(len<=0 ) return false;
    p += len; 
    len = sprintf(p, "CO=%dppm,", ValCO);    if(len<=0 ) return false;
    p += len; 
    len = sprintf(p, "WD=%d,", wd_state);    if(len<=0 ) return false;
    p += len;     
    len = sprintf(p, "VOL=%d%d.%d\r\n",val[0],val[1],val[2]);    if(len<=0 ) return false;
    p += len;     

	p[0] = '\0';

//	rt_kprintf("%s",resp_buf);
//	return 0;
	return SendResponse(id);
	//15197282170
}

/////////////////////////////////////////////////////////////////////
// ��������·��.
uint8_t cmdWind(char * id, int seq, int minutes)
{
	if( FwsPwrDect_isPowerLowAlarm() == PWR_ON )
	{	
		fwsRlyWindOpen(RLY_WIND_PHONE_APP,minutes*MIN_DIV);
		sprintf(resp_buf, "XF%d:OK\r\n", seq);
	}
	else
	{
  		sprintf(resp_buf, "XF%d:FAIL,<Voltage low>\r\n", seq);
	}
	rt_kprintf("[MSG]%s\r\n", resp_buf);
	return SendResponse(id);
}					 

/////////////////////////////////////////////////////////////////////
// �����������.
uint8_t HandleMsg(MSG_DATA * pmd)
{
	int  seq;
	char * pb;

	if( pmd->text[0] != 'X' || pmd->text[1] != 'F' )
		return false;

	pb = strchr(pmd->text, ':');
	if( NULL == pb )
		return false;

	*pb = '\0';
	pb += 1;
	SKIP_SPACE(pb);
           
	// ��Ϣ�����к�.
	seq = atoi(pmd->text + 2);

	pmd->text = pb;
     
	// ����·��.
	//if( pb[0] == 'Q' && pb[1] == '\0' )  //## mody by caai
	if( pb[0] == 'Q' )
	{
		// Query command.
		return cmdQuery(pmd->id, seq);
	}

//	if( pb[0] == 'C' && pb[1] == '1' )
//	{
//		int minutes;
//
//		// һ����硣
//		//XF9:C1,<minutes>
//
//		pb += 2;  
//		SKIP_SPACE(pb);
//		if( *pb == ',' ) 
//		{
//			pb+=2;
//			minutes = atoi(pb);
//		
//			if( minutes == 0 )
//				minutes = 5;
//			else if( minutes > 180 )
//				minutes = 180;
//		}
//		else
//		{
//			minutes = 5;
//		}
//
//
//		return cmdWind(pmd->id, seq, minutes);
//	}
	if( pb[0] == 'C' )
	{
		int minutes;

		pb += 1;
		minutes = atoi(pb);
		if( minutes > 20 )
			minutes = 20;
		return cmdWind(pmd->id, seq, minutes);
	}


	// Unknown command.
	return false;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void gsm(uint32_t op)
{
	int32_t len,init = 0;
	static char buf[40]; 
	static MSG_DATA msg;
	
	if(init == 0 )
	{
	 	init = 1;
		UART5_init(9600);
		fws_DS18B20_Init(1);
		fws_DS18B20_Init(2);
		ADC_PwrInit();
//		FwsCOdectProcessInit();
//		FwsCH2OdectProcessInit();
	}

	{
		strcpy(buf, "AT\r\n");	//��ѯ��������
		if( NULL == UART_SendString(buf) )
		{
			rt_kprintf("AT Send fail.\n");
			return ;
		}
		rt_thread_delay(100);
		/* Get return data in rx buffer */
		len = Sbuf_read(&U5RevBuf,(uint8_t*)buf,40);

		strcpy(buf, "AT\r\n");	//��ѯ��������
		if( NULL == UART_SendString(buf) )
		{
			rt_kprintf("AT Send fail.\n");
			return ;
		}
		rt_thread_delay(100);

		/* Get return data in rx buffer */
		len = Sbuf_read(&U5RevBuf,(uint8_t*)buf,40);
		
		if( GSM_ATisOK() == false )
			rt_kprintf("AT Command function error.\n");	
		else
			rt_kprintf("AT Command function OKOK.\n");	
		
	}

	if( op == 1 )
	{
		strcpy(buf, "AT+CMGF=1\r\n");	//�����ı�ģʽ
		if( NULL == UART_SendString(buf) )
		{
			rt_kprintf("AT Send fail.\n");
			return ;
		}
		rt_thread_delay(20);
		/* Get return data in rx buffer */
		len = Sbuf_read(&U5RevBuf,(uint8_t*)buf,40);	 	
	}

	if( op == 2 )
	{								
		len = GetMsgNum();
		rt_kprintf("GetMsgNum = %d\n",len);
	}

	if( op == 3 )
	{
		GetLastMsg(&msg);
		rt_kprintf("telephone number = %s\n",msg.id);
		rt_kprintf("text = %s\n",msg.text);	

	}
	if( op == 4 )
	{
		int slen = sprintf(buf, "AT+CMGDA=\"DEL ALL\"\r\n");
		buf[slen] = '\0';
		if( NULL == UART_SendString(buf) )
		{
			rt_kprintf("AT Send fail.\n");
			return ;
		}
		rt_thread_delay(20);
		/* Get return data in rx buffer */
		len = Sbuf_read(&U5RevBuf,(uint8_t*)buf,40);	 	
	}
	if( op == 5 )
	{
		if( cmdQuery("15874237466",18) == false )
			rt_kprintf("fail.\n");
		rt_kprintf("Send message successful.\n");		
	}
	if( op == 6 )
	{
		if( cmdQuery("18673217777",18) == false )
			rt_kprintf("fail.\n");
		rt_kprintf("Send message successful.\n");		
	}
	if( op == 7 )
	{
		if( cmdWind("15874237466",18,0) == false )
			rt_kprintf("fail.\n");
		rt_kprintf("Send message successful.\n");		
	}
	
}
FINSH_FUNCTION_EXPORT(gsm,this is a strlen test)


void msgdel(uint32_t index)
{
	int32_t init = 0;
	static char buf[40]; 
	
	if(init == 0 )
	{
	 	init = 1;
		UART5_init(9600);
	}

	{
		strcpy(buf, "AT\r\n");	//��ѯ��������
		if( NULL == UART_SendString(buf) )
		{
			rt_kprintf("AT Send fail.\n");
			return ;
		}
		rt_thread_delay(100);
		/* Get return data in rx buffer */
		Sbuf_read(&U5RevBuf,(uint8_t*)buf,40);			
	}

	{
		strcpy(buf, "AT\r\n");	//��ѯ��������
		if( NULL == UART_SendString(buf) )
		{
			rt_kprintf("AT Send fail.\n");
			return ;
		}
		rt_thread_delay(100);

		strcpy(buf, "AT\r\n");	//��ѯ��������
		if( NULL == UART_SendString(buf) )
		{
			rt_kprintf("AT Send fail.\n");
			return ;
		}
		rt_thread_delay(10);
		/* Get return data in rx buffer */
		Sbuf_read(&U5RevBuf,(uint8_t*)buf,40);			
	}

	{
		strcpy(buf, "AT+CMGF=1\r\n");	//�����ı�ģʽ
		if( NULL == UART_SendString(buf) )
		{
			rt_kprintf("AT Send fail.\n");
			return ;
		}
		rt_thread_delay(20);
		/* Get return data in rx buffer */
		Sbuf_read(&U5RevBuf,(uint8_t*)buf,40);	 	
	}


	{
		int slen = sprintf(buf, "AT+CMGD=%d\r\n",index);	
		buf[slen] = '\0';
		if( NULL == UART_SendString(buf) )
		{
			rt_kprintf("AT Send fail.\n");
			return ;
		}
		rt_thread_delay(20);
		/* Get return data in rx buffer */
		Sbuf_read(&U5RevBuf,(uint8_t*)buf,40);	 	
	}

}
FINSH_FUNCTION_EXPORT(msgdel,delete one message)

void msgalldel(uint32_t index)
{
	int32_t init = 0;
	static char buf[40]; 

	
	if(init == 0 )
	{
	 	init = 1;
		UART5_init(9600);
	}
	{
		int slen = sprintf(buf, "AT+CMGDA=\"DEL ALL\"\r\n");
		buf[slen] = '\0';
		if( NULL == UART_SendString(buf) )
		{
			rt_kprintf("AT Send fail.\n");
			return ;
		}
		rt_thread_delay(20);
		/* Get return data in rx buffer */
		Sbuf_read(&U5RevBuf,(uint8_t*)buf,40);	 	
	}

}
FINSH_FUNCTION_EXPORT(msgalldel,delete all message)
#endif

