/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	NumberPanel.c
**
**	AUTHOR:		Harry Qian
**
**	DATE:		13 - Sep - 2007
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**			2007-10-18,add mul-load function to numberpanel
**						
**				
**
**				
**	NOTES:
**		when using numberpanel.
**			CreateNumberPanel(HWND hWnd, int xPos, int yPos, NumProc lpNumerProc);
**			DeleteNumerPanel(HWND hWnd);
**			DrawNumberPanel(HWND hWnd, HDC Hdc);
**			NumPostionPro(HWND hWnd, int nXPos, int nYPos);
** 
** 
**
*/

/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <MoxFB.h>
#include <device.h>

#include "NumberPanel.h"
#include "SCLanguageWnd.h"
/************** USER INCLUDE FILES ***************************************************/

//#define NUMBERPANE_DEBUG


#define NUMBERPANEL_NUM		3
/************** DEFINES **************************************************************/
typedef	struct _NUMBERICON
{
	int						nId;
	char					szImage[256];
	short					nX1Pos;
	short					nY1Pos;
	short					nX2Pos;
	short					nY2Pos;
} NUMBERMENU;
/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

NUMBERMENU		number[NUMBER_MENU_CNT] = {
	{NUMBER_0_ID, IDC_NUMBER_0,	NUMBER_0_X, NUMBER_0_Y, NUMBER_0_X + NUMBER_LONG, NUMBER_0_Y + NUMBER_LONG},	
	{NUMBER_1_ID, IDC_NUMBER_1,	NUMBER_1_X, NUMBER_1_Y, NUMBER_1_X + NUMBER_LONG, NUMBER_1_Y + NUMBER_LONG},	
	{NUMBER_2_ID, IDC_NUMBER_2,	NUMBER_2_X, NUMBER_2_Y, NUMBER_2_X + NUMBER_LONG, NUMBER_2_Y + NUMBER_LONG},	
	{NUMBER_3_ID, IDC_NUMBER_3,	NUMBER_3_X, NUMBER_3_Y, NUMBER_3_X + NUMBER_LONG, NUMBER_3_Y + NUMBER_LONG},	
	{NUMBER_4_ID, IDC_NUMBER_4,	NUMBER_4_X, NUMBER_4_Y, NUMBER_4_X + NUMBER_LONG, NUMBER_4_Y + NUMBER_LONG},	
	{NUMBER_5_ID, IDC_NUMBER_5,	NUMBER_5_X, NUMBER_5_Y, NUMBER_5_X + NUMBER_LONG, NUMBER_5_Y + NUMBER_LONG},	
	{NUMBER_6_ID, IDC_NUMBER_6,	NUMBER_6_X, NUMBER_6_Y, NUMBER_6_X + NUMBER_LONG, NUMBER_6_Y + NUMBER_LONG},	
	{NUMBER_7_ID, IDC_NUMBER_7,	NUMBER_7_X, NUMBER_7_Y, NUMBER_7_X + NUMBER_LONG, NUMBER_7_Y + NUMBER_LONG},	
	{NUMBER_8_ID, IDC_NUMBER_8,	NUMBER_8_X, NUMBER_8_Y, NUMBER_8_X + NUMBER_LONG, NUMBER_8_Y + NUMBER_LONG},	
	{NUMBER_9_ID, IDC_NUMBER_9,	NUMBER_9_X, NUMBER_9_Y, NUMBER_9_X + NUMBER_LONG, NUMBER_9_Y + NUMBER_LONG},	
	{NUMBER_CL_ID, IDC_NUMBER_CLEAR,	NUMBER_CL_X,NUMBER_CL_Y,  NUMBER_CL_X + NUMBER_LONG, NUMBER_CL_Y + NUMBER_LONG},	
	{NUMBER_DL_ID, IDC_NUMBER_DEL,		NUMBER_DL_X,NUMBER_DL_Y,  NUMBER_DL_X + NUMBER_LONG, NUMBER_DL_Y + NUMBER_LONG}	
};
/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/
static int		GetNumberPanelIndex(HWND hWnd);

/************** LOCAL DECLARATIONS ***************************************************/
static NUMBERMODULE		g_NumInfo[NUMBERPANEL_NUM] = {{0,}, {0,}, {0,}};
/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MXAlloc
**	AUTHOR:			Harry Qian
**	DATE:			13 - Sep - 2007
**
**	DESCRIPTION:	
**			alloc memory
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nSize		[IN]		unsigned int
**	RETURNED VALUE:	
**				memory pointer that alloced
**	NOTES:
**			
*/
void
CreateNumberPanel(HWND hWnd, int xPos, int yPos, NumProc lpNumerProc)
{
	int i = 0;
	for(i = 0; i < NUMBERPANEL_NUM; i++)
	{
		if (g_NumInfo[i].bValid == TRUE)
		{
#ifdef NUMBERPANE_DEBUG
			printf("i = %d, exist a numberpanel..\n", i);
#endif
			continue;
		}
		else
		{
			g_NumInfo[i].bValid		= TRUE;
			g_NumInfo[i].hWnd		= hWnd;
			g_NumInfo[i].xPos		= xPos;
			g_NumInfo[i].yPos		= yPos;
			g_NumInfo[i].lpProc		= lpNumerProc;
#ifdef NUMBERPANE_DEBUG
			printf("create a new numberpanel..hwnd=%x.\n", hWnd);
#endif
			return;
		}
	}

}

void
DeleteNumerPanel(HWND hWnd)
{
	int nIndex = GetNumberPanelIndex(hWnd);
	if (nIndex !=  -1)
	{
#ifdef NUMBERPANE_DEBUG
		printf("delete a numberpanel, hWnd=%x.\n", hWnd);
#endif
		g_NumInfo[nIndex].bValid = FALSE;
	}
}

void
DrawNumberPanel(HWND hWnd, HDC Hdc)
{
	int		i = 0;
	char	szName[256] = {0};
	int		nIndex = GetNumberPanelIndex(hWnd);

	if (nIndex !=  -1)
	{
		for(i = 0; i < NUMBER_MENU_CNT; i++)
		{
			GetICONFileName(number[i].szImage, szName, 255);
			GdDrawImageFromFile(Hdc->psd, g_NumInfo[nIndex].xPos + number[i].nX1Pos, g_NumInfo[nIndex].yPos + number[i].nY1Pos, -1, -1, szName, 0);
		}
	}
	
}

void
NumPostionPro(HWND hWnd, int nXPos, int nYPos)
{
	int		i;
	int		nIndex = GetNumberPanelIndex(hWnd);
	if (nIndex != -1)
	{
		for (i = 0; i < NUMBER_MENU_CNT; i++)
		{
			if ( (nXPos >= g_NumInfo[nIndex].xPos + number[i].nX1Pos) 
				&& (nXPos <= g_NumInfo[nIndex].xPos + number[i].nX2Pos)
				&& (nYPos >= g_NumInfo[nIndex].yPos + number[i].nY1Pos) 
				&& (nYPos <= g_NumInfo[nIndex].yPos + number[i].nY2Pos))
			{
				g_NumInfo[nIndex].lpProc(hWnd, i);
				return;
			}
		}
	}
	

}

void
TouchNumberInput(UINT *pLen, char * pNum, UINT nMaxLen, UINT nKey)
{
	(*pLen)++;
	if (*pLen > nMaxLen)
	{
		*pLen = nMaxLen;
		return;
	}

	if (pNum == NULL)
	{
#ifdef NUMBERPANE_DEBUG
		printf("ERROR: the pointer of inputbuf is invalid.\n");
#endif
		return;
	}
	
	if (*pLen > 0)
	{
		pNum[*pLen - 1] = FindNumChar(nKey);
	}	
}

void
TouchNumberBack(UINT *pLen, char * pNum)
{
	if (pLen == NULL || pNum == NULL)
	{
#ifdef NUMBERPANE_DEBUG
		printf("the TouchNumberBack is error...,the pointer is invalid.\n");
#endif
		return;
	}
	if (*pLen > 0)
	{
		(*pLen)--;
		pNum[*pLen] = '\0';
	}
}

void
TouchNumerClear(UINT *pLen, char * pNum, int nMaxLen)
{
	if (pLen == NULL || pNum == NULL)
	{
#ifdef NUMBERPANE_DEBUG
		printf("the TouchNumerClear is error...,the pointer is invalid.\n");
#endif
		return;
	}
	(*pLen) = 0;
	memset(pNum, 0, nMaxLen);
}

static int
GetNumberPanelIndex(HWND hWnd)
{
	int i = 0;
	for(i = 0; i < NUMBERPANEL_NUM; i++)
	{
		if (TRUE == g_NumInfo[i].bValid && hWnd == g_NumInfo[i].hWnd)
		{
			return i;
		}
	}
	return -1;
}









