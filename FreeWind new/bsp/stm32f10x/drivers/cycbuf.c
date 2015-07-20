#include "stm32f10x.h"
#include "cycbuf.h"
#include "string.h"

//---------------------------------------------------------------------------------------
int32_t Cycbuf_Init(CBUF* handle,uint8_t* buf,uint16_t MaxFrmLen,uint16_t MaxFrmNum)
{
#if	CY_CHECK_PARAMETER
	if( ( buf == NULL ) ||( handle == NULL ) ) return CY_PARAMERR;
	if( ( MaxFrmLen == 0 ) ||( MaxFrmNum == 0 ) ) 	  return CY_PARAMERR; 
#endif	
	memset(buf,0,(MaxFrmLen+FRAME_HEAD)*MaxFrmNum);

	handle->buf = buf;
	handle->MaxFrmLen = MaxFrmLen;
	handle->MaxFrmNum = MaxFrmNum;
	handle->Head = 0;
	handle->Tail = 0;
#if	USED_MAX_FRM_CNT
		handle->MaxUsed = 0;	
#endif
	return CY_OK;
}


//---------------------------------------------------------------------------------------
int32_t Cycbuf_Clear(CBUF* handle)
{
#if	CY_CHECK_PARAMETER
	if( handle == NULL ) return CY_PARAMERR;
#endif	
	memset(handle->buf,0,handle->MaxFrmLen*handle->MaxFrmNum);
	handle->Head = 0;
	handle->Tail = 0;

	return CY_OK;
}


//---------------------------------------------------------------------------------------
int32_t	Cycbuf_WriteFrame(CBUF* handle,const uint8_t* buf,uint16_t len)
{
	uint8_t* bufhead;
#if	CY_CHECK_PARAMETER
	if( ( buf == NULL ) ||( handle == NULL ) ) return CY_PARAMERR;
	if(( len == 0 ) || ( len > handle->MaxFrmLen ))	  return CY_PARAMERR; 
#endif
	if( handle->Active >=  handle->MaxFrmNum )	
		return CY_BUFFULL;

	bufhead = handle->buf + (handle->MaxFrmLen+FRAME_HEAD)*handle->Head;

	memcpy(bufhead+FRAME_HEAD,buf,len);

	*((uint16_t*)bufhead) = len;

	handle->Active++;

#if	USED_MAX_FRM_CNT
	if( handle->Active > handle->MaxUsed )
		handle->MaxUsed = handle->Active;	
#endif

	handle->Head++;
	if( handle->Head == handle->MaxFrmNum )
		handle->Head = 0;		

	return len;
}


//---------------------------------------------------------------------------------------
int32_t Cycbuf_GetAvailFrmHeadp(CBUF* handle,uint8_t** Framehead)
{

#if	CY_CHECK_PARAMETER
	if( ( Framehead == NULL ) ||( handle == NULL ) ) return CY_PARAMERR;
#endif
	if( handle->Active >=  handle->MaxFrmNum )	
		return CY_BUFFULL;

	*Framehead = handle->buf + (handle->MaxFrmLen+FRAME_HEAD)*handle->Head + FRAME_HEAD;

	return CY_OK;
}


//---------------------------------------------------------------------------------------
int32_t Cycbuf_NextHeadFrame(CBUF* handle,uint16_t len)
{
	uint8_t* bufhead;
#if	CY_CHECK_PARAMETER
	if(  handle == NULL  ) return CY_PARAMERR;
	if(( len == 0 ) || ( len > handle->MaxFrmLen ))	  return CY_PARAMERR; 
#endif
	if( handle->Active >=  handle->MaxFrmNum )	
		return CY_BUFFULL;

	bufhead = handle->buf + (handle->MaxFrmLen+FRAME_HEAD)*handle->Head;

	*((uint16_t*)bufhead) = len;

	handle->Active++;

#if	USED_MAX_FRM_CNT
	if( handle->Active > handle->MaxUsed )
		handle->MaxUsed = handle->Active;	
#endif

	handle->Head++;
	if( handle->Head == handle->MaxFrmNum )
		handle->Head = 0;		

	return len;
}


//---------------------------------------------------------------------------------------
int32_t Cycbuf_ReadFrame(CBUF* handle,uint8_t* buf,uint16_t Maxlen)
{
	uint8_t* bufhead;
	uint16_t frmlen;
#if	CY_CHECK_PARAMETER
	if( ( buf == NULL ) ||( handle == NULL ) ) return CY_PARAMERR;
	if(( Maxlen == 0 ) || ( Maxlen > handle->MaxFrmLen ))	  return CY_PARAMERR; 
#endif
	if( handle->Active ==  0 )	
		return CY_BUFEMPTY;
	
	bufhead = handle->buf + (handle->MaxFrmLen+FRAME_HEAD)*handle->Tail;
	frmlen  = *((uint16_t*)bufhead);
	memcpy(buf,bufhead+FRAME_HEAD,frmlen);

#if CY_CLEAR_DATA_AFTER_READ
	memset(bufhead,0,frmlen+FRAME_HEAD);	
#endif

	handle->Active--;
	handle->Tail++;
	if( handle->Tail == handle->MaxFrmNum )
		handle->Tail = 0;		
	return frmlen;

}


//---------------------------------------------------------------------------------------
int32_t Cycbuf_GetAvailFrmTailp(CBUF* handle,uint8_t** Framehead)
{
	uint8_t* bufhead;
	uint16_t frmlen;
#if	CY_CHECK_PARAMETER
	if( ( Framehead == NULL ) ||( handle == NULL ) ) return CY_PARAMERR;
#endif
	if( handle->Active ==  0 )	
		return CY_BUFEMPTY;
	
	bufhead = handle->buf + (handle->MaxFrmLen+FRAME_HEAD)*handle->Tail;
	frmlen  = *((uint16_t*)bufhead);	
	*Framehead = bufhead + 2;

	return frmlen;
}


//---------------------------------------------------------------------------------------
int32_t Cycbuf_NextTailFrame(CBUF* handle)
{
#if	CY_CHECK_PARAMETER
	if(  handle == NULL  ) return CY_PARAMERR;
#endif
	if( handle->Active ==  0 )	
		return CY_BUFEMPTY;

#if CY_CLEAR_DATA_AFTER_READ	
	memset(	handle->buf + (handle->MaxFrmLen+FRAME_HEAD)*handle->Tail,
			0,
			handle->MaxFrmLen+FRAME_HEAD);	
#endif

	handle->Active--;
	handle->Tail++;
	if( handle->Tail == handle->MaxFrmNum )
		handle->Tail = 0;		
	return CY_OK;

}



//---------------------------------------------------------------------------------------
int32_t Cycbuf_isEmpty(CBUF* handle)
{
#if	CY_CHECK_PARAMETER
	if(  handle == NULL  ) return CY_PARAMERR;
#endif
	if( handle->Active ==  0 )
		return CY_EMPTY;
	else
		return CY_NOT_EMPTY;
}


//---------------------------------------------------------------------------------------
int32_t Cycbuf_isFull(CBUF* handle)
{
#if	CY_CHECK_PARAMETER
	if(  handle == NULL  ) return CY_PARAMERR;
#endif
	if( handle->Active >=  handle->MaxFrmNum )
		return CY_FULL;
	else
		return CY_NOT_FULL;
}

//---------------------------------------------------------------------------------------
int32_t Cycbuf_GetAvailNum(CBUF* handle)
{
#if	CY_CHECK_PARAMETER
	if(  handle == NULL  ) return CY_PARAMERR;
#endif
	if( handle->MaxFrmNum >= ( handle->Active + 1 ) )
		return ( handle->MaxFrmNum - ( handle->Active + 1 ) ) ;
	else
		return 0;
}


