#include "Singlebuf.h"
#define	SBUF_NULL 			(void *)0




int Sbuf_init(SBUF *sbf,unsigned char *pbuf,unsigned short maxlen)
{
	if( ( sbf == SBUF_NULL ) || ( pbuf == SBUF_NULL ) || ( maxlen <= 2 ) )
		return SBUF_PARAM_ERR;

	sbf->tail = sbf->avail = sbf->max_used = 0;
	sbf->head = 0;
#if	USED_OVERFLOW
	sbf->overflow = BUF_NORMAL;
#endif
	sbf->maxlen = maxlen;
	sbf->pbuf = pbuf;

	return SBUF_OK;
}

int Sbuf_GetPushNum(SBUF *sbf)
{
	return sbf->avail;
}

int Sbuf_write(SBUF *sbf,unsigned char *buf,unsigned short n)
{
	unsigned short i,len=0;

	if( ( sbf == SBUF_NULL ) || ( buf == SBUF_NULL ) || (n == 0) )
		return SBUF_PARAM_ERR;	
	
	for( i=0;i<n;i++ )
	{
		if(  sbf->avail < sbf->maxlen )
		{
		  	sbf->pbuf[sbf->head] = buf[i];
			sbf->head = (sbf->head + 1)%sbf->maxlen;
			sbf->avail++;
			if( sbf->avail > sbf->max_used )
				sbf->max_used = sbf->avail;	
			len++;	
		}
#if	USED_OVERFLOW
		else
		{
			sbf->overflow = BUF_OVERFLOW;
		}
#endif

	}

	return len;
}

int Sbuf_isBufOverflow(SBUF *sbf)
{	
	return sbf->overflow;
}

int Sbuf_read(SBUF *sbf,unsigned char *buf,unsigned short n)
{
	unsigned short i,len=0;

	if( ( sbf == SBUF_NULL ) || ( buf == SBUF_NULL ) || (n == 0) )
		return SBUF_PARAM_ERR;	

	for( i=0;i<n;i++ )
	{
		if(  sbf->avail )
		{		
			buf[i] = sbf->pbuf[sbf->tail];
			sbf->tail = (sbf->tail + 1)%sbf->maxlen;
			sbf->avail--;
			len++;	
		}
	}
	return len;	
}



