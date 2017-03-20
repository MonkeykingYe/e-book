#include <encoding_manager.h>
#include <config.h>
#include<string.h>

static int UTF16leCoding(unsigned char *pucBufHead);
static int UTF16leGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);



static T_EncodingOpr g_tUTF16leEncodingOpr	=	{
	.name			=	"utf-16le",
	.iHeadLen		=	2,
	.isSupport		=	UTF16leCoding,
	.GetCodeFrmBuf	=	UTF16leGetCodeFrmBuf,
};

static int UTF16leCoding(unsigned char *pucBufHead)
{
	const char aStrUtf16le[] = {0xFF, 0xFE, 0  };
	if(strncmp((const char *)pucBufHead, aStrUtf16le, 2) == 0)
	{
		return 1;	
	}
	else
	{
		return 0;
	}
}

static int UTF16leGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	if(pucBufStart +1 < pucBufEnd)
	{
		*pdwCode = ((unsigned int )pucBufStart[1] << 8) +pucBufStart[0];
		return 2;
	}
	else
	{
		return 0;
	}
	
}

int  Utf16leEncodingInit(void)
{
	AddFontOprForEncoding(&g_tUTF16leEncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&g_tUTF16leEncodingOpr, GetFontOpr("ascii"));	
	return RegisterEncodingOpr(&g_tUTF16leEncodingOpr);
}

