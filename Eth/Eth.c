/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Eth.c
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		10 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**						
**				
**
**				
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMdId.h"
#include "MXTypes.h"
#include "MXMsg.h"
#include "MXList.h"
#include "MXMem.h"
#include "Dispatch.h"
#include "BacpNet.h"
#include "BacpApp.h"
#include "BacpNetComm.h"
#include "BacpNetCtrl.h"
#include "Multimedia.h"
#include "ModuleTalk.h"
#include "Multimedia.h"
#include "TalkLogReport.h"
#include "TelnetLog.h"
#include "FileConfigure.h"
#include "rtc.h"
#include "MenuParaProc.h"
#ifdef __SUPPORT_PROTOCOL_900_1A__
#include "soapcommon.h"
#include "ParaSetting.h"
#endif
/************** DEFINES **************************************************************/

//#define	ETH_INIT_EXIT_DEBUG
//#define	ETH_MSG_DEBUG
//#define	ETH_NET_SEND_DATA_CALLBACK_DEBUG
//#define	ETH_MEM_DEBUG

#define	GET_INNER_SRC_DEV(dev)				HIBYTE(dev)
#define	GET_INNER_DEST_DEV(dev)				LOBYTE(dev)
#define	MAKE_INNER_DEV(src, dest)			MAKEWORD(dest, src)		
#define	EXCHANGE_INNER_SRC_DEST_DEV(dev)	MAKEWORD(GET_INNER_SRC_DEV(dev), GET_INNER_DEST_DEV(dev))

#define	MAX_NET_SEND_CALLBACK_ELM_CNT		20
#define	MIN_SEND_CALLBACK_ID				0
#define	MAX_SEND_CALLBACK_ID				65535
#define	INVALID_SEND_CALLBACK_ID			-1

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

typedef	struct _EthRdirAV 
{
	BOOL			bRdirAV;		// TRUE if redirect video, audio between source and dest
	char			SrcDev[MAX_LEN_DEV_CODE + 1];
	char			DestDev[MAX_LEN_DEV_CODE + 1];
	//BacpAppDevCode	Src;
	//BacpAppDevCode	Dest;
	unsigned int	nDestIPAddr;
	unsigned int	nPhotoDestIPAddr;
} EthRdirAV;

typedef	struct _NetSendCbElm
{
	BOOL						bUsed;
	int							nCbId;
	unsigned char*				pBuf;
	DWORD						dwDestMdId;
} NetSendCbElm;

typedef	struct _NetSendCbElmManStruct
{
	NetSendCbElm*				pElmArr;	
	int							nCurElmCnt;
	int							nMaxElmCnt;
	int							nNextCbId;
} NetSendCbElmManStruct;

typedef	struct _EthManStruct
{
	pthread_t					thWork;	
	BOOL						bthWorkQuit;

	EthRdirAV					RdirAVEthMm;

	NetSendCbElmManStruct		NetSendCbElmMan;
} EthManStruct;

typedef	struct _EthRUNINFO
{
	UINT	nRecvPackCnt;
	UINT	nSendPackCnt;	
} ETHRUNINFO;


/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static EthManStruct		EthMan;

void*					EthWorkThreadFun(void* arg);
static BOOL				EthGetMsg(MXMSG* pMsg);
static void				EthAppProcess();
static void				EthFreeMXMSG(MXMSG* pMsg);

static void				NetSendCbElmManInit(NetSendCbElmManStruct* pMan);
static void				NetSendCbElmManExit(NetSendCbElmManStruct* pMan);
//static BOOL				NetSendCbElmPut(NetSendCbElmManStruct* pMan, int nCbId, unsigned char* pBuf, DWORD dwDestMdId);
static BOOL				NetSendCbElmGet(NetSendCbElmManStruct* pMan, int nCbId, unsigned char** ppBuf, DWORD* pdwDestMdId);
//static int				GetNetNextCbId(NetSendCbElmManStruct* pMan);
//static int				NetSendCbStoreMXMSG(MXMSG* pMsg, DWORD dwDestMdId);

#ifdef ETH_MSG_DEBUG
static void				EthMsgDebug(MXMSG* pMsg, BOOL bPut);
#else
#define	EthMsgDebug(pMsg, bPut)
#endif

ETHRUNINFO	g_PackCnt;	

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetSendCbElmManInit
**	AUTHOR:			Jerry Huang
**	DATE:			21 - Oct - 2006
**
**	DESCRIPTION:	
**			Callback element init
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMan		[IN/OUT]	NetSendCbElmManStruct*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
NetSendCbElmManInit(NetSendCbElmManStruct* pMan)
{
	int				i;
	NetSendCbElm*	pElm	= NULL;

	pMan->nNextCbId		= MIN_SEND_CALLBACK_ID;
	pMan->nMaxElmCnt	= MAX_NET_SEND_CALLBACK_ELM_CNT;
	pMan->nCurElmCnt	= 0;
	pMan->pElmArr = (NetSendCbElm*) malloc(sizeof (NetSendCbElm) * pMan->nMaxElmCnt);

	if (NULL == pMan->pElmArr)
	{
		pMan->nMaxElmCnt = 0;
		return;
	}

#ifdef ETH_NET_SEND_DATA_CALLBACK_DEBUG
	printf("Eth: Cb: malloc %08X\n", (unsigned int) pMan->pElmArr);
#endif

	for (i = 0; i < pMan->nMaxElmCnt; i++)
	{
		pElm = pMan->pElmArr + i;

		pElm->bUsed = FALSE;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetSendCbElmPut
**	AUTHOR:			Jerry Huang
**	DATE:			21 - Oct - 2006
**
**	DESCRIPTION:	
**			put callback element
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMan		[IN/OUT]	NetSendCbElmManStruct*
**				nCbId		[IN]		int
**				pBuf		[IN]		unsigned char*	
**				dwDestMdId	[IN]		DWORD
**	RETURNED VALUE:	
**				TRUE if succeess
**	NOTES:
**			
*/
/*
static BOOL
NetSendCbElmPut(NetSendCbElmManStruct* pMan, int nCbId, unsigned char* pBuf, DWORD dwDestMdId)
{
	int				i;
	NetSendCbElm*	pElm		= NULL;
	NetSendCbElm*	pNewElmArr	= NULL;
	BOOL			bFind		= FALSE;
	BOOL			bRetCode	= FALSE;

	if (NULL == pMan)
	{
		return bRetCode;
	}

	if (pMan->nCurElmCnt >= pMan->nMaxElmCnt)
	{
		pNewElmArr = (NetSendCbElm*) realloc(
			pMan->pElmArr, 
			sizeof (NetSendCbElm) * (pMan->nCurElmCnt + MAX_NET_SEND_CALLBACK_ELM_CNT));
		if (pNewElmArr != NULL)
		{
#ifdef ETH_NET_SEND_DATA_CALLBACK_DEBUG
			printf("Eth: Cb: realloc %08X\n", (unsigned int) pNewElmArr);
#endif
			pMan->pElmArr = pNewElmArr;
			pMan->nMaxElmCnt = pMan->nCurElmCnt + MAX_NET_SEND_CALLBACK_ELM_CNT;
			for (i = pMan->nCurElmCnt; i < pMan->nMaxElmCnt; i++)
			{
				pElm = pMan->pElmArr + i;
				pElm->bUsed = FALSE;
			}

			pElm = pMan->pElmArr + pMan->nCurElmCnt;
			pElm->nCbId = nCbId;
			pElm->pBuf = pBuf;
			pElm->dwDestMdId = dwDestMdId;
			pElm->bUsed = TRUE;
			pMan->nCurElmCnt += 1;
			bRetCode = TRUE;

#ifdef ETH_NET_SEND_DATA_CALLBACK_DEBUG
			printf("Eth: Cb Put: number %d, CbId %d, pBuf %08X, DestMdId %u\n", 
				pMan->nCurElmCnt, nCbId, (unsigned int) pBuf, (unsigned int) dwDestMdId);
#endif
		}
	}
	else
	{
		bFind = FALSE;
		for (i = 0; i < pMan->nMaxElmCnt; i++)
		{
			pElm = pMan->pElmArr + i;

			if (!pElm->bUsed)
			{
				bFind = TRUE;
				break;
			}
		}

		if (bFind)
		{
			pElm->nCbId = nCbId;
			pElm->pBuf = pBuf;
			pElm->dwDestMdId = dwDestMdId;
			pElm->bUsed = TRUE;
			pMan->nCurElmCnt += 1;
			bRetCode = TRUE;

#ifdef ETH_NET_SEND_DATA_CALLBACK_DEBUG
			printf("Eth: Cb Put: number %d, CbId %d, pBuf %08X, DestMdId %u\n", 
				pMan->nCurElmCnt, nCbId, (unsigned int) pBuf, (unsigned int) dwDestMdId);
#endif
		}
	}

	return bRetCode;
}
*/
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetSendCbElmGet
**	AUTHOR:			Jerry Huang
**	DATE:			21 - Oct - 2006
**
**	DESCRIPTION:	
**			get callback element
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMan		[IN/OUT]	NetSendCbElmManStruct*
**				nCbId		[IN]		int
**				ppBuf		[OUT]		unsigned char**	
**				pdwDestMdId	[OUT]		DWORD*
**	RETURNED VALUE:	
**				TRUE if succeess
**	NOTES:
**			
*/
static BOOL
NetSendCbElmGet(NetSendCbElmManStruct* pMan, int nCbId, unsigned char** ppBuf, DWORD* pdwDestMdId)
{
	BOOL			bRetCode	= FALSE;
	int				i;
	NetSendCbElm*	pElm		= NULL;

	if (NULL == pMan)
	{
		return bRetCode;
	}

	for (i = 0; i < pMan->nMaxElmCnt; i++)
	{
		pElm = pMan->pElmArr + i;
		
		if (pElm->bUsed && (pElm->nCbId == nCbId))
		{
			*ppBuf = pElm->pBuf;
			*pdwDestMdId = pElm->dwDestMdId;
			pMan->nCurElmCnt -= 1;
			pElm->bUsed = FALSE;
			bRetCode = TRUE;

#ifdef ETH_NET_SEND_DATA_CALLBACK_DEBUG
			printf("Eth: Cb Get: number %d, CbId %d, pBuf %08X, DestMdId %u\n", 
				pMan->nCurElmCnt, nCbId, (unsigned int) *ppBuf, (unsigned int) *pdwDestMdId);
#endif
			break;
		}
	}

	return bRetCode;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetNetNextCbId
**	AUTHOR:			Jerry Huang
**	DATE:			21 - Oct - 2006
**
**	DESCRIPTION:	
**			get callback next element id
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMan		[IN/OUT]	NetSendCbElmManStruct*
**	RETURNED VALUE:	
**				Callback element id
**	NOTES:
**			
*/
/*
static int
GetNetNextCbId(NetSendCbElmManStruct* pMan)
{
	int		nNextCbId	= pMan->nNextCbId;

	pMan->nNextCbId++;
	if (pMan->nNextCbId > MAX_SEND_CALLBACK_ID)
	{
		pMan->nNextCbId = MIN_SEND_CALLBACK_ID;
	}

#ifdef ETH_NET_SEND_DATA_CALLBACK_DEBUG
	printf("Eth: Cb: Next Id %d\n", nNextCbId);
#endif

	return nNextCbId;
}
*/
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetSendCbElmManExit
**	AUTHOR:			Jerry Huang
**	DATE:			21 - Oct - 2006
**
**	DESCRIPTION:	
**			Callback element exit
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMan		[IN/OUT]	NetSendCbElmManStruct*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
NetSendCbElmManExit(NetSendCbElmManStruct* pMan)
{
	pMan->nMaxElmCnt	= 0;
	pMan->nCurElmCnt	= 0;

	if (pMan->pElmArr != NULL)
	{
#ifdef ETH_MEM_DEBUG
		printf("Eth: free %08X\n", (unsigned int) pMan->pElmArr);
#endif
		free(pMan->pElmArr);
		pMan->pElmArr = NULL;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EthInit
**	AUTHOR:			Jerry Huang
**	DATE:			13 - Oct - 2006
**
**	DESCRIPTION:	
**			Eth module init
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
EthInit()
{
	DpcAddMd(MXMDID_ETH, NULL);

	NetSendCbElmManInit(&EthMan.NetSendCbElmMan);

	BacpNetInit(TRUE);

	BacpNetCtrlInit(TRUE);

	if ((pthread_create(&EthMan.thWork, NULL, EthWorkThreadFun, NULL)) != 0)	
	{
		printf("Eth: create thread fail\n");
	}

	EthMan.bthWorkQuit = FALSE;

	EthMan.RdirAVEthMm.bRdirAV	= FALSE;

#ifdef ETH_INIT_EXIT_DEBUG
	printf("Eth: Initialize ...\n");
#endif

	g_PackCnt.nRecvPackCnt = 0;
	g_PackCnt.nSendPackCnt = 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EthProcess
**	AUTHOR:			Jerry Huang
**	DATE:			13 - Oct - 2006
**
**	DESCRIPTION:	
**			Eth module process
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
EthProcess()
{
	EthAppProcess();
	BacpNetProcess();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EthExit
**	AUTHOR:			Jerry Huang
**	DATE:			13 - Oct - 2006
**
**	DESCRIPTION:	
**			Eth module exit
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
EthExit()
{
	// thread exit
	EthMan.bthWorkQuit = TRUE;

	BacpNetCtrlExit();

	BacpNetExit();

	NetSendCbElmManExit(&EthMan.NetSendCbElmMan);

	DpcRmMd(MXMDID_ETH);

#ifdef ETH_INIT_EXIT_DEBUG
	printf("Eth: Exit ...\n");
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EthWorkThreadFun
**	AUTHOR:			Jerry Huang
**	DATE:			13 - Oct - 2006
**
**	DESCRIPTION:	
**			Eth module work thread function
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				arg			[IN]		void*
**	RETURNED VALUE:	
**				0
**	NOTES:
**			
*/
void*
EthWorkThreadFun(void* arg)
{
	int		nRepeat = 0;
	while (!EthMan.bthWorkQuit)
	{
		for (nRepeat = 0; nRepeat < 20; nRepeat++)
		{
			// ready all to avoid data lost
			EthProcess();
			ProcessIcmp();
		}

		usleep(1 * 1000);
	}

	pthread_exit(0);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EthGetMsg
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			GetEth module message
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMsg		[OUT]		MXMSG*
**	RETURNED VALUE:	
**				True if get
**	NOTES:
**			
*/
static BOOL
EthGetMsg(MXMSG* pMsg)
{
	pMsg->dwDestMd = MXMDID_ETH;
	return MxGetMsg(pMsg);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EthFreeMXMSG
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Eth free the memory of MXMSG -> pParam
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
EthFreeMXMSG(MXMSG* pMsg)
{
	if (pMsg->pParam != NULL)
	{
#ifdef ETH_NET_SEND_DATA_CALLBACK_DEBUG
		printf("Eth: Free MXMSG %08X\n", (unsigned int) pMsg->pParam);
#endif
		MXFree(pMsg->pParam);
		pMsg->pParam = NULL;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EthAppProcess
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Eth application process
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
EthAppProcess()
{
	MXMSG			MsgGet;
	unsigned char	AppFrm[MAX_APP_FRM_LEN];
	int				nAppFrmLen;
	BacpAppDevCode	Dest;
	BacpAppDevCode	Src;
	//int				nCbId;
	unsigned char	nAck		= 0;		// Prepared
//	unsigned int	nInAddr;
	unsigned short	nDataLen	= 0;

	int nType = 0;

	if (EthGetMsg(&MsgGet))
	{
		EthMsgDebug(&MsgGet, FALSE);
#ifdef __SUPPORT_PROTOCOL_900_1A__
		if(SoapSendMessage(&MsgGet))
		{
			g_PackCnt.nSendPackCnt++;
			EthFreeMXMSG(&MsgGet);
			return;
		}
#endif

		TelLogInput(&MsgGet);
		
		switch (MsgGet.dwMsg)
		{
		case FC_CALL_GM_MC:
		case FC_CALL_GM_HV:
		case FC_CALL_GM_GM:
		case FC_ACK_CALL_HV_GM:
		case FC_ACK_PICKUP_MC:
		case FC_ACK_PICKUP_GM:
		case FC_ACK_PICKUP_HV:
		case FC_HANGUP_GM:
		case FC_PICKUP_GM:
		case FC_ACK_HANGUP_MC:
		case FC_ACK_HANGUP_GM:
		case FC_ACK_HANGUP_HV:
		case FC_ACK_CALL_MC_GM:
		case FC_ACK_CALL_GM_HV:
		case FC_ACK_CALL_GM_GM:
		case FC_PICKUP_HV:
		case FC_ACK_MNT_START:
		case FC_ACK_MNT_CANCEL:
		case FC_MNT_INTERRUPT:
		case FC_ACK_MNT_INTERRUPT:
		case FC_ACK_UNLK_GATE:
		case FC_ALM_REPORT:
		case FC_ALM_REPORT_MHV:
		case FC_ACK_IMSG_SEND:
		case FC_CALL_BROADCAST:
		case FC_PICKUP_NTY:
		case FC_CALL_STP_BROADCAST:
		case FC_HANGUP_NTY:
		case FC_IMSG_BRADCAST:
		case FC_IMSG_QUERY_LIST:
		case FC_ACK_IMSG_QUERY_LIST:
		case FC_IMSG_QUERY_MSG:
		case FC_ACK_IMSG_QUERY_MSG:
		case FC_IMSG_DELETE:
		case FC_ACK_IMSG_DELETE:
		case FC_IMSG_NOTIFY:
		case FC_IMSG_GETCOUNT:
		case FC_ACK_IMSG_GETCOUNT:
		
		case FC_AV_STREAM_ANNOUNCEMENT:
		case FC_ACK_AV_STREAM_ANNOUNCEMENT:
		case FC_AV_STREAM_REQUEST:
		case FC_ACK_AV_STREAM_REQUEST:
		case FC_AV_START:
		case FC_AV_STOP:
			//security alarm zone set
		case FC_SA_ZONE_QUERY:
		case FC_ACK_SA_ZONE_QUERY:
		case FC_SA_ZONE_SET:
		case FC_ACK_SA_ZONE_SET:
		case FC_SA_ALARM_NOTIFY:
		case FC_SA_ALARM_CONFIRM_NOTIFY:
			//sa log
		case FC_SAL_QUERY_LIST:
		case FC_SAL_QUERY_LOG:
		case FC_SAL_QUERY_DEL:
		case FC_SAL_QUERY_CNT:
		case FC_ACK_SAL_QUERY_CNT:
		case FC_ACK_SAL_QUERY_LIST:
		case FC_ACK_SAL_QUERY_LOG:
		case FC_ACK_SAL_QUERY_DEL:
		case FC_SAL_STATUS_NOTIFY:
		case FC_ACK_ALM_REPORT:
		case FC_SA_SET_NOTIFY:
		case FC_SA_SET_REPORT:
		case FC_QUERY_CHILDREN:
			//Talk log
		case FC_TL_QUERY_LIST:
		case FC_TL_QUERY_LOG:
		case FC_TL_QUERY_DEL:
		case FC_TL_QUERY_PIC:
		case FC_ACK_TL_SEND_PIC:
		case FC_ACK_TL_QUERY_LIST:
		case FC_ACK_TL_QUERY_LOG:
		case FC_ACK_TL_QUERY_DEL:
		case FC_ACK_TL_QUERY_CNT:
		case FC_TL_SEND_PIC:
		case FC_TL_REPORT_TO_MHV:
		case FC_ACK_TL_REPORT_MHV:
		case FC_HO_GET_COUNT:
		case FC_HO_GET:
		case FC_TL_QUERY_CNT:
		case FC_TL_STATUS_NOTIFY:
		//password 
		case FC_PSW_CHANGE:
		case FC_ACK_PSW_CHANGE:
		case FC_QUERY_HDB:
		case FC_ACK_QUERY_HDB:
		case FC_IMSG_SEND:	
		//Forward talk
		case FC_CALL_FORWARD:
		case FC_ACK_CALL_FORWARD:
		// sys info query
		case FC_GET_FUNCINFO:
		case FC_GET_SYSINFO:
		case FC_ACK_GET_SYSINFO:
		case FC_ACK_GET_FUNCINFO:
		case FC_REPORT_LOGDATA:
		// Lift control
		case FC_LF_A_B:
		case FC_LF_UNLOCK:
		case FC_ACK_LF_A_B:
		case FC_ACK_LF_UNLOCK:
		// Access
		case FC_AC_SWIPE_CARD:
		case FC_AC_PWD_CHECK:
		case FC_AC_PWD_MODIFY:
		case FC_AC_RSD_PWD:
		case FC_ACK_AC_SWIPE_CARD:
		case FC_ACK_AC_PWD_CHECK:
		case FC_ACK_AC_PWD_MODIFY:
		case FC_ACK_AC_RSD_PWD:
		case FC_AC_OPEN_GATE:
		case FC_ACK_AC_OPEN_GATE:
        case FC_AC_CARD_PASSWORD_MOD: 
        case FC_ACK_AC_CARD_PASSWORD_MOD:
			if ((FC_PICKUP_GM == MsgGet.dwMsg) ||
				(FC_CALL_BROADCAST == MsgGet.dwMsg) )
			{
				//nInAddr = htonl(*((unsigned int*) MsgGet.pParam));
				// MsgGet.pParam store self ip address
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg, 
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					(unsigned char*) MsgGet.pParam, sizeof (unsigned int), 
					AppFrm);
			}
			else if ( (FC_ACK_PICKUP_MC == MsgGet.dwMsg)	||
					  (FC_ACK_PICKUP_GM == MsgGet.dwMsg)	||
					  (FC_ACK_PICKUP_HV == MsgGet.dwMsg)	||
					  (FC_ACK_HANGUP_MC == MsgGet.dwMsg)	||
					  (FC_ACK_HANGUP_GM == MsgGet.dwMsg)	||
					  (FC_ACK_HANGUP_HV == MsgGet.dwMsg)	||
					  (FC_ACK_CALL_MC_HV == MsgGet.dwMsg)	||
					  (FC_ACK_CALL_GM_HV == MsgGet.dwMsg)	||
					  (FC_ACK_PSW_CHANGE == MsgGet.dwMsg)	||					  
					  (FC_ACK_CALL_HV_HV == MsgGet.dwMsg))
			{
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg, 
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					&nAck, 1, 
					AppFrm);
			}
			else if (
					FC_SA_SET_REPORT == MsgGet.dwMsg	||
					FC_SA_SET_NOTIFY == MsgGet.dwMsg)
			{
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg, 
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					MsgGet.pParam, 8, 
					AppFrm);
			}
			else if (FC_ALM_REPORT_MHV == MsgGet.dwMsg )
			{
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg, 
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					MsgGet.pParam, 9, 
					AppFrm);
			}
			else if ( FC_IMSG_QUERY_MSG	== MsgGet.dwMsg	 ||
					FC_SAL_QUERY_LOG	== MsgGet.dwMsg)
			{
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg, 
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					MsgGet.pParam, 2, 
					AppFrm);

			}
			else if (FC_IMSG_QUERY_LIST == MsgGet.dwMsg ||
					FC_IMSG_NOTIFY		== MsgGet.dwMsg	||
					FC_SAL_QUERY_LIST	== MsgGet.dwMsg ||
					FC_SAL_STATUS_NOTIFY ==MsgGet.dwMsg	||
					FC_SA_ALARM_CONFIRM_NOTIFY == MsgGet.dwMsg ||
					(FC_CALL_FORWARD == MsgGet.dwMsg)) 
					
			{
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg,  
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					MsgGet.pParam, 4, 
					AppFrm);
			}
			else if (FC_ACK_IMSG_QUERY_LIST == MsgGet.dwMsg ||
					 FC_ACK_IMSG_QUERY_MSG	== MsgGet.dwMsg	||
					 FC_ACK_IMSG_GETCOUNT	== MsgGet.dwMsg ||
					 FC_ACK_SAL_QUERY_LIST	== MsgGet.dwMsg	||
					 FC_ACK_SAL_QUERY_LOG	== MsgGet.dwMsg	||
					 FC_ACK_TL_QUERY_LIST	== MsgGet.dwMsg ||
					 FC_TL_SEND_PIC			== MsgGet.dwMsg ||
					 FC_ACK_SAL_QUERY_CNT	== MsgGet.dwMsg )
			{
				nDataLen = *((unsigned short*)MsgGet.pParam);
				// MsgGet.pParam store self ip address
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg,  
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					MsgGet.pParam, nDataLen, 
					AppFrm);
			}
			else if (FC_IMSG_DELETE		== MsgGet.dwMsg ||
					FC_SAL_QUERY_DEL	== MsgGet.dwMsg ||
					FC_TL_QUERY_LIST	== MsgGet.dwMsg ||
					FC_TL_QUERY_LOG		== MsgGet.dwMsg	||
					FC_TL_QUERY_DEL		== MsgGet.dwMsg ||
					FC_ALM_REPORT		== MsgGet.dwMsg ||
					FC_PSW_CHANGE		== MsgGet.dwMsg ||
					FC_ACK_TL_QUERY_LOG	== MsgGet.dwMsg ||	
					FC_TL_QUERY_PIC		== MsgGet.dwMsg ||
					FC_TL_REPORT_TO_MHV	== MsgGet.dwMsg ||
					FC_ACK_SA_ZONE_QUERY == MsgGet.dwMsg ||
					FC_SA_ZONE_SET		== MsgGet.dwMsg ||
					FC_ACK_TL_QUERY_CNT	== MsgGet.dwMsg ||
					FC_GET_FUNCINFO		== MsgGet.dwMsg ||
					FC_ACK_GET_SYSINFO	== MsgGet.dwMsg ||
					FC_ACK_GET_FUNCINFO	== MsgGet.dwMsg ||
					FC_IMSG_SEND		== MsgGet.dwMsg ||
					FC_LF_A_B			== MsgGet.dwMsg ||
					FC_LF_UNLOCK		== MsgGet.dwMsg ||
					FC_AV_STREAM_ANNOUNCEMENT	== MsgGet.dwMsg ||
					FC_AV_STREAM_REQUEST	== MsgGet.dwMsg ||
					FC_ACK_AV_STREAM_REQUEST	== MsgGet.dwMsg||
					FC_CALL_GM_GM== MsgGet.dwMsg ||
					FC_CALL_GM_MC== MsgGet.dwMsg ||
					FC_CALL_GM_HV== MsgGet.dwMsg ||
					FC_ACK_CALL_GM_GM== MsgGet.dwMsg ||
					FC_ACK_CALL_MC_GM== MsgGet.dwMsg ||
					FC_ACK_CALL_HV_GM== MsgGet.dwMsg ||
					FC_ACK_MNT_START  == MsgGet.dwMsg ||
					FC_AC_SWIPE_CARD== MsgGet.dwMsg ||
					FC_AC_PWD_CHECK== MsgGet.dwMsg ||
					FC_AC_PWD_MODIFY== MsgGet.dwMsg ||
					FC_AC_RSD_PWD  == MsgGet.dwMsg ||
					FC_AC_OPEN_GATE  == MsgGet.dwMsg ||
					FC_AV_START==MsgGet.dwMsg ||
					FC_AV_STOP==MsgGet.dwMsg ||
					FC_AC_CARD_PASSWORD_MOD == MsgGet.dwMsg )
			{
				nDataLen = *((unsigned short*)MsgGet.pParam);
				// MsgGet.pParam store self ip address
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg,  
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					&MsgGet.pParam[2], nDataLen, 
					AppFrm);
			}
			else if (FC_HO_GET_COUNT == MsgGet.dwMsg	||
					FC_QUERY_HDB		== MsgGet.dwMsg ||
					FC_ACK_MNT_CANCEL == MsgGet.dwMsg ||
					FC_ACK_MNT_INTERRUPT == MsgGet.dwMsg ||

					FC_ACK_CALL_FORWARD ==  MsgGet.dwMsg) 
			{
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg,  
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					MsgGet.pParam, 1, 
					AppFrm);
			}
			else if (FC_HO_GET == MsgGet.dwMsg)
			{
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg,  
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					MsgGet.pParam, 5, 
					AppFrm);
			}
			else if (	FC_ACK_UNLK_GATE == MsgGet.dwMsg ||
						FC_ACK_LF_A_B == MsgGet.dwMsg ||
						FC_ACK_LF_UNLOCK == MsgGet.dwMsg ||
						FC_ACK_AC_SWIPE_CARD == MsgGet.dwMsg ||
						FC_ACK_AC_PWD_CHECK == MsgGet.dwMsg ||
						FC_ACK_AC_PWD_MODIFY == MsgGet.dwMsg ||
						FC_ACK_AC_RSD_PWD == MsgGet.dwMsg ||
						FC_ACK_AC_OPEN_GATE == MsgGet.dwMsg ||
						FC_ACK_AC_CARD_PASSWORD_MOD == MsgGet.dwMsg)
			{
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg,  
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					MsgGet.pParam, 1, 
					AppFrm);
			}
			else if ( FC_REPORT_LOGDATA == MsgGet.dwMsg )
			{
//				printf("--------------Eth Rev: FC_REPORT_LOGDATA \n");
				memcpy(&nDataLen, MsgGet.pParam, sizeof(INT));
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg,  
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					MsgGet.pParam+sizeof(INT), nDataLen, 
					AppFrm);
			}
			else
			{
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg, 
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					NULL, 0, 
					AppFrm);

			}

			strcpy(Dest.szDev, MsgGet.szDestDev);
			strcpy(Src.szDev, MsgGet.szSrcDev);
			//nCbId = NetSendCbStoreMXMSG(&MsgGet, MsgGet.dwSrcMd);
			//SendAppFrm2Net(NET_OUTER_FLAG, NET_DATA_TYPE_CMD, &Src, &Dest, AppFrm, nAppFrmLen, nCbId);
			SendAppFrm2Net(NET_DATA_TYPE_CMD, MsgGet.szSrcDev, MsgGet.szDestDev, AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID, MsgGet.dwParam);
			g_PackCnt.nSendPackCnt++;
            
			break;
		case FC_ACK_AV_REQUEST_DECARGS:
			nDataLen = *((unsigned short*)MsgGet.pParam);
			nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg,  
				MsgGet.szSrcDev, MsgGet.szDestDev, 
				&MsgGet.pParam[2], nDataLen, 
				AppFrm);
			SendAppFrm2Net(NET_DATA_TYPE_VIDEO, MsgGet.szSrcDev, MsgGet.szDestDev, AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID, MsgGet.dwParam);
			break;
		case MXMSG_STA_RDIR_ETH_MM_A:
		case MXMSG_STA_MM2ETH_V:
		case MXMSG_STA_ETH2MM_A_MM2ETH_AV:
		case MXMSG_PLY_GM_V:
			// Need   dest device code
			EthMan.RdirAVEthMm.bRdirAV = TRUE;
			strcpy(EthMan.RdirAVEthMm.SrcDev, MsgGet.szSrcDev);
			strcpy(EthMan.RdirAVEthMm.DestDev, MsgGet.szDestDev);
			EthMan.RdirAVEthMm.nDestIPAddr = MsgGet.dwParam;
			break;
		case MXMSG_SEND_PHOTO_BMP:
		case MXMSG_SEND_PHOTO_JPG:
			// Need   dest device code
			EthMan.RdirAVEthMm.bRdirAV = TRUE;
			strcpy(EthMan.RdirAVEthMm.SrcDev, MsgGet.szSrcDev);
			strcpy(EthMan.RdirAVEthMm.DestDev, MsgGet.szDestDev);
			EthMan.RdirAVEthMm.nPhotoDestIPAddr= MsgGet.dwParam;
			break;
		case MXMSG_STP_RDIR_ETH_MM_AV:
		case MXMSG_STP_MM2ETH_V:
		case MXMSG_STP_ETH2MM_A_MM2ETH_AV:
		case MXMSG_STP_GM_V:
			EthMan.RdirAVEthMm.bRdirAV = FALSE;
			break;
			
		case FC_REPORT_PIC:
			{
				memcpy(&nDataLen, MsgGet.pParam, sizeof(unsigned short));
				
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg, 
						MsgGet.szSrcDev, MsgGet.szDestDev, 
						MsgGet.pParam + sizeof(unsigned short), nDataLen, 
						AppFrm);				
				
				SendAppFrm2Net(NET_DATA_TYPE_CMD, MsgGet.szSrcDev, MsgGet.szDestDev, AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID, MsgGet.dwParam);
			}
			break;

		case FC_AC_REPORT_EVENT:
			{
				memcpy(&nDataLen, MsgGet.pParam, sizeof(UINT));
				
				nAppFrmLen = PckBacpApp( (unsigned short) MsgGet.dwMsg, 
					MsgGet.szSrcDev, MsgGet.szDestDev, 
					MsgGet.pParam + sizeof(UINT), nDataLen, 
					AppFrm);				
				
				SendAppFrm2Net(NET_DATA_TYPE_CMD, MsgGet.szSrcDev, MsgGet.szDestDev, AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID, MsgGet.dwParam);
				
			}
			break;

		case FC_ACK_GET_CONFINFO:
			{
#ifdef __SUPPORT_WIEGAND_CARD_READER__			
				memcpy(&nDataLen, MsgGet.pParam, sizeof(USHORT));
				nAppFrmLen = PckBacpApp((unsigned short)MsgGet.dwMsg, MsgGet.szSrcDev, MsgGet.szDestDev,
														MsgGet.pParam + sizeof(USHORT), nDataLen, AppFrm);
				SendAppFrm2Net(NET_DATA_TYPE_CMD, MsgGet.szSrcDev, MsgGet.szDestDev, AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID, MsgGet.dwParam);
#endif				
				break;


		}

#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
		case FC_ACK_AC_GET_CARD_CNT:
		case FC_ACK_AC_GET_CARD:
		case FC_ACK_AC_DEL_CARD:
		case FC_ACK_AC_LAST_SWIPE_V1:
		case FC_ACK_AC_ADD_CARD:
		case FC_ACK_AC_EDIT_CARD:
		case FC_ACK_AC_EVENT_READ:
		case FC_ACK_AC_EVENT_CNT:
        case FC_ACK_AC_GET_CARD_V2:
			nAppFrmLen = PckBacpApp((unsigned short)MsgGet.dwMsg, MsgGet.szSrcDev, MsgGet.szDestDev,	MsgGet.pParam, MsgGet.wDataLen, AppFrm);
			SendAppFrm2Net(NET_DATA_TYPE_CMD, MsgGet.szSrcDev, MsgGet.szDestDev, AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID, MsgGet.dwParam);
			break;
#endif		
		default:
			break;

		}

		EthFreeMXMSG(&MsgGet);
	}
	//查询MC是否在线的状态
//	if (g_bReqMC)//Judge whether MC is online
//	{
//		g_bReqMC	=	FALSE;
//		if (!g_TalkInfo.talking.dwTalkState) 
//		{
//			CheckMCStatus();
//		}
//	}

}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DoAppFrm
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Process received application data
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nFunc		[IN]		int
**				pAppFrm		[IN]		unsigned char*
**				nAppFrmLen	[IN]		int
**				nSrcIPAddr	[IN]		unsigned int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
MXSendCMD2Eth(unsigned short nFunCode, unsigned char* pBuf, int nBufLen,unsigned int nDestIPAddr,char* pDestDev)
{
	unsigned char	AppFrm[MAX_APP_FRM_LEN];
	int				nAppFrmLen=0,i;	
	
	nAppFrmLen = PckBacpApp(	nFunCode, 
				EthMan.RdirAVEthMm.SrcDev, 
				pDestDev, 
				pBuf, nBufLen, AppFrm);
				
	SendAppFrm2Net(	NET_DATA_TYPE_VIDEO, 
				EthMan.RdirAVEthMm.SrcDev, 
				pDestDev, 
				AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID,
				nDestIPAddr);			
}



void
DoAppFrm(int nFunc, unsigned char* pAppFrm, int nAppFrmLen, unsigned int nSrcIPAddr)
{
	MXMSG					MsgSend;
	unsigned char	AppSendFrm[MAX_APP_FRM_LEN] =  {0};
	int				nAppSendFrmLen = 0;
	unsigned char	nAck	= 0;		// Prepared
	unsigned short			nFunCode = 0;
	char*					pSrcDev = NULL;
	char*					pDestDev = NULL;
	unsigned char*			pData =	NULL;
	int						nDataLen = 0;
	HASH_VR					VersionResult;
	
	char					ntempt = 0;
	time_t	tmCurtime = 0;
	
	UnpckBacpAppEx(&nFunCode, &pSrcDev, &pDestDev, &pData, &nDataLen, pAppFrm, nAppFrmLen);
	MsgSend.wDataLen=nDataLen;
	//printf("nFunCode=0x%08x\n",nFunCode);
	//printf("nAppFrmLen=%d\n",MsgSend.wDataLen);
	
	g_PackCnt.nRecvPackCnt++;
	if(ConvertGPCMD(nFunCode,pData, nDataLen,nSrcIPAddr))
	{
		return;
	}

	switch (nFunCode)
	{
	case FC_PICKUP_MC:
	case FC_PICKUP_GM:
	case FC_ACK_HANGUP_GM:
	case FC_ACK_PICKUP_GM:
	case FC_HANGUP_MC:
	case FC_HANGUP_GM:
	case FC_HANGUP_HV:
	case FC_MNT_CANCEL:
	case FC_MNT_INTERRUPT:
	case FC_ACK_MNT_INTERRUPT:
	case FC_UNLK_GATE:
	case FC_CALL_BROADCAST:
	case FC_PICKUP_NTY:
	case FC_CALL_STP_BROADCAST:
	case FC_HANGUP_NTY:
	case FC_REDIRECT_MC:
	case FC_AV_TAKEPHOTO:		
		//HDS
	case FC_ACK_QUERY_HDB:
		//talk log
	case FC_ACK_TL_QUERY_DEL:
	case FC_ACK_TL_REPORT_MHV:
	case FC_TL_QUERY_CNT:
	case FC_TL_STATUS_NOTIFY:
	
	case FC_ACK_AV_STREAM_ANNOUNCEMENT:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_TALKING;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		if ( (FC_PICKUP_MC == nFunCode)	|| 
			 //(FC_PICKUP_GM == nFunCode)	|| 
			// (FC_PICKUP_HV == nFunCode) ||
			 (FC_REDIRECT_MC == nFunCode) ||
			(FC_CALL_BROADCAST == nFunCode) 
			) 
		{
			memcpy(&MsgSend.dwParam, pData, sizeof (unsigned int));
			MsgSend.dwParam = ntohl((UINT)MsgSend.dwParam);
		}
		else
		{
			MsgSend.dwParam		= nSrcIPAddr;
		}
		MsgSend.pParam		= NULL;
		MxPutMsg(&MsgSend);
		EthMsgDebug(&MsgSend, TRUE);
		
		break;
	case FC_ACK_QUERY_CHILDREN:
	case FC_TL_QUERY_LIST:
	case FC_TL_QUERY_LOG:
	case FC_TL_QUERY_DEL:
	case FC_TL_QUERY_PIC:
	case FC_ACK_TL_SEND_PIC:
	case FC_ACK_TL_QUERY_LIST:
	case FC_ACK_TL_QUERY_LOG:
	case FC_TL_SEND_PIC:
	case FC_TL_REPORT_TO_MHV:
	case FC_ACK_HO_GET_COUNT:
	case FC_ACK_HO_GET:
	case FC_QUERY_HDB:
	case FC_ACK_CALL_FORWARD:
	case FC_ACK_TL_QUERY_CNT:
		//Forward talk
	case FC_CALL_FORWARD:
	
	case FC_AV_STREAM_ANNOUNCEMENT:
	case FC_AV_STREAM_REQUEST:
	case FC_ACK_AV_STREAM_REQUEST:
	
	case FC_ACK_CALL_GM_MC:
	case FC_ACK_CALL_GM_HV:
	case FC_ACK_CALL_GM_GM:
	case FC_CALL_MC_GM:
	case FC_CALL_GM_GM:
	case FC_CALL_HV_GM:
	case FC_MNT_START:
	case FC_PICKUP_HV:
	
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_TALKING;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;
		if (FC_PICKUP_HV == nFunCode)
		{
			memcpy(&MsgSend.dwParam, pData, sizeof (unsigned int));
			MsgSend.dwParam = ntohl((UINT)MsgSend.dwParam);
		}
		if(nDataLen)
		{
			MsgSend.pParam		= (unsigned char *) malloc(nDataLen);
			memcpy((unsigned char *)MsgSend.pParam, pData, nDataLen);
		}
		else
		{
			MsgSend.pParam=NULL;
		}
		MxPutMsg(&MsgSend);
		break;
    case FC_UNLOCK_NOTIFY_GM:
        {
            strcpy(MsgSend.szSrcDev, pSrcDev);
			strcpy(MsgSend.szDestDev, pDestDev);
			MsgSend.dwDestMd	= MXMDID_ACC;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.dwParam		= nSrcIPAddr;				
			MsgSend.pParam		= (BYTE*)malloc(nDataLen);

			memcpy(MsgSend.pParam, pData, nDataLen);

			MxPutMsg(&MsgSend);
            break;
        }
	case FC_GET_FUNCINFO:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_PUBLIC;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;
		
		MsgSend.pParam		= (unsigned char *) malloc(nDataLen);
		memcpy((unsigned char *)MsgSend.pParam, pData, nDataLen);
		
		MxPutMsg(&MsgSend);		
		break;

	case FC_ACK_GET_SYSINFO:
	case FC_ACK_GET_FUNCINFO:
		
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_PUBLIC;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;
		
		MsgSend.pParam		= (unsigned char *) malloc(nDataLen + 2);
		memcpy((unsigned char *)MsgSend.pParam, &nDataLen, 2);
		memcpy((unsigned char *)&MsgSend.pParam[2], pData, nDataLen);
//		printf("msg=%08x.the ndatale=%d.\n", nFunCode, nDataLen);
		MxPutMsg(&MsgSend);
		break;
	case FC_GET_SYSINFO:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_PUBLIC;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;
		
		MsgSend.pParam		= NULL;
		MxPutMsg(&MsgSend);		
		break;
	case FC_ACK_AV_TAKEPHOTO:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_TALKING;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;
		
		MsgSend.pParam		= (unsigned char *) malloc(nDataLen + 2);
		memcpy((unsigned char *)MsgSend.pParam, &nDataLen, 2);
		memcpy((unsigned char *)&MsgSend.pParam[2], pData, nDataLen);
		
		MxPutMsg(&MsgSend);
		break;
	case FC_AV_SENDDATA:
		if (NET_DATA_TYPE_VIDEO == nFunc)
		{
			//if (EthMan.RdirAVEthMm.bRdirAV)	
			{
				// Send Video to MultiMedia module
				// to prevent the other module sending no-use packet.
				if (EthMan.RdirAVEthMm.nDestIPAddr == nSrcIPAddr)// 
				{
					MxSendAV2Mm(AV2MM_VIDEO, pData, nDataLen);
				}

			}
		}
		else if (NET_DATA_TYPE_AUDIO == nFunc)
		{
			//if (EthMan.RdirAVEthMm.bRdirAV)
			{
				// Send Audio to MultiMedia module
				// to prevent the other module sending no-use packet.
				if (EthMan.RdirAVEthMm.nDestIPAddr == nSrcIPAddr)
				{
					/*static DWORD dwPreTickCount = 0;
					static DWORD dwErrCount = 0;
					printf("Audio rcv Eth:GetTickCount()-dwPreTickCount=%d\n", GetTickCount()-dwPreTickCount);
					if ((GetTickCount()-dwPreTickCount)>50) 
					{
						dwErrCount++;
						printf("Audio rcv Error:dwErrCount=%d\n", dwErrCount);
					}
					dwPreTickCount = GetTickCount();
					*/
					MxSendAV2Mm(AV2MM_AUDIO, pData, nDataLen);
				}
			}
		}
		break;
		
	case FC_DTM_ADJUST://adjust the EthHV date and time.//the  pDate from the year
		if(GetFocus() == g_WndMan.MainWndHdle)
		{
			SetRtcTime(pData);
			ResetTimer(g_WndMan.MainWndHdle, 101, 1000, NULL);//for bug 11634
		}
		else
		{
			tmCurtime=GetRtcTime();
			strcpy(MsgSend.szSrcDev, pSrcDev);
			strcpy(MsgSend.szDestDev, pDestDev);
			MsgSend.dwDestMd	= MXMDID_RTC;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;
			MsgSend.dwParam		= nSrcIPAddr;
			MsgSend.pParam		= (unsigned char *) malloc(nDataLen+sizeof(tmCurtime));

			printf("tmCurtime : %ld \r\n",tmCurtime);
			memcpy((unsigned char *)MsgSend.pParam, pData, nDataLen);
			MsgSend.pParam[nDataLen]=(unsigned char)(tmCurtime&0x000000ff);
			MsgSend.pParam[nDataLen+1]=(unsigned char)(tmCurtime>>8&0x000000ff);
			MsgSend.pParam[nDataLen+1+1]=(unsigned char)(tmCurtime>>16&0x000000ff);
			MsgSend.pParam[nDataLen+1+1+1]=(unsigned char)(tmCurtime>>24&0x000000ff);
			tmCurtime=MsgSend.pParam[6]|MsgSend.pParam[7]<<8|MsgSend.pParam[8]<<16|MsgSend.pParam[9]<<24;
			printf("tmCurtime32 to 8 : %ld \r\n",tmCurtime);
			MxPutMsg(&MsgSend);
		}
		break;

	case FC_STATE_DIAGNOSE://The state_query from the MC to prevent the off-line.
		{
			nAppSendFrmLen = PckBacpApp(FC_ACK_STATE_DIAGNOSE , 
				pDestDev, pSrcDev, 
				&nAck, 1, 
				AppSendFrm);			

			SendAppFrm2Net(NET_DATA_TYPE_CMD, pDestDev, pSrcDev, AppSendFrm, nAppSendFrmLen, INVALID_SEND_CALLBACK_ID, nSrcIPAddr);
//			g_TalkInfo.dwMCIP = nSrcIPAddr;
			
			if (!g_TalkInfo.bMCStatus)
			{
				g_TalkInfo.bMCStatus = TRUE;
				NetCtrlUpdateTime();
			}
			
		}
		break;

	case FC_ACK_ALM_REPORT:
	case FC_SA_ZONE_QUERY:
	case FC_ACK_SA_ZONE_SET:
	case FC_SA_ALARM_NOTIFY:

		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_SA;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;
		MsgSend.pParam		= NULL;
		MxPutMsg(&MsgSend);
		
		EthMsgDebug(&MsgSend, TRUE);
		break;
	case FC_SAL_QUERY_LIST:
	case FC_SAL_QUERY_LOG:
	case FC_SAL_QUERY_DEL:
	case FC_ACK_SAL_QUERY_LIST:
	case FC_ACK_SAL_QUERY_LOG:
	case FC_ACK_SAL_QUERY_CNT:
	case FC_SAL_STATUS_NOTIFY:
	case FC_ALM_REPORT_MHV:
	case FC_SA_SET_REPORT:
	case FC_SA_SET_NOTIFY:
	case FC_ACK_SA_ZONE_QUERY:
	case FC_SA_ZONE_SET:
	case FC_SA_ALARM_CONFIRM_NOTIFY:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_SA;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;	
		MsgSend.pParam		= (unsigned char *) malloc(nDataLen);
		memcpy((unsigned char *)MsgSend.pParam, pData, nDataLen);
		MxPutMsg(&MsgSend);	
		
		break;
	case FC_ACK_SAL_QUERY_DEL:
	case FC_SAL_QUERY_CNT:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_SA;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;	
		MsgSend.pParam		= NULL;
		MxPutMsg(&MsgSend);	
	case FC_IMSG_BRADCAST:
		break;
	case FC_IMSG_SEND:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_SMG;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;
		
		MsgSend.pParam		= (unsigned char *) malloc(nDataLen + 4);
		memcpy((unsigned char *)MsgSend.pParam, &nDataLen, 4);
		memcpy((unsigned char *)&MsgSend.pParam[4], pData, nDataLen);
		MxPutMsg(&MsgSend);
		break;
	case FC_ACK_IMSG_QUERY_LIST:
	case FC_IMSG_QUERY_LIST:
	case FC_IMSG_QUERY_MSG:
	case FC_ACK_IMSG_QUERY_MSG:
	case FC_IMSG_DELETE:
	case FC_IMSG_NOTIFY:
	case FC_ACK_IMSG_GETCOUNT:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_SMG;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;

		MsgSend.pParam		= (unsigned char *) malloc(nDataLen);
		memcpy((unsigned char *)MsgSend.pParam, pData, nDataLen);
		MxPutMsg(&MsgSend);
		break;
	case FC_ACK_IMSG_DELETE:
	case FC_IMSG_GETCOUNT:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_SMG;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;
		MsgSend.pParam		= NULL;
		MxPutMsg(&MsgSend);		
		break;
	case FC_PSW_CHANGE:
	case FC_ACK_PSW_CHANGE:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		MsgSend.dwDestMd	= MXMDID_PSW;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= nFunCode;
		MsgSend.dwParam		= nSrcIPAddr;
		
		MsgSend.pParam		= (unsigned char *) malloc(nDataLen);
		memcpy((unsigned char *)MsgSend.pParam, pData, nDataLen);
		MxPutMsg(&MsgSend);

		break;
	case FC_ACK_REPORT_LOGDATA:
		strcpy(MsgSend.szSrcDev, pSrcDev);
		strcpy(MsgSend.szDestDev, pDestDev);
		memcpy(&g_LogReference, pData, nDataLen-1);
		
		if (pData[nDataLen-1] && g_TLInfo.bHavePic) 
		{
			MsgSend.pParam		= NULL;
			MsgSend.dwDestMd	= MXMDID_MM;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= MXMSG_SEND_PHOTO_JPG;	
		
			MxPutMsg(&MsgSend);
		
			MsgSend.dwDestMd	= MXMDID_ETH;
			MsgSend.dwMsg		= MXMSG_SEND_PHOTO_JPG;
			MsgSend.dwParam		= nSrcIPAddr;
			MxPutMsg(&MsgSend);
		}
		else
		{
			strcpy(MsgSend.szSrcDev, pSrcDev);
			strcpy(MsgSend.szDestDev, pDestDev);
			
			MsgSend.dwDestMd	= MXMDID_TALKING;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.pParam		= NULL;
			
			MxPutMsg(&MsgSend);
		}
		break;

	case FC_AV_REQUEST_VIDEO_IVOP:
		{
			strcpy(MsgSend.szSrcDev, pSrcDev);
			strcpy(MsgSend.szDestDev, pDestDev);
			
			MsgSend.dwDestMd	= MXMDID_MM;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.pParam		= NULL;
			
			MxPutMsg(&MsgSend);
//			printf("Eth:FC_AV_REQUEST_VIDEO_IVOP\n");				
		}
		break;

	case FC_AV_REQUEST_DECARGS:
		{
			if(DecInitFlag)
			{
				printf("Send FC_ACK_AV_REQUEST_DECARGS!\n");
				MTSendDecargsAckCMD();
			}
			/*if (1 == pData[0]) 
			{
				pData[0] = 1;//Video
				
				
				if(MM_VIDEO_FORMAT_h264==GetTalkLocStreamFormat())
				{
					pData[1] = 1;//H.264
				}
				else
				{
					pData[1] = 2;//MPEG4
				}
			
				memcpy(pData+2, &nVideoHeaderLen, sizeof(SHORT));
				memcpy(pData+4, &ucVideoFrameHeader, nVideoHeaderLen);
				
				nAppSendFrmLen = PckBacpApp( FC_ACK_AV_REQUEST_DECARGS,  
					pDestDev, pSrcDev,
					pData, nVideoHeaderLen + 4, 
					AppSendFrm);		
				
				SendAppFrm2Net(NET_DATA_TYPE_VIDEO, pDestDev, pSrcDev, AppSendFrm, nAppSendFrmLen, INVALID_SEND_CALLBACK_ID, nSrcIPAddr);

			}*/
			printf("Eth:FC_AV_REQUEST_DECARGS\n");
		}
		break;

	case FC_AC_MC_ADD_ONE_CARD:
	case FC_PC_MC_ADD_ONE_CARD:
		{
			if (DwnldOneCardProc(pData, nDataLen ,nFunCode)) 
			{	
				
				//SaveOrSynchronousCardHashInfo2Mem();
				nFunCode += 0x8000;
				VersionResult.hash_Version = 1;
				VersionResult.hash_Result = 0;
				nAppSendFrmLen = PckBacpApp(nFunCode , 
					pDestDev, pSrcDev, 
					&VersionResult, 2, 
					AppSendFrm);
			}else
			{
				nFunCode += 0x8000;
				VersionResult.hash_Version = 1;
				VersionResult.hash_Result = 1;
				nAppSendFrmLen = PckBacpApp(nFunCode , 
					pDestDev, pSrcDev, 
					&VersionResult, 2,  
					AppSendFrm);
			}
			SendAppFrm2Net(NET_DATA_TYPE_CMD, pDestDev, pSrcDev, AppSendFrm, nAppSendFrmLen, INVALID_SEND_CALLBACK_ID, nSrcIPAddr); 
	
			
		}
		break;

	case FC_AC_MC_DEL_ONE_CARD:
	case FC_PC_MC_DEL_ONE_CARD:
		{	
			if(DeleteOneCardProc(pData, nDataLen ,nFunCode))
			{
				//SaveOrSynchronousCardHashInfo2Mem();
				nFunCode += 0x8000;
				VersionResult.hash_Version = 1;
				VersionResult.hash_Result = 0;
				nAppSendFrmLen = PckBacpApp(nFunCode , 
					pDestDev, pSrcDev, 
					&VersionResult, 2, 
					AppSendFrm);
			}else
			{
				nFunCode += 0x8000;
				VersionResult.hash_Version = 1;
				VersionResult.hash_Result = 1;
				nAppSendFrmLen = PckBacpApp(nFunCode , 
					pDestDev, pSrcDev, 
					&VersionResult, 2, 
					AppSendFrm);
			}
			SendAppFrm2Net(NET_DATA_TYPE_CMD, pDestDev, pSrcDev, AppSendFrm, nAppSendFrmLen, INVALID_SEND_CALLBACK_ID, nSrcIPAddr); 
			
		}
		break;

	case FC_AC_MC_EDIT_ONE_CARD:
	case FC_PC_MC_EDIT_ONE_CARD:
		{
			if(EditOneCardProc(pData, nDataLen ,nFunCode))
				{	
					//SaveOrSynchronousCardHashInfo2Mem();
					nFunCode += 0x8000;
					VersionResult.hash_Version = 1;
					VersionResult.hash_Result = 0;
					nAppSendFrmLen = PckBacpApp(nFunCode , 
						pDestDev, pSrcDev, 
						&VersionResult, 2, 
						AppSendFrm);
				}else
				{	
					nFunCode += 0x8000;
					VersionResult.hash_Version = 1;
					VersionResult.hash_Result = 1;
					nAppSendFrmLen = PckBacpApp(nFunCode , 
						pDestDev, pSrcDev, 
						&VersionResult, 2, 
						AppSendFrm);
				}
				SendAppFrm2Net(NET_DATA_TYPE_CMD, pDestDev, pSrcDev, AppSendFrm, nAppSendFrmLen, INVALID_SEND_CALLBACK_ID, nSrcIPAddr); 				
		}
		break;


	case FC_AC_MC_QUERY_CARD:
	case FC_PC_MC_QUERY_CARD:
		{	
			nDataLen	=	UpldOneCardProc(pData ,nFunCode);
			nFunCode += 0x8000;
			nAppSendFrmLen = PckBacpApp(nFunCode , 
				pDestDev, pSrcDev, 
				pData, nDataLen, 
				AppSendFrm);
			SendAppFrm2Net(NET_DATA_TYPE_CMD, pDestDev, pSrcDev, AppSendFrm, nAppSendFrmLen, INVALID_SEND_CALLBACK_ID, nSrcIPAddr);	
		}
		break;

		
///////////////////////////////////////		
	case FC_CNF_DWNLD_FILE://
		{
			if (DwnldFileProc(pData, nDataLen)) 
			{
				//*(pData + 3) =0;
				nAppSendFrmLen = PckBacpApp(FC_ACK_CNF_DWNLD_FILE , 
					pDestDev, pSrcDev, 
					NULL,0, 
					AppSendFrm);
				SendAppFrm2Net(NET_DATA_TYPE_CMD, pDestDev, pSrcDev, AppSendFrm, nAppSendFrmLen, INVALID_SEND_CALLBACK_ID, nSrcIPAddr);	
				if(LC_thread_state()== FALSE)
				{
					usleep(20*1000);
					system("reboot");	
					while (1);
				}
			}			
		}
		break;
		
	case FC_CNF_UPLD_FILE:
		{
			nDataLen	=	UpldFileProc(pData);

			if(-1 == nDataLen)
				break;
			
			nAppSendFrmLen = PckBacpApp(FC_ACK_CNF_UPLD_FILE , 
				pDestDev, pSrcDev, 
				pData, nDataLen, 
				AppSendFrm);
			SendAppFrm2Net(NET_DATA_TYPE_CMD, pDestDev, pSrcDev, AppSendFrm, nAppSendFrmLen, INVALID_SEND_CALLBACK_ID, nSrcIPAddr);	
		}
		break;		

	case FC_CNF_REMOVE_FILE:
		{
			printf("-----Eth Rev FC_CNF_REMOVE_FILE\n");
			if (RemoveFileProc(pData)) 
			{
				printf("-----FC_CNF_REMOVE_FILE OK \n");
				ntempt = 0;
				nAppSendFrmLen = PckBacpApp(FC_ACK_CNF_REMOVE_FILE , 
					pDestDev, pSrcDev, 
					&ntempt, 1, 
					AppSendFrm);
			}
			else
			{
				printf("-----FC_CNF_REMOVE_FILE FAIL\n");
				ntempt = 1;
				nAppSendFrmLen = PckBacpApp(FC_ACK_CNF_REMOVE_FILE , 
					pDestDev, pSrcDev, 
					&ntempt, 1, 
					AppSendFrm);
			}

			printf("-----FC_CNF_REMOVE_FILE Set ACK\n");

			SendAppFrm2Net(NET_DATA_TYPE_CMD, pDestDev, pSrcDev, AppSendFrm, nAppSendFrmLen, INVALID_SEND_CALLBACK_ID, nSrcIPAddr);			
		}
		break;

	case FC_ACK_AC_REPORT_EVENT:
#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
	case FC_AC_GET_CARD_CNT:
	case FC_AC_GET_CARD:
	case FC_AC_DEL_CARD:
	case FC_AC_LAST_SWIPE_V1:
	case FC_AC_ADD_CARD:
	case FC_AC_EDIT_CARD:
	case FC_AC_EVENT_CNT:
	case FC_AC_EVENT_READ:
    case FC_AC_GET_CARD_V2:
#endif
		{
			strcpy(MsgSend.szSrcDev, pSrcDev);
			strcpy(MsgSend.szDestDev, pDestDev);
			
			MsgSend.dwDestMd	= MXMDID_ACC;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.dwParam		= nSrcIPAddr;				
			MsgSend.pParam		= (BYTE*)malloc(nDataLen);

			memcpy(MsgSend.pParam, pData, nDataLen);

			MxPutMsg(&MsgSend);
		}
		break;
	case FC_LF_STOP_UP:
	case FC_LF_STOP_DOWN:
	{
			MsgSend.dwDestMd	= MXMDID_LCA;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.dwParam		= nSrcIPAddr;	
			strcpy(MsgSend.szSrcDev, pSrcDev);
			strcpy(MsgSend.szDestDev, pDestDev);
			//printf("MsgSend.szSrcDev=%s\n",MsgSend.szSrcDev);
			//printf("MsgSend.szDestDev=%s\n",MsgSend.szDestDev);
			
			nDataLen=4;		
			MsgSend.pParam		= (BYTE*)malloc(sizeof(int) + nDataLen);

			memcpy(MsgSend.pParam, &nDataLen, sizeof(int));
			memcpy(MsgSend.pParam + sizeof(int), &nSrcIPAddr, nDataLen);

			MxPutMsg(&MsgSend);	

			break;
		}	
	case FC_LF_STOP:
	case FC_LF_A_B:
	case FC_LF_UNLOCK:
		{
			MsgSend.dwDestMd	= MXMDID_LCA;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.dwParam		= nSrcIPAddr;	
			strcpy(MsgSend.szSrcDev, pSrcDev);
			strcpy(MsgSend.szDestDev, pDestDev);
//printf("RX nSrcIPAddr=0x%08x\n",nSrcIPAddr);			
			MsgSend.pParam		= (BYTE*)malloc(sizeof(int) + nDataLen);
			memcpy(MsgSend.pParam, &nDataLen, sizeof(int));
			memcpy(MsgSend.pParam + sizeof(int), pData, nDataLen);

			MxPutMsg(&MsgSend);	

			break;
		}		
	case FC_ACK_LF_A_B:
	case FC_ACK_LF_UNLOCK:
		{
			MsgSend.dwDestMd	= MXMDID_LC;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.dwParam		= nSrcIPAddr;				
			MsgSend.pParam		= (BYTE*)malloc(nDataLen);
			
			memcpy(MsgSend.pParam, pData, nDataLen);
			
			MxPutMsg(&MsgSend);	
			break;
		}
	case FC_AC_SWIPE_CARD:
	case FC_AC_PWD_CHECK:
	case FC_AC_PWD_MODIFY:
	case FC_AC_RSD_PWD:
		{
			MsgSend.dwDestMd	= MXMDID_ACA;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.dwParam		= nSrcIPAddr;				
			MsgSend.pParam		= (BYTE*)malloc(sizeof(int) + nDataLen);
			
			memcpy(MsgSend.pParam, &nDataLen, sizeof(int));
			memcpy(MsgSend.pParam + sizeof(int), pData, nDataLen);
			MxPutMsg(&MsgSend);	
			
			break;
		}
    case FC_AC_CARD_PASSWORD_MOD:
        {
            MsgSend.dwDestMd	= MXMDID_ACA;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.dwParam		= nSrcIPAddr;				
			MsgSend.pParam		= (BYTE*)malloc(sizeof(short) + nDataLen);
			
			memcpy(MsgSend.pParam, &nDataLen, sizeof(short));
			memcpy(MsgSend.pParam + sizeof(short), pData, nDataLen);
			
			MxPutMsg(&MsgSend);	
			
            break;
        }
	case FC_ACK_AC_SWIPE_CARD:
	case FC_ACK_AC_PWD_CHECK:
	case FC_ACK_AC_PWD_MODIFY:
	case FC_ACK_AC_RSD_PWD:
    case FC_ACK_AC_CARD_PASSWORD_MOD:
		{
			MsgSend.dwDestMd	= MXMDID_ACC;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.dwParam		= nSrcIPAddr;				
			MsgSend.pParam		= (BYTE*)malloc(nDataLen);
			
			memcpy(MsgSend.pParam, pData, nDataLen);
			
			MxPutMsg(&MsgSend);	
			break;
		}
	case FC_AC_OPEN_GATE:
		{
			strcpy(MsgSend.szSrcDev, pSrcDev);
			strcpy(MsgSend.szDestDev, pDestDev);
			
			MsgSend.dwDestMd	= MXMDID_ACA;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.dwParam		= nSrcIPAddr;				
			MsgSend.pParam		= (BYTE*)malloc(sizeof(int) + nDataLen);
			
			memcpy(MsgSend.pParam, &nDataLen, sizeof(int));
			memcpy(MsgSend.pParam + sizeof(int), pData, nDataLen);
			
			MxPutMsg(&MsgSend);
			break;
		}		
	case FC_ACK_AC_OPEN_GATE:
		{
			MsgSend.dwDestMd	= MXMDID_ACC;
			MsgSend.dwSrcMd		= MXMDID_ETH;
			MsgSend.dwMsg		= nFunCode;	
			MsgSend.dwParam		= nSrcIPAddr;				
			MsgSend.pParam		= (BYTE*)malloc(nDataLen);
			
			memcpy(MsgSend.pParam, pData, nDataLen);
			
			MxPutMsg(&MsgSend);	
			break;
		}
#ifdef __SUPPORT_WIEGAND_CARD_READER__
    case FC_GET_CONFINFO:
        {
            printf("FC_GET_CONFINFO\n");
			SendConfInfoEth(pData,nSrcIPAddr);
            break;
        }
#endif
	default:
		g_PackCnt.nRecvPackCnt--;
		break;
	}
	TelLogInput(&MsgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DoNetSendCb
**	AUTHOR:			Jerry Huang
**	DATE:			21 - Oct - 2006
**
**	DESCRIPTION:	
**			Process net send ok fail callback function
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nCbId		[IN]		int
**				bSuccess	[IN]		BOOL
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
DoNetSendCb(int nCbId, BOOL bSuccess)
{
	unsigned char*	pBuf;
	DWORD			dwDestMdId;
	MXMSG			MsgSend;
	
	if (nCbId != INVALID_SEND_CALLBACK_ID)
	{
		if (NetSendCbElmGet(&EthMan.NetSendCbElmMan, nCbId, &pBuf, &dwDestMdId))
		{
			memset(&MsgSend, 0, sizeof (MXMSG));
			MsgSend.dwDestMd = dwDestMdId;
			MsgSend.dwSrcMd = MXMDID_ETH;
			MsgSend.dwMsg	= MXMSG_ETH_SENDDATA_NTY;
			if (bSuccess)
			{
				MsgSend.dwParam = 1;
			}
			else 
			{
				MsgSend.dwParam = 0;
			}
			MsgSend.pParam = pBuf;

			MxPutMsg(&MsgSend);

			EthMsgDebug(&MsgSend, TRUE);
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MxSendAV2Eth
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Multi-Media or other module send A/V to ethernet
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nType		[IN]		int		1 Audio; 2 Video
**				pBuf		[IN]		unsigned char*
**				nBufLen		[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**		15 - Nov - 2006: fix bug of packet app frame 	
*/
void
MxSendAV2Eth(int nType, unsigned char* pBuf, int nBufLen)
{
	unsigned char	AppFrm[MAX_APP_FRM_LEN];
	int				nAppFrmLen=0;	
	if(((2 == nType)|| (3 == nType)) && (TRUE==IsUseGPVideo()))
	{
		printf("Use GP Video \n");
		return;
	}
	if (EthMan.RdirAVEthMm.bRdirAV)	
	{
		if ((1 == nType) || (2 == nType))
		{
			nAppFrmLen = PckBacpApp(	FC_AV_SENDDATA, 
				EthMan.RdirAVEthMm.SrcDev, 
				EthMan.RdirAVEthMm.DestDev, 
				pBuf, nBufLen, AppFrm);
		}
		else if (3 == nType)
		{
			nAppFrmLen = PckBacpApp(	FC_ACK_AV_TAKEPHOTO, 
				EthMan.RdirAVEthMm.SrcDev, 
				EthMan.RdirAVEthMm.DestDev, 
				pBuf, nBufLen, AppFrm);
		}
		else if (4 == nType)
		{
			nAppFrmLen = PckBacpApp( FC_REPORT_PIC, 
			EthMan.RdirAVEthMm.SrcDev,
			EthMan.RdirAVEthMm.DestDev, 
				pBuf, nBufLen, AppFrm);			
		}

		if (1 == nType)
		{			
			SendAppFrm2Net(	NET_DATA_TYPE_AUDIO, 
				EthMan.RdirAVEthMm.SrcDev, 
				EthMan.RdirAVEthMm.DestDev, 
				AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID,
				EthMan.RdirAVEthMm.nDestIPAddr);
		}
		else if (2 == nType)
		{
			SendAppFrm2Net(	NET_DATA_TYPE_VIDEO, 
				EthMan.RdirAVEthMm.SrcDev, 
				EthMan.RdirAVEthMm.DestDev, 
				AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID,
				EthMan.RdirAVEthMm.nDestIPAddr);
		}
		else if (3 == nType)
		{
			SendAppFrm2Net(	NET_DATA_TYPE_VIDEO, 
				EthMan.RdirAVEthMm.SrcDev, 
				EthMan.RdirAVEthMm.DestDev, 
				AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID,
				EthMan.RdirAVEthMm.nPhotoDestIPAddr);
		}
		else if (4 == nType)
		{
			SendAppFrm2Net(NET_DATA_TYPE_CMD, 
				EthMan.RdirAVEthMm.SrcDev, 
				EthMan.RdirAVEthMm.DestDev, 
				AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID, 
				EthMan.RdirAVEthMm.nPhotoDestIPAddr);
		}
	}
}

#ifdef ETH_MSG_DEBUG
static void
EthMsgDebug(MXMSG* pMsg, BOOL bPut)
{
	if (bPut)
	{
		printf("Eth: MSG debug (Put)  ");
	}
	else
	{
		printf("Eth: MSG debug (Get)  ");
	}
	printf("SrcDev: %s, DestDev: %s, SrvMd: %02X, DestMd: %02X, MSG: %08X, DPARAM %08X, TIME=%d\n",
		pMsg->szSrcDev,
		pMsg->szDestDev,
		(unsigned char) pMsg->dwSrcMd,
		(unsigned char) pMsg->dwDestMd,
		(unsigned int) pMsg->dwMsg,
		(unsigned int) pMsg->dwParam,
		GetTickCount());
}
#endif

void
RequestVideoIOVP(unsigned int nDestIPAddr)
{
	unsigned char	AppFrm[MAX_APP_FRM_LEN];
	int				nAppFrmLen;
	char			szDevCode[MAX_LEN_DEV_CODE + 1]	= {0};
	
	nAppFrmLen = PckBacpApp(	FC_AV_REQUEST_VIDEO_IVOP, 
		szDevCode, 
		szDevCode, 
		NULL, 0, AppFrm);

	SendAppFrm2Net(	NET_DATA_TYPE_VIDEO, 
		szDevCode, 
		szDevCode, 
		AppFrm, nAppFrmLen, INVALID_SEND_CALLBACK_ID,
		EthMan.RdirAVEthMm.nDestIPAddr);
}


UINT
GetRecvPackNum(void)
{
	return g_PackCnt.nRecvPackCnt;
}

UINT
GetSendPackNum(void)
{
	return g_PackCnt.nSendPackCnt;
}








































