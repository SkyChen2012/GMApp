/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	DelModeSelWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		25 - Aug - 2008
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
#include "MenuParaProc.h"
#include "MXTypes.h"
#include "MXCommon.h"

#include "DelCardModeWnd.h"
#include "DelLocalNumModeWnd.h"
#include "CardProc.h"
#include "AccessProc.h"

/************** DEFINES **************************************************************/

#define DELMODESELDEPTH  3

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		DelModeSelWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK DelModeSelWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID DelCardModeProc();
static VOID DelLocalNumModeProc();
static VOID DelRdNumModeProc();

static VOID DelModeSelKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static MENUINFO g_DelModeSelInfo[DELMODESELDEPTH] = 
{
	{TRUE, HS_ID_BYCARD,"by Card",			"卡方式",			DelCardModeProc},
	{FALSE, HS_ID_BYROOM,"by Resident No.",	"房间号方式",		DelRdNumModeProc},
	{FALSE, HS_ID_BYLOCNUM,"by Local No.",		"本地编号方式",		DelLocalNumModeProc}
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
DelModeSelWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	int				i = 0;
	INT				xPos = 0;
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	


#ifdef NEW_OLED_ENABLE	
	for(i = 0; i < DELMODESELDEPTH; i++)
	{
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  Resident Number");
/*
			if (g_DelModeSelInfo[i].bSelected)
			{
				memset(pDateBuf, '*',1);
			}
			else
			{
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_DelModeSelInfo[i].MenuEnglishName);								
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_DelModeSelInfo[i].bSelected)
			{
				DrawEnglishCur(Hdc,xPos,i);
			}
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_BYLOCNUM));

			memset(pDateBuf, ' ',2);	
			strcpy(&pDateBuf[2],GetHebrewStr(g_DelModeSelInfo[i].HebrewStrID)	);								
			MXDrawText_Right(Hdc, pDateBuf, xPos, i );
			if (g_DelModeSelInfo[i].bSelected)
			{
				DrawHebrewCur(Hdc,xPos,i);
			}
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  本地编号方式");
/*
			if (g_DelModeSelInfo[i].bSelected)
			{
				memset(pDateBuf, '*',1);
			}
			else
			{
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_DelModeSelInfo[i].MenuChineseName);								
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_DelModeSelInfo[i].bSelected)
			{
				DrawChineseCur(Hdc,xPos,i);
			}
		}
	}
#else
	for(i = 0; i < DELMODESELDEPTH; i++)
	{
		if (g_DelModeSelInfo[i].bSelected)
		{
			Hdc->textcolor = FONT_COLOR_SEL;	
		}
		else
		{
			Hdc->textcolor = FONT_COLOR;	
		}
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("Resident Number");
			MXDrawText_Left(Hdc, g_DelModeSelInfo[i].MenuEnglishName, xPos, i+1);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("本地编号方式");
			MXDrawText_Left(Hdc, g_DelModeSelInfo[i].MenuChineseName, xPos, i+1);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc, GetHebrewStr(HS_ID_BYLOCNUM));
			MXDrawText_Right(Hdc, GetHebrewStr(g_DelModeSelInfo[i].HebrewStrID), xPos, i+1);
		}
	}
#endif
	ResetTimer(hWnd, TIMER_DELMODE_WND, INTERFACE_SETTLE_TIME, NULL);
	
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
DelModeSelWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_DELMODE_WND, wParam, NULL);
		break;
		
	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			DelModeSelWndPaint(hWnd, Msg, wParam, lParam);
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
		ResetTimer(hWnd, TIMER_DELMODE_WND, INTERFACE_SETTLE_TIME, NULL);
		DelModeSelKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_DELMODE_WND);
		KillAllChildWnd(hWnd);						
//		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_DELMODE_WND);
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
CreateDelModeSelWnd(HWND hwndParent)
{
	static char szAppName[] = "DelModeSelWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) DelModeSelWndProc;
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
		"DelModeSelWnd",			// Window name	(Not NULL)
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
DelModeSelKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int i = 0;

	wParam =  KeyBoardMap(wParam);

	switch(wParam) 
	{
	case KEY_RETURN:
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		g_ASInfo.ASWorkStatus = STATUS_OPEN;
	}
		break;
	case KEY_ENTER:
		{
			for (i = 0; i < DELMODESELDEPTH; i++)
			{
				if (g_DelModeSelInfo[i].bSelected) 
				{
					if(g_DelModeSelInfo[i].Menufun)
					{
						g_DelModeSelInfo[i].Menufun();					
					}
					break;
				}
			}
		}
		break;
	case KEY_UP:
		{
			for (i = 0; i < DELMODESELDEPTH; i++)
			{
				if ( g_DelModeSelInfo[i].bSelected ) 
				{
					if (0 == i) 
					{
						g_DelModeSelInfo[i].bSelected = FALSE;
						g_DelModeSelInfo[DELMODESELDEPTH-1].bSelected = TRUE;
					} 
					else
					{
						g_DelModeSelInfo[i].bSelected = FALSE;
						g_DelModeSelInfo[i-1].bSelected = TRUE;
					}
					break;
				}
			}
		}
		
		break;
	case KEY_DOWN:
		{
			for (i = 0; i < DELMODESELDEPTH; i++)
			{
				if ( g_DelModeSelInfo[i].bSelected ) 
				{
					if (DELMODESELDEPTH-1 == i) 
					{
						g_DelModeSelInfo[i].bSelected = FALSE;
						g_DelModeSelInfo[0].bSelected = TRUE;
					} 
					else
					{
						g_DelModeSelInfo[i].bSelected = FALSE;
						g_DelModeSelInfo[i+1].bSelected = TRUE;
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

static VOID 
DelCardModeProc()
{
	CreateDelCardModeWnd(GetFocus());
	g_ASInfo.ASWorkStatus = STATUS_DEL_CARD_MODE;
}

static VOID 
DelLocalNumModeProc()
{
	CreateDelLocalNumModeWnd(GetFocus());
	g_ASInfo.ASWorkStatus = STATUS_DEL_LOCALNUM_MODE;
}

static VOID 
DelRdNumModeProc()
{
	CreateDelRdModeWnd(GetFocus());
	g_ASInfo.ASWorkStatus = STATUS_DEL_RD_MODE;
}

