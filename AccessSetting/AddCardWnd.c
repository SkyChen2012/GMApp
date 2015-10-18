/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AddCardWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		24 - Aug - 2008
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
#include <time.h>

/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXMsg.h"
#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"
#include "rtc.h"

#include "AccessCommon.h"
#include "AddCardWnd.h"
#include "CardProc.h"
#include "AccessProc.h"
#include "BacpNetCtrl.h"
#include "AMT.h"
#include "hash.h"

/************** DEFINES **************************************************************/
#define TIME_TYPE_START			0
#define TIME_TYPE_END			1


#define Accecc_DEBUG	//debug printf switch
#ifdef Accecc_DEBUG
  #define DEBUG_PRINTF(...)  printf( __VA_ARGS__)
#else
	#define DEBUG_PRINTF(...)  {} 
#endif



/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/


static TIMERPROC CALLBACK	AddCardTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		AddCardWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK AddCardWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		AddCardKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static ADDCARDSTEP AddCardStep = STEP_ADD_CSN;

static VOID	CheckCardVldTime(CHAR* pTimeIn);
static VOID	LdDefLocalNum(CHAR* pLocalNumOut, DWORD* pLocalNumIn);
static VOID	ConvertID2Str(CHAR *pOutStr, DWORD nInData);
static VOID LoadDefTime(CHAR *pOutStr, time_t nDefTime);
//static VOID LoadDefRdNum(CHAR *pOutStr);
static time_t	AddCardVldTime(CHAR* pTimeIn, int nType);
static DWORD AddLocalNum(CHAR* pLocalNumIn);
static BOOL	AddRdNum(CHAR* pRdNumOut, CHAR* pRdNumIn, INT nInRdNumLen);

static CDINFO	CdInfo;
static CDINFO_OLD CdInfoOld;


static char CurDateBuffer[10] = { 0 };
static char StartDataBuffer[KEY_BUF_LEN]= { 0 };

extern HANDLE* g_HandleHashCard;

DWORD	g_LocalNum = MIN_CARD_ID;

static BYTE DataIndex = 0;

static BOOL bGetRoomCode = FALSE;

static BOOL bFirstEnt = FALSE;

HWND		g_AddCardWnd = NULL;

static CDINFO CdInfoDefault = 
{
		{
			0
		},//CSN
		TYPE_NORMAL_CARD,//CardType
		RD_NUM_DEFAULT,//RdNum
		CARD_PASSWORD_DEFAULT,//UlkPwd
		CARD_STATUS_ENABLED,//CardStatus
		
		GATE_NUMBER_DEFAULT,//GateNumber
		CARD_MODE_DEFAULT,//CardMode
		0,//UnlockTimeSliceID

		{
			0
		},//ValidStartTime		
		{
			0
		},//ValidEndTime
		0,//bAdmin
		{
			0
		}//Reserved

};


/**/
static CDINFO_OLD CdInfoDefaultOld = 
{
		{
			0
		},
		CARD_LOCALNUM_DEFAULT,
		CARD_PASSWORD_DEFAULT,
		RD_NUM_DEFAULT,
		GROUP_NUM_DEFAULT,
		
		TYPE_NORMAL_CARD,
		CARD_STATUS_ENABLED,
		CARD_MODE_DEFAULT,

		GATE_NUMBER_DEFAULT,		
		0,
		0,
		0,
	{
		0
	}

};
/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddCardTimerProc
**	AUTHOR:			Jeff Wang
**	DATE:			24 - Aug - 2008
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

static TIMERPROC CALLBACK	 
AddCardTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (STEP_ADD_SUCCESS == AddCardStep)
	{
		SendMessage(hWnd,  WM_CLOSE, 0, 0);
//		AddCardStep = STEP_ADD_CSN;
//		PostMessage(hWnd,  WM_PAINT, 0, 0);
//		ResetTimer(hWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);
	}
	else if (STEP_ADD_LOCAL_NUM_REGISTERED == AddCardStep) 
	{
		AddCardStep = STEP_ADD_LOCAL_NUM;
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		ResetTimer(hWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);
	}
	else if (STEP_ADD_RD_NUM_REGISTERED == AddCardStep) 
	{
		SendMessage(hWnd,  WM_CLOSE, 0, 0);
//		AddCardStep = STEP_ADD_RD_NUM;
//		PostMessage(hWnd,  WM_PAINT, 0, 0);
//		ResetTimer(hWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);
	}
	else if (STEP_ADD_CARD_REGISTERED == AddCardStep)
	{
		SendMessage(hWnd,  WM_CLOSE, 0, 0);
//		AddCardStep = STEP_ADD_CSN;
//		PostMessage(hWnd,  WM_PAINT, 0, 0);
//		ResetTimer(hWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);
	}
	else if (STEP_ADD_FAIL == AddCardStep)
	{
		SendMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else
	{
		if (STEP_ADD_RD_NUM == AddCardStep ||
			STEP_ADD_VLDSTART_TIME == AddCardStep ||
			STEP_ADD_VLDEND_TIME == AddCardStep)
		{
			if (g_LocalNum > 0) 
			{
				g_LocalNum --;
			}
		}
		
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
static VOID
AddCardWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
	
	switch(AddCardStep)
	{
	case STEP_ADD_CSN:
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
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_SWIPECARD), xPos, 1);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_PRESSEXIT), xPos, 2);
			}	
		}
		break;

	case STEP_ADD_CARD_REGISTERED:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_CARDWAS_EN);
				MXDrawText_Left(Hdc, STR_CARDWAS_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_REGISTER_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CARDHASREG_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CARDHASREG), 1);
			}
		}

		break;
	case STEP_ADD_LOCAL_NUM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRIDJING_EN);
				MXDrawText_Left(Hdc, STR_ETRIDJING_EN, xPos, 0);
//				MXDrawText_Left(Hdc, STR_ETRID_EN, xPos, 0);
//				MXDrawText_Left(Hdc, STR_PRESSENTER_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRID_CN);
				MXDrawText_Left(Hdc, STR_ETRID_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERLOCID), 0);	
			}	
			DrawDataParaSet(Hdc, (CHAR*)g_KeyInputBuffer, DataIndex);
		}
		break;
	case STEP_ADD_LOCAL_NUM_REGISTERED:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ISREG_EN);
				MXDrawText_Left(Hdc, STR_IDNUM_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_ISREG_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_IDNUMREG_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_IDHASREG), 0);	
			}
		}
		break;
	case STEP_ADD_RD_NUM_REGISTERED:
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
		}
		break;
	case STEP_ADD_RD_NUM:
		{
#ifdef NEW_OLED_ENABLE	
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ENTERNUM_EN);
				MXDrawText_Left(Hdc, STR_ENTERRES_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ENTERNUM_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ENTERRESNUM_CN);
				MXDrawText_Left(Hdc, STR_ENTERRESNUM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERRESNUM), 1);	
			}
			MXDrawText_Center(Hdc, (CHAR*)g_KeyInputBuffer, 2);
#else
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ENTERNUM_EN);
				MXDrawText_Left(Hdc, STR_ENTERRES_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_ENTERNUM_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ENTERRESNUM_CN);
				MXDrawText_Left(Hdc, STR_ENTERRESNUM_CN, xPos, 1);
				MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 2);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ENTERRESNUM), 1);	
			}
			MXDrawText_Center(Hdc, (CHAR*)g_KeyInputBuffer, 3);
#endif
		}
		break;
	case STEP_ADD_VLDSTART_TIME:
		{
#ifdef NEW_OLED_ENABLE	
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_VLDSTATIME_EN);
				MXDrawText_Left(Hdc, STR_ETRVLD_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_VLDSTATIME_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_ETRVLDSTATIME_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ETRVLDSTATIME), 1);	
			}	
#else
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRVLD_EN);
				MXDrawText_Left(Hdc, STR_ETRVLD_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_VLDSTATIME_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_ETRVLDSTATIME_CN, 2);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ETRVLDSTATIME), 2);	
			}
#endif
			DrawDataParaSet(Hdc, (CHAR*)g_KeyInputBuffer, DataIndex);
		}
		break;

	case STEP_ADD_VLDEND_TIME:
		{
#ifdef NEW_OLED_ENABLE	
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_VLDENDTIME_EN);
				MXDrawText_Left(Hdc, STR_ETRVLD_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_VLDENDTIME_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_ETRVLDENDTIME_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ETRVLDENDTIME), 1);	
			}			
#else
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRVLD_EN);
				MXDrawText_Left(Hdc, STR_ETRVLD_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_VLDENDTIME_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_ETRVLDENDTIME_CN, 2);
			}	
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ETRVLDENDTIME), 2);	
			}	
#endif
			if (bFirstEnt) 
			{
				bFirstEnt = FALSE;				
			}
			else
			{
				DrawDataParaSet(Hdc, (CHAR*)g_KeyInputBuffer, DataIndex);
			}
		}
		break;

	case STEP_ADD_SUCCESS:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CARDREGISTERED_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CARDREGISTERED_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CARDREGISTERED), 1);	
			}
		}
		break;		
	case STEP_ADD_FAIL:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CARDREGISTER_FAIL_EN, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_CARDREGISTER_FAIL_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
//				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_CARDREGISTERED), 1);	
			}
		}
		break;		
	default:
		break;
	}
	
	EndPaint(hWnd, &ps);
}

static BOOL
AddCard(CDINFO_OLD* pCdInfo)
{
    CDINFO_HV_t s;    
	if(pCdInfo == NULL)
	{
		printf("%s pCdInfo is NULL.\n",__FUNCTION__);
		return FALSE;
	}
    memset(&s,0,sizeof(CDINFO_HV_t));
    memcpy(&s,pCdInfo,sizeof(CDINFO_OLD));
	ConversionCdInfoOld2CdInfo(&CdInfo,&CdInfoOld);
	AsPrintCd(&CdInfo);
	if(g_ASInfo.ASWorkStatus == STATUS_ADD_CARD)
	{
		if(AsAddCd(pCdInfo))
		{
			SaveCdInfo2Mem();
		}
		else
		{
			return FALSE;
		}
		
	}
	else if(g_ASInfo.ASWorkStatus == STATUS_ADD_PATROL_CARD)
	{
		if(AsAddPatrolCd(pCdInfo))
		{
			SavePatrolCdInfo2Mem();
		}
		else
		{
			return FALSE;
		}
	}
	else if(g_ASInfo.ASWorkStatus == STATUS_ADD_AUTHORIZE_CARD)
	{
        if(isAST_HVFileExisted())
        {
            if(AsAddAuthorizeCdHV(&s))
    		{
    			SaveAuthorizeCdHVInfo2Mem();
    		}
    		else
    		{
    			return FALSE;
    		}
        }
		else
		{
			printf("%s,%d",__func__,__LINE__);
            if(AsAddAuthorizeCd(pCdInfo))
            {
                SaveAuthorizeCdInfo2Mem();
            }
            else
            {
                return FALSE;
            }
		}
	}

	

	return TRUE;
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
AddCardWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CHAR Buffer[20] = { 0 };
	//CDINFO *pCdInfo = NULL;	
	//int  nCardType = -1;
	CDINFO_HV_t sCD;
    memset(&sCD,0,sizeof(CDINFO_HV_t));
	switch (Msg)
	{
	case WM_CREATE:
		DataIndex = 0;
		bFirstEnt = FALSE;
		LoadDefTime(CurDateBuffer,GetRtcTime());				
		SaveKeyBuffer();
		if (DEVICE_CORAL_DB == g_DevFun.DevType) 
		{
			AddCardStep = STEP_ADD_CSN;
		}
		else if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType) 
		{
			AddCardStep = STEP_ADD_CSN;
			bGetRoomCode = FALSE;
		}
		else
		{
			ClearKeyBuffer();
		}
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_ADD_CARD, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			AddCardWndPaint(hWnd, Msg, wParam, lParam);		
		}
		break;

	case WM_REFRESHPARENT_WND:
		SetFocus(hWnd);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;		

	case WM_CARDREAD:
	    DEBUG_PRINTF("AddCardWndProc : WM_CARDREAD ^_^\n");
		DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
		ResetTimer(hWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);
		if (STEP_ADD_CSN == AddCardStep)
		{
			
			if (IsValidCard((BYTE*)lParam))//ÅÐ¶Ï¿¨ÊÇ·ñ´æÔÚ
			{
				DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
				AddCardStep = STEP_ADD_CARD_REGISTERED;
				ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
				StartPlayErrorANote();
			}
			else
			{	
				DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
				memcpy(CdInfoOld.CSN, (BYTE*)lParam, CSN_LEN);
//				LdDefLocalNum(Buffer, &g_LocalNum);
//				CdInfo.LocalNum = AddLocalNum(Buffer);
				memset(Buffer, 0, 20);
				ClearKeyBuffer();
				strcpy(CdInfoOld.RdNum, CORAL_DB_ROOM_CODE);
				LoadDefTime(Buffer, GetRtcTime());
				CdInfoOld.VldStartTime = AddCardVldTime(Buffer,TIME_TYPE_START);
/* 				CdInfo.ValidStartTime[0] = (BYTE)AddCardVldTime(Buffer,TIME_TYPE_START);
				CdInfo.ValidStartTime[1] = (BYTE)(AddCardVldTime(Buffer,TIME_TYPE_START)>>8);
				CdInfo.ValidStartTime[2] = (BYTE)(AddCardVldTime(Buffer,TIME_TYPE_START)>>16);
				CdInfo.ValidStartTime[3] = (BYTE)(AddCardVldTime(Buffer,TIME_TYPE_START)>>24);
*/
				memset(Buffer, 0, 20);
				CdInfoOld.VldEndTime= 0;
//				memset(CdInfo.ValidEndTime,0,sizeof(BYTE)*TIME_LEN);
                strncpy(CdInfoOld.UlkPwd,g_PUPara.ResiPwdDefault,strlen(g_PUPara.ResiPwdDefault));
                CdInfoOld.UlkPwd[strlen(g_PUPara.ResiPwdDefault)] = '\0';
                memcpy(&sCD,&CdInfoOld,sizeof(CDINFO_OLD));
                if(isAST_HVFileExisted())
                {
                    if (AsAddCdHV(&sCD))
    				{
    					ConversionCdInfoOld2CdInfo(&CdInfo,&CdInfoOld);
    					AsPrintCd(&CdInfo);
    					SaveCdHVInfo2Mem();
    					
    					AddCardStep = STEP_ADD_SUCCESS;
    					ClearKeyBuffer();
    					ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
						DEBUG_CARD_PRINTF("%s,%d,add card-HV success!^_^\n",__func__,__LINE__);
    					StartPlayRightANote();
    				}
    				else
    				{
    					DEBUG_CARD_PRINTF("%s,%d,add card-HV fail\n",__func__,__LINE__);
    					AddCardStep = STEP_ADD_FAIL;
    					ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
    					ClearKeyBuffer();
    					StartPlayErrorANote();
    				}
                }
				else
				{
                    if (AsAddCd(&CdInfoOld))
    				{
    					//ConversionCdInfoOld2CdInfo(&CdInfo,&CdInfoOld);
						 //WriteCardInfo(g_HandleHashCard,&CdInfo,CdInfo.CardType);
    					//AsPrintCd(&CdInfo);
    					//SaveCdInfo2Mem();
    					
    					AddCardStep = STEP_ADD_SUCCESS;
    					ClearKeyBuffer();
    					ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
						DEBUG_CARD_PRINTF("%s,%d,add card success!^_^\n",__func__,__LINE__);
    					StartPlayRightANote();
    				}
    				else
    				{
    					DEBUG_CARD_PRINTF("%s,%d,add card fail\n",__func__,__LINE__);
    					AddCardStep = STEP_ADD_FAIL;
    					ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
    					ClearKeyBuffer();
    					StartPlayErrorANote();
    				}
				}
			
				
			}
			
			

			PostMessage(hWnd,  WM_PAINT, 0, 0);
			/*
			if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
			{
				if (bGetRoomCode) //Add Card
				{
					printf("CDG Added Card Begin\n");					
					if (IsValidCard((BYTE*)lParam))
					{
						AddCardStep = STEP_ADD_CARD_REGISTERED;
						ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
						StartPlayErrorANote();
						printf("CDG Added Card Exist\n");					
					}
					else
					{
						printf("CDG Added Card No Exist\n");					
						memcpy(CdInfo.CSN, (BYTE*)lParam, CSN_LEN);
						
						LdDefLocalNum(Buffer, &g_LocalNum);
						CdInfo.LocalNum = AddLocalNum(Buffer);
						memset(Buffer, 0, 20);
						ClearKeyBuffer();
						LoadDefTime(Buffer, GetRtcTime());
						CdInfo.VldStartTime = AddCardVldTime(Buffer,TIME_TYPE_START);
						memset(Buffer, 0, 20);
						CdInfo.VldEndTime = 0;
						
						if (AddCard(&CdInfo))
						{
							ClearKeyBuffer();
							ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
							StartPlayRightANote();
						}
						else
						{
							ClearKeyBuffer();
							StartPlayErrorANote();
						}
						
						printf("CDG Added Card OK\n");
					}
					bGetRoomCode = FALSE;
					AddCardStep = STEP_ADD_CSN;
//					g_ASInfo.ASWorkStatus = STATUS_OPEN;
					PostMessage(hWnd,  WM_CLOSE, 0, 0);
					
				}
				else				//Delete Card
				{
					printf("CDG Deleted Card Begin \n");
					pCdInfo = AsGetCdInfobyCd((BYTE*)lParam);
					if (NULL == pCdInfo) 
					{
						printf("CDG Deleted Card Fail,  Card is Not Exist\n");
						StartPlayErrorANote();
					}
					else
					{
						if (TYPE_AUTHORIZE_CARD == pCdInfo->CardType) 
						{
							printf("CDG Deleted Card Fail, the Card is TYPE_AUTHORIZE_CARD\n");
							StartPlayErrorANote();
						}
						else
						{
							nCardType=RmCard((BYTE*)lParam);
							
							if(nCardType != -1)
							{
								SaveCardInfo2Mem(nCardType);								
								StartPlayRightANote();
								printf("CDG Deleted Card OK\n");
							}
							else
							{
								StartPlayErrorANote();
								printf("CDG Deleted Card Fail\n");
							}
						}
					}									
					AddCardStep = STEP_ADD_CSN;
//					g_ASInfo.ASWorkStatus = STATUS_OPEN;
					PostMessage(hWnd,  WM_CLOSE, 0, 0);
					
				}
			}
			else
			{
				if (IsValidCard((BYTE*)lParam))
				{
					AddCardStep = STEP_ADD_CARD_REGISTERED;
					ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
					StartPlayErrorANote();
				}
				else
				{
					memcpy(CdInfo.CSN, (BYTE*)lParam, CSN_LEN);
					AddCardStep = STEP_ADD_LOCAL_NUM;
					LdDefLocalNum(g_KeyInputBuffer, &g_LocalNum);
					StartPlayRightANote();
				}

				
	
				PostMessage(hWnd,  WM_PAINT, 0, 0);
			}
			*/
		}

		break;
		
	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		{
			if (STEP_ADD_LOCAL_NUM == AddCardStep ||
				STEP_ADD_RD_NUM == AddCardStep ||
				STEP_ADD_CSN == AddCardStep ||
				STEP_ADD_VLDSTART_TIME == AddCardStep ||
				STEP_ADD_VLDEND_TIME == AddCardStep) 
			{
                printf("AddCardWndProc : WM_CHAR\n");
				ResetTimer(hWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);		
				AddCardKeyProcess(hWnd, Msg, wParam, lParam);
			}
		}
		break;
		
	case WM_TIMER:
		AddCardTimerProc(hWnd, Msg, wParam, lParam);
		break;
		
	case WM_DESTROY:
		g_AddCardWnd = NULL;
		KillTimer(hWnd, TIMER_ADD_CARD);
		AddCardStep = STEP_ADD_CSN;
		g_ASInfo.ASWorkStatus = STATUS_OPEN;
		DataIndex = 0;
//		PostMessage(GetParent(hWnd),  WM_REFRESHPARENT_WND, 0, 0);
//		SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		LoadKeyBuffer();
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
VOID
CreateAddCardWnd(HWND hwndParent)
{
	static char szAppName[] = "AddCardWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;

	if (NULL != g_AddCardWnd) 
	{
		g_ASInfo.ASWorkStatus = STATUS_OPEN;
		return;
	}
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) AddCardWndProc;
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
		"AddCardWnd",			// Window name	(Not NULL)
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
	
	g_AddCardWnd = hWnd;
	AddCardStep = STEP_ADD_CSN;
	
	switch(g_ASInfo.ASWorkStatus) 
	{
	case STATUS_ADD_AUTHORIZE_CARD:
		{
			CdInfoDefaultOld.CardType = TYPE_AUTHORIZE_CARD;
		}
		break;

	case STATUS_ADD_CARD:
		{
			CdInfoDefaultOld.CardType = TYPE_NORMAL_CARD;
		}
		break;
	case STATUS_ADD_PATROL_CARD:
		{
			CdInfoDefaultOld.CardType = TYPE_PATROL_CARD;
		}
		break;

	default:
		break;
	}
	memcpy(&CdInfoOld, &CdInfoDefaultOld, sizeof(CDINFO_OLD));
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
AddCardKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CHAR Buffer[20] = { 0 };

	DWORD	TempLoclNum = 0;	

	wParam =  KeyBoardMap(wParam);
	if (KEY_RETURN == wParam)
	{
		memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
		g_KeyInputBufferLen = 0;
		memset(&CdInfoOld, 0, sizeof(CDINFO_OLD));
		AddCardStep = STEP_ADD_FAIL;

		SendMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if (KEY_ENTER == wParam)
	{
		DataIndex = 0;

		switch(AddCardStep)
		{
		case STEP_ADD_LOCAL_NUM:
			{
				if (DEVICE_CORAL_DB == g_DevFun.DevType)
				{
//					LdDefLocalNum(Buffer, &g_LocalNum);
//					CdInfo.LocalNum = AddLocalNum(Buffer);
					memset(Buffer, 0, 20);
					ClearKeyBuffer();
					strcpy(CdInfoOld.RdNum, CORAL_DB_ROOM_CODE);
					LoadDefTime(Buffer, GetRtcTime());
					CdInfoOld.VldStartTime= AddCardVldTime(Buffer,TIME_TYPE_START);
/*					CdInfo.ValidStartTime[0] = (BYTE)AddCardVldTime(Buffer,TIME_TYPE_START);
					CdInfo.ValidStartTime[1] = (BYTE)(AddCardVldTime(Buffer,TIME_TYPE_START)>>8);
					CdInfo.ValidStartTime[2] = (BYTE)(AddCardVldTime(Buffer,TIME_TYPE_START)>>16);
					CdInfo.ValidStartTime[3] = (BYTE)(AddCardVldTime(Buffer,TIME_TYPE_START)>>24);
*/
					memset(Buffer, 0, 20);
					CdInfoOld.VldEndTime= 0;
//					memset(CdInfo.ValidEndTime,0,sizeof(BYTE)*TIME_LEN);
					if (AsAddCd(&CdInfoOld))
					{
						ConversionCdInfoOld2CdInfo(&CdInfo,&CdInfoOld);
						AsPrintCd(&CdInfo);
						SaveCdInfo2Mem();
						
						AddCardStep = STEP_ADD_SUCCESS;
						ClearKeyBuffer();
						ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
						StartPlayRightANote();
					}
					else
					{
						AddCardStep = STEP_ADD_CSN;
						ClearKeyBuffer();
						StartPlayErrorANote();
					}
				}
				else
				{
					g_KeyInputBuffer[5] = '\0';

					TempLoclNum = ConvertStr2ID(g_KeyInputBuffer);
					
					if (IsLocalNumExist(TempLoclNum)) 
					{
						AddCardStep = STEP_ADD_LOCAL_NUM_REGISTERED;
						ClearKeyBuffer();
						StartPlayErrorANote();
//						LdDefLocalNum(g_KeyInputBuffer, &g_LocalNum);
						ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
					}
					else
					{
						if(g_ASInfo.ASWorkStatus == STATUS_ADD_CARD)
						{
//							CdInfo.LocalNum = TempLoclNum;
							AddCardStep = STEP_ADD_RD_NUM;
							ClearKeyBuffer();
							ResetTimer(hWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);
						}
						else if(g_ASInfo.ASWorkStatus == STATUS_ADD_PATROL_CARD)
						{
							AddCardStep = STEP_ADD_VLDSTART_TIME;
							ResetTimer(hWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);
							ClearKeyBuffer();
							LoadDefTime(g_KeyInputBuffer, GetRtcTime());
						}
						else if(g_ASInfo.ASWorkStatus == STATUS_ADD_AUTHORIZE_CARD)
						{
							if (AddCard(&CdInfoOld))
							{
								AddCardStep = STEP_ADD_SUCCESS;
								ClearKeyBuffer();
								ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);

								if (++g_LocalNum > MAX_CARD_ID)
								{
//									g_LocalNum = MIN_CARD_ID;
								}
								
							}	
						}
					}
				}						
			}
			break;

		case STEP_ADD_RD_NUM:
			{
				memset(CdInfoOld.RdNum, 0, RD_CODE_LEN);
				if (AddRdNum(CdInfoOld.RdNum, g_KeyInputBuffer, g_KeyInputBufferLen))
				{
					AddCardStep = STEP_ADD_VLDSTART_TIME;
					ResetTimer(hWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);
					ClearKeyBuffer();
					LoadDefTime(g_KeyInputBuffer, GetRtcTime());
				}
				else
				{
					AddCardStep = STEP_ADD_RD_NUM_REGISTERED;
					ClearKeyBuffer();
//					LoadDefRdNum(g_KeyInputBuffer);
					ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);
					StartPlayErrorANote();
				}	
			}
			break;

		case STEP_ADD_VLDSTART_TIME:
			{
				memset(StartDataBuffer,0,KEY_BUF_LEN);
				memcpy(StartDataBuffer,g_KeyInputBuffer,8);
				CdInfoOld.VldStartTime= AddCardVldTime(g_KeyInputBuffer,TIME_TYPE_START);		
/*				CdInfo.ValidStartTime[0] = (BYTE)AddCardVldTime(g_KeyInputBuffer,TIME_TYPE_START);
				CdInfo.ValidStartTime[1] = (BYTE)(AddCardVldTime(g_KeyInputBuffer,TIME_TYPE_START)>>8);
				CdInfo.ValidStartTime[2] = (BYTE)(AddCardVldTime(g_KeyInputBuffer,TIME_TYPE_START)>>16);
				CdInfo.ValidStartTime[3] = (BYTE)(AddCardVldTime(g_KeyInputBuffer,TIME_TYPE_START)>>24);
*/				ClearKeyBuffer();
				ResetTimer(hWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);
				LoadDefTime(g_KeyInputBuffer, CARD_VLD_ENDTIME_DEFAULT);
				AddCardStep = STEP_ADD_VLDEND_TIME;
				bFirstEnt = TRUE;
			}
			break;
		case STEP_ADD_VLDEND_TIME:
			{
				CdInfoOld.VldEndTime= AddCardVldTime(g_KeyInputBuffer,TIME_TYPE_END);
/*				CdInfo.ValidEndTime[0] = (BYTE)AddCardVldTime(g_KeyInputBuffer,TIME_TYPE_END);
				CdInfo.ValidEndTime[1] = (BYTE)(AddCardVldTime(g_KeyInputBuffer,TIME_TYPE_END)>>8);
				CdInfo.ValidEndTime[2] = (BYTE)(AddCardVldTime(g_KeyInputBuffer,TIME_TYPE_END)>>16);
				CdInfo.ValidEndTime[3] = (BYTE)(AddCardVldTime(g_KeyInputBuffer,TIME_TYPE_END)>>24);
*/
				if (AddCard(&CdInfoOld))
				{
	
					
					AddCardStep = STEP_ADD_SUCCESS;
					ClearKeyBuffer();
					ResetTimer(hWnd, TIMER_ADD_CARD, PROMPT_SHOW_TIME, NULL);

					if (++g_LocalNum > MAX_CARD_ID)
					{
//						g_LocalNum = MIN_CARD_ID;
					}
					
				}	
			}
			break;

		default:
				break;
		}
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		
	}

	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9))
	{
		switch(AddCardStep) 
		{
			case STEP_ADD_LOCAL_NUM:
				{
					CircleKeyEnter(g_KeyInputBuffer, &DataIndex, CD_LOCALNUM_LEN-1, (CHAR)wParam);
				}
				break;

			case STEP_ADD_RD_NUM:
				{
					printf("g_KeyInputBufferLen = %d\n",g_KeyInputBufferLen);
					g_KeyInputBuffer[g_KeyInputBufferLen++] = (CHAR)wParam;
					if (g_KeyInputBufferLen > RD_CODE_LEN-1) 
					{
						ClearKeyBuffer();
						g_KeyInputBuffer[g_KeyInputBufferLen++] = (CHAR)wParam;
					}
				}
				break;

			case STEP_ADD_VLDSTART_TIME:
				{
					CircleKeyEnter(g_KeyInputBuffer, &DataIndex, 8, (CHAR)wParam);
					CheckCardVldTime(g_KeyInputBuffer);
				}
				break;
				
			case STEP_ADD_VLDEND_TIME:
				{
					CircleKeyEnter(g_KeyInputBuffer, &DataIndex, 8, (CHAR)wParam);
					CheckCardVldTime(g_KeyInputBuffer);

					if (strcmp(StartDataBuffer,g_KeyInputBuffer) > 0) 
					{
						memcpy(g_KeyInputBuffer,StartDataBuffer,8);
					}
				}
				break;
		
			default:
				break;			
		}
		PostMessage(hWnd,  WM_PAINT, 0, 0);		
	}
	else if ((wParam >= KEY_ROOM_1) && (wParam <= KEY_ROOM_15))
	{
		if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
		{
			strcpy(CdInfoOld.RdNum, g_SysConfig.CallCode[wParam - 1]);			
			AddCardStep = STEP_ADD_CSN;
			bGetRoomCode = TRUE;
			printf("Add Card Get Room Code: %s\n",CdInfoOld.RdNum);
		}
	}

				
		
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CircKeyInput
**	AUTHOR:		   Jeff Wang
**	DATE:		9 - July - 2008
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

VOID
CircleKeyEnter(CHAR* pKeyInputBuffer, UCHAR *pDataIndex, INT MaxDataLen, CHAR KeyValue)
{
	pKeyInputBuffer[*pDataIndex] = KeyValue;
	(*pDataIndex)++;
	if (MaxDataLen == *pDataIndex)
	{
		*pDataIndex = 0;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LdDefLocalNum
**	AUTHOR:		   Jeff Wang
**	DATE:		14 - May - 2009
**
**	DESCRIPTION:	
**				Load default Local Number
**
**	ARGUMENTS:		
**				pLocalNumIn		[IN]	  CHAR*
**				pLocalNumOut    [OUT]	  WORD*
**	RETURNED VALUE:	
**	NOTES:
**
*/

static VOID
LdDefLocalNum(CHAR* pLocalNumOut, DWORD* pLocalNumIn)
{
	while (TRUE)
	{		
		if (!IsLocalNumExist(*pLocalNumIn))
		{
			ConvertID2Str(pLocalNumOut, *pLocalNumIn);					
			break;
		}
		else
		{
			(*pLocalNumIn)++;
			if ((*pLocalNumIn) > MAX_CARD_ID)
			{
				(*pLocalNumIn) = MIN_CARD_ID;
			}
		}
	}
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddLocalNum
**	AUTHOR:		   Jeff Wang
**	DATE:		15 - April - 2009
**
**	DESCRIPTION:	
**				Encrease Card ID
**
**	ARGUMENTS:		
**				pLocalNumIn		[IN]	  CHAR*
**				pLocalNumOut    [OUT]	  WORD*
**	RETURNED VALUE:	
**	NOTES:
**
*/

static DWORD
AddLocalNum(CHAR* pLocalNumIn)
{
/*	DWORD	TempLoclNum = 0;

	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
	if (strlen(pLocalNumIn) > 0)
	{
		TempLoclNum = ConvertStr2ID(pLocalNumIn);
	}
	else
	{	
		TempLoclNum = g_LocalNum;
	}

	while (TRUE)
	{
		if (!IsLocalNumExist(TempLoclNum))
		{
			DEBUG_CARD_PRINTF("%s,%d,TempLocalNum = %d\r\n",__func__,__LINE__,TempLoclNum);
			g_LocalNum = ++TempLoclNum;
			if (g_LocalNum > MAX_CARD_ID)
			{
				g_LocalNum = MIN_CARD_ID;
			}

			return TempLoclNum -1;
		}
		else
		{
			if (TempLoclNum > MAX_CARD_ID)
			{
				TempLoclNum = MIN_CARD_ID;
			}			
			TempLoclNum++;
		}
	}*/				
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddRdNum
**	AUTHOR:		   Jeff Wang
**	DATE:		15 - April - 2009
**
**	DESCRIPTION:	
**				Encrease Card ID
**
**	ARGUMENTS:		
**				pRdNumOut		[IN]	  CHAR*
**				pRdNumOut		[OUT]	 CHAR*
**	RETURNED VALUE:	
**
**	NOTES:
**
*/
static BOOL
AddRdNum(CHAR* pRdNumOut, CHAR* pRdNumIn, INT nInRdNumLen)
{
	EthResolvInfo		ResolvInfo;

	memset(&ResolvInfo,0,sizeof(EthResolvInfo));

	if (0 == nInRdNumLen)
	{
		strcpy(pRdNumOut, RD_NUM_DEFAULT);
	}
	else if ((2 == nInRdNumLen) && (0 == strcmp(pRdNumIn,"00"))) 
	{
		strcpy(pRdNumOut, RD_NUM_DEFAULT);
	}
	else
	{
		memcpy(ResolvInfo.szDevCode, pRdNumIn, nInRdNumLen);
		
		ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_CODE;
		FdFromAMTResolv(&ResolvInfo);	
		
		printf("ResolvInfo.szDevCode = %s; ResolvInfo.nType = %d\n",ResolvInfo.szDevCode,ResolvInfo.nType);
		
		if (ATM_TYPE_MC == ResolvInfo.nType || ATM_TYPE_EHV == ResolvInfo.nType || ATM_TYPE_EGM == ResolvInfo.nType)
		{
			nInRdNumLen = strlen(ResolvInfo.szDevCode);
			memcpy(pRdNumOut, ResolvInfo.szDevCode, nInRdNumLen);
			pRdNumOut[nInRdNumLen] = '\0';

			return TRUE;
		}					
		else 
		{
			return FALSE;
		}
	}

	return TRUE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddCardVldTime
**	AUTHOR:		   Jeff Wang
**	DATE:		15 - April - 2009
**
**	DESCRIPTION:	
**				Add Valid time of Card
**
**	ARGUMENTS:		
**				nTimesetOut			[OUT]	  time_t*
**				pTimeIn					[IN]		 CHAR*
**				tm_DefaultTime	  [IN]		   time_t*
**	RETURNED VALUE:	
**
**	NOTES:
**
*/
time_t
AddCardVldTime(CHAR* pTimeIn, int nType)
{
	time_t tempTime;
	struct tm tmTime = { 0 };
	
	if (strlen(pTimeIn) > 0)
	{
		tmTime.tm_year		  = 1000 * (pTimeIn[0] - 0x30) +  100 * (pTimeIn[1] - 0x30) 
												+ 10 * (pTimeIn[2] - 0x30) + (pTimeIn[3] - 0x30) - 1900;
		tmTime.tm_mon		 = (10 * (pTimeIn[4] - 0x30) +  (pTimeIn[5] - 0x30)) - 1;
		tmTime.tm_mday		= 10 * (pTimeIn[6] - 0x30) +  (pTimeIn[7] - 0x30);
		
//		tmTime.tm_hour		 = 0;
//		tmTime.tm_min		  = 0;
//		tmTime.tm_sec		  =  0;
		if (TIME_TYPE_START == nType) 
		{
			tmTime.tm_hour		= 0;
			tmTime.tm_min		= 0;
			tmTime.tm_sec		= 0;
		}
		else if (TIME_TYPE_END == nType) 
		{
			tmTime.tm_hour		= 23;
			tmTime.tm_min		= 59;
			tmTime.tm_sec		= 59;
		}
		tempTime  = mktime(&tmTime);
	}
	else
	{
		tempTime	=	0;
	}
	
	return tempTime;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CheckCardVldTime
**	AUTHOR:		   Jeff Wang
**	DATE:		15 - April - 2009
**
**	DESCRIPTION:	
**				Show Valid Data Time
**
**	ARGUMENTS:		
**				pTimeIn		[IN]	  CHAR*
**	RETURNED VALUE:	
**
**	NOTES:
**
*/
static VOID
CheckCardVldTime(CHAR* pTimeIn)
{
	INT nYear = 0;
	INT nMonth = 0;
	INT nDay = 0;

	nYear	 = 1000 * (pTimeIn[0] - 0x30) +  100 * (pTimeIn[1] - 0x30) 
						+ 10 * (pTimeIn[2] - 0x30) + (pTimeIn[3] - 0x30);

	if (2038 == nYear) 
	{
		pTimeIn[4] = '0';
		pTimeIn[5] = '1';
		pTimeIn[6] = '1';
		pTimeIn[7] = '8';
	}

	if (nYear < 2009)
	{
		pTimeIn[0] = '2';
		pTimeIn[1] = '0';
		pTimeIn[2] = '0';
		pTimeIn[3] = '9';
	}
	else if (nYear > 2099) 
	{
		pTimeIn[0] = '2';
		pTimeIn[1] = '0';
		pTimeIn[2] = '9';
		pTimeIn[3] = '9';
	}
	nMonth = 10 * (pTimeIn[4] - 0x30) +  (pTimeIn[5] - 0x30);

	if (nMonth < 1)
	{
		pTimeIn[4] = '0';
		pTimeIn[5] = '1';
	}
	else if (nMonth > 12) 
	{
		pTimeIn[4] = '1';
		pTimeIn[5] = '2';
	}
	nDay	 = 10 * (pTimeIn[6] - 0x30) +  (pTimeIn[7] - 0x30);

	if (nDay < 1)
	{
		pTimeIn[6] = '0';
		pTimeIn[7] = '1';
	}
	
	if (1 == nMonth
	  || 3 == nMonth
	  || 5 == nMonth
	  || 7 == nMonth
	  || 8 == nMonth
	  || 10 == nMonth
	  || 12 == nMonth	  
	  ) 
	{
		if (nDay > 31) 
		{
			pTimeIn[6] = '3';
			pTimeIn[7] = '1';
		}
	}
	else if (14 == nMonth
				|| 6 == nMonth
				|| 9 == nMonth
				|| 11 == nMonth
			 )
	{
		if (nDay > 30)
		{
			pTimeIn[6] = '3';
			pTimeIn[7] = '0';
		}
	}
	else if (2 == nMonth) 
	{
		if (0 == nYear % 4) 
		{
			if (nDay > 29)
			{
				pTimeIn[6] = '2';
				pTimeIn[7] = '9';
			}
		}
		else
		{
			if (nDay > 28)
			{
				pTimeIn[6] = '2';
				pTimeIn[7] = '8';
			}
		}
	}

	if(strcmp(g_KeyInputBuffer,"20380119") >= 0 || strcmp(g_KeyInputBuffer,"19000000") <= 0)
	{
		memcpy(g_KeyInputBuffer,CurDateBuffer,8);
	}

	
	pTimeIn[8] = '\0';
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ShowAddCardWnd
**	AUTHOR:		   Jeff Wang
**	DATE:		15 - April - 2009
**
**	DESCRIPTION:	
**				Show add card window
**
**	ARGUMENTS:		
**
**	RETURNED VALUE:	
**
**	NOTES:
**
*/
VOID
ShowAddCardWnd(BYTE* pCSN, BYTE CardMode)
{

	if (NULL == g_AddCardWnd) 
	{
//		printf("---------ShowAddCardWnd: NULL == g_AddCardWnd \n");
		g_ASInfo.ASWorkStatus = STATUS_OPEN;
		return;
	}

	if (STEP_ADD_FAIL == AddCardStep) 
	{
//		printf("---------ShowAddCardWnd: STEP_ADD_FAIL == AddCardStep\n");
		g_ASInfo.ASWorkStatus = STATUS_OPEN;
		return;
	}
	
	
	if (STEP_ADD_CSN == AddCardStep) 
	{
		if (GetFocus() != g_AddCardWnd) 
		{
//			printf("---------ShowAddCardWnd: Need SetFocus\n");
			SetFocus(g_AddCardWnd);
		}
		CdInfoOld.CardMode = CardMode;
		SendMessage(g_AddCardWnd, WM_CARDREAD, 0, (LPARAM)pCSN);
//		ResetTimer(g_AddCardWnd, TIMER_ADD_CARD, INTERFACE_SETTLE_TIME, NULL);
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ConvertInt2Str
**	AUTHOR:		   Jeff Wang
**	DATE:		22 - April - 2009
**
**	DESCRIPTION:	
**				Convert ID to str
**
**	ARGUMENTS:		
**
**	RETURNED VALUE:	
**
**	NOTES:
**
*/
static VOID
ConvertID2Str(CHAR *pOutStr, DWORD nInData)
{
	CHAR Temp[5 +1] = { '\0' };
	DWORD		  nTempData = 0;

	nTempData = nInData;
	Temp[0] = nTempData / 10000 + 0x30;

	nTempData %= 10000;
	Temp[1] = nTempData / 1000 + 0x30;

	nTempData %= 1000;
	Temp[2] = nTempData / 100 + 0x30;

	nTempData %= 100;
	Temp[3] = nTempData / 10 + 0x30;

	nTempData %= 10;
	Temp[4] = nTempData + 0x30;

	strcpy(pOutStr, Temp);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ConvertInt2Str
**	AUTHOR:		   Jeff Wang
**	DATE:		22 - April - 2009
**
**	DESCRIPTION:	
**				Convert ID to str
**
**	ARGUMENTS:		
**
**	RETURNED VALUE:	
**
**	NOTES:
**
*/
DWORD
ConvertStr2ID(CHAR *pIntStr)
{
	DWORD		  nTempData =	0;

	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
	
	nTempData = (pIntStr[0] - 0x30) * 10000
						  + (pIntStr[1] - 0x30)  * 1000
						  + (pIntStr[2] - 0x30)  * 100
						  + (pIntStr[3] - 0x30)  * 10
						  + (pIntStr[4] - 0x30);	

	return nTempData;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadDefTime
**	AUTHOR:		   Jeff Wang
**	DATE:		22 - April - 2009
**
**	DESCRIPTION:	
**			Load Default Time
**
**	ARGUMENTS:		
**
**	RETURNED VALUE:	
**
**	NOTES:
**
*/
static VOID
LoadDefTime(CHAR *pOutStr, time_t nDefTime)
{
	struct tm *ptmTime = 0;	
	CHAR Temp[8 +1] = { 0 };

	int nYear, nMonth, nDay = 0;
	INT		  nTempData = 0;

	ptmTime = localtime(&nDefTime);

	nYear = ptmTime->tm_year + 1900;
	nMonth = ptmTime->tm_mon+1;
	nDay = (UCHAR)(ptmTime->tm_mday);

	nTempData = nYear;
	Temp[0] = nTempData / 1000 + 0x30;

	nTempData %= 1000;
	Temp[1] = nTempData / 100 + 0x30;

	nTempData %= 100;
	Temp[2] = nTempData / 10 + 0x30;

	nTempData %= 10;
	Temp[3] = nTempData + 0x30;

	Temp[4] = nMonth / 10 + 0x30;
	Temp[5] = nMonth % 10 + 0x30;

	Temp[6] = nDay / 10 + 0x30;
	Temp[7] = nDay % 10 + 0x30;

	if (nDefTime == CARD_VLD_ENDTIME_DEFAULT) 
	{
		strcpy(pOutStr, "");
	}
	else
	{
		strcpy(pOutStr, Temp);
	}
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadDefRdNum
**	AUTHOR:		   Jeff Wang
**	DATE:		22 - April - 2009
**
**	DESCRIPTION:	
**			Load Default Time
**
**	ARGUMENTS:		
**
**	RETURNED VALUE:	
**
**	NOTES:
**
*/
/*
static VOID
LoadDefRdNum(CHAR *pOutStr)
{
	strcpy(pOutStr, RD_NUM_DEFAULT);
}
*/






















