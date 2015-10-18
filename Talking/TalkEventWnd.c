/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MXList.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		20 - Aug - 2008
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
#include <windows.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <MoxFB.h>
#include <device.h>

/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"
#include "ModuleTalk.h"
#include "BacpNetCtrl.h"
#include "Multimedia.h"
#include "TalkEventWnd.h"
#include "MenuParaProc.h"
#include "ASProcWnd.h"
//#include "AsAlarmWnd.h"
//#include "SecurityAlarm.h"

/************** DEFINES **************************************************************/
#define DEBUG_TALKWND

#define	TALK_DISP_COUNT	4
#define CALL_DELAY_TIME 2000

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/
extern HWND	g_HwndAsAlarm;

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/
static void KeyDBCallProcess(void);
static void KeyCodeCallProcess(char *pKeyBufferValue, int *pKeyBufferLen);

static void SendGMHangupCmd2Talking();
static void SendGMPickupCmd2Talking();
static void ShowCode_MXDrawText(HDC Hdc, RECT* pRect, char *pBuf, UCHAR Count);
void	CodeShowFormat(HDC Hdc, RECT* pRect, UCHAR CountStart);

BOOL bPickUp = FALSE;

HWND g_hWNDTalk = NULL;
static BOOL bRecallOut = FALSE;
static BOOL bLeavePhotoValid = TRUE;	//用于设置取消留图的状态
static CHAR g_KeyCalloutBack[KEY_BUF_LEN] = { 0 };
static WPARAM dwOldParam = -1;


CALLSTATUS g_CallStatus = CALLING_GM;
/*************************************************************************************/

void
TalkWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	INT   LinkingTime = 0;
	HDC				Hdc;
	PAINTSTRUCT		ps;
	RECT			Rect;	
	char			pDateBuf[TITLE_BUF_LEN] = {0};
	int					xPos = 0;
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;
	Hdc->textcolor = FONT_COLOR;
	
	switch(g_TalkInfo.talking.dwTalkState) 
	{
	case ST_CO_WAIT_CALLOUT_ACK:
	case ST_CO_WAIT_PICKUP:
		{
			if (MM_TYPE_TALK_MC ==  g_TalkInfo.talking.bMMType)
			{
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_CALLOUT_EN);
					MXDrawText_Left(Hdc, STR_CALLOUT_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_MC_EN, xPos, 2);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{		
					xPos = GetXPos(STR_MC_CN);
					MXDrawText_Left(Hdc, STR_CALLOUT_CN, xPos, 1);
					MXDrawText_Left(Hdc, STR_MC_CN, xPos, 2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_CALLOUT));
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_CALLOUT), xPos, 1);
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_MC), xPos, 2);
				}	
			}
			else if (MM_TYPE_TALK_GM ==  g_TalkInfo.talking.bMMType)
			{
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_CALLOUT_EN);
					MXDrawText_Left(Hdc, STR_CALLOUT_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_CGM_EN, xPos, 2);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{		
					xPos = GetXPos(STR_MC_CN);
					MXDrawText_Left(Hdc, STR_CALLOUT_CN, xPos, 1);
					MXDrawText_Left(Hdc, STR_CGM_CN, xPos, 2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_CALLOUT));
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_CALLOUT), xPos, 1);
					MXDrawText_Right(Hdc,"CGM", xPos, 2);
				}	
			}
			else
			{
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_CALLOUT_EN, 1);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{	
					MXDrawText_Center(Hdc, STR_CALLOUT_CN, 1);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CALLOUT), 1);
				}	
				CodeShowFormat(Hdc, &Rect, 2);
			}			 
			break;
		}
		
	case ST_CO_TALKING:
	case ST_CI_TALKING:
		{
			LinkingTime = g_SysConfig.TalkTime - ((g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer) / 1000);
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_CONVERTIME_EN);
				MXDrawText_Left(Hdc, STR_CONVERTIME_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_REMAIN_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_CONVERTIME_CN);
				MXDrawText_Left(Hdc, STR_CONVERTIME_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_CONVERTIME));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_CONVERTIME), xPos, 1);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_REMAINING), xPos, 2);
			}	 
			memset(pDateBuf, 0, TITLE_BUF_LEN);
			pDateBuf[0] =  LinkingTime / 100		 + 0x30;
			pDateBuf[1] = (LinkingTime % 100) / 10 + 0x30;
			pDateBuf[2] = (LinkingTime % 100) % 10 + 0x30;
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				strcat(pDateBuf, STR_SEC_EN);
				MXDrawText_Left(Hdc, pDateBuf, xPos, 3);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				strcat(pDateBuf, STR_SEC_CN);
				MXDrawText_Left(Hdc, pDateBuf, xPos, 2);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				strcat(pDateBuf,  GetHebrewStr( HS_ID_SECONDS));
				MXDrawText_Right(Hdc, pDateBuf,xPos, 2);	
			}			
			break;
		}
	case ST_CI_WAIT_PICKUP:
	case ST_CI_WAIT_PICKUP_ACK:
		{
			if (MM_TYPE_TALK_MC == g_TalkInfo.talking.bMMType)
			{
#ifdef NEW_OLED_ENABLE	
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_PRESSANSWER_EN);
					MXDrawText_Left(Hdc, STR_MCCALL_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_EN, xPos, 2);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_MC_CN);
					MXDrawText_Left(Hdc, STR_MC_CN, xPos, 0);
					MXDrawText_Left(Hdc, STR_CALLIN_CN, xPos, 1);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_CN, xPos, 3);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_MCCALLIN));
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_MCCALLIN), xPos, 1);
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_PRESSANSWER), xPos, 2);
				}	
#else
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_PRESSANSWER_EN);
					MXDrawText_Left(Hdc, STR_MCCALL_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_EN, xPos, 2);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_MC_CN);
					MXDrawText_Left(Hdc, STR_MC_CN, xPos, 1);
					MXDrawText_Left(Hdc, STR_CALLIN_CN, xPos, 2);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_CN, xPos, 4);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_MCCALLIN));
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_MCCALLIN), xPos, 1);
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_PRESSANSWER), xPos, 2);
				}
#endif
			}
			else if (MM_TYPE_TALK_GM == g_TalkInfo.talking.bMMType)
			{
#ifdef NEW_OLED_ENABLE
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_PRESSANSWER_EN);
					MXDrawText_Left(Hdc, STR_CGMCALL_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_EN, xPos, 2);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_CALLIN_CN);
					MXDrawText_Left(Hdc, STR_CGM_CN, xPos, 0);
					MXDrawText_Left(Hdc, STR_CALLIN_CN, xPos, 1);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_CN, xPos, 3);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_PRESSANSWER));
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_CGMCALLIN), xPos, 1);
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_PRESSANSWER), xPos, 2);
				}
#else
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_PRESSANSWER_EN);
					MXDrawText_Left(Hdc, STR_CGMCALL_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_EN, xPos, 2);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_CALLIN_CN);
					MXDrawText_Left(Hdc, STR_CGM_CN, xPos, 1);
					MXDrawText_Left(Hdc, STR_CALLIN_CN, xPos, 2);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_CN, xPos, 4);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_PRESSANSWER));
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_CGMCALLIN), xPos, 1);
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_PRESSANSWER), xPos, 2);
				}
#endif
			}
			else 
			{
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_PRESSANSWER_EN);
					MXDrawText_Left(Hdc, STR_HMCALL_EN, xPos, 0);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_EN, xPos, 1);
					CodeShowFormat(Hdc, &Rect, 2);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_HVCALLIN_CN);
					MXDrawText_Left(Hdc, STR_HVCALLIN_CN, xPos, 0);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_CN, xPos, 1);
					CodeShowFormat(Hdc, &Rect, 2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_HVCALLIN));
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_HVCALLIN), xPos, 0);
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_PRESSANSWER), xPos, 1);
					CodeShowFormat(Hdc, &Rect, 2);
				}
				
				
			}			
			break;
		}
	case ST_CO_CALL_FAIL:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TRYAGAIN_EN);
				MXDrawText_Left(Hdc, STR_LINEBUSY_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_TRYAGAIN_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TRYAGAIN_CN);
				MXDrawText_Left(Hdc, STR_LINEBUSY_CN, xPos, 1);
				MXDrawText_Left(Hdc, STR_TRYAGAIN_CN, xPos, 2);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_TRYAGAIN));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_LINEBUSY), xPos, 1);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_TRYAGAIN), xPos, 2);
			}
			break;
		}
	case ST_CI_WAIT_HANGUP_ACK:
	case ST_CO_WAIT_HANGUP_ACK:
	case ST_TALK_END:
		{
			if (!bPickUp) 
			{
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_CALLEND_EN, 1);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_CALLEND_CN, 1);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CALLEND), 1);	
				}	
			}
			else
			{
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_CONVEREND_EN, 1);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_CONVEREND_CN, 1);
				}	
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr( HS_ID_CONVEREND), 1);	
				}				
			}
			break;
		}
	case ST_CO_NO_ANSWER:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TRYAGAIN_EN);
				MXDrawText_Left(Hdc, STR_NOANSWER_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_TRYAGAIN_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TRYAGAIN_CN);
				MXDrawText_Left(Hdc, STR_NOANSWER_CN, xPos, 1);
				MXDrawText_Left(Hdc, STR_ANSWERLATER_CN, xPos, 2);
			}	
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ANSWERLATER));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_NOANSWER), xPos, 1);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ANSWERLATER), xPos, 2);
			}
			break;
		}
	case ST_TALK_LEAVEPHOTO:
	case ST_TALK_WAIT_LEAVEPHOTO:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TRYLATER_EN);
				MXDrawText_Left(Hdc, STR_IMGCAP_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_TRYLATER_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ANSWERLATER_CN);
				MXDrawText_Left(Hdc, STR_IMGCAP_CN, xPos, 1);
				MXDrawText_Left(Hdc, STR_ANSWERLATER_CN, xPos, 2);
			}	
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ANSWERLATER));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_IMGCAP), xPos, 1);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ANSWERLATER), xPos, 2);
			}
			break;
		}

	case ST_CO_CALL_CODE_INEXIST:
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
			else if(SET_HEBREW == g_SysConfig.LangSel)
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_NOTREGISTER));
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_NOTREGISTER), xPos, 1);	
			}
			break;
		}
	default:
		break;
	}
				
	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysInfoWndPaint
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
static void
TalkUlkWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC				Hdc;
	PAINTSTRUCT		ps;
	RECT			Rect;	
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
    
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;
	Hdc->textcolor = FONT_COLOR;
	
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
	EndPaint(hWnd, &ps);
}


static LRESULT CALLBACK
TalkWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	
	
	switch (Msg)
	{
	case WM_CREATE:
//		printf("g_TalkInfo.unlock.dwUnLockShowState = %d\n",g_TalkInfo.unlock.dwUnLockShowState);
		bRecallOut = FALSE;
		bLeavePhotoValid = TRUE;

//		memcpy(g_KeyCalloutBack, g_KeyInputBuffer, KEY_BUF_LEN);

		if (ST_UK_DELAY !=  g_TalkInfo.unlock.dwUnLockShowState)
		{
			switch(g_CallStatus) 
			{
			case CALLING_DB_EHV:
				{
					KeyDBCallProcess();
				}
				break;
			case CALLING_DB_MC:
				{
					KeyCodeCallProcess(g_KeyInputBuffer, &g_KeyInputBufferLen);
				}
				break;
			case CALLING_GM:
				{
					KeyCodeCallProcess(g_KeyInputBuffer, &g_KeyInputBufferLen);
				}
				break;
			case CALLED:
				break;
			default:
				PostMessage(hWnd,  WM_CLOSE, 0, 0);
				break;
			}
			
			ClearKeyBuffer();		
		}
		break;
		
	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			if (ST_UK_DELAY ==  g_TalkInfo.unlock.dwUnLockShowState)
			{
				TalkUlkWndPaint(hWnd, Msg, wParam, lParam);
			}
			else
			{
				TalkWndPaint(hWnd, Msg, wParam, lParam);
			}
		}
		break;
		
	case WM_CHAR:
//		if (ST_UK_DELAY ==  g_TalkInfo.unlock.dwUnLockState) 

		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		if (ST_UK_DELAY ==  g_TalkInfo.unlock.dwUnLockShowState)
		{
//			SendMessage(g_hWNDTalk, WM_CLOSE, 0, 0);		
			break;
		}

/*
		wParam =  KeyBoardMap(wParam);
		if (wParam == KEY_RETURN)
		{
			if(ST_CO_TALKING == g_TalkInfo.talking.dwTalkState
				|| ST_CO_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState
				|| ST_CI_TALKING == g_TalkInfo.talking.dwTalkState)
			{		
				SendGMHangupCmd2Talking();
			}
			else if (ST_CI_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState) 
			{
				SendGMPickupCmd2Talking();
			}
		}
*/
#ifdef	DEBUG_TALKWND
		printf("%s,wParam=%x,DevType=%d,dwTalkState=%d\n",__FUNCTION__,wParam,g_DevFun.DevType,g_TalkInfo.talking.dwTalkState);
#endif
		if(ST_CO_TALKING == g_TalkInfo.talking.dwTalkState
			|| ST_CO_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState
			|| ST_CI_TALKING == g_TalkInfo.talking.dwTalkState)
		{	
			wParam =  KeyBoardMap(wParam);
			
			if (wParam == KEY_RETURN || isDBCallKeyPressed(wParam))
			{
				if(ST_CO_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState)
				{
					bLeavePhotoValid = FALSE;
				}
				SendGMHangupCmd2Talking();
			}	
			if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType && ST_CO_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState)
			{
				if (wParam >= KEY_ROOM_1 && wParam <= KEY_ROOM_15) 
				{
					if (!IsAlarming()) 
					{
						if (0 != strcmp(g_KeyCalloutBack,g_SysConfig.CallCode[wParam - 1]))
						{
#ifdef	DEBUG_TALKWND							
							printf("*********call***********wParam=%x\n",wParam);
#endif
							bLeavePhotoValid = FALSE;
							dwOldParam = wParam;
							memcpy(g_KeyCalloutBack, g_SysConfig.CallCode[wParam - 1], KEY_BUF_LEN);
							SendGMHangupCmd2Talking();
							bRecallOut = TRUE;
						}
						else
						{
#ifdef	DEBUG_TALKWND							
							printf("*********invalid call***********\n");
#endif
						}
//						PostMessage(g_hMainWnd,  WM_CHAR, wParam, 0);
					}
					else
					{
						printf("Alarm Status!!!!!!\n");
					}
					
				}
			}
		}
		else if (ST_CI_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState) 
		{
			wParam =  KeyBoardMap(wParam);
			if (wParam == KEY_RETURN)
			{
				SendGMPickupCmd2Talking();
			}
		}

		break;
		
	case WM_TIMER:
		break;

	case WM_REFRESHPARENT_WND:
		SetFocus(hWnd);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_DESTROY:
#ifdef	DEBUG_TALKWND		
		printf("%s destroy\n",__FUNCTION__);
#endif
		ClearKeyBuffer();
//		SendMessage(GetParent(g_hWNDTalk),  WM_REFRESHPARENT_WND, 0, 0);
		g_hWNDTalk = NULL;		
//		SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		break;
		
	default:

		DefWindowProc(hWnd, Msg, wParam, lParam);
		
		if (WM_CLOSE == Msg) 
		{
			RemoveOneWnd(hWnd);
		}
//		return 
	}
	return 0;
}

void
CreateTalkWnd(HWND hwndParent)
{
	static char szAppName[] = "Talk";
	HWND		hWnd;
	WNDCLASS	WndClass;

	if (g_hWNDTalk)
	{
		return;
	}


/*
	if (NULL != g_HwndAsAlarm) 
	{
		return;
	}
	else
	{
		KillChildWnd(g_hMainWnd);
	}
*/



	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) TalkWndProc;
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
	

	AddOneWnd(hWnd,WND_CALL_PRIORITY_2);
	if (hWnd == GetFocus())
	{
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
	}
	else
	{
		ShowWindow(hWnd, SW_HIDE);
	}
	
#ifdef DEBUG_TALKWND
	printf("Talk Wnd create successfully\n");
#endif
	g_hWNDTalk = hWnd;
	bRecallOut = FALSE;

}


void
ShowTalkWnd(void)
{
    if (g_hWndAS)
	{
        /*Inputing PWD window's priority is higher than calling window.
          Only destroying the former can make the latter closer to user.
          Special case:when taling in or talking out happened,break execution.
          */
        if(
            ST_CO_TALKING != g_TalkInfo.talking.dwTalkState &&
            ST_CI_TALKING != g_TalkInfo.talking.dwTalkState
        )
        {
            SendMessage(g_hWndAS, WM_CLOSE, 0, 0);
        }
	}
	g_TalkInfo.Timer.dwCurrentTick	=	GetTickCount();

	if (ST_CI_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState)
	{
		g_CallStatus = CALLED;
		
	}
	else if (ST_CO_TALKING == g_TalkInfo.talking.dwTalkState 
		|| ST_CI_TALKING == g_TalkInfo.talking.dwTalkState)
	{
		bPickUp = TRUE;
	}

	if (g_hWNDTalk) 
	{
		SendMessage(g_hWNDTalk, WM_PAINT, 0, 0);
	}
	else
	{
		CreateTalkWnd(g_hMainWnd);
	}
}

void
WndHideTalkWindow(void)
{
	if (g_hWNDTalk)
	{
		bPickUp = FALSE;
		SendMessage(g_hWNDTalk, WM_CLOSE, 0, 0);		
	}
}

void ShowTalkUlkWnd(void)
{
    if (g_hWndAS)
	{
		SendMessage(g_hWndAS, WM_CLOSE, 0, 0);
	}
	if (g_hWNDTalk) 
	{
		g_TalkInfo.unlock.dwUnLockShowState = ST_UK_DELAY;
        g_TalkInfo.unlock.dwUnLockState = ST_UK_DELAY;
		g_TalkInfo.Timer.dwUnlockShowTimer = GetTickCount();
		SendMessage(g_hWNDTalk, WM_PAINT, 0, 0);
	}
	else
	{
		g_TalkInfo.unlock.dwUnLockShowState = ST_UK_DELAY;
		g_TalkInfo.Timer.dwUnlockShowTimer = GetTickCount();
		CreateTalkWnd(g_hMainWnd);
	}
}

void
HideTalkUlkWnd(void)
{
	if (g_hWNDTalk) 
	{
		if (ST_ORIGINAL == g_TalkInfo.talking.dwTalkState) 
		{
			SendMessage(g_hWNDTalk, WM_CLOSE, 0, 0);
		}
		else
		{
			SendMessage(g_hWNDTalk, WM_PAINT, 0, 0);
		}
	}
}

void 
SetCalloutKey(char * keybuff)
{
	strcpy(g_KeyCalloutBack,keybuff);
}

BOOL
CheckRecallKey()
{
	return bRecallOut;
}

BOOL 
GetLeavePhotoStatus()
{
	return bLeavePhotoValid;
}

void
ClearRecallKey()
{
	bRecallOut=FALSE;
}


DWORD
GetRecallKey()
{
	return dwOldParam;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	KeyDBCallProcess
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
KeyDBCallProcess(void)
{	
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	
	strcpy(msgSend.szDestDev, CORAL_DB_ROOM_CODE);
	
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_TALKING;
	
	msgSend.dwMsg		= MXMSG_USER_GM_CODECALLOUT;	
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	KeyCodeCallProcess
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
KeyCodeCallProcess(char *pKeyBufferValue, int *pKeyBufferLen)
{	
	MXMSG	msgSend;

	pKeyBufferValue[*pKeyBufferLen] = '\0';
	
	memset(&msgSend, 0, sizeof(MXMSG));

	memcpy(msgSend.szDestDev, pKeyBufferValue, *pKeyBufferLen+1);
	
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_TALKING;
	
	msgSend.dwMsg		= MXMSG_USER_GM_CODECALLOUT;	
	
	MxPutMsg(&msgSend);

}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendGMHangupCmd2Talking
**	AUTHOR:		   Jeff Wang
**	DATE:		29 - July - 2008
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
static void SendGMHangupCmd2Talking()
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_TALKING;
	
	msgSend.dwMsg		= FC_HANGUP_GM;

	msgSend.dwParam		= FLAG_SELF_GM;
	
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendGMPickupCmd2Talking
**	AUTHOR:		   Jeff Wang
**	DATE:		31 - July - 2008
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
SendGMPickupCmd2Talking()
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_TALKING;
	
	msgSend.dwMsg		= FC_PICKUP_GM;
	
	msgSend.dwParam		= 0;
	
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
}


void
CodeShowFormat(HDC Hdc, RECT* pRect, UCHAR CountStart)
{
	char pTemp[TITLE_BUF_LEN] = { 0 };
	char        pDstCode[19+1] = { 0 };
	int				nTempLen = 0;
	
	nTempLen = strlen(g_TalkInfo.talking.szTalkDestDevCode);

	if (g_SysConfig.VldCodeDigits  > nTempLen)
	{
		strcpy(pDstCode, g_TalkInfo.talking.szTalkDestDevCode);
	}
	else
	{
		memcpy(pDstCode, &g_TalkInfo.talking.szTalkDestDevCode[nTempLen-g_SysConfig.VldCodeDigits],g_SysConfig.VldCodeDigits);
	}

	if (nTempLen <= 8) 
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pDstCode, nTempLen);
		ShowCode_MXDrawText(Hdc, pRect, pTemp, CountStart);
	}
	else if ((nTempLen > 8) && (nTempLen <= 16))
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pDstCode, 8);	
		ShowCode_MXDrawText(Hdc, pRect, pTemp, CountStart);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pDstCode+8, nTempLen-8);
		ShowCode_MXDrawText(Hdc, pRect, pTemp, CountStart+1);
	}
	else
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pDstCode, 8);
		ShowCode_MXDrawText(Hdc, pRect, pTemp, CountStart);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pDstCode+8, 8);
		ShowCode_MXDrawText(Hdc, pRect, pTemp, CountStart+1);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pDstCode+16, nTempLen-16);
		ShowCode_MXDrawText(Hdc, pRect, pTemp, CountStart+2);
	}
}
static void 
ShowCode_MXDrawText(HDC Hdc, RECT* pRect, char *pBuf, UCHAR Count)
{
	if (SET_HEBREW == g_SysConfig.LangSel) 
	{
		MXDrawText_Hebrew(Hdc,pRect,pBuf,Count)	;
	}
	else
	{
		MXDrawText_Cn(Hdc,pRect,pBuf,Count)	;
	}
}

/************************************************************************************************
**FunctionName    : isDBCallKeyPressed
**Function        : return TRUE when the CALL key is pressed while GM is waiting HV's pickup.
**InputParameters : wParam : the key pressed by user
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BOOL isDBCallKeyPressed(WPARAM wParam)
{
    BOOL bRet = FALSE;
    if((DEVICE_CORAL_DB == g_DevFun.DevType) && (KEY_DB_CALL == wParam))
    {
        bRet = TRUE;
    }
    return bRet;
}


