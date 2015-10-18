/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	GateParaSetWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		22 - Aug - 2008
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


/************** USER INCLUDE FILES ***************************************************/

#include "MenuCommon.h"
#include "MXTypes.h"
#include "MXCommon.h"
#include "AccessCommon.h"
#include "MenuParaProc.h"

#include "GataParaSetWnd.h"
#include "GateOvertimeSetWnd.h"
#include "IvdCardSwipeSetWnd.h"
#include "ManOpenSetWnd.h"
#include "GatePulseWidthSetWnd.h"
#include "InfraredSetWnd.h"

/************** DEFINES **************************************************************/

#define GATEPARASETDEPTH  4

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		GateParaSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK GateParaSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);


static VOID GateParaSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID GateOvertimeSet();
static VOID InvldCardSwipeSet();
static VOID DoorOpenManuSet();
static VOID GateRelayPulseWidth();
static VOID InfraredAlarm();

static MENUINFO g_GateParaSetting[GATEPARASETDEPTH] = 
{
	{TRUE, HS_ID_OPENOVERTIME, "Open Overtime",		"开门超时",		GateOvertimeSet},
	{FALSE, HS_ID_INVALIDCARD,"Invalid Card",				"无效刷卡",		InvldCardSwipeSet},
	{FALSE, HS_ID_MANUALSWITCH,"Manual Switch",		"手动开关",		DoorOpenManuSet},
	{FALSE,  HS_ID_INFRAREDALARM,"Infrared Alarm",		"红外报警", InfraredAlarm}
};

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MainMenuWndPaint
**	AUTHOR:			Jeff Wang
**	DATE:			21 - Aug - 2008
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
static VOID
GateParaSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	int				i = 0;
	INT			xPos = 0;
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	


#ifdef NEW_OLED_ENABLE	
	for(i = 0; i < GATEPARASETDEPTH; i++)
	{	
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  Open Overtime");
/*
			if (g_GateParaSetting[i].bSelected)
			{
				memset(pDateBuf, '*',1);
			}
			else
			{
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_GateParaSetting[i].MenuEnglishName);								
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_GateParaSetting[i].bSelected)
			{
				DrawEnglishCur(Hdc,xPos,i);
			}
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_OPENOVERTIME));

			memset(pDateBuf, ' ',2);	
			strcpy(&pDateBuf[2],GetHebrewStr(g_GateParaSetting[i].HebrewStrID)	);								
			MXDrawText_Right(Hdc, pDateBuf, xPos, i +1);
			if (g_GateParaSetting[i].bSelected)
			{
				DrawHebrewCur(Hdc,xPos,i);
			}
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  门磁脉冲宽度");
/*
			if (g_GateParaSetting[i].bSelected)
			{
				memset(pDateBuf, '*',1);
			}
			else
			{
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_GateParaSetting[i].MenuChineseName);								
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_GateParaSetting[i].bSelected)
			{
				DrawChineseCur(Hdc,xPos,i);
			}
		}
	}
#else
	for(i = 0; i < GATEPARASETDEPTH; i++)
	{	
		if (g_GateParaSetting[i].bSelected)
		{
			Hdc->textcolor = FONT_COLOR_SEL;			
		}
		else
		{
			Hdc->textcolor = FONT_COLOR;				
		}
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("Open Overtime");
			MXDrawText_Left(Hdc, g_GateParaSetting[i].MenuEnglishName, xPos, i);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("门磁脉冲宽度");
			MXDrawText_Left(Hdc, g_GateParaSetting[i].MenuChineseName, xPos, i);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_OPENOVERTIME));
			MXDrawText_Right(Hdc, GetHebrewStr(g_GateParaSetting[i].HebrewStrID), xPos, i+2);
		}
	}
#endif
	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MainMenuWndProc
**	AUTHOR:			Jeff Wang
**	DATE:			21 - Aug - 2008
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
GateParaSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_GATE_PARA, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			GateParaSetWndPaint(hWnd, Msg, wParam, lParam);
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
		ResetTimer(hWnd, TIMER_GATE_PARA, INTERFACE_SETTLE_TIME, NULL);
		GateParaSetKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_GATE_PARA);
//		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		KillAllChildWnd(hWnd);				
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_GATE_PARA);
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
**	FUNCTION NAME:	CreateMainMenuWnd
**	AUTHOR:			Jeff Wang
**	DATE:			21 - Aug - 2008
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
VOID
CreateGateParaSetWnd(HWND hwndParent)
{
	static char szAppName[] = "GateParaSetWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) GateParaSetWndProc;
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
		"GateParaSetWnd",			// Window name	(Not NULL)
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
**	FUNCTION NAME:	MainMenuKeyProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		7 - July - 2008
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

static VOID 
GateParaSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int i = 0;

	wParam =  KeyBoardMap(wParam);
	switch(wParam) 
	{
	case KEY_RETURN:
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
		break;
	case KEY_ENTER:
		{
			for (i = 0; i < GATEPARASETDEPTH; i++)
			{
				if (g_GateParaSetting[i].bSelected) 
				{
					if(g_GateParaSetting[i].Menufun)
					{
						g_GateParaSetting[i].Menufun();					
					}
					break;
				}
			}
		}
		break;
	case KEY_UP:
		{
			for (i = 0; i < GATEPARASETDEPTH; i++)
			{
				if ( g_GateParaSetting[i].bSelected ) 
				{
					if (0 == i) 
					{
						g_GateParaSetting[i].bSelected = FALSE;
						g_GateParaSetting[GATEPARASETDEPTH-1].bSelected = TRUE;
					} 
					else
					{
						g_GateParaSetting[i].bSelected = FALSE;
						g_GateParaSetting[i-1].bSelected = TRUE;
					}
					break;
				}
			}
		}
		
		break;
	case KEY_DOWN:
		{
			for (i = 0; i < GATEPARASETDEPTH; i++)
			{
				if ( g_GateParaSetting[i].bSelected ) 
				{
					if (GATEPARASETDEPTH-1 == i) 
					{
						g_GateParaSetting[i].bSelected = FALSE;
						g_GateParaSetting[0].bSelected = TRUE;
					} 
					else
					{
						g_GateParaSetting[i].bSelected = FALSE;
						g_GateParaSetting[i+1].bSelected = TRUE;
					}
					break;
				}
			}
		}
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;		
	default:
		break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOvertimeSet
**	AUTHOR:		   Jeff Wang
**	DATE:		9 - July - 2008
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

static VOID 
GateOvertimeSet()
{
	CreateGateOvertimeSetWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	InvldCardSet
**	AUTHOR:		   Jeff Wang
**	DATE:		9 - July - 2008
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

static VOID 
GateRelayPulseWidth()
{
	CreatePulseWidthSetWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	InvldCardSwipeSet
**	AUTHOR:		   Jeff Wang
**	DATE:		9 - July - 2008
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

static VOID 
InvldCardSwipeSet()
{
	CreateIvdCardSwipeSetWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DoorOpenManuSet
**	AUTHOR:		   Jeff Wang
**	DATE:		9 - July - 2008
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

static VOID 
DoorOpenManuSet()
{
	CreateManOpenSetWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	InfraredAlarm
**	AUTHOR:		   Jeff Wang
**	DATE:		30 - April - 2009
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
static VOID 
InfraredAlarm()
{
	CreateInfraredSetWnd(GetFocus());
}


