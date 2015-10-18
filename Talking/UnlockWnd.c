/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	UnlockWnd.c
**
**	AUTHOR:		Mike Zhang
**
**	DATE:		28 - Jan - 2011
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				
**								
**	NOTES:
** 
*/
#include <windows.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <MoxFB.h>
#include <device.h>

/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"
#include "ModuleTalk.h"
#include "BacpNetCtrl.h"
#include "Multimedia.h"
#include "TalkEventWnd.h"
#include "UnlockWnd.h"
#include "MenuParaProc.h"
#include "ASProcWnd.h"
//#include "AsAlarmWnd.h"
//#include "SecurityAlarm.h"

/************** DEFINES **************************************************************/
#define DEBUG_ULKWND

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/
extern HWND	g_HwndAsAlarm;

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/
HWND g_hWNDUlk = NULL;

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UlkWndPaint
**	AUTHOR:			Mike Zhang
**	DATE:			28 - Jan - 2011
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
UlkWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC				Hdc;
	PAINTSTRUCT		ps;
	RECT			Rect;	
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;
	Hdc->textcolor = FONT_COLOR;
	
	if (SET_CHINESE == g_SysConfig.LangSel) 
	{	
		MXDrawText_Center(Hdc, STR_GATEUNLOCK_CN, 1);
	}
	else if (SET_ENGLISH == g_SysConfig.LangSel) 
	{
		MXDrawText_Center(Hdc, STR_GATEUNLOCK_EN, 1);	
	}
	else if (SET_HEBREW == g_SysConfig.LangSel) 
	{
		MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_GATEUNLOCK), 1);
	}	
	EndPaint(hWnd, &ps);
}


static LRESULT CALLBACK
UlkWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		break;
		
	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			UlkWndPaint(hWnd, Msg, wParam, lParam);
		}
		break;
		
	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		break;
		
	case WM_TIMER:
		break;

	case WM_REFRESHPARENT_WND:
		SetFocus(hWnd);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_DESTROY:
		ClearKeyBuffer();
		g_hWNDUlk = NULL;
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

void
CreateUlkWnd(HWND hwndParent)
{
	static char szAppName[] = "Unlock";
	HWND		hWnd;
	WNDCLASS	WndClass;

	if (NULL != g_HwndAsAlarm) 
	{
		return;
	}
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) UlkWndProc;
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
		0L,										// Extend style	(0)
		szAppName,								// Class name	(NULL)
		"UlkWnd",								// Window name	(Not NULL)
		WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,	// Style		(0)
		0,										// x			(0)
		0,										// y			(0)
		SCREEN_WIDTH,							// Width		
		SCREEN_HEIGHT,							// Height
		g_hMainWnd /*hwndParent*/,								// Parent		(MwGetFocus())
		NULL,									// Menu			(NULL)
		NULL,									// Instance		(NULL)
		NULL);									// Parameter	(NULL)	
	

	AddOneWnd(hWnd,WND_UNLOCK_PRIORITY_4);
	if (hWnd == GetFocus())
	{
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
	}
	else
	{
		ShowWindow(hWnd, SW_HIDE);
	}
	
#ifdef DEBUG_ULKWND
	printf("Ulk Wnd create successfully\n");
#endif
	g_hWNDUlk = hWnd;
}


void
ShowUlkWnd(void)
{
	if (g_hWndAS)
	{
	//if(ST_MT_MONITOR == g_TalkInfo.monitor.dwMonState)
		SendMessage(g_hWndAS, WM_CLOSE, 0, 0);
	}
	if (g_hWNDUlk) 
	{
		g_TalkInfo.unlock.dwUnLockShowState = ST_UK_DELAY;
		g_TalkInfo.Timer.dwUnlockShowTimer = GetTickCount();
		SendMessage(g_hWNDUlk, WM_PAINT, 0, 0);
	}
	else
	{
		g_TalkInfo.unlock.dwUnLockShowState = ST_UK_DELAY;
		g_TalkInfo.Timer.dwUnlockShowTimer = GetTickCount();
		CreateUlkWnd(GetFocus());
	}
}

void
HideUlkWnd(void)
{
	if (g_hWNDUlk) 
	{
		SendMessage(g_hWNDUlk, WM_CLOSE, 0, 0);
	}
}



