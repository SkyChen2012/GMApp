
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AccessProc.c
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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"

#include "AccessCommon.h"
#include "AccessProc.h"
#include "CardProc.h"
#include "IOControl.h"
#include "PioApi.h"
#include "MenuParaProc.h"
#include "TalkEventWnd.h"
#include "ModuleTalk.h"
#include "AsAlarmWnd.h"
#include "SAAlarmWnd.h"
#include "AccessLogReport.h"
#include "ParaSetting.h"
#include "UnlockWnd.h"
#include "TalkLogReport.h"
#include "hash.h"


/************** DEFINES **************************************************************/

#define DEBUG_AS_INIT
#define DEBUG_AS_PROC
#define ACC_DEBUG
#define ACCESSPROC_DEBUG

/************** TYPEDEFS *************************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

#define DOOR_OPEN	1
#define DOOR_CLOSE	0

#define LOCAL_IP	0x7f000001//127.0.0.1

//static BOOL		bFirstDetectDI1	=   FALSE;
static BOOL		bLow2High		=   FALSE;
static DWORD	PreDctStartDI1	=	0;
static BOOL     bLastDetectHighDI1 = FALSE;
	
//static BOOL		bFirstDetectDI2	=   FALSE;
static BOOL		bLow2High2		=   FALSE;
static DWORD	PreDctStartDI2	=	0;
static BOOL     bLastDetectHighDI2 = FALSE;

extern HANDLE*   g_HandleHashCard ;//卡片存储的句柄
BOOL bGateAla;

//extern UCHAR   SyncDataBuf[SYNCDATABUFSIZE];
/*
ASINFO g_AHASHCARDInfo =
{
	STATUS_OPEN,
	
	{
		AST_VERSION,
		0
	 },

	 &SyncDataBuf[0]
};
*/

ASINFO	g_ASInfo = 
{	
	STATUS_OPEN,
	
	{
		AST_VERSION,
		0
	 },

	 NULL
};

ASINFO	g_AS_HVInfo = 
{	
	STATUS_OPEN,
	
	{
		AST_VERSION,
		0
	 },

	 NULL
};

ASINFO	g_APInfo = 
{	
	STATUS_OPEN,
		
	{
		APT_VERSION,
			0
	},
	
	NULL
};

ASINFO	g_AAInfo = 
{	
	STATUS_OPEN,
		
	{
		APT_VERSION,
			0
	},
	
	NULL
};

ACCINFO	g_ACCInfo = 
{
    {0},
	{0},
	0,
	0x0,
	0,
	{0}
};

BOOL	bFirstDetectDI_R[2] = {FALSE, FALSE};
DWORD	PreDctStartDI_R[2]  = { 0 };

BOOL	bFirstDetectDI_F[2] = {FALSE, FALSE};
DWORD	PreDctStartDI_F[2]  = { 0 };

BOOL	bLastDoorSta;


BOOL	bFlag_R[2] = {FALSE, FALSE};
BOOL	bFlag_F[2] = {FALSE, FALSE};

BOOL	g_bTamperAlarmOn    =	FALSE;

//static  BOOL bOpenAlarmIO1 = FALSE;
//static  BOOL bOpenAlarmIO2 = FALSE;

static	RDPWDINFO g_RdPwdInfo;
static  CSNPWDINFO g_CsnPwdInfo;
static	UnlockINFO g_UnlockInfo;
static	BYTE g_UnlockType = 0;

static	UCHAR g_IvdCardSwipeTimes = 0;
static VOID AcClearResource(MXMSG *pmsg);
static VOID SetGateStateFlag();
static VOID GateStateDIProc();
static VOID GateStateProc();

static VOID GateOpenOvertimeProc();
static VOID ManSwichProc();
static VOID InfraredAlarmProc();
static VOID	AsSendOpenGateAck2MC(MXMSG* pMsg);

static VOID ACCStateTimeOut(VOID);

static BOOL AccSwipeCard(MXMSG *pmsg);
static BOOL AccRdPwdUnlock(MXMSG *pmsg);
static BOOL AccRdPwdMod(MXMSG *pmsg);
static BOOL AccUnlockGate(MXMSG *pmsg);
static BOOL AccCsnPwdMod(MXMSG *pmsg);

static BOOL SwipeCardOriginalPro(MXMSG *pmsg);
static BOOL WaitSwipeCardAckPro(MXMSG *pmsg);
static BOOL RdPwdUnlockOriginalPro(MXMSG* pmsg);
static BOOL WaitRdPwdUnlockAckPro(MXMSG* pmsg);
static BOOL CsnPwdModOriginalPro();
static BOOL WaitCsnPwdModAckPro(MXMSG *pmsg);
static BOOL RdPwdModOriginalPro(MXMSG* pmsg);
static BOOL WaitQueryRdPwdAckPro(MXMSG* pmsg);
static BOOL EnterRdNewPwdPro(MXMSG* pmsg);
static BOOL WaitModRdPwdAckPro(MXMSG* pmsg);
static BOOL UnlockOriginalPro(MXMSG *pmsg);
static BOOL WaitUnlockAckPro(MXMSG *pmsg);

static VOID ACCSendSwipeCard2ACA(MXMSG *pmsg);
static VOID ACCSendRdPwdUnlock2ACA(MXMSG* pmsg);
static VOID ACCSendUnlockGate2ACA(MXMSG *pmsg);
static VOID ACCSendRdPwdCheck2ACA(MXMSG* pmsg);
static VOID ACCSendRdPwdMod2ACA(MXMSG* pmsg);
static VOID ACCSendCsnPwdMod2ACA(MXMSG *pmsg);

static BOOL IsPatrolCardSwipe(BYTE* pCSN);
static BOOL IsPatrolCardUnlock(BYTE* pCSN);

static VOID ACCSendCSNCallLift2LC(BYTE CSN[CSN_LEN]);
static VOID ACCSendRDCODECallLift2LC(char CallCode[RD_CODE_LEN]);
static VOID ACCSendCALLCODECallLift2LC(char CallCode[RD_CODE_LEN]);
static VOID MultiDoorOpenHandle(MXMSG *pmsg);

static VOID IvdCdNumAlarm(VOID);
#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
static BOOL AccessPrePro(MXMSG *pmsg);
#endif
static void UlkBehaviourHandle(void);
static BOOL init_AST_HV_space(void);
/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AccessProc
**	AUTHOR:		   Jeff Wang
**	DATE:		13 - April - 2009
**
**	DESCRIPTION:
**				
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**
**	NOTES:
**	
*/

VOID 
AccessProc()
{
	MXMSG	msgRecev;
	//BOOL	bRet = FALSE;
	
	memset(&msgRecev, 0, sizeof(msgRecev));
	msgRecev.dwDestMd	= MXMDID_ACC;
	msgRecev.pParam		= NULL;	
	
	CardProc();	
	ManSwichProc();
	GateStateProc();
	//	GateOpenOvertimeProc();
	//	InfraredAlarmProc();
	//	TamperAlarmProc();
	
	AsLogReportTimeOut();
	ACCStateTimeOut();
	
	if (MxGetMsg(&msgRecev))
	{
#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
		AccessPrePro(&msgRecev);
#endif
		if (!AsLogProc(&msgRecev))
		{
			if (!AccSwipeCard(&msgRecev))
			{
                if(!AccCsnPwdMod(&msgRecev))
                {
				    if (!AccRdPwdUnlock(&msgRecev))
				    {
					    if (!AccRdPwdMod(&msgRecev))
					    {
						    if (!AccUnlockGate(&msgRecev))
						    {
							    ;
						    }						
					    }
				    }
                }
			}					
		}
		
		AcClearResource(&msgRecev);	
	}
	
	if (	IsMCOnline() 
			&& IsReportIdle())
	{
		AsLogReport();
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsSendOpenGateAck2MC
**	AUTHOR:		    Jeff Wang
**	DATE:		    11 - May - 2009
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**	
**	NOTES:
**	
*/
static VOID
AsSendOpenGateAck2MC(MXMSG* pMsg)
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, pMsg->szDestDev);
	strcpy(msgSend.szDestDev, pMsg->szSrcDev);
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_AC_OPEN_GATE;
	msgSend.dwParam		= pMsg->dwParam;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsSendOpenGateAck2MC
**	AUTHOR:		    Jeff Wang
**	DATE:		    11 - May - 2009
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**	
**	NOTES:
**	
*/
static VOID
AsPollReport(BYTE Status)
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_AC_POLL;
	msgSend.dwParam		= g_TalkInfo.dwMCIP;
	msgSend.pParam		= (BYTE*)malloc(1);
	*(msgSend.pParam)	= Status;
	
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AccessInit
**	AUTHOR:		    Jeff Wang
**	DATE:		    7 - July - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**	
**	NOTES:
**	
*/

VOID
AccessInit()
{
	DpcAddMd(MXMDID_ACC, NULL);
	
	g_APInfo.MXASBuf = (CHAR*)malloc(MAX_AS_PCD_CNT * sizeof(CDINFO_OLD));
	memset(g_APInfo.MXASBuf, 0, MAX_AS_PCD_CNT * sizeof(CDINFO_OLD));
	g_APInfo.ASTHead.nCardCnt = 0;
            


	bGateAla  = FALSE;
	
	g_CardPWDInfo.MXPWDBuf = (CHAR*)malloc(MAX_AS_CD_CNT * sizeof(CDPWDINFO));
	memset(g_CardPWDInfo.MXPWDBuf, 0, MAX_AS_CD_CNT * sizeof(CDPWDINFO));
	g_CardPWDInfo.PWDHead.nCardCnt = 0;

	bLastDetectHighDI1 = DIDetect(PIO_TYPE_DI1);
	bLastDetectHighDI2 = DIDetect(PIO_TYPE_DI2);

	memset(&g_ACCInfo, 0, sizeof(g_ACCInfo));
	strcpy(g_ACCInfo.szACCCode, g_ASPara.szACCDevCode);
	strcpy(g_ACCInfo.szACACode, "");
	g_ACCInfo.dwACCState = ST_ACC_ORIGINAL;
	
	if(g_ASPara.bEnableAS )
	{
		g_ACCInfo.dwACAIP=LOCAL_IP;//127.0.0.1
	}
	else
	{
		g_ACCInfo.dwACAIP = g_ASPara.dwAccServerIP;
	}

	g_ACCInfo.dwTickCount = GetTickCount();

	memset(&g_RdPwdInfo, 0, sizeof(RDPWDINFO));
	strcpy(g_RdPwdInfo.szACCCode, g_ASPara.szACCDevCode);
	strcpy(g_RdPwdInfo.szACACode, "");
	g_RdPwdInfo.dwACCState = ST_ACC_RDPWD_ORIGINAL;
	
	if(g_ASPara.bEnableAS )
	{
		g_RdPwdInfo.dwACAIP = LOCAL_IP;
	}
	else
	{
		g_RdPwdInfo.dwACAIP = g_ASPara.dwAccServerIP;
	}

	g_RdPwdInfo.dwTickCount = GetTickCount();

	memset(&g_UnlockInfo, 0, sizeof(UNLOCKINFO));
	strcpy(g_UnlockInfo.szACCCode, g_ASPara.szACCDevCode);
	strcpy(g_UnlockInfo.szACACode, "");
	g_UnlockInfo.dwACCState = ST_ACC_UNLOCK_ORIGINAL;
	if(g_ASPara.bEnableAS )
	{
		g_UnlockInfo.dwACAIP = LOCAL_IP;
	}
	else
	{
		g_UnlockInfo.dwACAIP = g_ASPara.dwAccServerIP;
	}
	
	g_UnlockInfo.dwTickCount = GetTickCount();

    memset(&g_CsnPwdInfo, 0, sizeof(CSNPWDINFO));
	strcpy(g_CsnPwdInfo.szACCCode, g_ASPara.szACCDevCode);
	strcpy(g_CsnPwdInfo.szACACode, "");
	g_CsnPwdInfo.dwACCState = ST_ACC_CSNPWD_ORIGINAL;
	
	if(g_ASPara.bEnableAS )
	{
		g_CsnPwdInfo.dwACAIP = LOCAL_IP;
	}
	else
	{
		g_CsnPwdInfo.dwACAIP = g_ASPara.dwAccServerIP;
	}

	g_CsnPwdInfo.dwTickCount = GetTickCount();

	if (FUN_GATE_OPEN_TIMEOVER == g_SysConfig.DI1)
	{
		if (g_ASPara.GateOpenContactor)
		{
			bLastDoorSta = DIDetect(PIO_TYPE_DI1);
		}
		else
		{
			bLastDoorSta = !DIDetect(PIO_TYPE_DI1);
		}
	}
	else if (FUN_GATE_OPEN_TIMEOVER == g_SysConfig.DI2)
	{
		if (g_ASPara.GateOpenContactor)
		{
			bLastDoorSta = DIDetect(PIO_TYPE_DI2);
		}
		else
		{
			bLastDoorSta = !DIDetect(PIO_TYPE_DI2);
		}
	}

    if(isAST_HVFileExisted())
    {
        if(init_AST_HV_space())
        {
            LoadCdHVInfoFromFile();
        }
        if(NULL != (g_AAInfo.MXASBuf = (CHAR*)malloc(MAX_AS_ACD_CNT * sizeof(CDINFO_HV_t))))
        {        
        	memset(g_AAInfo.MXASBuf,0,MAX_AS_ACD_CNT * sizeof(CDINFO_HV_t));
        	g_AAInfo.ASTHead.nCardCnt = 0;
            LoadAuthorizeCdHVInfoFromMem();            
        }
    }
    else
    {
        g_ASInfo.MXASBuf = (CHAR*)malloc(MAX_AS_CD_CNT * sizeof(CDINFO));
    	memset(g_ASInfo.MXASBuf, 0, MAX_AS_CD_CNT * sizeof(CDINFO));
    	g_ASInfo.ASTHead.nCardCnt = 0;

        g_AAInfo.MXASBuf = (CHAR*)malloc(MAX_AS_ACD_CNT * sizeof(CDINFO));
    	memset(g_AAInfo.MXASBuf, 0, MAX_AS_ACD_CNT * sizeof(CDINFO));
    	g_AAInfo.ASTHead.nCardCnt = 0;
        LoadCdInfoFromMem();
        LoadAuthorizeCdInfoFromMem();
    }
	LoadPatrolCdInfoFromMem();
	

	/*if(isAHASHFILEExisted())
	{
		SyncHashFile2RAM();
	}else
	{
		ClearAllCardData();
	}*/
	g_HandleHashCard = OpenCardDataBase();//初始化 hashcard

	UPdata_AST();
	UPdata_APT();


	CardInit();

	AsLogInit();

#ifdef DEBUG_AS_INIT
	printf("AccessSystem Init...\n");
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AccessExit
**	AUTHOR:			Jeff Wang
**	DATE:			13 - April - 2009
**
**	DESCRIPTION:	
**			Access Exit
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**					
**	RETURNED VALUE:	
**			
**	NOTES:
**			
*/
VOID
AccessExit()
{
	AsRmAllCd();
	SaveCdInfo2Mem();
	SavePatrolCdInfo2Mem();
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ManSwichProc
**	AUTHOR:			Jeff Wang
**	DATE:			05 - May - 2009
**
**	DESCRIPTION:	
**	Process Mannual Switch
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID 
ManSwichProc()
{
	BYTE    byDI1Value      =   DI_NO_DETECT;
    BYTE    byDI2Value      =   DI_NO_DETECT;
    BYTE    byTrigger       =   0x00;//No trigger    

    if(g_ASPara.GateOpenManFlag)
    {
        if(FUN_MANUAL_SWITCH == g_SysConfig.DI1) 
        {
            byDI1Value = DIDetect(PIO_TYPE_DI1);
            if(DI_HIGH_LEVEL == byDI1Value)
			{					
				if(!bLastDetectHighDI1)
				{
					bLow2High = TRUE;
					PreDctStartDI1 = GetTickCount();
				}
				if(bLow2High && GetTickCount() - PreDctStartDI1 > 200)
				{
					bLow2High = FALSE;
                    byTrigger = 0x01;//trigger
                    printf("DI1: Manually Unlock\n");
				}
			}
			else if(DI_LOW_LEVEL == byDI1Value) 
			{
				bLow2High = FALSE;				
				PreDctStartDI1 = GetTickCount();
			}
			bLastDetectHighDI1 = byDI1Value;
        }
        if(FUN_MANUAL_SWITCH == g_SysConfig.DI2)
        {
            byDI2Value	= DIDetect(PIO_TYPE_DI2);
            if(DI_HIGH_LEVEL == byDI2Value)
			{					
				if(!bLastDetectHighDI2) 
				{
					bLow2High2 = TRUE;					
					PreDctStartDI2 = GetTickCount();
				}
				if(bLow2High2 && GetTickCount() - PreDctStartDI2 > 200)
				{
					bLow2High2 = FALSE;					
					byTrigger = 0x01;//trigger
					printf("DI2: Manually Unlock\n");
				}
			}
			else if(DI_LOW_LEVEL == byDI2Value) 
			{
				bLow2High2 = FALSE;					
				PreDctStartDI2 = GetTickCount();
			}
			bLastDetectHighDI2 = byDI2Value;
        }
    }

    if(!!byTrigger)
    {
        SendUnlockGate2ACC(UNLOCKTYPE_MANUAL);
    }
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SetGateStateFlag
**	AUTHOR:			Wayde Zeng
**	DATE:			08 - Dec - 2009
**
**	DESCRIPTION:	
**	Process gate state log
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID
SetGateStateFlag()
{
	static	BYTE	preDIValue1 = DI_NO_DETECT;
	static	BYTE	preDIValue2 = DI_NO_DETECT;
	
	BYTE	DIValue1 = DI_NO_DETECT;
	BYTE	DIValue2 = DI_NO_DETECT;

	// DI1
	if (FUN_GATE_OPEN_TIMEOVER == g_SysConfig.DI1)
	{		
		DIValue1 = ReadDI1();

		if (DI_NO_DETECT == DIValue1)
		{
			DIValue1 = preDIValue1;
		}

		if (DI_LOW_LEVEL == preDIValue1 && DI_HIGH_LEVEL == DIValue1)
		{
			printf("DI1 flag ascend\n");
			bFlag_R[0] = TRUE;
		}
		else if (DI_HIGH_LEVEL == preDIValue1 && DI_LOW_LEVEL == DIValue1)
		{
			printf("DI1 flag descend\n");
			bFlag_F[0] = TRUE;
		}

		preDIValue1 = DIValue1;
	}
	// DI2
	if (FUN_GATE_OPEN_TIMEOVER == g_SysConfig.DI2)
	{
		DIValue2 = ReadDI2();

		if (DI_NO_DETECT == DIValue2)
		{
			DIValue2 = preDIValue2;
		}
		
		if (DI_LOW_LEVEL == preDIValue2 && DI_HIGH_LEVEL == DIValue2)
		{
			printf("DI2 flag ascend\n");
			bFlag_R[1] = TRUE;
		}
		else if (DI_HIGH_LEVEL == preDIValue2 && DI_LOW_LEVEL == DIValue2)
		{
			printf("DI2 flag descend\n");
			bFlag_F[1] = TRUE;
		}
		
		preDIValue2 = DIValue2;
	}

}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateStateDIProc
**	AUTHOR:			Wayde Zeng
**	DATE:			08 - Dec - 2009
**
**	DESCRIPTION:	
**	Process gate state log
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID
GateStateDIProc()
{
	BYTE	DI1Value_R = DI_NO_DETECT;
	BYTE	DI1Value_F = DI_NO_DETECT;
	BYTE	DI2Value_R = DI_NO_DETECT;
	BYTE	DI2Value_F = DI_NO_DETECT;

	DWORD		CurTickCount =	0;	
	
	// DI1
	if (FUN_GATE_OPEN_TIMEOVER == g_SysConfig.DI1) 
	{
		// ascend
		if (TRUE == bFlag_R[0])
		{
			DI1Value_R = DIDetect(PIO_TYPE_DI1);
			if (DI_HIGH_LEVEL == DI1Value_R)
			{
				if (FALSE == bFirstDetectDI_R[0]) 
				{
					bFirstDetectDI_R[0] = TRUE;
					PreDctStartDI_R[0] = GetTickCount();
				}
				
				CurTickCount = GetTickCount();
				if (CurTickCount - PreDctStartDI_R[0] > GATE_STA_TIME)
				{
					bFirstDetectDI_R[0] = FALSE;
			
					if (g_ASPara.GateOpenContactor)
					{
						if (DOOR_CLOSE == bLastDoorSta) 
						{
							printf("DI1 gate open log\n");
							RecordDoorStsLog(TYPE_GATE_OPEN, GATE_ID_DEFAULT);
                            UlkBehaviourHandle();
							bLastDoorSta = DOOR_OPEN;
						}
					}
					else
					{
						if (DOOR_OPEN == bLastDoorSta) 
						{
							printf("DI1 gate close log\n");
							RecordDoorStsLog(TYPE_GATE_CLOSE, GATE_ID_DEFAULT);
							bLastDoorSta = DOOR_CLOSE;
						}
					}					
					
					bFlag_R[0]  = FALSE;
				}				
			}
		}
		// descend
		if (TRUE == bFlag_F[0])
		{
			DI1Value_F = DIDetect(PIO_TYPE_DI1);
			if (DI_LOW_LEVEL == DI1Value_F)
			{
				if (FALSE == bFirstDetectDI_F[0]) 
				{
					bFirstDetectDI_F[0] = TRUE;
					PreDctStartDI_F[0] = GetTickCount();
				}
				
				CurTickCount = GetTickCount();
				if (CurTickCount - PreDctStartDI_F[0] > GATE_STA_TIME)
				{
					bFirstDetectDI_F[0] = FALSE;
					
					if (g_ASPara.GateOpenContactor)
					{
						if (DOOR_OPEN == bLastDoorSta) 
						{
							printf("DI1 gate close log\n");
							RecordDoorStsLog(TYPE_GATE_CLOSE, GATE_ID_DEFAULT);
							bLastDoorSta = DOOR_CLOSE;
						}
					}
					else
					{
						if (DOOR_CLOSE == bLastDoorSta) 
						{
							printf("DI1 gate open log\n");
							RecordDoorStsLog(TYPE_GATE_OPEN, GATE_ID_DEFAULT);
                            UlkBehaviourHandle();
							bLastDoorSta = DOOR_OPEN;
						}
					}						 					
					bFlag_F[0]  = FALSE;
				}				
			}
		}
	}	
	// DI2
	if (FUN_GATE_OPEN_TIMEOVER == g_SysConfig.DI2) //else if -> if by [MichaelMa] at 4.9.2012
	{
		// ascend
		if (TRUE == bFlag_R[1])
		{
			DI2Value_R = DIDetect(PIO_TYPE_DI2);
			if (DI_HIGH_LEVEL == DI2Value_R)
			{
				if (FALSE == bFirstDetectDI_R[1]) 
				{
					bFirstDetectDI_R[1] = TRUE;
					PreDctStartDI_R[1] = GetTickCount();
				}
				
				CurTickCount = GetTickCount();
				if (CurTickCount - PreDctStartDI_R[1] > GATE_STA_TIME)
				{
					printf("DI2 ascend %d\n", DI2Value_R);
					bFirstDetectDI_R[1] = FALSE;
					
					if (g_ASPara.GateOpenContactor)
					{
						printf("DI2 gate open log\n");
						RecordDoorStsLog(TYPE_GATE_OPEN, GATE_ID_DEFAULT);
                        UlkBehaviourHandle();
					}
					else
					{
						printf("DI2 gate close log\n");
						RecordDoorStsLog(TYPE_GATE_CLOSE, GATE_ID_DEFAULT);
					}						 					
					bFlag_R[1]  = FALSE;
				}			
			}
		}
		// descend
		if (TRUE == bFlag_F[1])
		{
			DI2Value_F = DIDetect(PIO_TYPE_DI2);
			if (DI_LOW_LEVEL == DI2Value_F)
			{
				if (FALSE == bFirstDetectDI_F[1]) 
				{
					bFirstDetectDI_F[1] = TRUE;
					PreDctStartDI_F[1] = GetTickCount();
				}
				
				CurTickCount = GetTickCount();
				if (CurTickCount - PreDctStartDI_F[1] > GATE_STA_TIME)
				{
					printf("DI2 descend %d\n", DI2Value_F);
					bFirstDetectDI_F[1] = FALSE;
					
					if (g_ASPara.GateOpenContactor)
					{
						printf("DI2 gate close log\n");
						RecordDoorStsLog(TYPE_GATE_CLOSE, GATE_ID_DEFAULT);
					}
					else
					{
						printf("DI2 gate open log\n");
						RecordDoorStsLog(TYPE_GATE_OPEN, GATE_ID_DEFAULT);
                        UlkBehaviourHandle();
					}						 					
					bFlag_F[1]  = FALSE;
				}			
			}
		}
	}	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateStateProc
**	AUTHOR:			Wayde Zeng
**	DATE:			08 - Dec - 2009
**
**	DESCRIPTION:	
**	Process gate state log
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID
GateStateProc()
{
	SetGateStateFlag();
	GateStateDIProc();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AcClearResource()
**	AUTHOR:			Jeff Wang
**	DATE:			08 - May - 2009
**
**	DESCRIPTION:	
**			release the resource of MXMSG  .
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID AcClearResource(MXMSG *pmsg)
{
	if (NULL != pmsg->pParam)
	{
#ifdef HV_TALKMD_DEBUG
		printf("## Free ## :MSG.pParam = %s\n", pmsg->pParam);
#endif		
		free(pmsg->pParam);
		pmsg->pParam = NULL;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AccSwipeCard()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
AccSwipeCard(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	
	if (	NULL == pmsg
			&& STATUS_OPEN != g_ASInfo.ASWorkStatus)
	{
		return bRet;
	}
	
	switch(g_ACCInfo.dwACCState) 
	{
		case ST_ACC_ORIGINAL:
		{
			bRet = SwipeCardOriginalPro(pmsg);
			
			break;
		}
		case ST_ACC_WAIT_SWIPECARD_ACK:
		{
			bRet = WaitSwipeCardAckPro(pmsg);

			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AccCsnPwdMod()
**	AUTHOR:			Michael Ma
**	DATE:			17 - Oct - 2012
**
**	DESCRIPTION:	
**			        controling center of waiting and sending 
**			
**
**	ARGUMENTS:	    ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				    pmsg                    MXMSG
**	RETURNED VALUE:	
**				    BOOL
**	NOTES:
**			
*/
static
BOOL
AccCsnPwdMod(MXMSG *pmsg)
{
    BOOL bRet = FALSE;
    if(NULL == pmsg || STATUS_PWDMOD != g_ASInfo.ASWorkStatus || NULL == pmsg->pParam)
    {
        return bRet;
    }   
    
    printf("g_CsnPwdInfo.dwACCState : %u\n",(unsigned int)g_CsnPwdInfo.dwACCState);
    switch(g_CsnPwdInfo.dwACCState)
    {
        case ST_ACC_CSNPWD_ORIGINAL :
            {
                bRet = CsnPwdModOriginalPro(pmsg);
            }
            break;
        case ST_ACC_WAIT_CSNPWD_ACK :
            {
                bRet = WaitCsnPwdModAckPro(pmsg);
            }
            break;
        default:
            {

            }
    }
    return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SwipeCardOriginalPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
SwipeCardOriginalPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	//unsigned short nDataLen = 0;
	BYTE byCardMode = 0;
	BYTE CSN[CSN_LEN] = { 0 };

	if (NULL == pmsg)
	{
		return bRet;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_AC_SWIPE_CARD:
		{
#ifdef ACC_DEBUG
			printf("%s get the FC_AC_SWIPE_CARD \n", __FUNCTION__);

			
#endif
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return bRet;
			}
			byCardMode = *(pmsg->pParam + sizeof(unsigned short));
			memcpy(CSN, pmsg->pParam + sizeof(unsigned short) + 2, CSN_LEN);

			ACCSendSwipeCard2ACA(pmsg);
			g_ACCInfo.dwACCState = ST_ACC_WAIT_SWIPECARD_ACK;
			g_ACCInfo.dwTickCount = GetTickCount();

			bRet = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	WaitSwipeCardAckPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
WaitSwipeCardAckPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	BYTE byResult = 0;
	BYTE byCardMode = 0;
	BYTE CSN[CSN_LEN] = { 0 };

	if (NULL == pmsg)
	{
		return bRet;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_ACK_AC_SWIPE_CARD:
		{
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return FALSE;
			}
			byResult = *pmsg->pParam;
			
			if(ST_TALK_END ==  g_TalkInfo.talking.dwTalkState) // bug 8774 
			{
				if (g_TalkInfo.talking.bNeedPhotoSaved && !g_TalkInfo.talking.bHavePhotoSaved && GetLeavePhotoStatus()) 
				{
					g_TalkInfo.talking.dwTalkState	= ST_TALK_WAIT_LEAVEPHOTO;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();			
					ShowTalkWnd();
				}
				else
				{
					g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
					WndHideTalkWindow();
				}
			}
			
#ifdef ACC_DEBUG
			printf("%s get the FC_ACK_AC_SWIPE_CARD : byResult=%d,cardtype=%d\n", __FUNCTION__,byResult,(BYTE)pmsg->dwParam);
#endif
			if(byResult == SWIPECARD_CARD_AUTHORIZE)
			{
				if (IsAlarming()) 
				{
					return TRUE;
				}				
				StartPlayRightANote();
				g_ASInfo.ASWorkStatus = STATUS_ADD_CARD;	
				CreateAddCardWnd(GetFocus());
			}
			else if(SWIPECARD_UNLOCK == byResult) // for bug 9616
			{
                if((g_WndMan.MainWndHdle == GetFocus()) || (!GetASProcWnd()/*For bug 11448*/))
                {
                    printf("*****Unlock by card directly*****\n");
                    CreateASCtrlWnd(GetFocus());
                }
				PostMessage(GetASProcWnd(), WM_CARDREAD, 0, byResult);
			}
            else
            {
                CreateASCtrlWnd(GetFocus());
				PostMessage(GetASProcWnd(), WM_CARDREAD, 0, byResult);
            }   

			byCardMode = g_ACCInfo.ACCData[sizeof(unsigned short)];
			memcpy(CSN, &g_ACCInfo.ACCData[sizeof(unsigned short) + 2], CSN_LEN);
        
			//if(SWIPECARD_ENTER_PASSWORD!=byResult)
			//{
			//	RecordSwipeCardLog(byCardMode, CSN, READER_PORT_DEFAULT, READER_ID_DEFAULT, GATE_ID_DEFAULT, 
			//				INOUT_DEFAULT, byResult);
			//}
			//else 
			if (SWIPECARD_UNLOCK == byResult || SWIPECARD_CARD_PATROL_UNLOCK == byResult)
			{
				CDINFO* CdInfo			= NULL;
				CDINFO_OLD* CdInfo_Old  = NULL;
				CDINFO pCDInfo				  ;
				if( TRUE == ReadCard(g_HandleHashCard,CSN,TYPE_NORMAL_CARD,&pCDInfo) )
				{
					CdInfo = &pCDInfo;
					DEBUG_CARD_PRINTF("%s,%d,TYPE_NORMAL_CARD Card existed\r\n",__func__,__LINE__);	
				}
				else if(TRUE == ReadCard(g_HandleHashCard,CSN,TYPE_PATROL_CARD,&pCDInfo))
				{  	
					CdInfo = &pCDInfo;
					DEBUG_CARD_PRINTF("%s,%d,TYPE_PATROL_CARD Card existed\r\n",__func__,__LINE__);	
				}
				else
				{
					CdInfo_Old = AsGetCdInfobyCd(CSN);
					if(ConversionCdInfoOld2CdInfo(&pCDInfo,CdInfo_Old))
					{
						CdInfo = &pCDInfo;
			        	printf("card found\n");
					}
				}
				//CdInfo = AsGetCdInfobyCd(CSN);
				if(CdInfo != NULL)
				{
					memcpy(CSN,CdInfo->CSN,CSN_LEN);
				}
               
				ACCSendCSNCallLift2LC(CSN);
			}

			if (SWIPECARD_INVALID == byResult || SWIPECARD_CARD_DISABLED == byResult || SWIPECARD_EXPIRED == byResult)
			{
				IvdCdNumAlarm();
			}
			else
			{
				g_IvdCardSwipeTimes = 0;
			}

			g_ACCInfo.dwACCState = ST_ACC_ORIGINAL;
			g_ACCInfo.dwTickCount = GetTickCount();
			bRet = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}

	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCSendSwipeCard2ACA()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
ACCSendSwipeCard2ACA(MXMSG *pmsg)
{
	MXMSG	msgSend;
	unsigned short nDataLen = 0;
	//int i;
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, g_ACCInfo.szACCCode);
	strcpy(msgSend.szDestDev, g_ACCInfo.szACACode);
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_AC_SWIPE_CARD;	
	msgSend.dwParam		= g_ACCInfo.dwACAIP;
#ifdef ACC_DEBUG
	printf("the ACAIP = %08x.msgSend.szSrcDev=%s\n",(unsigned int)g_ACCInfo.dwACAIP,msgSend.szSrcDev);
#endif
#ifdef ACC_DEBUG
	printf("*(pmsg->pParam + sizeof(unsigned short) + 25)=%d\n", *(pmsg->pParam + sizeof(unsigned short) + 25));
#endif

	if(2 == *(pmsg->pParam + sizeof(unsigned short) + 25))
	{
		memcpy(&nDataLen, g_ACCInfo.ACCData, sizeof(unsigned short));
		msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);
		memcpy(msgSend.pParam, g_ACCInfo.ACCData, sizeof(unsigned short) + 10);
		memcpy(msgSend.pParam + sizeof(unsigned short) + 10, pmsg->pParam + sizeof(unsigned short) + 10, nDataLen - 10);
	}
	else
	{
		memcpy(&nDataLen, pmsg->pParam, sizeof(unsigned short));
		memcpy(g_ACCInfo.ACCData, pmsg->pParam, sizeof(unsigned short) + nDataLen);
		msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);
		memcpy(msgSend.pParam, pmsg->pParam, sizeof(unsigned short) + nDataLen);
        if (MODE_CARD_ONLY_SET & g_ASPara.ASOpenMode )
	    {
		
	    }
	    else if(MODE_CARD_PASSWORD_SET & g_ASPara.ASOpenMode)
	    {
		    if(*(msgSend.pParam + sizeof(unsigned short) + 25) == 0)
		    {
			    *(msgSend.pParam + sizeof(unsigned short) + 25) = 1;		//set status to need password
		    }
	    }
	}

	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CsnPwdModOriginalPro
**	AUTHOR:			Michael Ma
**	DATE:			17- Oct - 2012
**
**	DESCRIPTION:	
**			        sending corresponding net package to the gate controling sevver.
**			
**
**	ARGUMENTS:	    ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				    pmsg                    MXMSG
**	RETURNED VALUE:	
**				    BOOL
**	NOTES:
**			
*/
static BOOL CsnPwdModOriginalPro(MXMSG *pmsg)
{
    BOOL bRet = FALSE;

	switch(pmsg->dwMsg) 
	{
		case FC_AC_CARD_PASSWORD_MOD:
		{
			ACCSendCsnPwdMod2ACA(pmsg);
            if( 3 == *(pmsg->pParam + sizeof(unsigned short) + 3 + CSN_LEN + PWD_LEN)) 
            {
                g_CsnPwdInfo.dwACCState = ST_ACC_CSNPWD_ORIGINAL;
            }
            else
            {
                g_CsnPwdInfo.dwACCState = ST_ACC_WAIT_CSNPWD_ACK;
            }
			g_CsnPwdInfo.dwTickCount = GetTickCount();
			bRet = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	WaitCsnPwdAckPro
**	AUTHOR:			Michael Ma
**	DATE:			17- Oct - 2012
**
**	DESCRIPTION:	
**			        waiting for the response from the gate controling server.
**			
**
**	ARGUMENTS:	    ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				    pmsg                    MXMSG
**	RETURNED VALUE:	
**			        BOOL	
**	NOTES:
**			
*/
static BOOL WaitCsnPwdModAckPro(MXMSG *pmsg)
{
    BOOL bRet           = FALSE;
	BYTE byResult       = 0;
	
	switch(pmsg->dwMsg) 
	{
		case FC_ACK_AC_CARD_PASSWORD_MOD :
		{
            
			byResult = *pmsg->pParam;
            printf("byResult : %x\n",byResult);
            switch(byResult)
            {   
                case CSN_PWD_MOD_CARD_EXIST             ://5
                case CSN_PWD_MOD_PWD_CORRECT            ://6
                case CSN_PWD_MOD_PWD_ERROR              ://9
                case CSN_PWD_MOD_PATROL                 ://7
                case CSN_PWD_MOD_AUTHORIZING            ://4
                case CSN_PWD_MOD_DISABLED               ://3 
                case CSN_PWD_MOD_EXPIRED                ://1
                case CSN_PWD_MOD_INVALID                ://0
                //case CSN_PWD_MOD_NOT_IN_TIME_SLICE      ://2
                    {
                        PostMessage(GetFocus(), WM_CARDREAD, 0, byResult);
                        break;
                    }
                default                                 :
                    {
                        
                    }
            }
			g_CsnPwdInfo.dwACCState = ST_ACC_CSNPWD_ORIGINAL;
			g_CsnPwdInfo.dwTickCount = GetTickCount();
            bRet = TRUE;
			break;

		}
        default : 
            {

            }
	}
    
    return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCSendCsnPwdMod2ACA
**	AUTHOR:			Michael Ma
**	DATE:			17- Oct - 2012
**          
**	DESCRIPTION:	
**			        according to different work status,sending corresponding net package to 
**			        the gate controling server.
**
**	ARGUMENTS:	    ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				    pmsg                    MXMSG
**	RETURNED VALUE:	
**				    None
**	NOTES:
**			
*/
static VOID ACCSendCsnPwdMod2ACA(MXMSG *pmsg)
{
    MXMSG	msgSend;
	unsigned short nDataLen = 0;
	memset(&msgSend, 0, sizeof(msgSend));
    size_t nTypeSize = sizeof(unsigned short);
    
	
	strcpy(msgSend.szSrcDev, g_ACCInfo.szACCCode);
	strcpy(msgSend.szDestDev, g_ACCInfo.szACACode);
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_AC_CARD_PASSWORD_MOD;	
	msgSend.dwParam		= g_ACCInfo.dwACAIP;
#ifdef ACC_DEBUG
	printf("the ACAIP = %X.msgSend.szSrcDev=%s\n",(unsigned int)g_ACCInfo.dwACAIP,msgSend.szSrcDev);
#endif
#ifdef ACC_DEBUG
	printf("STATUS = %d\n", *(pmsg->pParam + nTypeSize + 3 + CSN_LEN + PWD_LEN));
#endif

    if(      0 == *(pmsg->pParam + nTypeSize + 3 + CSN_LEN + PWD_LEN) ) 
	{
        printf("Sending CSN number to ACA .\n");
		memcpy(&nDataLen, pmsg->pParam, nTypeSize);
		memcpy(g_ACCInfo.ACCData, pmsg->pParam, nTypeSize + nDataLen);
		msgSend.pParam		= (unsigned char *) malloc(nTypeSize + nDataLen);
		memcpy(msgSend.pParam, pmsg->pParam, nTypeSize + nDataLen);  
        *(msgSend.pParam + nTypeSize + 3 + CSN_LEN + PWD_LEN) = SWIPECARD_REQUEST_VERIFY_CSN;              //请求验证卡号
	}
	else if( 2 == *(pmsg->pParam + nTypeSize + 3 + CSN_LEN + PWD_LEN) )
	{
        printf("Sending old PWD to ACA .\n");
		memcpy(&nDataLen, g_ACCInfo.ACCData, nTypeSize);
		msgSend.pParam		= (unsigned char *) malloc(nTypeSize + nDataLen);
		memcpy(msgSend.pParam, g_ACCInfo.ACCData, nTypeSize + 2 + CSN_LEN);
		memcpy(msgSend.pParam + nTypeSize + 2 + CSN_LEN, pmsg->pParam + nTypeSize + 2 + CSN_LEN, nDataLen - 2 -CSN_LEN);
        *(msgSend.pParam + nTypeSize + 3 + CSN_LEN + PWD_LEN) = SWIPECARD_REQUEST_VERIFY_PWD;              //请求验证密码
	}
    else if( 3 == *(pmsg->pParam + nTypeSize + 3 + CSN_LEN + PWD_LEN) )
    {
        printf("Sending new PWD to ACA .\n");
        memcpy(&nDataLen, g_ACCInfo.ACCData, nTypeSize);
		msgSend.pParam		= (unsigned char *) malloc(nTypeSize + nDataLen);
		memcpy(msgSend.pParam, g_ACCInfo.ACCData, nTypeSize + 2 + CSN_LEN);
		memcpy(msgSend.pParam + nTypeSize + 2 + CSN_LEN, pmsg->pParam + nTypeSize + 2 + CSN_LEN, nDataLen - 2 -CSN_LEN);
        *(msgSend.pParam + nTypeSize + 3 + CSN_LEN + PWD_LEN) = SWIPECARD_REQUEST_SAVE_NEW_PWD;            //向服务器发送密码保存请求   
    }
    else 
    {
        //容错
    }
    
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCStateTimeOut()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
ACCStateTimeOut(VOID)
{
	// swipe card timeout
	if (	ST_ACC_WAIT_SWIPECARD_ACK == g_ACCInfo.dwACCState
			&& GetTickCount() - g_ACCInfo.dwTickCount > IRIS_ACK_TIMEOUT)
	{
		g_ACCInfo.dwACCState = ST_ACC_ORIGINAL;
		g_ACCInfo.dwTickCount = GetTickCount();
	}
	// resident password timeout
	if (	ST_ACC_WAIT_RDPASSWORD_ACK == g_RdPwdInfo.dwACCState
			&& GetTickCount() - g_RdPwdInfo.dwTickCount > IRIS_ACK_TIMEOUT)
	{
		g_RdPwdInfo.dwACCState = ST_ACC_RDPWD_ORIGINAL;
		g_RdPwdInfo.dwTickCount = GetTickCount();
	}
	// resident password check timeout
	if (	ST_ACC_WAIT_CHECKRDPWD_ACK == g_RdPwdInfo.dwACCState
			&& GetTickCount() - g_RdPwdInfo.dwTickCount > IRIS_ACK_TIMEOUT)
	{
		g_RdPwdInfo.dwACCState = ST_ACC_RDPWD_ORIGINAL;
		g_RdPwdInfo.dwTickCount = GetTickCount();
	}
	// resident password enter timeout
	if (	ST_ACC_ENTERRDNEWPWD == g_RdPwdInfo.dwACCState
			&& GetTickCount() - g_RdPwdInfo.dwTickCount > INTERFACE_SETTLE_TIME)
	{
		g_RdPwdInfo.dwACCState = ST_ACC_RDPWD_ORIGINAL;
		g_RdPwdInfo.dwTickCount = GetTickCount();
	}
	// resident password modify timeout
	if (	ST_ACC_WAIT_MODRDPWD_ACK == g_RdPwdInfo.dwACCState
			&& GetTickCount() - g_RdPwdInfo.dwTickCount > IRIS_ACK_TIMEOUT)
	{
		g_RdPwdInfo.dwACCState = ST_ACC_RDPWD_ORIGINAL;
		g_RdPwdInfo.dwTickCount = GetTickCount();
	}
	// unlock timeout
	if (	ST_ACC_WAIT_UNLOCK_ACK == g_UnlockInfo.dwACCState
			&& GetTickCount() - g_UnlockInfo.dwTickCount > IRIS_ACK_TIMEOUT)
	{
		g_UnlockInfo.dwACCState = ST_ACC_UNLOCK_ORIGINAL;
		g_UnlockInfo.dwTickCount = GetTickCount();
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendUnlockGate2ACC()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
VOID
SendUnlockGate2ACC(BYTE byUnlockType)
{
	MXMSG	msgSend;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ACC;
	msgSend.dwMsg		= FC_AC_OPEN_GATE;

	nDataLen = IRIS_CODE_LEN + 1 + 1/*Gate ID*/ + 1/*unlock type*/;
	msgSend.pParam = (unsigned char*)malloc(sizeof(unsigned short) + nDataLen);
	memcpy(msgSend.pParam, &nDataLen, sizeof(unsigned short));
	memcpy(msgSend.pParam + sizeof(unsigned short), g_TalkInfo.szTalkDevCode, IRIS_CODE_LEN + 1);
	*(msgSend.pParam + sizeof(unsigned short) + IRIS_CODE_LEN + 1) = GATE_ID_DEFAULT;
	*(msgSend.pParam + sizeof(unsigned short) + IRIS_CODE_LEN + 1 + 1) = byUnlockType;

	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AccRdPwdUnlock()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
AccRdPwdUnlock(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	
	if (NULL == pmsg)
	{
		return bRet;
	}
	
	switch(g_RdPwdInfo.dwACCState) 
	{
		case ST_ACC_RDPWD_ORIGINAL:
		{
			bRet = RdPwdUnlockOriginalPro(pmsg);

			break;
		}
		case ST_ACC_WAIT_RDPASSWORD_ACK:
		{
			bRet = WaitRdPwdUnlockAckPro(pmsg);
			
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RdPwdUnlockOriginalPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
RdPwdUnlockOriginalPro(MXMSG* pmsg)
{
	BOOL bRet = FALSE;
    
	if (NULL == pmsg)
	{
		return bRet;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_AC_RSD_PWD:
		{
#ifdef ACC_DEBUG
			printf("%s get the FC_AC_RSD_PWD \n", __FUNCTION__);
			
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return bRet;
			}
#endif
			memcpy(g_RdPwdInfo.szCallCode, pmsg->pParam + sizeof(unsigned short), RD_CODE_LEN);

			ACCSendRdPwdUnlock2ACA(pmsg);
			
			g_RdPwdInfo.dwACCState = ST_ACC_WAIT_RDPASSWORD_ACK;
			g_RdPwdInfo.dwTickCount = GetTickCount();
			
			bRet = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	WaitRdPwdUnlockAckPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
WaitRdPwdUnlockAckPro(MXMSG* pmsg)
{
	BOOL bRet = FALSE;
	BYTE byResult = 0;
	
	if (NULL == pmsg)
	{
		return bRet;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_ACK_AC_RSD_PWD:
		{
#ifdef ACC_DEBUG
			printf("%s get the FC_ACK_AC_RSD_PWD \n", __FUNCTION__);
			
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return bRet;
			}
#endif
			byResult = *(pmsg->pParam);
			//CreatePwdUnlockWnd(GetFocus()); //for bug 9616
			PostMessage(GetFocus(), WM_RDPASSWORD, 0, byResult);

			if (0 == byResult)
			{
				ACCSendRDCODECallLift2LC(g_RdPwdInfo.szCallCode);
			}
			
			g_RdPwdInfo.dwACCState = ST_ACC_RDPWD_ORIGINAL;
			g_RdPwdInfo.dwTickCount = GetTickCount();
			
			bRet = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCSendRdPwdUnlock2ACA()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
ACCSendRdPwdUnlock2ACA(MXMSG* pmsg)
{
	MXMSG	msgSend;
	unsigned short nDataLen = 0;

	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, g_RdPwdInfo.szACCCode);
	strcpy(msgSend.szDestDev, g_RdPwdInfo.szACACode);
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_AC_RSD_PWD;
	msgSend.dwParam		= g_RdPwdInfo.dwACAIP;
#ifdef ACC_DEBUG
	printf("the ACAIP = %X.\n", (unsigned int)g_RdPwdInfo.dwACAIP);
#endif
	
	memcpy(&nDataLen, pmsg->pParam, sizeof(unsigned short));
	msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);
	memcpy(msgSend.pParam, pmsg->pParam, sizeof(unsigned short) + nDataLen);
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AccRdPwdMod()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
AccRdPwdMod(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	
	if (NULL == pmsg)
	{
		return bRet;
	}

    if(MXMSG_CLOSE_WINDOW == pmsg->dwMsg)
    {
        g_RdPwdInfo.dwACCState = ST_ACC_RDPWD_ORIGINAL; //when get this msg,refresh this variable immediately to make 
        return (bRet = TRUE);                           //next password input/modify usable.added by [MichaelMa] at 30.8.2012
    }
    
	printf("g_RdPwdInfo.dwACCState : %x\n",(unsigned int)g_RdPwdInfo.dwACCState);
	switch(g_RdPwdInfo.dwACCState) 
	{
		case ST_ACC_RDPWD_ORIGINAL:
		{
			bRet = RdPwdModOriginalPro(pmsg);

			break;
		}
		case ST_ACC_WAIT_CHECKRDPWD_ACK:
		{
			bRet = WaitQueryRdPwdAckPro(pmsg);
			
			break;
		}
		case ST_ACC_ENTERRDNEWPWD:
		{
			bRet = EnterRdNewPwdPro(pmsg);
			
			break;
		}
		case ST_ACC_WAIT_MODRDPWD_ACK:
		{
			bRet = WaitModRdPwdAckPro(pmsg);
			
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RdPwdModOriginalPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
RdPwdModOriginalPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	
	if (NULL == pmsg)
	{
		return bRet;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_AC_PWD_CHECK:
		{
#ifdef ACC_DEBUG
			printf("%s get the FC_AC_PWD_CHECK \n", __FUNCTION__);
			
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return bRet;
			}
#endif
			ACCSendRdPwdCheck2ACA(pmsg);
			
			g_RdPwdInfo.dwACCState = ST_ACC_WAIT_CHECKRDPWD_ACK;
			g_RdPwdInfo.dwTickCount = GetTickCount();
			
			bRet = TRUE;
			break;
		}
		case FC_AC_PWD_MODIFY:
		{
#ifdef ACC_DEBUG
			printf("%s get the FC_AC_PWD_MODIFY \n", __FUNCTION__);
			
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return bRet;
			}
#endif
			ACCSendRdPwdMod2ACA(pmsg);
			
			g_RdPwdInfo.dwACCState = ST_ACC_WAIT_MODRDPWD_ACK;
			g_RdPwdInfo.dwTickCount = GetTickCount();
			
			bRet = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCSendRdPwdCheck2ACA()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
ACCSendRdPwdCheck2ACA(MXMSG* pmsg)
{
	MXMSG	msgSend;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, g_RdPwdInfo.szACCCode);
	strcpy(msgSend.szDestDev, g_RdPwdInfo.szACACode);
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_AC_PWD_CHECK;
	msgSend.dwParam		= g_RdPwdInfo.dwACAIP;
#ifdef ACC_DEBUG
	printf("the ACAIP = %X.\n", (unsigned int)g_RdPwdInfo.dwACAIP);
#endif
	
	memcpy(&nDataLen, pmsg->pParam, sizeof(unsigned short));
	msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);
	memcpy(msgSend.pParam, pmsg->pParam, sizeof(unsigned short) + nDataLen);
	
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	WaitQueryRdPwdAckPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
WaitQueryRdPwdAckPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	BYTE byResult = 0;
	
	if (NULL == pmsg)
	{
		return bRet;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_ACK_AC_PWD_CHECK:
		{
#ifdef ACC_DEBUG
			printf("%s get the FC_ACK_AC_PWD_CHECK \n", __FUNCTION__);
			
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return bRet;
			}
#endif
			byResult = *(pmsg->pParam);
			//CreatePwdModifyWnd(GetFocus()); //for bug 9616
			PostMessage(GetFocus(), WM_RDPASSWORD, 1, byResult);
			
			if (0 == byResult)
			{
				g_RdPwdInfo.dwACCState = ST_ACC_ENTERRDNEWPWD;
			}
			else
			{
				g_RdPwdInfo.dwACCState = ST_ACC_RDPWD_ORIGINAL;
			}
			
			g_RdPwdInfo.dwTickCount = GetTickCount();
			
			bRet = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EnterRdNewPwdPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
EnterRdNewPwdPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	
	if (NULL == pmsg)
	{
		return bRet;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_AC_PWD_MODIFY:
		{
#ifdef ACC_DEBUG
			printf("%s get the FC_AC_PWD_MODIFY \n", __FUNCTION__);
			
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return bRet;
			}
#endif
			ACCSendRdPwdMod2ACA(pmsg);
			
			g_RdPwdInfo.dwACCState = ST_ACC_WAIT_MODRDPWD_ACK;
			g_RdPwdInfo.dwTickCount = GetTickCount();
			
			bRet = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCSendRdPwdMod2ACA()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
ACCSendRdPwdMod2ACA(MXMSG* pmsg)
{
	MXMSG	msgSend;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, g_RdPwdInfo.szACCCode);
	strcpy(msgSend.szDestDev, g_RdPwdInfo.szACACode);
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_AC_PWD_MODIFY;
	msgSend.dwParam		= g_RdPwdInfo.dwACAIP;
#ifdef ACC_DEBUG
	printf("the ACAIP = %X.\n", (unsigned int)g_RdPwdInfo.dwACAIP);
#endif
	
	memcpy(&nDataLen, pmsg->pParam, sizeof(unsigned short));
	msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);
	memcpy(msgSend.pParam, pmsg->pParam, sizeof(unsigned short) + nDataLen);
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	WaitModRdPwdAckPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
WaitModRdPwdAckPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	BYTE byResult = 0;
	
	if (NULL == pmsg)
	{
		return bRet;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_ACK_AC_PWD_MODIFY:
		{
#ifdef ACC_DEBUG
			printf("%s get the FC_ACK_AC_PWD_MODIFY \n", __FUNCTION__);
			
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return bRet;
			}
#endif
			byResult = *(pmsg->pParam);
			//CreatePwdModifyWnd(GetFocus()); //for bug 9616
			PostMessage(GetFocus(), WM_RDPASSWORD, 2, byResult);
			
			g_RdPwdInfo.dwACCState = ST_ACC_RDPWD_ORIGINAL;
			g_RdPwdInfo.dwTickCount = GetTickCount();
			
			bRet = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AccUnlockGate()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
AccUnlockGate(MXMSG *pmsg)
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return bRet;
	}
    
	switch(g_UnlockInfo.dwACCState) 
	{
		case ST_ACC_UNLOCK_ORIGINAL:
		{
			bRet = UnlockOriginalPro(pmsg);

			break;
		}
		case ST_ACC_WAIT_UNLOCK_ACK:
		{
			bRet = WaitUnlockAckPro(pmsg);

			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UnlockOriginalPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
UnlockOriginalPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	
	if (NULL == pmsg)
	{
		return bRet;
	}
	switch(pmsg->dwMsg) 
	{
		case FC_AC_OPEN_GATE:
		{
#ifdef ACC_DEBUG
			printf("%s get the FC_AC_OPEN_GATE \n", __FUNCTION__);
			
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return bRet;
			}
#endif
			ACCSendUnlockGate2ACA(pmsg);

			g_UnlockInfo.dwACCState = ST_ACC_WAIT_UNLOCK_ACK;
			g_UnlockInfo.dwTickCount = GetTickCount();

			bRet = TRUE;
			break;
		}
        case FC_UNLOCK_NOTIFY_GM:/*GM send log to MC when HV unlock AC or self*/
        {
            
            MultiDoorOpenHandle(pmsg);
            bRet = TRUE;
        }
		default:
		{
			break;
		}
	}
	
	return bRet;
}

static VOID MultiDoorOpenHandle(MXMSG *pmsg)
{
#ifdef ACC_DEBUG
    printf(" SRC:%s\n",pmsg->szSrcDev);
    printf(" DST:%s\n",pmsg->szDestDev);
#endif
#ifdef __SUPPORT_PROTOCOL_900_1A__	
	if(MOX_PROTOCOL_SUPPORT(MOX_P_900_1A))
	{
		ShowTalkUlkWnd();
		return;
	}
#endif
    strcpy(g_TalkInfo.unlock.szULDestDevCode,pmsg->szSrcDev);
    RecordLogData(TYPE_UNLOCK, STEP_START, &g_TalkInfo);
    /*
        提示开锁成功.只要HV开了某个门的锁，就提示.
    */
    ShowTalkUlkWnd();
    if(MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType )
	{                      
	    ACCSendCALLCODECallLift2LC(g_TalkInfo.talking.szTalkDestDevCode);
	}
	else if(MON_TYPE_EHV == g_TalkInfo.monitor.bType)
	{
	    ACCSendCALLCODECallLift2LC(g_TalkInfo.monitor.szMonSrcDevCode);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	WaitUnlockAckPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
WaitUnlockAckPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	BYTE byResult = 1;
	BYTE byCardMode = 0;
	BYTE CSN[CSN_LEN] = { 0 };
	
	if (NULL == pmsg)
	{
		return bRet;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_ACK_AC_OPEN_GATE:
		{
#ifdef ACC_DEBUG
			printf("%s get the FC_ACK_AC_OPEN_GATE \n", __FUNCTION__);
			
			if (pmsg == NULL || pmsg->pParam == NULL)
			{
				printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
				return bRet;
			}
#endif
			byResult = *(pmsg->pParam);
			if (0 == byResult)
			{
				switch(g_UnlockType)
				{
					case UNLOCKTYPE_PATROL:
					{
						CreateASCtrlWnd(GetFocus());
						PostMessage(GetFocus(), WM_CARDREAD, 0, SWIPECARD_CARD_PATROL_UNLOCK);
						
						byCardMode = g_ACCInfo.ACCData[sizeof(unsigned short)];
						memcpy(CSN, &g_ACCInfo.ACCData[sizeof(unsigned short) + 2], CSN_LEN);

						ACCSendCSNCallLift2LC(CSN);

						break;
					}
					case UNLOCKTYPE_MANUAL:
					{
						ShowUlkWnd();
						
						RecordOpenMnlLog(BUTTON_ID_DEFAULT, GATE_ID_DEFAULT);

						break;
					}
					case UNLOCKTYPE_TALK:
					{
						if (ST_MT_MONITOR == g_TalkInfo.monitor.dwMonState)
						{
							ShowUlkWnd();					
						}
						else
						{
							ShowTalkUlkWnd();				
						}

						RecordLogData(TYPE_UNLOCK, STEP_START, &g_TalkInfo);
						if(MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType )
						{
                          
							ACCSendCALLCODECallLift2LC(g_TalkInfo.talking.szTalkDestDevCode);
						}
						else if(MON_TYPE_EHV == g_TalkInfo.monitor.bType)
						{
							ACCSendCALLCODECallLift2LC(g_TalkInfo.monitor.szMonSrcDevCode);
						}
						
						break;
					}
					case UNLOCKTYPE_CARDPWD:
					{
						CreateASCtrlWnd(GetFocus());
						/*
						if( GetCardType() == TYPE_PATROL_CARD)
						{
							PostMessage(GetFocus(), WM_CARDREAD,TYPE_PATROL_CARD, SWIPECARD_CARD_PATROL_UNLOCK);
							
						}
						else
						{
							PostMessage(GetFocus(), WM_CARDREAD, 0, SWIPECARD_UNLOCK);
						}
						*/
						PostMessage(GetFocus(), WM_CARDREAD, 0, SWIPECARD_UNLOCK);
						
						byCardMode = g_ACCInfo.ACCData[sizeof(unsigned short)];
						memcpy(CSN, &g_ACCInfo.ACCData[sizeof(unsigned short) + 2], CSN_LEN);

						ACCSendCSNCallLift2LC(CSN);

						break;
					}
					case UNLOCKTYPE_RDPWD:
					{
						CreatePwdUnlockWnd(GetFocus());
						PostMessage(GetFocus(), WM_RDPASSWORD, 0, byResult);

						RecordLogData(TYPE_UNLOCK, STEP_START, &g_TalkInfo);

						ACCSendRDCODECallLift2LC(g_RdPwdInfo.szCallCode);

						break;
					}
					default:
					{
						break;
					}
				}
			}

			g_UnlockInfo.dwACCState = ST_ACC_UNLOCK_ORIGINAL;
			g_UnlockInfo.dwTickCount = GetTickCount();
			
			bRet = TRUE;
			break;
		}
		default:
		{
			break;
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCSendUnlockGate2ACA()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
ACCSendUnlockGate2ACA(MXMSG *pmsg)
{
	MXMSG	msgSend;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, g_UnlockInfo.szACCCode);
	strcpy(msgSend.szDestDev, g_UnlockInfo.szACACode);
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_AC_OPEN_GATE;
	msgSend.dwParam		= g_UnlockInfo.dwACAIP;
#ifdef ACC_DEBUG
	printf("the ACAIP = %X.\n", (unsigned int)g_UnlockInfo.dwACAIP);
#endif
	g_UnlockType = *(pmsg->pParam + sizeof(unsigned short) + IRIS_CODE_LEN + 1 + 1);
	memcpy(&nDataLen, pmsg->pParam, sizeof(unsigned short));
	msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);
	memcpy(msgSend.pParam, pmsg->pParam, sizeof(unsigned short) + nDataLen);
	
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCSendCSNCallLift2LC()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
ACCSendCSNCallLift2LC(BYTE CSN[CSN_LEN])
{
	MXMSG msgSend;

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_LC;
	msgSend.dwMsg		= MXMSG_GM_CALL_LIFT;

	msgSend.dwParam = 1 + RD_CODE_LEN + CSN_LEN;
	msgSend.pParam = (UCHAR*)malloc(1 + RD_CODE_LEN + CSN_LEN);
	msgSend.pParam[0] = HO_QUERY_METHOD_CSN;
	memcpy(msgSend.pParam + 1, g_TalkInfo.szTalkDevCode, RD_CODE_LEN);
	memcpy(msgSend.pParam + 1 + RD_CODE_LEN, CSN, CSN_LEN);
    
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCSendRDCODECallLift2LC()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
ACCSendRDCODECallLift2LC(char CallCode[RD_CODE_LEN])
{
	MXMSG msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_LC;
	msgSend.dwMsg		= MXMSG_GM_CALL_LIFT;
	
	msgSend.dwParam = 1 + RD_CODE_LEN + RD_CODE_LEN;
	msgSend.pParam = (UCHAR*)malloc(1 + RD_CODE_LEN + RD_CODE_LEN);
	msgSend.pParam[0] = HO_QUERY_METHOD_RDCODE;
	memcpy(msgSend.pParam + 1, g_TalkInfo.szTalkDevCode, RD_CODE_LEN);
	memcpy(msgSend.pParam + 1 + RD_CODE_LEN, CallCode, RD_CODE_LEN);
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCSendCALLCODECallLift2LC()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
ACCSendCALLCODECallLift2LC(char CallCode[RD_CODE_LEN])
{
	MXMSG msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_LC;
	msgSend.dwMsg		= MXMSG_GM_CALL_LIFT;
	
	msgSend.dwParam = 1 + DEVICE_CODE_LEN + RD_CODE_LEN;
	msgSend.pParam = (UCHAR*)malloc(1 + DEVICE_CODE_LEN + RD_CODE_LEN);
	msgSend.pParam[0] = HO_QUERY_METHOD_CALLCODE;
	memcpy(msgSend.pParam + 1               , g_TalkInfo.szTalkDevCode  , DEVICE_CODE_LEN); //GM code
	memcpy(msgSend.pParam + 1 + RD_CODE_LEN , CallCode                  , RD_CODE_LEN);     //HV code

	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACCSendCALLCODECallLift2LC()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
IvdCdNumAlarm(VOID)
{
	g_IvdCardSwipeTimes++;				
		
	if (	g_ASPara.InvldCardSwipeAlmFlag 
			&& g_IvdCardSwipeTimes >= g_ASPara.InvldCardSwipeRptSet) 
	{
		g_IvdCardSwipeTimes	=	0;
		{
			RecordAsAlarmLog(TYPE_INVALID_CARD_ALARM);
		}
	}
}

#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
#define SAVE_NORMAL_CARD		1
#define SAVE_AUTHORIZE_CARD		2
#define SAVE_ALL_CARD			3
static BOOL AccGetCardCnt(MXMSG *pmsg)
{
	BOOL    bRet = FALSE;
	UINT    total = 0;
	USHORT  nDataLen = 20;      //固定数据长度预留
	BYTE*   pData = NULL;
	MXMSG	MsgSend;
	UINT	GateNum;
	memcpy(&GateNum, &pmsg->pParam[1], sizeof(UINT));
	if(GateNum != 1)
		return FALSE;

    if(!isAST_HVFileExisted())
    {
        printf("AST_HV not existed.\n");
        //进行数据转换
        if(init_AST_HV_space())
        {
            //转换用户卡
            printf("converting normal card.\n");
            convertNormalCardFileData();
        }
        //转换授权功能卡
        convertAuthCardFileData();
    }
    total = g_AS_HVInfo.ASTHead.nCardCnt + g_AAInfo.ASTHead.nCardCnt;

	strcpy(MsgSend.szSrcDev, pmsg->szDestDev);
	strcpy(MsgSend.szDestDev, pmsg->szSrcDev);
					
	MsgSend.dwSrcMd 	= MXMDID_ACC;
	MsgSend.dwDestMd	= MXMDID_ETH;
	MsgSend.dwMsg		= FC_ACK_AC_GET_CARD_CNT;
	MsgSend.dwParam 	= (pmsg->dwParam);				//IP
	MsgSend.wDataLen	= nDataLen;
	
	if(NULL != (MsgSend.pParam = (BYTE*)malloc(nDataLen)))
	{
        pData = MsgSend.pParam;
    	memcpy(pData, &total, sizeof(UINT));
    	MxPutMsg(&MsgSend);
    	bRet = TRUE;
	}
	return bRet;
}

#define TYPE_INDEX	1
#define TYPE_CSN	2

static BOOL AccGetCard(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	USHORT nDataLen = 10+sizeof(CDINFO);
	BYTE type  = 1, CSN[CSN_LEN];
	MXMSG	MsgSend;
	CDINFO_HV_t *pCdInfo = NULL;
	UINT index = 0, GateNum = 0;

	printf("%s \n", __FUNCTION__);
	type = pmsg->pParam[0];
	switch(type)
	{
		case TYPE_INDEX:
			memcpy(&index, pmsg->pParam+1, 4);
			memcpy(&GateNum, pmsg->pParam+1+4, 4);
//			printf("index:%d,GateNum:%d\n",index, GateNum);
			break;
		case TYPE_CSN:
			memcpy(CSN, pmsg->pParam+1, 5);// 此类型后续完成
		default:
			return bRet;
	}

	MsgSend.pParam		= (BYTE*)malloc(nDataLen);
	if(NULL == MsgSend.pParam)
		return bRet;


	pCdInfo = AsGetCdHVinfobyIndex(index);

	if(pCdInfo)
	{
//		printf("[ %02x %02x %02x %02x %02x]\n", pCdInfo->CSN[0], pCdInfo->CSN[1], pCdInfo->CSN[2], pCdInfo->CSN[3], pCdInfo->CSN[4]);
		MsgSend.pParam[0] = 0;//Get OK.
		MsgSend.pParam[1] = sizeof(CDINFO);
		memcpy(&MsgSend.pParam[2], pCdInfo, sizeof(CDINFO));
	}
	else
	{
		printf("card not existed!\n");
		MsgSend.pParam[0] = 1;//card not existed
		MsgSend.pParam[1] = 0;
	}
		
		
	strcpy(MsgSend.szSrcDev, pmsg->szDestDev);
	strcpy(MsgSend.szDestDev, pmsg->szSrcDev);
	MsgSend.dwSrcMd 	= MXMDID_ACC;
	MsgSend.dwDestMd	= MXMDID_ETH;
	MsgSend.dwMsg		= FC_ACK_AC_GET_CARD;
	MsgSend.dwParam 	= (pmsg->dwParam);				//IP
	MsgSend.wDataLen	= nDataLen;
	MxPutMsg(&MsgSend); 
	bRet = TRUE;

	return bRet;
}

/************************************************************************************************
**FunctionName    : AccGetCard_V2
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
static BOOL AccGetCard_V2(MXMSG *pmsg)
{
    BOOL bRet = FALSE;
	USHORT nDataLen = 10 + sizeof(CDINFO_HV_t);
	//BYTE CSN[CSN_LEN];
	MXMSG	MsgSend;
	CDINFO_HV_t *pCdInfo = NULL;
	UINT index = 0;
    //UINT GateNum = 0;

	switch(pmsg->pParam[0])
	{
		case TYPE_INDEX:
			memcpy(&index, pmsg->pParam+1, 4);
			//memcpy(&GateNum, pmsg->pParam+1+4, 4);
			break;
		case TYPE_CSN:
			//memcpy(CSN, pmsg->pParam+1, 5);// 此类型后续完成
		default:
			return bRet;
	}

	if(NULL != (MsgSend.pParam = (BYTE*)malloc(nDataLen)))
	{
        if(NULL != (pCdInfo = AsGetCdHVinfobyIndex(index)))
    	{
    		MsgSend.pParam[0] = 0;//Get OK.
    		MsgSend.pParam[1] = sizeof(CDINFO_HV_t);
    		memcpy(&MsgSend.pParam[2], pCdInfo, sizeof(CDINFO_HV_t));
    	}
    	else
    	{
    		printf("card not existed!\n");
    		MsgSend.pParam[0] = 1;//card not existed
    		MsgSend.pParam[1] = 0;
    	}    			
    	strcpy(MsgSend.szSrcDev, pmsg->szDestDev);
    	strcpy(MsgSend.szDestDev, pmsg->szSrcDev);
    	MsgSend.dwSrcMd 	= MXMDID_ACC;
    	MsgSend.dwDestMd	= MXMDID_ETH;
    	MsgSend.dwMsg		= FC_ACK_AC_GET_CARD;
    	MsgSend.dwParam 	= (pmsg->dwParam);				//IP
    	MsgSend.wDataLen	= nDataLen;
    	MxPutMsg(&MsgSend); 
    	bRet = TRUE;
	}
	return bRet;
}

#define DEL_CARD_OK			0
#define DEL_CARD_NOT_EXIST	1
static BOOL AccDelCard(MXMSG *pmsg)
{
	USHORT nDataLen = 20;
	BYTE type = 0, flag = 0, Result = 0, SaveFlag;
	MXMSG	MsgSend;
	UINT index = 0, GateNum = 0 ;
    //realGateNum = 0, iLoop = 0;
	CDINFO_HV_t *pCdInfo = NULL;
	//int	sec_sn = 0, offset = 0, ListInd = 0;

	printf("%s \n", __FUNCTION__);	
	type = pmsg->pParam[0];
	memcpy(&GateNum, &pmsg->pParam[1], sizeof(UINT));
	memcpy(&index, &pmsg->pParam[1+sizeof(UINT)], sizeof(UINT));
	flag = pmsg->pParam[1+sizeof(UINT)+sizeof(UINT)];
	printf("type:%d, getenumber:%d, index:%d, flag:%d\n", type, GateNum, index,flag);

	pCdInfo = AsGetCdHVinfobyIndex(index);
    printf("Norm cnt : %u\n",(unsigned int)g_AS_HVInfo.ASTHead.nCardCnt);
    printf("Auth cnt : %u\n",(unsigned int)g_AAInfo.ASTHead.nCardCnt);
	if(pCdInfo == NULL)
	{
		Result = DEL_CARD_NOT_EXIST;
		goto SEND_DEL_MSG;
	}
	if(pCdInfo->CardType == TYPE_AUTHORIZE_CARD)
	{
		SaveFlag = SAVE_AUTHORIZE_CARD;
        AsRmAuthorizeCdHV(pCdInfo->CSN);
        SaveAuthorizeCdHVInfo2Mem();
		//LoadAuthorizeCdHVInfoFromMem();
	}
	else
	{
		SaveFlag = SAVE_NORMAL_CARD;
        AsRmCdHV(pCdInfo->CSN);
        SaveCdHVInfo2Mem();
		//LoadCdHVInfoFromFile();
	}
	Result = DEL_CARD_OK;

SEND_DEL_MSG:
	strcpy(MsgSend.szSrcDev, pmsg->szDestDev);
	strcpy(MsgSend.szDestDev, pmsg->szSrcDev);
					
	MsgSend.dwSrcMd 	= MXMDID_ACC;
	MsgSend.dwDestMd	= MXMDID_ETH;
	MsgSend.dwMsg		= FC_ACK_AC_DEL_CARD;
	MsgSend.dwParam 	= (pmsg->dwParam);				//IP
	MsgSend.wDataLen	= nDataLen;
	MsgSend.pParam		= (BYTE*)malloc(nDataLen);
	if(NULL == MsgSend.pParam)
		return FALSE;

	MsgSend.pParam[0] = Result;

	MxPutMsg(&MsgSend); 

	return TRUE;
}

#define EDIT_CARD_OK			0	
#define EDIT_CARD_NOT_EXIST		1
static BOOL AccEditCard(MXMSG *pmsg)
{
	USHORT nDataLen = 20;
	BYTE  type  = 1, Result = 0 ,index = 0, SaveFlag = 0;
//	BYTE  Len = 0;
	MXMSG	MsgSend;
	CDINFO_HV_t* pCdInfo = NULL;
    CDINFO_HV_t* pDesCdInfo = NULL;
	//int	sec_sn = 0, offset = 0, StartAddr = 0 ,StartSec = 0;

	printf("%s \n", __FUNCTION__);
	type = pmsg->pParam[0];
	index = pmsg->pParam[1];
//	Len = pmsg->pParam[2];	

	switch(type)
	{
		case TYPE_INDEX:

			pCdInfo = (CDINFO_HV_t*)(pmsg->pParam + 3);
			printf("CardID:%d,[ %02x %02x %02x %02x %02x]\n",pCdInfo->Reserved[0], pCdInfo->CSN[0], pCdInfo->CSN[1], pCdInfo->CSN[2], pCdInfo->CSN[3], pCdInfo->CSN[4]);
			break;
		case TYPE_CSN:
			//memcpy(CSN, pmsg->pParam+1, 5);// 此类型后续完成
		default:
			return FALSE;
	}

	MsgSend.pParam		= (BYTE*)malloc(nDataLen);
	if(NULL == MsgSend.pParam)
		return FALSE;

	pDesCdInfo = AsGetCdHVInfobyCd(pCdInfo->CSN);
    printf("pDesCdInfo[%02x %02x %02x %02x %02x]\n",
                                                        pDesCdInfo->CSN[0],
                                                        pDesCdInfo->CSN[1],
                                                        pDesCdInfo->CSN[2],
                                                        pDesCdInfo->CSN[3],
                                                        pDesCdInfo->CSN[4]);
	if( pDesCdInfo == NULL)
	{
		Result = EDIT_CARD_NOT_EXIST;
		goto SEND_EDIT_MSG;
	}
	else
	{
		if(pDesCdInfo->CardType != pCdInfo->CardType)
		{
			SaveFlag = SAVE_ALL_CARD;
		}
		else 
		{
			if(pDesCdInfo->CardType == TYPE_AUTHORIZE_CARD)
			{
				SaveFlag = SAVE_AUTHORIZE_CARD;
			}
			else
			{
				SaveFlag = SAVE_NORMAL_CARD;
			}
		}
		//pDesCdInfo->CardType = pCdInfo->CardType;
		//pDesCdInfo->Reserved[0] = pCdInfo->Reserved[0];
		//memcpy(pCdInfo, pDesCdInfo, sizeof(CDINFO_HV_t));
		if(AsRmCdHV(pDesCdInfo->CSN))
		{
            printf("remove the original normal card firstly.\n");
		}
        else if(AsRmAuthorizeCdHV(pDesCdInfo->CSN))
        {
            
        }
        printf("card type : %u\n",pCdInfo->CardType);
		if(pCdInfo->CardType == TYPE_AUTHORIZE_CARD)
		{
			AsAddAuthorizeCdHV(pCdInfo);
		}
		else if(pCdInfo->CardType == TYPE_NORMAL_CARD)
		{
			AsAddCdHV(pCdInfo);
            printf("add the new normal card secondly.\n");
		}
		else
			return FALSE;
		Result = EDIT_CARD_OK;
		if(SaveFlag == SAVE_AUTHORIZE_CARD || SaveFlag == SAVE_ALL_CARD)
		{
			SaveAuthorizeCdHVInfo2Mem();
			LoadAuthorizeCdHVInfoFromMem();
		}
		if(SaveFlag == SAVE_NORMAL_CARD || SaveFlag == SAVE_ALL_CARD)
		{
			SaveCdHVInfo2Mem();
			LoadCdHVInfoFromFile();
		}
	}
		
SEND_EDIT_MSG:
	strcpy(MsgSend.szSrcDev, pmsg->szDestDev);
	strcpy(MsgSend.szDestDev, pmsg->szSrcDev);
					
	MsgSend.dwSrcMd 	= MXMDID_ACC;
	MsgSend.dwDestMd	= MXMDID_ETH;
	MsgSend.dwMsg		= FC_ACK_AC_EDIT_CARD;
	MsgSend.dwParam 	= (pmsg->dwParam);				//IP
	MsgSend.wDataLen	= nDataLen;
	MsgSend.pParam		= (BYTE*)malloc(nDataLen);
	if(NULL == MsgSend.pParam)
		return FALSE;
	
	MsgSend.pParam[0] = Result;
	MxPutMsg(&MsgSend); 

	return TRUE;
}
static BOOL AccLastSwipeCard(MXMSG *pmsg)
{
	USHORT nDataLen = 1+1+4+4+2+SWIPECARD_DATA_LEN+1;
	BYTE* pData = NULL, GateNum = 0;
	MXMSG	MsgSend;
	ALDATA* pLastLog;
    
	printf("%s \n", __FUNCTION__);	
	GateNum = pmsg->pParam[0];
	printf("%s GateNumber:%d\n", __FUNCTION__, GateNum);
	if(GateNum < 1)
		return FALSE;
	
	strcpy(MsgSend.szSrcDev, pmsg->szDestDev);
	strcpy(MsgSend.szDestDev, pmsg->szSrcDev);
    
	MsgSend.dwSrcMd 	= MXMDID_ACC;
	MsgSend.dwDestMd	= MXMDID_ETH;
	MsgSend.dwMsg		= FC_ACK_AC_LAST_SWIPE_V1;
	MsgSend.dwParam 	= (pmsg->dwParam);				//IP
	MsgSend.wDataLen	= nDataLen;
	MsgSend.pParam		= (BYTE*)malloc(nDataLen);
	if(NULL == MsgSend.pParam)
		return FALSE;
	
	pData = MsgSend.pParam;

	pLastLog = GetLastSwipCardLog();
    printf("the last log exieted ? %d\n",(NULL == pLastLog) ? 0 : 1);
	if(pLastLog == NULL || HaveLastSwipeCardLog() == FALSE)
	{
        printf("FALSE\n");
		pData[0] = 1;
	}
	else
	{
        printf("TRUE\n");
		pData[0] = 0;
		pData += 1;
		pData[0] = TYPE_SWIPE_CARD_V2;
		pData += 1;
		memcpy(pData, &pLastLog->EventTime,sizeof(time_t));
		pData += 4;
		memcpy(pData, &pLastLog->EventID, 4);
		pData += 4;
		memcpy(pData, &pLastLog->DataLen, 2);
		pData += 2;
		memcpy(pData, pLastLog->pData, pLastLog->DataLen);
		ClearLastSwipeCardInfo();
	}
	MxPutMsg(&MsgSend); 
	return TRUE;
}

#define ADDCARD_OK		0
#define ADDCARD_EXIST	1
#define ADDCARD_LIMIT	2
static BOOL AccAddCard(MXMSG *pmsg)
{
	USHORT nDataLen = 20;
	BYTE* pDataIn = NULL, Result = 1;
	MXMSG	MsgSend;
	CDINFO_HV_t  AddCard;
	CDINFO_HV_t *pTmpCdinfo = NULL;
	//DWORD	CardID;
	WORD GateNumber;
    
	DEBUG_CARD_PRINTF("%s,%d \n", __FUNCTION__,__LINE__);
	pDataIn = pmsg->pParam;
	nDataLen = pDataIn[0];
	pDataIn += 1;
	memcpy(&AddCard, pDataIn, sizeof(CDINFO_HV_t));
	//AsPrintCd(&AddCard);
	AddCard.GateNumber = 0x01 << (AddCard.GateNumber - 1);
	GateNumber = AddCard.GateNumber;
	pTmpCdinfo = AsGetCdHVInfobyCd(AddCard.CSN);
	if(pTmpCdinfo != NULL)
		Result = ADDCARD_EXIST;
	else 
	{
        strncpy(AddCard.UlkPwd,g_PUPara.ResiPwdDefault,strlen(g_PUPara.ResiPwdDefault));
        AddCard.UlkPwd[strlen(g_PUPara.ResiPwdDefault)] = '\0';
		if(AddCard.CardType == TYPE_AUTHORIZE_CARD)
		{
			if(g_AAInfo.ASTHead.nCardCnt + 1 < MAX_AS_ACD_CNT)
			{
				AsAddAuthorizeCdHV(&AddCard);
				Result = ADDCARD_OK;
				SaveAuthorizeCdHVInfo2Mem();
				LoadAuthorizeCdHVInfoFromMem();
			}
			else
				Result = ADDCARD_LIMIT;
		}
		else if(AddCard.CardType == TYPE_NORMAL_CARD)
		{
			if (g_ASInfo.ASTHead.nCardCnt + 1 < MAX_AS_CD_CNT)
			{
				AsAddCdHV(&AddCard);
				Result = ADDCARD_OK;
				SaveCdHVInfo2Mem();
				LoadCdHVInfoFromFile();
			}
			else
				Result = ADDCARD_LIMIT;
		}
		else
			return FALSE;
		
	}

	strcpy(MsgSend.szSrcDev, pmsg->szDestDev);
	strcpy(MsgSend.szDestDev, pmsg->szSrcDev);
					
	MsgSend.dwSrcMd 	= MXMDID_ACC;
	MsgSend.dwDestMd	= MXMDID_ETH;
	MsgSend.dwMsg		= FC_ACK_AC_ADD_CARD;
	MsgSend.dwParam 	= (pmsg->dwParam);				//IP
	MsgSend.wDataLen	= nDataLen;
	MsgSend.pParam		= (BYTE*)malloc(nDataLen);
	if(NULL == MsgSend.pParam)
		return FALSE;
	
	MsgSend.pParam[0] = Result;
	MxPutMsg(&MsgSend);
	
	return TRUE;
}

static BOOL AccGetEventCnt(MXMSG *pmsg)
{
	MXMSG	MsgSend;
	USHORT nDataLen = 20;
	BYTE	GateNum;
	UINT Result;

	DEBUG_CARD_PRINTF("%s,%d \n", __FUNCTION__,__LINE__);	
	GateNum = pmsg->pParam[0];
//	printf("Get [GM%d] Event Cnt\n", GateNum);

	Result = GetSwipeCardLogCntByGate();

	
	strcpy(MsgSend.szSrcDev, pmsg->szDestDev);
	strcpy(MsgSend.szDestDev, pmsg->szSrcDev);
					
	MsgSend.dwSrcMd 	= MXMDID_ACC;
	MsgSend.dwDestMd	= MXMDID_ETH;
	MsgSend.dwMsg		= FC_ACK_AC_EVENT_CNT;
	MsgSend.dwParam 	= (pmsg->dwParam);				//IP
	MsgSend.wDataLen	= nDataLen;
	MsgSend.pParam		= (BYTE*)malloc(nDataLen);

	if(NULL == MsgSend.pParam)
		return FALSE;

	memcpy(MsgSend.pParam, &Result, 4);
	MxPutMsg(&MsgSend);

	return TRUE;
}

static BOOL AccEventRead(MXMSG *pmsg)
{
	MXMSG	MsgSend;
	USHORT  nDataLen = 2+1+4+4+2+SWIPECARD_DATA_LEN + 20 + 20;
    USHORT  Result = 1;
    USHORT  Index;
	BYTE    GateNum; 
    BYTE*   pData = NULL;
	ALDATA* pALData = NULL;
    CDINFO_HV_t* pCdInfo = NULL;
	printf("%s \n", __FUNCTION__);
	memcpy(&Index,pmsg->pParam,2);
	GateNum = pmsg->pParam[2];

	pALData = GetSwipeCardLogByIndex(Index+1);

	if(pALData == NULL)
		Result = 0;
	
	strcpy(MsgSend.szSrcDev, pmsg->szDestDev);
	strcpy(MsgSend.szDestDev, pmsg->szSrcDev);
					
	MsgSend.dwSrcMd 	= MXMDID_ACC;
	MsgSend.dwDestMd	= MXMDID_ETH;
	MsgSend.dwMsg		= FC_ACK_AC_EVENT_READ;
	MsgSend.dwParam 	= (pmsg->dwParam);				//IP
	MsgSend.wDataLen	= nDataLen;
	MsgSend.pParam		= (BYTE*)malloc(nDataLen);
	if(NULL == MsgSend.pParam)
		return FALSE;
	memset(MsgSend.pParam, 0, nDataLen);
	pData = MsgSend.pParam;
	memcpy(pData, &Result, 2);
	pData += 2;
	*pData = pALData->EventType;
	pData += 1;
	memcpy(pData, &pALData->EventTime, 4);
	pData += 4;
	memcpy(pData, &pALData->EventID, 4);
	pData += 4;
	memcpy(pData, &pALData->DataLen, 2);
	pData += 2;
	memcpy(pData, pALData->pData, EVENT_DATA_LEN);
    pData += EVENT_DATA_LEN;
    
    if(NULL != (pCdInfo = AsGetCdHVInfobyCd(pALData->pData + SWIPECARD_CSN_OFFSET)))
    {
        memcpy(pData,(BYTE*)((pCdInfo->strCardHolderName)),20);
        pData += 20;
        memcpy(pData,(BYTE*)(pCdInfo->strRemark),20);
    }
	MxPutMsg(&MsgSend);
	return TRUE;
}

static BOOL AccessPrePro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;

	switch(pmsg->dwMsg)
	{
		case FC_AC_GET_CARD_CNT:
            printf("--------------------FC_AC_GET_CARD_CNT\n");
			bRet = AccGetCardCnt(pmsg);//OKOK
			break;
		case FC_AC_GET_CARD:
            printf("--------------------FC_AC_GET_CARD\n");
			bRet = AccGetCard(pmsg);//OK
			break;
		case FC_AC_DEL_CARD:
            printf("--------------------FC_AC_DEL_CARD\n");
			bRet = AccDelCard(pmsg);//OKOK
			break;
		case FC_AC_LAST_SWIPE_V1:
            printf("--------------------FC_AC_LAST_SWIPE_V1\n");
			bRet = AccLastSwipeCard(pmsg);//OKOK
			break;
		case FC_AC_ADD_CARD:
            printf("--------------------FC_AC_ADD_CARD\n");
			bRet = AccAddCard(pmsg);//OKOK
			break;
		case FC_AC_EDIT_CARD:
            printf("--------------------FC_AC_EDIT_CARD\n");
			bRet = AccEditCard(pmsg);//OKOK
			break;
		case FC_AC_EVENT_CNT:
            printf("--------------------FC_AC_EVENT_CNT\n");
			bRet = AccGetEventCnt(pmsg);//OKOK
			break;
		case FC_AC_EVENT_READ:
            printf("--------------------FC_AC_EVENT_READ\n");
			bRet = AccEventRead(pmsg);//OKOK
			break;
        case FC_AC_GET_CARD_V2:
            printf("--------------------FC_AC_GET_CARD_V2\n");
            bRet = AccGetCard_V2(pmsg);//OKOK
		default:
			bRet = FALSE;
	}

	return bRet;
}
#endif

/************************************************************************************************
**FunctionName    : UlkBehaviourHandle
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
static void UlkBehaviourHandle(void)
{
#ifdef FORCE_ULK_FALG
    printf("g_DoorUlkBehav.DoorUlkBehaviour : %u\n",g_DoorUlkBehav.DoorUlkBehaviour);
    if(g_ASPara.EnableForceUlkReport && (DOOR_ACTIVE_OPEN_START != g_DoorUlkBehav.DoorUlkBehaviour))
    {
        printf("force ulk report\n");
        sendForceUlkLog2MC();
    }
#endif //FORCE_ULK_FALG    
}

/************************************************************************************************
**FunctionName    : init_AST_HV_space
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
static BOOL init_AST_HV_space(void)
{
    BOOL bRet = FALSE;
    if(NULL != (g_AS_HVInfo.MXASBuf = (CHAR*)malloc(MAX_AS_CD_CNT * sizeof(CDINFO_HV_t))))
    {
        memset(g_AS_HVInfo.MXASBuf, 0, MAX_AS_CD_CNT * sizeof(CDINFO_HV_t));
        g_AS_HVInfo.ASTHead.nCardCnt = 0;
        bRet = TRUE;
    }
    return bRet;
}

