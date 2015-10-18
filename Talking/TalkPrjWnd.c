/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	TalkPrjWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		05 - Sep - 2008
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
#include "Dispatch.h"
#include "MXCommon.h"
#include "ModuleTalk.h"
#include "MenuParaProc.h"
#include "TalkEventWnd.h"

/************** DEFINES **************************************************************/

#define	TALK_PRJ_COUNT	4
#define CALL_DELAY_TIME 2000

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

extern VOID	CodeShowFormat(HDC Hdc, RECT* pRect, UCHAR CountStart);
extern	BOOL	bPickUp;
/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID TalkPrjTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void SendGMHangupCmd2Talking();
static void	TalkPrjWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK TalkPrjWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void KeyPrjCallProcess(char *pKeyBufferValue, int *pKeyBufferLen);

/*************************************************************************************/

static VOID
TalkPrjTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (g_KeyInputBufferLen > 0) 
	{
		KeyPrjCallProcess(g_KeyInputBuffer, &g_KeyInputBufferLen);
		KillTimer(hWnd, TIMER_PRJ);
	}
	else
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
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

static void
TalkPrjWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	INT   LinkingTime = 0;
	HDC				Hdc;
	PAINTSTRUCT		ps;
	RECT			Rect;	
	char			pDateBuf[TITLE_BUF_LEN] = {0};
	INT xPos = 0;
	
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
				MXDrawText_Left(Hdc, STR_CONVERTIME_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_REMAIN_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_CONVERTIME_CN);
				MXDrawText_Left(Hdc, STR_CONVERTIME_CN, xPos, 1);
			}
			 else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_CONVERTIME));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_CONVERTIME), xPos, 0);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_REMAINING), xPos, 1);
			}	
			memset(pDateBuf, 0, TITLE_BUF_LEN);
			pDateBuf[0] =  LinkingTime / 100		 + 0x30;
			pDateBuf[1] = (LinkingTime % 100) / 10 + 0x30;
			pDateBuf[2] = (LinkingTime % 100) % 10 + 0x30;
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				strcat(pDateBuf, STR_SEC_EN);
				MXDrawText_Left(Hdc, pDateBuf, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				strcat(pDateBuf, STR_SEC_CN);
				MXDrawText_Left(Hdc, pDateBuf, xPos, 2);
			}	
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				strcat(pDateBuf,  GetHebrewStr( HS_ID_SECONDS));
				MXDrawText_Right(Hdc, pDateBuf, xPos,2);	
			}			
			break;
		}
	case ST_CI_WAIT_PICKUP:
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
#endif
			}
			else 
			{
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_PRESSANSWER_EN);
					MXDrawText_Left(Hdc, STR_HMCALL_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_EN, xPos, 2);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_CALLIN_CN);
					MXDrawText_Left(Hdc, STR_CALLIN_CN, xPos, 1);
					MXDrawText_Left(Hdc, STR_PRESSANSWER_CN, xPos, 2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_HVCALLIN));
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_HVCALLIN), xPos, 1);
					MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_PRESSANSWER), xPos, 2);
					CodeShowFormat(Hdc, &Rect, 2);
				}
				
				CodeShowFormat(Hdc, &Rect, 3);
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
		PostMessage(hWnd,  WM_SETTIMER, 2000, 0);			
		break;
	}

	if (ST_ORIGINAL == g_TalkInfo.talking.dwTalkState)
	{
		if (g_KeyInputBufferLen <= 6)
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_PHYADDR_EN);
				MXDrawText_Left(Hdc, STR_IPADDR_EN, xPos, 0);

				memset(pDateBuf, 0, TITLE_BUF_LEN);
				memcpy(pDateBuf, g_KeyInputBuffer, g_KeyInputBufferLen);
				MXDrawText_Center(Hdc, pDateBuf, 1);
				
				MXDrawText_Left(Hdc, STR_PHYADDR_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_PHYADDR_CN);
				MXDrawText_Left(Hdc, STR_IPADDR_CN, xPos, 0);
				
				memset(pDateBuf, 0, TITLE_BUF_LEN);
				memcpy(pDateBuf, g_KeyInputBuffer, g_KeyInputBufferLen);
				MXDrawText_Center(Hdc, pDateBuf, 1);

				MXDrawText_Left(Hdc, STR_PHYADDR_CN, xPos, 2);
			}	
			else if(SET_HEBREW == g_SysConfig.LangSel)
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_PHYADDRESS));
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_IPADDRESS), xPos, 0);
				
				memset(pDateBuf, 0, TITLE_BUF_LEN);
				memcpy(pDateBuf, g_KeyInputBuffer, g_KeyInputBufferLen);
				MXDrawText_Center(Hdc, pDateBuf, 1);

				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PHYADDRESS), xPos, 2);
			
			}			
			
		}
		else if	(g_KeyInputBufferLen <= 12)
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_PHYADDR_EN);
				MXDrawText_Left(Hdc, STR_IPADDR_EN, xPos, 0);
				
				memset(pDateBuf, 0, TITLE_BUF_LEN);
				memcpy(pDateBuf, g_KeyInputBuffer, 6);
				MXDrawText_Center(Hdc, pDateBuf, 1);
				
				MXDrawText_Left(Hdc, STR_PHYADDR_EN, xPos, 2);

				memset(pDateBuf, 0, TITLE_BUF_LEN);
				memcpy(pDateBuf, g_KeyInputBuffer+6, g_KeyInputBufferLen - 6);
				MXDrawText_Center(Hdc, pDateBuf, 3);	
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_PHYADDR_CN);
				MXDrawText_Left(Hdc, STR_IPADDR_CN, xPos, 0);
				
				memset(pDateBuf, 0, TITLE_BUF_LEN);
				memcpy(pDateBuf, g_KeyInputBuffer, 6);
				MXDrawText_Center(Hdc, pDateBuf, 1);
				
				MXDrawText_Left(Hdc, STR_PHYADDR_CN, xPos, 2);

				memset(pDateBuf, 0, TITLE_BUF_LEN);
				memcpy(pDateBuf, g_KeyInputBuffer+6, g_KeyInputBufferLen - 6);
				MXDrawText_Center(Hdc, pDateBuf, 3);	
			}
			else if(SET_HEBREW == g_SysConfig.LangSel)
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_PHYADDRESS));
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_IPADDRESS), xPos, 0);
				
				memset(pDateBuf, 0, TITLE_BUF_LEN);
				memcpy(pDateBuf, g_KeyInputBuffer, 6);
				MXDrawText_Center(Hdc, pDateBuf, 1);
				
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PHYADDRESS), xPos, 2);

				memset(pDateBuf, 0, TITLE_BUF_LEN);
				memcpy(pDateBuf, g_KeyInputBuffer+6, g_KeyInputBufferLen - 6);
				MXDrawText_Center(Hdc, pDateBuf, 3);	
			
			}
		}
		else if (g_KeyInputBufferLen > 12) 
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_ENTERROR_EN, 1);

			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_ENTERROR_CN, 1);
			}
			else if(SET_HEBREW == g_SysConfig.LangSel)
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERROR), 1);	
			}
			memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
			g_KeyInputBufferLen = 0;
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}	
	}

	EndPaint(hWnd, &ps);
}

static LRESULT CALLBACK
TalkPrjWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		ClearKeyBuffer();
		PostMessage(hWnd,  WM_SETTIMER, 2000, 0);	
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_PRJ, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			if (ST_UK_DELAY ==  g_TalkInfo.unlock.dwUnLockState)
			{
				TalkUlkWndPaint(hWnd, Msg, wParam, lParam);
			}
			else
			{
	//			TalkWndPaint(hWnd, Msg, wParam, lParam);
				TalkPrjWndPaint(hWnd, Msg, wParam, lParam);
			}
		}
		
		break;
		
	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		wParam =  KeyBoardMap(wParam);
		if (wParam == KEY_RETURN)
		{
			if(ST_CO_TALKING == g_TalkInfo.talking.dwTalkState
				|| ST_CO_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState
				|| ST_CI_TALKING == g_TalkInfo.talking.dwTalkState)
			{		
				SendGMHangupCmd2Talking();
			}			
			else if(ST_ORIGINAL == g_TalkInfo.talking.dwTalkState)
			{
				memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
				g_KeyInputBufferLen = 0;
				PostMessage(hWnd,  WM_CLOSE, 0, 0);
			}

		}
		
		else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9)) 
		{						
			ResetTimer(hWnd, TIMER_PRJ, 2000, NULL);
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
		
		PostMessage(hWnd,  WM_PAINT, 0, 0);		
		break;
		
	case WM_TIMER:
		TalkPrjTimerProc(hWnd, Msg, wParam, lParam);
		break;

	case WM_REFRESHPARENT_WND:
		SetFocus(hWnd);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_PRJ);
//		PostMessage(GetParent(hWnd),  WM_REFRESHPARENT_WND, 0, 0);
		g_hWNDTalk = NULL;
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

void
CreateTalkPrjWnd(HWND hwndParent)
{
	static char szAppName[] = "TalkPrjWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) TalkPrjWndProc;
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
		"TalkPrjWnd",			// Window name	(Not NULL)
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
	
	g_hWNDTalk = hWnd;
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
KeyPrjCallProcess(char *pKeyBufferValue, int *pKeyBufferLen)
{	
	MXMSG	msgSend;

	pKeyBufferValue[*pKeyBufferLen] = '\0';	
	
	memset(&msgSend, 0, sizeof(MXMSG));
	memcpy(msgSend.szDestDev, pKeyBufferValue, *pKeyBufferLen+1);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_TALKING;
	
	msgSend.dwMsg		= MXMSG_USER_GM_PRJCALLOUT;	
	
	MxPutMsg(&msgSend);

	memset(pKeyBufferValue, 0, *pKeyBufferLen);
	*pKeyBufferLen = 0;	
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






















