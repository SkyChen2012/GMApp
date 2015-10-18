

/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SecurityAlarm.c
**
**	AUTHOR:		Mike Zhang
**
**	DATE:		13 - Dec - 2010
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
#include "AccessLogReport.h"
#include "SecurityAlarm.h"
#include "SAAlarmWnd.h"
#include "SALogReport.h"



/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/
DETECT_TIMER_t g_TamperDetecter;

BOOL bGateAla;

SAManStruct			SAMan;

static VOID InfraredAlarmProc();


static VOID GateOpenOvertimeProc();
static VOID	AsSendOpenGateAck2MC(MXMSG* pMsg);

static BOOL		SAGetMsg(MXMSG* pMsg);

static BOOL TamperAlarm();
static void TamperAlarmProc();

static int IsOpenAlarmTrigger(void);
static int IsInfraredAlarmTrigger(void);
static void NeedUpdateSAWnd();
static void	SANotifyOtherMd(void);
static BOOL	ISReportAlarmIdle(void);
//static void	SendAlarm2MC(int nAlarmType);
static void			SAFreeMXMSG(MXMSG* pMsg);


int
SecurityAlarmInit()
{
	int i = 0;
	DpcAddMd(MXMDID_SA, NULL);

	SAMan.nSARunStatus	= ALARM_RUN_NORMAL;
	SAMan.dwSendTick	= GetTickCount();

	SALogInit();

	for(i=0; i< MAX_ALARM_TYPE; i ++)
	{
		SAMan.SADetailMan[i].bNeedReport		= FALSE;
		SAMan.SADetailMan[i].bAlarmAlter		= FALSE;
		SAMan.SADetailMan[i].bWaitAck			= FALSE;
		SAMan.SADetailMan[i].bAlarmSigLastSta	= FALSE;
		SAMan.SADetailMan[i].nSARunStatus		= SUB_ALARM_NORMAL;
		SAMan.SADetailMan[i].dwDelayTick		= GetTickCount();
	}

	return 0;
}

void
SecurityAlarmExit()
{	
	int i = 0;	
	SAMan.nSARunStatus	= ALARM_RUN_NORMAL;
	SAMan.dwSendTick	= GetTickCount();	
	for(i=0; i< MAX_ALARM_TYPE; i ++)
	{
		SAMan.SADetailMan[i].bNeedReport		= FALSE;
		SAMan.SADetailMan[i].bAlarmAlter		= FALSE;
		SAMan.SADetailMan[i].bWaitAck			= FALSE;
		SAMan.SADetailMan[i].bAlarmSigLastSta	= FALSE;
		SAMan.SADetailMan[i].nSARunStatus		= SUB_ALARM_NORMAL;
		SAMan.SADetailMan[i].dwDelayTick		= GetTickCount();
	}
	DpcRmMd(MXMDID_SA);
}


void
SecurityAlarmProcess()
{
	MXMSG				MsgGet;
	int                 i = 0;
	BOOL				bAllIdle = TRUE;

	if (SAGetMsg(&MsgGet))
	{
		if (!SALogProc(&MsgGet))
		{
			switch (MsgGet.dwMsg)
			{
/*
			case FC_ACK_ALM_REPORT:
				for(i = 0; i < MAX_ALARM_TYPE;i++)
				{
					if(SAMan.SADetailMan[i].bWaitAck)
					{
						SAMan.SADetailMan[i].bWaitAck = FALSE;
						SAMan.SADetailMan[i].bNeedReport = FALSE;
					}
				}
				break;
*/
			case MXMSG_SECURITY_ALARM_CONFIRM:
				
				SAMan.nSARunStatus =  ALARM_RUN_NORMAL;
				StopPlayAlarmANote();
				
				for(i = 0; i < MAX_ALARM_TYPE;i++)
				{
					if(SUB_ALARM_RUN == SAMan.SADetailMan[i].nSARunStatus)
					{
						SAMan.SADetailMan[i].nSARunStatus = SUB_ALARM_NORMAL;
					}
				}
				UpdateAlarmConfirmWnd(ALA_WND_CONFIRM);
				break;
			default:
				break;
			}
		}			


		SAFreeMXMSG(&MsgGet);
	}


	TamperAlarmProc();
	GateOpenOvertimeProc();
	InfraredAlarmProc();
	
	
	if(ALARM_RUN_NORMAL == SAMan.nSARunStatus)
	{
		for(i = 0; i < MAX_ALARM_TYPE;i++)
		{
			if(SUB_ALARM_RUN == SAMan.SADetailMan[i].nSARunStatus)
			{
				StartPlayAlarmANote();
				SANotifyOtherMd();
				SAMan.nSARunStatus =  ALARM_RUN_TRIGGGER;
				break;
			}
		}			
	}
	else if(ALARM_RUN_TRIGGGER == SAMan.nSARunStatus)
	{
		for(i = 0; i < MAX_ALARM_TYPE;i++)
		{
			if(SUB_ALARM_RUN == SAMan.SADetailMan[i].nSARunStatus)
			{
				bAllIdle = FALSE;
			}
		}
		if(bAllIdle)
		{
			SAMan.nSARunStatus =  ALARM_RUN_NORMAL;
			StopPlayAlarmANote();
		}
	}



	
/*
	if(!ISReportAlarmIdle() && (GetTickCount() - SAMan.dwSendTick > MC_ACK_TIMEOUT))
	{
		for(i = 0; i < MAX_ALARM_TYPE;i++)
		{
			if(SAMan.SADetailMan[i].bWaitAck)
			{
				SAMan.SADetailMan[i].bWaitAck = FALSE;	
			}
		}
		SAMan.dwSendTick = GetTickCount();
	}
	
	
	if((GetTickCount() - SAMan.dwSendTick > SA_REPORT_TIME) 
		&& ISReportAlarmIdle() 
		&& g_TalkInfo.bMCStatus)
	{
		for(i = 0; i < MAX_ALARM_TYPE;i++)
		{
			if (SAMan.SADetailMan[i].bNeedReport)
			{
				if(ALARM_TYPE_TAMPER == i)
				{
 					SendAlarm2MC(ALARM_TYPE_DISASSEMBLY);
					SAMan.SADetailMan[i].bWaitAck = TRUE;
					SAMan.dwSendTick = GetTickCount();
				}
				else if(ALARM_TYPE_OPEN_OVERTIME == i)
				{
					SendAlarm2MC(ALARM_TYPE_DOOR_OPEN_TIMEOUT);
					SAMan.SADetailMan[i].bWaitAck = TRUE;
					SAMan.dwSendTick = GetTickCount();
				}
				else if(ALARM_TYPE_INFRARED == i)
				{
					SendAlarm2MC(ALARM_TYPE_GM_INFRARED);
					SAMan.SADetailMan[i].bWaitAck = TRUE;
					SAMan.dwSendTick = GetTickCount();
				}
			}
		}	
	}
*/
	if (IsMCOnline() 
		&& IsReportIdle())
	{
		SALogReport();
	}

	SALogReportTimeOut();


}


static BOOL
SAGetMsg(MXMSG* pMsg)
{
	pMsg->dwDestMd = MXMDID_SA;
	return MxGetMsg(pMsg);
}




/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TamperAlarmProc
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
TamperAlarmProc()
{
	BOOL	bAlarmSig			= FALSE;
	
	if (!g_SysConfig.EnTamperAlarm)
	{
		if(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus)
		{
			SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus = SUB_ALARM_NORMAL;
		}
		SAMan.SADetailMan[ALARM_TYPE_TAMPER].bAlarmSigLastSta = FALSE;
		return;
	}

    bAlarmSig = ReadDI_V2(&g_TamperDetecter,PIO_TYPE_TAMPER_ALARM,TamperAlarm);    
	//bAlarmSig = TamperAlarm();// detect dis-assembly action, disassembly alarm.
	//printf("bAlarmSig : %u\n",bAlarmSig);

	switch(SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus)
	{
	case SUB_ALARM_NORMAL:
		{
			if(bAlarmSig  && !SAMan.SADetailMan[ALARM_TYPE_TAMPER].bAlarmSigLastSta)
			{
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus = SUB_ALARM_ON_DELAY;
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].dwDelayTick  = GetTickCount();
			}
			break;
		}
	case SUB_ALARM_ON_DELAY:
		{
			if(!bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus = SUB_ALARM_NORMAL;
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].bAlarmAlter = FALSE;
			}
			else if(GetTickCount() - SAMan.SADetailMan[ALARM_TYPE_TAMPER].dwDelayTick > ALARM_DELAY_TIMEOUT)
			{
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus = SUB_ALARM_RUN;
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].bAlarmAlter = TRUE;
				
				NeedUpdateSAWnd();				
			}
			break;
		}
	case SUB_ALARM_OFF_DELAY:
		{
			if(bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus = SUB_ALARM_RUN;
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].bAlarmAlter = FALSE;
			}
			else if(GetTickCount() - SAMan.SADetailMan[ALARM_TYPE_TAMPER].dwDelayTick > ALARM_DELAY_TIMEOUT)
			{
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus = SUB_ALARM_NORMAL;
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].bAlarmAlter = TRUE;
				
				NeedUpdateSAWnd();
			}
			break;
		}
	case SUB_ALARM_RUN:
		{
			if(!bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus = SUB_ALARM_OFF_DELAY;
				SAMan.SADetailMan[ALARM_TYPE_TAMPER].dwDelayTick  = GetTickCount();
			}
			break;
		}
	default:
		break;
	}
	
	if(SAMan.SADetailMan[ALARM_TYPE_TAMPER].bAlarmAlter)
	{ 
		SAMan.SADetailMan[ALARM_TYPE_TAMPER].bAlarmAlter = FALSE;
		if(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus)
		{
			RecordAlarmLog(ALARM_TYPE_DISASSEMBLY);
//			SAMan.SADetailMan[ALARM_TYPE_TAMPER].bNeedReport = TRUE;
//			ReadRtcTime(SAMan.SAReportTime);
		}
		
	}
	
	SAMan.SADetailMan[ALARM_TYPE_TAMPER].bAlarmSigLastSta = bAlarmSig;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DO1Set
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

static BOOL 
TamperAlarm()
{
	BOOL bRet = FALSE;

	if(!PIORead(PIO_TYPE_TAMPER_ALARM))
	{
		bRet = TRUE;
	}
	return bRet;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	InfraredAlarmProc
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
InfraredAlarmProc()
{
		
	BOOL bAlarmSig = FALSE;
	
	if (!g_ASPara.InfraredAlmFlag)
	{
		if(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus)
		{
			SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus = SUB_ALARM_NORMAL;
		}
		SAMan.SADetailMan[ALARM_TYPE_INFRARED].bAlarmSigLastSta = FALSE;
		return;
	}
	
	
	bAlarmSig = IsInfraredAlarmTrigger();// detect dis-assembly action, disassembly alarm.
	
	
	switch(SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus)
	{
	case SUB_ALARM_NORMAL:
		{
			if(bAlarmSig && !SAMan.SADetailMan[ALARM_TYPE_INFRARED].bAlarmSigLastSta)
			{
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus = SUB_ALARM_ON_DELAY;
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].dwDelayTick  = GetTickCount();
			}
			break;
		}
	case SUB_ALARM_ON_DELAY:
		{
			if(!bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus = SUB_ALARM_NORMAL;
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].bAlarmAlter = FALSE;
			}
			else if(GetTickCount() - SAMan.SADetailMan[ALARM_TYPE_INFRARED].dwDelayTick > ALARM_DELAY_TIMEOUT)
			{
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus = SUB_ALARM_DELAY;
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].dwDelayTick  = GetTickCount();
			}
			break;
		}
	case SUB_ALARM_DELAY:
		{
			if(!bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus = SUB_ALARM_NORMAL;
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].bAlarmAlter = FALSE;
			}
			else if(GetTickCount() - SAMan.SADetailMan[ALARM_TYPE_INFRARED].dwDelayTick > g_ASPara.InfraredOverTime*1000)
			{
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus = SUB_ALARM_RUN;
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].bAlarmAlter = TRUE;
				
				NeedUpdateSAWnd();
			}
			break;
		}
	case SUB_ALARM_OFF_DELAY:
		{
			if(bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus = SUB_ALARM_RUN;
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].bAlarmAlter = FALSE;
			}
			else if(GetTickCount() - SAMan.SADetailMan[ALARM_TYPE_INFRARED].dwDelayTick > ALARM_DELAY_TIMEOUT)
			{
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus = SUB_ALARM_NORMAL;
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].bAlarmAlter = TRUE;
				
				NeedUpdateSAWnd();
			}
			break;
		}
	case SUB_ALARM_RUN:
		{
			if(!bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus = SUB_ALARM_OFF_DELAY;
				SAMan.SADetailMan[ALARM_TYPE_INFRARED].dwDelayTick  = GetTickCount();
			}
			break;
		}
	default:
		break;
	}
	
	if(SAMan.SADetailMan[ALARM_TYPE_INFRARED].bAlarmAlter)
	{ 
		SAMan.SADetailMan[ALARM_TYPE_INFRARED].bAlarmAlter = FALSE;
		if(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus)
		{
			RecordAlarmLog(ALARM_TYPE_GM_INFRARED);
//			SAMan.SADetailMan[ALARM_TYPE_INFRARED].bNeedReport = TRUE;
//			ReadRtcTime(SAMan.SAReportTime);
		}
	}
	
	SAMan.SADetailMan[ALARM_TYPE_INFRARED].bAlarmSigLastSta = bAlarmSig;			

}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenOvertimeProc
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
GateOpenOvertimeProc()
{


	int bAlarmSig = 0;
	if (!g_ASPara.GateOpenAlmFlag)
	{
		if(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus)
		{
			SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus = SUB_ALARM_NORMAL;
		}
		SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmSigLastSta = FALSE;
		return;
	}


	bAlarmSig = IsOpenAlarmTrigger();// detect dis-assembly action, disassembly alarm.

	if (-1 == bAlarmSig) 
	{

		if(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus)
		{
			SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus = SUB_ALARM_NORMAL;
		}
		SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmSigLastSta = FALSE;
		return;
	}


	switch(SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus)
	{
		case SUB_ALARM_NORMAL:
		{
			if(bAlarmSig && !SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmSigLastSta)
			{
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus = SUB_ALARM_ON_DELAY;
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].dwDelayTick  = GetTickCount();
			}
			break;
		}
		case SUB_ALARM_ON_DELAY:
		{
			if(!bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus = SUB_ALARM_NORMAL;
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmAlter = FALSE;
			}
			else if(GetTickCount() - SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].dwDelayTick > ALARM_DELAY_TIMEOUT)
			{
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus = SUB_ALARM_DELAY;
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].dwDelayTick  = GetTickCount();
			}
			break;
		}
		case SUB_ALARM_DELAY:
		{
			if(!bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus = SUB_ALARM_NORMAL;
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmAlter = FALSE;
			}
			else if(GetTickCount() - SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].dwDelayTick > g_ASPara.GateOpenOverTime*1000)
			{
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus = SUB_ALARM_RUN;
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmAlter = TRUE;

				NeedUpdateSAWnd();
			}
			break;
		}
		case SUB_ALARM_OFF_DELAY:
		{
			if(bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus = SUB_ALARM_RUN;
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmAlter = FALSE;
			}
			else if(GetTickCount() - SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].dwDelayTick > ALARM_DELAY_TIMEOUT)
			{
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus = SUB_ALARM_NORMAL;
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmAlter = TRUE;

				NeedUpdateSAWnd();
			}
			break;
		}
		case SUB_ALARM_RUN:
		{
			if(!bAlarmSig)
			{
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus = SUB_ALARM_OFF_DELAY;
				SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].dwDelayTick  = GetTickCount();
			}
			break;
		}
		default:
			break;
	}

	if(SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmAlter)
	{ 
		SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmAlter = FALSE;
		if(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus)
		{
			RecordAlarmLog(ALARM_TYPE_DOOR_OPEN_TIMEOUT);
//			SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bNeedReport = TRUE;
//			ReadRtcTime(SAMan.SAReportTime);
		}
	}
	SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].bAlarmSigLastSta = bAlarmSig;			


}

void StartAlarmOutPut(void)
{

	MXMSG  msgSend;
		printf("StartAlarmOutPut\n");
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_SA;
	msgSend.dwDestMd	= MXMDID_IOCTRL;

	msgSend.dwMsg		= COMM_OVERTIME_ALARM_ON;
	msgSend.dwParam		= 0;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
}
void StopAlarmOutPut(void)
{

	MXMSG  msgSend;
		printf("StopAlarmOutPut\n");
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_SA;
	msgSend.dwDestMd	= MXMDID_IOCTRL;

	msgSend.dwMsg		= COMM_OVERTIME_ALARM_OFF;
	msgSend.dwParam		= 0;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
}

static int 
IsOpenAlarmTrigger(void)
{
    char nRetDI1 = -1;
    char nRetDI2 = -1;
    
    if(g_ASPara.GateOpenContactor)
    {
        if(FUN_GATE_OPEN_TIMEOVER == g_SysConfig.DI1)
        {
            nRetDI1 = ( DI_HIGH_LEVEL == DIDetect(PIO_TYPE_DI1) ) ? 1 : 0;
        }
        if(FUN_GATE_OPEN_TIMEOVER == g_SysConfig.DI2)
        {
            nRetDI2 = ( DI_HIGH_LEVEL == DIDetect(PIO_TYPE_DI2) ) ? 1 : 0;
        }
    }
    else
    {
        if(FUN_GATE_OPEN_TIMEOVER == g_SysConfig.DI1)
        {
            nRetDI1 = ( DI_LOW_LEVEL == DIDetect(PIO_TYPE_DI1) ) ? 1 : 0;
        }
        if(FUN_GATE_OPEN_TIMEOVER == g_SysConfig.DI2)
        {
            nRetDI2 = ( DI_LOW_LEVEL == DIDetect(PIO_TYPE_DI2) ) ? 1 : 0;
        }
    }
    /*DI1 and DI2 are not set FUN_GATE_OPEN_TIMEOVER*/
    if(-1 == nRetDI1 && -1 == nRetDI2)
    {
        return -1;
    }
    /*One or all of them triggered*/
    else if(1 == nRetDI1 || 1 == nRetDI2)
    {
        return 1;
    }
    /*One or all of them not triggered*/
    else
    {
        return 0;
    }
    return -1;
}

static int 
IsInfraredAlarmTrigger(void)
{
	if (FUN_INFRARED_ALARM == g_SysConfig.DI1) 
	{
		if(g_ASPara.InfraredContactor)
		{				
			if (DI_HIGH_LEVEL == ReadDI1()) 
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			if (DI_LOW_LEVEL == ReadDI1()) 
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		
	}
	else if(FUN_INFRARED_ALARM == g_SysConfig.DI2)
	{
		if(g_ASPara.InfraredContactor)
		{				
			if (DI_HIGH_LEVEL == ReadDI2()) 
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			if (DI_LOW_LEVEL == ReadDI2()) 
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}	
	else
	{
		return 77;
	}		
	
}





static void
NeedUpdateSAWnd()
{
	int nFreshType = 0;

	if ((SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus) &&
		(SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus) &&
		(SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus))
	{
		nFreshType = ALA_WND_DESTROY;
	}
	if ((SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus) &&
		(SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus) &&
		(SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus))
	{
		nFreshType = ALA_SHOW_TAMPER;
	}
	if ((SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus) &&
		(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus) &&
		(SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus))
	{
		nFreshType = ALA_SHOW_OPEN;
	}
	if ((SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus) &&
		(SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus) &&
		(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus))
	{
		nFreshType = ALA_SHOW_INFRARED;
	}
	if ((SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus) &&
		(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus) &&
		(SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus))
	{
		nFreshType = ALA_SHOW_TAMPER_AND_OPEN;
	}
	if ((SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus) &&
		(SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus) &&
		(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus))
	{
		nFreshType = ALA_SHOW_TAMPER_AND_INFRARED;
	}
	if ((SUB_ALARM_NORMAL == SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus) &&
		(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus) &&
		(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus))
	{
		nFreshType = ALA_SHOW_OPEN_AND_INFRARED;
	}
	if ((SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_TAMPER].nSARunStatus) &&
		(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_OPEN_OVERTIME].nSARunStatus) &&
		(SUB_ALARM_RUN == SAMan.SADetailMan[ALARM_TYPE_INFRARED].nSARunStatus))
	{
		nFreshType = ALA_SHOW_ALL;
	}				
	
	UpdateAlarmConfirmWnd(nFreshType);
}


static void
SANotifyOtherMd()
{
	MXMSG		MsgSend;
	
	memset(&MsgSend, 0, sizeof(MXMSG));
	
	MsgSend.dwSrcMd = MXMDID_SA;
	MsgSend.dwDestMd = MXMDID_TALKING;
	MsgSend.dwMsg = MXMSG_SECURITY_ALARM;
	MxPutMsg(&MsgSend);
}

static BOOL
ISReportAlarmIdle()
{
	int i = 0;
	for(i=0;i<MAX_ALARM_TYPE;i++)
	{
		if(SAMan.SADetailMan[i].bWaitAck)
		{
			return FALSE;
		}
	}
	return TRUE;
}



/*
static void
SendAlarm2MC(int nAlarmType)
{
	MXMSG	msgSend;
	UCHAR OutTimeData[6] = { 0 };
	USHORT		nDataLen = 8;
	time_t		tmTmp = 0;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_PUBLIC;
	msgSend.dwDestMd	= MXMDID_ETH;
	
	msgSend.dwMsg		= FC_ALM_REPORT;
	
	msgSend.dwParam	    =  g_TalkInfo.dwMCIP;
	
	msgSend.pParam = (UCHAR*)malloc(2+2+6);
	
	memcpy(msgSend.pParam, &nDataLen, 2);
	msgSend.pParam[2]	=	nAlarmType;//Remove
	msgSend.pParam[3]	=	0;
	
	tmTmp = GetSysTime();
	ConvertFromTime_t(OutTimeData, &tmTmp);
	
	memcpy(&msgSend.pParam[4], OutTimeData, 6);
	
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode); 	
	
	MxPutMsg(&msgSend);
	
	
}
*/



static void
SAFreeMXMSG(MXMSG* pMsg)
{
	if (pMsg->pParam != NULL)
	{
		MXFree(pMsg->pParam);
		pMsg->pParam = NULL;
	}
}

BOOL
IsAlarming()
{
	if (ALARM_RUN_TRIGGGER == SAMan.nSARunStatus)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}







