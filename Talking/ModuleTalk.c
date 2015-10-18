/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	ModuleTalk.c
**
**	AUTHOR:		Harry Qian
**
**	DATE:		25 - Sep - 2006
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
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <errno.h>
#include <sys/timeb.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <signal.h>
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
#include "TalkLogReport.h"
#include "MenuParaProc.h"
#include "AMT.h"
#include "PioApi.h"
#include "IOControl.h"
#include "RS485.h"
#include "JpegApp.h"
#include "UnlockWnd.h"
#include "AccessProc.h"
#include "AccessLogReport.h"
#include "GPVideo.h"
#include "ParaSetting.h"

/************** DEFINES **************************************************************/
//#define	TALK_DISP_COUNT	2
/************** TYPEDEFS *************************************************************/
#define TALK_DEBUG
#define TALK_TIMETICK_DEBUG
#define DEBUG_CALL_MCGM
#define HV_TALKMD_DEBUG
#define TALK_LOG_DEBUG
//#define DNS_DEBUG
//#define GM_DEBUG
//#define PHOTO_DEBUG
//#define TKL_DEBUG
#define DNS_DEBUG
//#define TKCODE_DEBUG
//#define MGM_DEBUG
//#define	MM_LEAK_DEBUG
//#define LMSG_DEBUG
//#define FW_DEBUG

/************** STRUCTURES ***********************************************************/

typedef	struct _TAKEPHOTO
{
	BYTE *	pBuf ;
	UINT	nBufLen;
	UINT	nFileLen;
	UINT	nRetryCnt;
	DWORD	nIP;
	DWORD	dwTick;
	BOOL	bReady;
	UINT	nStatus;
	char	szCode[20];
	char	szAddr[100];
} TAKEPHOTO;

typedef	struct _TALKEVENTREPORT
{
	UINT	nStatus;
	DWORD	dwTick;
	UINT	nRefNum;
	UINT	nCurrRefNum;// the refernum of talk event during talking...
} TLEREPORT;

typedef	struct _TalkFuncInfo
{
	BYTE	bDirection;
	BYTE	bAudio;
	BYTE	bVideo;
	BYTE	bValid;
	DWORD	dwAudioFormat;
	DWORD	dwVideoFormat;
} TALKFUNCINFO;


typedef	struct _TalkStreamInfo
{
	BYTE 	bDestAudioCapture;
	BYTE 	bDestAudioFormat;
	BYTE 	bDestVideoCapture;
	BYTE 	bDestVideoFormat;
	
	BYTE 	bLocAudioCapture;
	BYTE 	bLocAudioFormat;
	BYTE 	bLocVideoCapture;
	BYTE 	bLocVideoFormat;
} TALKSTREAMINFO;
/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/
DWORD					GetMasterHVIP(EthResolvInfo * pResolvInfo);

//static void				ChangeTalkLogStatus(BYTE nType, BYTE bStatus, UINT nFileNameRefNum);

static	void			TalkSendGMcallOutDevice(void);
static void TalkSendGMHangupCmd(void);
static void InstallSigAlrmHandler();
static void TimeHandler(int signo);

void TalkSendGMhangupToNet(void);
void MTStopMonitorVideo();
void TalkSendHVPickupACK(MXMSG * pmsg);
void TalkSendGMPickupACK(MXMSG * pmsg);

static void	TalkSendHVcallGMAck(MXMSG *pmsg);

static void	StopGMMonState();

static void GMSendPhoto2Net();
static void TalkSavePhotoNote2MM();

void UKTimeOutCtrl(void);
void UnlockGateStart(BOOL LCenable, unsigned char mode, DWORD IP, CHAR RdNum[RD_CODE_LEN], BYTE CSN[CSN_LEN]);
void UnlockGateEnd();

static void TalkErrorSoundNote2MM();
static BOOL	IsSlaveHV(DWORD dwIPAddr);
static void TalkSendLedStsNote(DWORD LedSts);
static void GMStartCallingVideo(void);
static void MTStartMonitorVideo(void);
static BOOL CheckMMFormat(BYTE bType,BYTE bCapture,BYTE bFormat);
static void GetMMFormat(MXMSG *pmsg);
static void TalkWriteMMFormat(unsigned char * pData);

static VOID TalkSendUnlockGate2ACC(BYTE byUnlockType);


/************** LOCAL DECLARATIONS ***************************************************/

GMTALKINFO g_TalkInfo;
TAKEPHOTO	g_TakePhoto;
TLEREPORT	g_TLEReport;
TALKFUNCINFO	g_TalkFunc;
TALKSTREAMINFO	g_TalkStream;
DWORD g_MonitorTime = 0; //监视时间

static MXMSG g_MxMsgBk;
static MXMSG HVMsg;
static BOOL  bPhotoSaved = FALSE;
static DWORD nCurTick = 0; // 收到FC_PICKUP_HV  的时间

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkInit()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Initial the talk module.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TalkInit()
{
	memset(&g_TalkInfo, 0, sizeof(g_TalkInfo));
	memset(&g_TakePhoto, 0, sizeof(g_TakePhoto));
	TlLogInit();
	g_TakePhoto.nRetryCnt = TAKE_PHOTO_RETRY_NUM;
	g_TLEReport.nStatus = TLE_IDLE;

	g_TalkInfo.dwMCIP = g_SysConfig.MCIPSERVER; 

	g_TalkInfo.talking.bNeedPhotoSaved	=	FALSE;
	g_TalkInfo.talking.bHavePhotoSaved	=	FALSE;

	g_TalkInfo.unlock.dwUnLockState = ST_ORIGINAL;
	g_TalkInfo.Timer.dwUnlockTimer = GetTickCount();
	g_TalkInfo.unlock.dwUnLockShowState = ST_ORIGINAL;
	g_TalkInfo.Timer.dwUnlockShowTimer = GetTickCount();
	
				

	DpcAddMd(MXMDID_TALKING, NULL);
	ReadConfig();
	GetTalkDeviceCode(); //Could get the Value from AMT	

#ifdef TALK_DEBUG
	printf("Talk Init...\n");
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadConfig()
**	AUTHOR:			Harry Qian
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			After initial, read configuration file to set the Flags needed.
**			but now there is no talk module configuration file
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
ReadConfig()
{
	g_TalkInfo.bAutoPick			= FALSE;
	g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
	g_TalkInfo.monitor.dwMonState	= ST_ORIGINAL;
	
	g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
	g_TalkInfo.Timer.dwMonitorTimer = GetTickCount();
	g_TalkInfo.Timer.dwUnlockTimer	= GetTickCount();
	
	g_TalkInfo.bMCStatus			= FALSE;

	g_TakePhoto.nStatus				= ST_ORIGINAL;

	g_TalkInfo.bSCR					= FALSE;
//	g_TalkInfo.nHVid				= GetSelfHVId();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PriorCtrl()
**	AUTHOR:			Harry Qian
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			control the prior of the calling and the monitor, 
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				1 means continue process the message, 0 do not stop the message 
**	NOTES:
**			
*/
BOOL
PriorCtrl(MXMSG * pmsg)
{
	BOOL bRet = TRUE;	
	EthResolvInfo 	ResolvInfo;
	
	memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
 
	if (g_TalkInfo.talking.dwTalkState)
	{
		switch(pmsg->dwMsg)
		{
		case FC_MNT_START:
			{
				printf("MNT when Talking\n");
				bRet		=	FALSE;
			}
			break;
			
		default:
			break;
		}
	}
	else if (g_TalkInfo.monitor.dwMonState)
	{
		switch(pmsg->dwMsg)
		{
		case FC_CALL_HV_GM:
		case FC_CALL_MC_GM:
		case FC_CALL_GM_GM:
			{
				if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
				{
					return;
				}
				else
				{
					if ('0' == pmsg->szDestDev[0] 
						&& '0' == pmsg->szDestDev[1]
						&& 2 == strlen(pmsg->szDestDev))
					{
						bRet = FALSE;
					}
					else if (MXMSG_USER_GM_PRJCALLOUT == pmsg->dwMsg
						|| FC_CALL_HV_GM == pmsg->dwMsg
						|| FC_CALL_MC_GM == pmsg->dwMsg
						|| FC_CALL_GM_GM == pmsg->dwMsg) 
					{
						bRet = FALSE;
					}
					else if (MXMSG_USER_GM_CODECALLOUT == pmsg->dwMsg)
					{
						ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_CODE;
						strcpy(ResolvInfo.szDevCode, pmsg->szDestDev);
						FdFromAMTResolv(&ResolvInfo);
						if (0 == ResolvInfo.nType) 
						{
							bRet  = TRUE;
							return bRet;
						}
					}				
					StopGMMonState();	
					
					memcpy(&g_MxMsgBk, pmsg, sizeof(MXMSG));
					if(pmsg->wDataLen)
					{
						g_MxMsgBk.pParam=(unsigned char*)malloc(pmsg->wDataLen);
						memcpy(g_MxMsgBk.pParam,pmsg->pParam,pmsg->wDataLen);
					}
					
					g_TalkInfo.monitor.dwMonState =	ST_MT_INT_ACK;
					g_TalkInfo.Timer.dwMonitorTimer	= GetTickCount();
					bRet							= FALSE;
				}
				break;
			}
		case MXMSG_USER_GM_CODECALLOUT:
		case MXMSG_USER_GM_PRJCALLOUT:
			{
				if ('0' == pmsg->szDestDev[0] 
					&& '0' == pmsg->szDestDev[1]
					&& 2 == strlen(pmsg->szDestDev))
				{
					bRet = FALSE;
				}
				else if (MXMSG_USER_GM_PRJCALLOUT == pmsg->dwMsg
						|| FC_CALL_HV_GM == pmsg->dwMsg
						|| FC_CALL_MC_GM == pmsg->dwMsg
						|| FC_CALL_GM_GM == pmsg->dwMsg) 
				{
					bRet = FALSE;
				}
				else if (MXMSG_USER_GM_CODECALLOUT == pmsg->dwMsg)
				{
					ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_CODE;
					strcpy(ResolvInfo.szDevCode, pmsg->szDestDev);
					FdFromAMTResolv(&ResolvInfo);
					if (0 == ResolvInfo.nType) 
					{
						bRet  = TRUE;
						return bRet;
					}
				}				

				StopGMMonState();	

				memcpy(&g_MxMsgBk, pmsg, sizeof(MXMSG));

				g_TalkInfo.monitor.dwMonState =	ST_MT_INT_ACK;
				g_TalkInfo.Timer.dwMonitorTimer	= GetTickCount();
				bRet							= FALSE;
				break;
			}

		case FC_ACK_MNT_INTERRUPT:
			{
				printf("FC_ACK_MNT_INTERRUPT\n");
				if (MON_TYPE_EHV == g_TalkInfo.monitor.bType) 
				{
#ifdef TALK_LOG_DEBUG					
					printf("RecordLogData line=%d\n",__LINE__);
#endif
					RecordLogData(TYPE_MONITOR, STEP_END, &g_TalkInfo);
				}
				
				g_TalkInfo.monitor.dwMonState =	ST_ORIGINAL;
				g_TalkInfo.monitor.bType = MOM_TYPE_NODEVICE;
				MTStopMonitorVideo();
				
				DisableUnlock();

				
				MxPutMsg(&g_MxMsgBk);
				memset(&g_MxMsgBk, 0, sizeof(MXMSG));
				bRet		=	FALSE;
				break;
			}
		case FC_MNT_START:
			{
				bRet		=	FALSE;
			}
			break;
		case FC_MNT_CANCEL:
			{
				if(g_TalkInfo.monitor.dwDestIP == pmsg->dwParam)
				{
					bRet	=	TRUE;
				}
				else
				{
					bRet       =    FALSE; 
				}
			}

		default:
			break;
		}
	}

	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkProcess()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			the main process of the Talk Module. get the message from dispatch.
**			then action according to the message and the state .
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
VOID
TalkProcess()
{
	MXMSG	msgRecev;

	memset(&msgRecev, 0, sizeof(msgRecev));
	msgRecev.dwDestMd	= MXMDID_TALKING;
	msgRecev.pParam		= NULL;
/*
	if (g_TalkInfo.talking.bNeedPhotoSaved && !g_TalkInfo.talking.bHavePhotoSaved) 
	{
		if (bYUV2JPG) 
		{
			printf("%s ###########this is not good place to set bHavePhotoSaved to true\n",__FUNCTION__);
			g_TalkInfo.talking.bHavePhotoSaved	=	TRUE;
			bYUV2JPG = FALSE;
		}
			
	}
*/		

	TimeOutCtrl();
	if (MxGetMsg(&msgRecev))
	{
		if (IsAlarming())
		{
			if (ST_ORIGINAL == g_TalkInfo.talking.dwTalkState && ST_ORIGINAL == g_TalkInfo.monitor.dwMonState )
			{
				TlLogProc(&msgRecev);
				DoClearResource(&msgRecev);
				return;//when alarming, donot reponse the calling request.
			}
		}
		printf("TALK: Get Message: %x\tTC: %x\tTM: %x\tTick %d\n", 
			                                                            (UINT)msgRecev.dwMsg, 
			                                                            (UINT)g_TalkInfo.talking.dwTalkState, 
			                                                            (UINT)g_TalkInfo.monitor.dwMonState, 
			                                                            (UINT)GetTickCount());
		if (!TlLogProc(&msgRecev))
		{
			DoMaster(&msgRecev);
		}

		DoClearResource(&msgRecev);
	}

	if (IsAlarming()) 
	{
		if (ST_ORIGINAL != g_TalkInfo.talking.dwTalkState) 
		{
			TalkSendGMHangupCmd();
			GMStopTalking();
			WndHideTalkWindow();
			
			g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
			g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
		}
		else if (ST_ORIGINAL != g_TalkInfo.monitor.dwMonState) 
		{
			StopGMMonState();
			
			g_TalkInfo.monitor.dwMonState =	ST_ORIGINAL;
			g_TalkInfo.Timer.dwMonitorTimer	= GetTickCount();
		}
	}
	if (nCurTick !=0 && GetTickCount() -  nCurTick > 700) // 200ms
	{
		printf("Jason--->12222\n");
		MxPutMsg(&HVMsg);		
	}	
	TlReport();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetTalkDeviceCode()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**				if have no self devicecode, then get it .
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				the length of the device code of the Ehv.
**	NOTES:
**			
*/
UINT
GetTalkDeviceCode(void)
{
	EthResolvInfo iGMIPinfo;

	memset(&iGMIPinfo, 0, sizeof(EthResolvInfo));
	g_TalkInfo.dwSelfGMIP	= GetSelfIP();
	iGMIPinfo.nQueryMethod	= HO_QUERY_METHOD_IP;
	iGMIPinfo.nIP = ChangeIPFormat(g_TalkInfo.dwSelfGMIP);
	FdFromAMTResolv(&iGMIPinfo);

	strcpy(g_TalkInfo.szTalkDevCode, iGMIPinfo.szDevCode);

#ifdef DNS_DEBUG
	printf("####  Get talk deviced code: %s  ...\n", g_TalkInfo.szTalkDevCode);
#endif	
	
	return strlen(iGMIPinfo.szDevCode);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TimeOutCtrl()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Timeout control
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TimeOutCtrl(void)
{
	TLTimeOutCtrl();
	TalkTimeOutCtrl();
	MTTimeOutCtrl();
	UKTimeOutCtrl();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkTimeOutCtrl()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			talking timeout control
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TalkTimeOutCtrl(void)
{
	static DWORD dwIntervalTalkTimer = IRIS_TALKING_COUNT_TIME;

	g_TalkInfo.Timer.dwCurrentTick = GetTickCount();

	if ((ST_CI_TALKING != g_TalkInfo.talking.dwTalkState) 
		&& (ST_CO_TALKING != g_TalkInfo.talking.dwTalkState))
	{
		dwIntervalTalkTimer = IRIS_TALKING_COUNT_TIME;
	}
	
//	if ((ST_CI_WAIT_PICKUP != g_TalkInfo.talking.dwTalkState) 
//		&& (ST_CO_WAIT_PICKUP != g_TalkInfo.talking.dwTalkState))
//	{
//		bPhotoSaved		=	TRUE; 
//	}

/**************************call in timeout control  ********************************/	

	if (ST_CI_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState && 
		(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > (g_SysConfig.RingTime * 1000)))
	{
			GMStopCallRing();
			GMStopCallingVideo();
			
			g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
			g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();				
			
			WndHideTalkWindow();			

#ifdef TALK_DEBUG
			printf(" ST_CI_WAIT_PICKUP status timeout.\n");
#endif

	}

	if (ST_CI_WAIT_PICKUP_ACK == g_TalkInfo.talking.dwTalkState &&
		(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > IRIS_ACK_TIMEOUT))
	{
		GMStopCallRing();
		GMStopCallingVideo();
		
/*
		if (MM_TYPE_TALK_MC ==  g_TalkInfo.talking.bMMType) 
		{
			GMStopCallingVideo();
		}
		else if (MM_TYPE_TALK_GM ==  g_TalkInfo.talking.bMMType) 
		{
			//GMStopCallingTypeGM();
		}
*/
	
		g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();

#ifdef TALK_DEBUG
		printf("wait pick up ack status timeoput..\n");
#endif
		WndHideTalkWindow();
	}
	if (ST_CI_TALKING == g_TalkInfo.talking.dwTalkState && (g_TalkInfo.bAutoPick == TRUE) &&
		(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > 20000)) 
	{
		if (MM_TYPE_TALK_GM		== g_TalkInfo.talking.bMMType)
		{
			GMStopTalking();
			TalkSendGMhangupToNet();
			g_TalkInfo.talking.dwTalkState	= ST_CI_WAIT_HANGUP_ACK;
			g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
			
			ShowTalkWnd();
		}
		else 
		{
			GMStopTalking();
			TalkSendGMhangupToNet();
			g_TalkInfo.talking.dwTalkState	= ST_CI_WAIT_HANGUP_ACK;
			g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
			
			ShowTalkWnd();
		}
	}
	
	if (ST_CI_TALKING == g_TalkInfo.talking.dwTalkState &&
		(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > (g_SysConfig.TalkTime * 1000))) 
	{
		GMStopTalking();
		TalkSendGMhangupToNet();
		g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();

		ShowTalkWnd();
	}

	if (ST_CI_TALKING	==  g_TalkInfo.talking.dwTalkState)
	{			
		if ((g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer) >= dwIntervalTalkTimer)
		{
			ShowTalkWnd();
			dwIntervalTalkTimer += IRIS_TALKING_COUNT_TIME;
		}		
	}
	
	g_TalkInfo.Timer.dwCurrentTick = GetTickCount();
	if (ST_CI_WAIT_HANGUP_ACK ==  g_TalkInfo.talking.dwTalkState &&
		(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > IRIS_ACK_TIMEOUT))
	{
	
//		g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
		g_TalkInfo.talking.dwTalkState = ST_TALK_END;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
		
		WndHideTalkWindow();
	}

//	if (ST_CI_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState && bPhotoSaved &&
//		((g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer)/1000 == IRIS_TAKE_PHOTO_TIME)) 
//	{
//		bPhotoSaved = FALSE;
//		TalkSavePhotoNote2MM();
//	}

/************************** call out timeout control  ********************************/
	
	if (ST_CO_WAIT_CALLOUT_ACK ==  g_TalkInfo.talking.dwTalkState &&
		(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > IRIS_ACK_TIMEOUT))
	{
//		TalkSendGMhangupToNet();

		TalkErrorSoundNote2MM();

		g_TalkInfo.talking.dwTalkState	= ST_CO_CALL_FAIL;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
		ShowTalkWnd();
#ifdef TALK_LOG_DEBUG		
		printf("RecordLogData line=%d\n",__LINE__);
#endif
		RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
	}
	
	g_TalkInfo.Timer.dwCurrentTick = GetTickCount();
	if (ST_CO_CALL_FAIL ==  g_TalkInfo.talking.dwTalkState &&
		(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > IRIS_SHOW_INFO))
	{
		g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
		WndHideTalkWindow();
	}
	if (ST_CO_WAIT_PICKUP ==  g_TalkInfo.talking.dwTalkState &&
		(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > (g_SysConfig.RingTime * 1000)))
	{
		GMStopCallRing();

		GMStopCallingVideo();
		
		g_TalkInfo.talking.dwTalkState	= ST_CO_NO_ANSWER;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();

		ShowTalkWnd();
#ifdef TALK_LOG_DEBUG		
		printf("RecordLogData line=%d\n",__LINE__);
		printf(" func = %s,line=%d;g_SysConfig.RingTime = %d\n",__func__,__LINE__,g_SysConfig.RingTime);		
		printf(" func = %s,line=%d;g_SysConfig.RingTime = %d\n",__func__,__LINE__,g_NewSysConfig.RingTime);
#endif
		RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
	}
#ifdef __SUPPORT_PROTOCOL_900_1A__	//for bug 14857
		if(MOX_PROTOCOL_SUPPORT(MOX_P_900_1A))
		{
			g_SysConfig.TalkTime = g_NewSysConfig.TalkTime;
		}
#endif
	if (ST_CO_TALKING	==  g_TalkInfo.talking.dwTalkState &&
		(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > (g_SysConfig.TalkTime * 1000)))
	{
		if (MM_TYPE_TALK_MC == g_TalkInfo.talking.bMMType)
		{
			GMStopTalking();
		}
		else
		{
			GMStopTalking();
		}
		TalkSendGMhangupToNet();
		
		g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();

		ShowTalkWnd();
#ifdef TALK_LOG_DEBUG		
		printf("RecordLogData line=%d\n",__LINE__);
#endif
		RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
	}

	if (ST_CO_TALKING	==  g_TalkInfo.talking.dwTalkState)
	{			
		if ((g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer) >= dwIntervalTalkTimer)
		{
			ShowTalkWnd();
			dwIntervalTalkTimer += IRIS_TALKING_COUNT_TIME;
		}		
	}
	
	g_TalkInfo.Timer.dwCurrentTick = GetTickCount();
	if (ST_CO_WAIT_HANGUP_ACK ==  g_TalkInfo.talking.dwTalkState &&
		(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > IRIS_ACK_TIMEOUT))
	{
		TalkErrorSoundNote2MM();
		
//		GMStopCallRing();
		TalkSendGMhangupToNet();
//
//		GMStopTalking();

//		g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
		g_TalkInfo.talking.dwTalkState = ST_TALK_END;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();

		WndHideTalkWindow();
#ifdef TALK_LOG_DEBUG		
		printf("RecordLogData line=%d\n",__LINE__);
#endif
		RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
	}	

	if ((ST_CO_CALL_CODE_INEXIST ==  g_TalkInfo.talking.dwTalkState)
		&&(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > IRIS_SHOW_INFO2))
	{
		g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();	
		
		WndHideTalkWindow();
	}
	
	if ((ST_CO_NO_ANSWER ==  g_TalkInfo.talking.dwTalkState)		
		&&(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > IRIS_SHOW_INFO))
	{
		
		if (MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType)
		{
			g_TalkInfo.talking.dwTalkState	= ST_TALK_LEAVEPHOTO;
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

	if ((ST_TALK_LEAVEPHOTO ==  g_TalkInfo.talking.dwTalkState)		
		&&(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > IRIS_LEAVE_PHOTO_TIMEOUT))
	{
		g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
		WndHideTalkWindow();
	}
	
	if ((ST_TALK_LEAVEPHOTO_END ==  g_TalkInfo.talking.dwTalkState)		
		&&(g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > IRIS_LEAVE_PHOTO_END_DELAY))
	{
		g_TalkInfo.talking.dwTalkState	= ST_ORIGINAL;
		g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
		WndHideTalkWindow();
	}
	
	if ((ST_TALK_END ==  g_TalkInfo.talking.dwTalkState)		
		&&((g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer > IRIS_SHOW_INFO)||
			(DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType && !GetLeavePhotoStatus())))
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

//	if (ST_CO_WAIT_PICKUP == g_TalkInfo.talking.dwTalkState && bPhotoSaved &&
//		((g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer)/1000 == IRIS_TAKE_PHOTO_TIME)) 
//	{
//		bPhotoSaved = FALSE;
//		TalkSavePhotoNote2MM();
//	}

				
	if (bPhotoSaved &&
		((g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwPhotoSavedTimer)/1000 >= IRIS_TAKE_PHOTO_TIME)) 
	{
		if(GetLeavePhotoStatus())
		{
			bPhotoSaved = FALSE;
			TalkSavePhotoNote2MM();
		}
	}

	if (ST_TALK_WAIT_LEAVEPHOTO	==	g_TalkInfo.talking.dwTalkState) 
	{
		if ((g_TalkInfo.talking.bNeedPhotoSaved && g_TalkInfo.talking.bHavePhotoSaved) || 
			((g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwTalkTimer) >= IRIS_LEAVE_PHOTO_TIMEOUT)) 
		{
#ifdef TALK_LOG_DEBUG			
			printf("GMStopTalking 1111\n");
#endif
			GMStopTalking();
			GMStopCallingVideo();			
			g_TalkInfo.talking.bNeedPhotoSaved	=	FALSE;
			g_TalkInfo.talking.bHavePhotoSaved	=	FALSE;
			g_TalkInfo.talking.dwTalkState		=	ST_ORIGINAL;
			g_TalkInfo.Timer.dwTalkTimer		=	GetTickCount();		
			WndHideTalkWindow();
		}
	}
	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MTTimeOutCtrl()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			The timeout control when HV monitoring the GM.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
MTTimeOutCtrl(void)
{
	g_TalkInfo.Timer.dwCurrentTick = GetTickCount();

	if (ST_MT_MONITOR == g_TalkInfo.monitor.dwMonState && 
		g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwMonitorTimer > setMonitoringTime())
	{

		if ( (g_TalkInfo.monitor.bType == MON_TYPE_MC)
			|| (g_TalkInfo.monitor.bType == MON_TYPE_EHV) )
		{
			MTStopMonitorVideo();

			DisableUnlock();
//			HideTalkUlkWnd();			
			if (MON_TYPE_EHV == g_TalkInfo.monitor.bType) 
			{
#ifdef TALK_LOG_DEBUG				
				printf("RecordLogData line=%d\n",__LINE__);
#endif
				RecordLogData(TYPE_MONITOR, STEP_END, &g_TalkInfo);
			}
			g_TalkInfo.monitor.dwMonState	= ST_ORIGINAL;
			g_TalkInfo.Timer.dwMonitorTimer = GetTickCount();
			g_TalkInfo.monitor.bType = MOM_TYPE_NODEVICE;
		}		
	}
	if (ST_MT_INT_ACK == g_TalkInfo.monitor.dwMonState && 
		g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwMonitorTimer > IRIS_ACK_TIMEOUT)
	{
		if ( (g_TalkInfo.monitor.bType == MON_TYPE_MC)
			|| (g_TalkInfo.monitor.bType == MON_TYPE_EHV) )
		{
			MTStopMonitorVideo();

			DisableUnlock();
			HideTalkUlkWnd();			
			
			MxPutMsg(&g_MxMsgBk);
			memset(&g_MxMsgBk, 0, sizeof(MXMSG));
			
			if (MON_TYPE_EHV == g_TalkInfo.monitor.bType) 
			{
#ifdef TALK_LOG_DEBUG				
				printf("RecordLogData line=%d\n",__LINE__);
#endif
				RecordLogData(TYPE_MONITOR, STEP_END, &g_TalkInfo);
			}
			g_TalkInfo.monitor.dwMonState	= ST_ORIGINAL;
			g_TalkInfo.Timer.dwMonitorTimer = GetTickCount();
			g_TalkInfo.monitor.bType = MOM_TYPE_NODEVICE;
		}		
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UKTimeOutCtrl()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			The timeout control when HV unlock GM
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
UKTimeOutCtrl(void)
{
 	g_TalkInfo.Timer.dwCurrentTick = GetTickCount();

	if ((ST_UK_DELAY == g_TalkInfo.unlock.dwUnLockShowState || ST_UK_DELAY == g_TalkInfo.unlock.dwUnLockState)&&
		g_TalkInfo.Timer.dwCurrentTick - g_TalkInfo.Timer.dwUnlockShowTimer > 2000)
	{
		g_TalkInfo.unlock.dwUnLockShowState = ST_ORIGINAL;
        g_TalkInfo.unlock.dwUnLockState = ST_ORIGINAL;
		g_TalkInfo.Timer.dwUnlockShowTimer = GetTickCount();
		
		HideTalkUlkWnd();
		HideUlkWnd();
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DoMaster()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			the Master HV, It first do the prior conflict, then process the talk 
**			control, after that, process the talking.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
DoMaster(MXMSG *pmsg)
{
	if (!PriorCtrl(pmsg))
	{
		return;
	}

	if (!DoGMTalkingIn(pmsg))
	{
		if (!DoGMTalkingOut(pmsg))
		{
			if (!DoGMMonitor(pmsg))
			{
				if (!DoGMUnlock(pmsg))
				{
					DoFault(pmsg);				
				}
			}
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DoFault()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			process the message that Talk module default process
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
DoFault(MXMSG *pmsg)
{
	;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DoClearResource()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
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
void
DoClearResource(MXMSG *pmsg)
{
	if (NULL != pmsg->pParam)
	{

		free(pmsg->pParam);
		pmsg->pParam = NULL;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DoGMTalkingIn()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Talk control process when other device calls in.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
DoGMTalkingIn(MXMSG *pmsg)
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return TRUE;
	}

	if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
	{
		return FALSE;
	}

	switch(g_TalkInfo.talking.dwTalkState) 
	{
		case ST_ORIGINAL:
		{
			bRet = GMTalkInOriginalPro(pmsg);

			break;
		}
		case ST_CI_WAIT_PICKUP:
		{
			bRet = GMTalkInWaitGMPickupPro(pmsg);

			break;
		}

		case ST_CI_WAIT_PICKUP_ACK:
		{
			bRet = TalkInWaitforPickupAck(pmsg);

			break;
		}
		case ST_CI_TALKING:
		{
			bRet = TalkInTalkingPro(pmsg);

			break;
		}
		case ST_CI_WAIT_HANGUP_ACK:
		{
			bRet = WaitForHangupAckPro(pmsg);

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
**	FUNCTION NAME:	DoGMTalkingOut()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Talk control process when HV call out.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, hasnot processed.
**	NOTES:
**			
*/
BOOL
DoGMTalkingOut(MXMSG *pmsg)
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return TRUE;
	}

	switch(g_TalkInfo.talking.dwTalkState) 
	{
		case ST_ORIGINAL:
		{
			bRet = TalkOutOriginalPro(pmsg);

			break;
		}
		case ST_CO_WAIT_CALLOUT_ACK:
		{
			bRet = WaitCalloutAckPro(pmsg);
			break;
		}
		case ST_CO_WAIT_PICKUP:
		{
			bRet = TalkOutWaitForPickupPro(pmsg);

			break;
		}
		case ST_CO_TALKING:
		{
			bRet = TalkOutTalkingPro(pmsg);

			break;
		}
		case ST_CO_WAIT_HANGUP_ACK:
		{
			bRet = WaitForHangupAckPro(pmsg);

			break;
		}
		case ST_TALK_LEAVEPHOTO:
		{
			bRet = LeavePhotoPro(pmsg);
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
**	FUNCTION NAME:	DoGMMonitor()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Talk control process when Master HV monitor GM.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, hasnot processed.
**	NOTES:
**			
*/
BOOL
DoGMMonitor(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	if (NULL == pmsg)
	{
		return TRUE;
	}

	switch(g_TalkInfo.monitor.dwMonState) 
	{
		case ST_ORIGINAL:
		{
			bRet = MTOriginalPro(pmsg);

			break;
		}
		case ST_MT_MONITOR:
		{
			bRet = MTMonitorPro(pmsg);
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
**	FUNCTION NAME:	DoGMUnlock()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Talk control process when MHV unlock GM.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
DoGMUnlock(MXMSG *pmsg)
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return TRUE;
	}
	
	if ((ST_ORIGINAL != g_TalkInfo.talking.dwTalkState && pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
		||(ST_MT_MONITOR == g_TalkInfo.monitor.dwMonState && pmsg->dwParam == g_TalkInfo.monitor.dwDestIP)
		|| IsSlaveHV(pmsg->dwParam))
	{
		if (HVCanUnLock()) 
		{
			bRet = UKOriginalPro(pmsg);
		}
	}
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GMTalkInOriginalPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Talk control process in original state when other device
**			calls in, 
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
GMTalkInOriginalPro(MXMSG * pmsg)
{
	BOOL bRet = FALSE;
//	char	szCode[20] = {0};
	EthResolvInfo 	ResolvInfo;

	memset(&ResolvInfo, 0 , sizeof(ResolvInfo));

	if (NULL == pmsg)
	{
		return bRet;
	}

	switch(pmsg->dwMsg)
	{
		case FC_CALL_GM_GM:
		{
			if (g_hWNDTalk) 
			{
				return;
			}
			strcpy(g_TalkInfo.szTalkDevCode, pmsg->szDestDev);
			strcpy(g_TalkInfo.talking.szTalkDestDevCode, pmsg->szSrcDev);
			
			g_TalkInfo.talking.dwDestIP = pmsg->dwParam;
			g_TalkInfo.talking.bMMType	= MM_TYPE_TALK_GM;
			GetMMFormat(pmsg);
			TalkSendGMcallGMAck(pmsg);
		
			GMStartCallRing();
			g_TalkInfo.talking.dwTalkState = ST_CI_WAIT_PICKUP;
			g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
			ShowTalkWnd();

			bRet = TRUE;
			break;
		}

		case FC_CALL_MC_GM:
		{
			if (g_hWNDTalk) 
			{
				return;
			}
			strcpy(g_TalkInfo.szTalkDevCode, pmsg->szDestDev);
			strcpy(g_TalkInfo.talking.szTalkDestDevCode, pmsg->szSrcDev);
			
			g_TalkInfo.talking.dwDestIP = pmsg->dwParam;
			g_TalkInfo.talking.bMMType	= MM_TYPE_TALK_MC;
			
#ifdef DEBUG_CALL_MCGM
			printf("DNS_1: MC name:%s, code:%s.ip:%x\n", g_TalkInfo.talking.szName, g_TalkInfo.talking.szTalkDestDevCode, (UINT)g_TalkInfo.talking.dwDestIP);
#endif 			
			GetMMFormat(pmsg);
			TalkSendMCcallGMAck(pmsg);
			GMStartCallRing();
			g_TalkInfo.talking.dwTalkState	= ST_CI_WAIT_PICKUP;
			GMStartCallingVideo();
			g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
            ShowTalkWnd();
			
			EnableUnlock();
				
			bRet = TRUE;
			break;
		}
		
		case FC_CALL_HV_GM:
			{
				if (g_hWNDTalk) 
				{
					return;
				}
				strcpy(g_TalkInfo.szTalkDevCode, pmsg->szDestDev);
				strcpy(g_TalkInfo.talking.szTalkDestDevCode, pmsg->szSrcDev);
				
				g_TalkInfo.talking.dwDestIP = pmsg->dwParam;
				g_TalkInfo.talking.bMMType	= MM_TYPE_TALK_EHV;
				
				GetMMFormat(pmsg);
				TalkSendHVcallGMAck(pmsg);			
				
				GMStartCallRing();
				g_TalkInfo.talking.dwTalkState	= ST_CI_WAIT_PICKUP;
				GMStartCallingVideo();
				
				g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();

				ShowTalkWnd();				

				EnableUnlock();

//				bPhotoSaved							=	TRUE; 
//				g_TalkInfo.Timer.dwPhotoSavedTimer	=	GetTickCount();
//				g_TalkInfo.talking.bNeedPhotoSaved	=	TRUE;
//				g_TalkInfo.talking.bHavePhotoSaved	=	FALSE;

				
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
**	FUNCTION NAME:	TalkOutOriginalPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Talk control process in original state when HV calls out
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
TalkOutOriginalPro(MXMSG * pmsg)
{
	BOOL bRet = FALSE;
	EthResolvInfo 	ResolvInfo;

	memset(&ResolvInfo, 0, sizeof(EthResolvInfo));

	if (NULL == pmsg)
	{
		return TRUE;
	}

	switch(pmsg->dwMsg)
	{
		case MXMSG_USER_GM_CODECALLOUT:
		{
			if (strlen(pmsg->szDestDev))
			{
#ifdef __SUPPORT_PROTOCOL_900_1A__	//bug 14748
			if(MOX_PROTOCOL_SUPPORT(MOX_P_900_1A))
			{
				strcpy(g_TalkInfo.talking.szTalkDestDevCode, pmsg->szDestDev);//save the calling device code.					
				g_TalkInfo.talking.bMMType = MM_TYPE_TALK_EHV;
				RecordLogData(TYPE_CALLOUT, STEP_START, &g_TalkInfo);
				g_TalkInfo.talking.dwDestIP = g_NewSysConfig.ServerIP;
			}
			else
#endif	
			if (('0' == pmsg->szDestDev[0]) 
					&& ('0' == pmsg->szDestDev[1])
					&&(2 == strlen(pmsg->szDestDev)))
				{
					strcpy(ResolvInfo.szDevCode, pmsg->szDestDev);
					strcpy(g_TalkInfo.talking.szTalkDestDevCode, pmsg->szDestDev);//save the calling device code.					

					ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_CODE;
					FdFromAMTResolv(&ResolvInfo);

//					printf("ResolvInfo.nType = %d\n",ResolvInfo.nType);
					

					if (ATM_TYPE_MC == ResolvInfo.nType)
					{
						g_TalkInfo.talking.dwDestIP = ChangeIPFormat(ResolvInfo.nIP);
//						printf("g_TalkInfo.talking.dwDestIP = %x\n",g_TalkInfo.talking.dwDestIP);
						g_TalkInfo.talking.bMMType = MM_TYPE_TALK_MC;
						strcpy(g_TalkInfo.talking.szTalkDestDevCode, pmsg->szDestDev);//save the calling device code.
						memcpy(&g_MxMsgBk, pmsg, sizeof(MXMSG));
					}
//					else if (IsMCOnline()) 
					else 
					{
						g_TalkInfo.talking.bMMType = MM_TYPE_TALK_MC;
						strcpy(g_TalkInfo.talking.szTalkDestDevCode, pmsg->szDestDev);//save the calling device code.
//						g_TalkInfo.talking.dwDestIP = g_TalkInfo.dwMCIP;
						g_TalkInfo.talking.dwDestIP = g_SysConfig.MCIPCLIENT;
						memcpy(&g_MxMsgBk, pmsg, sizeof(MXMSG));
					}
				}
				else
				{
					strcpy(ResolvInfo.szDevCode, pmsg->szDestDev);

					ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_CODE;
					FdFromAMTResolv(&ResolvInfo);

					strcpy(g_TalkInfo.talking.szTalkDestDevCode, ResolvInfo.szDevCode);//save the calling device code.					
                        
					if (ATM_TYPE_MC == ResolvInfo.nType)
					{
						g_TalkInfo.talking.bMMType = MM_TYPE_TALK_MC;
						memcpy(&g_MxMsgBk, pmsg, sizeof(MXMSG));
					}					
					else if (ATM_TYPE_EHV == ResolvInfo.nType || ATM_TYPE_HV == ResolvInfo.nType)
					{
						g_TalkInfo.talking.bMMType = MM_TYPE_TALK_EHV;
						RecordLogData(TYPE_CALLOUT, STEP_START, &g_TalkInfo);
					}
					else if (ATM_TYPE_EGM == ResolvInfo.nType)
					{
						g_TalkInfo.talking.bMMType = MM_TYPE_TALK_GM;
						RecordLogData(TYPE_CALLOUT, STEP_START, &g_TalkInfo);
					}
					else 
					{
						g_TalkInfo.talking.bMMType = MM_TYPE_NORMAL;
						g_TalkInfo.talking.dwTalkState = ST_CO_CALL_CODE_INEXIST;
						g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
						ShowTalkWnd();
						TalkErrorSoundNote2MM();
						return TRUE;
					}
					g_TalkInfo.talking.dwDestIP = ChangeIPFormat(ResolvInfo.nIP);
				}

				TalkSendGMcallOutDevice();
				
				g_TalkInfo.talking.dwTalkState = ST_CO_WAIT_CALLOUT_ACK;
				g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
				ShowTalkWnd();
			}
			else
			{
				g_TalkInfo.talking.bMMType = MM_TYPE_NORMAL;
				g_TalkInfo.talking.dwTalkState = ST_CO_CALL_CODE_INEXIST;
				g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
				ShowTalkWnd();
				TalkErrorSoundNote2MM();
				return TRUE;
			}
			break;
		}
		case MXMSG_USER_GM_PRJCALLOUT:
		{
			if (strlen(pmsg->szDestDev))
			{
				UCHAR IPAddr3, IPAddr4 = 0;
				strcpy(g_TalkInfo.talking.szTalkDestDevCode, pmsg->szDestDev);//save the calling device code.

				if (0 == strcmp(&pmsg->szDestDev[6], "091000"))
				{
					g_TalkInfo.talking.bMMType = MM_TYPE_TALK_GM;
					RecordLogData(TYPE_CALLOUT, STEP_START, &g_TalkInfo);
				}
				else
				{
					g_TalkInfo.talking.bMMType = MM_TYPE_TALK_EHV;
					RecordLogData(TYPE_CALLOUT, STEP_START, &g_TalkInfo);
				}
				
				IPAddr3 = 100*(pmsg->szDestDev[0]-0x30) + 10*(pmsg->szDestDev[1]-0x30) + (pmsg->szDestDev[2]-0x30);
				IPAddr4 = 100*(pmsg->szDestDev[3]-0x30) + 10*(pmsg->szDestDev[4]-0x30) + (pmsg->szDestDev[5]-0x30);	
				g_TalkInfo.talking.dwDestIP = (DWORD)((0xAC<<24) | (0x10<<16) | (IPAddr3<<8) | IPAddr4);
			}	
			
			strcpy( g_TalkInfo.talking.szTalkDestDevCode, "");			
			
			TalkSendGMcallOutDevice();
			
			g_TalkInfo.talking.dwTalkState = ST_CO_WAIT_CALLOUT_ACK;
			g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
			ShowTalkWnd();
			bRet	=	TRUE;
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
**	FUNCTION NAME:	MTOriginalPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Talk control process in original state when HV monitors GM
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
MTOriginalPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
//	EthResolvInfo 	ResolvInfo;
	
	if (NULL == pmsg)
	{
		return TRUE;
	}

	if (FC_MNT_START == pmsg->dwMsg)
	{
		g_TalkInfo.monitor.dwDestIP = pmsg->dwParam,
		strcpy(g_TalkInfo.monitor.szMonSrcDevCode, pmsg->szSrcDev);
		g_TalkInfo.monitor.dwMonState = ST_MT_MONITOR;
		printf("g_TalkInfo.monitor.dwDestIP = %x\n",g_TalkInfo.monitor.dwDestIP);
		printf("g_TalkInfo.dwMCIP = %x\n",g_TalkInfo.dwMCIP);
//		if (g_TalkInfo.monitor.dwDestIP == g_TalkInfo.dwMCIP )
		if (g_TalkInfo.monitor.dwDestIP == g_SysConfig.MCIPCLIENT )
		{
			g_TalkInfo.monitor.bType = MON_TYPE_MC;
		}
		else
		{
			g_TalkInfo.monitor.bType = MON_TYPE_EHV;
		}
		printf(" [%s__%d ] pmsg->wDataLen = %d\n",__func__,__LINE__,pmsg->wDataLen);

//设置监视时间，如果没有设置监视时间则返回默认值
		if(pmsg->wDataLen <=0 )//老门口机版本
		{
			g_MonitorTime = 0;
		}else if((pmsg->wDataLen-1)%4 !=0)
		{	
			//int MonitorTime;
			memcpy(&g_MonitorTime,pmsg->pParam + pmsg->wDataLen - 2 ,2);
			g_MonitorTime = g_MonitorTime * 1000;
			printf(" [%s__%d ] Monitor  Time  = %d\n",__func__,__LINE__,g_MonitorTime);	
		}else
		{
			g_MonitorTime = 0;
		}

		
		GetMMFormat(pmsg);
		MTSendMonStartAckCMD(pmsg);
		MTStartMonitorVideo();

		
		g_TalkInfo.Timer.dwMonitorTimer = GetTickCount();
		RecordLogData(TYPE_MONITOR, STEP_START, &g_TalkInfo);

		EnableUnlock();

		
	}

	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UKOriginalPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Talk control process in original state when HV unlock GM
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
UKOriginalPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	BOOL bUnlock = FALSE;    
    EthResolvInfo ResolvInfo;
	
    memset(&ResolvInfo, 0, sizeof(EthResolvInfo));

	if (NULL == pmsg)
	{
		return TRUE;
	}
	
	switch(pmsg->dwMsg)
	{
		case FC_UNLK_GATE:
		{
			MakeUnLockIP();
            
			if (pmsg->dwParam != g_TalkInfo.unlock.dwDestIP) 
			{
                //通过目标设备码(配置于AMT表中)，IP查询方式，查询的IP，找到这个设备
				strcpy(ResolvInfo.szDevCode,g_TalkInfo.talking.szTalkDestDevCode);
                ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_IP;
                ResolvInfo.nIP = ChangeIPFormat(pmsg->dwParam);
                FdFromAMTResolv(&ResolvInfo);
                if(ATM_TYPE_NODEVICE == ResolvInfo.nType)
                {
                    return TRUE;
                }
			}
			
			UKSendUnlockAck(pmsg->dwParam);

			TalkSendUnlockGate2ACC(UNLOCKTYPE_TALK);

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
**	FUNCTION NAME:	GMTalkInWaitGMPickupPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			the process of the Talk Control in Wait for MHV pickup state.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
GMTalkInWaitGMPickupPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return TRUE;
	}

	switch(pmsg->dwMsg)
	{
		case FC_PICKUP_GM:
		{
			//Note: save GMcode which picked up.
			GMStopCallRing();
			GMSendGMPickupToNet();
				
			g_TalkInfo.talking.dwTalkState = ST_CI_WAIT_PICKUP_ACK;
			g_TalkInfo.Timer.dwTalkTimer = GetTickCount();

			//ShowTalkWnd();	

			bRet = TRUE;
			break;	
		}
		case FC_HANGUP_GM:
			{
				if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
				{
					GMStopCallRing();
					TalkSendGMHangupAck(pmsg);			
					
					g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
					ShowTalkWnd();
					bRet = TRUE;
				}
				break;
			}
		case FC_HANGUP_HV:
			{
				if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
				{
					GMStopCallRing();
					TalkSendHVHangupAck(pmsg);	
					GMStopCallingVideo();
					
					g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();					
					
					ShowTalkWnd();
					bRet = TRUE;
				}

				break;
			}
		case MXMSG_SECURITY_ALARM:
			{
//				GMStopCallRing();
				if (MM_TYPE_TALK_GM == g_TalkInfo.talking.bMMType)
				{
					GMStopCallRing();
					GMSendGMPickupToNet();//Temp for the bug 2729
					TalkSendGMhangupToNet();
				}				
				else if (MM_TYPE_TALK_MC == g_TalkInfo.talking.bMMType)
				{
					GMStopCallRing();
					TalkSendGMhangupToNet();					
				}
				else if (MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType)
				{
					GMSendGMPickupToNet();//Temp for the bug 2729
					TalkSendGMhangupToNet();
					
				}	

				g_TalkInfo.talking.dwTalkState = ST_ORIGINAL;
				WndHideTalkWindow();

				bRet = TRUE;
#ifdef TALK_LOG_DEBUG				
				printf("RecordLogData line=%d\n",__LINE__);
#endif
				RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
				break;
			}

		case FC_HANGUP_MC:
			{
				if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
				{
					TalkSendMCHangupAck(pmsg);
					GMStopCallRing();
					GMStopCallingVideo();
					g_TalkInfo.talking.dwTalkState = ST_TALK_END;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();					
					ShowTalkWnd();
					bRet = TRUE;
				}

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
**	FUNCTION NAME:	TalkInWaitforPickupAck()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			the process of the Talk Control in Wait for HV_pickup ACK state.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.None
**	NOTES:
**			
*/
BOOL
TalkInWaitforPickupAck(MXMSG * pmsg)
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return TRUE;
	}

	switch(pmsg->dwMsg)
	{
		case FC_ACK_PICKUP_GM:
		{
			g_TalkInfo.talking.dwTalkState = ST_CI_TALKING;
			g_TalkInfo.Timer.dwTalkTimer = GetTickCount();

			ShowTalkWnd();

			GMStopCallRing();
			GMStartTalking();

#ifdef HV_TALKMD_DEBUG
			printf("TC-STATE: -------> STC_CI_TALKING..\n");
#endif
			bRet = TRUE;
			break;
		}
		case FC_HANGUP_GM:
		{
			TalkSendGMHangupAck(pmsg);

			g_TalkInfo.talking.dwTalkState = ST_TALK_END;
			g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
			ShowTalkWnd();
			bRet = TRUE;
			break;
		}
		case FC_HANGUP_MC:
			{
				TalkSendMCHangupAck(pmsg);

				g_TalkInfo.talking.dwTalkState = ST_TALK_END;
				g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
				ShowTalkWnd();
				bRet = TRUE;
				break;
			}
		case MXMSG_SECURITY_ALARM:
			{
				TalkSendGMhangupToNet();
				g_TalkInfo.talking.dwTalkState = ST_ORIGINAL;				
				
				WndHideTalkWindow();
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
**	FUNCTION NAME:	TalkInTalkingPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			the process of  talking state.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
TalkInTalkingPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return TRUE;
	}
	
	switch(pmsg->dwMsg)
	{
		case FC_HANGUP_GM:
		{
			GMStopTalking();
			if (FLAG_SELF_GM == pmsg->dwParam)
			{
				TalkSendGMHangupCmd();			
			
				g_TalkInfo.talking.dwTalkState	= ST_CI_WAIT_HANGUP_ACK;
				g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
			
				bRet = TRUE;
			}
			else if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
			{				
				TalkSendGMHangupAck(pmsg);	
								
				g_TalkInfo.talking.dwTalkState = ST_TALK_END;
				g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
				ShowTalkWnd();
				
				bRet = TRUE;
			}
			
			break;
		}
		case FC_HANGUP_MC:
			{
				if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
				{
					TalkSendMCHangupAck(pmsg);
					
					GMStopTalking();
					
					GMStopCallingVideo();
					
					g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();			
					ShowTalkWnd();				
					bRet = TRUE;
				}
				break;
			}
		case FC_HANGUP_HV:
			{	
				if (g_TalkInfo.talking.dwDestIP == pmsg->dwParam) 
				{
					TalkSendHVHangupAck(pmsg);
				
					GMStopTalking();
				
					GMStopCallingVideo();				
		
					g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();			
				
					ShowTalkWnd();				
					bRet = TRUE;				
				}

				break;
			}			

		case MXMSG_SECURITY_ALARM://when talking, the alarm happened.
			{
				if (MM_TYPE_TALK_MC == g_TalkInfo.talking.bMMType) 
				{
					GMStopTalking();
					TalkSendGMhangupToNet();
				}
				else if (MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType)
				{
					GMStopTalking();
					TalkSendGMhangupToNet();					
				}
				else
				{
					GMStopTalking();
					TalkSendGMhangupToNet();
				}

				g_TalkInfo.talking.dwTalkState = ST_ORIGINAL;
				WndHideTalkWindow();
#ifdef TALK_LOG_DEBUG				
				printf("RecordLogData line=%d\n",__LINE__);
#endif
				RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
				
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
**	FUNCTION NAME:	TalkOutTalkingPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			the process of talking state.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
TalkOutTalkingPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return TRUE;
	}

	switch(pmsg->dwMsg)
	{
		case FC_HANGUP_MC:
		{
			if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
			{
				TalkSendMCHangupAck(pmsg);
				GMStopTalking();
				
				g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
				g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
				
				ShowTalkWnd();
				bRet = TRUE;
			}
			break;
		}
		case FC_HANGUP_GM:
		{
			if (FLAG_SELF_GM == pmsg->dwParam)
			{
				// 900 hv 死机
				if ( IsUseGPVideo())
				{
					g_TalkInfo.talking.bNeedPhotoSaved = FALSE;
					GMStopTalking();
					g_TalkInfo.talking.dwTalkState	= ST_CO_WAIT_HANGUP_ACK;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
					bRet = TRUE;	
				}
				else
				{
					if (g_TalkInfo.talking.bNeedPhotoSaved && !g_TalkInfo.talking.bHavePhotoSaved && GetLeavePhotoStatus()) 
					{
					}
					else
					{
						GMStopTalking();
					}
					TalkSendGMHangupCmd();			
					
					g_TalkInfo.talking.dwTalkState	= ST_CO_WAIT_HANGUP_ACK;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
					
					bRet = TRUE;
				}
			}
			else if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
			{
				GMStopTalking();
				TalkSendGMHangupAck(pmsg);						
				
				g_TalkInfo.talking.dwTalkState = ST_TALK_END;
				g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
				ShowTalkWnd();
#ifdef TALK_LOG_DEBUG				
				printf("RecordLogData line=%d\n",__LINE__);
#endif
				RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
//				printf("----------------------FC_HANGUP_GM: RecordLogData Time Cost:%d\n",GetTickCount() - g_TalkInfo.Timer.dwTalkTimer);
				
				bRet = TRUE;
			}

			break;
		}
		case FC_HANGUP_HV:
		{
			if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
			{
				// 900 hv 死机
				if ( IsUseGPVideo())
				{
					g_TalkInfo.talking.bNeedPhotoSaved = FALSE;
					GMStopTalking();
					g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
					
					ShowTalkWnd();
#ifdef TALK_LOG_DEBUG					
					printf("RecordLogData line=%d\n",__LINE__);
#endif
					RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
	//				printf("----------------------FC_HANGUP_HV: RecordLogData Time Cost:%d\n",GetTickCount() - g_TalkInfo.Timer.dwTalkTimer);
					
					bRet = TRUE;
				}
				else
				{
					TalkSendHVHangupAck(pmsg);
					if (g_TalkInfo.talking.bNeedPhotoSaved && !g_TalkInfo.talking.bHavePhotoSaved ) 
					{
					}
					else
					{
						GMStopTalking();
					}
					
					
					g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
					
					ShowTalkWnd();
#ifdef TALK_LOG_DEBUG					
					printf("RecordLogData line=%d\n",__LINE__);
#endif
					RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
	//				printf("----------------------FC_HANGUP_HV: RecordLogData Time Cost:%d\n",GetTickCount() - g_TalkInfo.Timer.dwTalkTimer);
					
					bRet = TRUE;
				}
			}

			break;
		}
		case MXMSG_SECURITY_ALARM://when talking, the alarm happened.
			{
				TalkSendGMhangupToNet();
				if (MM_TYPE_TALK_MC == g_TalkInfo.talking.bMMType)
				{
					GMStopTalking();
				}
				else if (MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType)
				{
					GMStopTalking();
				}
				else if (MM_TYPE_TALK_GM == g_TalkInfo.talking.bMMType)
				{
					GMStopTalking();
				}
				else
				{
					GMStopTalking();
				}
				g_TalkInfo.talking.dwTalkState = ST_ORIGINAL;
				WndHideTalkWindow();
#ifdef TALK_LOG_DEBUG				
				printf("RecordLogData line=%d\n",__LINE__);
#endif
				RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
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
**	FUNCTION NAME:	WaitForHangupAckPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	888888
**			process of waiting for HV hangup ACK state.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
WaitForHangupAckPro(MXMSG *pmsg )
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return TRUE;
	}

	switch(pmsg->dwMsg) 
	{
		case FC_ACK_HANGUP_GM:
		{
//			GMStopCallRing();
//			GMStopTalking();
//			GMStopCallingVideo();
			


			g_TalkInfo.talking.dwTalkState = ST_TALK_END;

			g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();

			ShowTalkWnd();
#ifdef TKL_DEBUG			
			printf("%s FC_ACK_HANGUP_GM\n",__FUNCTION__);
#endif
//			if (ST_CO_WAIT_HANGUP_ACK == g_TalkInfo.talking.dwTalkState)
			{
#ifdef TALK_LOG_DEBUG				
				printf("RecordLogData line=%d\n",__LINE__);
#endif
				RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
			}

//			printf("----------------------RecordLogData Time Cost:%d\n",GetTickCount() - g_TalkInfo.Timer.dwTalkTimer);


			bRet = TRUE;
			break;
		}

		case MXMSG_SECURITY_ALARM://when talking, the alarm happened.
			{
				g_TalkInfo.talking.dwTalkState = ST_ORIGINAL;
				WndHideTalkWindow();
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
**	FUNCTION NAME:	LeavePhotoPro()
**	AUTHOR:			Jeff Wang
**	DATE:			6 - Oct - 2008
**
**	DESCRIPTION:	
**			the process of leave photo
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
LeavePhotoPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
	
	if (NULL == pmsg)
	{
		return TRUE;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_AV_TAKEPHOTO:
			{
				GMSendPhoto2Net();
				bRet = TRUE;
				break;
			}
		case COMM_TAKEPHOTO_END:
			{
				g_TalkInfo.talking.dwTalkState = ST_TALK_LEAVEPHOTO_END;
				g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
//				WndHideTalkWindow();
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
**	FUNCTION NAME:	WaitCalloutAckPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			the process of Waiting callout ACK state.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
WaitCalloutAckPro(MXMSG *pmsg)
{
	BOOL bRet = FALSE;
//	EthResolvInfo 	ResolvInfo;

	if (NULL == pmsg)
	{
		return TRUE;
	}
	switch(pmsg->dwMsg) 
	{
		case FC_ACK_CALL_GM_HV:
		{
			g_TalkInfo.talking.dwTalkState =  ST_CO_WAIT_PICKUP;
			g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
			GetMMFormat(pmsg);
			GMStartCallRing();
			GMStartCallingVideo();
			ShowTalkWnd();
			EnableUnlock();

//			RecordLogData(TYPE_CALLOUT, STEP_START, &g_TalkInfo);

			bPhotoSaved							=	TRUE; 
			g_TalkInfo.Timer.dwPhotoSavedTimer	=	GetTickCount();			
			g_TalkInfo.talking.bNeedPhotoSaved	=	TRUE;
			g_TalkInfo.talking.bHavePhotoSaved	=	FALSE;
			
			bRet = TRUE;
			break;
		}
		case FC_ACK_CALL_GM_MC:
			g_TalkInfo.talking.dwTalkState =  ST_CO_WAIT_PICKUP;
			g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
			GetMMFormat(pmsg);
			GMStartCallRing();
			GMStartCallingVideo();		
			ShowTalkWnd();
			EnableUnlock();

			bRet = TRUE;			

			break;
		case FC_ACK_CALL_GM_GM:
			g_TalkInfo.talking.dwTalkState =  ST_CO_WAIT_PICKUP;
			g_TalkInfo.Timer.dwTalkTimer = GetTickCount();	
			GetMMFormat(pmsg);			
			GMStartCallRing();
			
			ShowTalkWnd();		//GM wait for other device picking up.

//			RecordLogData(TYPE_CALLOUT, STEP_START, &g_TalkInfo);
			
			bRet = TRUE;			

			break;
		case FC_REDIRECT_MC:
			{
				g_TalkInfo.talking.bMMType = MM_TYPE_TALK_MC;
				strcpy(g_TalkInfo.talking.szTalkDestDevCode, g_MxMsgBk.szDestDev);//save the calling device code.
				g_TalkInfo.talking.dwDestIP = pmsg->dwParam;
				TalkSendGMcallOutDevice();
				
				g_TalkInfo.talking.dwTalkState = ST_CO_WAIT_CALLOUT_ACK;
				g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
				ShowTalkWnd();
				
				bRet = TRUE;

				break;
			}
		case MXMSG_HANGUP_GM:
		{
			TalkSendGMhangupToNet();
			g_TalkInfo.talking.dwTalkState = ST_CO_WAIT_HANGUP_ACK;
			g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
	#ifdef HV_TALKMD_DEBUG
				printf("TC-STATE: ------->   STC_CO_WAIT_HANGUP_ACK..\n");
	#endif
			ShowTalkWnd();
			bRet = TRUE;
			break;
		}
		case MXMSG_SECURITY_ALARM://when talking, the alarm happened.
			{
				TalkSendGMhangupToNet();
				g_TalkInfo.talking.dwTalkState = ST_ORIGINAL;
				WndHideTalkWindow();
				bRet = TRUE;
				break;
			}
		default:
		break;

	}
	
	return bRet;
}

static void TimeHandler(int signo)
{
	printf("Jason--->TimeHandler\n");
	//MxPutMsg(&HVMsg);
}

/*
** Jason: for bug 4581
*/
static void InstallSigAlrmHandler()
{
	if (signal(SIGALRM, TimeHandler) == SIG_ERR)
	{
		printf("Jason---> SIGALRM handler install error\n");
		return;
	}
	printf("Jason---> SIGALRM handler install finshed\n");
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkOutWaitForPickupPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			  the process of waiting destDevice picking up state
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**	 		
*/
BOOL
TalkOutWaitForPickupPro(MXMSG * pmsg)
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return TRUE;
	}	
	switch(pmsg->dwMsg) 
	{
		case FC_PICKUP_MC:
		{
			if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
			{				
				GMStopCallRing();
				TalkSendMCPickupACK(pmsg);
				GMStartTalking();
				g_TalkInfo.talking.dwTalkState = ST_CO_TALKING;
				g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
#ifdef HV_TALKMD_DEBUG
				printf("TC-STATE:-> STC_CO_TALKING ..\n");
#endif	
				ShowTalkWnd();
				bRet = TRUE;
			}
			break;
		}

		case FC_HANGUP_MC:
			{
				if (MM_TYPE_TALK_MC == g_TalkInfo.talking.bMMType
					|| pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
				{
					TalkSendMCHangupAck(pmsg);
					GMStopCallRing();
#ifdef TALK_LOG_DEBUG					
					printf("GMStopTalking 2222\n");
#endif
					GMStopTalking();
					
					g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
					
					ShowTalkWnd();
					bRet = TRUE;
				}
				break;
			}
			
		case FC_PICKUP_HV:
		{
			if (TRUE==IsUseGPVideo() && NULL == HVMsg.pParam)
			{
				char *pParam = malloc(2*sizeof(DWORD));
				memset(pParam, 0, 2*sizeof(DWORD));
				memcpy(pParam, pmsg->pParam, 2*sizeof(DWORD));
				memcpy(&HVMsg, pmsg, sizeof(MXMSG));
				HVMsg.pParam = pParam;
				//InstallSigAlrmHandler();
				//alarm(1);
				nCurTick = GetTickCount();
				printf("Jason--->set time HVMsg.pParam=%d\n", strlen(HVMsg.pParam));
				bRet = TRUE;
				break;		
			}
			nCurTick = 0;
			memset(&HVMsg, 0, sizeof(MXMSG));
			HVMsg.pParam = NULL;
			if (pmsg->dwParam != g_TalkInfo.talking.dwDestIP) 
			{
                BYTE bBackup = g_TalkStream.bLocVideoFormat;
                GetMMFormat(pmsg);
                if(bBackup != g_TalkStream.bLocVideoFormat)
                {
                    GMStopCallingVideo();
				    GMStartCallingVideo();
                    EnableUnlock();
                }
			}
			TalkSendHVPickupACK(pmsg);
            
			g_TalkInfo.talking.dwTalkState = ST_CO_TALKING;
			GMStopCallRing();
			
			g_TalkInfo.talking.dwDestIP = pmsg->dwParam;
			GMStartTalking();
			g_TalkInfo.Timer.dwTalkTimer = GetTickCount();

			ShowTalkWnd();
			RecordLogData(TYPE_CALLOUT, STEP_PICK, &g_TalkInfo);
			
			bRet = TRUE;
			break;
		}

		case FC_HANGUP_HV:
			{
				if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP) 
				{
					if (TRUE==IsUseGPVideo())
					{
						//alarm(0);
						nCurTick = 0;
						DoClearResource(&HVMsg);
						memset(&HVMsg, 0, sizeof(MXMSG));
						printf("Jason--->destroy time\n");
					}
					GMStopCallRing();
					TalkSendHVHangupAck(pmsg);	
					//GMStopCallingVideo();
					if (g_TalkInfo.talking.bNeedPhotoSaved && !g_TalkInfo.talking.bHavePhotoSaved && GetLeavePhotoStatus()) 
					{
					}
					else
					{
						GMStopCallingVideo();
					}
					g_TalkInfo.talking.dwTalkState	= ST_TALK_END;
					g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();

					ShowTalkWnd();
#ifdef TALK_LOG_DEBUG					
					printf("RecordLogData line=%d\n",__LINE__);
#endif
					RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
					
					bRet = TRUE;
				}
				
				break;
			}
		
		case FC_PICKUP_GM:
			{
				GMStopCallRing();
				TalkSendGMPickupACK(pmsg);
				GMStartTalking();
				g_TalkInfo.talking.dwTalkState = ST_CO_TALKING;
				g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
				
				ShowTalkWnd();
				RecordLogData(TYPE_CALLOUT, STEP_PICK, &g_TalkInfo);
				
				bRet = TRUE;
				break;
			}		

		case FC_HANGUP_GM:
		{
			GMStopCallRing();
			printf("------------------------g_TalkInfo.talking.bNeedPhotoSaved = %d; g_TalkInfo.talking.bHavePhotoSaved =%d\n",g_TalkInfo.talking.bNeedPhotoSaved,g_TalkInfo.talking.bHavePhotoSaved);

			if (g_TalkInfo.talking.bNeedPhotoSaved && !g_TalkInfo.talking.bHavePhotoSaved && GetLeavePhotoStatus()) 
			{
#ifdef TALK_LOG_DEBUG				
				printf("GMStopCallingVideo 3333\n");
#endif
			}
			else
			{
#ifdef TALK_LOG_DEBUG				
				printf("GMStopCallingVideo 4444\n");
#endif
				GMStopCallingVideo();
			}

						
			if (FLAG_SELF_GM == pmsg->dwParam)
			{
#ifdef HV_TALKMD_DEBUG
				printf("Self Hangup GM\n");
#endif	
				TalkSendGMHangupCmd();				
				
				g_TalkInfo.talking.dwTalkState	= ST_CO_WAIT_HANGUP_ACK;
				g_TalkInfo.Timer.dwTalkTimer	= GetTickCount();
				ShowTalkWnd();
				bRet = TRUE;
			}
			else if (pmsg->dwParam == g_TalkInfo.talking.dwDestIP)
			{
				TalkSendGMHangupAck(pmsg);

				g_TalkInfo.talking.dwTalkState = ST_TALK_END;
				g_TalkInfo.Timer.dwTalkTimer = GetTickCount();
				ShowTalkWnd();
				bRet = TRUE;
			}	
			break;
		}
		case MXMSG_SECURITY_ALARM://when talking, the alarm happened.
			{
				GMStopCallRing();
				TalkSendGMhangupToNet();
				g_TalkInfo.talking.dwTalkState = ST_ORIGINAL;
				WndHideTalkWindow();
				RecordLogData(TYPE_CALLOUT, STEP_END, &g_TalkInfo);
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
**	FUNCTION NAME:	MTMonitorPro()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			   monitor
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, the message get has been processed in this function. FALSE, has not processed.
**	NOTES:
**			
*/
BOOL
MTMonitorPro(MXMSG * pmsg)
{
	BOOL bRet = FALSE;

	if (NULL == pmsg)
	{
		return TRUE;
	}

	switch(pmsg->dwMsg) 
	{		
		case FC_MNT_INTERRUPT:
		{
			SendMNTInterruptCMD(pmsg);			
			bRet = TRUE;
			break;
		}
		
		case MXMSG_SECURITY_ALARM://when talking, the alarm happens.
			{			
				SendMNTInterruptCMD(pmsg);			
				MTStopMonitorVideo();
				if (MON_TYPE_EHV == g_TalkInfo.monitor.bType) 
				{
					RecordLogData(TYPE_MONITOR, STEP_END, &g_TalkInfo);
				}
				g_TalkInfo.monitor.dwMonState = ST_ORIGINAL;
				g_TalkInfo.Timer.dwMonitorTimer = GetTickCount();
				DisableUnlock();
				g_TalkInfo.monitor.bType = MOM_TYPE_NODEVICE;
				bRet = TRUE;
				break;
			}
		case FC_MNT_CANCEL:
		{
			g_TalkInfo.monitor.dwDestIP = pmsg->dwParam;
			strcpy(g_TalkInfo.monitor.szMonSrcDevCode, pmsg->szSrcDev);
				
			if((MON_TYPE_EHV == g_TalkInfo.monitor.bType)
				||(MON_TYPE_MC == g_TalkInfo.monitor.bType))
			{
				MTSendMonCancelAckCMD();

				MTStopMonitorVideo();

				printf("g_TalkInfo.monitor.bType = %d\n",g_TalkInfo.monitor.bType);

				if (MON_TYPE_EHV == g_TalkInfo.monitor.bType) 
				{
					RecordLogData(TYPE_MONITOR, STEP_END, &g_TalkInfo);
				}
				g_TalkInfo.monitor.dwMonState = ST_ORIGINAL;
				g_TalkInfo.Timer.dwMonitorTimer = GetTickCount();

				DisableUnlock();
				g_TalkInfo.monitor.bType = MOM_TYPE_NODEVICE;
				
				bRet = TRUE;
			}
			
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
**	FUNCTION NAME:	SendMNTInterruptCMD()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			 Send FC_ACK_CALL_GM_HV to GM
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
SendMNTInterruptCMD(MXMSG *pmsg)
{
	MXMSG	msgSend ;

	if (NULL == pmsg)
	{
		return ;
	}
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, pmsg->szSrcDev);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_MNT_INTERRUPT;
	msgSend.dwParam		= pmsg->dwParam;
	msgSend.pParam		= NULL;
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkSendGMcallGMAck()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			 Send FC_ACK_CALL_GM_HV to GM
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TalkSendGMcallGMAck(MXMSG *pmsg)
{
	MXMSG	msgSend ;
	unsigned short	MsgLen=14;
	unsigned char * pData;
	if (NULL == pmsg)
	{
		return ;
	}

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_CALL_GM_GM;
	msgSend.dwParam		= pmsg->dwParam;
	msgSend.pParam			= (unsigned char *) malloc(MsgLen+2);
	
	pData=msgSend.pParam;
	memcpy(pData,&MsgLen,2);
	pData+=2;
	*pData=0;//Prepared
	pData++;
	TalkWriteMMFormat(pData);
	MxPutMsg(&msgSend);
}


static void TalkWriteMMFormat(unsigned char * pData)
{
	*pData=MM_FORMAT_CNT;
	pData++;
	*pData=MM_AUDIO_TYPE;
	pData++;
	*pData=MM_AUDIO_CAPTURE_DEFAULT;
	pData++;
	*pData=MM_AUDIO_FORMAT_DEFAULT;
	pData++;
	*pData=0;
	pData++;
	*pData=MM_VIDEO_TYPE;
	pData++;
	*pData=MM_VIDEO_CAPTURE_DEFAULT;
	pData++;
	*pData=MM_VIDEO_FORMAT_h264;
	pData++;
	*pData=0;
	pData++;
	*pData=MM_VIDEO_TYPE;
	pData++;
	*pData=MM_VIDEO_CAPTURE_DEFAULT;
	pData++;
	*pData=MM_VIDEO_FORMAT_MPEG4_PAL;
	pData++;
	*pData=0;
	pData++;

}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkSendMCcallGMAck()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			 send FC_ACK_CALL_MC_HV to MV
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TalkSendMCcallGMAck(MXMSG *pmsg)
{
	MXMSG	msgSend ;
	unsigned short	MsgLen=14;
	unsigned char * pData;
	memset(&msgSend, 0, sizeof(MXMSG));

	if (NULL == pmsg)
	{
		return ;
	}

	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, pmsg->szSrcDev);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_CALL_MC_GM;
	msgSend.dwParam		= pmsg->dwParam;
	msgSend.pParam			= (unsigned char *) malloc(MsgLen+2);
	
	pData=msgSend.pParam;
	memcpy(pData,&MsgLen,2);
	pData+=2;
	*pData=0;//Prepared
	pData++;
	TalkWriteMMFormat(pData);
	MxPutMsg(&msgSend);
	
#ifdef DEBUG_CALL_MCGM
	printf("TalkSendMCcallGMAck\n");
#endif 				
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkSendMCPickupACK()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**				send MC pickup ack to MC.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TalkSendMCPickupACK(MXMSG * pmsg)
{
	MXMSG	msgSend ;

	if (NULL ==  pmsg)
	{
		return;
	}
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_PICKUP_MC;
	msgSend.dwParam		= pmsg->dwParam;//here using the MC ip qurried.
	msgSend.pParam		= NULL;

	MxPutMsg(&msgSend);
}


void
TalkSendHVPickupACK(MXMSG * pmsg)
{
	MXMSG	msgSend ;
	unsigned short	MsgLen=13;
	unsigned char * pData;
	if (NULL ==  pmsg)
	{
		return;
	}
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_PICKUP_HV;
	msgSend.dwParam		= pmsg->dwParam;//here using the MC ip qurried.
	msgSend.pParam			= (unsigned char *) malloc(MsgLen+2);
	
	pData=msgSend.pParam;
	memcpy(pData,&MsgLen,2);
	pData+=2;
	TalkWriteMMFormat(pData);
	MxPutMsg(&msgSend);
}

void
TalkSendGMPickupACK(MXMSG * pmsg)
{
	MXMSG	msgSend ;
	
	if (NULL ==  pmsg)
	{
		return;
	}
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_PICKUP_GM;
	msgSend.dwParam		= pmsg->dwParam;//here using the MC ip qurried.
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkSendGMHangupAck()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			send ACK of every message HV received from other device through 
**			Ethernet Module.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TalkSendGMHangupAck(MXMSG *pmsg)
{
	MXMSG	msgSend;

	if (NULL == pmsg)
	{
		return ;
	}

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_HANGUP_GM;
	msgSend.dwParam		= pmsg->dwParam;
	msgSend.pParam		= NULL;

	MxPutMsg(&msgSend);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkSendMCHangupAck()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			send ACK of every message HV received from other device through 
**			Ethernet Module.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TalkSendMCHangupAck(MXMSG *pmsg)
{
	MXMSG	msgSend;

	if (NULL == pmsg)
	{
		return ;
	}

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_HANGUP_MC;
	msgSend.dwParam		= pmsg->dwParam;
	msgSend.pParam		= NULL;

	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkSendHVHangupAck()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			send ACK of every message HV received from other device through 
**			Ethernet Module.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TalkSendHVHangupAck(MXMSG *pmsg)
{
	MXMSG	msgSend;

	if (NULL == pmsg)
	{
		return ;
	}

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_HANGUP_HV;
	msgSend.dwParam		= pmsg->dwParam;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GMSendGMPickupToNet()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Send FC_PICKUP_HV to net.
**			
**			 
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**		
*/
void
GMSendGMPickupToNet(void)
{
	MXMSG	msgSend;

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);	
	msgSend.dwSrcMd			= MXMDID_TALKING;
	msgSend.dwDestMd		= MXMDID_ETH;
	msgSend.dwMsg			= FC_PICKUP_GM;
	msgSend.dwParam			= g_TalkInfo.talking.dwDestIP;

	msgSend.pParam			= (unsigned char *) malloc(sizeof(DWORD));
	g_TalkInfo.dwSelfGMIP	= ChangeIPFormat(GetSelfIP());
	memcpy(msgSend.pParam, &g_TalkInfo.dwSelfGMIP, sizeof(DWORD));
	printf("GM pick up send to %x.\n", g_TalkInfo.talking.dwDestIP);
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkSendGMhangupToNet()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
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
void
TalkSendGMhangupToNet(void)
{
	MXMSG	msgSend;

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);	
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_HANGUP_GM;
	msgSend.dwParam		= g_TalkInfo.talking.dwDestIP;
	msgSend.pParam		= NULL;

	MxPutMsg(&msgSend);
#ifdef FW_DEBUG
	printf("Send FC_HANGUP_HV message to %x.\n", msgSend.dwParam);
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkSendHVcallGMAck()
**	AUTHOR:			Jeff Wang
**	DATE:			26 - Aug - 2008
**
**	DESCRIPTION:	
**			 send FC_ACK_CALL_MC_HV to MV
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
TalkSendHVcallGMAck(MXMSG *pmsg)
{
	MXMSG	msgSend ;
	unsigned short	MsgLen=14;
	unsigned char * pData;
	if (NULL == pmsg)
	{
		return ;
	}
	memset(&msgSend, 0, sizeof(MXMSG));

	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, pmsg->szSrcDev);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_CALL_HV_GM;
	msgSend.dwParam		= pmsg->dwParam;
	msgSend.pParam			= (unsigned char *) malloc(MsgLen+2);
	
	pData=msgSend.pParam;
	memcpy(pData,&MsgLen,2);
	pData+=2;
	*pData=0;//Prepared
	pData++;
	TalkWriteMMFormat(pData);
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkExit()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			the exit of the TALK moduel, to clear resource here.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TalkExit(void)
{
	GMCODELIST * pNext = NULL;
	GMCODELIST * pTemp = NULL;

//	if ((NULL != g_TalkInfo.hTalk) && IsWindow(g_TalkInfo.hTalk))
//	{
//		DestroyWindow(g_TalkInfo.hTalk);
//	}


	if (NULL != g_TakePhoto.pBuf)
	{
		free(g_TakePhoto.pBuf);
#ifdef MM_LEAK_DEBUG
		printf("PHOTO BBB: EHV free a memory: entry is %x.\n", g_TakePhoto.pBuf);
#endif
	}
	
//	pNext = g_TalkInfo.gmCode;
	while (pNext != NULL)
	{
		pTemp = (GMCODELIST *)pNext->pNext;
		free(pNext);
		pNext = pTemp;
	}
//	g_TalkInfo.gmCode = NULL;
//	CallRelease();
	TlLogExit();
	DpcRmMd(MXMDID_TALKING);
#ifdef HV_TALKMD_DEBUG
		printf("#######: talk module exit with 0\n");
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SaveHVcodePickup()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			when TC received HV pickup, save the code of the picking HV.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
SaveHVcodePickup(void)
{
	;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkStartLeaveWord()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			when TC received HV pickup, save the code of the picking HV.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TalkStartLeaveWord(void)
{
;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TCStopLeaveWord()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			when TC received HV pickup, save the code of the picking HV.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TCStopLeaveWord(void)
{
;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MTSendMonStartAckCMD()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			when TC received HV pickup, save the code of the picking HV.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
MTSendMonStartAckCMD(MXMSG *pmsg)
{
	MXMSG	msgSend;	
	unsigned short	MsgLen=14;
	unsigned char * pData;
#ifdef MGM_DEBUG
	printf("the GM: %d, code:%s..\n",g_TalkInfo.monitor.nGMID, g_TalkInfo.monitor.szMonSrcDevCode);
#endif
	if(g_MonitorTime > 0)
		{
			MsgLen = 16;
	}

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.monitor.szMonSrcDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_MNT_START;
	msgSend.dwParam		= g_TalkInfo.monitor.dwDestIP;
	msgSend.pParam			= (unsigned char *) malloc(MsgLen+2);
	
	pData=msgSend.pParam;
	memcpy(pData,&MsgLen,2);
	pData+=2;
	
	if (ST_CI_TALKING == g_TalkInfo.talking.dwTalkState
		|| ST_CO_TALKING == g_TalkInfo.talking.dwTalkState)
	{
		*(pData) = 1; 
	}
	else
	{
		*(pData) = 0; 
	}
	pData++;

	
	TalkWriteMMFormat(pData);

	if(g_MonitorTime > 0)
	{	
		//int MonitorTime = (int)(g_MonitorTime * 0.001)
		pData += 13;
		*(pData) = g_MonitorTime * 0.001;
		pData++;
		*(pData) = (int)(g_MonitorTime * 0.001)>>8;
		pData++;

		printf(" [%s__%d ] Monitor  Time  = %d\n",__func__,__LINE__,g_MonitorTime);	
	}
	
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MTSendMonStartAckCMD()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			when TC received HV pickup, save the code of the picking HV.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
MTSendMonCancelAckCMD(void)
{
	MXMSG	msgSend;
#ifdef MGM_DEBUG
	printf("the GM: %d, code:%s..\n",g_TalkInfo.monitor.nGMID, g_TalkInfo.monitor.szMonSrcDevCode);
#endif
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.monitor.szMonSrcDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_MNT_CANCEL;
	msgSend.dwParam		= g_TalkInfo.monitor.dwDestIP;
	msgSend.pParam		= (BYTE*)malloc(1);
	
	if (ST_CI_TALKING == g_TalkInfo.talking.dwTalkState
		|| ST_CO_TALKING == g_TalkInfo.talking.dwTalkState)
	{
		*(msgSend.pParam) = 1; 
	}
	else
	{
		*(msgSend.pParam) = 0; 
	}

	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UKSendUnlockAck()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			when TC received HV pickup, save the code of the picking HV.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
UKSendUnlockAck(DWORD dwUlkIP)
{
	MXMSG	msgSend ;

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.unlock.szULDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_UNLK_GATE;
	msgSend.dwParam		= dwUlkIP;
	msgSend.pParam		= (UCHAR*)malloc(1);
	*(msgSend.pParam)	= 0;

	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	HVEnableUnlock()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			unlock enable
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
EnableUnlock(void)
{
	EthResolvInfo 	ResolvInfo;
	memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
	
	if (DEVICE_CORAL_DB == g_DevFun.DevType)
	{
		if (ST_MT_MONITOR == g_TalkInfo.monitor.dwMonState) 
		{
			ResolvInfo.nIP = ChangeIPFormat(g_TalkInfo.monitor.dwDestIP);
			
			ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_IP;
			FdFromAMTResolv(&ResolvInfo);
		}
	}
	

	if (((MM_TYPE_TALK_MC == g_TalkInfo.talking.bMMType) && (ST_ORIGINAL != g_TalkInfo.talking.dwTalkState))
		|| ((MON_TYPE_MC == g_TalkInfo.monitor.bType)  && (ST_ORIGINAL != g_TalkInfo.monitor.dwMonState)))
//		|| ATM_TYPE_MC == ResolvInfo.nType )
	{
		g_TalkInfo.bUnLock = g_ASPara.MCUnlockEnable;
	}
	else
	{
		g_TalkInfo.bUnLock = TRUE;
	}

#ifdef TALK_DEBUG
	printf("****Talk Device: %d Unlock Status: %d****\n", g_TalkInfo.talking.bMMType, g_TalkInfo.bUnLock);
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DisableUnlock()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			when TC received HV pickup, save the code of the picking HV.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
DisableUnlock(void)
{
	g_TalkInfo.bUnLock = FALSE;
#ifdef HV_TALKMD_DEBUG
	printf("### disable unlock function................\n");
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	HVCanUnLock()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			to know if can unlock gm.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, unlock enable, FLASE, unlock disable.
**	NOTES:
**			
*/
BOOL
HVCanUnLock(void)
{
	return g_TalkInfo.bUnLock;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MakeUnLockIP()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			to get the unlock GM IP .
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
MakeUnLockIP(void)
{
	if (g_TalkInfo.talking.dwTalkState )
	{
		g_TalkInfo.unlock.dwDestIP = g_TalkInfo.talking.dwDestIP;
		strcpy(g_TalkInfo.unlock.szULDestDevCode, g_TalkInfo.talking.szTalkDestDevCode);
		g_TalkInfo.unlock.nType = g_TalkInfo.talking.bMMType;
	}
	else if (ST_MT_MONITOR == g_TalkInfo.monitor.dwMonState)
	{
		g_TalkInfo.unlock.dwDestIP = g_TalkInfo.monitor.dwDestIP;
		strcpy(g_TalkInfo.unlock.szULDestDevCode, g_TalkInfo.monitor.szMonSrcDevCode);	
		g_TalkInfo.unlock.nType = g_TalkInfo.monitor.bType;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetTalkState()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			when TC received HV pickup, save the code of the picking HV.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
DWORD
GetTalkState(void)
{
	return g_TalkInfo.talking.dwTalkState;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetMonitorState()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			when TC received HV pickup, save the code of the picking HV.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
DWORD
GetMonitorState(void)
{
	return g_TalkInfo.monitor.dwMonState;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsTalkNoTalking()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			If the Talk Module is not in talking status.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				TRUE, means the talk module is idle. FALSE, means the talk module is busy.
**	NOTES:
**			
*/
BOOL
IsTalkNoTalking(void)
{
	if (ST_ORIGINAL == g_TalkInfo.talking.dwTalkState)
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
**	FUNCTION NAME:	ResetDestiDevice()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			clear the destDeviceCode when talking ended.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
ResetDestiDevice(void)
{
	g_TalkInfo.talking.dwDestIP = 0;
	strcpy(g_TalkInfo.talking.szName,"");
	strcpy(g_TalkInfo.talking.szTalkDestDevCode, "");
	g_TalkInfo.talking.bMMType	= MM_TYPE_NORMAL;

	g_TalkInfo.monitor.dwDestIP = 0;
	strcpy(g_TalkInfo.monitor.szMonSrcDevCode, "");
	strcpy(g_TalkInfo.monitor.szName, "");

	strcpy(g_TakePhoto.szAddr, "");
//	g_TalkInfo.nForwardSta = 0;

}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GMStartCallRing()
**	AUTHOR:			Harry Qian
**	DATE:			16 - Jan - 2007
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
GMStartCallRing()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_PLY_CALL_A;
	msgSend.dwParam		= 0;
	msgSend.pParam		= NULL;

	MxPutMsg(&msgSend);
	printf("Talk: Send calling Note\n");
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GMStopCallRing()
**	AUTHOR:			Harry Qian
**	DATE:			16 - Jan - 2007
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
GMStopCallRing(void)
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_STP_CALL_A;
	msgSend.dwParam		= 0;
	msgSend.pParam		= NULL;

	MxPutMsg(&msgSend);
}

BOOL
SetCallDestIPCode(const char * pCode, UINT nIP)
{
	BOOL bRet = FALSE;

	g_TalkInfo.talking.dwDestIP = nIP;
	strcpy(g_TalkInfo.talking.szTalkDestDevCode, pCode);
	bRet = TRUE;
	
	return bRet;
}

BOOL ConvertGPCMD(unsigned short nFunCode, unsigned char* pData, int nDataLen,unsigned int nDestIPAddr)
{
	BOOL bRet=FALSE;
	int nCheckGPVideoCMDResult=0;
	char DestDev[20]={0};
	
	nCheckGPVideoCMDResult=CheckGPVideoCMD(nFunCode,nDestIPAddr);
	if(CHECK_GP_RESULT_GP2DEST==nCheckGPVideoCMDResult)
	{
		if (g_TalkInfo.monitor.dwMonState) 
		{
			MXSendCMD2Eth(nFunCode, pData, nDataLen,g_TalkInfo.monitor.dwDestIP, g_TalkInfo.monitor.szMonSrcDevCode);
		}
		else
		{
			MXSendCMD2Eth(nFunCode, pData, nDataLen,g_TalkInfo.talking.dwDestIP,g_TalkInfo.talking.szTalkDestDevCode);
		}
		bRet=TRUE;
	}
	else if(CHECK_GP_RESULT_DEST2GP==nCheckGPVideoCMDResult)
	{
		MXSendCMD2Eth(nFunCode, pData, nDataLen,GetGPIP(),DestDev);
		bRet=TRUE;
	}
	return bRet;
}

static	void
TalkSendGMcallOutDevice(void)
{
	MXMSG		msgSend;
	unsigned short	MsgLen=13;
	unsigned char * pData;
	memset(&msgSend, 0, sizeof(msgSend));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	if (MM_TYPE_TALK_GM == g_TalkInfo.talking.bMMType)
	{
		msgSend.dwMsg		= FC_CALL_GM_GM;
	}
	else if (MM_TYPE_TALK_MC == g_TalkInfo.talking.bMMType)
	{
		msgSend.dwMsg		= FC_CALL_GM_MC;
	}
	else if (MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType)
	{
		msgSend.dwMsg		= FC_CALL_GM_HV;
	}

	msgSend.dwParam		= g_TalkInfo.talking.dwDestIP;
	msgSend.pParam			= (unsigned char *) malloc(MsgLen+2);
	pData=msgSend.pParam;
	memcpy(pData,&MsgLen,2);
	pData+=2;
    
	TalkWriteMMFormat(pData);
	MxPutMsg(&msgSend);	
//	printf("FC: the msg:%x, the ip:%x, the code:%s.\n", msgSend.dwMsg, msgSend.dwParam, msgSend.szDestDev);
}


UINT
GetHVSelfID(void)
{
	static BOOL bUpdate = FALSE;
	if (FALSE == bUpdate)
	{
		EthResolvInfo iHVIPinfo;
		int iLoop = 0;
		

		iHVIPinfo.nQueryMethod	= HO_QUERY_METHOD_IP;
		iHVIPinfo.nIP			= GetSelfIP();
		if (BacpNetCtrlResolv(&iHVIPinfo)) 
		{
//			g_TalkInfo.nHVCnt = iHVIPinfo.nIPIDCnt;
			for (iLoop = 0; iLoop < iHVIPinfo.nIPIDCnt; iLoop++)
			{
				if ( iHVIPinfo.nIP == iHVIPinfo.IPIDArr[iLoop].nIPAddr )
				{
//					g_TalkInfo.nHVid = iHVIPinfo.IPIDArr[iLoop].dwID;

					bUpdate = TRUE;
				}
			}

		}
	}

//	return g_TalkInfo.nHVid;
	return 0;
}


void
GetMdTalkInfo(PBYTE pBuf)
{
	BYTE	bVer = 0;
	PBYTE	pData = pBuf;
	unsigned short nDataLen = *(unsigned short *)pData;
	pData += 2;
	bVer = *pData;
	pData += 1;
	g_TalkFunc.bDirection = *pData;

	pData += 1;
	g_TalkFunc.bAudio = *pData;
	pData += 1;
	g_TalkFunc.dwAudioFormat = *(DWORD *)pData;
	pData += 4;
	g_TalkFunc.bVideo = *pData;
	pData += 1;
	g_TalkFunc.dwVideoFormat = *(DWORD *)pData;

	g_TalkFunc.bValid = 1;	

	printf("the ver=%d, the ndatalen=%d.the bdirection=%d, the baudio=%d, the bvideo=%d \n",
		bVer, nDataLen, g_TalkFunc.bDirection, g_TalkFunc.bAudio, g_TalkFunc.bVideo	);
	return;
}

BOOL
TalkFuncAudioVideo(BOOL * pbCapA2Eth, BOOL * pbCapV2Eth, BOOL * pbPlayEthA, BOOL * pbPlayEthV)
{
	if (g_TalkFunc.bValid)
	{
		*pbCapA2Eth = g_TalkFunc.bAudio & 0x01;
		*pbPlayEthA = g_TalkFunc.bAudio & 0x02;
		*pbCapV2Eth = g_TalkFunc.bVideo & 0x01;
		*pbPlayEthV = g_TalkFunc.bVideo & 0x02;
	}
	
	return g_TalkFunc.bValid;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GMStartCallingVideo
**	AUTHOR:			Jeff Wang
**	DATE:			01 - Aug - 2008
**
**	DESCRIPTION:	
**			GM 
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
GMStartCallingVideo(void)
{
	MXMSG	msgSend;
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);		
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;
	
	if (MM_TYPE_TALK_MC	== g_TalkInfo.talking.bMMType
		 || MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STA_MM2ETH_V;
		TalkSendLedStsNote(COMM_CCDLED_ON);
	}
	else if (MM_TYPE_TALK_GM == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STA_CALLRING;
	}
	
	msgSend.dwParam		= g_TalkInfo.talking.dwDestIP;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);	
	
	if (MM_TYPE_TALK_MC	== g_TalkInfo.talking.bMMType
		|| MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STA_MM2ETH_V;
		msgSend.dwDestMd	= MXMDID_ETH;	
		MxPutMsg(&msgSend);
	}
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GMStopCallingVideo
**	AUTHOR:			Jeff Wang
**	DATE:			01 - Aug - 2008
**
**	DESCRIPTION:	
**			GM 
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
GMStopCallingVideo(void)
{
	MXMSG	msgSend;
		
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);		
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;

	if (MM_TYPE_TALK_MC	== g_TalkInfo.talking.bMMType
		 || MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STP_MM2ETH_V;
		TalkSendLedStsNote(COMM_CCDLED_OFF);
	}
	else if (MM_TYPE_TALK_GM == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STP_CALLRING;
	}

	
	msgSend.dwParam		= g_TalkInfo.talking.dwDestIP;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);	
	
	if (MM_TYPE_TALK_MC	== g_TalkInfo.talking.bMMType
		|| MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STP_MM2ETH_V;
		msgSend.dwDestMd	= MXMDID_ETH;	
		MxPutMsg(&msgSend);
		
		DisableUnlock();
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GMStartTalking
**	AUTHOR:			Jeff Wang
**	DATE:			01 - Aug - 2008
**
**	DESCRIPTION:	
**			GM 
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
GMStartTalking(void)
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);		
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;
	
	if (MM_TYPE_TALK_MC	== g_TalkInfo.talking.bMMType
		|| MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STA_RDIR_ETH_MM_A;
		TalkSendLedStsNote(COMM_CCDLED_ON);
	}
	else if (MM_TYPE_TALK_GM == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STA_RDIR_ETH_MM_A;
	}
	
	msgSend.dwParam		= g_TalkInfo.talking.dwDestIP;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);	
	
	if (MM_TYPE_TALK_MC	== g_TalkInfo.talking.bMMType
		|| MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STA_ETH2MM_A_MM2ETH_AV;
	}
	else if (MM_TYPE_TALK_GM == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STA_RDIR_ETH_MM_A;
	}

	msgSend.dwDestMd	= MXMDID_ETH;	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GMStopTalking
**	AUTHOR:			Jeff Wang
**	DATE:			01 - Aug - 2008
**
**	DESCRIPTION:	
**			GM 
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
GMStopTalking(void)
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);		
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;
	
	if (MM_TYPE_TALK_MC	== g_TalkInfo.talking.bMMType
		|| MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STP_ETH2MM_A_MM2ETH_AV;
		TalkSendLedStsNote(COMM_CCDLED_OFF);
	}
	else if (MM_TYPE_TALK_GM == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STP_RDIR_ETH_MM_A;
	}
	
	msgSend.dwParam		= g_TalkInfo.talking.dwDestIP;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);	
	
	if (MM_TYPE_TALK_MC	== g_TalkInfo.talking.bMMType
		|| MM_TYPE_TALK_EHV == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STP_ETH2MM_A_MM2ETH_AV;
	}
	else if (MM_TYPE_TALK_GM == g_TalkInfo.talking.bMMType) 
	{
		msgSend.dwMsg = MXMSG_STP_RDIR_ETH_MM_A;
	}

	msgSend.dwDestMd	= MXMDID_ETH;	
	MxPutMsg(&msgSend);
	
	DisableUnlock();
}

static void
MTStartMonitorVideo(void)
{
	MXMSG	msgSend;
printf("MTStartMonitorVideo\n");
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.monitor.szMonSrcDevCode);		
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;
	
	msgSend.dwMsg		= MXMSG_STA_MM2ETH_V;
	
	msgSend.dwParam		= g_TalkInfo.monitor.dwDestIP;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);	
	
	msgSend.dwMsg		= MXMSG_STA_MM2ETH_V;
	msgSend.dwDestMd	= MXMDID_ETH;	
	MxPutMsg(&msgSend);

	TalkSendLedStsNote(COMM_CCDLED_ON);
}
VOID 
MTStopMonitorVideo()
{
	MXMSG	msgSend;
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.monitor.szMonSrcDevCode);		
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;
	
	msgSend.dwMsg = MXMSG_STP_MM2ETH_V;	
	
	msgSend.dwParam		= g_TalkInfo.monitor.dwDestIP;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);	
	
	msgSend.dwMsg = MXMSG_STP_MM2ETH_V;
	msgSend.dwDestMd	= MXMDID_ETH;	
	MxPutMsg(&msgSend);

	TalkSendLedStsNote(COMM_CCDLED_OFF);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME: ChangeIPFormat
**	AUTHOR:			Jeff Wang
**	DATE:		 29 - July - 2008
**
**	DESCRIPTION:	
**			Change IP address format
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
DWORD 
ChangeIPFormat(DWORD OldIPAddr)
{
	DWORD NewGPIP;
	UCHAR tempIPAddr1[4], tempIPAddr2[4];
	
	memcpy(tempIPAddr1, &OldIPAddr, sizeof(DWORD));
	
	tempIPAddr2[3] = tempIPAddr1[0];
	tempIPAddr2[2] = tempIPAddr1[1];
	tempIPAddr2[1] = tempIPAddr1[2];
	tempIPAddr2[0] = tempIPAddr1[3];
	
	memcpy(&NewGPIP, tempIPAddr2, sizeof(DWORD));
	return NewGPIP;	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkSendGMHangupCmd()
**	AUTHOR:			Jeff Wang
**	DATE:			29 - July - 2008
**
**	DESCRIPTION:	
**				send GM Hang up cmd to net.
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
TalkSendGMHangupCmd(void)
{
	MXMSG	msgSend;

	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_HANGUP_GM;
	msgSend.dwParam		= g_TalkInfo.talking.dwDestIP;
	msgSend.pParam		= NULL;

	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UnlockGateEnd
**	AUTHOR:		   Jeff Wang
**	DATE:		19 - Jun - 2008
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
SendComm2IOCtrl(DWORD dwComm,DWORD dwSrcMD)
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= dwSrcMD;
	msgSend.dwDestMd	= MXMDID_IOCTRL;

	msgSend.dwMsg		= dwComm;
	msgSend.dwParam		= 0;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UnlockGate
**	AUTHOR:		   Jeff Wang
**	DATE:		19 - Jun - 2008
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
void 
UnlockGateStart(BOOL LCenable, unsigned char QueryMode, DWORD IP, CHAR RdNum[RD_CODE_LEN], BYTE CSN[CSN_LEN])
{
	MXMSG msgSend;

	printf("*********************** UnlockGateStart!\n");

	if(DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
	{
		PIOWrite(PIO_TYPE_UNLOCK, 0);
	}
	else
	{
		if (g_DevFun.bNewPnUsed) 
		{
			PIOWrite(PIO_TYPE_UNLOCK, 0);
		}
		else
		{
			PIOWrite(PIO_TYPE_UNLOCK, 1);
		}
	}
    g_DoorUlkBehav.DoorUlkBehaviour = DOOR_ACTIVE_OPEN_START;
    g_DoorUlkBehav.ForceUlkDetectTimer = GetTickCount();
	
	if(QueryMode==UNLOCKTYPE_FIREALM)
		SendComm2IOCtrl(COMM_UNLOCK_LINKAGE_ON,MXMDID_IOCTRL);
	else 
		SendComm2IOCtrl(COMM_UNLOCK_LINKAGE_ON,MXMDID_TALKING);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UnlockGateEnd
**	AUTHOR:		   Jeff Wang
**	DATE:		19 - Jun - 2008
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
void 
UnlockGateEnd()
{
	//printf("*********************** UnlockGateEnd!\n");

	if(DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
	{
		PIOWrite(PIO_TYPE_UNLOCK, 1);
	}
	else
	{
		if (g_DevFun.bNewPnUsed) 
		{
			PIOWrite(PIO_TYPE_UNLOCK, 1);
		}
		else
		{
			PIOWrite(PIO_TYPE_UNLOCK, 0);
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	StopGMMonState
**	AUTHOR:		   Jeff Wang
**	DATE:		28 - Aug - 2008
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
StopGMMonState()
{
	MXMSG  msgSend;
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	strcpy(msgSend.szSrcDev, g_TalkInfo.monitor.szMonSrcDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.szTalkDevCode);
	msgSend.dwMsg		= FC_MNT_INTERRUPT;
	msgSend.dwParam		= g_TalkInfo.monitor.dwDestIP;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
	TalkSendLedStsNote(COMM_CCDLED_OFF);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GMSendPhoto2Net
**	AUTHOR:		   Jeff Wang
**	DATE:		29 - Aug - 2008
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
GMSendPhoto2Net()
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);		
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;
	
	msgSend.dwMsg = MXMSG_SEND_PHOTO_BMP;	
	
	msgSend.dwParam		= g_TalkInfo.talking.dwDestIP;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);	
	
	msgSend.dwMsg = MXMSG_SEND_PHOTO_BMP;
	msgSend.dwDestMd	= MXMDID_ETH;	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkSavePhotoNote2MM
**	AUTHOR:		   Jeff Wang
**	DATE:		29 - Aug - 2008
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
TalkSavePhotoNote2MM()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_SAVE_PHOTO;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TalkErrorSoundNote2MM
**	AUTHOR:		   Jeff Wang
**	DATE:		29 - Aug - 2008
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
TalkErrorSoundNote2MM()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_PLY_ERROR_A;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GMStopCallRing()
**	AUTHOR:			Harry Qian
**	DATE:			16 - Jan - 2007
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static BOOL
IsSlaveHV(DWORD dwIPAddr)
{
	EthResolvInfo 	ResolvInfo;
	memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
	
	ResolvInfo.nIP = ChangeIPFormat(dwIPAddr);
			
	ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_IP;
	FdFromAMTResolv(&ResolvInfo);
	if (ResolvInfo.nType) 
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
**	FUNCTION NAME:	TalkSendLedStsNote
**	AUTHOR:		   Jeff Wang
**	DATE:		16 - Jun - 2009
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
static void 
TalkSendLedStsNote(DWORD LedSts)
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_IOCTRL;
	msgSend.dwMsg		= LedSts;
	msgSend.pParam		= NULL;
	MxPutMsg(&msgSend);
}
BYTE GetTalkDestStreamFormat(void)
{
	if(g_TalkStream.bDestVideoFormat)
	{
		return g_TalkStream.bDestVideoFormat;
	}
	return MM_VIDEO_FORMAT_DEFAULT;
}
BYTE GetTalkLocStreamFormat(void)
{
	if(g_TalkStream.bLocVideoFormat)
	{
		return g_TalkStream.bLocVideoFormat;
	}
	return MM_VIDEO_FORMAT_DEFAULT;
}

static BOOL CheckMMFormat(BYTE bType,BYTE bCapture,BYTE bFormat)
{
	BOOL bRet=FALSE;
	if(AV2MM_AUDIO==bType)
	{
		if(MM_AUDIO_CAPTURE_DEFAULT==bCapture && MM_AUDIO_FORMAT_DEFAULT==bFormat)
		{
			bRet=TRUE;
		}
	}
	else if(AV2MM_VIDEO==bType)
	{
		if(bCapture==MM_VIDEO_CAPTURE_DEFAULT && (bFormat==MM_VIDEO_FORMAT_h264 || bFormat==MM_VIDEO_FORMAT_MPEG4_PAL))
		{
			bRet=TRUE;
		}
	}
	return bRet;
}


static void GetMMFormat(MXMSG *pmsg)
{
	WORD	wAppOffset=0;
	unsigned char * pData;
	BYTE FormatCnt,MediaType,MediaCapture,MediaFormat;
	UINT i;
	BOOL	bSelLocAudioFormat=FALSE,bSelLocVideoFormat=FALSE;
	
	if (NULL == pmsg)
	{
		return ;
	}
	
	switch(pmsg->dwMsg) 
	{
		case FC_ACK_CALL_GM_HV:
		case FC_ACK_CALL_GM_GM:
		case FC_ACK_CALL_GM_MC:
			wAppOffset=1;
			break;
		case FC_CALL_GM_GM:
		case FC_CALL_MC_GM:
		case FC_CALL_HV_GM:
		case FC_MNT_START:
			wAppOffset=0;
			break;
		case FC_PICKUP_HV:
			wAppOffset=4;
			break;
		default:
			break;
	}
    
	if(pmsg->wDataLen>wAppOffset)
	{
		pData=pmsg->pParam;
	
		pData+=wAppOffset;
		FormatCnt=*pData;

		pData++;
		for (i=0;i<FormatCnt;i++)
		{
			MediaType=*pData;
			pData++;
			MediaCapture=*pData;
			pData++;
			MediaFormat=*pData;
			pData+=2;
			if(MediaType==MM_AUDIO_TYPE && (!bSelLocAudioFormat))
			{
				if (CheckMMFormat(MediaType,MediaCapture,MediaFormat))
				{
						bSelLocAudioFormat=TRUE;
						g_TalkStream.bLocAudioCapture=MediaCapture;
						g_TalkStream.bLocAudioFormat=MediaFormat;	
						
				}
			}
			else if(MediaType==MM_VIDEO_TYPE && (!bSelLocVideoFormat))
			{
				if (CheckMMFormat(MediaType,MediaCapture,MediaFormat))
				{
					bSelLocVideoFormat=TRUE;
					g_TalkStream.bLocVideoCapture=MediaCapture;
					g_TalkStream.bLocVideoFormat=MediaFormat;	
				}
			}
		}
	}
	
	if(!bSelLocAudioFormat)
	{
		g_TalkStream.bLocAudioCapture=MM_AUDIO_CAPTURE_DEFAULT;
		g_TalkStream.bLocAudioFormat=MM_AUDIO_FORMAT_DEFAULT;	
	}
	if(!bSelLocVideoFormat)
	{
		g_TalkStream.bLocVideoCapture=MM_VIDEO_CAPTURE_DEFAULT;
		g_TalkStream.bLocVideoFormat=MM_VIDEO_FORMAT_DEFAULT;	
	}
	
}
void
MTSendDecargsAckCMD(void)
{
	MXMSG	msgSend;
	unsigned short	MsgLen=nVideoHeaderLen+4;
	unsigned char * pData;
	memset(&msgSend, 0, sizeof(MXMSG));
	strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	
	
	if (ST_ORIGINAL != g_TalkInfo.talking.dwTalkState)
	{
		strcpy(msgSend.szDestDev, g_TalkInfo.talking.szTalkDestDevCode);
		msgSend.dwParam		=  g_TalkInfo.talking.dwDestIP;
	}
	else if(ST_MT_MONITOR == g_TalkInfo.monitor.dwMonState)
	{
		strcpy(msgSend.szDestDev, g_TalkInfo.monitor.szMonSrcDevCode);
		msgSend.dwParam		= g_TalkInfo.monitor.dwDestIP;
	}
	else
	{
		return;
	}
	msgSend.dwSrcMd		= MXMDID_MM;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_AV_REQUEST_DECARGS;

	msgSend.pParam			= (unsigned char *) malloc(MsgLen+2);
	
	pData=msgSend.pParam;
	memcpy(pData,&MsgLen,2);
	pData+=2;
	*pData=1;//video
	pData++;
	if(MM_VIDEO_FORMAT_h264==GetTalkLocStreamFormat())
	{
		*pData= 1;//H.264
	}
	else
	{
		*pData = 2;//MPEG4
	}
	pData++;
	memcpy(pData, &nVideoHeaderLen, sizeof(SHORT));
	pData+=2;
	memcpy(pData, &ucVideoFrameHeader, nVideoHeaderLen);

	MxPutMsg(&msgSend);	
}

static
VOID
TalkSendUnlockGate2ACC(BYTE byUnlockType)
{
	MXMSG	msgSend;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_MM;
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
**
**     Copyright Mox Group
**
**     FILE NAME:      ModuleTalk.c
**
**     AUTHOR:          Benson
**
**     DATE:                 24 - 03 - 2015
**
**     FILE DESCRIPTION:
**                                 设置监视时间
**
**     FUNCTIONS: setMonitoringTime
**
**     NOTES:
**             
**    
*/

/**
 * @return MonitoringTime
 */
DWORD setMonitoringTime(){

	printf("%s,%d,...\r\n",__func__,__LINE__);
	if(g_MonitorTime > 0 )
	{
		return g_MonitorTime;
	}


	if(MOX_PROTOCOL_SUPPORT(MOX_P_900_1A))
	{
		return  IRIS_MONITOR_TIMEOUT_9001A;
	}	

	return IRIS_MONITOR_TIMEOUT;
}

