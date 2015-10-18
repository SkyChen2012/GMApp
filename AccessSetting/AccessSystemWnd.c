/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AccessSystemWnd.c
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
#include "MXMem.h"
#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"

#include "ASSetWnd.h"
#include "CardMngWnd.h"

/************** DEFINES **************************************************************/

#define ACCESSSYSTEMDEPTH  2

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		AccessSystemWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK AccessSystemWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID AccessSystemKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID ASSysSet();
static VOID MngResident();

static MENUINFO g_AccessSystem[ACCESSSYSTEMDEPTH] = 
{
	{TRUE,  HS_ID_ACCESSSYSSETTINGS,"Setting","门禁系统设置", ASSysSet},
	{FALSE, HS_ID_CARDMANAGE,"Card Manage",  "卡管理", MngResident}
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
AccessSystemWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	int								  i	 = 0;
	INT			xPos = 0;
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	
	Hdc->bkcolor = BACKGROUND_COLOR;

#ifdef NEW_OLED_ENABLE	
	for(i = 0; i < ACCESSSYSTEMDEPTH; i++)
	{
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  Card Manage");
/*
			if (g_AccessSystem[i].bSelected)
			{
				memset(pDateBuf, '*',1);
			}
			else
			{
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_AccessSystem[i].MenuEnglishName);								
			MXDrawText_Left(Hdc,pDateBuf , xPos, i+1);
			if (g_AccessSystem[i].bSelected)
			{
				DrawEnglishCur(Hdc,xPos,i);
			}
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_CARDMANAGE));
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],GetHebrewStr(g_AccessSystem[i].HebrewStrID));								
			MXDrawText_Right(Hdc,pDateBuf , xPos, i+1);
			if (g_AccessSystem[i].bSelected)
			{
				DrawHebrewCur(Hdc,xPos,i);
			}
		}
		
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  门禁系统设置");
/*
			if (g_AccessSystem[i].bSelected)
			{
				memset(pDateBuf, '*',1);
			}
			else
			{
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_AccessSystem[i].MenuChineseName);								
			MXDrawText_Left(Hdc, pDateBuf, xPos, i+1);
			if (g_AccessSystem[i].bSelected)
			{
				DrawChineseCur(Hdc,xPos,i+1);
			}
		}
	}
#else
	for(i = 0; i < ACCESSSYSTEMDEPTH; i++)
	{
		if (g_AccessSystem[i].bSelected)
		{
			Hdc->textcolor = FONT_COLOR_SEL;
		}
		else
		{
			Hdc->textcolor = FONT_COLOR;	
		}
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("Card Manage");
			MXDrawText_Left(Hdc, g_AccessSystem[i].MenuEnglishName, xPos, i+2);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("门禁系统设置");
			MXDrawText_Left(Hdc, g_AccessSystem[i].MenuChineseName, xPos, i+2);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc, GetHebrewStr(HS_ID_CARDMANAGE));
			MXDrawText_Right(Hdc, GetHebrewStr(g_AccessSystem[i].HebrewStrID), xPos, i+2);
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
AccessSystemWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_CREATE:
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;	
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_AS_MAIN, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			AccessSystemWndPaint(hWnd, Msg, wParam, lParam);
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
		ResetTimer(hWnd, TIMER_AS_MAIN, INTERFACE_SETTLE_TIME, NULL);
		AccessSystemKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_AS_MAIN);
		KillAllChildWnd(hWnd);		
		//		PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_AS_MAIN);
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
CreateAccessSystemWnd(HWND hwndParent)
{
	static char szAppName[] = "AccessSystemWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) AccessSystemWndProc;
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
		"AccessSystemWnd",			// Window name	(Not NULL)
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
AccessSystemKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
			for (i = 0; i < ACCESSSYSTEMDEPTH; i++)
			{
				if (g_AccessSystem[i].bSelected) 
				{
					if(g_AccessSystem[i].Menufun)
					{
						g_AccessSystem[i].Menufun();					
					}
					break;
				}
			}
		}
		break;
	case KEY_UP:
		{
			for (i = 0; i < ACCESSSYSTEMDEPTH; i++)
			{
				if ( g_AccessSystem[i].bSelected ) 
				{
					if (0 == i) 
					{
						g_AccessSystem[i].bSelected = FALSE;
						g_AccessSystem[ACCESSSYSTEMDEPTH-1].bSelected = TRUE;
					} 
					else
					{
						g_AccessSystem[i].bSelected = FALSE;
						g_AccessSystem[i-1].bSelected = TRUE;
					}
					break;
				}
			}
		}
		
		break;
	case KEY_DOWN:
		{
			for (i = 0; i < ACCESSSYSTEMDEPTH; i++)
			{
				if ( g_AccessSystem[i].bSelected ) 
				{
					if (ACCESSSYSTEMDEPTH-1 == i) 
					{
						g_AccessSystem[i].bSelected = FALSE;
						g_AccessSystem[0].bSelected = TRUE;
					} 
					else
					{
						g_AccessSystem[i].bSelected = FALSE;
						g_AccessSystem[i+1].bSelected = TRUE;
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
**	FUNCTION NAME:	ASSysSet
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
ASSysSet()
{
	CreateASSetWnd(GetFocus());
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MngResident
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
MngResident()
{
	CreateResiMngWnd(GetFocus());
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ResidentSetting
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
/*
static void ResidentSetting()
{
	
}
*/
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ASLog
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
/*
static void ASLog()
{
	printf("Access System Log\n");
}
*/

