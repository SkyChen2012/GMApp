/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	TalkLogReport.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		01 - Sep - 2008
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
#include <sys/stat.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "Dispatch.h"
#include "MXCommon.h"
#include "TalkLogReport.h"
#include "ModuleTalk.h"
#include "rtc.h"
#include "JpegApp.h"
#include "AccessLogReport.h"
#include "SALogReport.h"

/************** DEFINES **************************************************************/

#define DEBUG_TALKLOGREPORT

#define TALK_LOG_VERSION	0x01000000

/************** TYPEDEFS *************************************************************/


/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/
extern  pthread_mutex_t		MutexCovCapVideo;


//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

TLINFO	g_TLInfo;
MANTL	ManTalkLog;

static VOID AddOneLog(TLDATA *pTlData);
static BOOL	DelOneLog();

#ifdef DEBUG_TALKLOGREPORT
static VOID PrintAllLog();
static VOID PrintOneLog(TLDATA *pTlData);
#endif
/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TlLogProc
**	AUTHOR:			Jeff Wang
**	DATE:		21 - May - 2009
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

BOOL
TlLogProc(MXMSG* pMsg)
{	
	BOOL bRet = FALSE;
	
	if (NULL == pMsg)
	{
		return TRUE;
	}
	
	switch(pMsg->dwMsg)
	{
	case FC_ACK_REPORT_LOGDATA:
		{
#ifdef DEBUG_TALKLOGREPORT
			printf("Talk: FC_ACK_REPORT_LOGDATA\n");
#endif

			if (!DelOneLog())
			{
				printf("As Log has been deleted\n");
			}
		
			g_TLInfo.dwTickCount	=	GetTickCount();
			g_TLInfo.nStatus		=	TL_IDLE;
			
			bRet = TRUE;
		}
		break;
		
	default:
		break;
	}
	
	return bRet;
}  

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TLTimeOutCtrl
**	AUTHOR:			Jeff Wang
**	DATE:		21 - May - 2009
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
TLTimeOutCtrl()
{
	DWORD	dwTicks	=	0;

	if (TL_BUSY == g_TLInfo.nStatus) 
	{
		dwTicks = GetTickCount();
		if (dwTicks - g_TLInfo.dwTickCount > 2500) 
		{
#ifdef DEBUG_TALKLOGREPORT			
			printf("++++++++++++TLTimeOutCtrl++++++++++++\n");
#endif
			g_TLInfo.nStatus		=	TL_IDLE;
			g_TLInfo.dwTickCount	=	GetTickCount();
			g_TLInfo.bHavePic		=	FALSE;
			g_TLInfo.nPicLen		=	0;
			g_TLInfo.PicBuf			=	NULL;
		}
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RecordLogData
**	AUTHOR:			Jeff Wang
**	DATE:		01 - Sep - 2008
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
VOID
RecordLogData(ACTIONTYPE ActionType, ACTIONSTEP ActionStep, GMTALKINFO *pGMTalkInfo)
{	
	MSG		Msg;
	BOOL	bHaveMsg;
	DWORD		dwLogStartTime=GetTickCount();
	
	
	static TLDATA TlDataTalk;
	static TLDATA TlDataMon;
	static TLDATA TlDataUlk;

	printf("RecordLogData ActionType=%d,ActionStep=%d,bMMType=%d\n",ActionType,ActionStep,g_TalkInfo.talking.bMMType);
	if ((TYPE_MONITOR	== ActionType)
		&& (MON_TYPE_MC == g_TalkInfo.monitor.bType)) 
	{
		return;
	}
	
	if ((TYPE_CALLOUT == ActionType)
		&& (MM_TYPE_TALK_MC	== g_TalkInfo.talking.bMMType)) 
	{
		return;
	}
	
	switch(ActionType)
	{
	case TYPE_CALLOUT:
		{
			TlDataTalk.Type = STATUS_CALL;
			if (STEP_START == ActionStep)
			{
				strcpy(TlDataTalk.ProCode, pGMTalkInfo->szTalkDevCode);
				strcpy(TlDataTalk.ConCode, pGMTalkInfo->talking.szTalkDestDevCode);
				
				TlDataTalk.state	 =   CS_NEWCALL;
				TlDataTalk.TimeStart = GetSysTime();
			}
			else if(STEP_PICK == ActionStep)
			{
				TlDataTalk.state	 =	CS_COMPLETE;
				TlDataTalk.TimePick = GetSysTime();
			}
			else if (STEP_END == ActionStep)
			{
				TlDataTalk.TimeEnd = GetSysTime();
				if(GetLeavePhotoStatus())
				{
					TlDataTalk.bHavePic	=	TRUE;
				}
				else
				{
					TlDataTalk.bHavePic	=	FALSE;
				}
				if((MM_TYPE_TALK_EHV	== g_TalkInfo.talking.bMMType) && (TlDataTalk.bHavePic))
				{		
					printf("11111111@@@@@@@@@@@@@@@g_TalkInfo.talking.bHavePhotoSaved\n");
					while(!bYUV2JPG)
					{
						if(GetTickCount()-dwLogStartTime>10*1000)
						{
							printf("[error]GetTickCount()=%d,dwLogStartTime=%d\n",GetTickCount(),dwLogStartTime);
							return;
						}
						else
						{
							bHaveMsg = PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE);
							if (bHaveMsg)
							{
								if (Msg.message == WM_QUIT)
								{
									break;
								}
								TranslateMessage(&Msg);
								DispatchMessage(&Msg);
							}
							MwHookProcess();
							WatchDog();
							usleep(1000);
						}
					}
					printf("2222222222@@@@@@@@@@@@@@@g_TalkInfo.talking.bHavePhotoSaved\n");
					g_TalkInfo.talking.bHavePhotoSaved	=	TRUE;
					bYUV2JPG = FALSE;
					//if(IsMMRunning())
					//{
						GMStopTalking();
					//}
					
				}

				if (0 == strcmp(pGMTalkInfo->szTalkDevCode,""))
				{
					GetTalkDeviceCode();										
				}
	
				TlDataTalk.nPicLen = nJpgBufLen;
				TlDataTalk.PicBuf = (BYTE*)malloc(TlDataTalk.nPicLen);
				

				if(MM_TYPE_TALK_GM	== g_TalkInfo.talking.bMMType)
				{					
					nJpgBufLen	=	0;
					memset(JpgPicBuf,0,PHOTO_WIDTH * PHOTO_HEIGHT);	
					TlDataTalk.nPicLen = nJpgBufLen;
					memcpy(TlDataTalk.PicBuf, JpgPicBuf, TlDataTalk.nPicLen);
				}
				else
				{
					memcpy(TlDataTalk.PicBuf, JpgPicBuf, TlDataTalk.nPicLen);
//					memset(JpgPicBuf,0,PHOTO_WIDTH * PHOTO_HEIGHT);	
				}
				
#ifdef DEBUG_TALKLOGREPORT
				printf("Talk Log\n");				
				PrintOneLog(&TlDataTalk);
#endif				
				AddOneLog(&TlDataTalk);
				memset(&TlDataTalk, 0, sizeof(TLDATA));
			}	
		}
		break;

	case TYPE_MONITOR:
		{	
			TlDataMon.Type = STATUS_MONITOR;
			if (STEP_START == ActionStep) 
			{
				strcpy(TlDataMon.ProCode, pGMTalkInfo->monitor.szMonSrcDevCode);
				strcpy(TlDataMon.ConCode, pGMTalkInfo->szTalkDevCode);

				TlDataMon.state	 =   MS_SUCCESSFUL;
				TlDataMon.TimeStart = GetSysTime();
			}			
			else if (STEP_END == ActionStep) 
			{
				TlDataMon.TimeEnd = GetSysTime();

#ifdef DEBUG_TALKLOGREPORT
				printf("Monitor Log\n");				
				PrintOneLog(&TlDataMon);
#endif
				AddOneLog(&TlDataMon);
				memset(&TlDataMon, 0, sizeof(TLDATA));
			} 
		}
		break;
	case TYPE_UNLOCK:
		{
			TlDataUlk.Type = STATUS_UNLOCK;
			if ((STEP_START == ActionStep) 
				||(STEP_END == ActionStep)) 
			{
				strcpy(TlDataUlk.ProCode, pGMTalkInfo->unlock.szULDestDevCode);
				strcpy(TlDataUlk.ConCode, pGMTalkInfo->szTalkDevCode);
				TlDataUlk.state		= US_SUCCESSFUL;
				TlDataUlk.TimeStart	= GetSysTime();
				TlDataUlk.TimeEnd		= GetSysTime();

				printf("%s,%d,bType:%d,bMMType:%d\n",__func__,__LINE__,
											g_TalkInfo.monitor.bType,
											g_TalkInfo.talking.bMMType);
				
				if ((MON_TYPE_EHV == g_TalkInfo.monitor.bType)||
					(MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType))
				{
#ifdef DEBUG_TALKLOGREPORT							
					PrintOneLog(&TlDataUlk);
#endif
					AddOneLog(&TlDataUlk);
				}
				memset(&TlDataUlk, 0, sizeof(TLDATA));
				TlReport();
			}
		}
		break;

	default:
		break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SaveLog2Mem
**	AUTHOR:			Jeff Wang
**	DATE:		21 - May - 2009
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
AddOneLog(TLDATA *pTlData)
{
	INT j	=	0;

	if (ManTalkLog.TlHeader.nMaxCnt == ManTalkLog.TlHeader.nCurCnt)
	{
		if (ManTalkLog.TlDataHeader[0].PicBuf)
		{
			free(ManTalkLog.TlDataHeader[0].PicBuf);
		}
		memset(&(ManTalkLog.TlDataHeader[0]), 0, sizeof(TLDATA));

		ManTalkLog.TlHeader.nCurCnt--;		
		
		for(j = 0; j < ManTalkLog.TlHeader.nCurCnt; j++)
		{
			memcpy(&(ManTalkLog.TlDataHeader[j]), &(ManTalkLog.TlDataHeader[j+1]), sizeof(TLDATA));
		}
		
		memset(&(ManTalkLog.TlDataHeader[ManTalkLog.TlHeader.nCurCnt]), 0, sizeof(TLDATA));
	}

	memcpy(&(ManTalkLog.TlDataHeader[ManTalkLog.TlHeader.nCurCnt]),	pTlData, sizeof(TLDATA));
	
	if (ManTalkLog.TlHeader.nCurCnt < ManTalkLog.TlHeader.nMaxCnt) 
	{
		ManTalkLog.TlHeader.nCurCnt++;
	}

#ifdef DEBUG_TALKLOGREPORT
	printf("Add Talk Log Count = %d\n", ManTalkLog.TlHeader.nCurCnt);
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendLogData2MC
**	AUTHOR:			Jeff Wang
**	DATE:		13 - May - 2008
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
TlSendLog2Eth(BYTE* pSendBuf, INT nSendLen)
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");		
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ETH;
	
	msgSend.dwMsg		= FC_REPORT_LOGDATA;	
	
	msgSend.dwParam		= g_TalkInfo.dwMCIP;
#ifdef DEBUG_TALKLOGREPORT
	printf("---------TlSendLog2Eth: g_TalkInfo.dwMCIP = %x\n",g_TalkInfo.dwMCIP);
#endif
	
	msgSend.pParam		= (BYTE*)malloc(nSendLen + sizeof(INT));
	
	memcpy(msgSend.pParam, &nSendLen, sizeof(INT));
	memcpy(msgSend.pParam + sizeof(INT), pSendBuf, nSendLen);   
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BuildLogData
**	AUTHOR:			Jeff Wang
**	DATE:		13 - May - 2008
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

static WORD 
BuildOneTalkLog(TLDATA *pTlData, BYTE *pOUT)
{
	BYTE BufferTime[6] = { 0 };
	WORD nAppLen = LEN_LOGDATA;	
	
	memcpy(pOUT + 0, &pTlData->Type,  1);
	memcpy(pOUT + 1, &pTlData->state, 1);
	
	ConvertFromTime_t(BufferTime, &pTlData->TimeStart);
	memcpy(pOUT + 2, BufferTime, 6);

	ConvertFromTime_t(BufferTime, &pTlData->TimePick);
	memcpy(pOUT + 8, BufferTime, 6);

	ConvertFromTime_t(BufferTime, &pTlData->TimeEnd);
	memcpy(pOUT + 14, BufferTime, 6);	
	
	memcpy(pOUT + 20, pTlData->ProCode, IRIS_CODE_LEN);
	memcpy(pOUT + 39, pTlData->ConCode, IRIS_CODE_LEN);

	return nAppLen;
}  

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BuildLogData
**	AUTHOR:			Jeff Wang
**	DATE:		13 - May - 2008
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
ConvertFromTime_t(UCHAR *pOUTTime, time_t *pTime_tTime)
{
	struct tm *ptmTime = 0;	
	ptmTime = localtime(pTime_tTime);
	
	pOUTTime[0] = (UCHAR)(ptmTime->tm_year-100);
	pOUTTime[1] = (UCHAR)(ptmTime->tm_mon+1);
	pOUTTime[2] = (UCHAR)(ptmTime->tm_mday);
	pOUTTime[3] = (UCHAR)(ptmTime->tm_hour);
	pOUTTime[4] = (UCHAR)(ptmTime->tm_min);
	pOUTTime[5] = (UCHAR)(ptmTime->tm_sec);
}  


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TlLogInit
**	AUTHOR:			Jeff Wang
**	DATE:		27 - May - 2008
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
TlLogInit()
{
	ManTalkLog.TlHeader.dwVersion = TALK_LOG_VERSION;
	ManTalkLog.TlHeader.nCurCnt	= 0;
	ManTalkLog.TlHeader.nMaxCnt	= MAX_TALK_LOG_COUNT;
	memset(ManTalkLog.TlDataHeader, 0, ManTalkLog.TlHeader.nMaxCnt * sizeof(TLDATA));

	g_TLInfo.dwTickCount	=	GetTickCount();
	g_TLInfo.nStatus		=	TL_IDLE;
	
#ifdef DEBUG_TALKLOGREPORT
	printf("Talk Log Inital...\n");
#endif
}  

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TlLogExit
**	AUTHOR:			Jeff Wang
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
**	
*/

VOID
TlLogExit()
{
	memset(&ManTalkLog, 0, sizeof(ManTalkLog));

#ifdef DEBUG_TALKLOGREPORT
	printf("Talk Log Exit\n");
#endif
}  

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsMCOnline
**	AUTHOR:			Jeff Wang
**	DATE:		14 - May - 2008
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
BOOL IsMCOnline()
{
	return g_TalkInfo.bMCStatus;
}


BOOL
IsReportIdle()
{
	if (AL_IDLE == g_AsLogInfo.nStatus
		&& TL_IDLE	==	g_TLInfo.nStatus
		&& SAL_IDLE	==	g_SALogInfo.nStatus)
//		&& ST_ORIGINAL == g_TalkInfo.talking.dwTalkState
//		&& ST_ORIGINAL == g_TalkInfo.monitor.dwMonState
//		&& ST_ORIGINAL == g_TalkInfo.unlock.dwUnLockState) 
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
**	FUNCTION NAME:	DelOneLog
**	AUTHOR:			Jeff Wang
**	DATE:		21 - May - 2009
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
DelOneLog()
{
	UINT		j	 =	0;
	
	if (0 == ManTalkLog.TlHeader.nCurCnt) 
	{
		printf("Talk Log Empty\n");
		return FALSE;
	}

#ifdef DEBUG_TALKLOGREPORT
	PrintOneLog(&(ManTalkLog.TlDataHeader[0]));
#endif

	if (ManTalkLog.TlDataHeader[0].PicBuf)
	{
		free(ManTalkLog.TlDataHeader[0].PicBuf);
	}
	memset(&(ManTalkLog.TlDataHeader[0]), 0, sizeof(TLDATA));

	if (ManTalkLog.TlHeader.nCurCnt > 0) 
	{
		ManTalkLog.TlHeader.nCurCnt--;
	}

	for(j = 0; j < ManTalkLog.TlHeader.nCurCnt; j++)
	{
		memcpy(&(ManTalkLog.TlDataHeader[j]), &(ManTalkLog.TlDataHeader[j+1]), sizeof(TLDATA));
	}

	memset(&(ManTalkLog.TlDataHeader[ManTalkLog.TlHeader.nCurCnt]), 0, sizeof(TLDATA));	

#ifdef DEBUG_TALKLOGREPORT
	printf("****First Log deleted, Left Log Count = %d***\n", ManTalkLog.TlHeader.nCurCnt);
#endif
	return TRUE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsTlExist
**	AUTHOR:			Jeff Wang
**	DATE:		21 - May - 2009
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
IsTlExist()
{	
	if (ManTalkLog.TlHeader.nCurCnt > 0)
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
**	FUNCTION NAME:	GetOneTalkLog
**	AUTHOR:			Jeff Wang
**	DATE:		21 - May - 2009
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
static TLDATA*
GetOneTalkLog()
{
	TLDATA*	 pTlData = NULL;
		
	if (0 < ManTalkLog.TlHeader.nCurCnt)
	{
		pTlData = (TLDATA*)(ManTalkLog.TlDataHeader);

#ifdef DEBUG_TALKLOGREPORT
		PrintOneLog(pTlData);
		printf("****First Log get, Left Log Count = %d***\n", ManTalkLog.TlHeader.nCurCnt);
#endif		
	}	
	return pTlData;
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TlReport
**	AUTHOR:			Jeff Wang
**	DATE:		21 - May - 2009
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
TlReport()
{
	TLDATA*		 pTlData	=	NULL;
	BYTE		 SdLogBuf[1024] = { 0 };
	UINT		 nSdLogLen		=	0;

	if (!(IsMCOnline() && IsReportIdle()))
	{
		return;
	}

	if (IsTlExist()) 
	{
		pTlData		=	GetOneTalkLog();
		nSdLogLen	=	BuildOneTalkLog(pTlData, SdLogBuf);
		TlSendLog2Eth(SdLogBuf, nSdLogLen);

		g_TLInfo.dwTickCount	=	GetTickCount();
		g_TLInfo.nStatus		=	TL_BUSY;
		g_TLInfo.bHavePic		=	pTlData->bHavePic;
		g_TLInfo.nPicLen		=	pTlData->nPicLen;
		g_TLInfo.PicBuf			=	pTlData->PicBuf;
	}
}


#ifdef DEBUG_TALKLOGREPORT
static VOID
PrintAllLog()
{
	UINT nIndex = 0;
	TLDATA *pTlData = NULL;
	
	printf("*********All Talk log*********************\n");
	for(nIndex=0;nIndex<ManTalkLog.TlHeader.nCurCnt;nIndex++)
	{		
		pTlData = (TLDATA*)&(ManTalkLog.TlDataHeader[nIndex]);
		PrintOneLog(pTlData);
	}

}

static VOID
PrintOneLog(TLDATA *pTlData)
{	
	printf("Type: %d state: %d Timestart: %x PicBuf: %p, PicLen: %d\n", 
		pTlData->Type,
		pTlData->state,
		pTlData->TimeStart,
		pTlData->PicBuf,
		pTlData->nPicLen);
}

#endif
