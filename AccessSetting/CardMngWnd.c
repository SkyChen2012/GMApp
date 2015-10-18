/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	ResiMngWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		24 - Aug - 2008
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
#include "MenuParaProc.h"

#include "CardMngWnd.h"
#include "AddCardWnd.h"
#include "DelModeSelWnd.h"
#include "CardProc.h"
#include "AccessProc.h"

/************** DEFINES **************************************************************/

#define RESIMNGDEPTH  4

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		ResiMngWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK ResiMngWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID AddNormalProc();
static VOID DelCardProc();
static VOID AddAccreditCardProc();
static VOID AddPatrolProc();

static VOID ResiMngKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static MENUINFO g_ResiMngInfo[RESIMNGDEPTH] = 
{
	{TRUE, HS_ID_ADDRESIDENTCARD, "Add Resident Card",		"新增住户卡",					AddNormalProc},
	{FALSE, HS_ID_ADDPATROLCARD,"Add Patrol Card",			"新增巡更卡",				   AddPatrolProc},
	{FALSE, HS_ID_ADDAUTHCARD,"Add Authority Card",	    "新增授权卡",			 	 AddAccreditCardProc},
	{FALSE, HS_ID_DELECARD,"Delete Card",				"删除卡",						DelCardProc},
};

/*************************************************************************************/

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
ResiMngWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	int								  i	 = 0;
	INT								xPos = 0;
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	

#ifdef NEW_OLED_ENABLE	
	Hdc->textcolor = FONT_COLOR;			
	for(i = 0; i < RESIMNGDEPTH; i++)
	{
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  Add Authority Card");
/*
			if (g_ResiMngInfo[i].bSelected)
			{			
				memset(pDateBuf, '*',1);
			}
			else
			{			
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_ResiMngInfo[i].MenuEnglishName);								
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_ResiMngInfo[i].bSelected)
			{
				DrawEnglishCur(Hdc,xPos,i);
			}
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ADDAUTHCARD));

			memset(pDateBuf, ' ',2);	
			strcpy(&pDateBuf[2],GetHebrewStr(g_ResiMngInfo[i].HebrewStrID)	);								
			MXDrawText_Right(Hdc, pDateBuf, xPos, i);
			if (g_ResiMngInfo[i].bSelected)
			{
				DrawHebrewCur(Hdc,xPos,i);
			}
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  新增住户卡");
/*
			if (g_ResiMngInfo[i].bSelected)
			{			
				memset(pDateBuf, '*',1);
			}
			else
			{			
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_ResiMngInfo[i].MenuChineseName);								
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_ResiMngInfo[i].bSelected)
			{
				DrawChineseCur(Hdc,xPos,i);
			}
		}	
	}
#else
	for(i = 0; i < RESIMNGDEPTH; i++)
	{
		if (g_ResiMngInfo[i].bSelected)
		{			
			Hdc->textcolor = FONT_COLOR_SEL;
		}
		else
		{			
			Hdc->textcolor = FONT_COLOR;
		}
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos(" Add Authority Card");
			MXDrawText_Left(Hdc, g_ResiMngInfo[i].MenuEnglishName, xPos, i);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("新增住户卡");
			MXDrawText_Left(Hdc, g_ResiMngInfo[i].MenuChineseName, xPos, i);
		}	
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc, GetHebrewStr(HS_ID_ADDAUTHCARD));
			MXDrawText_Right(Hdc, GetHebrewStr(g_ResiMngInfo[i].HebrewStrID), xPos, i);
		}
		
	}
#endif
	ResetTimer(hWnd, TIMER_RESI_MNG, INTERFACE_SETTLE_TIME, NULL);			
	
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
ResiMngWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;	
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_RESI_MNG, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			ResiMngWndPaint(hWnd, Msg, wParam, lParam);
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
		ResetTimer(hWnd, TIMER_RESI_MNG, INTERFACE_SETTLE_TIME, NULL);			
		ResiMngKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_RESI_MNG);
//		PostMessage(hWnd, WM_CLOSE, 0, 0);
		KillAllChildWnd(hWnd);		
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_RESI_MNG);
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
CreateResiMngWnd(HWND hwndParent)
{
	static char szAppName[] = "ResiMngWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) ResiMngWndProc;
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
		"ResiMngWnd",			// Window name	(Not NULL)
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
ResiMngKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
			for (i = 0; i < RESIMNGDEPTH; i++)
			{
				if (g_ResiMngInfo[i].bSelected) 
				{
					if(g_ResiMngInfo[i].Menufun)
					{
						g_ResiMngInfo[i].Menufun();					
					}
					break;
				}
			}
		}
		break;
	case KEY_UP:
		{
			for (i = 0; i < RESIMNGDEPTH; i++)
			{
				if ( g_ResiMngInfo[i].bSelected ) 
				{
					if (0 == i) 
					{
						g_ResiMngInfo[i].bSelected = FALSE;
						g_ResiMngInfo[RESIMNGDEPTH-1].bSelected = TRUE;
					} 
					else
					{
						g_ResiMngInfo[i].bSelected = FALSE;
						g_ResiMngInfo[i-1].bSelected = TRUE;
					}
					break;
				}
			}
		}
		
		break;
	case KEY_DOWN:
		{
			for (i = 0; i < RESIMNGDEPTH; i++)
			{
				if ( g_ResiMngInfo[i].bSelected ) 
				{
					if (RESIMNGDEPTH-1 == i) 
					{
						g_ResiMngInfo[i].bSelected = FALSE;
						g_ResiMngInfo[0].bSelected = TRUE;
					} 
					else
					{
						g_ResiMngInfo[i].bSelected = FALSE;
						g_ResiMngInfo[i+1].bSelected = TRUE;
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
AddNormalProc()
{
	g_ASInfo.ASWorkStatus = STATUS_ADD_CARD;	
	CreateAddCardWnd(GetFocus());
}

static VOID 
DelCardProc()
{
	CreateDelModeSelWnd(GetFocus());
}

static VOID 
AddAccreditCardProc()
{
	g_ASInfo.ASWorkStatus = STATUS_ADD_AUTHORIZE_CARD;	
	CreateAddCardWnd(GetFocus());
}

static VOID 
AddPatrolProc()
{
	g_ASInfo.ASWorkStatus = STATUS_ADD_PATROL_CARD;	
	CreateAddCardWnd(GetFocus());
}

