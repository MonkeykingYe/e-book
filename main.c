#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <config.h>
#include <draw.h>
#include <disp_manager.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <string.h>
int main(int argc, char **argv)
{
	int iError;
	int bList = 0;
	unsigned int dwFontSize = 16;

	char acFreeTypeFile[128];
	char acHzkFile[128];
	char acTextFile[128];
	char acDisplay[128];
	char cOpr;

	acFreeTypeFile[0] = '\0';
	acHzkFile[0]	       = '\0';
	acTextFile[0]	       = '\0';
	//acDisplay[0]	       = '\0';

	strcpy(acDisplay, "fb");
	
	while((iError = getopt(argc, argv, "ls:f:h:d:"))!= -1)
	{
		switch(iError)
		{
			case 'l':
			{
				bList = 1;
				break;
			}
			case 's':
			{
				dwFontSize = strtoul(optarg, NULL, 0);
				break;
			}
			case 'f':
			{
				strncpy(acFreeTypeFile, optarg, 128);
				acFreeTypeFile[127] = '\0';
				
				break;
			}
			case 'h':
			{
				
				strncpy(acHzkFile, optarg, 128);
				acHzkFile[127] = '\0';
	
				break;
			}
			case 'd':
			{
				strncpy(acDisplay, optarg, 128);
				acDisplay[127] = '\0';
				
				break;
			}
			default:
			{
				printf("Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n", argv[0]);
				printf("Usage: %s -l\n", argv[0]);
				return -1;
				break;
			}
		}
	}
	if (!bList && (optind >= argc))
	{
		printf("Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n", argv[0]);
		printf("Usage: %s -l\n", argv[0]);
		return -1;
	}
	
	printf("DisplayInit\n");
	iError = DisplayInit();
	if(iError)
	{
		printf("DisplayInit error!\n");
		return -1;
	}
	printf("FontsInit\n");
	iError = FontsInit();
	if(iError)
	{
		printf("FontsInit error!\n");
		return -1;
	}
	iError = EncodingInit();
	if(iError)
	{
		printf("EncodingInit error!\n");
		return -1;
	}
	if(bList)
	{
		printf("supported display:\n");
		ShowDispOpr();
		printf("supported font:\n");
		ShowFontOpr();
		printf("supported encoding:\n");
		ShowEncodingOpr();
		return 0;
	}

	strncpy(acTextFile, argv[optind], 128);
	acTextFile[127] = '\0';
	iError = OpenTextFile(acTextFile);
	if(iError)
	{
		printf("OpenTextFile error!\n");
		return -1;
	}

	printf("open file success\n");
	
	iError = SetTextDetail(acHzkFile, acFreeTypeFile, dwFontSize);
	if(iError)
	{
		printf("SetTextDetail error!\n");
		return -1;
	}
	iError = SelectAndInitDisplay(acDisplay);
	if (iError)
	{
		printf("SelectAndInitDisplay error!\n");
		return -1;
	}
	iError = ShowNextPage();
	if (iError)
	{
		printf("Error to show first page\n");
		return -1;
	}	

	while(1)
	{
		printf("Enter 'n' to show next page, 'u' to show previous page, 'q' to exit: ");
		do
		{
			cOpr = getchar();
		}while ((cOpr != 'n') && (cOpr != 'u') && (cOpr != 'q'));

		if(cOpr == 'n')
		{
			ShowNextPage();
		}

		else if(cOpr == 'u')
		{
			ShowPrePage();
		}
		else
		{
			//ExitDisplay();
			return 0;
		}
	}
	return 0;
}


