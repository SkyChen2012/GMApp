/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AccessLogReport.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		06 - May - 2009
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
#include "AccessLogReport.h"
#include "TalkLogReport.h"
#include "rtc.h"

/************** DEFINES **************************************************************/
//#define ACCESSLOGREPORT_DEBUG

#define AS_LOG_VERSION				0x00010101

/************** TYPEDEFS *************************************************************/


/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/


//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static BOOL	DelOneByEventId(UINT EventID);
static VOID	AddOneAlLog(ALDATA*	 pAlData);

static BOOL	IsAsExist();

static ALDATA*	GetOneAsLog();
static INT	BuildOneAsLog(ALDATA* pAlData, BYTE* pTotalLog);
static VOID AsSendLog2Eth(BYTE* pAsLogData, INT nAsLogLen);

static	UINT		g_EventId = 1;
static	MANASLOG	ManAsLog;
ASLOGINFO	g_AsLogInfo;
static UINT	g_LastLogState = 0;
/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RecordSwipeCardLog
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
static VOID
AsRecordLogInit()
{
	ManAsLog.AlHeader.dwVersion = AS_LOG_VERSION;
	ManAsLog.AlHeader.nCurCnt	= 0;
	ManAsLog.AlHeader.nMaxCnt	= MAX_AS_LOG_COUNT;
	memset(ManAsLog.AlDataHeader, 0, ManAsLog.AlHeader.nMaxCnt * sizeof(ALDATA));
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
AsLogInit()
{
	g_AsLogInfo.nStatus = AL_IDLE;
	AsRecordLogInit();
#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__	
	LoadALogFromMem();
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsLogReport
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
AsLogReport()
{
	ALDATA*		 pAlData	=	NULL;
	BYTE		 SdLogBuf[1024] = { 0 };
	UINT		 nSdLogLen		=	0;
    
	if (IsAsExist()) 
	{
		pAlData		=	GetOneAsLog();
		nSdLogLen	=	BuildOneAsLog(pAlData, SdLogBuf);

		AsSendLog2Eth(SdLogBuf, nSdLogLen);
        
		g_AsLogInfo.nCurEventID =	pAlData->EventID;
		g_AsLogInfo.dwTickCount	=	GetTickCount();
		g_AsLogInfo.nStatus		=	AL_BUSY;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsLogReport
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
BOOL
AsLogProc(MXMSG* pMsg)
{
	BOOL bRet = FALSE;
	UINT EventID	=	0;
	
	if (NULL == pMsg)
	{
		return TRUE;
	}
	
	switch(pMsg->dwMsg)
	{
	case FC_ACK_AC_REPORT_EVENT:
		{
			memcpy(&EventID, pMsg->pParam + 5, sizeof(INT));
			if (EventID == g_AsLogInfo.nCurEventID)
			{
				if (!DelOneByEventId(g_AsLogInfo.nCurEventID))
				{
					printf("As Log has been deleted\n");
				}
#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
				else
					SaveALog2Mem();
#endif
			}
			
			g_AsLogInfo.nCurEventID =	0;
			g_AsLogInfo.dwTickCount	=	GetTickCount();
			g_AsLogInfo.nStatus		=	AL_IDLE;
			
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
**	FUNCTION NAME:	AsLogReportProc
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
AsLogReportTimeOut()
{
	DWORD	dwTicks	=	0;

	if (AL_BUSY == g_AsLogInfo.nStatus) 
	{
		dwTicks = GetTickCount();
		if (dwTicks - g_AsLogInfo.dwTickCount > IRIS_ACK_LOG_TIMEOUT) 
		{
			g_AsLogInfo.nStatus = AL_IDLE;
			g_AsLogInfo.dwTickCount	=	GetTickCount();
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RecordSwipeCardLog
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
RecordSwipeCardLog(BYTE CardMode, CHAR* pCSN, BYTE ReaderPort, BYTE ReaderID, BYTE GateID, BYTE InOut, BYTE Result, UINT CardID)
{
	ALDATA	AlData;

	memset(&AlData, 0, sizeof(ALDATA));
	AlData.bValid		=	TRUE;	
	AlData.EventType	=	TYPE_SWIPE_CARD_V2;
	AlData.EventTime	=	GetSysTime();
	AlData.EventID		=	g_EventId++;

	AlData.DataLen		=	SWIPECARD_DATA_LEN;	

	AlData.pData[SWIPECARD_CSN_LEN_OFFSET]		=	SWIPECARD_CSN_LEN;
	AlData.pData[SWIPECARD_CDMODE_OFFSET]		=	CardMode;
	memcpy(&AlData.pData[SWIPECARD_CSN_OFFSET], pCSN, SWIPECARD_CSN_LEN);
	AlData.pData[SWIPECARD_READERPORT_OFFSET]	=	ReaderPort;
	AlData.pData[SWIPECARD_READERID_OFFSET]		=	ReaderID;
	AlData.pData[SWIPECARD_GATEID_OFFSET]		=	GateID;
	AlData.pData[SWIPECARD_INOUT_OFFSET]		=	InOut;
    (SWIPECARD_PASSWORD_ERROR == Result)&&(Result = 6);
	AlData.pData[SWIPECARD_RESULT_OFFSET]		=	Result;
	AlData.pData[SWIPECARD_CARDID_OFFSET]		=	CardID;
#ifdef ACCESSLOGREPORT_DEBUG	
	printf("%s AddOneAlLog:CardMode=%d,pCSN=%s,ReaderPort=%d,ReaderID=%d,GateID=%d,InOut=%d,Result=%d,CardID=%d\n",__FUNCTION__,CardMode,pCSN,ReaderPort,ReaderID,GateID,InOut,Result,CardID);
#endif
#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
	SetLastSwipeCardInfo();
#endif
	AddOneAlLog(&AlData);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RecordOpenMnlLog
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
RecordOpenMnlLog(BYTE ButtonID, BYTE GateID)
{
	ALDATA	AlData;

	memset(&AlData, 0, sizeof(ALDATA));
	AlData.bValid		=	TRUE;	
	AlData.EventType	=	TYPE_GATE_OPEN_MANUAL;
	AlData.EventTime	=	GetSysTime();
	AlData.EventID		=	g_EventId++;

	AlData.DataLen		=	MANUALOPEN_DATA_LEN;
	
	AlData.pData[MANUALOPEN_BUTTONID_OFFSET]	=	ButtonID;
	AlData.pData[MANUALOPEN_GATEID_OFFSET]		=	GateID;
#ifdef ACCESSLOGREPORT_DEBUG	
	printf("%s AddOneAlLog\n",__FUNCTION__);
#endif
	AddOneAlLog(&AlData);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RecordOpenMnlLog
**	AUTHOR:			Jeff Wang
**	DATE:		07 - May - 2009
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
RecordDoorStsLog(BYTE GateSts, BYTE GateID)
{
	ALDATA	AlData;

	memset(&AlData, 0, sizeof(ALDATA));
	AlData.bValid		=	TRUE;	
	AlData.EventType	=	GateSts;
	AlData.EventTime	=	GetSysTime();
	AlData.EventID		=	g_EventId++;
	
	AlData.DataLen		=	DOORSTATUS_DATA_LEN;	
	
	AlData.pData[DOORSTATUS_GATEID_OFFSET]		=	GateID;
#ifdef ACCESSLOGREPORT_DEBUG	
	printf("%s AddOneAlLog\n",__FUNCTION__);
#endif
	AddOneAlLog(&AlData);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RecordOpenMnlLog
**	AUTHOR:			Jeff Wang
**	DATE:		07 - May - 2009
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
RecordAsAlarmLog(BYTE AlarmType)
{
	ALDATA	AlData;

	memset(&AlData, 0, sizeof(ALDATA));
	AlData.bValid		=	TRUE;	
	AlData.EventType	=	AlarmType;
	AlData.EventTime	=	GetSysTime();
	AlData.EventID		=	g_EventId++;	
//	AlData.DataLen		=	0;	
//	AlData.pData		=	NULL;
	AlData.DataLen		=	1;	
	AlData.pData[0]		=	1;
#ifdef ACCESSLOGREPORT_DEBUG	
	printf("%s AddOneAlLog\n",__FUNCTION__);
#endif
	AddOneAlLog(&AlData);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddOneAlLog
**	AUTHOR:			Jeff Wang
**	DATE:		07 - May - 2009
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
AddOneAlLog(ALDATA*	 pAlData)
{
	UINT j	=	0;

	if (ManAsLog.AlHeader.nCurCnt == ManAsLog.AlHeader.nMaxCnt) 
	{
		memset(&(ManAsLog.AlDataHeader[0]), 0, sizeof(ALDATA));
		
		if (ManAsLog.AlHeader.nCurCnt > 0) 
		{
			ManAsLog.AlHeader.nCurCnt--;
		}
		
		for(j = 0; j < ManAsLog.AlHeader.nCurCnt; j++)
		{
			memcpy(&(ManAsLog.AlDataHeader[j]), &(ManAsLog.AlDataHeader[j+1]), sizeof(ALDATA));
		}

		memset(&(ManAsLog.AlDataHeader[ManAsLog.AlHeader.nCurCnt]), 0, sizeof(ALDATA));
	}

	memcpy(&(ManAsLog.AlDataHeader[ManAsLog.AlHeader.nCurCnt]), pAlData, sizeof(ALDATA));
	
	if (ManAsLog.AlHeader.nCurCnt < ManAsLog.AlHeader.nMaxCnt) 
	{
		ManAsLog.AlHeader.nCurCnt++;
	}
#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__	
	SaveALog2Mem();
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetSwipeCardLog
**	AUTHOR:			Jeff Wang
**	DATE:		07 - May - 2009
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
static ALDATA*
GetOneAsLog()
{
	ALDATA*	 pAlData = NULL;
		
	if (0 < ManAsLog.AlHeader.nCurCnt)
	{
		pAlData = (ALDATA*)&(ManAsLog.AlDataHeader[0]);
	}

	return pAlData;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsSwipeCardLogExist
**	AUTHOR:			Jeff Wang
**	DATE:		07 - May - 2009
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
IsAsExist()
{	
	if (ManAsLog.AlHeader.nCurCnt > 0)
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
**	FUNCTION NAME:	DelOneByEventId
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
DelOneByEventId(UINT EventID)
{
	ALDATA*		 pAlData	=	NULL;

	UINT		i	 =	0;
	UINT		j	 =	0;

	for(i = 0; i < ManAsLog.AlHeader.nCurCnt; i++)
	{
		pAlData = (ALDATA*)&(ManAsLog.AlDataHeader[i]);
		if (ManAsLog.AlDataHeader[i].EventID == EventID)
		{
			memset(&(ManAsLog.AlDataHeader[i]), 0, sizeof(ALDATA));
			if (ManAsLog.AlHeader.nCurCnt > 0) 
			{
				ManAsLog.AlHeader.nCurCnt--;
			}

			for(j = i; j < ManAsLog.AlHeader.nCurCnt; j++)
			{
				memcpy(&(ManAsLog.AlDataHeader[j]), &(ManAsLog.AlDataHeader[j+1]), sizeof(ALDATA));
			}
	
			memset(&(ManAsLog.AlDataHeader[ManAsLog.AlHeader.nCurCnt]), 0, sizeof(ALDATA));
			
			return TRUE;
		}
	}

	return FALSE;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BuildOneAsLog
**	AUTHOR:			Jeff Wang
**	DATE:		06 - May - 2009
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
static INT
BuildOneAsLog(ALDATA* pAlData, BYTE* pTotalLog)
{
	INT		nTotalLogLen	=	ASLOG_COMMON_LEN;
	WORD	nTotalDataLen	=	0;

	memcpy(&pTotalLog[ASLOG_EVENTTYPE_OFFSET], &pAlData->EventType, ASLOG_EVENTTYPE_LEN);
	memcpy(&pTotalLog[ASLOG_EVENTTIME_OFFSET], &pAlData->EventTime, ASLOG_EVENTTIME_LEN);
	memcpy(&pTotalLog[ASLOG_EVENTID_OFFSET],   &pAlData->EventID, ASLOG_EVENTID_LEN);

	if (pAlData->DataLen > 0)
	{
		nTotalDataLen	=	pAlData->DataLen + sizeof(pAlData->DataLen);
		memcpy(&pTotalLog[ASLOG_DATALEN_OFFSET],   &(pAlData->DataLen), ASLOG_DATALEN_LEN);
		memcpy(&pTotalLog[ASLOG_DATA_OFFSET],	   pAlData->pData, pAlData->DataLen);
		
		nTotalLogLen +=	nTotalDataLen;
	}

	return nTotalLogLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendAsLog2MC
**	AUTHOR:			Jeff Wang
**	DATE:		06 - May - 2009
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
AsSendLog2Eth(BYTE* pAsLogData, INT nAsLogLen)
{
	MXMSG	msgSend;

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");		
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ETH;
	
	msgSend.dwMsg		= FC_AC_REPORT_EVENT;	
	
	msgSend.dwParam		= g_TalkInfo.dwMCIP;
	
	msgSend.pParam		= (BYTE*)malloc(nAsLogLen + sizeof(UINT));

	memcpy(msgSend.pParam, &nAsLogLen, sizeof(UINT));
	memcpy(msgSend.pParam + sizeof(UINT), pAsLogData, nAsLogLen);   

	MxPutMsg(&msgSend);
#ifdef ACCESSLOGREPORT_DEBUG	
	printf("========FC_AC_REPORT_EVENT========\n");
#endif
}
#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
ALDATA* GetLastSwipCardLog()
{
	ALDATA*	 pAlData = NULL;
	UINT 	i, Count;
	Count = ManAsLog.AlHeader.nCurCnt;
	if(Count)
	{
		for(i = 0; i < Count; i++)
		{
			if(ManAsLog.AlDataHeader[Count - i - 1].EventType == TYPE_SWIPE_CARD_V2)
			{
				pAlData = (ALDATA*)&(ManAsLog.AlDataHeader[Count - i - 1]);
				break;
			}
		}
	}

	return pAlData;
	
}

VOID ClearLastSwipeCardInfo()
{
	g_LastLogState = 0;
}
VOID SetLastSwipeCardInfo()
{
	g_LastLogState = AS_LOG_VERSION;
}
BOOL HaveLastSwipeCardLog()
{
	if(g_LastLogState == AS_LOG_VERSION)
		return TRUE;
	else
		return FALSE;
}
UINT GetSwipeCardLogCntByGate()
{
	UINT 	i, Count = 0;

	for(i = 0; i < ManAsLog.AlHeader.nCurCnt; i++)
	{
		if(ManAsLog.AlDataHeader[i].EventType == TYPE_SWIPE_CARD_V2)
		{
			Count += 1;
		}
	}
	return Count;
}
ALDATA* GetSwipeCardLogByIndex(USHORT index)
{
	UINT i, Count = 0;
	ALDATA*	 pAlData = NULL;
	for(i = ManAsLog.AlHeader.nCurCnt - 1; i >= 0; i--)
	{
		if(ManAsLog.AlDataHeader[i].EventType == TYPE_SWIPE_CARD_V2)
		{
			Count += 1;
		}
		if(Count == index)
		{
			pAlData = (ALDATA*)&(ManAsLog.AlDataHeader[i]);
			break;
		}
	}
	return pAlData;
}

void SaveALog2Mem()
{
	FILE * fd			= NULL;

	if ((fd = fopen(ALOGFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fwrite(&ManAsLog, sizeof(ManAsLog), (size_t)1, fd);

#ifdef DEBUG_AS_COMMON		
		printf("Save Log to memory\n");
#endif		
		fclose(fd);
	}
	else
	{
		printf("ALOG file open error!\n");
	}

}

void LoadALogFromMem()
{
		FILE * fd			= NULL;
	
		if ((fd = fopen(ALOGFILE, "r+")) != NULL)
		{
			fseek(fd, 0, SEEK_SET);
			fread(&ManAsLog, sizeof(ManAsLog), (size_t)1, fd);
	
			fclose(fd);
		}
		else
		{
			printf("ALOG file open error,create the file\n");
			if ((fd = fopen(ALOGFILE, "w+")) != NULL)
			{
				AsRecordLogInit();
				SaveALog2Mem();
			}
			else
			{
	
				printf("ALOG file open error,Write error\n");
				return;
			}
		}
			
		if( ManAsLog.AlHeader.dwVersion != AS_LOG_VERSION ) 	
		{	
			printf("Version error\n");
			return;
		}
#ifdef ACCESS_COMMON_DEBUG	
		printf("%s,current count =%d , max count=%d\n",__FUNCTION__,ManAsLog.AlHeader.nCurCnt,ManAsLog.AlHeader.nMaxCnt);
#endif
		if ( ManAsLog.AlHeader.nMaxCnt != MAX_AS_LOG_COUNT || ManAsLog.AlHeader.nMaxCnt < ManAsLog.AlHeader.nCurCnt)
		{
			printf("Card count error\n");
			return;
		}

}
#endif

/************************************************************************************************
**FunctionName    : sendForceUlkLog2MC
**Function        : 
**InputParameters :                                                                          
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
void sendForceUlkLog2MC(void)
{
    ALDATA	AlData;
	memset(&AlData, 0, sizeof(ALDATA));
	AlData.bValid		=	TRUE;	
	AlData.EventType	=	TYPE_FORCE_UNLOCK;
	AlData.EventTime	=	GetSysTime();
	AlData.EventID		=	g_EventId++;
	AlData.DataLen		=	1;	
    AlData.pData[0]	    =	1;
	AddOneAlLog(&AlData);
}

