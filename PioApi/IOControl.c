/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	IOControl.c
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
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"

#include "PioApi.h"
#include "IOControl.h"
#include "ModuleTalk.h"
#include "MenuParaProc.h"
#include "TalkLogReport.h"
#include "TalkEventWnd.h"
#include "rtc.h"
#include "AccessProc.h"
#ifdef __SUPPORT_PROTOCOL_900_1A__
#include "ParaSetting.h"
#endif 
/************** DEFINES **************************************************************/
#define IO_CONTROL_DEBUG
//#define DEBUG_IOCONTROL

#define GPIO_DETECT_TIME 200
//#define SENSITIVE_DETECT_VALID_TIME 30000
#define KEY_SENSITIVE_DETECT_VALID_TIME 1000 //For testing
#define CCD_SENSITIVE_DETECT_VALID_TIME 1000

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/
BOOL bTamperAla;

static BOOL		DOCtrlProc(MXMSG *pMsg);

static VOID KeyLedProc();
static BOOL CCDLedProc(MXMSG* pMsg);
#ifdef __SUPPORT_PROTOCOL_900_1A__
static BOOL NewDIDetect(INT nIOType);
static void NewDOInit();
#endif 
BOOL	g_bLedSts			=	FALSE;
BOOL	g_bCcdSts			=	FALSE;


static DOTimerInfo g_DO1TimerInfo;
static DOTimerInfo g_DO2TimerInfo;

//static BOOL g_DOEnable;
/*控制火警联动的一个flag*/
static BOOL g_DIFireAlarmLinkage;


static BOOL bFireAlaOn = FALSE;
DWORD  dwSendUnlockTick = 0;

static INT	iCCDOffTickCount = 0;
static BOOL bCCDLedNeedOn = FALSE;

DETECT_TIMER_t g_DIDetecter[2];
/*************************************************************************************/
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsPhotoSensitiveDetect
**	AUTHOR:			Jeff Wang
**	DATE:			05 - Sep - 2006
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/

BOOL 
IsPhotoSensitiveDetect()
{
	if (0 == PIORead(PIO_TYPE_PHOTOSENSITIVE)) 
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CCDLedCtrl
**	AUTHOR:			Jeff Wang
**	DATE:			05 - Sep - 2006
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void 
CCDLedCtrl(UCHAR LedStatus)
{
	PIOWrite(PIO_TYPE_CCD_LED, LedStatus);
#ifdef DEBUG_IOCONTROL
	if(LedStatus)
	{
		printf("CCD Led ON\n");
	}
	else
	{
		printf("CCD Led OFF\n");
	}
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	KeyLedCtrl
**	AUTHOR:			Jeff Wang
**	DATE:			05 - Sep - 2006
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void 
KeyLedCtrl(UCHAR LedStatus)
{
	PIOWrite(PIO_TYPE_KEY_LED, LedStatus);
#ifdef DEBUG_IOCONTROL
	if(LedStatus)
	{
		printf("Key Led OFF\n");
	}
	else
	{
		printf("Key Led ON\n");
	}
#endif	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DIDetect
**	AUTHOR:			Jeff Wang
**	DATE:			05 - May - 2009
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nIOTyoe		[IN]		INT*
**	RETURNED VALUE:	
**				TRUE if detected, else FALSE
**	NOTES:
**			
*/
BOOL 
DIDetect(INT nIOType)
{	
#ifdef __SUPPORT_PROTOCOL_900_1A__
	if(MOX_PROTOCOL_SUPPORT(MOX_P_900_1A))
		return NewDIDetect(nIOType);
	else
#endif 
	if(PIORead(nIOType))
	{
		return TRUE;
	}
	return FALSE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadDI1
**	AUTHOR:			Jeff Wang
**	DATE:			26 - Nov - 2008
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				0: Low Level Detect
**				1: High Level Detect
**				2: No Detect
**	NOTES:
**			
*/
BYTE 
ReadDI1()
{
	static DWORD	nCountAll 		=	0;
	static DWORD	nCountON  		=	0;
	static DWORD	nCountOFF  		=	0;
	static DWORD	PreGetTicks 	=	0;
	DWORD			GetCurTicks		=	0;
	BYTE			bRet			=	DI_NO_DETECT;
	
	nCountAll++;
	if (DIDetect(PIO_TYPE_DI1)) 
	{
		nCountON++;
	}
	else
	{
		nCountOFF++;
	}
	GetCurTicks = GetTickCount();
	
	if (GetCurTicks - PreGetTicks > GPIO_DETECT_TIME) 
	{
		PreGetTicks = GetCurTicks;
		if (nCountAll == nCountON) 
		{
			bRet	=	DI_HIGH_LEVEL;
		}
		else if (nCountAll == nCountOFF)
		{
			bRet	=	DI_LOW_LEVEL;
		}
		nCountAll	= 0;
		nCountON	= 0;
		nCountOFF	= 0;
	}
	else
	{
		bRet	=	DI_NO_DETECT;
	}

	return bRet;
}

/************************************************************************************************
**FunctionName    : ReadDI_V2
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BYTE ReadDI_V2(DETECT_TIMER_t* pTimer,INT uDIType,BOOL (*FUN)(INT))
{
    BYTE bRet = 0;
    
    if(FUN(uDIType))
    {
        /*发现DI有输入*/
        if(pTimer->Border == 0)
        {
            printf("up\n");
            /*发现一个跳变沿*/
            pTimer->Border = 1;
            pTimer->LastTickCount = GetTickCount();
            if(pTimer->UsedState == 0)
                pTimer->UsedState = 1;
            pTimer->TotalVal = pTimer->CurPosVal = pTimer->CurNegVal = 0;
        }
        ++(pTimer->CurPosVal);
    }
    else
    {
        /*发现DI没有输入*/
        if(pTimer->Border == 1)
        {
            printf("down\n");
            /*发现一个跳变沿*/
            pTimer->Border = 0;
            pTimer->LastTickCount = GetTickCount();
            if(pTimer->UsedState == 0)
                pTimer->UsedState = 2;
            pTimer->TotalVal = pTimer->CurPosVal = pTimer->CurNegVal = 0;
        }
        ++(pTimer->CurNegVal);
    }
    ++(pTimer->TotalVal);
    /*对输入做处理*/
    if(GetTickCount() - pTimer->LastTickCount > GPIO_DETECT_TIME)
    {
        if(pTimer->TotalVal == pTimer->CurPosVal)
        {
            bRet = 1;
        }
        else if(pTimer->TotalVal == pTimer->CurNegVal)
        {
            bRet = 0;
        }
        pTimer->UsedState = 0;
        pTimer->TotalVal = pTimer->CurPosVal = pTimer->CurNegVal = 0;
    }
    if(pTimer->UsedState == 1)
    {
        bRet = 0;
    }
    else if(pTimer->UsedState == 2)
    {
        bRet = 1;
    }
    return bRet;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadDI2
**	AUTHOR:			Jeff Wang
**	DATE:			26 - Nov - 2008
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
BYTE 
ReadDI2()
{
	static DWORD	nCountAll 		=	0;
	static DWORD	nCountON  		=	0;
	static DWORD	nCountOFF  		=	0;
	static DWORD	PreGetTicks 	=	0;
	DWORD			GetCurTicks		=	0;
	BOOL			bRet			=	DI_NO_DETECT;
	
	nCountAll++;
	if (DIDetect(PIO_TYPE_DI2)) 
	{
		nCountON++;
	}
	else
	{
		nCountOFF++;
	}
	GetCurTicks = GetTickCount();
	
	if (GetCurTicks - PreGetTicks > GPIO_DETECT_TIME) 
	{
		PreGetTicks = GetCurTicks;
		if (nCountAll == nCountON) 
		{
			bRet	=	DI_HIGH_LEVEL;
		}
		else if (nCountAll == nCountOFF)
		{
			bRet	=	DI_LOW_LEVEL;
		}
		nCountAll	= 0;
		nCountON	= 0;
		nCountOFF	= 0;
	}
	else
	{
		bRet	=	DI_NO_DETECT;
	}
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DOSet
**	AUTHOR:			Jeff Wang
**	DATE:			05 - May - 2009
**
**	DESCRIPTION:	
**				nIOTyoe		[IN]		INT
**				status		[IN]		BYTE
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID 
DOSet(INT nIOType, BYTE status)
{	
	PIOWrite(nIOType, status);
#ifdef DEBUG_IOCONTROL
	printf("DO Type: %d, Status: %d\n", nIOType, status);
#endif
}

static void 
LedTimeOut()
{
#ifdef IO_CONTROL_DEBUG	
	BOOL bStatus;
#endif
	int iTickCount;
	iTickCount = GetTickCount();
	if(((iTickCount-iCCDOffTickCount) > CCD_LED_ON_DELAY_TIME))
	{
		if(bCCDLedNeedOn )
		{
#ifdef IO_CONTROL_DEBUG		
			bStatus = IsPhotoSensitiveDetect();
			if(bStatus  != g_bLedSts)
			{
				printf("@@@@@@@light sensor  status change!!! bStatus=%d g_bLedSts=%d  @@@@@@@@@@@@\n",bStatus,g_bLedSts);
			}
#endif					
			bCCDLedNeedOn = FALSE;
			if(g_bLedSts)
			{
				g_bCcdSts = TRUE;
				CCDLedCtrl(CCD_LED_ON);
			}
		}
	}
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LedStsDet
**	AUTHOR:			Jeff Wang
**	DATE:			16 - Jun - 2009
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/

static void 
LedStsDet()
{
	static DWORD	nCountAll 	= 0;
	static DWORD	nCountON  	= 0;
	static DWORD	nCountOFF 	= 0;
	static DWORD	PreGetTicks 	= 0;

	DWORD GetCurTicks			= 0;

	nCountAll++;
	if (IsPhotoSensitiveDetect()) 
	{
		nCountON++;
	}
	else
	{
		nCountOFF++;	
	}
	
	GetCurTicks = GetTickCount();
	
	if (GetCurTicks - PreGetTicks > KEY_SENSITIVE_DETECT_VALID_TIME) 
	{
		PreGetTicks = GetCurTicks;
		if ((nCountAll == nCountON && !g_bLedSts) ||
			g_bCcdSts) 
		{
			g_bLedSts	=	TRUE;
		}
		else if	(nCountAll == nCountOFF && g_bLedSts)
		{
			g_bLedSts	=	FALSE;
		}
		nCountAll	= 0;
		nCountOFF	= 0;		
		nCountON	= 0;
	}
}

static void
DIPlayNote()
{
	BYTE DI1Value = DI_NO_DETECT;
	BYTE DI2Value = DI_NO_DETECT;
	static BOOL bFirstDetectDI1 = FALSE;
	static BOOL bHavePlayDI1 = FALSE;
	static DWORD PreDctStartDI1 =	 0;	
	static BOOL bFirstDetectDI2 = FALSE;
	static BOOL bHavePlayDI2 = FALSE;
	static DWORD PreDctStartDI2 = 0;
	static DWORD DI1GetCurTicks = 0;
	static DWORD DI2GetCurTicks = 0;

	if (FUN_PLAY_NOTE == g_SysConfig.DI1) 
	{
		DI1Value = ReadDI1();
		
		if (DI_HIGH_LEVEL == DI1Value)
		{
			if (!bHavePlayDI1)
			{
				if (!bFirstDetectDI1) 
				{
					bFirstDetectDI1 = TRUE;
					PreDctStartDI1 = GetTickCount();
				}
				else
				{
					DI1GetCurTicks = GetTickCount();
							
					if (DI1GetCurTicks - PreDctStartDI1 > 150)
					{
						StartPlayDI1ANote();
						bHavePlayDI1 = TRUE;
					}
				}
			}
		}
		else if (DI_LOW_LEVEL == DI1Value) 
		{
			PreDctStartDI1 = GetTickCount();
			bFirstDetectDI1 = FALSE;
			bHavePlayDI1 = FALSE;
		}
	}
	if (FUN_PLAY_NOTE == g_SysConfig.DI2) 
	{
		DI2Value = ReadDI2();
		
		if (DI_HIGH_LEVEL == DI2Value)
		{
			if (!bHavePlayDI2)
			{
				if (!bFirstDetectDI2) 
				{
					bFirstDetectDI2 = TRUE;
					PreDctStartDI2 = GetTickCount();
				}
				else
				{
					DI2GetCurTicks = GetTickCount();
							
					if (DI2GetCurTicks - PreDctStartDI2 > 150)
					{
						StartPlayDI2ANote();
						bHavePlayDI2 = TRUE;
					}
				}
			}
		}
		else if (DI_LOW_LEVEL == DI2Value) 
		{
			PreDctStartDI2 = GetTickCount();
			bFirstDetectDI2 = FALSE;
			bHavePlayDI2 = FALSE;
		}
	}	
}

static void
DOTimerInfoInit(DOTimerInfo *pDoTimerInfo)
{
	pDoTimerInfo->used = FALSE;
	pDoTimerInfo->pulseTimerTick = 0;
}

static void
CheckDOPulseTimerProc()
{
	if (g_DO1TimerInfo.used)
	{
		if ((GetTickCount() - g_DO1TimerInfo.pulseTimerTick) > (1000 * g_SysConfig.DO1_Pulse_Time))
		{
            DOSet(PIO_TYPE_DO_1,
                        (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType | g_DevFun.bNewPnUsed) 
                        ? DO_OFF : DO_ON);
			DOTimerInfoInit(&g_DO1TimerInfo);
			printf("DO1: TimeOver Close \n");
		}
	}
	if (g_DO2TimerInfo.used)
	{
		if ((GetTickCount() - g_DO2TimerInfo.pulseTimerTick) > (1000 * g_SysConfig.DO2_Pulse_Time))
		{
			DOSet(PIO_TYPE_DO_2,
                        (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType | g_DevFun.bNewPnUsed) 
                        ? DO_OFF : DO_ON);
			DOTimerInfoInit(&g_DO2TimerInfo);
			printf("DO2: TimeOver Close\n");
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LedControl
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Oct - 2008
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/

static VOID 
KeyLedProc()
{
	static BOOL		PreLedSts	=	FALSE;
	
	if (g_bLedSts && !PreLedSts) 
	{
		KeyLedCtrl(KEY_LED_ON);
		PreLedSts = g_bLedSts;
	}
	else if (!g_bLedSts && PreLedSts) 
	{
		KeyLedCtrl(KEY_LED_OFF);
		PreLedSts = g_bLedSts;
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LedControl
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Oct - 2008
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/

static BOOL 
CCDLedProc(MXMSG* pMsg)
{
	BOOL bRet = FALSE;
	BOOL bStatus ;
	if (COMM_CCDLED_ON == pMsg->dwMsg) 
	{
		if(!bCCDLedNeedOn)
		{
			bCCDLedNeedOn = TRUE;		
		}
		bRet = TRUE;
	}
	else if (COMM_CCDLED_OFF == pMsg->dwMsg) 
	{
		iCCDOffTickCount = GetTickCount();
		g_bCcdSts = FALSE;
		bCCDLedNeedOn = FALSE;
		CCDLedCtrl(CCD_LED_OFF);
		bRet = TRUE;
	}

	return bRet;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CheckDIFireAlarmLinkageProc
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Oct - 2008
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
CheckDIFireAlarmLinkageProc()
{
    //火警发生了
	if (((FUN_FIREALARM_LINKAGE == g_SysConfig.DI1) && (ReadDI_V2(g_DIDetecter,PIO_TYPE_DI1,DIDetect)/*DIDetect(PIO_TYPE_DI1)*/)) || 
        ((FUN_FIREALARM_LINKAGE == g_SysConfig.DI2) && (ReadDI_V2(g_DIDetecter + 1,PIO_TYPE_DI2,DIDetect)/*DIDetect(PIO_TYPE_DI2)*/))
        )
	{
        printf("fire alarm happens\n");
		if (!g_DIFireAlarmLinkage)
		{
			//DOTimerInfoInit(&g_DO1TimerInfo);
			//DOTimerInfoInit(&g_DO2TimerInfo);
			//g_DOEnable = TRUE; //FALSE->TRUE by [MichaelMa] at 22.8.2012 for bug 9995
			/*开启火警联动flag*/
			g_DIFireAlarmLinkage = TRUE;
            /*开启火警flag，标示火灾已经发生*/
			//bFireAlaOn = TRUE;
			dwSendUnlockTick = GetTickCount();
		}
	}
    //火警消失了
	else if (g_DIFireAlarmLinkage)
	{
        printf("fire alarm leaves\n");
        //这个变量只有DO开启了开锁联动才会置为TRUE
       /* if (g_DO1TimerInfo.used)
	    {
            printf("*****fire alarm has gone before turn off DO1*****\n");
            DOSet(PIO_TYPE_DO_1,
                        (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType | g_DevFun.bNewPnUsed) 
                        ? DO_OFF : DO_ON);
	    }
        //这个变量只有DO开启了开锁联动才会置为TRUE
	    if (g_DO2TimerInfo.used)
	    {
            printf("*****fire alarm has gone before turn off DO2*****\n");
            DOSet(PIO_TYPE_DO_2,
                        (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType | g_DevFun.bNewPnUsed) 
                        ? DO_OFF : DO_ON);
	    }
        DOTimerInfoInit(&g_DO1TimerInfo);
		DOTimerInfoInit(&g_DO2TimerInfo);
		*/       
		//g_DOEnable = FALSE; //TRUE->FALSE by [MichaelMa] at 22.8.2012 for bug 9995
		g_DIFireAlarmLinkage = FALSE;
		bFireAlaOn = FALSE;
		dwSendUnlockTick = GetTickCount();
	}
	if (g_DIFireAlarmLinkage)
	{
        //即使开锁时间少于2000，由于关门时检测到发生了火警，所以不会关门
        //查看UnlockTimeoutCtrl()
		if (GetTickCount() - dwSendUnlockTick > 2000) 
		{
			SendUnlockGate2ACC(UNLOCKTYPE_FIREALM);
                       
			dwSendUnlockTick = GetTickCount();
		}		
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IOCtrlInit
**	AUTHOR:			Jeff Wang
**	DATE:			06 - Feb - 2009
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
VOID 
IOCtrlInit(VOID)
{
	DpcAddMd(MXMDID_IOCTRL, NULL);
//	PIOInit();	

	//keyboard power off(need delay 50ms)
//	KeyboardPwrCtrl(SHELL_KEYBOARD_OFF);
	//oled power off(need delay 1500ms)
//	LcdPwrCtrl(SHELL_LCD_OFF);
	//readcard power off(need delay 100ms)
	ReadCardPwrCtrl(SHELL_READCARD_OFF);
#ifdef IO_CONTROL_DEBUG
	printf("=========power delay start,time=%ld=======\n",GetTickCount());
#endif
	usleep(3000*1000);	//at least delay 1500ms	
#ifdef IO_CONTROL_DEBUG
	printf("=========power delay end,time=%ld=======\n",GetTickCount());
#endif

	//keyboard power on
//	KeyboardPwrCtrl(SHELL_KEYBOARD_ON);
	//oled power on
//	LcdPwrCtrl(SHELL_LCD_ON);
	//readcard power on
	ReadCardPwrCtrl(SHELL_READCARD_ON);

	usleep(150*1000);	//at least delay 100ms


	CCDLedCtrl(CCD_LED_OFF);
	KeyLedCtrl(KEY_LED_OFF);

#ifdef __SUPPORT_PROTOCOL_900_1A__
	if(MOX_PROTOCOL_SUPPORT(MOX_P_900_1A))
	{
		NewDOInit();
	}
	else
#endif
	if(DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
	{
		DOSet(PIO_TYPE_DO_1, DO_OFF);
		DOSet(PIO_TYPE_DO_2, DO_OFF);
	}
	else
	{
		if (g_DevFun.bNewPnUsed) 
		{
			DOSet(PIO_TYPE_DO_1, DO_OFF);
			DOSet(PIO_TYPE_DO_2, DO_OFF);
		}
		else
		{
			DOSet(PIO_TYPE_DO_1, DO_ON);
			DOSet(PIO_TYPE_DO_2, DO_ON);
		}
	}
	
	

	DOTimerInfoInit(&g_DO1TimerInfo);
	DOTimerInfoInit(&g_DO2TimerInfo);	

	//g_DOEnable = TRUE;
	g_DIFireAlarmLinkage = FALSE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IOCtrlProc
**	AUTHOR:			Jeff Wang
**	DATE:			24 - Sep - 2008
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
VOID 
IOCtrlProc(VOID)
{
	MXMSG	msgRecev;	
	
	memset(&msgRecev, 0, sizeof(msgRecev));
	msgRecev.dwDestMd	= MXMDID_IOCTRL;
	msgRecev.pParam		= NULL;	

	LedStsDet();
	LedTimeOut();
//	TamperAlarmProc();
	KeyLedProc();

	DIPlayNote();

	if (MxGetMsg(&msgRecev))
	{
		if (!CCDLedProc(&msgRecev)) 
		{
			if (!DOCtrlProc(&msgRecev)) 
			{
				;
			}
		}

		if (NULL != msgRecev.pParam)
		{
			free(msgRecev.pParam);
			msgRecev.pParam = NULL;
		}
	}

	CheckDOPulseTimerProc();
	CheckDIFireAlarmLinkageProc();
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DOCtrlProc
**	AUTHOR:			Jeff Wang
**	DATE:			06 - Feb - 2009
**
**	DESCRIPTION:	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static BOOL 
DOCtrlProc(MXMSG *pMsg)
{
	BOOL bRet = FALSE;

	//if (!g_DOEnable) return bRet;

	switch(pMsg->dwSrcMd)
	{

		case MXMDID_IOCTRL:
			if (COMM_UNLOCK_LINKAGE_ON == pMsg->dwMsg)
			{
              
				if (FUN_LINKAGE_UNLOCK == g_SysConfig.DO1 && !g_DO1TimerInfo.used&&!bFireAlaOn)
				{
#ifdef DEBUG_IOCONTROL
					printf("DO1: FUN_LINKAGE_UNLOCK ON\n");
					printf("DO1: g_SysConfig.DO1_Pulse_Time = %d\n",g_SysConfig.DO1_Pulse_Time);
#endif
                    DOSet(PIO_TYPE_DO_1,
                        (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType | g_DevFun.bNewPnUsed) 
                        ? DO_ON : DO_OFF);

					g_DO1TimerInfo.used = TRUE;
					g_DO1TimerInfo.pulseTimerTick = GetTickCount();
					bRet = TRUE;
				}
				if (FUN_LINKAGE_UNLOCK == g_SysConfig.DO2 && !g_DO2TimerInfo.used&&!bFireAlaOn) 
				{
#ifdef DEBUG_IOCONTROL
					printf("DO2: FUN_LINKAGE_UNLOCK ON\n");
					printf("DO2: g_SysConfig.DO2_Pulse_Time = %d\n",g_SysConfig.DO1_Pulse_Time);
#endif
                    DOSet(PIO_TYPE_DO_2,
                        (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType | g_DevFun.bNewPnUsed) 
                        ? DO_ON : DO_OFF);

					g_DO2TimerInfo.used = TRUE;
					g_DO2TimerInfo.pulseTimerTick = GetTickCount();
					bRet = TRUE;
				}
			}
			 bFireAlaOn=TRUE;
			 break;

		case MXMDID_TALKING:
			if (COMM_UNLOCK_LINKAGE_ON == pMsg->dwMsg)
			{
                //如果DO1配置了开锁联动且与其相关的timer处于未被使用状态
				if (FUN_LINKAGE_UNLOCK == g_SysConfig.DO1 && !g_DO1TimerInfo.used)
				{
#ifdef DEBUG_IOCONTROL
					printf("DO1: FUN_LINKAGE_UNLOCK ON\n");
					printf("DO1: g_SysConfig.DO1_Pulse_Time = %d\n",g_SysConfig.DO1_Pulse_Time);
#endif
                    DOSet(PIO_TYPE_DO_1,
                        (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType | g_DevFun.bNewPnUsed) 
                        ? DO_ON : DO_OFF);

					g_DO1TimerInfo.used = TRUE;
					g_DO1TimerInfo.pulseTimerTick = GetTickCount();
					bRet = TRUE;
				}
				if (FUN_LINKAGE_UNLOCK == g_SysConfig.DO2 && !g_DO2TimerInfo.used) 
				{
#ifdef DEBUG_IOCONTROL
					printf("DO2: FUN_LINKAGE_UNLOCK ON\n");
					printf("DO2: g_SysConfig.DO2_Pulse_Time = %d\n",g_SysConfig.DO1_Pulse_Time);
#endif
                    DOSet(PIO_TYPE_DO_2,
                        (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType | g_DevFun.bNewPnUsed) 
                        ? DO_ON : DO_OFF);

					g_DO2TimerInfo.used = TRUE;
					g_DO2TimerInfo.pulseTimerTick = GetTickCount();
					bRet = TRUE;
				}
			}
			else if(COMM_GP_VIDEO_ON==pMsg->dwMsg)
			{
				DOSet(PIO_TYPE_DO_2, DO_ON);
			}
			else if(COMM_GP_VIDEO_OFF==pMsg->dwMsg)
			{
				DOSet(PIO_TYPE_DO_2, DO_OFF);
			}
			break;
			
		case MXMDID_LC:
			if (COMM_UNLOCK_DO1 == pMsg->dwMsg)
			{
#ifdef DEBUG_IOCONTROL
					printf("DO1: FUN_DO_LIFTCTRL_UP\n");
#endif
					if(DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
					{
						DOSet(PIO_TYPE_DO_1, DO_ON);
					}
					else
					{
						if (g_DevFun.bNewPnUsed) 
						{
							DOSet(PIO_TYPE_DO_1, DO_ON);
						}
						else
						{
							DOSet(PIO_TYPE_DO_1, DO_OFF);
						}
					}
					g_DO1TimerInfo.used = TRUE;
					g_DO1TimerInfo.pulseTimerTick = GetTickCount();
						
					bRet = TRUE;				
			}
			else if (COMM_UNLOCK_DO2 == pMsg->dwMsg)
			{
#ifdef DEBUG_IOCONTROL
					printf("DO2: FUN_DO_LIFTCTRL_UP\n");
#endif
					if(DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
					{
						DOSet(PIO_TYPE_DO_2, DO_ON);
					}
					else
					{
						if (g_DevFun.bNewPnUsed) 
						{
							DOSet(PIO_TYPE_DO_2, DO_ON);
						}
						else
						{
							DOSet(PIO_TYPE_DO_2, DO_OFF);
						}
					}
					g_DO2TimerInfo.used = TRUE;
					g_DO2TimerInfo.pulseTimerTick = GetTickCount();
					
					bRet = TRUE;					
			}
			break;
		case MXMDID_SA:

			if (COMM_OVERTIME_ALARM_ON == pMsg->dwMsg)
			{
				if(FUN_DO_ALARM_OUTPUT == g_SysConfig.DO1 )
				{
					if(DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
					{
						DOSet(PIO_TYPE_DO_1, DO_ON);
					}
					else
					{
						if (g_DevFun.bNewPnUsed) 
						{
							DOSet(PIO_TYPE_DO_1, DO_ON);
						}
						else
						{
							DOSet(PIO_TYPE_DO_1, DO_OFF);
						}
					}
				}
				if(FUN_DO_ALARM_OUTPUT == g_SysConfig.DO2 )
				{
					if(DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
					{
						DOSet(PIO_TYPE_DO_2, DO_ON);
					}
					else
					{
						if (g_DevFun.bNewPnUsed) 
						{
							DOSet(PIO_TYPE_DO_2, DO_ON);
						}
						else
						{
							DOSet(PIO_TYPE_DO_2, DO_OFF);
						}
					}
				}
			}
			else if (COMM_OVERTIME_ALARM_OFF == pMsg->dwMsg)
			{
				if(FUN_DO_ALARM_OUTPUT == g_SysConfig.DO1 )
				{
					if(DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
					{
						DOSet(PIO_TYPE_DO_1, DO_OFF);
					}
					else
					{
						if (g_DevFun.bNewPnUsed) 
						{
							DOSet(PIO_TYPE_DO_1, DO_OFF);
						}
						else
						{
							DOSet(PIO_TYPE_DO_1, DO_ON);
						}
					}
				}
				if(FUN_DO_ALARM_OUTPUT == g_SysConfig.DO2 )
				{
					if(DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
					{
						DOSet(PIO_TYPE_DO_2, DO_OFF);
					}
					else
					{
						if (g_DevFun.bNewPnUsed) 
						{
							DOSet(PIO_TYPE_DO_2, DO_OFF);
						}
						else
						{
							DOSet(PIO_TYPE_DO_2, DO_ON);
						}
					}
				}
			}
			break;
		default:
			break;
	}	
	return bRet;
}


void 
KeyboardPwrCtrl(UCHAR KbStatus)
{
	PIOWrite(PIO_TYPE_SHELL_KEYBOARD_POWER, KbStatus);
#ifdef DEBUG_IOCONTROL
	if(KbStatus)
	{
		printf("Keyboard ON\n");
	}
	else
	{
		printf("Keyboard OFF\n");
	}
#endif
}

void 
LcdPwrCtrl(UCHAR LcdStatus)
{
	PIOWrite(PIO_TYPE_SHELL_LCD_POWER, LcdStatus);
#ifdef DEBUG_IOCONTROL
	if(LcdStatus)
	{
		printf("Lcd ON\n");
	}
	else
	{
		printf("Lcd Led OFF\n");
	}
#endif
}

void 
ReadCardPwrCtrl(UCHAR ReadCardStatus)
{
	PIOWrite(PIO_TYPE_SHELL_READCARD_POWER, ReadCardStatus);
#ifdef DEBUG_IOCONTROL
	if(ReadCardStatus)
	{
		printf("Read Card ON\n");
	}
	else
	{
		printf("Read Card OFF\n");
	}
#endif
}





BOOL IsFireAlarmOn()
{
    printf("bFireAlaOn : %d\n",bFireAlaOn);
	return bFireAlaOn;
}


#ifdef __SUPPORT_PROTOCOL_900_1A__
static BOOL 
NewDIDetect(INT nIOType)
{
	int nType;
	switch(nIOType){
		case PIO_TYPE_DI1:
			nType = GetDIMode(DIPORT_1);
			break;
		case PIO_TYPE_DI2:
			nType = GetDIMode(DIPORT_2);
			break;
		default:
			nType = DIMT_NOPEN;
			break;
	}
	if(PIORead(nIOType))
	{
		if(nType == DIMT_NCLOSE)
			return FALSE;
		return TRUE;
	}
	if(nType == DIMT_NCLOSE)
		return TRUE;
	return FALSE;
}

static void NewDOInit()
{
	if(GetDOMode(DOPORT_1) == DOMT_NOPEN)
		DOSet(PIO_TYPE_DO_1, DO_OFF);
	else
		DOSet(PIO_TYPE_DO_1, DO_ON);

	if(GetDOMode(DOPORT_2) == DOMT_NOPEN)
		DOSet(PIO_TYPE_DO_2, DO_OFF);
	else
		DOSet(PIO_TYPE_DO_2, DO_ON);
}

#endif

















