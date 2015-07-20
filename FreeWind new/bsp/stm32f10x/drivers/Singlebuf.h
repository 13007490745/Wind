#ifndef	__SINGLEBUF_H__
#define	__SINGLEBUF_H__

#define	SBUF_OK				0
#define	SBUF_FAIL			-1
#define	SBUF_PARAM_ERR		-2
#define	SBUF_BUF_FULL		-3
#define	SBUF_BUF_EMPTY		-4

#define USED_OVERFLOW		1
#define BUF_OVERFLOW 		1
#define BUF_NORMAL			0	
typedef	struct SBUF
{
	unsigned short head;
	unsigned short tail;
	unsigned short avail;
	unsigned short maxlen;
	unsigned short max_used;
#if	USED_OVERFLOW
	unsigned short overflow;
#endif
	unsigned char  *pbuf;
} SBUF;

extern int Sbuf_GetPushNum(SBUF *sbf);
extern int Sbuf_init(SBUF *sbf,unsigned char *pbuf,unsigned short maxlen);
extern int Sbuf_write(SBUF *sbf,unsigned char *buf,unsigned short n);
extern int Sbuf_read(SBUF *sbf,unsigned char *buf,unsigned short n);

#endif

