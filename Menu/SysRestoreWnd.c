/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SysRestoreWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		27 - Sep - 2008
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

#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"
#include "AccessSystemWnd.h"
#include "ModuleTalk.h"
#include "SysRestoreWnd.h"


static bConfirmAgain = FALSE;

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysRestoreWndPaint
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
SysRestoreWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;
	INT xPos = 0;
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;
	
	if (bConfirmAgain) 
	{
		if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("���������˳�");

			MXDrawText_Left(Hdc, "ȷ����?", xPos, 0);
			MXDrawText_Left(Hdc, "��#ȷ��", xPos, 1);
			MXDrawText_Left(Hdc, "���������˳�", xPos, 2);
		}
		else if (SET_ENGLISH == g_SysConfig.LangSel)
		{
			xPos = GetXPos("Other keys to Return");
			MXDrawText_Left(Hdc, "Sure?", xPos, 0);
			MXDrawText_Left(Hdc, "Press # to restore", xPos, 1);
			MXDrawText_Left(Hdc, "Other keys to Return", xPos, 2);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel)
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_PRESSOTHER));
			MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_SURE), xPos, 0);
			MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_CONFIRM), xPos, 1);
			MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_PRESSOTHER), xPos, 2);
		}
	}
	else
	{
		if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("������������");
			MXDrawText_Left(Hdc, "��#ϵͳ��ԭ", xPos, 1);
			MXDrawText_Left(Hdc, "������������", xPos, 2);
		}
		else if (SET_ENGLISH == g_SysConfig.LangSel)
		{
			xPos = GetXPos("Other Keys to Return");
			MXDrawText_Left(Hdc, "Press # to Restore", xPos, 1);
			MXDrawText_Left(Hdc, "Other Keys to Return", xPos, 2);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel)
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_PRESSOTHER2));
			MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_RESTORE), xPos, 1);
			MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_PRESSOTHER2), xPos, 2);
		}
	}
		
	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysInfoWndProc
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
SysRestoreWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{ 
	switch (Msg)
	{
	case WM_CREATE:
		bConfirmAgain = FALSE;
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_SYS_RESTORE, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			SysRestoreWndPaint(hWnd, Msg, wParam, lParam);
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
		ResetTimer(hWnd, TIMER_SYS_RESTORE, INTERFACE_SETTLE_TIME, NULL);		

		wParam =  KeyBoardMap(wParam);

		
		
		if (KEY_ENTER == wParam) 
		{
			if (!bConfirmAgain) 
			{
				bConfirmAgain = TRUE;
				PostMessage(hWnd,  WM_PAINT, 0, 0);
			}
			else
			{
                system("mount -n -o remount rw /mox");
                system("mv /mox/rdwr/DI1.pcm /mox");
                system("mv /mox/rdwr/DI2.pcm /mox");
				system("rm -f /mox/rdwr/*");
                system("mv /mox/DI1.pcm /mox/rdwr");
                system("mv /mox/DI2.pcm /mox/rdwr");
                system("ls /mox/rdwr/");
				system("reboot");
				while (1);
			}
		}
		else
		{
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_SYS_RESTORE);
		KillAllChildWnd(hWnd);		
		//		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_SYS_RESTORE);
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
**	FUNCTION NAME:	CreateSysConMenuWnd
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
void
CreateSysRestoreWnd(HWND hwndParent)
{
	static char szAppName[] = "SysRestoreWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) SysRestoreWndProc;
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
		"SysRestoreWnd",			// Window name	(Not NULL)
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

