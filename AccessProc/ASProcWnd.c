
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	ASProcWnd.c
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <errno.h>
#include <string.h>
#include <time.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "MXCommon.h"

#include "ASProcWnd.h"
#include "ASPwdModWnd.h"
#include "AccessCommon.h"
#include "CardProc.h"
#include "MenuParaProc.h"
#include "AccessLogReport.h"
#include "BacpNetCtrl.h"
#include "ModuleTalk.h"
#include "TalkEventWnd.h"
#include "AccessProc.h"

/************** DEFINES **************************************************************/
//#define ASPROCWND_DEBUG

/************** TYPEDEFS *************************************************************/

typedef enum _ASPMTMSGTYPE 
{
	PMTMSG_NORMAL	= 1,
	PMTMSG_FUN_DISABLE,
	PMTMSG_CARDNOTAUTHORIZED,
	PMTMSG_CARDDISABLED,
	PMTMSG_OUTOFTIMESLICE,
	PMTMSG_EXPIRED,
	PMTMSG_ENTERPASSWORD,
	PMTMSG_PASSWORDERROR,
	PMTMSG_UNLOCKSUCESS,
	PMTMSG_UNLOCKSUCESS_PATROL,
	PMTMSG_PATROLCARD
}ASPMTMSGTYPE;

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

#define	AS_DISP_COUNT	4

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID ASProcTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void ASCtrlKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void ASCtrlWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ASCtrlWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID SendCardPassword2ACC(VOID);

static ASPMTMSGTYPE ASPmtMsgType = PMTMSG_NORMAL;

HWND g_hWndAS = 0;
/*static CDINFO* CdInfo = NULL;*/


/*************************************************************************************/

static VOID
ASProcTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	KillTimer(hWnd, TIMER_AS_PROC);
	switch (ASPmtMsgType)
	{
		case PMTMSG_UNLOCKSUCESS:
		case PMTMSG_UNLOCKSUCESS_PATROL:
		{
//			UnlockGateEnd();
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		case PMTMSG_PATROLCARD:
		{
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		case PMTMSG_PASSWORDERROR:
		{
			ASPmtMsgType = PMTMSG_ENTERPASSWORD;
			PostMessage(hWnd,  WM_PAINT, 0, 0);
			ResetTimer(hWnd, TIMER_PWDUNLCOK, INTERFACE_SETTLE_TIME, NULL);
			break;
		}
		case PMTMSG_ENTERPASSWORD:
		{
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		default:
		{
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
	}
}

static void
ASCtrlWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC				Hdc;
	PAINTSTRUCT		ps;
	RECT			Rect;
	char			pDateBuf[50] = {0};	
	INT		xPos = 0;
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;
	Hdc->textcolor = FONT_COLOR;
	
	switch(ASPmtMsgType) 
	{
		case PMTMSG_FUN_DISABLE:
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
		case PMTMSG_CARDNOTAUTHORIZED:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_CARDISNOT_EN);
				MXDrawText_Left(Hdc, STR_CARDISNOT_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_AUTHORIZED_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{	
#ifdef ASPROCWND_DEBUG				
				printf("%s,PMTMSG_CARDNOTAUTHORIZED\n",__FUNCTION__);
#endif
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
		case PMTMSG_ENTERPASSWORD:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{

				MXDrawText_Center(Hdc, STR_ETRPWD_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
		
				xPos = GetXPos(STR_ETRULKPWD_CN);
				MXDrawText_Left(Hdc, STR_ETRULKPWD_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
		
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ETRULKPWD));
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_ETRULKPWD), xPos, 1);
			//	MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PRESSENTER), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);
			break;
		}
		case PMTMSG_PASSWORDERROR:
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
		case PMTMSG_UNLOCKSUCESS:
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
		case PMTMSG_PATROLCARD:
		{
			if (SET_CHINESE == g_SysConfig.LangSel) 
			{	
				MXDrawText_Center(Hdc, STR_PATROLCARD_CN, 1);
			}
			else if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_PATROLCARD_EN, 1);	
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_PATROLCARD), 1);
			}		
			break;
		}
		case PMTMSG_UNLOCKSUCESS_PATROL:
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{	
					MXDrawText_Center(Hdc, STR_PATROLCARD_CN, 1);
					MXDrawText_Center(Hdc, STR_GATEUNLOCK_CN, 2);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_PATROLCARD_EN, 1);	
					MXDrawText_Center(Hdc, STR_GATEUNLOCK_EN, 2);	
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_PATROLCARD));
					MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PATROLCARD), xPos, 0);
					MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_GATEUNLOCK), xPos, 1);
				}
				break;
			}
	default:
		break;
	}			

	EndPaint(hWnd, &ps);
}

static LRESULT CALLBACK
ASCtrlWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{	
	BYTE	byResult = 0;

	switch (Msg)
	{
	case WM_CREATE:	
		ASPmtMsgType = PMTMSG_NORMAL;
		SaveKeyBuffer();
		break;
	
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_AS_PROC, wParam, NULL);
		break;
		
	case WM_PAINT:
//		printf("-------------AS Get WM_PAINT ASPmtMsgType = %d\n",ASPmtMsgType);
		if (GetFocus() == hWnd)
		{
			ASCtrlWndPaint(hWnd, Msg, wParam, lParam);		
//			printf("-------------AS WM_PAINT \n");
		}
		break;
		
	case WM_CHAR:
//		printf("-------------AS Get WM_CHAR¡¡GetFocus = %d\n",GetFocus());
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
//		printf("-------------AS Get WM_CHAR GetFocus() = hWnd\n");
		if (PMTMSG_ENTERPASSWORD == ASPmtMsgType)
		{
			ResetTimer(hWnd, TIMER_AS_PROC, INTERFACE_SETTLE_TIME, NULL);
			ASCtrlKeyProcess(hWnd, Msg, wParam, lParam);			
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}	
		break;
		
	case WM_CARDREAD:
#ifdef ASPROCWND_DEBUG	
		printf("%s WM_CARDREAD ,ASPmtMsgType:%d,byResult:%d,g_CardTye:%d\n",__FUNCTION__,ASPmtMsgType,(BYTE)lParam,(BYTE)wParam);
#endif
		if (PMTMSG_FUN_DISABLE == ASPmtMsgType) 
		{
			return;
		} 

		ResetTimer(hWnd, TIMER_AS_PROC, INTERFACE_SETTLE_TIME, NULL);

		byResult = (BYTE)lParam;

		switch(byResult)
		{
			case SWIPECARD_UNLOCK:
			{
				ASPmtMsgType = PMTMSG_UNLOCKSUCESS;
				StartPlayRightANote();

				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case SWIPECARD_EXPIRED:
			{
				ASPmtMsgType = PMTMSG_EXPIRED;
				StartPlayErrorANote();
				
				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case SWIPECARD_INVALID:
			{
				ASPmtMsgType = PMTMSG_CARDNOTAUTHORIZED;
				StartPlayErrorANote();

				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case SWIPECARD_CARD_PATROL:
			{
				ASPmtMsgType = 	PMTMSG_PATROLCARD;
				StartPlayRightANote();
				
				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case SWIPECARD_CARD_DISABLED:
			{
				ASPmtMsgType = PMTMSG_CARDDISABLED;
				StartPlayErrorANote();

				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case SWIPECARD_PASSWORD_ERROR:
			{
				ASPmtMsgType = PMTMSG_PASSWORDERROR;
				StartPlayErrorANote();

				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				break;
			}
			case SWIPECARD_CARD_PATROL_UNLOCK:
			{
				ASPmtMsgType = 	PMTMSG_UNLOCKSUCESS_PATROL;
				StartPlayRightANote();
				
				ResetTimer(hWnd, TIMER_AS_PROC, PROMPT_SHOW_TIME, NULL);
				
				break;
			}
			case SWIPECARD_CARD_PATROL_ENTER_PASSWORD:
			case SWIPECARD_ENTER_PASSWORD:
			{
//				g_CardTye = (BYTE)wParam;

				ClearKeyBuffer();
				ASPmtMsgType = 	PMTMSG_ENTERPASSWORD;
				StartPlayRightANote();
				
				break;
			}
			case SWIPECARD_FUN_DISABLE:
			{
				ASPmtMsgType = PMTMSG_FUN_DISABLE;
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

		ASProcTimerProc(hWnd, Msg, wParam, lParam);
		break;
		
	case WM_REFRESHPARENT_WND:
		SetFocus(hWnd);
		printf("-----WM_REFRESHPARENT_WND--------hWnd = %d; GetFocus = %d\n",hWnd,GetFocus());
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;	
	
	case WM_DESTROY:
//		printf("-------------AS Get WM_DESTROY\n");
		KillTimer(hWnd, TIMER_AS_PROC);
		ClearKeyBuffer();
		g_hWndAS = 0;
		//CdInfo = NULL;
//		PostMessage(GetParent(hWnd),  WM_REFRESHPARENT_WND, 0, 0);
// 		SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		LoadKeyBuffer();
		break;
		
	default:
		
		DefWindowProc(hWnd, Msg, wParam, lParam);

		if (WM_CLOSE == Msg) 
		{
//			printf("-------------AS Get WM_CLOSE\n");
//			printf("-------------g_hWndAS = %d; GetFocus = %d\n",g_hWndAS,GetFocus());
			RemoveOneWnd(hWnd);
		}

	}
	return 0;
}

VOID
CreateASCtrlWnd(HWND hwndParent)
{
	static char szAppName[] = "TalkASWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	if (g_hWndAS)
	{
		return;
	}
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) ASCtrlWndProc;
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
		"TalkWnd",			// Window name	(Not NULL)
		WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,	// Style		(0)
		0,					// x			(0)
		0,					// y			(0)
		SCREEN_WIDTH,		// Width		
		SCREEN_HEIGHT,		// Height
		g_hMainWnd /*hwndParent*/,		// Parent		(MwGetFocus())
		NULL,				// Menu			(NULL)
		NULL,				// Instance		(NULL)
		NULL);				// Parameter	(NULL)

	AddOneWnd(hWnd,WND_CARD_PRIORITY_3);

	if (hWnd == GetFocus())
	{
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
	}
	else
	{
		ShowWindow(hWnd, SW_HIDE);
	}
	
	g_hWndAS = hWnd;
}
HWND GetASProcWnd(void)
{
	return g_hWndAS;
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
static void 
ASCtrlKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{	
	CHAR		UlkPwdStr[PWD_LEN]	= {0};

	wParam =  KeyBoardMap(wParam);
	if (wParam == KEY_RETURN)
	{
// 		if (PMTMSG_ENTERPASSWORD ==	ASPmtMsgType) 
// 		{
// 			RecordSwipeCardLog(CdInfo->CardMode, CdInfo->CSN, READER_PORT_DEFAULT, READER_ID_DEFAULT, CdInfo->GateNumber, 
// 				INOUT_DEFAULT, SWIPECARD_PASSWORD_NOT_ENTER);
// 		}
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9)) 
	{
		CircleBufEnter(g_KeyInputBuffer, &g_KeyInputBufferLen, KEY_BUF_LEN -1, (CHAR)wParam);
	}
	else if (wParam == KEY_ENTER)
	{
		if (PMTMSG_FUN_DISABLE == ASPmtMsgType) 
		{
			return;
		} 
	    SendCardPassword2ACC();
		ClearKeyBuffer();		
		//PostMessage(hWnd,  WM_CLOSE, 0, 0); //for bug 9616
	}
}

static
VOID
SendCardPassword2ACC(VOID)
{
	MXMSG		msgSend;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ACC;
	msgSend.dwMsg		= FC_AC_SWIPE_CARD;
	
	nDataLen = 1/*Card Mode*/ + 1/*CSN len*/ + CSN_LEN + 1/*Port*/ + 1/*Reader ID*/ + 1/*Gate ID*/ + 1/*Pwd len*/ + PWD_LEN + 1/*In Out*/+1/*VERIFYTYPE*/;
	msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);
	
	if (g_KeyInputBufferLen < PWD_LEN)
	{
		*(msgSend.pParam + sizeof(unsigned short) + 10) = g_KeyInputBufferLen;
		memcpy(msgSend.pParam + sizeof(unsigned short) + 11, g_KeyInputBuffer, g_KeyInputBufferLen);
	}
	 
	*(msgSend.pParam + sizeof(unsigned short) + 25) = 2;
	
	MxPutMsg(&msgSend);	
}



