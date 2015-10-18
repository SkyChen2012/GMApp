
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	ASRdPwdModWnd.c
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
**	NOTES:
** 
*/
/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"
#include "AccessProc.h"

#include "AccessCommon.h"
#include "ASRdPwdModWnd.h"

/************** DEFINES **************************************************************/


/************** TYPEDEFS *************************************************************/

typedef enum _ASPWDMODSTEP
{
	STEP_FUN_DISABLE = 1,
	STEP_ETR_RDNUM,
	STEP_RDNUM_NOTREGISTERED,
	STEP_INPUTOLDPASSWORD,
	STEP_OLDPASSWORDERROR,
	STEP_INPUTNEWPASSWORD,	
	STEP_NEW_PASSWORD_AGAIN,
	STEP_NEW_PASSWORD_ERROR,
	STEP_PASSWORDMODIFYSUCCESS
}ASRDPWDMODSTEP;

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

#define	AS_DISP_COUNT	5

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID ASRdPwdModKeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID ASRdPwdModWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ASRdPwdModWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID ASModPwdByRdNum(CHAR* pRdNumber, CHAR* pNewPwd);

static ASRDPWDMODSTEP ASRdPwdModStep = STEP_ETR_RDNUM;
static CHAR*				  pRdNumber  = NULL;
static CHAR*				  pNewPassword = NULL;

/*************************************************************************************/

static VOID
ASRdPwdModWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
	
	switch(ASRdPwdModStep) 
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
		
	case STEP_ETR_RDNUM:
		{
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
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERRESNUM), 1);
			}
			MulLineDisProc(Hdc, g_KeyInputBuffer, g_KeyInputBufferLen, 3);
			break;
		}
		
	case STEP_RDNUM_NOTREGISTERED:
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
	case STEP_INPUTOLDPASSWORD:
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
	case STEP_OLDPASSWORDERROR:
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
	case STEP_INPUTNEWPASSWORD:
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
				//MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PRESSENTER), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);
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
		
	case STEP_PASSWORDMODIFYSUCCESS:
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
ASRdPwdModWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{    
	switch (Msg)
	{
	case WM_CREATE:
		if (!(MODE_CARD_ONLY_SET & g_ASPara.ASOpenMode) 
			&&!(MODE_CARD_PASSWORD_SET & g_ASPara.ASOpenMode)) 
		{
			ASRdPwdModStep = STEP_FUN_DISABLE;
			PostMessage(hWnd,  WM_SETTIMER, PROMPT_SHOW_TIME, 0);
			StartPlayErrorANote();
		}
		else
		{
			ASRdPwdModStep = STEP_ETR_RDNUM;
			PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);	
		}
		break;
	
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_AS_PWDMOD, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			ASRdPwdModWndPaint(hWnd, Msg, wParam, lParam);
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
		if (STEP_ETR_RDNUM == ASRdPwdModStep
			|| STEP_INPUTOLDPASSWORD == ASRdPwdModStep
			|| STEP_INPUTNEWPASSWORD == ASRdPwdModStep
			|| STEP_NEW_PASSWORD_AGAIN == ASRdPwdModStep)
		{
			ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
			ASRdPwdModKeyProc(hWnd, Msg, wParam, lParam);			
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
		
	case WM_TIMER:
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
		
	case WM_DESTROY:
		g_ASInfo.ASWorkStatus = STATUS_OPEN;
		KillTimer(hWnd, TIMER_AS_PWDMOD);
		ClearKeyBuffer();
		free(pNewPassword);
		pNewPassword = NULL;
		free(pRdNumber);
		pRdNumber = NULL;	
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

VOID
CreateRdPwdModWnd(HWND hwndParent)
{
	static char szAppName[] = "ASRdPwdModWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) ASRdPwdModWndProc;
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
		"AsRdPwdModWnd",			// Window name	(Not NULL)
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
**	FUNCTION NAME:	KeyASCtrlProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		27 - May - 2008
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
static VOID 
ASRdPwdModKeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	wParam =  KeyBoardMap(wParam);
	if (wParam == KEY_RETURN)
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);		
	}
	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9)) 
	{
		if (STEP_ETR_RDNUM == ASRdPwdModStep 
			||STEP_INPUTOLDPASSWORD == ASRdPwdModStep 
			||STEP_INPUTNEWPASSWORD == ASRdPwdModStep
			||STEP_NEW_PASSWORD_AGAIN == ASRdPwdModStep)	
		{
			CircleBufEnter(g_KeyInputBuffer, &g_KeyInputBufferLen, KEY_BUF_LEN, (CHAR)wParam);
		}
		else
		{
			ClearKeyBuffer();
		}		
	}
	else if (wParam == KEY_ENTER)
	{
		switch(ASRdPwdModStep) 
		{
		case STEP_ETR_RDNUM:
			{
				if (IsRdExist(g_KeyInputBuffer, g_KeyInputBufferLen))
				{
					pRdNumber = (CHAR*)malloc(g_KeyInputBufferLen+1);
					memset(pRdNumber, 0, g_KeyInputBufferLen+1);
					memcpy(pRdNumber, g_KeyInputBuffer, g_KeyInputBufferLen);
					StartPlayErrorANote();
					ASRdPwdModStep = STEP_INPUTOLDPASSWORD;
				}
				else
				{
					ASRdPwdModStep = STEP_RDNUM_NOTREGISTERED;
					ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
					StartPlayErrorANote();
				}
				break;
			}
			
		case STEP_INPUTOLDPASSWORD:
			{
				if (CodeCompare(g_KeyInputBuffer, g_SysConfig.SysPwd, g_KeyInputBufferLen)) 
				{
					ASRdPwdModStep = STEP_INPUTNEWPASSWORD;
					StartPlayErrorANote();
				}
				else
				{
					ASRdPwdModStep = STEP_OLDPASSWORDERROR;
					ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
					StartPlayErrorANote();
				}
				break;
			}
		case STEP_INPUTNEWPASSWORD:
			{
				pNewPassword = (CHAR*)malloc(KEY_BUF_LEN);
				memset(pNewPassword, 0, KEY_BUF_LEN);
				memcpy(pNewPassword, g_KeyInputBuffer, g_KeyInputBufferLen);
				ASRdPwdModStep =  STEP_NEW_PASSWORD_AGAIN;
				memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
				g_KeyInputBufferLen		=	0;				
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
				break;
			}
		case STEP_NEW_PASSWORD_AGAIN:
			{	
				if (g_KeyInputBufferLen < MIN_PWD_LEN
					|| g_KeyInputBufferLen > MAX_PWD_LEN
					|| !CodeCompare(g_KeyInputBuffer, pNewPassword, g_KeyInputBufferLen)
					)
				{
					ASRdPwdModStep = STEP_NEW_PASSWORD_ERROR;
					StartPlayErrorANote();
				}
				else
				{
					ASModPwdByRdNum(pRdNumber, pNewPassword);
					SaveCdInfo2Mem();
					ASRdPwdModStep = STEP_PASSWORDMODIFYSUCCESS;
					StartPlayErrorANote();
					free(pRdNumber);
					pRdNumber = NULL;					
				}
				
				free(pNewPassword);
				pNewPassword = NULL;
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
				break;
			}
		default:
			break;
		}
				
		ClearKeyBuffer();		
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ASModPwdByRdNum
**	AUTHOR:		   Jeff Wang
**	DATE:		27 - May - 2008
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
static VOID 
ASModPwdByRdNum(CHAR* pRdNumber, CHAR* pNewPwd)
{
	CDINFO*				pCdInfo	= NULL;
	CHAR*				  p = NULL;
	INT						   nRdCount = 0;
	
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return;
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt)
	{
		pCdInfo = (CDINFO*)p;		
		if (0 == strcmp(pCdInfo->RdNum, pRdNumber))
		{
			strcpy(pCdInfo->UlkPwd, pNewPwd);
		}			
		p += sizeof(CDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}	
}
