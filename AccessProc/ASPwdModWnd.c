
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	ASPwdModWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		08 - Sep - 2008
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
#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"
#include "AccessProc.h"
#include "AccessLogReport.h"
#include "AccessCommon.h"
#include "ASPwdModWnd.h"

/************** DEFINES **************************************************************/


/************** TYPEDEFS *************************************************************/

typedef enum _ASPWDMODSTEP
{
	STEP_STAMPCARD = 1,
	STEP_FUN_DISABLE,
	STEP_CARDNOTAUTHORIZED,
	PMTMSG_CARDDISABLED,
	PMTMSG_EXPIRED,
	PMTMSG_ACCREDIT,
	STEP_INPUTOLDPASSWORD,
	STEP_OLDPASSWORDERROR,
	STEP_INPUTNEWPASSWORD,
	STEP_NEW_PASSWORD_DIGIT_ERROR,
	STEP_NEW_PASSWORD_AGAIN,
	STEP_NEW_PASSWORD_ERROR,
	STEP_PASSWORDMODIFYSUCCESS,
	PMTMSG_PATROL
	
}ASPWDMODSTEP;

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

static VOID ASPwdModKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID ASPwdModWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ASPwdModWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

//static CDINFO* CdInfo = NULL;
static ASPWDMODSTEP ASPwdModStep    = STEP_STAMPCARD;
static CHAR		    *pNewPassword   = NULL;

static VOID SendCsnPwdModPwd2ACC(BYTE nPwdType);

extern HANDLE*   g_HandleHashCard ;//¿¨Æ¬´æ´¢µÄ¾ä±ú


/*************************************************************************************/

static VOID
ASPwdModWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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

	switch(ASPwdModStep) 
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
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_SWIPECARD), xPos, 1);
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PRESSEXIT), xPos, 2);
			}
			break;
		}
		
	case STEP_CARDNOTAUTHORIZED:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_CARDISNOT_EN);
				MXDrawText_Left(Hdc, STR_CARDISNOT_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_AUTHORIZED_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{			
				MXDrawText_Center(Hdc, STR_CARDISNOTAU_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CARDISNOTAU), 1);
			}
			break;
		}
		
	case PMTMSG_CARDDISABLED:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_CARDISNOT_EN);
				MXDrawText_Left(Hdc, STR_CARDISNOT_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_ENABLED_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{			
				MXDrawText_Center(Hdc, STR_CARDISNOTENABLED_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CARDISNOTENABLED), 1);
			}
			break;
		}
		
	case PMTMSG_EXPIRED:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CARDEXPIRED_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{			
				MXDrawText_Center(Hdc, STR_CARDEXPIRED_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CARDEXPIRED), 1);
			}
			break;
		}
	case PMTMSG_PATROL:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_PATROLCARD_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{			
				MXDrawText_Center(Hdc, STR_PATROLCARD_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_PATROLCARD), 1);
			}
			break;
		}
    case PMTMSG_ACCREDIT:
        {
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_AUTHORIZEDCARD_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{			
				MXDrawText_Center(Hdc, STR_AUTHORIZEDCARD_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_AUTHORIZEDCARD), 1);
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
	case STEP_NEW_PASSWORD_DIGIT_ERROR :
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
			//	MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PRESSENTER), xPos, 1);
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
ASPwdModWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{    
	//BYTE CdSts = 0;
    BYTE byResult = 0;
	switch (Msg)
	{
	case WM_CREATE:
		if (!(MODE_CARD_ONLY_SET & g_ASPara.ASOpenMode) 
			&&!(MODE_CARD_PASSWORD_SET & g_ASPara.ASOpenMode)) 
		{
			ASPwdModStep = STEP_FUN_DISABLE;
			PostMessage(hWnd,  WM_SETTIMER, PROMPT_SHOW_TIME, 0);
			StartPlayErrorANote();
		}
		else
		{
			ASPwdModStep = STEP_STAMPCARD;
			PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		}
		break;
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_AS_PWDMOD, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			ASPwdModWndPaint(hWnd, Msg, wParam, lParam);
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
		if (STEP_STAMPCARD == ASPwdModStep
			|| STEP_INPUTOLDPASSWORD == ASPwdModStep
			|| STEP_INPUTNEWPASSWORD == ASPwdModStep
			|| STEP_NEW_PASSWORD_AGAIN == ASPwdModStep)
		{
			ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
			ASPwdModKeyProcess(hWnd, Msg, wParam, lParam);			
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;

	case WM_CARDREAD:
	{
        if (STEP_FUN_DISABLE == ASPwdModStep) 
		{
			return;
		} 
        
		ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);

		byResult = (BYTE)lParam;
		switch(byResult)
		{
            case CSN_PWD_MOD_FUN_DISABLED                 :
            {
                ClearKeyBuffer();
				ASPwdModStep = STEP_FUN_DISABLE;
				StartPlayErrorANote();
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
                break;
            }
			case CSN_PWD_MOD_CARD_EXIST               :
			{
                ClearKeyBuffer();
				ASPwdModStep = STEP_INPUTOLDPASSWORD;
				StartPlayRightANote();
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case CSN_PWD_MOD_AUTHORIZING               :
			{
				ASPwdModStep = PMTMSG_ACCREDIT;
				StartPlayErrorANote();
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
				break;
			}
            case CSN_PWD_MOD_PATROL           :
            {
                ASPwdModStep = PMTMSG_PATROL;
				StartPlayErrorANote();
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
				break;
            }
			case CSN_PWD_MOD_EXPIRED                      :
			{
				ASPwdModStep = PMTMSG_EXPIRED;
				StartPlayErrorANote();
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case CSN_PWD_MOD_INVALID                      :
			{
				ASPwdModStep = 	STEP_CARDNOTAUTHORIZED;
				StartPlayErrorANote();	
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case CSN_PWD_MOD_DISABLED                :
			{
				ASPwdModStep = PMTMSG_CARDDISABLED;
				StartPlayErrorANote();
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
				break;
			}
            case CSN_PWD_MOD_NOT_IN_TIME_SLICE            :
            {
                //ASPwdModStep = ;
                StartPlayErrorANote();
                ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL); 
                break;
            }
			case CSN_PWD_MOD_PWD_ERROR               :
			{
				ASPwdModStep = STEP_OLDPASSWORDERROR;
				StartPlayErrorANote();
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL); 
				break;
			}
			case CSN_PWD_MOD_PWD_CORRECT    :
			{
                ClearKeyBuffer();
				ASPwdModStep = 	STEP_INPUTNEWPASSWORD;
				StartPlayRightANote();
				ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL); 
				break;
			}
			default:
			{
				break;
			}
		}
		PostMessage(hWnd,  WM_PAINT, 0, 0);
        break;
	}
	case WM_TIMER:
        switch(ASPwdModStep)
        {
            case STEP_INPUTOLDPASSWORD          :
            case STEP_INPUTNEWPASSWORD          :
            case STEP_NEW_PASSWORD_AGAIN        :
                {
                    PostMessage(hWnd,  WM_PAINT, 0, 0);
                    ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
                    break;
                }
            case STEP_OLDPASSWORDERROR          :
                {
                    ASPwdModStep = STEP_INPUTOLDPASSWORD;
			        PostMessage(hWnd,  WM_PAINT, 0, 0);
                    ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
                    break;
                }
            case STEP_NEW_PASSWORD_ERROR        :
                {
                    ASPwdModStep = STEP_INPUTNEWPASSWORD;
			        PostMessage(hWnd,  WM_PAINT, 0, 0);
			        ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
                    break;
                }
            case STEP_NEW_PASSWORD_DIGIT_ERROR  :
                {
			        ASPwdModStep = STEP_INPUTNEWPASSWORD;
                    PostMessage(hWnd,  WM_PAINT, 0, 0);
                    ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
                    break;
                }   
            case STEP_CARDNOTAUTHORIZED         :
                {
                    ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
			        PostMessage(hWnd,  WM_PAINT, 0, 0);
			        ASPwdModStep = STEP_STAMPCARD;
                    break;
                }
            case PMTMSG_ACCREDIT                :
                {
                    ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
			        PostMessage(hWnd,  WM_PAINT, 0, 0);
			        ASPwdModStep = STEP_STAMPCARD;
                    break;
                }
            
            case PMTMSG_PATROL                  :
                {
                    ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
			        PostMessage(hWnd,  WM_PAINT, 0, 0);
			        ASPwdModStep = STEP_STAMPCARD;
                    break;
                }
            
            default                             :
                {
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    break;
                }      
        }
        break;
		
	case WM_DESTROY:
		g_ASInfo.ASWorkStatus = STATUS_OPEN;
		KillTimer(hWnd, TIMER_AS_PWDMOD);
		ClearKeyBuffer();
        if(pNewPassword)
        {
            free(pNewPassword);
            printf("free the memory held by pNewPassword .\n");
        }
		pNewPassword = NULL;
		//CdInfo = NULL;
		//PostMessage(GetParent(hWnd),  WM_REFRESHPARENT_WND, 0, 0);
		//SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		break;
		
	default:
		//if (WM_CLOSE == Msg) 
		//{
		//	RemoveOneWnd(hWnd);
		//}
		//return DefWindowProc(hWnd, Msg, wParam, lParam);
		
		DefWindowProc(hWnd, Msg, wParam, lParam);
		
		if (WM_CLOSE == Msg) 
		{
			RemoveOneWnd(hWnd);
		}
	}
}

VOID
CreatePwdModWnd(HWND hwndParent)
{
	static char szAppName[] = "ASPwdModWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) ASPwdModWndProc;
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
		"AsPwdModWnd",			// Window name	(Not NULL)
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
ASPwdModKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{	
	wParam =  KeyBoardMap(wParam);
	if (wParam == KEY_RETURN)
	{
		ClearKeyBuffer();
		PostMessage(hWnd,  WM_CLOSE, 0, 0);		
	}
	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9)) 
	{
		if (STEP_INPUTOLDPASSWORD == ASPwdModStep 
			||STEP_INPUTNEWPASSWORD == ASPwdModStep
			||STEP_NEW_PASSWORD_AGAIN == ASPwdModStep)	
		{
			CircleBufEnter(g_KeyInputBuffer, &g_KeyInputBufferLen, KEY_BUF_LEN, (CHAR)wParam);
		}
	}
	else if (wParam == KEY_ENTER)
	{
		switch(ASPwdModStep) 
		{
		case STEP_INPUTOLDPASSWORD:
			{
				if (CodeCompare(g_KeyInputBuffer, g_SysConfig.SysPwd, g_KeyInputBufferLen)) 
				{
					ASPwdModStep = STEP_INPUTNEWPASSWORD;
                    ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
                    StartPlayRightANote();
                }
                else
                {
                    BYTE nOldPwd = 2;
                    SendCsnPwdModPwd2ACC(nOldPwd);
                }
				break;
			}
		case STEP_INPUTNEWPASSWORD:
			{
				if (g_KeyInputBufferLen >= MIN_PWD_LEN && g_KeyInputBufferLen <= MAX_PWD_LEN)
				{
					pNewPassword = (char*)malloc(KEY_BUF_LEN);
					memset(pNewPassword, 0, KEY_BUF_LEN);
					memcpy(pNewPassword, g_KeyInputBuffer, g_KeyInputBufferLen);
					ASPwdModStep =  STEP_NEW_PASSWORD_AGAIN;
					ResetTimer(hWnd, TIMER_AS_PWDMOD, INTERFACE_SETTLE_TIME, NULL);
					StartPlayRightANote();
				}
				else
				{
					ASPwdModStep =  STEP_NEW_PASSWORD_DIGIT_ERROR;
					ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME, NULL);
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
					ASPwdModStep = STEP_NEW_PASSWORD_ERROR;
					StartPlayErrorANote();
				}
				else
				{
					ASPwdModStep = STEP_PASSWORDMODIFYSUCCESS;
                    BYTE nNewPwd = 3;
                    SendCsnPwdModPwd2ACC(nNewPwd);
					StartPlayRightANote();
				}
				if(pNewPassword)
				{
                    free(pNewPassword);
                    printf("free the memory held by pNewPassword .\n");
				}
				pNewPassword = NULL;
                /*PROMPT_SHOW_TIME - 1000 : update g_ASInfo.ASWorkStatus before user's action of swiping card*/
                ResetTimer(hWnd, TIMER_AS_PWDMOD, PROMPT_SHOW_TIME - 1000, NULL);
				break;
			}
		default:
            {

		    }
			break;
		}
				
		ClearKeyBuffer();
	}
}

VOID
ShowPwdModWnd(BYTE* pCSN)
{

	CDINFO* CdInfo			= NULL;
	CDINFO_OLD* CdInfo_Old  = NULL;
	CDINFO pCDInfo				  ;

	if (STEP_STAMPCARD == ASPwdModStep) 
	{

		if( TRUE == ReadCard(g_HandleHashCard,pCSN,TYPE_NORMAL_CARD,&pCDInfo) )
		{
			CdInfo = &pCDInfo;
			DEBUG_CARD_PRINTF("%s,%d,TYPE_NORMAL_CARD Card existed\r\n",__func__,__LINE__);	
		}
		else if(TRUE == ReadCard(g_HandleHashCard,pCSN,TYPE_PATROL_CARD,&pCDInfo))
		{  	
			CdInfo = &pCDInfo;
			DEBUG_CARD_PRINTF("%s,%d,TYPE_PATROL_CARD Card existed\r\n",__func__,__LINE__);	
		}
		else
		{
			CdInfo_Old = AsGetCdInfobyCd(pCSN);
			if(ConversionCdInfoOld2CdInfo(&pCDInfo,CdInfo_Old))
			{
				CdInfo = &pCDInfo;
	        	printf("card found\n");
			}
		}
		//pCdInfo = AsGetCdInfobyCd(pCSN);
		PostMessage(GetFocus(), WM_CARDREAD, 0, CdInfo);
	}
}

static VOID SendCsnPwdModPwd2ACC(BYTE nPwdType) //2: old pwd , 3:new pwd 
{
    MXMSG		msgSend;
	unsigned short nDataLen = 0;
	size_t nTypeSize = sizeof(unsigned short);
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ACC;
	msgSend.dwMsg		= FC_AC_CARD_PASSWORD_MOD;
	
	nDataLen = 1/*card mode*/ + 1/*card length*/ + CSN_LEN + 1/*Pwd len*/ + PWD_LEN + 1/*VERIFYTYPE*/;
	msgSend.pParam		= (unsigned char *) malloc(nTypeSize + nDataLen);
	
	if (g_KeyInputBufferLen < PWD_LEN)
	{
		*(msgSend.pParam + nTypeSize + 2 + CSN_LEN) = g_KeyInputBufferLen;
		memcpy(msgSend.pParam + nTypeSize + 3 + CSN_LEN, g_KeyInputBuffer, g_KeyInputBufferLen);
	}
	 
	*(msgSend.pParam + nTypeSize + 3 + CSN_LEN + PWD_LEN) = nPwdType;
	
	MxPutMsg(&msgSend);	
}

