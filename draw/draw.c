#include <draw.h>
#include <config.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include <string.h>
#include <fcntl.h>

typedef struct PageDesc{
	int iPage;
	unsigned char *pucLcdFirstPosAtFile;
	unsigned char *pucLcdNextPageFirstPosAtFile;
	struct PageDesc *ptPrePage;
	struct PageDesc *ptNextPage;
}T_PageDesc, *PT_PageDesc;

static PT_EncodingOpr g_ptEncodingOprForFile;
static PT_DispOpr	       g_ptDispOpr;

static int g_iFdTextFile;
static int g_dwFontSize;

static unsigned char *g_pucTextFileMem;
static unsigned char *g_pucTextFileMemEnd;

static unsigned char *g_pucLcdFirstPosAtFile;
static unsigned char *g_pucLcdNextPosAtFile;

static PT_PageDesc g_ptPages   = NULL;
static PT_PageDesc g_ptCurPage = NULL;


int IncLcdY(int iY)
{
	if(iY + g_dwFontSize <g_ptDispOpr->iYres)//current position less than device row
	{
		return iY + g_dwFontSize;
	}
	else
	{
		return 0;
	}

}



int OpenTextFile(char *pcFileName)
{
	struct stat tStat;
	g_iFdTextFile = open(pcFileName,O_RDONLY);
	if(0 > g_iFdTextFile)
	{
		printf("can't open text file %s\n", pcFileName);
		return -1;
	}
	if(fstat(g_iFdTextFile, &tStat))
	{
		printf("can't get fstat\n");
		return -1;
	}
	g_pucTextFileMem = (unsigned char *)mmap(NULL , tStat.st_size, PROT_READ  , MAP_SHARED , g_iFdTextFile , 0);
	if((unsigned char*)-1 == g_pucTextFileMem)
	{
		printf("can't mmap for file\n");
		return -1;
	}
	g_pucTextFileMemEnd    = g_pucTextFileMem + tStat.st_size;
	g_ptEncodingOprForFile =  SelectEncodingOprForFile(g_pucTextFileMem);
	if(g_ptEncodingOprForFile)
	{
		g_pucLcdFirstPosAtFile = g_pucTextFileMem + g_ptEncodingOprForFile->iHeadLen;//正文的位置
		return 0;
	}
	else
	{
		return -1;
	}	
	
}
int SetTextDetail(char *pcHZKFile, char *pcFileFreetype, unsigned int dwFontSize)
{
	int iError = 0;
	PT_FontOpr ptFontOpr;
	PT_FontOpr ptTmp;
	int iRet = -1;
	g_dwFontSize = dwFontSize;
	ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead;
	
	while(ptFontOpr)
	{
		if(strcmp(ptFontOpr->name, "ascii") == 0)
		{
			iError = ptFontOpr->FontInit(NULL, dwFontSize);
			printf("ascii  %d\n",iError);
		}
		else if(strcmp(ptFontOpr->name, "gbk") == 0)
		{
			iError = ptFontOpr->FontInit(pcHZKFile, dwFontSize);
			printf("GBK  %d\n",iError);
		}
		else 
		{
			iError = ptFontOpr->FontInit(pcFileFreetype, dwFontSize);
			printf("freetype  %d\n",iError);
		}
		DBG_PRINTF("%s, %d\n",ptFontOpr->name, iError);
		ptTmp = ptFontOpr->ptNext;

		if(ptTmp == NULL)
		{
			printf("ptTmp is NULL\n");
		}
		
		if(0 == iError)
		{
			iRet = 0;
		}
		else
		{
			DelFontOprFrmEncoding(g_ptEncodingOprForFile, ptFontOpr);
		}
		ptFontOpr = ptTmp;
	}
	return iRet;
}


int SelectAndInitDisplay(char *pcName)
{
	int iError;
	
	g_ptDispOpr = GetDispOpr(pcName);
	if(!g_ptDispOpr)
	{
		return -1;
	}

	iError = g_ptDispOpr->DeviceInit();
	return iError;
}

int RelocateFontPos(PT_FontBitMap ptFontBitMap)
{
	int iLcdY;
	int iDeltaX;
	int iDeltaY;
	if(ptFontBitMap->iYMax >g_ptDispOpr->iYres)
	{
		return -1;
	}
	if(ptFontBitMap->iXMax>g_ptDispOpr->iXres)
	{
		iLcdY = IncLcdY(ptFontBitMap->iCurOriginY);	
		if(0 == iLcdY)
		{
			return -1;
		}
		else
		{
		
			iDeltaX = 0 - ptFontBitMap->iCurOriginX;
			iDeltaY = iLcdY - ptFontBitMap->iCurOriginY;
			ptFontBitMap->iCurOriginX  += iDeltaX;
			ptFontBitMap->iCurOriginY  += iDeltaY;
			ptFontBitMap->iNextOriginX += iDeltaX;
			ptFontBitMap->iNextOriginY += iDeltaY;
			ptFontBitMap->iXLeft += iDeltaX;
			ptFontBitMap->iXMax  += iDeltaX;
			ptFontBitMap->iYTop  += iDeltaY;
			ptFontBitMap->iYMax  += iDeltaY;
			return 0;

		}
	}
	return 0;
}

void ExitDisplay(void)
{
	if (g_ptDispOpr->DeviceExit){
		g_ptDispOpr->DeviceExit();
	}

}


int ShowOneFont(PT_FontBitMap ptFontBitMap)
{
	int y;
	int x;
	int i=0;
	int bit;
	unsigned char ucByte = 0;
	
	if(ptFontBitMap->iBpp == 1)
	{
		for(y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++)
		{
			i =  (y - ptFontBitMap->iYTop) * ptFontBitMap->iPitch;
			for(x = ptFontBitMap->iXLeft, bit = 7; x <ptFontBitMap->iXMax; x++)//show one  row /8bits or row/16bits
			{
				if(bit == 7)
				{
					ucByte = ptFontBitMap->pucBuffer[i++];
				}
				if(ucByte & (1<<bit))
				{
					g_ptDispOpr->ShowPixel(x, y, COLOR_FOREGROUND);
				}
				else
				{
				
				}
				bit--;
				if(bit == -1)
				{
					bit = 7;
				}
			}
		}
	}
	else if(ptFontBitMap->iBpp == 8)
	{
		for(y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++)
		{
			for(x = ptFontBitMap->iXLeft; x <ptFontBitMap->iXMax; x++)//i Byte represent one pixel
			{
				if(ptFontBitMap->pucBuffer[i++])
				{
					g_ptDispOpr->ShowPixel(x, y, COLOR_FOREGROUND);
				}
			}
		}
	}
	else
	{
		DBG_PRINTF("ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp);
		return -1;
	}
	return 0;


}

int ShowOnePage(unsigned char *pucTextFileMemCurPos)
{
	int iLen;
	int iError;
	unsigned char *pucBufStart;
	unsigned int dwCode;
	T_FontBitMap tFontBitMap;
	PT_FontOpr ptFontOpr;
	int bHasNotClrSceen = 1;
	int bHasGetCode = 0;
	
	tFontBitMap.iCurOriginX = 0;
	tFontBitMap.iCurOriginY = g_dwFontSize;
	pucBufStart = pucTextFileMemCurPos;
	while(1)
	{
		iLen = g_ptEncodingOprForFile->GetCodeFrmBuf(pucBufStart, g_pucTextFileMemEnd, &dwCode ); //获得编码值，一次读一个字，该字可能是一个字节也可能是两个字节，也可能是多个字节
		if(0 == iLen)
		{
			if (0 == iLen)
			{
				/* 文件结束 */
				if (!bHasGetCode)
				{
				//没有从文件中获得过字符吧，也就是说该文件是空的
					return -1;
				}
				else
				{
					return 0;
				}
			}
		}
		bHasGetCode =1;
		pucBufStart += iLen;
		//下一个汉字的地址
		/* 有些文本, \n\r两个一起才表示回车换行
		 * 碰到这种连续的\n\r, 只处理一次
		 */
		 if(dwCode == '\n')
		 {
		 	g_pucLcdNextPosAtFile = pucBufStart;
			tFontBitMap.iCurOriginX = 0;
			tFontBitMap.iCurOriginY = IncLcdY(tFontBitMap.iCurOriginY);
			if(0 == tFontBitMap.iCurOriginY)
			{
				return 0;//current screen have finish
			}
			else
			{
				continue;
			}
		 }
	
		 else if(dwCode =='\r')
		 {
		 	continue;
		 }
		 else if(dwCode =='\t')
		 {
		 	dwCode = ' ';
		 }
		// printf("dwCode = 0x%x\n", dwCode);

		ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead;
		while(ptFontOpr)//font struct not null
		{
			DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			//get char's position in the lcd and  code position in the fontdata_8x16
			iError = ptFontOpr->GetFontBitmap(dwCode, &tFontBitMap);
			DBG_PRINTF("%s %s %d, ptFontOpr->name = %s, %d\n", __FILE__, __FUNCTION__, __LINE__, ptFontOpr->name, iError);
			if( 0 == iError)
			{
				DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				if(RelocateFontPos(&tFontBitMap))
				{
				/* 剩下的LCD空间不能满足显示这个字符 */
					return 0;
				}
				DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				if(bHasNotClrSceen)
				{
				/* 首先清屏 */
					g_ptDispOpr->CleanScreen(COLOR_BACKGROUND);
					bHasNotClrSceen = 0;
				}
				DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				if(ShowOneFont(&tFontBitMap))
				{
					/* 显示一个字符 */
					return -1;
				}
				tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
				tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
				g_pucLcdNextPosAtFile = pucBufStart;
				break; // show next word
			}
			ptFontOpr = ptFontOpr->ptNext; //code do not support  current font, find next font .
		}
		
	}	
	return 0;
}

static void RecordPage(PT_PageDesc ptPageNew)
{
	PT_PageDesc ptPage;
	if(!g_ptPages)
	{
		g_ptPages = ptPageNew;
	}
	else
	{
		ptPage = g_ptPages;
		while(ptPage->ptNextPage)
		{
			ptPage = ptPage->ptNextPage;
		}
		ptPage->ptNextPage 	   = ptPageNew;
		ptPageNew->ptPrePage = ptPage;
	}
}

int ShowNextPage(void)
{
	int iError;
	unsigned char *pucTextFileMemCurPos;
	PT_PageDesc ptPage;
	if(g_ptCurPage)
	{
		pucTextFileMemCurPos = g_ptCurPage->pucLcdNextPageFirstPosAtFile;//如果不是第一次显示
	}
	else
	{
		pucTextFileMemCurPos = g_pucLcdFirstPosAtFile;
	}
	iError = ShowOnePage(pucTextFileMemCurPos);
	DBG_PRINTF("%s %d, %d\n", __FUNCTION__, __LINE__, iError);

	if (iError == 0)
	{
		if(g_ptCurPage && g_ptCurPage->ptNextPage)
		{
			g_ptCurPage = g_ptCurPage->ptNextPage;
			return 0;
		}
		ptPage = malloc(sizeof(T_PageDesc));
		if(ptPage)
		{
			ptPage->pucLcdFirstPosAtFile 			= pucTextFileMemCurPos;
			ptPage->pucLcdNextPageFirstPosAtFile	= g_pucLcdNextPosAtFile;
			ptPage->ptPrePage					= NULL;
			ptPage->ptNextPage					= NULL;
			g_ptCurPage 							= ptPage;
			DBG_PRINTF("%s %d, pos = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)ptPage->pucLcdFirstPosAtFile);
			RecordPage(ptPage);
			return 0;
		}
		else
		{
			return -1;
		}
	}
	return iError;
}
int ShowPrePage(void)
{
	int iError;
	DBG_PRINTF("%s %d\n", __FUNCTION__, __LINE__);
	if (!g_ptCurPage || !g_ptCurPage->ptPrePage)//if current page and prepage are empty
	{
		return -1;
	}
	DBG_PRINTF("%s %d, pos = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)g_ptCurPage->ptPrePage->pucLcdFirstPosAtFile);
	iError = ShowOnePage(g_ptCurPage->ptPrePage->pucLcdFirstPosAtFile);
	if(iError == 0)
	{
		DBG_PRINTF("%s %d\n", __FUNCTION__, __LINE__);
		g_ptCurPage = g_ptCurPage->ptPrePage;
	}
	return iError;
	

}
