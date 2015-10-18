/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	DelCardModeWnd.c
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
#include "MXMem.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"

#include "AccessCommon.h"
#include "DelCardModeWnd.h"
#include "CardProc.h"
#include "AccessProc.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

typedef enum _DELCARDMODSTEP
{
	STEP_STAMPCARD = 1,
	STEP_CARDHASDEL,
	STEP_CONFIRM,
	STEP_CARDDELSUCCESS
}DELCARDMODSTEP;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/


//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/


static TIMERPROC CALLBACK	DelCardModeTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		DelCardModeWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK DelCardModeWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		DelCardModeKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static DELCARDMODSTEP DelCardModeStep = STEP_STAMPCARD;

static CSNBuf[10] = {0};

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddResiTimerProc
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

static TIMERPROC CALLBACK 
DelCardModeTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{	
	if (STEP_CARDDELSUCCESS == DelCardModeStep || STEP_CARDHASDEL== DelCardModeStep)
	{
		DelCardModeStep = STEP_STAMPCARD;
		ResetTimer(hWnd, TIMER_DEL_CARD_MODE, INTERFACE_SETTLE_TIME, NULL);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
	}
	else
	{
//		if (STEP_CARDHASDEL== DelCardModeStep) 
//		{
//			PostMessage(hWnd,  WM_CLOSE, 0, 0);
//		}
//		else
//		{
			DelCardModeStep = STEP_STAMPCARD;
			KillAllChildWnd(hWnd);		
			//		}
	}
	return 0;
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
DelCardModeWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;
	INT							  xPos	=	0;
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;
	
	switch(DelCardModeStep)
	{
	case STEP_STAMPCARD:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_SWIPECARD_EN);
				MXDrawText_Left(Hdc, STR_SWIPECARD_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_PRESSEXIT_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_PRESSEXIT_CN);
				MXDrawText_Left(Hdc, STR_SWIPECARD_CN, xPos, 1);
				MXDrawText_Left(Hdc, STR_PRESSEXIT_CN, xPos, 2);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_SWIPECARD));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_SWIPECARD), xPos, 1);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_PRESSEXIT), xPos, 2);
			}	
		}
		break;

	case STEP_CARDHASDEL:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CARDHASDELETED_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CARDHASDELETED_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CARDHASDELETED), 1);
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


	case STEP_CARDDELSUCCESS:
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
DelCardModeWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		memset(CSNBuf,0,10);
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);	
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_DEL_CARD_MODE, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			DelCardModeWndPaint(hWnd, Msg, wParam, lParam);
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
		if (STEP_STAMPCARD == DelCardModeStep || STEP_CONFIRM == DelCardModeStep)
		{
			ResetTimer(hWnd, TIMER_DEL_CARD_MODE, INTERFACE_SETTLE_TIME, NULL);
			DelCardModeKeyProcess(hWnd, Msg, wParam, lParam);
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
		
	case WM_TIMER:
		DelCardModeTimerProc(hWnd, Msg, wParam, lParam);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_DEL_CARD_MODE);
		DelCardModeStep = STEP_STAMPCARD;
		g_ASInfo.ASWorkStatus = STATUS_OPEN;
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
CreateDelCardModeWnd(HWND hwndParent)
{
	static char szAppName[] = "DelCardModeWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) DelCardModeWndProc;
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
		"DelCardModeWnd",			// Window name	(Not NULL)
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
**	FUNCTION NAME:	IPSetKeyProcess
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

static void 
DelCardModeKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int nCardType = -1;
	wParam =  KeyBoardMap(wParam);

	switch(wParam) 
	{
	case KEY_RETURN:
		{
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
			g_ASInfo.ASWorkStatus = STATUS_OPEN;
			DelCardModeStep = STEP_STAMPCARD;			
		}
		break;
	case KEY_ENTER:
		{
			if (STEP_CONFIRM == DelCardModeStep) 
			{
				nCardType = RmCard(CSNBuf);
				SaveCardInfo2Mem(nCardType);
				DelCardModeStep = STEP_CARDDELSUCCESS;
				StartPlayRightANote();	
				memset(CSNBuf,0,10);				
				ResetTimer(GetFocus(), TIMER_DEL_CARD_MODE, PROMPT_SHOW_TIME, NULL);
			}
/*
			else
			{
				SaveCdInfo2Mem();
				PostMessage(hWnd,  WM_CLOSE, 0, 0);
				g_ASInfo.ASWorkStatus = STATUS_OPEN;
				DelCardModeStep = STEP_STAMPCARD;
			}
*/
			
		}
		break;
		
	default:
		break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ShowGateOpenDelaySetPromptInfo
**	AUTHOR:			Jeff Wang
**	DATE:			10 - Sep - 2008
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
ShowDelCardModWnd(BYTE* pCSN)
{
	if (STEP_CONFIRM == DelCardModeStep) 
	{
		return;
	}

	
	if (IsValidCard(pCSN)/*AsRmCd(pCSN)*/)
	{	
		memcpy(CSNBuf,pCSN,5);
		DelCardModeStep = STEP_CONFIRM;
		ResetTimer(GetFocus(), TIMER_DEL_CARD_MODE, INTERFACE_SETTLE_TIME, NULL);
		StartPlayRightANote();			
//		SaveCdInfo2Mem();
//		DelCardModeStep = STEP_CARDDELSUCCESS;
//		StartPlayRightANote();
	}
	else
	{
		DelCardModeStep = STEP_CARDHASDEL;
		StartPlayErrorANote();
		ResetTimer(GetFocus(), TIMER_DEL_CARD_MODE, PROMPT_SHOW_TIME, NULL);
	}
	PostMessage(GetFocus(),  WM_PAINT, 0, 0);

}

