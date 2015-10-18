/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SysOtherParaSetWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		27 - Oct - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	
**
**	NOTES:
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

/************** USER INCLUDE FILES ***************************************************/

#include "MenuCommon.h"
#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"
#include "AccessSystemWnd.h"
#include "ModuleTalk.h"
#include "SysOtherParaSetWnd.h"
#include "RoomCodeDigitSetWnd.h"
#include "TalkTimeSetWnd.h"
#include "RingTimeSetWnd.h"

/************** DEFINES **************************************************************/

#define OTHERPARASETDEPTH  3//4

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static void		OtherParaSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static void Vlddigits();
static void TamperAlarmSel();
static void TalkTime();
static void RingTime();

static MENUINFO g_OtherParaSetInfo[OTHERPARASETDEPTH] = 
{
	{TRUE, HS_ID_VALIDROOMNUM, "Resident No. Length", "房间号有效位",	Vlddigits},
//	{FALSE, "Tamper Alarm",		 "防拆报警",	TamperAlarmSel},
	{FALSE, HS_ID_TALKTIME,"Talk Time",		 "通话时间",	TalkTime},
	{FALSE, HS_ID_RINGTIME,"Ring Time",		 "振铃时间",	RingTime},
};

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenAlarmWndPaint
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
OtherParaSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	int								  i	 = 0;
	char	     				   pDateBuf[TITLE_BUF_LEN] = {0};
	INT								xPos = 0;
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	

#ifdef NEW_OLED_ENABLE	
	for(i = 0; i < OTHERPARASETDEPTH; i++)
	{			
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  Resident No. Length");
/*
			if (g_OtherParaSetInfo[i].bSelected)
			{	
				memset(pDateBuf, '*',1);
			}
			else
			{		
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);							
			strcpy(&pDateBuf[2],g_OtherParaSetInfo[i].MenuEnglishName);					
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_VALIDROOMNUM));
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],GetHebrewStr(g_OtherParaSetInfo[i].HebrewStrID));								
			MXDrawText_Right(Hdc,pDateBuf , xPos, i);
			if (g_OtherParaSetInfo[i].bSelected)
			{
				DrawHebrewCur(Hdc,xPos,i);
			}
		}
		
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  房间号有效位");			
/*
			if (g_OtherParaSetInfo[i].bSelected)
			{	
				memset(pDateBuf, '*',1);
			}
			else
			{		
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_OtherParaSetInfo[i].MenuChineseName);					
		}
		
/*
		if(1 == i && g_SysConfig.EnTamperAlarm) 
		{
			strcat(pDateBuf, "*");
		}		
*/
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_OtherParaSetInfo[i].bSelected)
			{
				DrawEnglishCur(Hdc,xPos,i);
			}
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_OtherParaSetInfo[i].bSelected)
			{
				DrawChineseCur(Hdc,xPos,i);
			}
		}
	}
#else
	for(i = 0; i < OTHERPARASETDEPTH; i++)
	{	
		if (g_OtherParaSetInfo[i].bSelected)
		{	
			Hdc->textcolor = FONT_COLOR_SEL;			
		}
		else
		{		
			Hdc->textcolor = FONT_COLOR;					
		}
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("Resident No. Length");
			strcpy(pDateBuf, g_OtherParaSetInfo[i].MenuEnglishName);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("房间号有效位");			
			strcpy(pDateBuf, g_OtherParaSetInfo[i].MenuChineseName);
		}
		
	/*	if(1 == i && g_SysConfig.EnTamperAlarm) 
		{
			strcat(pDateBuf, "*");
		}		*/
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			MXDrawText_Left(Hdc, pDateBuf, xPos, i+1);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			MXDrawText_Left(Hdc, pDateBuf, xPos, i+1);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc, GetHebrewStr(HS_ID_VALIDROOMNUM));
			MXDrawText_Right(Hdc, GetHebrewStr(g_OtherParaSetInfo[i].HebrewStrID), xPos, i+1);
		}
	}
#endif
	ResetTimer(hWnd, TIMER_OTHER_PARA, INTERFACE_SETTLE_TIME, NULL);						
	
	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenDelaySetWndProc
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static LRESULT CALLBACK 
OtherParaSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);	
		break;
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_OTHER_PARA, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			OtherParaSetWndPaint(hWnd, Msg, wParam, lParam);
		}
		break;
		
	case WM_REFRESHPARENT_WND:
		SetFocus(hWnd);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;

	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		ResetTimer(hWnd, TIMER_OTHER_PARA, INTERFACE_SETTLE_TIME, NULL);						
		OtherParaSetKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_OTHER_PARA);
		KillAllChildWnd(hWnd);		
		//		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_OTHER_PARA);
//		SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		break;
		
	default:
//		if (WM_CLOSE == Msg) 
//		{
//			RemoveOneWnd(hWnd);
//		}
//		return DefWindowProc(hWnd, Msg, wParam, lParam);
		
		DefWindowProc(hWnd, Msg, wParam, lParam);
		
		if (WM_CLOSE == Msg) 
		{
			RemoveOneWnd(hWnd);
		}
	}
	return 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CreateGateOpenDelaySetWnd
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
CreateOtherParaSetWnd(HWND hwndParent)
{
	static char szAppName[] = "OtherParaSetWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) OtherParaSetWndProc;
	WndClass.cbClsExtra		= 0;
	WndClass.cbWndExtra		= 0;
	WndClass.hInstance		= 0;
	WndClass.hIcon			= 0;
	WndClass.hCursor		= 0;
	WndClass.hbrBackground	= (HBRUSH)GetStockObject(BACKGROUND_COLOR);
	WndClass.lpszMenuName	= NULL;
	WndClass.lpszClassName	= szAppName;
	
	RegisterClass(&WndClass);
	
	hWnd = CreateWindowEx(
		0L,					// Extend style	(0)
		szAppName,				// Class name	(NULL)
		"OtherParaSetWnd",			// Window name	(Not NULL)
		WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,	// Style		(0)
		0,					// x			(0)
		0,					// y			(0)
		SCREEN_WIDTH,		// Width		
		SCREEN_HEIGHT,		// Height
		g_hMainWnd /*hwndParent*/,		// Parent		(MwGetFocus())
		NULL,				// Menu			(NULL)
		NULL,				// Instance		(NULL)
		NULL);				// Parameter	(NULL)
	
	AddOneWnd(hWnd,WND_MEUN_PRIORITY_1);
	if (hWnd == GetFocus())
	{
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
	}
	else
	{
		ShowWindow(hWnd, SW_HIDE);
	}
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenDelaySetKeyProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		22 - Aug - 2008
**
**	DESCRIPTION:	
**			Process UP,DOWN,RETURM and ENTER	
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/

static void 
OtherParaSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int i = 0;

	wParam =  KeyBoardMap(wParam);
	switch(wParam) 
	{
	case KEY_RETURN:
		{
			SaveMenuPara2Mem();
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		break;
	case KEY_ENTER:
		{
			for (i = 0; i < OTHERPARASETDEPTH; i++)
			{
				if (g_OtherParaSetInfo[i].bSelected) 
				{
					if(g_OtherParaSetInfo[i].Menufun)
					{
						g_OtherParaSetInfo[i].Menufun();	
						PostMessage(hWnd,  WM_PAINT, 0, 0);
					}
					break;
				}
			}
		}
		break;
	case KEY_UP:
		{
			for (i = 0; i < OTHERPARASETDEPTH; i++)
			{
				if ( g_OtherParaSetInfo[i].bSelected ) 
				{
					if (0 == i) 
					{
						g_OtherParaSetInfo[i].bSelected = FALSE;
						g_OtherParaSetInfo[OTHERPARASETDEPTH-1].bSelected = TRUE;
					} 
					else
					{
						g_OtherParaSetInfo[i].bSelected = FALSE;
						g_OtherParaSetInfo[i-1].bSelected = TRUE;
					}
					break;
				}
			}
		}
		
		break;
	case KEY_DOWN:
		{
			for (i = 0; i < OTHERPARASETDEPTH; i++)
			{
				if ( g_OtherParaSetInfo[i].bSelected ) 
				{
					if (OTHERPARASETDEPTH-1 == i) 
					{
						g_OtherParaSetInfo[i].bSelected = FALSE;
						g_OtherParaSetInfo[0].bSelected = TRUE;
					} 
					else
					{
						g_OtherParaSetInfo[i].bSelected = FALSE;
						g_OtherParaSetInfo[i+1].bSelected = TRUE;
					}
					break;
				}
			}
		}
		break;
	default:
		break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RoomCodePassword
**	AUTHOR:		   Jeff Wang
**	DATE:		23 - Oct - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/

static void 
Vlddigits()
{
	CreateRoomCodeDigitSetWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CardPassword
**	AUTHOR:		   Jeff Wang
**	DATE:		23 - Oct - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/

static void 
TamperAlarmSel()
{
	g_SysConfig.EnTamperAlarm = !g_SysConfig.EnTamperAlarm;
}


static void 
TalkTime()
{
	CreateTalkTimeSetWnd(GetFocus());
}

static void 
RingTime()
{
	CreateRingTimeSetWnd(GetFocus());
}



