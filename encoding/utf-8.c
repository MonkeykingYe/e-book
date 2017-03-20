#include <encoding_manager.h>
#include <config.h>
#include <string.h>
static int UTF8Coding(unsigned char *pucBufHead);
static int UTF8GetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);


static T_EncodingOpr g_tUTF8EncodingOpr	=	{
	.name			=	"utf-8",
	.iHeadLen		=	3,
	.isSupport		=	UTF8Coding,
	.GetCodeFrmBuf	=	UTF8GetCodeFrmBuf,
};

static int UTF8Coding(unsigned char *pucBufHead)
{
	const char aStrUtf8[] = {0xEF, 0xBB, 0xBF, 0};
	if(strncmp((const char *)pucBufHead, aStrUtf8, 3) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static int GetPreOneBits(unsigned char ucVal)
{
	int i;
	int j=0;
	for(i=7; i>=0; i--)
	{
		if(!(ucVal & (1<<i)))
		{
			break;
		}
		else
		{
			j++;
		}
	}
	return j;
}

static int UTF8GetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	int i;
	int iNum;
	unsigned char ucVal;
	unsigned int dwSum = 0;
	if(pucBufStart > pucBufEnd)
	{
		return 0;
	}
	ucVal = pucBufStart[0];
	iNum = GetPreOneBits(pucBufStart[0]);
	if(( pucBufStart + iNum ) > pucBufEnd)
	{
		return 0;//文件结束
	}

	if(iNum ==0)
	{
		*pdwCode = pucBufStart[0];
		return 1;
	}
	else
	{
		ucVal 	= ucVal<<iNum;
		ucVal 	= ucVal>>iNum;//除去前面的1
		dwSum	+= ucVal; 
		for(i=1; i<iNum; i++)
		{
			ucVal 	= pucBufStart[i] & 0x3f;
			dwSum	= dwSum <<6;
			dwSum	= ucVal + dwSum;
		}
		*pdwCode = dwSum;
		return iNum;
	}
	
}


int  Utf8EncodingInit(void)
{
	AddFontOprForEncoding(&g_tUTF8EncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&g_tUTF8EncodingOpr, GetFontOpr("ascii"));	
	return RegisterEncodingOpr(&g_tUTF8EncodingOpr);
}
