/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	RoomCodeDigitSetWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		05 - Dec - 2008
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
#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"
#include "MenuParaProc.h"
#include "AccessSystemWnd.h"
#include "ModuleTalk.h"
#include "RoomCodeDigitSetWnd.h"
/************** DEFINES **************************************************************/

#define MAX_ROOM_CODE_DIGITS	19
#define MIN_ROOM_CODE_DIGITS	 1

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static void		RoomCodeDigitSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK RoomCodeDigitSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void RoomCodeDigitSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void	ShowRoomCodeDigitSetPromptInfo(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static BOOL bShowPromptInfo = FALSE;
static BYTE RoomCodeDigitIndex = 0;
static BYTE TempVldCodeDigits = 0;

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenDelaySetWndPaint
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
RoomCodeDigitSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	UCHAR Data[3] = { 0 };
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;			

	Data[0] = (BYTE)(TempVldCodeDigits / 10) + 0x30;
	Data[1] = (BYTE)(TempVldCodeDigits % 10) + 0x30;

	DrawDataParaSet(Hdc, (CHAR*)Data, RoomCodeDigitIndex);

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
RoomCodeDigitSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		RoomCodeDigitIndex = 0;
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		TempVldCodeDigits	=	g_SysConfig.VldCodeDigits;
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_ROOM_CODE_DIGIT, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			if (bShowPromptInfo)
			{
				ShowRoomCodeDigitSetPromptInfo(hWnd, Msg, wParam, lParam);
			}
			else
			{
				RoomCodeDigitSetWndPaint(hWnd, Msg, wParam, lParam);
			}
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
		if (!bShowPromptInfo) 
		{
			ResetTimer(hWnd, TIMER_ROOM_CODE_DIGIT, INTERFACE_SETTLE_TIME, NULL);
			RoomCodeDigitSetKeyProcess(hWnd, Msg, wParam, lParam);
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_ROOM_CODE_DIGIT);
//		LoadMenuParaFromMem();
		if (bShowPromptInfo) 
		{
			bShowPromptInfo = FALSE;		
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		else
		{
			KillAllChildWnd(hWnd);		
		}
		
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_ROOM_CODE_DIGIT);
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
CreateRoomCodeDigitSetWnd(HWND hwndParent)
{
	static char szAppName[] = "RoomCodeDigitSetWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) RoomCodeDigitSetWndProc;
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
		"RoomCodeDigitSetWnd",			// Window name	(Not NULL)
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
RoomCodeDigitSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BYTE Data[2]	= { 0 };

	wParam =  KeyBoardMap(wParam);
	if (KEY_RETURN == wParam) 
	{	
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if (KEY_ENTER == wParam) 
	{
		bShowPromptInfo = TRUE;
		g_SysConfig.VldCodeDigits = TempVldCodeDigits;
		SaveMenuPara2Mem();
	}
	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9))
	{
		Data[0] = (BYTE)(TempVldCodeDigits / 10) ;
		Data[1] = (BYTE)(TempVldCodeDigits % 10) ;
		
		Data[RoomCodeDigitIndex] = ((BYTE)wParam - 0x30);
		
		TempVldCodeDigits = 10 * Data[0] +Data[1];
		
		if (TempVldCodeDigits > MAX_ROOM_CODE_DIGITS) 
		{
			TempVldCodeDigits = MAX_ROOM_CODE_DIGITS;
		}
		else if (TempVldCodeDigits < MIN_ROOM_CODE_DIGITS) 
		{
			TempVldCodeDigits = MIN_ROOM_CODE_DIGITS;
		}		
		RoomCodeDigitIndex++;
		if (2 == RoomCodeDigitIndex)
		{
			RoomCodeDigitIndex = 0;
		}
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ShowGateOpenDelaySetPromptInfo
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
ShowRoomCodeDigitSetPromptInfo(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;		
	
	if (SET_CHINESE == g_SysConfig.LangSel) 
	{
		MXDrawText_Cn(Hdc, &Rect, "房间号有效位已修改", 1);
	}
	else if (SET_ENGLISH == g_SysConfig.LangSel) 
	{		
		MXDrawText_En(Hdc, &Rect, "Modified", 1);
	}	
	else if (SET_HEBREW == g_SysConfig.LangSel) 
	{
		MXDrawText_Center(Hdc,GetHebrewStr(HS_ID_MODIFYED), 1);
	}
	
	ResetTimer(hWnd, TIMER_ROOM_CODE_DIGIT, PROMPT_SHOW_TIME, NULL);
	
	EndPaint(hWnd, &ps);
}




