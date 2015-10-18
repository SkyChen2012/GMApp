/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	DelLocalNumModeWnd.c
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
#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"

#include "DelLocalNumModeWnd.h"
#include "AccessProc.h"
#include "AddCardWnd.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/


static VOID		DelLocalNumModeTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		DelLocalNumModeWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK DelLocalNumModeWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID DelLocalNumModeKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static DELLOCALNUMMODESTEP DelLocalNumModeStep =  STEP_ETR_LOCALNUM;
static	BYTE	DataIndex = 0;

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddLocalNumModeTimerProc
**	AUTHOR:			Jeff Wang
**	DATE:			24 - Aug - 2008
**
**	DESCRIPTION:	
**			Process SysCon Menu Process Timer
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/

static VOID 
DelLocalNumModeTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	if (STEP_ETR_LOCALNUM == DelLocalNumModeStep || STEP_CONFIRM == DelLocalNumModeStep) 
	{
		DelLocalNumModeStep =  STEP_ETR_LOCALNUM;
		KillAllChildWnd(hWnd);		
		KillTimer(hWnd, TIMER_DEL_LOCALNUM_MODE);
		ClearKeyBuffer();
	}
	else
	{
		DataIndex = 0;
		memset(g_KeyInputBuffer,0,KEY_BUF_LEN);
		strcpy(g_KeyInputBuffer, "00000");		
		DelLocalNumModeStep =  STEP_ETR_LOCALNUM;
		ResetTimer(hWnd, TIMER_DEL_LOCALNUM_MODE, INTERFACE_SETTLE_TIME, NULL);		
//		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
	}

	
//	PostMessage(hWnd,  WM_CLOSE, 0, 0);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IPSetWndPaint
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
DelLocalNumModeWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	INT							   xPos = 0;
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;

	switch(DelLocalNumModeStep) 
	{
	case STEP_ETR_LOCALNUM:
		{
#ifdef NEW_OLED_ENABLE	
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos	=	GetXPos(STR_ETR_LOCAL_EN);
				MXDrawText_Left(Hdc, STR_ETR_LOCAL_EN, xPos, 0);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos	=	GetXPos(STR_ETR_LOCOALNUM_CN);
				MXDrawText_Left(Hdc, STR_ETR_LOCOALNUM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERLOCID), 1);
			}
			
			DrawDataParaSet(Hdc, (CHAR*)g_KeyInputBuffer, DataIndex);
#else
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos	=	GetXPos(STR_ETR_LOCAL_EN);
				MXDrawText_Left(Hdc, STR_ETR_LOCAL_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos	=	GetXPos(STR_ETR_LOCOALNUM_CN);
				MXDrawText_Left(Hdc, STR_ETR_LOCOALNUM_CN, xPos, 1);
				MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 2);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERLOCID), 1);
			}
			DrawDataParaSet(Hdc, (CHAR*)g_KeyInputBuffer, DataIndex);
#endif
		}
		break;

	case STEP_LOCALNUM_NOTEXIST:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos	=	GetXPos(STR_IDNOTREGISTERED_EN);
				MXDrawText_Left(Hdc, STR_IDNOTREGISTERED_EN, xPos, 1);
//				MXDrawText_Left(Hdc, STR_NOTREGISTERED_EN, xPos,2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_ID_NOTREGISTERED_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ID_NOTREGISTERED), 1);
			}
			
		}
		break;
	case STEP_CONFIRM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_AREYOUSURE_EN);
				MXDrawText_Left(Hdc, STR_AREYOUSURE_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_TODELETE_EN, xPos, 2);
//				MXDrawText_Center(Hdc, STR_CONFDELETE_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CONFDELETE_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CONFDELETE), 1);
			}
			
		}
		break;
	case STEP_DEL_SUCCESS:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CARDDELETED_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CARDDELETED_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CARDDELETED), 1);
			}
			
		}
		break;

	default:
		break;
	}
	
	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IPSetWndProc
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
DelLocalNumModeWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		DataIndex = 0;
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		strcpy(g_KeyInputBuffer, "00000");		
		break;	
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_DEL_LOCALNUM_MODE, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			DelLocalNumModeWndPaint(hWnd, Msg, wParam, lParam);
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
		if (STEP_ETR_LOCALNUM == DelLocalNumModeStep || STEP_CONFIRM == DelLocalNumModeStep)
		{
			ResetTimer(hWnd, TIMER_DEL_LOCALNUM_MODE, INTERFACE_SETTLE_TIME, NULL);		
			DelLocalNumModeKeyProcess(hWnd, Msg, wParam, lParam);
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
		
	case WM_TIMER:
		DelLocalNumModeTimerProc(hWnd, Msg, wParam, lParam);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_DEL_LOCALNUM_MODE);
		ClearKeyBuffer();
		DelLocalNumModeStep =  STEP_ETR_LOCALNUM;
		g_ASInfo.ASWorkStatus = STATUS_OPEN;
		DataIndex	=	0;
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
VOID
CreateDelLocalNumModeWnd(HWND hwndParent)
{
	static char szAppName[] = "DelLocalNumModeWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) DelLocalNumModeWndProc;
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
		"DelLocalNumModeWnd",			// Window name	(Not NULL)
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
**	FUNCTION NAME:	DelLocalNumModeKeyProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		21 - Aug - 2008
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
DelLocalNumModeKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	wParam =  KeyBoardMap(wParam);

	if (KEY_RETURN == wParam) 
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if (KEY_ENTER == wParam) 
	{
		if (STEP_ETR_LOCALNUM == DelLocalNumModeStep) 
		{
			if (IsLocalNumExist(ConvertStr2ID(g_KeyInputBuffer))) 
			{
				DelLocalNumModeStep = STEP_CONFIRM;
				ResetTimer(hWnd, TIMER_DEL_LOCALNUM_MODE, INTERFACE_SETTLE_TIME, NULL);		
			}
			else
			{
				DelLocalNumModeStep = STEP_LOCALNUM_NOTEXIST;
				ResetTimer(hWnd, TIMER_DEL_LOCALNUM_MODE, PROMPT_SHOW_TIME, NULL);
				StartPlayErrorANote();
			}
		}
		else if(STEP_CONFIRM == DelLocalNumModeStep)
		{
			AsRmCdbyLocalNum(ConvertStr2ID(g_KeyInputBuffer));
			DelLocalNumModeStep = STEP_DEL_SUCCESS;
			ResetTimer(hWnd, TIMER_DEL_LOCALNUM_MODE, PROMPT_SHOW_TIME, NULL);
		}
/*
		if (AsRmCdbyLocalNum(ConvertStr2ID(g_KeyInputBuffer)))
		{
			SaveCdInfo2Mem();
			DelLocalNumModeStep = STEP_DEL_SUCCESS;
		}
		else
		{
			DelLocalNumModeStep = STEP_LOCALNUM_NOTEXIST;
		}
*/
	}
	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9))
	{
		CircleKeyEnter(g_KeyInputBuffer, &DataIndex, CD_LOCALNUM_LEN-1, (CHAR)wParam);		
	}
}


