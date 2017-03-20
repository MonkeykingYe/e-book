#include <string.h>
#include <disp_manager.h>
#include <config.h>

static PT_DispOpr g_ptDispOprHead;

int RegisterDispOpr(PT_DispOpr ptDispOpr)
{
	PT_DispOpr ptTmp;
	/* 如果链表头为空*/
	if (!g_ptDispOprHead)  
	{
		g_ptDispOprHead = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptDispOprHead;
		/*找到最后一个链表成员*/
		while(ptTmp->ptNext){  
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext 		= ptDispOpr;
		ptDispOpr->ptNext 	= NULL;
		
	}
	

	return 0;
}

void ShowDispOpr(void)//显示所有设备名称
{
	int  i = 0;
	PT_DispOpr ptTmp = g_ptDispOprHead;
	while(ptTmp){
		printf("%02d  %s\n",i++ , ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_DispOpr GetDispOpr(char * pcName)
{
	PT_DispOpr ptTmp = g_ptDispOprHead;
	while( ptTmp){
		if(strcmp(ptTmp->name,  pcName)==0){
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	
	}
	return NULL;
}

int DisplayInit(void)
{
	int iError;
	iError = FBInit();
	return iError;


}





