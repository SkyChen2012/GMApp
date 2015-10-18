/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	GatePulseWidthSetWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		03 - Dec - 2008
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

#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"

#include "GatePulseWidthSetWnd.h"

/************** DEFINES **************************************************************/

#define MAX_GATE_PULSE_WIDTH_TIME 30000
#define MIN_GATE_PULSE_WIDTH_TIME 200

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		GatePulseWidthSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK GatePulseWidthSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID GatePulseWidthSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID	ShowGatePulseWidthSetPromptInfo(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static BOOL bShowPromptInfo = FALSE;
static UCHAR PulseWidthIndex = 0;

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
static VOID
GatePulseWidthSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	UCHAR DlyTimeNum[6] = { 0 };
	CHAR	pBuf[TITLE_BUF_LEN] = { 0 };
	DWORD nTemp			=	0;

	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);	
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;		

	nTemp				=	g_ASPara.RelayPulseWith;
	DlyTimeNum[0] = (UCHAR)(nTemp/10000) + 0x30;
	nTemp				= nTemp % 10000;
	DlyTimeNum[1] = (UCHAR)(nTemp/1000) + 0x30;
	nTemp				= nTemp % 1000;
	DlyTimeNum[2] = (UCHAR)(nTemp/100) + 0x30;
	nTemp				= nTemp % 100;
	DlyTimeNum[3] = (UCHAR)(nTemp/10) + 0x30;
	nTemp				= nTemp % 10;
	DlyTimeNum[4] = (UCHAR)(nTemp) + 0x30;	

	strcpy(pBuf, (char*)DlyTimeNum);

	if (SET_ENGLISH == g_SysConfig.LangSel) 
	{	
		strcat(pBuf, "ms");	
	}
	else if (SET_CHINESE == g_SysConfig.LangSel) 
	{	
		strcat(pBuf, "ºÁÃë");
	}
	if (SET_HEBREW == g_SysConfig.LangSel) 
	{	
		strcat(pBuf, "ms");	
	}
	DrawDataParaSet(Hdc, pBuf, PulseWidthIndex);
	
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
GatePulseWidthSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		PulseWidthIndex = 0;
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_GATE_PULSE_WIDTH, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			if (bShowPromptInfo)
			{
				bShowPromptInfo = FALSE;
				ShowGatePulseWidthSetPromptInfo(hWnd, Msg, wParam, lParam);
			}
			else
			{
				GatePulseWidthSetWndPaint(hWnd, Msg, wParam, lParam);
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
		ResetTimer(hWnd, TIMER_GATE_PULSE_WIDTH, INTERFACE_SETTLE_TIME, NULL);
		GatePulseWidthSetKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_GATE_PULSE_WIDTH);
		LoadMenuParaFromMem();
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_GATE_PULSE_WIDTH);
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
VOID
CreatePulseWidthSetWnd(HWND hwndParent)
{
	static char szAppName[] = "GatePulseWidthSetWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) GatePulseWidthSetWndProc;
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
		"GatePulseWidthSetWnd",			// Window name	(Not NULL)
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

static VOID 
GatePulseWidthSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	UCHAR DlyTimeNum[6]	= { 0 };
	DWORD nTemp			=	0;
	static BOOL FirstEnter = TRUE;
	static DWORD StoreGatePulseWidthTime = 0;

	wParam =  KeyBoardMap(wParam);

	if (KEY_RETURN == wParam) 
	{
		if (!FirstEnter)
		{
			g_ASPara.RelayPulseWith = StoreGatePulseWidthTime;
		}
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		FirstEnter = TRUE;
	}
	else if (KEY_ENTER == wParam) 
	{
		bShowPromptInfo = TRUE;
		FirstEnter		= TRUE;
		SaveMenuPara2Mem();
	}
	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9))
	{
		if (FirstEnter)
		{
			StoreGatePulseWidthTime = g_ASPara.RelayPulseWith;
			FirstEnter = FALSE;
		}
		
		nTemp				=	g_ASPara.RelayPulseWith;
		DlyTimeNum[0] = (UCHAR)(nTemp/10000) ;
		nTemp				= nTemp % 10000;
		DlyTimeNum[1] = (UCHAR)(nTemp/1000) ;
		nTemp				= nTemp % 1000;
		DlyTimeNum[2] = (UCHAR)(nTemp/100) ;
		nTemp				= nTemp % 100;
		DlyTimeNum[3] = (UCHAR)(nTemp/10) ;
		nTemp				= nTemp % 10;
		DlyTimeNum[4] = (UCHAR)(nTemp) ;	
		
		DlyTimeNum[PulseWidthIndex] = ((unsigned char)wParam - 0x30);
		
		g_ASPara.RelayPulseWith =	10000* DlyTimeNum[0]
															+1000* DlyTimeNum[1]
															+100* DlyTimeNum[2]
															+10* DlyTimeNum[3]
															+DlyTimeNum[4];
		
		if (g_ASPara.RelayPulseWith > MAX_GATE_PULSE_WIDTH_TIME) 
		{
			g_ASPara.RelayPulseWith = MAX_GATE_PULSE_WIDTH_TIME;
		}
		else if (g_ASPara.RelayPulseWith < MIN_GATE_PULSE_WIDTH_TIME) 
		{
			g_ASPara.RelayPulseWith = MIN_GATE_PULSE_WIDTH_TIME;
		}		
		PulseWidthIndex++;
		if (5 == PulseWidthIndex)
		{
			PulseWidthIndex = 0;
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
static VOID
ShowGatePulseWidthSetPromptInfo(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
		MXDrawText_Center(Hdc, "ÃÅ´ÅÂö³å¿í¶ÈÒÑÐÞ¸Ä", 1);
	}
	else if (SET_ENGLISH == g_SysConfig.LangSel) 
	{
		MXDrawText_Center(Hdc, "Modified", 1);
	}
	else if (SET_HEBREW == g_SysConfig.LangSel) 
	{
		MXDrawText_Center(Hdc, GetHebrewStr( HS_ID_GATEPULSEWIDTHMODFYED), 1);	
	}	
	
	bShowPromptInfo = FALSE;

	ResetTimer(hWnd, TIMER_GATE_PULSE_WIDTH, PROMPT_SHOW_TIME, NULL);

	EndPaint(hWnd, &ps);
}




