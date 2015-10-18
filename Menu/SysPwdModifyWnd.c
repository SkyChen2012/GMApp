/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SysPwdModifyWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		21 - Aug - 2008
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
#include "SysPwdModifyWnd.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

typedef enum _PWDMODSTEP
{
	STEP_OLD_PASSWORD,
	STEP_OLD_PASSWORD_ERROR,
	STEP_NEW_PASSWORD,
	STEP_NEW_PASSWORD_DIGIT_ERROR,
	STEP_NEW_PASSWORD_AGAIN,
	STEP_PASSWORD_MOD_SUCCESS,
	STEP_NEW_PASSWORD_ERROR
}SYSPWDMODSTEP;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/


static VOID		SysPwdModifyTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		SysPwdModifyWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK SysPwdModifyWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		SysPwdModifyKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static	SYSPWDMODSTEP	PwdModStep = STEP_OLD_PASSWORD;

static UCHAR StoreSysPwd[RD_CODE_LEN] = { 0 };

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IPSetTimerProc
**	AUTHOR:			Jeff Wang
**	DATE:			21 - Aug - 2008
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
SysPwdModifyTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (STEP_OLD_PASSWORD_ERROR == PwdModStep) 
	{
		ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, INTERFACE_SETTLE_TIME, NULL);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		PwdModStep = STEP_OLD_PASSWORD;
	}
	
	else if (STEP_PASSWORD_MOD_SUCCESS == PwdModStep) 
	{
		ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, INTERFACE_SETTLE_TIME, NULL);
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if (STEP_NEW_PASSWORD_ERROR == PwdModStep)
	{
		ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, INTERFACE_SETTLE_TIME, NULL);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		PwdModStep = STEP_NEW_PASSWORD;
	}
	else if (STEP_NEW_PASSWORD_DIGIT_ERROR == PwdModStep) 
	{
		ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, INTERFACE_SETTLE_TIME, NULL);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		PwdModStep = STEP_NEW_PASSWORD;
	}
	else
	{
//		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
		g_KeyInputBufferLen = 0;
		KillAllChildWnd(hWnd);		
		
	}
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
static void
SysPwdModifyWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	char DisBuffer[19] = {0};
	INT xPos = 0;

	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;		
	
	if (SET_CHINESE == g_SysConfig.LangSel) 
	{
		if (STEP_OLD_PASSWORD == PwdModStep) 
		{
			xPos = GetXPos(STR_ETROLDPWD_CN);
			MXDrawText_Left(Hdc, STR_ETROLDPWD_CN, xPos,0);
			MXDrawText_Left(Hdc,STR_PRESSENTER_CN, xPos, 1);
			memset(DisBuffer, 42, 19);
			MulLineDisProc(Hdc, DisBuffer, g_KeyInputBufferLen, 2);
		}
		else if (STEP_OLD_PASSWORD_ERROR == PwdModStep) 
		{
			MXDrawText_Center(Hdc, STR_OLDPWDINCORRECT_CN, 1);
		}
		else if (STEP_NEW_PASSWORD == PwdModStep) 
		{
			xPos = GetXPos(STR_ETRNEWPWD_CN);
			MXDrawText_Left(Hdc, STR_ETRNEWPWD_CN, xPos,0);
			MXDrawText_Left(Hdc,STR_PRESSENTER_CN, xPos, 1);
			memset(DisBuffer, 42, 19);
			MulLineDisProc(Hdc, DisBuffer, g_KeyInputBufferLen, 2);
		}

		else if (STEP_NEW_PASSWORD_DIGIT_ERROR == PwdModStep)
		{
			MXDrawText_Center(Hdc, STR_DIGITSERROR_CN, 1);
		}
		
		else if (STEP_NEW_PASSWORD_AGAIN == PwdModStep) 
		{
			xPos = GetXPos(STR_ETRNEWPWDAGAIN_CN);
			MXDrawText_Left(Hdc, STR_ETRNEWPWDAGAIN_CN, xPos,0);
			MXDrawText_Left(Hdc,STR_PRESSENTER_CN, xPos, 1);
			memset(DisBuffer, 42, 19);
			MulLineDisProc(Hdc, DisBuffer, g_KeyInputBufferLen, 2);
		}
		else if (STEP_PASSWORD_MOD_SUCCESS == PwdModStep) 
		{
			MXDrawText_Center(Hdc, STR_PWDMODIFY_CN, 1);
		}
		else if (STEP_NEW_PASSWORD_ERROR == PwdModStep) 
		{
			xPos = GetXPos(STR_NOTIDL_CN);
			MXDrawText_Left(Hdc, STR_NOTIDL_CN, xPos,1);
			MXDrawText_Left(Hdc,STR_ETRPWDAGAIN_CN, xPos, 2);
		}		
	}
	else if (SET_HEBREW == g_SysConfig.LangSel) 
	{
		if (STEP_OLD_PASSWORD == PwdModStep) 
		{
			//xPos = GetXPos(GetHebrewStr(HS_ID_ENTEROLDPSW));
			MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTEROLDPSW),1);
			//MXDrawText_Left(Hdc,STR_PRESSENTER_CN, xPos, 1);
			memset(DisBuffer, 42, 19);
			MulLineDisProc(Hdc, DisBuffer, g_KeyInputBufferLen, 2);
		}
		else if (STEP_OLD_PASSWORD_ERROR == PwdModStep) 
		{
			MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_OLDPSWERR), 1);
		}
		else if (STEP_NEW_PASSWORD == PwdModStep) 
		{
			//xPos = GetXPos(STR_ETRNEWPWD_CN);
			MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERNEWPSW),1);
		//	MXDrawText_Left(Hdc, STR_ETRNEWPWD_CN, xPos,0);
			//MXDrawText_Left(Hdc,STR_PRESSENTER_CN, xPos, 1);
			memset(DisBuffer, 42, 19);
			MulLineDisProc(Hdc, DisBuffer, g_KeyInputBufferLen, 2);
		}

		else if (STEP_NEW_PASSWORD_DIGIT_ERROR == PwdModStep)
		{
			MXDrawText_Center(Hdc,  GetHebrewStr(HS_ID_PSWDIGITERROR), 1);
		}
		
		else if (STEP_NEW_PASSWORD_AGAIN == PwdModStep) 
		{
			//xPos = GetXPos(STR_ETRNEWPWDAGAIN_CN);
			//MXDrawText_Left(Hdc, STR_ETRNEWPWDAGAIN_CN, xPos,0);
			//MXDrawText_Left(Hdc,STR_PRESSENTER_CN, xPos, 1);
			MXDrawText_Center(Hdc,  GetHebrewStr(HS_ID_ENTERNEWPSWAG), 1);
			memset(DisBuffer, 42, 19);
			MulLineDisProc(Hdc, DisBuffer, g_KeyInputBufferLen, 2);
		}
		else if (STEP_PASSWORD_MOD_SUCCESS == PwdModStep) 
		{
			MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_PSWMODIFYOK), 1);
		}
		else if (STEP_NEW_PASSWORD_ERROR == PwdModStep) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_PSWDIFF));
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PSWDIFF), xPos,1);
			MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_INPUTAG), xPos, 2);
			
		}		
	}
	else if (SET_ENGLISH == g_SysConfig.LangSel) 
	{
		if (STEP_OLD_PASSWORD == PwdModStep) 
		{
			xPos = GetXPos(STR_OLDPWD_EN);
			MXDrawText_Left(Hdc, STR_ETR_EN, xPos, 0);
			MXDrawText_Left(Hdc, STR_OLDPWDJING_EN, xPos, 1);

			memset(DisBuffer, 42, 19);
			MulLineDisProc(Hdc, DisBuffer, g_KeyInputBufferLen, 2);
		}
		else if (STEP_OLD_PASSWORD_ERROR == PwdModStep) 
		{
			xPos = GetXPos(STR_OLDPWD_EN);
			MXDrawText_Left(Hdc, STR_OLDPWD_EN, xPos, 1);
			MXDrawText_Left(Hdc, STR_INCORRECT_EN, xPos, 2);
		}
		else if (STEP_NEW_PASSWORD == PwdModStep) 
		{
				xPos = GetXPos(STR_NEWPWD_EN);
				MXDrawText_Left(Hdc, STR_ETR_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_NEWPWD_EN, xPos, 1);
			memset(DisBuffer, 42, 19);
			MulLineDisProc(Hdc, DisBuffer, g_KeyInputBufferLen, 2);
		}
		else if (STEP_NEW_PASSWORD_DIGIT_ERROR == PwdModStep)
		{
			MXDrawText_Center(Hdc, STR_DIGITSERROR_EN, 1);
		}
		else if (STEP_NEW_PASSWORD_AGAIN == PwdModStep) 
		{
				xPos = GetXPos(STR_ENTERNEWPWD_EN);
				MXDrawText_Left(Hdc, STR_ENTERNEWPWD_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_AGAIN_EN, xPos, 1);

			memset(DisBuffer, 42, 19);
			MulLineDisProc(Hdc, DisBuffer, g_KeyInputBufferLen, 2);
		}

		else if (STEP_PASSWORD_MOD_SUCCESS == PwdModStep) 
		{
			MXDrawText_Center(Hdc, STR_PWDMODIFY_EN, 1);
		}
		else if (STEP_NEW_PASSWORD_ERROR == PwdModStep) 
		{
			xPos = GetXPos(STR_TRYAGAIN_EN);
			MXDrawText_Left(Hdc, STR_NOTIDL_EN, xPos, 1);
			MXDrawText_Left(Hdc, STR_TRYAGAIN_EN, xPos, 2);
		}
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
SysPwdModifyWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);	
		memcpy(StoreSysPwd, g_SysConfig.SysPwd, RD_CODE_LEN);		
		ClearKeyBuffer();
		break;	
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_SYSPWD_MODIFY, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			SysPwdModifyWndPaint(hWnd, Msg, wParam, lParam);		
		}
		break;
		
	case WM_REFRESHPARENT_WND:
		if (STEP_OLD_PASSWORD == PwdModStep ||
			STEP_NEW_PASSWORD == PwdModStep ||
			STEP_NEW_PASSWORD_AGAIN == PwdModStep) 
		{
			ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, INTERFACE_SETTLE_TIME, NULL);
		}
		else
		{
			ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, PROMPT_SHOW_TIME, NULL);
		}

			
		SetFocus(hWnd);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;

	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		if (STEP_OLD_PASSWORD == PwdModStep
			|| STEP_NEW_PASSWORD == PwdModStep
			|| STEP_NEW_PASSWORD_AGAIN == PwdModStep) 
		{
			ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, INTERFACE_SETTLE_TIME, NULL);		
			SysPwdModifyKeyProcess(hWnd, Msg, wParam, lParam);
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
		
	case WM_TIMER:
		SysPwdModifyTimerProc(hWnd, Msg, wParam, lParam);
		break;
		
	case WM_DESTROY:
		PwdModStep = STEP_OLD_PASSWORD;
		KillTimer(hWnd, TIMER_SYSPWD_MODIFY);
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
CreateSysPwdModifyWnd(HWND hwndParent)
{
	static char szAppName[] = "SysPwdModifyWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) SysPwdModifyWndProc;
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
		"SysPwdModifyWnd",			// Window name	(Not NULL)
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
SysPwdModifyKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL FirstEnter = TRUE;
	static CHAR				  *pNewPassword = NULL;
	
	wParam =  KeyBoardMap(wParam);

	if (wParam == KEY_RETURN)
	{
		memcpy(g_SysConfig.SysPwd, StoreSysPwd, RD_CODE_LEN);
		memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
		g_KeyInputBufferLen = 0;
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9)) 
	{
		if (FirstEnter)
		{
			FirstEnter = FALSE;
		}

		if (g_KeyInputBufferLen < KEY_BUF_LEN)
		{
			g_KeyInputBuffer[g_KeyInputBufferLen++] = (UCHAR)wParam;
		}
		else
		{
			g_KeyInputBufferLen		=	0;
			g_KeyInputBuffer[g_KeyInputBufferLen++] = (UCHAR)wParam;
		}
	}
	else if (wParam == KEY_ENTER)
	{
		if (STEP_OLD_PASSWORD == PwdModStep) 
		{
			if (CodeCompare(g_KeyInputBuffer, g_SysConfig.SysPwd, g_KeyInputBufferLen)) 
			{
				PwdModStep = STEP_NEW_PASSWORD; 
				StartPlayRightANote();
			}
			else
			{
				PwdModStep = STEP_OLD_PASSWORD_ERROR;
				ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, PROMPT_SHOW_TIME, NULL);
				StartPlayErrorANote();
			}
			memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
			g_KeyInputBufferLen = 0;
		}
		
		else if (STEP_NEW_PASSWORD == PwdModStep) 
		{
			if (g_KeyInputBufferLen >=4 && g_KeyInputBufferLen <=12)
			{
				pNewPassword = (char*)malloc(KEY_BUF_LEN);
				memset(pNewPassword, 0, KEY_BUF_LEN);
				memcpy(pNewPassword, g_KeyInputBuffer, g_KeyInputBufferLen);
				PwdModStep =  STEP_NEW_PASSWORD_AGAIN;
				ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, INTERFACE_SETTLE_TIME, NULL);
				StartPlayRightANote();
			}
			else
			{
				PwdModStep =  STEP_NEW_PASSWORD_DIGIT_ERROR;
				ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, PROMPT_SHOW_TIME, NULL);
				StartPlayErrorANote();
			}
			ClearKeyBuffer();
		}	
		else if (STEP_NEW_PASSWORD_AGAIN == PwdModStep)
		{
			if (g_KeyInputBufferLen < MIN_PWD_LEN
				|| g_KeyInputBufferLen > MAX_PWD_LEN
				|| !CodeCompare(g_KeyInputBuffer, pNewPassword, g_KeyInputBufferLen)
				)
			{
				PwdModStep =STEP_NEW_PASSWORD_ERROR;
				StartPlayErrorANote();
			}
			else
			{
				memset(g_SysConfig.SysPwd, 0, sizeof(g_SysConfig.SysPwd));
				memcpy(g_SysConfig.SysPwd, g_KeyInputBuffer, g_KeyInputBufferLen);				
				FirstEnter		= TRUE;
				PwdModStep = STEP_PASSWORD_MOD_SUCCESS; 
				StartPlayRightANote();
				SaveMenuPara2Mem();
			}
			memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
			g_KeyInputBufferLen = 0;
			ResetTimer(hWnd, TIMER_SYSPWD_MODIFY, PROMPT_SHOW_TIME, NULL);				

			free(pNewPassword);
			pNewPassword = NULL;
		}
	}
}
