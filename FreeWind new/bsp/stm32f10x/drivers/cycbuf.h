
#include <stm32f10x.h>

#ifndef	__CYCBUF_H__
#define	__CYCBUF_H__

#define	FRAME_HEAD		2

#define	CY_CLEAR_DATA_AFTER_READ	1
#define	CY_CHECK_PARAMETER			1

#define	CY_EMPTY					0
#define	CY_NOT_EMPTY  				1

#define	CY_FULL						0
#define	CY_NOT_FULL					1

#define	CY_OK	  					0
#define	CY_FAIL						-1
#define	CY_BUFFULL					-2
#define	CY_BUFEMPTY					-3
#define	CY_PARAMERR					-4

/* 使用最大缓冲区使用统计 */
#define	USED_MAX_FRM_CNT  			1

typedef struct CYCLEBUF
{
	unsigned short MaxFrmLen;
	unsigned short MaxFrmNum;
	unsigned short Head;
	unsigned short Tail;
	unsigned short Active;	 
	unsigned short MaxUsed;
	uint8_t* buf;
}	CBUF;
//-----------------------------------------------------------------------------
 int32_t Cycbuf_Init(CBUF* handle,uint8_t* buf,uint16_t MaxFrmLen,uint16_t MaxFrmNum);

 int32_t Cycbuf_Clear(CBUF* handle);

//-----------------------------------------------------------------------------

 int32_t	Cycbuf_WriteFrame(CBUF* handle,const uint8_t* buf,uint16_t len);
 int32_t Cycbuf_GetAvailFrmHeadp(CBUF* handle,uint8_t** Framehead);
 int32_t Cycbuf_NextHeadFrame(CBUF* handle,uint16_t len);

//-----------------------------------------------------------------------------
 int32_t Cycbuf_ReadFrame(CBUF* handle,uint8_t* buf,uint16_t Maxlen);
 int32_t Cycbuf_GetAvailFrmTailp(CBUF* handle,uint8_t** Framehead);
 int32_t Cycbuf_NextTailFrame(CBUF* handle);

//-----------------------------------------------------------------------------
 int32_t Cycbuf_isEmpty(CBUF* handle);
 int32_t Cycbuf_isFull(CBUF* handle);

//-----------------------------------------------------------------------------
 int32_t Cycbuf_GetAvailNum(CBUF* handle);









#endif
