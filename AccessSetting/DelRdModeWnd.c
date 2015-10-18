/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	DelRdModeWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		27 - April - 2009
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
#include "AccessCommon.h"

#include "DelRdModeWnd.h"
#include "AccessProc.h"
#include "BacpNetCtrl.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/


static VOID		DelRdModeTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		DelRdModeWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK DelRdModeWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID DelRdModeKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static DELRDMODESTEP DelRdModeStep =  STEP_ETR_RDNUM;

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddResiTimerProc
**	AUTHOR:			Jeff Wang
**	DATE:			27 - April - 2009
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
DelRdModeTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	KillTimer(hWnd, TIMER_DEL_RDNUM_MODE);
	if (STEP_ETR_RDNUM== DelRdModeStep || STEP_CONFIRM == DelRdModeStep) 
	{
		DelRdModeStep =  STEP_ETR_RDNUM;
		KillAllChildWnd(hWnd);							
	}
	else
	{
		DelRdModeStep =  STEP_ETR_RDNUM;
		ResetTimer(hWnd, TIMER_DEL_RDNUM_MODE, INTERFACE_SETTLE_TIME, NULL);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
//		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	
	DelRdModeStep =  STEP_ETR_RDNUM;
//	PostMessage(hWnd,  WM_CLOSE, 0, 0);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IPSetWndPaint
**	AUTHOR:			Jeff Wang
**	DATE:			27 - April - 2009
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
DelRdModeWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;
	INT							  xPos = 0;
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;

	switch(DelRdModeStep) 
	{
	case STEP_ETR_RDNUM:
		{
#ifdef NEW_OLED_ENABLE	
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos	=	GetXPos(STR_ENTERNUM_EN);
				MXDrawText_Left(Hdc, STR_ENTERRES_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ENTERNUM_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos	=	GetXPos(STR_ENTERRESNUM_CN);
				MXDrawText_Left(Hdc, STR_ENTERRESNUM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERRESNUM), 1);	
			}	
			MulLineDisProc(Hdc, g_KeyInputBuffer, g_KeyInputBufferLen, 2);			
#else
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos	=	GetXPos(STR_ENTERNUM_EN);
				MXDrawText_Left(Hdc, STR_ENTERRES_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_ENTERNUM_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos	=	GetXPos(STR_ENTERRESNUM_CN);
				MXDrawText_Left(Hdc, STR_ENTERRESNUM_CN, xPos, 1);
				MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 2);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERRESNUM), 2);	
			}
			MulLineDisProc(Hdc, g_KeyInputBuffer, g_KeyInputBufferLen, 3);			
#endif
		}
		break;
	case STEP_CONFIRM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
// 				MXDrawText_Center(Hdc, STR_CONFDELETE_EN, 1);
				xPos = GetXPos(STR_AREYOUSURE_EN);
				MXDrawText_Left(Hdc, STR_AREYOUSURE_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_TODELETE_EN, xPos, 2);
				
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
	case STEP_DEL_FAIL:
		{

			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_RDNUMNTEX_EN);
				MXDrawText_Left(Hdc, STR_RDNUM_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_RDNUMNTEX_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_RDNUMNTEX_CN, 2);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_RDNUMNTEX), 2);	
			}
/*
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_RESINUM_EN);
				MXDrawText_Left(Hdc, STR_RESINUM_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_NOTREGISTER_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_ROOMNUMERROR_CN, 1);
			}	
*/		}
		break;
	case STEP_DEL_NO_CARD:
		{
/*
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_RDNUMNTEX_EN);
				MXDrawText_Left(Hdc, STR_RDNUM_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_RDNUMNTEX_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_RDNUMNTEX_CN, 2);
			}
*/
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, PMT_ROOMCODE_NO_CARD_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, PMT_ROOMCODE_NO_CARD_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ROOMCODE_NO_CARD), 1);	
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
**	DATE:			27 - April - 2009
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
DelRdModeWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		ClearKeyBuffer();		
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_DEL_RDNUM_MODE, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			DelRdModeWndPaint(hWnd, Msg, wParam, lParam);
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
		if (STEP_ETR_RDNUM == DelRdModeStep || STEP_CONFIRM == DelRdModeStep)
		{
			DelRdModeKeyProcess(hWnd, Msg, wParam, lParam);
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
		
	case WM_TIMER:
		DelRdModeTimerProc(hWnd, Msg, wParam, lParam);
		ClearKeyBuffer();
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_DEL_RDNUM_MODE);
		ClearKeyBuffer();
		DelRdModeStep =  STEP_ETR_RDNUM;
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
**	DATE:			27 - April - 2009
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
CreateDelRdModeWnd(HWND hwndParent)
{
	static char szAppName[] = "DelRdModeWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) DelRdModeWndProc;
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
		"DelRdModeWnd",			// Window name	(Not NULL)
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
**	FUNCTION NAME:	DelRdModeKeyProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		27 - April - 2009
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
DelRdModeKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int					nCardType = 0;
	EthResolvInfo		ResolvInfo;
	wParam =  KeyBoardMap(wParam);
	memset(&ResolvInfo,0,sizeof(EthResolvInfo));

	if (KEY_RETURN == wParam) 
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if (KEY_ENTER == wParam) 
	{

		if (STEP_ETR_RDNUM == DelRdModeStep) 
		{
			if (0 == strcmp(g_KeyInputBuffer,"00")) 
			{
				DelRdModeStep = STEP_CONFIRM;
				ResetTimer(hWnd, TIMER_DEL_RDNUM_MODE, INTERFACE_SETTLE_TIME, NULL);				
			}
			else
			{
				memcpy(ResolvInfo.szDevCode, g_KeyInputBuffer, g_KeyInputBufferLen);		
				ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_CODE;
				FdFromAMTResolv(&ResolvInfo);
				
				
				
				if (ATM_TYPE_NODEVICE == ResolvInfo.nType)
				{
					DelRdModeStep = STEP_DEL_FAIL;
					ResetTimer(hWnd, TIMER_DEL_RDNUM_MODE, PROMPT_SHOW_TIME, NULL);
					StartPlayErrorANote();
				}
				else if (ATM_TYPE_MC == ResolvInfo.nType  || 
						 ATM_TYPE_EHV == ResolvInfo.nType || 
						 ATM_TYPE_EGM == ResolvInfo.nType)
				{
					if (IsCdInRoom(ResolvInfo.szDevCode)) 
					{
						DelRdModeStep = STEP_CONFIRM;
						ResetTimer(hWnd, TIMER_DEL_RDNUM_MODE, INTERFACE_SETTLE_TIME, NULL);
					}
					else
					{
						DelRdModeStep = STEP_DEL_NO_CARD;
						ResetTimer(hWnd, TIMER_DEL_RDNUM_MODE, PROMPT_SHOW_TIME, NULL);
						StartPlayErrorANote();
					}
				}
				else
				{
					DelRdModeStep = STEP_DEL_NO_CARD;
					ResetTimer(hWnd, TIMER_DEL_RDNUM_MODE, PROMPT_SHOW_TIME, NULL);
					StartPlayErrorANote();
				}
			}
		}
		else if (STEP_CONFIRM == DelRdModeStep) 
		{
			nCardType=AsRmCdByRdNum(g_KeyInputBuffer, g_KeyInputBufferLen);
			if(nCardType == TYPE_NORMAL_CARD)
			{
				SaveCdInfo2Mem();
				DelRdModeStep = STEP_DEL_SUCCESS;
				ResetTimer(hWnd, TIMER_DEL_RDNUM_MODE, PROMPT_SHOW_TIME, NULL);
			}
			else
			{
				DelRdModeStep = STEP_DEL_NO_CARD;
				ResetTimer(hWnd, TIMER_DEL_RDNUM_MODE, PROMPT_SHOW_TIME, NULL);
				StartPlayErrorANote();
			}
			
		}
/*
		memcpy(ResolvInfo.szDevCode, g_KeyInputBuffer, g_KeyInputBufferLen);		
		ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_CODE;
		FdFromAMTResolv(&ResolvInfo);
				
		if (ATM_TYPE_MC == ResolvInfo.nType || ATM_TYPE_EHV == ResolvInfo.nType)
		{
			if (AsRmCdByRdNum(g_KeyInputBuffer, g_KeyInputBufferLen))
			{
				SaveCdInfo2Mem();
				DelRdModeStep = STEP_DEL_SUCCESS;
			}
			else
			{
				DelRdModeStep = STEP_DEL_NO_CARD;
			}
		}
		else
		{
			DelRdModeStep = STEP_DEL_FAIL;
		}				
*/
	}
	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9))
	{
		CircleBufEnter(g_KeyInputBuffer, &g_KeyInputBufferLen, KEY_BUF_LEN, (CHAR)wParam);
		ResetTimer(hWnd, TIMER_DEL_RDNUM_MODE, INTERFACE_SETTLE_TIME, NULL);
	}

				
}


