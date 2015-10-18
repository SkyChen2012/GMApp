
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	CardRead.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		13 - April - 2009
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
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"

#include "CardRead.h"
#include "AccessCommon.h"
#include "ModuleTalk.h"
#include "AccessProc.h"
#include "ParaSetting.h"

#include "AddCardWnd.h"
#include "ASProcWnd.h"
#include "ASPwdModWnd.h"
#include "DelCardModeWnd.h"
#include "MenuParaProc.h"
#include "CardProc.h"
#include "AsAlarmWnd.h"
#include "SAAlarmWnd.h"
#include "AccessLogReport.h"
#include "SecurityAlarm.h"
/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/
extern HWND g_hWNDTalk;
extern SAManStruct	SAMan;

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static	CHAR	g_CSN[CSN_LEN] = { 0 };
static	INT		g_CSNLen			   = 0;

static VOID SendSwipeCard2ACC(BYTE byCardMode);
static VOID SendCsnPwdMod2ACC(BYTE byCardMode);

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CardProc
**	AUTHOR:		Jeff Wang
**	DATE:			13 - April - 2009
**
**	DESCRIPTION:	
**			Dispatch Card Number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**			
**	RETURNED VALUE:	
**
**	NOTES:
**			
*/

VOID
CardProc()
{ 
    CDINFO *pCdInfo = NULL;	
	BYTE	CardMode;

	time_t tmCurTime = 0;
	
	
	tmCurTime = GetSysTime();
	
	g_CSNLen = ReadCSN(g_CSN, &CardMode);

	
	if (g_CSNLen != CSN_LEN
		|| ST_TALK_LEAVEPHOTO == g_TalkInfo.talking.dwTalkState
		|| g_hWndAS
		|| !g_DevFun.bCardControlUsed //For PN's not supporting swiping card
		)
	{
		if (g_CSNLen > 0)
		{
			printf("ReadCSN g_CSNLen = %d; g_CSN = %s\n",g_CSNLen,g_CSN);
			printf("g_ASInfo.ASWorkStatus = %d\n",g_ASInfo.ASWorkStatus);
		}
		return;
	}

	memcpy(g_CardUldInfo.CSN, g_CSN, CSN_LEN);
	g_CardUldInfo.nCardLen = CSN_LEN;
	g_CardUldInfo.bCardUpLoad = TRUE;

	switch(g_ASInfo.ASWorkStatus) 
	{
	case STATUS_OPEN:
		{

			if (g_hWNDTalk && ST_UK_DELAY ==  g_TalkInfo.unlock.dwUnLockShowState)
			{
				return;
			}
			SendSwipeCard2ACC(CardMode);	
	/*		
			
			pCdInfo = AsGetCdInfobyCd(g_CSN);
			AsPrintCd(pCdInfo);

			if (pCdInfo) 
			{
				pCdInfo->CardMode = CardMode;

				printf("Card OK; pCdInfo->CardType = %d; pCdInfo->CardStatus =	%d\n",pCdInfo->CardType,pCdInfo->CardStatus);
				if (TYPE_AUTHORIZE_CARD == pCdInfo->CardType) 
				{
					if (ALARM_RUN_TRIGGGER ==  SAMan.nSARunStatus) 
					{
						return;
					}
					if (CARD_STATUS_DISABLED == pCdInfo->CardStatus) 
					{
						CreateASCtrlWnd(GetFocus());
						PostMessage(GetFocus(), WM_CARDREAD, 0, pCdInfo);
					}
					else if ((tmCurTime > pCdInfo->VldEndTime && 0 != pCdInfo->VldEndTime)
						|| (tmCurTime < pCdInfo->VldStartTime && 0 != pCdInfo->VldStartTime)) 
					{
						CreateASCtrlWnd(GetFocus());
						PostMessage(g_hWndAS, WM_CARDREAD, 0, pCdInfo);
					}
					else
					{
						StartPlayRightANote();
						g_ASInfo.ASWorkStatus = STATUS_ADD_CARD;
						CreateAddCardWnd(GetFocus());					
					}
					
				}
				else if (TYPE_NORMAL_CARD == pCdInfo->CardType || TYPE_PATROL_CARD == pCdInfo->CardType)
				{
					CreateASCtrlWnd(GetFocus());
					PostMessage(g_hWndAS, WM_CARDREAD, 0, pCdInfo);
				}
				g_IvdCardSwipeTimes =	0;
			}
			else
			{
				printf("Invalid card\n");
				if ((MODE_CARD_ONLY_SET & g_ASPara.ASOpenMode)
					|| (MODE_CARD_PASSWORD_SET & g_ASPara.ASOpenMode))
				{
					RecordSwipeCardLog(CardMode, g_CSN, READER_PORT_DEFAULT, READER_ID_DEFAULT, GATE_ID_DEFAULT, 
						INOUT_DEFAULT, SWIPECARD_INVALID);					
				}
				
				IvdCdNumAlarm();				// Ë¢ÎÞÐ§¿¨±¨¾¯½ûÖ¹

				if (!g_hWndAS )
				{
					CreateASCtrlWnd(GetFocus());
					PostMessage(g_hWndAS, WM_CARDREAD, 0, pCdInfo);
				}
			}	
*/			
		}
		
		break;
		
	case STATUS_PWDMOD:
		{
			//ShowPwdModWnd(g_CSN);
			if(MODE_CARD_ONLY_SET & g_ASPara.ASOpenMode) 
			{
                PostMessage(GetFocus(),WM_CARDREAD, 0, CSN_PWD_MOD_FUN_DISABLED);
			}
            else if(MODE_CARD_PASSWORD_SET & g_ASPara.ASOpenMode) 
            {
                SendCsnPwdMod2ACC(CardMode);
		    }
		}
		break;
		
	case STATUS_ADD_CARD:
	case STATUS_ADD_AUTHORIZE_CARD:
	case STATUS_ADD_PATROL_CARD:
		{
			ShowAddCardWnd(g_CSN, CardMode);
		}
		break;
		
	case STATUS_DEL_CARD_MODE:
		{
			ShowDelCardModWnd(g_CSN);
		}
		break;
	default:
		break;
	}	


//	SendSwipeCard2ACC(CardMode);	
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CardInit
**	AUTHOR:		Jeff Wang
**	DATE:			13 - April - 2009
**
**	DESCRIPTION:	
**			Dispatch Card Number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**			
**	RETURNED VALUE:	
**
**	NOTES:
**			
*/

VOID
CardInit()
{
	ReadCardInit();
	printf("Card Init Finished\n");
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ClearCSN
**	AUTHOR:			Jeff Wang
**	DATE:		23 - April - 2009
**
**	DESCRIPTION:	
**			Clear swiped card number	
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			none
*/
VOID 
ClearCSN()
{
	memset(g_CSN, 0, CSN_LEN);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendSwipeCard2ACC
**	AUTHOR:			Wayde Zeng
**	DATE:		28 - February - 2011
**
**	DESCRIPTION:	
**			send swipe card to Access Client	
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			none
*/
static
VOID
SendSwipeCard2ACC(BYTE byCardMode)
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

	memcpy(msgSend.pParam, &nDataLen, sizeof(unsigned short));
	*(msgSend.pParam + 2) = byCardMode;
	*(msgSend.pParam + 3) = CSN_LEN;
	memcpy(msgSend.pParam + 4, g_CSN, CSN_LEN);
	*(msgSend.pParam + 9) = 1;//´®¿ÚºÅ
	*(msgSend.pParam + 10) = g_ASPara.nPatrolNum;//Õ¾ºÅ
	*(msgSend.pParam + 11) = 1;//ÃÅºÅ
	*(msgSend.pParam + 12) = PWD_LEN;
	strncpy(msgSend.pParam + 13, "", PWD_LEN); 
	*(msgSend.pParam + 26) = 0;//In Out
	*(msgSend.pParam + 27) = 0;//Verify By Card
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendCsnPwdMod2ACC
**	AUTHOR:			Michel Ma
**	DATE:		    1 - Nov - 2012
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			none
*/
static
VOID
SendCsnPwdMod2ACC(BYTE byCardMode)
{
	MXMSG		msgSend;
	unsigned short nDataLen = 0;
	size_t nTypeSize = sizeof(unsigned short);
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		    = MXMDID_ACC;
	msgSend.dwDestMd	    = MXMDID_ACC;
	msgSend.dwMsg		    = FC_AC_CARD_PASSWORD_MOD;
	
	nDataLen                = 1/*card mode*/ + 1/*card length*/ +CSN_LEN + 1/*PWD length*/ + PWD_LEN + 1/*request*/;
	msgSend.pParam		    = (unsigned char *) malloc(nTypeSize + nDataLen);

	memcpy(msgSend.pParam, &nDataLen, nTypeSize);
    *(msgSend.pParam + nTypeSize)   = byCardMode;
    *(msgSend.pParam + nTypeSize + 1)   = CSN_LEN;
	memcpy(msgSend.pParam + nTypeSize + 2, g_CSN, CSN_LEN); 
	*(msgSend.pParam + nTypeSize + 2 + CSN_LEN)   = PWD_LEN;
    strncpy(msgSend.pParam + nTypeSize + 3 + CSN_LEN, "", PWD_LEN); 
    *(msgSend.pParam + nTypeSize + 3 + CSN_LEN + PWD_LEN)  = 0;

	MxPutMsg(&msgSend);	
}


