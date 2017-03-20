#include <fonts_manager.h>
#include <config.h>
#include<string.h>

static PT_FontOpr pt_FontOprHead = NULL;

int RegisterFontOpr(PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptTmp;
	if(!pt_FontOprHead)
	{
		pt_FontOprHead     =ptFontOpr;
		ptFontOpr->ptNext= NULL;
	}
	else
	{
		ptTmp=pt_FontOprHead;
		while(ptTmp->ptNext)
		{
			ptTmp=ptTmp->ptNext;
		}
		ptTmp->ptNext		=	ptFontOpr;
		ptFontOpr->ptNext	=	NULL;
	}
	return 0;
}


/*显示注册的fonts*/
void ShowFontOpr(void)
{
	int i = 0;
	PT_FontOpr ptTmp	= pt_FontOprHead;
	while(ptTmp)
	{
		printf("%02d %s\n",i++ , ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


PT_FontOpr GetFontOpr(char *pcName)
{
	PT_FontOpr ptTmp	= pt_FontOprHead;
	while(ptTmp)
	{
		if(strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

int FontsInit(void)
{
	int iError;
	iError = ASCIIInit();
	if(iError)
	{
		DBG_PRINTF("ASCIIInit error!\n");
		return -1;
	}
	iError = GBKInit();
	if(iError)
	{
		DBG_PRINTF("GBKInit error!\n");
		return -1;
	}
	iError = FreeTypeInit();
	if(iError)
	{
		DBG_PRINTF("FreeTypeInit error!\n");
		return -1;
	}
	return 0;
}


