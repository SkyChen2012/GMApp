

/* defines for, or consts and inline functions for C++ */

/* global includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>	/* Only needed for the test function */
#include <ctype.h>
#include <unistd.h>
#include <windows.h>
/* local includes */
#include "IniFile.h"
#include "UnicodeString.h"
#include "MXList.h"
#include "MXCommon.h"
#include "MenuParaProc.h"

/* Global Variables */
#define HEBREWSTINGFILE																"/mox/rdwr/language/Hebrew.str"
#define MIN_HERVREW_UNICODE		0x05B0
#define MAX_HERVREW_UNICODE		0x05F4

#define MIN_ENGLISH_UNICODE		0x20
#define MAX_ENGLISH_UNICODE		0xFF
#define MIN_NUM_UNICODE		0x30
#define MAX_NUM_UNICODE		0x39

typedef struct _LANGINFO
{
	UNICODE_LANG_ID 	LangID;
	MXListHead		Head;
	int				(*CheckUnicode)(char cData1,char cData2);
	bool 			bUsed;
}LANGINFO;

typedef struct _LANG_STR_INFO
{
	MXList						List;
	char *	pData;
}LANG_STR_INFO;




FILE *StringFile;
static LANGINFO	gLangInfo[LANG_CNT];


static bool OpenUnicodeStringFile (UNICODE_LANG_ID LangID,cchr * FileName);
static int CheckHebrewUnicode(char cData1, char cData2);
static int ConvertHebrewStr(char * SrcStr,char * DestStr);

void InitUnicodeString(void)
{
	memset(gLangInfo,0,sizeof(gLangInfo));
}


void LoadHebrewString(void)
{
	gLangInfo[LANG_HEBREW].CheckUnicode=CheckHebrewUnicode;
	OpenUnicodeStringFile(LANG_HEBREW,HEBREWSTINGFILE);
	
}

static bool 
OpenUnicodeStringFile (UNICODE_LANG_ID LangID,cchr * FileName)
{
	char Str[10];
	char ValidStr[255];
	int	Len;
	//int i;
	int nReadIndex=0;
	LANG_STR_INFO * pLangStr;
	int nStrLen=0;
	if (FileName == NULL)
	{
		return FALSE;
	}
	if ((StringFile = fopen (FileName, "r")) == NULL)
	{
	return FALSE;
	}

	gLangInfo[LangID].bUsed=TRUE;

	while ((Len=fread (Str,2,1, StringFile))>0)
	{
		fseek(StringFile, nReadIndex, SEEK_SET);
		if(0x00==Str[0] && 0x0a==Str[1])
		{
			pLangStr=(LANG_STR_INFO *)malloc(sizeof(LANG_STR_INFO));
			pLangStr->pData=malloc(nStrLen+1);
			//ConvertHebrewStr(ValidStr,pLangStr->pData,nStrLen);
			memcpy(pLangStr->pData,ValidStr,nStrLen);
			pLangStr->pData[nStrLen]='\0';
			MXListAdd(&gLangInfo[LangID].Head, (MXList*) pLangStr);
			nStrLen=0;
		}
		else
		{
			switch(gLangInfo[LANG_HEBREW].CheckUnicode(Str[0],Str[1]))
			{
				case 2:
					memcpy(&ValidStr[nStrLen],Str,2);
					nStrLen+=2;
					break;
				case 1:
					memcpy(&ValidStr[nStrLen],&Str[1],1);
					nStrLen+=1;
					break;
				default:
					break;
			}
		}
		nReadIndex+=2;
	}

	fclose (StringFile);
	
	/*MXList*					pNext;
	pNext = gLangInfo[LangID].Head.pHead;

	while (pNext != NULL)
	{
		pLangStr=(LANG_STR_INFO *)pNext;
		for(i=0;i<strlen(pLangStr->pData);i++)
		{
			printf("Str[%d]=0x%02x ",i,pLangStr->pData[i]);
		}
		printf("\n");
		pNext = pNext->pNext;
	}*/
    StringFile = NULL;
	return TRUE;
}
char * GetHebrewStr(HEBREW_STR_ID StrID)
{
	int i;
	MXList*					pNext;
	LANG_STR_INFO * pLangStr;
	pNext = gLangInfo[LANG_HEBREW].Head.pHead;
	if(pNext==NULL)
	{
		return NULL;
	}
	for(i=0;i<StrID;i++)
	{	
		pNext = pNext->pNext;
		if(pNext==NULL)
		{
			return NULL;
		}
	}
	pLangStr=(LANG_STR_INFO *)pNext;
	return pLangStr->pData;
}

static int CheckHebrewUnicode(char cData1,char cData2)
{
	unsigned short sDestUnicode=0;
	sDestUnicode=(cData1<<8)+cData2;
	if(sDestUnicode>=MIN_HERVREW_UNICODE && sDestUnicode<=MAX_HERVREW_UNICODE)
	{
		return 2;
	}
	else if(sDestUnicode>=MIN_ENGLISH_UNICODE && sDestUnicode<=MAX_ENGLISH_UNICODE)
	{
		return 1;
	}
	return 0;
}
/*void HebrewDrewText(HDC Hdc,RECT * pRect,char * Str)
{
	char DestStr[255];
	ConvertHebrewStr(Str,DestStr);
	DrawText(Hdc, DestStr, -1, pRect, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
}*/
int WINAPI
HebrewDrawText(HDC hdc, LPCSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
	char DestStr[255];
	//printf("HebrewDrawText len=%d\n",strlen(lpString));
	if(strlen(lpString)>255)
	{
		return 0;
	}
	ConvertHebrewStr((char *)lpString,DestStr);
	DrawText(hdc, DestStr, nCount, lpRect,uFormat);
	return 0;
}
int WINAPI
MXDrawText(HDC hdc, LPCSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
	if(SET_HEBREW == g_SysConfig.LangSel)
	{
		HebrewDrawText(hdc, lpString, nCount, lpRect, uFormat);	
	}
	else
	{
		DrawText(hdc, lpString, nCount, lpRect, uFormat);
	}
	return 0;	
}

int HebrewGetTextWidth(HDC hdc, LPCVOID lpString)
{
int	 width, height, baseline;
GdGetTextSize(hdc->font->pfont, lpString, strlen(lpString),&width, &height, &baseline, MWTF_ASCII);	
	//return hebrew_gettextwidth(lpString);
	return width;
}



long 
HebrewGetXPos(HDC hdc,char *pBuf)
{
	int nWidth=0;
	//printf("Step1,pBuf=%d\n",pBuf);
	SelectObject( hdc, GetHZKFont32());
	//printf("Step2\n");
#ifdef NEW_OLED_ENABLE
	nWidth=HebrewGetTextWidth(hdc,pBuf);
	//printf("Step3\n");
	if(SCREEN_WIDTH>nWidth)
	{
		return SCREEN_WIDTH-((SCREEN_WIDTH -nWidth ) / 2);
	}
	else
	{
		return 0;
	}
#else
	if(SCREEN_WIDTH>16* strlen(pBuf))
	{
		return (SCREEN_WIDTH - 16 * strlen(pBuf)) / 2;
	}
	else
	{
		return 0;
	}
	
#endif
}
#define STR_DOT_UNICODE	0x2E
#define STR_ROD_UNICODE	0x2D
#define STR_SPACE_UNICODE	0x20


#define MIN_ENGLET_UNICODE	0x41
#define MAX_ENGLET_UNICODE	0x7A


static BOOL IsHeberwNoConv(char cStr)
{
	/*if(((cStr>=MIN_NUM_UNICODE) && (cStr<=MAX_NUM_UNICODE))
		|| ((cStr>=MIN_ENGLET_UNICODE) && (cStr<=MAX_ENGLET_UNICODE))
		|| (STR_DOT_UNICODE==cStr)
		|| (STR_ROD_UNICODE==cStr))
	{
		return TRUE;
	}*/
	if((cStr>=MIN_ENGLISH_UNICODE) && (cStr<=MAX_ENGLISH_UNICODE))
	{
		return TRUE;
	}
	return FALSE;
}
static BOOL IsHeberwSign(char cStr)
{
	if(((cStr>=MIN_NUM_UNICODE) && (cStr<=MAX_NUM_UNICODE))
		|| ((cStr>=MIN_ENGLET_UNICODE) && (cStr<=MAX_ENGLET_UNICODE))
		|| (STR_SPACE_UNICODE==cStr)
		)
	{
		return FALSE;
	}
	return TRUE;
}
static BOOL IsCharHebrew(char * pChar)
{
	int nLen;
	unsigned short sDestUnicode=0;
	nLen=strlen(pChar);
	if(nLen>1)
	{
		sDestUnicode=(pChar[0]<<8)+pChar[1];
		if(sDestUnicode>=MIN_HERVREW_UNICODE && sDestUnicode<=MAX_HERVREW_UNICODE)
		{
			return TRUE;
		}
	}
	return FALSE;
}
static int GetConvertLen(char * pChar)
{
	int nLen,i;
	unsigned short sDestUnicode=0;
	int nHerbrewIndex=0;
	int nLetIndex=0;
	nLen=strlen(pChar);
	
	for(i=0;i<nLen;i++)
	{
		if(nLen-i>1)
		{
			sDestUnicode=(pChar[i]<<8)+pChar[i+1];
		
			if(sDestUnicode>=MIN_HERVREW_UNICODE && sDestUnicode<=MAX_HERVREW_UNICODE)
			{
				nHerbrewIndex=i;
				i+=1;
				break;
			}
		}
		else
		{
			nHerbrewIndex=nLen;
			break;
		}
	}
	//printf("nHerbrewIndex=%d\n",nHerbrewIndex);	
	for(i=0;i<nHerbrewIndex;i++)
	{
		if(IsHeberwNoConv(pChar[nHerbrewIndex-i-1]) && (!IsHeberwSign(pChar[nHerbrewIndex-i-1])))
		{
			nLetIndex=nHerbrewIndex-i;
			break;
		}
	}
	return nLetIndex;
}

static void ConvertHebrewNumStr(char * SrcStr,char * DestStr)
{
	int i,j;
	int offset=0;
	int nStrlen;
	int nConvertLen;
	char TempStr[50];
	unsigned short sDestUnicode=0;
	nStrlen=strlen(SrcStr);
	strcpy(DestStr,SrcStr);
	

	while((nStrlen-offset)>0)
	{
		if(IsCharHebrew(&SrcStr[offset]))
		{
			offset+=2;
		}
		else
		{
			if(IsHeberwNoConv(SrcStr[offset]))
			{
				nConvertLen=0;
				i=0;
				nConvertLen+=GetConvertLen(&SrcStr[offset]);
				for(j=0;j<nConvertLen;j++)
				{
					TempStr[j]=SrcStr[nConvertLen+offset-j-1];
				}	
				memcpy(&DestStr[offset],TempStr,nConvertLen);
				if(nConvertLen)
				{
					offset+=nConvertLen;
				}
				else
				{
					offset++;
				}
				
			}
			else
			{
				offset++;	
			}
		}
	}
}

static int ConvertHebrewStr(char * SrcStr,char * DestStr)
{
	int i;
	int nWriteLen=0;
	unsigned short sDestUnicode=0;
	int nSrcStrLen=strlen(SrcStr);
	char TempDestStr[255];
	ConvertHebrewNumStr(SrcStr,TempDestStr);
	for(i=0;i<nSrcStrLen;i++)
	{
		if(nSrcStrLen-i>1)
		{
			sDestUnicode=(TempDestStr[nSrcStrLen-i-2]<<8)+TempDestStr[nSrcStrLen-i-1];
		
			if(sDestUnicode>=MIN_HERVREW_UNICODE && sDestUnicode<=MAX_HERVREW_UNICODE)
			{
				nWriteLen+=2;
				memcpy(&DestStr[i],&TempDestStr[nSrcStrLen-i-2],2);
				i+=1;
			}
			else if(TempDestStr[nSrcStrLen-i-1]>=MIN_ENGLISH_UNICODE && TempDestStr[nSrcStrLen-i-1]<=MAX_ENGLISH_UNICODE)
			{
				memcpy(&DestStr[i],&TempDestStr[nSrcStrLen-i-1],1);
				nWriteLen+=1;
			}
		}
		else
		{
			if(TempDestStr[nSrcStrLen-i-1]>=MIN_ENGLISH_UNICODE && TempDestStr[nSrcStrLen-i-1]<=MAX_ENGLISH_UNICODE)
			{
				memcpy(&DestStr[i],&TempDestStr[nSrcStrLen-i-1],1);
				nWriteLen+=1;
			}
		}

	}

	DestStr[nWriteLen]='\0';

	return nWriteLen;
}

