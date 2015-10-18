/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	PwdModifyWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		31 - Aug - 2008
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <errno.h>
#include <sys/timeb.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>  

/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"

#include "ModuleTalk.h"
#include "MenuParaProc.h"
#include "PwdUlkWnd.h"
#include "PwdUlk.h"


/************** DEFINES **************************************************************/

#define	PM_DISP_COUNT	5

#define	CODE_LEN        19

/************** TYPEDEFS *************************************************************/

typedef enum _PWDMODSTEP
{
	STEP_RESI_CODE = 1,
	STEP_FUN_DISABLE,
	STEP_RESI_ERROR,
	STEP_OLD_PASSWORD,
	STEP_OLD_PASSWORD_ERROR,
	STEP_NEW_PASSWORD,
	STEP_NEW_PASSWORD_DIGIT_ERROR,
	STEP_NEW_PASSWORD_AGAIN,
	STEP_NEW_PASSWORD_ERROR,
	STEP_PASSWORD_MOD_SUCCESS
}ROOMPWDMODSTEP;

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static RDINFO RdInfo;
static ROOMPWDMODSTEP	  g_PwdChgStep		= STEP_RESI_CODE;

static VOID		PwdModifyTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		PwdModifyWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK PwdModifyWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		PwdModifyKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID		PwdModifyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		SendRdOldPassword2ACC(VOID);
static VOID		SendRdNewPassword2ACC(VOID);

static VOID     SendCloseWndMsg(VOID);//added by [MichaelMa]
/*************************************************************************************/

static VOID
PwdModifyTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
//	KillTimer(hWnd, TIMER_PWDMODIFY);	

	switch(g_PwdChgStep) 
	{
	case STEP_RESI_ERROR:
		g_PwdChgStep = STEP_RESI_CODE;
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		ResetTimer(hWnd, TIMER_PWDMODIFY, INTERFACE_SETTLE_TIME, NULL);
		break;
	case STEP_OLD_PASSWORD_ERROR:
		g_PwdChgStep = STEP_OLD_PASSWORD;
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		ResetTimer(hWnd, TIMER_PWDMODIFY, INTERFACE_SETTLE_TIME, NULL);
		break;
	case STEP_NEW_PASSWORD_ERROR:
	case STEP_NEW_PASSWORD_DIGIT_ERROR:
		g_PwdChgStep = STEP_NEW_PASSWORD;
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		ResetTimer(hWnd, TIMER_PWDMODIFY, INTERFACE_SETTLE_TIME, NULL);
		break;
	case STEP_PASSWORD_MOD_SUCCESS:
	case STEP_FUN_DISABLE:
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		break;
	default:
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		break;
	}
}

static void
PwdModifyWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC				Hdc;
	PAINTSTRUCT		ps;
	RECT			Rect;	
	char			pDateBuf[TITLE_BUF_LEN] = {0};	
	INT		xPos = 0;
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;
	Hdc->textcolor = FONT_COLOR;
	
	switch(g_PwdChgStep) 
	{
		case STEP_FUN_DISABLE:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_NOTENABLED_EN);
				MXDrawText_Left(Hdc, STR_FUNIS_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_NOTENABLED_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{	
				MXDrawText_Center(Hdc, STR_FUNISNOTEN_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_FUNISNOTEN), 1);
			}
			break;
		}
		
		case STEP_RESI_CODE:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ENTERNUM_EN);
				MXDrawText_Left(Hdc, STR_ENTERRES_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ENTERNUM_EN, xPos, 1);

				MulLineDisProc(Hdc, g_KeyInputBuffer, g_KeyInputBufferLen, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ENTERRESNUM_CN);
				MXDrawText_Left(Hdc, STR_ENTERRESNUM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 1);

				MulLineDisProc(Hdc, g_KeyInputBuffer, g_KeyInputBufferLen, 2);
			}	
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERRESNUM), 1);
				MulLineDisProc(Hdc, g_KeyInputBuffer, g_KeyInputBufferLen, 2);
			}			
			break;
		}
		case STEP_RESI_ERROR:
		{
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
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_NOTREGISTER), 1);
			}
			break;
		}
		case STEP_OLD_PASSWORD:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_OLDPWD_EN);
				MXDrawText_Left(Hdc, STR_ETR_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_OLDPWDJING_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETROLDPWD_CN);
				MXDrawText_Left(Hdc, STR_ETROLDPWD_CN, xPos,0);
				MXDrawText_Left(Hdc,STR_PRESSENTER_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTEROLDPSW), 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);
			break;
		}
		case STEP_OLD_PASSWORD_ERROR:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_OLDPWD_EN);
				MXDrawText_Left(Hdc, STR_OLDPWD_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_INCORRECT_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{			
				MXDrawText_Center(Hdc, STR_OLDPWDINCORRECT_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_OLDPSWERR), 1);
			}
			break;
		}
		case STEP_NEW_PASSWORD:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_NEWPWD_EN);
				MXDrawText_Left(Hdc, STR_ETR_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_NEWPWD_EN, xPos, 1);
				
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRNEWPWD_CN);
				MXDrawText_Left(Hdc, STR_ETRNEWPWD_CN, xPos,0);
				MXDrawText_Left(Hdc,STR_PRESSENTER_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ENTERNEWPSW));
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_ENTERNEWPSW), xPos, 1);
			//	MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PRESSENTER), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);
			break;
		}

		case STEP_NEW_PASSWORD_DIGIT_ERROR:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_DIGITSERROR_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{			
				MXDrawText_Center(Hdc, STR_DIGITSERROR_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_PSWDIGITERROR), 1);
			}
			break;
		}

		case STEP_NEW_PASSWORD_AGAIN:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ENTERNEWPWD_EN);
				MXDrawText_Left(Hdc, STR_ENTERNEWPWD_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_AGAIN_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRNEWPWDAGAIN_CN);
				MXDrawText_Left(Hdc, STR_ETRNEWPWDAGAIN_CN, xPos,0);
				MXDrawText_Left(Hdc,STR_PRESSENTER_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ENTERNEWPSWAG));
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_ENTERNEWPSWAG), xPos, 1);
				//MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PRESSENTER), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);
			break;
		}
		case STEP_NEW_PASSWORD_ERROR:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TRYAGAIN_EN);
				MXDrawText_Left(Hdc, STR_NOTIDL_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_TRYAGAIN_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{	
				xPos = GetXPos(STR_NOTIDL_CN);
				MXDrawText_Left(Hdc, STR_NOTIDL_CN, xPos,1);
				MXDrawText_Left(Hdc,STR_ETRPWDAGAIN_CN, xPos, 2);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_PSWDIFF));
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PSWDIFF), xPos, 0);
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_INPUTAG), xPos, 1);
			}
			break;
		}
		
		case STEP_PASSWORD_MOD_SUCCESS:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_PWDMODIFY_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{			
				MXDrawText_Center(Hdc, STR_PWDMODIFY_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_PSWMODIFYOK), 1);
			}
			break;
		}
		
	default:
		break;
	}
	
	EndPaint(hWnd, &ps);
}

static LRESULT CALLBACK
PwdModifyWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	BYTE byResult = 0;
	BYTE byType	= 0;

	switch (Msg)
	{
	case WM_CREATE:
		ClearKeyBuffer();
		if (MODE_ROOMCODE_PASSWORD_SET & g_ASPara.ASOpenMode) 
		{
			if (DEVICE_CORAL_DB == g_DevFun.DevType) 
			{
				PostMessage(hWnd,  WM_CHAR, KEY_ENTER, 0);
			}			
			g_PwdChgStep	= STEP_RESI_CODE;
			PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		}
		else
		{
			g_PwdChgStep	= STEP_FUN_DISABLE;
			PostMessage(hWnd,  WM_SETTIMER, PROMPT_SHOW_TIME, 0);
			StartPlayErrorANote();
		}
		break;
		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_PWDMODIFY, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			PwdModifyWndPaint(hWnd, Msg, wParam, lParam);
		}
		break;
		
	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}

		if (STEP_RESI_CODE == g_PwdChgStep
		   || STEP_OLD_PASSWORD == g_PwdChgStep
		   || STEP_NEW_PASSWORD == g_PwdChgStep
		   || STEP_NEW_PASSWORD_AGAIN == g_PwdChgStep)
		{
			ResetTimer(hWnd, TIMER_PWDMODIFY, INTERFACE_SETTLE_TIME, NULL);
			PwdModifyKeyProcess(hWnd, Msg, wParam, lParam);
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;

	case WM_RDPASSWORD:
		ResetTimer(hWnd, TIMER_AS_PROC, INTERFACE_SETTLE_TIME, NULL);
		
		byResult = (BYTE)lParam;
		switch(byResult)
		{
			case 0:
			{
				byType = (BYTE)wParam;
				if (1 == byType)
				{
					g_PwdChgStep = STEP_NEW_PASSWORD;
				}
				else if (2 == byType)
				{
					g_PwdChgStep = STEP_PASSWORD_MOD_SUCCESS;
					ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				}		
				
				StartPlayRightANote();		
				
				break;
			}
			case 1:
			{
				g_PwdChgStep = STEP_RESI_ERROR;
				
				StartPlayErrorANote();
				
				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case 2:
			{
				g_PwdChgStep = STEP_OLD_PASSWORD_ERROR;
				StartPlayErrorANote();
				
				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);

				break;
			}
			default:
			{
				break;
			}
		}
		
		PostMessage(hWnd, WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		PwdModifyTimerProc(hWnd, Msg, wParam, lParam);
		break;

	case WM_REFRESHPARENT_WND:
		if (STEP_RESI_CODE == g_PwdChgStep
			|| STEP_OLD_PASSWORD == g_PwdChgStep
			|| STEP_NEW_PASSWORD == g_PwdChgStep
			|| STEP_NEW_PASSWORD_AGAIN == g_PwdChgStep)
		{
			ResetTimer(hWnd, TIMER_PWDMODIFY, INTERFACE_SETTLE_TIME, NULL);
		}
		else
		{
			ResetTimer(hWnd, TIMER_PWDMODIFY, PROMPT_SHOW_TIME, NULL);
		}
			
		SetFocus(hWnd);
		SendMessage(hWnd,  WM_PAINT, 0, 0);
		break;	
		
	case WM_DESTROY:
		ClearKeyBuffer();
		KillTimer(hWnd, TIMER_PWDMODIFY);
//		PostMessage(GetParent(hWnd),  WM_REFRESHPARENT_WND, 0, 0);
//		SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		break;
		
	default:
		DefWindowProc(hWnd, Msg, wParam, lParam);
		
		if (WM_CLOSE == Msg) 
		{
			RemoveOneWnd(hWnd);
		}

//		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

void
CreatePwdModifyWnd(HWND hwndParent)
{
	static char szAppName[] = "PwdModifyWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) PwdModifyWndProc;
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
		"PwdModifyWnd",			// Window name	(Not NULL)
		WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,	// Style		(0)
		0,					// x			(0)
		0,					// y			(0)
		SCREEN_WIDTH,		// Width		
		SCREEN_HEIGHT,		// Height
		g_hMainWnd /*hwndParent*/,	// Parent		(MwGetFocus())
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
**	FUNCTION NAME:	SendCloseWndMsg
**	AUTHOR:		    Michael Ma
**	DATE:		    4 - Sep - 2012
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
SendCloseWndMsg(VOID)
{
    MXMSG		msgSend;

	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ACC;
	msgSend.dwMsg		= MXMSG_CLOSE_WINDOW;
	msgSend.pParam      = NULL;
    switch(g_PwdChgStep)
    {
        case STEP_OLD_PASSWORD: 
                                    MxPutMsg(&msgSend); break;
        case STEP_NEW_PASSWORD: 
                                    MxPutMsg(&msgSend); break;
        case STEP_NEW_PASSWORD_AGAIN : 
                                    MxPutMsg(&msgSend); break;
        default:
		{
			break;
		}
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

static void 
PwdModifyKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	wParam =  KeyBoardMap(wParam);
	
	if (wParam == KEY_RETURN)
	{
        SendCloseWndMsg(); // added by [MichaelMa]
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9)) 
	{
		if (g_KeyInputBufferLen < (KEY_BUF_LEN - 1))
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
		PwdModifyProcess(hWnd, Msg, wParam, lParam);	
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PwdModifyProcess
**	AUTHOR:			Jeff Wang
**	DATE:			31 - Aug - 2008
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
PwdModifyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static CHAR* pNewPassword = NULL;

	switch(g_PwdChgStep) 
	{
		case STEP_RESI_CODE:
		{
			if (DEVICE_CORAL_DB == g_DevFun.DevType) 
			{
				strcpy(g_KeyInputBuffer, CORAL_DB_ROOM_CODE);
				g_KeyInputBufferLen = 3;
			}

			if (IsRdExist(g_KeyInputBuffer, g_KeyInputBufferLen))
			{
				if (g_KeyInputBufferLen < RD_CODE_LEN)
				{
					memset(RdInfo.RdCode, 0, RD_CODE_LEN);
					memcpy(RdInfo.RdCode, g_KeyInputBuffer, g_KeyInputBufferLen);
				}

				g_PwdChgStep = STEP_OLD_PASSWORD;
				StartPlayRightANote();
			}
			else
			{
				g_PwdChgStep = STEP_RESI_ERROR;
				ResetTimer(hWnd, TIMER_PWDMODIFY, PROMPT_SHOW_TIME, NULL);
				StartPlayErrorANote();
			}

			ClearKeyBuffer();
			break;
		}
		case STEP_OLD_PASSWORD:
		{
			if (CodeCompare(g_KeyInputBuffer, g_SysConfig.SysPwd, g_KeyInputBufferLen)) 
			{
				g_PwdChgStep = STEP_NEW_PASSWORD;
				StartPlayRightANote();
			}
			else
			{
				if (g_KeyInputBufferLen < PWD_LEN)
				{
					memset(RdInfo.RdPwd, 0, PWD_LEN);
					memcpy(RdInfo.RdPwd, g_KeyInputBuffer, g_KeyInputBufferLen);
				}

				SendRdOldPassword2ACC();
				//PostMessage(hWnd,  WM_CLOSE, 0, 0); //for bug 9616
			}
			
			ClearKeyBuffer();
			break;
		}
		case STEP_NEW_PASSWORD:
		{
			if (g_KeyInputBufferLen >=4 && g_KeyInputBufferLen <=12)
			{
				pNewPassword = (char*)malloc(KEY_BUF_LEN);
				memset(pNewPassword, 0, KEY_BUF_LEN);
				memcpy(pNewPassword, g_KeyInputBuffer, g_KeyInputBufferLen);
				g_PwdChgStep =  STEP_NEW_PASSWORD_AGAIN;
				ResetTimer(hWnd, TIMER_PWDMODIFY, INTERFACE_SETTLE_TIME, NULL);
                StartPlayRightANote();
			}
			else
			{
				g_PwdChgStep =  STEP_NEW_PASSWORD_DIGIT_ERROR;
				ResetTimer(hWnd, TIMER_PWDMODIFY, PROMPT_SHOW_TIME, NULL);
				StartPlayErrorANote();
			}
			ClearKeyBuffer();
			break;
		}
		case STEP_NEW_PASSWORD_AGAIN:
		{			
			if (g_KeyInputBufferLen < MIN_PWD_LEN
				|| g_KeyInputBufferLen > MAX_PWD_LEN
				|| !CodeCompare(g_KeyInputBuffer, pNewPassword, g_KeyInputBufferLen)
				)
			{
				g_PwdChgStep =STEP_NEW_PASSWORD_ERROR;
				ResetTimer(hWnd, TIMER_PWDMODIFY, PROMPT_SHOW_TIME, NULL);
				StartPlayErrorANote();
			}
			else
			{
				if (g_KeyInputBufferLen < PWD_LEN)
				{
					memset(RdInfo.RdPwd, 0, PWD_LEN);
					memcpy(RdInfo.RdPwd, g_KeyInputBuffer, g_KeyInputBufferLen);
				}
				RdInfo.bPwdCfg = 1;
				
				SendRdNewPassword2ACC();
				//PostMessage(hWnd,  WM_CLOSE, 0, 0); //for bug 9616
			}
			
			ClearKeyBuffer();
			free(pNewPassword);
			pNewPassword = NULL;
			break;
		}
		default:
		{
			break;
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendRdOldPassword2ACC
**	AUTHOR:		   Wayde Zeng
**	DATE:		3 - March - 2011
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/
static
VOID
SendRdOldPassword2ACC(VOID)
{
	MXMSG		msgSend;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ACC;
	msgSend.dwMsg		= FC_AC_PWD_CHECK;
	
	nDataLen = RD_CODE_LEN + 1/*Pwd len*/ + PWD_LEN;
	msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);

	memcpy(msgSend.pParam, &nDataLen, sizeof(unsigned short));
	memcpy(msgSend.pParam + sizeof(unsigned short), RdInfo.RdCode, RD_CODE_LEN);
	if (g_KeyInputBufferLen < PWD_LEN)
	{
		*(msgSend.pParam + sizeof(unsigned short) + RD_CODE_LEN) = g_KeyInputBufferLen;
		memcpy(msgSend.pParam + sizeof(unsigned short) + RD_CODE_LEN + 1, g_KeyInputBuffer, g_KeyInputBufferLen);
	}
	
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendRdNewPassword2ACC
**	AUTHOR:		   Wayde Zeng
**	DATE:		3 - March - 2011
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/
static
VOID
SendRdNewPassword2ACC(VOID)
{
	MXMSG		msgSend;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ACC;
	msgSend.dwMsg		= FC_AC_PWD_MODIFY;
	
	nDataLen = RD_CODE_LEN + 1/*Pwd len*/ + PWD_LEN;
	msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);

	memcpy(msgSend.pParam, &nDataLen, sizeof(unsigned short));
	memcpy(msgSend.pParam + sizeof(unsigned short), RdInfo.RdCode, RD_CODE_LEN);
	if (g_KeyInputBufferLen < PWD_LEN)
	{
		*(msgSend.pParam + sizeof(unsigned short) + RD_CODE_LEN) = g_KeyInputBufferLen;
		memcpy(msgSend.pParam + sizeof(unsigned short) + RD_CODE_LEN + 1, g_KeyInputBuffer, g_KeyInputBufferLen);
	}
	
	MxPutMsg(&msgSend);	
}



