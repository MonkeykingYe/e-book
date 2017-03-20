#include <encoding_manager.h>
#include <config.h>
#include <string.h>
static int UTF16beCoding(unsigned char *pucBufHead);
static int UTF16beGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);


static T_EncodingOpr g_tUTF16beEncodingOpr = {
	.name			=	"utf-16be",
	.iHeadLen		=	2,
	.isSupport		=	UTF16beCoding,
	.GetCodeFrmBuf	=	UTF16beGetCodeFrmBuf,
};

static int UTF16beCoding(unsigned char *pucBufHead)
{
	const char aStrUtf16be[] = { 0xFE, 0xFF, 0  }; 
	if(strncmp((const char *)pucBufHead, aStrUtf16be, 2) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static int UTF16beGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	if(pucBufStart +1 < pucBufEnd)
	{
		*pdwCode = ((unsigned int )pucBufStart[0] << 8) +pucBufStart[1];
		return 2;
	}
	else
	{
		return 0;
	}

}

int Utf16beEncodingInit(void)
{
	AddFontOprForEncoding(&g_tUTF16beEncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&g_tUTF16beEncodingOpr, GetFontOpr("ascii"));	
	return RegisterEncodingOpr(&g_tUTF16beEncodingOpr);
}
