
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	HelpInfoWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		5 - sep - 2008
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
/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXCommon.h"
#include "MenuParaProc.h"
#include "HelpInfoWnd.h"

/************** DEFINES **************************************************************/

#define  HELPINFODISCOUNT 4
#define  HELPINFOPERSISTTIME 5000

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//extern void	DecodePhoto(unsigned char* pDisBuf);

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static void HelpInfoWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK HelpInfoWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
INT g_repainttimeout = 0;
/*************************************************************************************/

void
CreateHelpInfoWnd(HWND hwndParent)
{
	static char szAppName[] = "CreateHelpInfoWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) HelpInfoWndProc;
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
		"CreateHelpInfoWnd",			// Window name	(Not NULL)
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
    g_repainttimeout = GetTickCount();
}


static void
HelpInfoWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC				Hdc;
	PAINTSTRUCT		ps;
	RECT			Rect;
	CHAR*			pPicName = "/mox/rdwr/HelpInfo.bmp";
	INT							  xPos = 0;
	if(GetTickCount() - g_repainttimeout > 200) 
	{
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);	

	if (g_SysConfig.bHelpInfoDld)
	{
		GdDrawImageFromFile(Hdc->psd, 
			Rect.left, 
			Rect.top, 
			SCREEN_WIDTH, 
			SCREEN_HEIGHT, 
			pPicName, 
			0);	
	}
	else
	{
		Hdc->bkcolor = BACKGROUND_COLOR;
		Hdc->textcolor = FONT_COLOR;
		
		if (SET_ENGLISH == g_SysConfig.LangSel)
		{
			xPos = GetXPos("Enter 00 to Call MC");
			MXDrawText_Left(Hdc, "Enter Number to", xPos, 0);			
			MXDrawText_Left(Hdc, "Call Resident", xPos, 1);			
			MXDrawText_Left(Hdc, "Enter 00 to Call MC", xPos, 3);			
//			MXDrawText_En(Hdc, &Rect, "Enter Number to ", 0);
//			MXDrawText_En(Hdc, &Rect, "Call Resident,", 1);
//			MXDrawText_En(Hdc, &Rect, "Enter 00 to", 2);
//			MXDrawText_En(Hdc, &Rect, "Call MC", 3);	
		}
		else if (SET_CHINESE == g_SysConfig.LangSel)
		{
			xPos = GetXPos("呼叫住户请直接按");
			MXDrawText_Left(Hdc, "呼叫住户请直接按", xPos, 0);			
			MXDrawText_Left(Hdc, "房间号", xPos, 1);			
			MXDrawText_Left(Hdc, "呼叫管理中心请按", xPos, 2);			
			MXDrawText_Left(Hdc, "00", xPos, 3);
			
//			MXDrawText_Cn(Hdc, &Rect, "呼叫住户请直接按", 0);
//			MXDrawText_Cn(Hdc, &Rect, "房间号", 1);
//			MXDrawText_Cn(Hdc, &Rect, "呼叫管理中心请按", 2);
//			MXDrawText_Cn(Hdc, &Rect, "00", 3);
		}
		else if(SET_HEBREW == g_SysConfig.LangSel)
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_CALLPRESSCODE));
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_CALLPRESSCODE), xPos, 0);	
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_CALLPRESS00), xPos, 2);	
		}
	}	
	EndPaint(hWnd, &ps);
    g_repainttimeout = GetTickCount();	
	}
}



static LRESULT CALLBACK
HelpInfoWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{   
	switch (Msg)
	{
	case WM_CREATE:
		PostMessage(hWnd,  WM_SETTIMER, HELPINFOPERSISTTIME, 0);		
		break;
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_HELPMSG, wParam, NULL);
		break;

	case WM_PAINT:	
		if (GetFocus() == hWnd)
		{
			HelpInfoWndPaint(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		wParam =  KeyBoardMap(wParam);
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		break;

	case WM_TIMER:
		KillTimer(hWnd, TIMER_HELPMSG);
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		break;
	case WM_REFRESHPARENT_WND:
		SetFocus(hWnd);
		ResetTimer(hWnd, WM_SETTIMER, HELPINFOPERSISTTIME, NULL);
		HelpInfoWndPaint(hWnd, Msg, wParam, lParam);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_HELPMSG);
//		PostMessage(GetParent(hWnd),  WM_REFRESHPARENT_WND, 0, 0);
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





