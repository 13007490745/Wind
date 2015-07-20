#include "stm32f10x.h"
#include <rtthread.h>
#include <string.h>
#include "gsmlib.h"
#include "uart5_driver.h"


void FwsGSMdectProcess_thread_entry(void* parameter)
{
	static MSG_DATA md;
	UART5_init(9600);

	rt_thread_delay(200);
	GSM_ATisOK();
	rt_thread_delay(50);
	GSM_ATisOK();
	rt_thread_delay(50);
	if( GSM_ATisOK() == true )
	 	rt_kprintf("[MSG]AT Command OK.\n");
	else
		rt_kprintf("[MSG]AT Command ERR.\n");
	rt_thread_delay(20);
	if( DeleteAllMsg() == true )
	 	rt_kprintf("[MSG]Delete all message OK.\n");
	else
		rt_kprintf("[MSG]Delete all message ERR.\n");
	while(1)
	{
		while(1)
		{
			if( GSM_ATisOK() == true )
				break;
			rt_thread_delay(15);
		}	
		// 获取短信.
		if( GetLastMsg(&md) )
		{  
			// 如果是从飞信发过来的，将数据发到我的手机上。
			if(strcmp("12520404643070",md.id) == 0)
				strcpy(md.id,"15874237466");
			rt_thread_delay(50);
			//rt_kprintf("\n[MSG]TEL=%s\n",md.id);
			//rt_kprintf("[MSG]TXT=%s\n",md.text);			
			// 处理消息.
			HandleMsg(&md); 
			rt_thread_delay(50);
			// 删除所有消息.
			DeleteAllMsg();
			rt_thread_delay(50);
		}	
			
		rt_thread_delay(20);	
	
	}	

}

int FwsGSMProcessInit(void)
{
	rt_thread_t init_thread;

	init_thread = rt_thread_create( "GSM",
									FwsGSMdectProcess_thread_entry,
									RT_NULL,
									4096,22,10);

	if( init_thread != RT_NULL )
	{
		rt_thread_startup(init_thread);
	}

	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void fws_msgstart(void)
{
 	FwsGSMProcessInit();	
}
FINSH_FUNCTION_EXPORT(fws_msgstart,Starup GSM process.)

#endif
