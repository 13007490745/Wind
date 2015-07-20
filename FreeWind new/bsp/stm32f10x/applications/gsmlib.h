#ifndef __GSMLIB_H__
#define __GSMLIB_H__

// 短消息内容.
typedef struct _MSG_DATA
{
	char * id; //短信号码。
	char * text; //短信内容.
} MSG_DATA;

#define true		1          
#define false		0

extern uint8_t GSM_ATisOK(void);
extern uint8_t DeleteAllMsg(void);
extern uint8_t GetLastMsg(MSG_DATA * pmd);
extern uint8_t HandleMsg(MSG_DATA * pmd);






#endif
