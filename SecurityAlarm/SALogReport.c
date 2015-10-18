/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SALogReport.c
**
**	AUTHOR:		Mike Zhang
**
**	DATE:		20 - Dec - 2010
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
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "Dispatch.h"
#include "MXCommon.h"
#include "AccessCommon.h"
#include "SALogReport.h"
#include "TalkLogReport.h"
#include "rtc.h"

/************** DEFINES **************************************************************/

#define SA_LOG_VERSION				0x00000001

/************** TYPEDEFS *************************************************************/


/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/


//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static BOOL	DelOneSALog();
static VOID	AddOneSALog(SALDATA*	 pAlData);

static BOOL	IsSAExist();

static SALDATA*	GetOneSALog();
static VOID SendSALog2Eth(SALDATA* pSAlData);


static	UINT		g_SAEventId = 1;
static	MANSALOG	ManSALog;
SALOGINFO			g_SALogInfo;

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsRecordLogInit
**	AUTHOR:			Mike Zhang	
**	DATE:		20 - Dec - 2010
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
static VOID
AsRecordLogInit()
{
	ManSALog.SAlHeader.dwVersion = SA_LOG_VERSION;
	ManSALog.SAlHeader.nCurCnt	= 0;
	ManSALog.SAlHeader.nMaxCnt	= MAX_SA_LOG_COUNT;
	memset(ManSALog.SAlDataHeader, 0, ManSALog.SAlHeader.nMaxCnt * sizeof(SALDATA));
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsLogInit
**	AUTHOR:			Jeff Wang
**	DATE:		07 - May - 2009
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
SALogInit()
{
	g_SALogInfo.nStatus = SAL_IDLE;
	AsRecordLogInit();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SALogReport
**	AUTHOR:			Mike Zhang
**	DATE:		20 - Dec - 2010
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
SALogReport()
{
	SALDATA*	 pSAlData		=	NULL;
	BYTE		 SdLogBuf[1024] =	{0};
	UINT		 nSdLogLen		=	0;

	
	if (IsSAExist()) 
	{		
		pSAlData	=	GetOneSALog();
		SendSALog2Eth(pSAlData);

		g_SALogInfo.nCurEventID =	pSAlData->EventID;
		g_SALogInfo.dwTickCount	=	GetTickCount();
		g_SALogInfo.nStatus		=	SAL_BUSY;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SALogProc
**	AUTHOR:			Mike Zhang
**	DATE:		20 - Dec - 2010
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
BOOL
SALogProc(MXMSG* pMsg)
{
	BOOL bRet = FALSE;
	UINT EventID	=	0;
	
	if (NULL == pMsg)
	{
		return TRUE;
	}
	
	switch(pMsg->dwMsg)
	{
	case FC_ACK_ALM_REPORT:
		{
			DelOneSALog();
			
			g_SALogInfo.nCurEventID =	0;
			g_SALogInfo.dwTickCount	=	GetTickCount();
			g_SALogInfo.nStatus		=	SAL_IDLE;
			
			bRet = TRUE;
			break;	
		}
	default:
		break;
	}

	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SALogReportTimeOut
**	AUTHOR:			Mike Zhang
**	DATE:		20 - Dec - 2010
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
SALogReportTimeOut()
{
	DWORD	dwTicks	=	0;

	if (SAL_BUSY == g_SALogInfo.nStatus) 
	{
		dwTicks = GetTickCount();
		if (dwTicks - g_SALogInfo.dwTickCount > IRIS_ACK_LOG_TIMEOUT) 
		{
			g_SALogInfo.nStatus = SAL_IDLE;
			g_SALogInfo.dwTickCount	=	GetTickCount();
		}
	}
}




/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RecordAlarmLog
**	AUTHOR:			Mike Zhang
**	DATE:		20 - Dec - 2010
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
RecordAlarmLog(BYTE AlarmType)
{
	SALDATA	SAlData;

	memset(&SAlData, 0, sizeof(SALDATA));
	SAlData.bValid		=	TRUE;	
	SAlData.EventType	=	AlarmType;
	SAlData.EventTime	=	GetSysTime();
	SAlData.EventID		=	g_SAEventId++;	
	SAlData.DataLen		=	0;	
	SAlData.pData		=	NULL;
	
	AddOneSALog(&SAlData);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddOneSALog
**	AUTHOR:			Mike Zhang	
**	DATE:		20 - Dec - 2010
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**	
**	
**	NOTES:
**
*/
static VOID
AddOneSALog(SALDATA*	pSAlData)
{
	UINT j	=	0;

	if (ManSALog.SAlHeader.nCurCnt == ManSALog.SAlHeader.nMaxCnt) 
	{
		if (ManSALog.SAlDataHeader[0].pData) 
		{
			free(ManSALog.SAlDataHeader[0].pData);
			ManSALog.SAlDataHeader[0].pData = NULL;
		}
		memset(&(ManSALog.SAlDataHeader[0]), 0, sizeof(SALDATA));
		
		if (ManSALog.SAlHeader.nCurCnt > 0) 
		{
			ManSALog.SAlHeader.nCurCnt--;
		}
		
		for(j = 0; j < ManSALog.SAlHeader.nCurCnt; j++)
		{
			memcpy(&(ManSALog.SAlDataHeader[j]), &(ManSALog.SAlDataHeader[j+1]), sizeof(SALDATA));
		}

		memset(&(ManSALog.SAlDataHeader[ManSALog.SAlHeader.nCurCnt]), 0, sizeof(SALDATA));
	}

	memcpy(&(ManSALog.SAlDataHeader[ManSALog.SAlHeader.nCurCnt]), pSAlData, sizeof(SALDATA));
	
	if (ManSALog.SAlHeader.nCurCnt < ManSALog.SAlHeader.nMaxCnt) 
	{
		ManSALog.SAlHeader.nCurCnt++;
	}

	printf("AddOneSALog pSAlData->EventType = %d; ManSALog.SAlHeader.nCurCnt = %d\n",pSAlData->EventType,ManSALog.SAlHeader.nCurCnt);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetOneSALog
**	AUTHOR:			Mike Zhang
**	DATE:		20 - Dec - 2010
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**	
**	
**	NOTES:
**
*/
static SALDATA*
GetOneSALog()
{
	SALDATA*	 pSAlData = NULL;
		
	if (0 < ManSALog.SAlHeader.nCurCnt)
	{
		pSAlData = (SALDATA*)&(ManSALog.SAlDataHeader[0]);
	}
	printf("**** SA Log get, Left Log Count = %d***\n", ManSALog.SAlHeader.nCurCnt);
	

	return pSAlData;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsSAExist
**	AUTHOR:			Mike Zhang
**	DATE:		20 - Dec - 2010
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**	
**	Return the Log Type
**	NOTES:
**
*/
static BOOL
IsSAExist()
{	
	if (ManSALog.SAlHeader.nCurCnt > 0)
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
**	FUNCTION NAME:	DelOneSALog
**	AUTHOR:			Jeff Wang
**	DATE:		08 - May - 2009
**
**	DESCRIPTION:	
**			Delete One Log According Event ID	
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**	
**	TRUE if delete, else FALSE
**	NOTES:
**
*/
static BOOL
DelOneSALog()
{
	SALDATA*		 pSAlData	=	NULL;

	UINT		i	 =	0;
	UINT		j	 =	0;

	pSAlData = (SALDATA*)&(ManSALog.SAlDataHeader[i]);

	if (ManSALog.SAlDataHeader[i].pData) 
	{
		free(ManSALog.SAlDataHeader[i].pData);
		ManSALog.SAlDataHeader[i].pData = NULL;
	}		
	memset(&(ManSALog.SAlDataHeader[i]), 0, sizeof(SALDATA));
	if (ManSALog.SAlHeader.nCurCnt > 0) 
	{
		ManSALog.SAlHeader.nCurCnt--;
	}

	for(j = i; j < ManSALog.SAlHeader.nCurCnt; j++)
	{
		memcpy(&(ManSALog.SAlDataHeader[j]), &(ManSALog.SAlDataHeader[j+1]), sizeof(SALDATA));
	}

	memset(&(ManSALog.SAlDataHeader[ManSALog.SAlHeader.nCurCnt]), 0, sizeof(SALDATA));

	printf("SA Log has been deleted; Left = %d \n",ManSALog.SAlHeader.nCurCnt);
				
	
	return TRUE;


}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsSendSALog2Eth
**	AUTHOR:			Mike Zhang
**	DATE:		20 - Dec - 2010
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
static VOID 
SendSALog2Eth(SALDATA* pSAlData)
{
	MXMSG	msgSend;
	USHORT	nDataLen = 8;
	UCHAR	OutTimeData[6] = { 0 };
	
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");		
	msgSend.dwSrcMd		= MXMDID_SA;
	msgSend.dwDestMd	= MXMDID_ETH;
	
	msgSend.dwMsg		= FC_ALM_REPORT;	
	
	msgSend.dwParam		= g_TalkInfo.dwMCIP;
	
	msgSend.pParam = (UCHAR*)malloc(2+2+6);
	
	memcpy(msgSend.pParam, &nDataLen, 2);
	msgSend.pParam[2]	=	pSAlData->EventType;//Remove
	msgSend.pParam[3]	=	0;
	

	ConvertFromTime_t(OutTimeData, &pSAlData->EventTime);
	memcpy(&msgSend.pParam[4], OutTimeData, 6);
	
	MxPutMsg(&msgSend);
}







