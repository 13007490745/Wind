#include <board.h>
#include <rtthread.h>
#include "AudioServer.h"

#include "fws_audio.h"

#define AUD_SEVR_LEN			10
//-------------------------
//定义接收缓冲区,用来接收语音播放命令
uint8_t	AudServerCmd[AUD_SEVR_LEN];
SBUF 	Ascb; //Audio server cmd buf
				



#define WARNING_CH2O_AUD				0
#define WARNING_CO_AUD					1
#define WARNING_DIDI_AUD				4
#define WARNING_DIDIDIDI_AUD			5


void AudioServer_thread_entry(void* parameter)
{
	static uint8_t  Cmd,CH2O_Cmd = 0,CO_Cmd = 0,PlayFlag = 0,CH2O_Warning = 0,CO_Warning = 0;
	static uint32_t cnt = 0;
	AUD_Init();
	rt_thread_delay(100);
	//rt_kprintf("[AUD]Audio Server init.\n");
	while(1)
	{
		/* Get a command in receive buffer */
		
		//AUD_Play(WARNING_DIDIDIDI_AUD);
		//AUD_Play(WARNING_DIDI_AUD);
		//AUD_Play(WARNING_CO_AUD);
		//AUD_Play(WARNING_CH2O_AUD);

		
   		if( Sbuf_read(&Ascb,&Cmd,1) == 1 )
		{
		//	rt_kprintf("[AUD]Cmd =0x%02x.\n",Cmd);
//			/* Store CH2O and CO command flag */
//		 	if(CH2O_MARK & Cmd)
//			{
//			 	CH2O_Cmd = SPEAK_MARK & Cmd;
//				CH2O_Warning = DIDI_WARNING_MARK & Cmd;
//			}
//			else
//			{
//			 	CO_Cmd = SPEAK_MARK & Cmd;
//				CO_Warning = DIDI_WARNING_MARK & Cmd;	
//			}
//			/* Calculate play audio information */	
//			PlayFlag = CH2O_Cmd | CO_Cmd | ( PlayFlag & SPEAK_MARK );
//			CO_Cmd = CH2O_Cmd = 0;

			/* Store CH2O and CO command flag */
		 	if(CH2O_MARK & Cmd)
			{
				CH2O_Warning = DIDI_WARNING_MARK & Cmd;
			}
			else
			{
				CO_Warning = DIDI_WARNING_MARK & Cmd;	
			}
			/* Calculate play audio information */	
			PlayFlag = (SPEAK_MARK & Cmd) | PlayFlag;

		}
		PlayFlag |= CH2O_Warning;
		PlayFlag |= CO_Warning;



		/* human speak warning audio in CH2O channel */
		if( PlayFlag & CH2O_WARNING_SPEAK_MARK )
		{
			/* reduce mark bit */
			PlayFlag &= ~CH2O_WARNING_SPEAK_MARK;

			/* Play frist audio */
			AUD_Play(WARNING_CH2O_AUD);

			/* waitting for end */
			while( !AUD_IsEnd() )
			{
				rt_thread_delay(40); 	
			}

			/* Delay 3 second */
			rt_thread_delay(300);

			/* Play again */
			AUD_Play(WARNING_CH2O_AUD);

			/* Waitting for end */
			while( !AUD_IsEnd() )
			{
				rt_thread_delay(40); 	
			}

			/* Delay shot time */
			rt_thread_delay(50);

			/* Go to next process loop */
			continue;
		}

		/* human speak warning audio in CO channel */
		if( PlayFlag & CO_WARNING_SPEAK_MARK )
		{
			/* reduce mark bit */
			PlayFlag &= ~CO_WARNING_SPEAK_MARK;

			/* Play frist audio */
			AUD_Play(WARNING_CO_AUD);

			/* waitting for end */
			while( !AUD_IsEnd() )
			{
				rt_thread_delay(40); 	
			}

			/* Delay 3 second */
			rt_thread_delay(300);

			/* Play again */
			AUD_Play(WARNING_CO_AUD);

			/* Waitting for end */
			while( !AUD_IsEnd() )
			{
				rt_thread_delay(40); 	
			}

			/* Delay shot time */
			rt_thread_delay(50);

			/* Go to next process loop */
			continue;
		}

		/* 'didi..didi..' audio warnning play  */
		if( PlayFlag & WARNING_DIDIDIDI_MARK )
		{
		 	cnt++;
			/* play audio again for every 10 second */
			if( cnt >= 15 )
			{
				cnt = 0;
				PlayFlag &= ~WARNING_DIDIDIDI_MARK;
				AUD_Play(WARNING_DIDIDIDI_AUD);
				while( !AUD_IsEnd() )
				{
					rt_thread_delay(40); 	
				}
				rt_thread_delay(80);
			}
		}		
		else if( PlayFlag & WARNING_DIDI_MARK )
		{
		 	cnt++;
			/* play audio again for every 3 second */
			if( cnt >= 45 )
			{
				cnt = 0;
				PlayFlag &= ~WARNING_DIDI_MARK;
				AUD_Play(WARNING_DIDI_AUD);
				while( !AUD_IsEnd() )
				{
					rt_thread_delay(40); 	
				}
				rt_thread_delay(80);
			}
		}

		/* This delay time is process tick time. */
		rt_thread_delay(20);
	}
}

int AudioServer_init(void)
{
    rt_thread_t init_thread;

	Sbuf_init(&Ascb,AudServerCmd,AUD_SEVR_LEN);

    init_thread = rt_thread_create("AudSvr",
                                   AudioServer_thread_entry, RT_NULL,
                                   2048, 28, 10);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);		

    return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
int fws_audcmd(uint32_t index)
{
 	static uint8_t init = 0;
	static uint8_t Cmd;
	if( init == 0 )
	{
		init = 1;
		AudioServer_init();
	}
	Cmd = index;
	Sbuf_write(&Ascb,&Cmd,1);
 
	return 0;
}
FINSH_FUNCTION_EXPORT(fws_audcmd,fws_audcmd(index))
#endif

