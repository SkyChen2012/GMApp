/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	PwdModifyWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		01 - Sep - 2008
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
#include <string.h>
#include <stdlib.h>


/************** USER INCLUDE FILES ***************************************************/
#include "MXTypes.h"
#include "MXCommon.h"

#include "ModuleTalk.h"
#include "MenuParaProc.h"
#include "PwdUlkWnd.h"
#include "TalkLogReport.h"
#include "PwdUlk.h"
#include "AccessLogReport.h"
#include "AccessProc.h"

/************** DEFINES **************************************************************/

#define	PU_DISP_COUNT	5

/************** TYPEDEFS *************************************************************/

typedef enum _PUSTEP
{
	STEP_FUN_DISABLE = 1,
	STEP_INPUT_RESI_NUM,
	STEP_RESI_NUM_INEXISTENCE,
	STEP_INPUT_PASSWORD,
	STEP_PASSWORD_ERROR,
	STEP_UNLOCK_SUCCESS
}PWDULKSTEP;

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static RDINFO RdInfo;
static PWDULKSTEP	  g_PwdUnlockStep		= STEP_INPUT_RESI_NUM;


static VOID		PwdUnlockTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		PwdUnlockWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK PwdUnlockWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		PwdUnlockKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID		PwdUnlockProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		SendRdPassword2ACC(VOID);

/*************************************************************************************/

static VOID
PwdUnlockTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
//	KillTimer(hWnd, TIMER_PWDUNLCOK);	
	switch(g_PwdUnlockStep) 
	{
	case STEP_RESI_NUM_INEXISTENCE:
		{
			g_PwdUnlockStep = STEP_INPUT_RESI_NUM;
			PostMessage(hWnd,  WM_PAINT, 0, 0);
			ResetTimer(hWnd, TIMER_PWDUNLCOK, INTERFACE_SETTLE_TIME, NULL);
		}
		break;
	case STEP_PASSWORD_ERROR:
		{
			g_PwdUnlockStep = STEP_INPUT_PASSWORD;
			PostMessage(hWnd,  WM_PAINT, 0, 0);
			ResetTimer(hWnd, TIMER_PWDUNLCOK, INTERFACE_SETTLE_TIME, NULL);
		}
		break;
	case STEP_UNLOCK_SUCCESS:
		{
//			UnlockGateEnd();
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		break;
	case STEP_FUN_DISABLE:
		{
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		break;
	default:
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		break;
	}
}

static VOID
PwdUnlockWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
	switch(g_PwdUnlockStep) 
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
		
		case STEP_INPUT_RESI_NUM:
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
		case STEP_RESI_NUM_INEXISTENCE:
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

		case STEP_INPUT_PASSWORD:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_ETRPWD_EN, 1);	
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRPWD_CN);
				MXDrawText_Left(Hdc, STR_ETRPWD_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ETRPWD), 1);
			}
			//	printf("g_KeyInputBufferLen=%d\n",g_KeyInputBufferLen);
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);
			break;
		}
		case STEP_PASSWORD_ERROR:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_PWDIS_EN);
				MXDrawText_Left(Hdc, STR_PWDIS_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_INCORRECT_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{			
				MXDrawText_Center(Hdc, STR_PWDINCORRECT_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_PWDINCORRECT), 1);
			}
			break;
		}
		case STEP_UNLOCK_SUCCESS:
		{
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
			break;
		}
		default:
			break;
	}	
	EndPaint(hWnd, &ps);
}

static LRESULT CALLBACK
PwdUnlockWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	BYTE byResult = 0;

	switch (Msg)
	{
	case WM_CREATE:
		ClearKeyBuffer();
		if (MODE_ROOMCODE_PASSWORD_SET & g_ASPara.ASOpenMode) 
		{
			if (DEVICE_CORAL_DB == g_DevFun.DevType) 
			{
				g_PwdUnlockStep	= STEP_INPUT_PASSWORD;
				
				strcpy(RdInfo.RdCode,CORAL_DB_ROOM_CODE);
				g_KeyInputBufferLen=0;
			}
			else
			{
				g_PwdUnlockStep	= STEP_INPUT_RESI_NUM;
			}
			PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);	
		}
		else
		{
			g_PwdUnlockStep = STEP_FUN_DISABLE;
			PostMessage(hWnd,  WM_SETTIMER, PROMPT_SHOW_TIME, 0);			
			StartPlayErrorANote();
		}		
		break;

	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_PWDUNLCOK, wParam, NULL);
		break;
	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			PwdUnlockWndPaint(hWnd, Msg, wParam, lParam);
		}
		break;
		
	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		if (STEP_INPUT_RESI_NUM == g_PwdUnlockStep
			|| STEP_INPUT_PASSWORD == g_PwdUnlockStep) 
		{
			ResetTimer(hWnd, TIMER_PWDUNLCOK, INTERFACE_SETTLE_TIME, NULL);
			PwdUnlockKeyProcess(hWnd, Msg, wParam, lParam);
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
				g_PwdUnlockStep = STEP_UNLOCK_SUCCESS;
				StartPlayRightANote();
				
				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case 1:
			{
				if(DEVICE_CORAL_DB == g_DevFun.DevType)
				{
					g_PwdUnlockStep = STEP_PASSWORD_ERROR;
				}
				else
				{
					g_PwdUnlockStep = STEP_RESI_NUM_INEXISTENCE;
				}
				StartPlayErrorANote();
				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case 2:
			{
				g_PwdUnlockStep = STEP_PASSWORD_ERROR;
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
		PwdUnlockTimerProc(hWnd, Msg, wParam, lParam);
		break;

	case WM_REFRESHPARENT_WND:
		if (STEP_INPUT_RESI_NUM == g_PwdUnlockStep || STEP_INPUT_PASSWORD == g_PwdUnlockStep) 
		{
			ResetTimer(hWnd, TIMER_PWDUNLCOK, INTERFACE_SETTLE_TIME, NULL);  
		}
		else
		{
			ResetTimer(hWnd, TIMER_PWDUNLCOK, PROMPT_SHOW_TIME, NULL);
		}
		
		SetFocus(hWnd);
		SendMessage(hWnd,  WM_PAINT, 0, 0);
		break;	
		
	case WM_DESTROY:
		ClearKeyBuffer();
		KillTimer(hWnd, TIMER_PWDUNLCOK);
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
CreatePwdUnlockWnd(HWND hwndParent)
{
	static char szAppClassName[] = "PwdUnlock";
	static char	szAppName[]	= "PwdUnlockWnd";

	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) PwdUnlockWndProc;
	WndClass.cbClsExtra		= 0;
	WndClass.cbWndExtra		= 0;
	WndClass.hInstance		= 0;
	WndClass.hIcon			= 0;
	WndClass.hCursor		= 0;
	WndClass.hbrBackground	= (HBRUSH)GetStockObject(BACKGROUND_COLOR);
	WndClass.lpszMenuName	= NULL;
	WndClass.lpszClassName	= szAppClassName;
	
	RegisterClass(&WndClass);
	
	hWnd = CreateWindowEx(
		0L,					// Extend style	(0)
		szAppClassName,				// Class name	(NULL)
		szAppName,			// Window name	(Not NULL)
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

static void PwdUnlockKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	wParam =  KeyBoardMap(wParam);
	if (wParam == KEY_RETURN)
	{
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
		PwdUnlockProcess(hWnd, Msg, wParam, lParam);	
	}
	else if (KEY_DB_CALL == wParam && DEVICE_CORAL_DB == g_DevFun.DevType) 
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
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
PwdUnlockProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(g_PwdUnlockStep) 
	{
		case STEP_INPUT_RESI_NUM:
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
                    int i = 0;
                    for( ; i < 20 ; i++)
                    {
                        printf("%02x",(RdInfo.RdCode)[i]);
                    }
                    printf("\n");
                }

				g_PwdUnlockStep = STEP_INPUT_PASSWORD;
				StartPlayRightANote();
			}
			else
			{
				g_PwdUnlockStep = STEP_RESI_NUM_INEXISTENCE;
				ResetTimer(hWnd, TIMER_PWDUNLCOK, PROMPT_SHOW_TIME, NULL);
				StartPlayErrorANote();
			}		
			
			//memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
			//g_KeyInputBufferLen		=	0;
            ClearKeyBuffer();
            printf("\n");
			break;
		}		
		case STEP_INPUT_PASSWORD:
		{
			//if (CodeCompare(g_KeyInputBuffer, g_SysConfig.SysPwd, g_KeyInputBufferLen)) 
			//{
			//	SendUnlockGate2ACC(UNLOCKTYPE_RDPWD);
			//}
			//else
			//{
				if (g_KeyInputBufferLen < PWD_LEN)
				{
					memset(RdInfo.RdPwd, 0, PWD_LEN);
					memcpy(RdInfo.RdPwd, g_KeyInputBuffer, g_KeyInputBufferLen);
				}
				SendRdPassword2ACC();
			//}

			ClearKeyBuffer();		
			//PostMessage(hWnd,  WM_CLOSE, 0, 0); //for bug 9616
			printf("\n");
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
**	FUNCTION NAME:	SendRdPassword2ACC
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
SendRdPassword2ACC(VOID)
{
	MXMSG		msgSend;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ACC;
	msgSend.dwMsg		= FC_AC_RSD_PWD;
	
	nDataLen = RD_CODE_LEN + 1/*Pwd len*/ + PWD_LEN + 1/*Gate Index*/;
	msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);

	memcpy(msgSend.pParam, &nDataLen, sizeof(unsigned short));
	memcpy(msgSend.pParam + sizeof(unsigned short), RdInfo.RdCode, RD_CODE_LEN);
	if (g_KeyInputBufferLen < PWD_LEN)
	{
		*(msgSend.pParam + sizeof(unsigned short) + RD_CODE_LEN) = g_KeyInputBufferLen;
		memcpy(msgSend.pParam + sizeof(unsigned short) + RD_CODE_LEN + 1, g_KeyInputBuffer, g_KeyInputBufferLen);
	}
	 
	*(msgSend.pParam + sizeof(unsigned short) + RD_CODE_LEN + 1 + PWD_LEN) = GATE_ID_DEFAULT;
	
	MxPutMsg(&msgSend);	
}


