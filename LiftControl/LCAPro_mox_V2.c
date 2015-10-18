/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	LCAPro_mox.c
**
**	AUTHOR:		Harry	Qian
**
**	DATE:		22 - Jul - 2010
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				lift control for MOX protocol
**					
**				
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
#include "BacpSerial.h"
#include "BacpSerialApp.h"
#include "EGMLiftControl.h"
#include "EGMLCAgent.h"
#include "LCAPro_mox_V1.h"
#include "LCAPro_mox_V2.h"
#include "IniFile.h"
/************** DEFINES **************************************************************/

//#define MCMT_DEBUG

#define MOX2_HEARTBEAT_TIME		5000
#define MOX2_WAIT_ACK_TIME		200
#define MOX2_MIN_DATA_LEN		17


#define MOX2_MAX_LIFT_CNT			50
#define MOX2_MAX_CARDREADER_CNT		50
#define MOX2_MAX_DOOR_CNT			50
#define MOX2_MAX_USER_CNT			5000

#define MOX2_HV2HV_STATUS_IDLE				0
#define MOX2_HV2HV_STATUS_WAIT_STOP_ACK		1
#define MOX2_HV2HV_STATUS_WAIT_ULOCK_ACK		2

typedef struct _M2CCMT
{
	unsigned int nLiftCnt;
	unsigned int nCardReaderCnt;
	unsigned int nUserCnt;
	unsigned int nDoorCnt;
	MOXLIFT	*	pLift;
	MOXCARDREADER	*	pCardReader;
	MOXUSER	*		pUser;
	MOXDOOR	*		pDoor;
}M2CCMT;


typedef struct _M2HV2HV
{
	unsigned int nStatus;
	char szSrcUserCode[20];
	char szDestUserCode[20];
}M2HV2HV;

/************** TYPEDEFS *************************************************************/


static M2CCMT g_m2ccmt;
static unsigned char g_Seq=0;
static M2HV2HV g_HV2HV;
//static int g_HV2HVStatus=MOX2_HV2HV_STATUS_IDLE;
/************** STRUCTURES ***********************************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static DWORD	g_dwHeartBeatTick;

static unsigned char SendData2RS485(unsigned char* pAppBuf, int nAppBufLen, PBYTE pSA, PBYTE pDA,BOOL bNeedRsp,unsigned char usSeq);
static int PackCommRqtEx(unsigned char* pAppBuf, int nAppBufLen, PBYTE pSA, PBYTE pDA, BOOL bNeedRsp, unsigned char nSeq, unsigned char* pFrm);
static int PckCommApp(unsigned short nFunCode,  unsigned char* pData, int nDataLen, unsigned char* pFrm);
static void MOX2SendHeartBreak(void);
static BOOL LoadM2CCMTFile(void);
static void MOX2SendData2485(MXMSG * pMsg,BOOL bNeedRsp);
static VOID SendErrorACK2Target(DWORD dwMsg, DWORD dwIP, unsigned char ucRetCode);
static BOOL IsCpltMOX2Commfrm(unsigned char* pBuf, int* pnLen);
static BOOL IsCorrectMoxCheckSum(unsigned char* pBuf, unsigned short len, unsigned char * chksum);
static void ProcMsgHV2HV(MXMSG * pMsg);
static void SetProtocolStatus(DWORD	nMsg,DWORD	dwIP);
static void MOX2HV2HVSendStopCMD(MXMSG * pMsg);
static void MOX2HV2HVProc(void);
static void MOX2HV2HVSendUnLockCMD(void);
/*************************************************************************************/
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMOXProtoInit()
**	AUTHOR:			Harry Qian
**	DATE:			16 - Sep - 2010
**
**	DESCRIPTION:	
**				initialize the MOX protocol process
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/


void
LCAMOX2ProtoInit(void)
{
	printf("%s\n",__FUNCTION__);
	if (LC_MOX_MODE_V2 == g_SysConfig.LC_ComMode)
	{
		if(LoadM2CCMTFile())
		{
			memset(&g_HV2HV,0,sizeof(g_HV2HV));
			DpcAddMd(MXMDID_LT_MOX_V2, NULL);
		}		
	}
}
static BOOL LoadM2CCMTFile(void)
{
	const char * pChar = NULL;
	int i = 0;
	char szSec[20] = {0};

	memset(&g_m2ccmt,0,sizeof(g_m2ccmt));
	


	OpenIniFile(M2CCMT_FILE);
	
	

	g_m2ccmt.nLiftCnt = ReadInt(M2CCMT_GLOBAL_SECTION, M2CCMT_KEY_LIFT_CNT, 0);
	g_m2ccmt.nCardReaderCnt = ReadInt(M2CCMT_GLOBAL_SECTION, M2CCMT_KEY_CARDREADER_CNT, 0);
	g_m2ccmt.nUserCnt = ReadInt(M2CCMT_GLOBAL_SECTION, M2CCMT_KEY_USER_CNT, 0);
	g_m2ccmt.nDoorCnt = ReadInt(M2CCMT_GLOBAL_SECTION, M2CCMT_KEY_DOOR_CNT, 0);
	
	if(g_m2ccmt.nLiftCnt>MOX2_MAX_LIFT_CNT)
	{
		return FALSE;
	}
	if(g_m2ccmt.nCardReaderCnt>MOX2_MAX_CARDREADER_CNT)
	{
		return FALSE;
	}
	if(g_m2ccmt.nUserCnt>MOX2_MAX_USER_CNT)
	{
		return FALSE;
	}
	if(g_m2ccmt.nDoorCnt>MOX2_MAX_DOOR_CNT)
	{
		return FALSE;
	}
	
	g_m2ccmt.pLift= malloc(sizeof(MOXLIFT) * g_m2ccmt.nLiftCnt);
	if(NULL==g_m2ccmt.pLift)
	{
		printf("%s Error %d \n",__FUNCTION__,__LINE__);
		return FALSE;
	}
	memset(g_m2ccmt.pLift, 0, sizeof(MOXLIFT) * g_m2ccmt.nLiftCnt);
	
	g_m2ccmt.pCardReader= malloc(sizeof(MOXCARDREADER) * g_m2ccmt.nCardReaderCnt);
	if(NULL==g_m2ccmt.pCardReader)
	{
		printf("%s Error %d \n",__FUNCTION__,__LINE__);
		return FALSE;
	}
	memset(g_m2ccmt.pCardReader, 0, sizeof(MOXCARDREADER) * g_m2ccmt.nCardReaderCnt);
		
	g_m2ccmt.pUser= malloc(sizeof(MOXUSER) * g_m2ccmt.nUserCnt);
	if(NULL==g_m2ccmt.pUser)
	{
		printf("%s Error %d \n",__FUNCTION__,__LINE__);
		return FALSE;
	}
	memset(g_m2ccmt.pUser, 0, sizeof(MOXUSER) * g_m2ccmt.nUserCnt);
	
	g_m2ccmt.pDoor= malloc(sizeof(MOXDOOR) * g_m2ccmt.nDoorCnt);
	if(NULL==g_m2ccmt.pDoor)
	{
		printf("%s Error %d \n",__FUNCTION__,__LINE__);
		return FALSE;
	}
	memset(g_m2ccmt.pDoor, 0, sizeof(MOXDOOR) * g_m2ccmt.nDoorCnt);



	for(i=0;i<g_m2ccmt.nLiftCnt;i++)
	{
		sprintf(szSec,M2CCMT_LIFT_SECTION, i+1);
		g_m2ccmt.pLift[i].cDA1= ReadInt(szSec, M2CCMT_KEY_LIFT_DA1, 0);
		g_m2ccmt.pLift[i].cDA2= ReadInt(szSec, M2CCMT_KEY_LIFT_DA2, 0);
		g_m2ccmt.pLift[i].cDA3= ReadInt(szSec, M2CCMT_KEY_LIFT_DA3, 0);
		g_m2ccmt.pLift[i].cGroup= ReadInt(szSec, M2CCMT_KEY_LIFT_GROUP, 0);
	}
	

	for(i=0;i<g_m2ccmt.nCardReaderCnt;i++)
	{
		sprintf(szSec,M2CCMT_CARDREADER_SECTION, i+1);
		g_m2ccmt.pCardReader[i].cLiftID= ReadInt(szSec, M2CCMT_KEY_CARDREADER_LIFTID, 0);
		g_m2ccmt.pCardReader[i].cStationNum= ReadInt(szSec, M2CCMT_KEY_CARDREADER_STATIONNUM, 0);
		g_m2ccmt.pCardReader[i].cLiftIndex= ReadInt(szSec, M2CCMT_KEY_CARDREADER_LIFTINDEX, 0);
	}



	for(i=0;i<g_m2ccmt.nUserCnt;i++)
	{
		sprintf(szSec,M2CCMT_USER_SECTION, i+1);

		pChar = ReadString(szSec, M2CCMT_KEY_USER_AMTCODE, "");
		strcpy(g_m2ccmt.pUser[i].szAMTCode, pChar);
		
		pChar = ReadString(szSec, M2CCMT_KEY_USER_DNSCODE, "");
		strcpy(g_m2ccmt.pUser[i].szDNSCode, pChar);
		
		pChar = ReadString(szSec, M2CCMT_KEY_USER_DOORCODE, "");
		strcpy(g_m2ccmt.pUser[i].szDoorCode, pChar);
		
		g_m2ccmt.pUser[i].cLiftID= ReadInt(szSec, M2CCMT_KEY_USER_LIFTID, 0);
		g_m2ccmt.pUser[i].cLevel= ReadInt(szSec, M2CCMT_KEY_USER_LEVEL, 0);
		g_m2ccmt.pUser[i].cDoorType= ReadInt(szSec, M2CCMT_KEY_USER_TYPE, 0);
		
		
	}
	

	for(i=0;i<g_m2ccmt.nDoorCnt;i++)
	{
		sprintf(szSec,M2CCMT_DOOR_SECTION, i+1);

		pChar = ReadString(szSec, M2CCMT_KEY_DOOR_DOORCODE, "");
		strcpy(g_m2ccmt.pDoor[i].szDoorCode, pChar);
	
		g_m2ccmt.pDoor[i].cLevel= ReadInt(szSec, M2CCMT_KEY_DOOR_LEVEL, 0);
		g_m2ccmt.pDoor[i].cDoorType= ReadInt(szSec, M2CCMT_KEY_DOOR_TYPE, 0);
		
	}

	CloseIniFile();
	return TRUE;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMOXProtoExit()
**	AUTHOR:			Harry Qian
**	DATE:			16 - Sep - 2010
**
**	DESCRIPTION:	
**				exit the mox protocol module
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
void
LCAMOX2ProtoExit(void)
{
printf("%s\n",__FUNCTION__);
	if (LC_MOX_MODE_V2 == g_SysConfig.LC_ComMode) 
	{
		DpcRmMd(MXMDID_LT_MOX_V2);
	}
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMoxThreadFun()
**	AUTHOR:			Harry Qian
**	DATE:			16 - Sep - 2010
**
**	DESCRIPTION:	
**				process the command to hitachi protocol
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
void
LCAMox2ThreadFun(void)
{
	MXMSG msgRecv;

	memset(&msgRecv, 0, sizeof(MXMSG));
	msgRecv.dwDestMd	= MXMDID_LT_MOX_V2;
	msgRecv.pParam		= NULL;

	if (MxGetMsg(&msgRecv)) 
	{
		switch(msgRecv.dwMsg)
		{
			case FC_LF_HEARTBEAT:
				if (GetTickCount() - g_dwHeartBeatTick > MOX2_HEARTBEAT_TIME)
				{
					g_dwHeartBeatTick = GetTickCount();
					
					printf("TX: HertBeat\n");
					MOX2SendHeartBreak();
				}
				break;
			case FC_LF_A_B:
			case FC_LF_STOP_UP:
			case FC_LF_STOP_DOWN:
			case FC_LF_UNLOCK:
				MOX2SendData2485(&msgRecv,NEED_RSP);
				break;
			case MXMSG_MOX2LIFT_HV2HV:
				ProcMsgHV2HV(&msgRecv);
				break;
			default:
				break;
		}
		DoClearResource(&msgRecv);
	}
	
	
}
static void MOX2HV2HVProc(void)
{
	if(MOX2_HV2HV_STATUS_WAIT_STOP_ACK==g_HV2HV.nStatus )
	{
		MOX2HV2HVSendUnLockCMD();
		g_HV2HV.nStatus=MOX2_HV2HV_STATUS_WAIT_ULOCK_ACK;
	}
	else if( MOX2_HV2HV_STATUS_WAIT_ULOCK_ACK==g_HV2HV.nStatus )
	{
		g_HV2HV.nStatus=MOX2_HV2HV_STATUS_IDLE;
		SendErrorACK2Target(g_LCAProtocol.nMsg, g_LCAProtocol.dwIP, 0x00);
		g_LCAProtocol.nStatus	= LCA_STATUS_IDLE;
	}
}
static void ProcMsgHV2HV(MXMSG * pMsg)
{
	if(g_HV2HV.nStatus==MOX2_HV2HV_STATUS_IDLE)
	{
		memcpy(g_HV2HV.szSrcUserCode,pMsg->pParam,RD_CODE_LEN);
		memcpy(g_HV2HV.szDestUserCode,pMsg->pParam+RD_CODE_LEN,RD_CODE_LEN);
		MOX2HV2HVSendStopCMD(pMsg);
	}
}
static void MOX2HV2HVSendUnLockCMD(void)
{
	unsigned char	AppTemp[1024];
	int				nAppTempLen = 0;
	unsigned char nDataLen = 12-4+1*2;
	BYTE DA[3] = {0,0,1};
	BYTE SA[3] = {0};
	unsigned char Data[nDataLen];

	MOXUSER	*	pDestUser;
	MOXLIFT	*	pLift;
	unsigned char * pData=Data;
	unsigned short sIdentity=0;
	
	unsigned short nFunCode;
	unsigned short sTime=0;
	

	pDestUser=GetpUserByCode( g_HV2HV.szDestUserCode,"");
	if(NULL==pDestUser)
	{
		return ;
	}
	
	pLift=GetpLiftByLiftID(pDestUser->cLiftID);
	if(NULL ==pLift)
	{
		return ;
	}
	nFunCode=FC_LF_UNLOCK;

	
	
	*pData=MOX2_LOCATION_TYPE_HV;
	pData++;
	
	*pData=pLift->cGroup;
	pData++;
	*pData=MOX2_LIFT_INDEX_BROADCAST;
	pData++;
	*pData=sIdentity>>8;
	pData++;
	*pData=sIdentity & 0xff;
	pData++;
	
	*pData=1;
	
	pData++;
	*pData=pDestUser->cLevel;
	pData++;
	*pData=pDestUser->cDoorType;
	pData++;
	*pData=sTime>>8;
	pData++;
	*pData=sTime & 0xff;
	pData++;	
	
	DA[0]=pLift->cDA1;
	DA[1]=pLift->cDA2;
	DA[2]=pLift->cDA3;
	
	nAppTempLen = PckCommApp( nFunCode, 
		Data, nDataLen , 
		AppTemp);
	SendData2RS485(AppTemp, nAppTempLen, SA, DA,NEED_RSP,++g_Seq);
	SetProtocolStatus(FC_LF_A_B,0);
	
	g_HV2HV.nStatus=MOX2_HV2HV_STATUS_WAIT_STOP_ACK;
	return ;
}
static void MOX2HV2HVSendStopCMD(MXMSG * pMsg)
{
	
	//MOX2SendData2485(&msgRecv,NEED_RSP);
	
	unsigned char	AppTemp[1024];
	int				nAppTempLen = 0;
	unsigned char nDataLen = 7;
	BYTE DA[3] = {0,0,1};
	BYTE SA[3] = {0};
	unsigned char Data[nDataLen];
//	int i;
	MOXUSER	*	pSrcUser;
	MOXUSER	*	pDestUser;
	MOXLIFT	*	pLift;
	unsigned char * pData=Data;
	unsigned short sIdentity=0;
	
	unsigned short nFunCode;
	
	
	pSrcUser=GetpUserByCode( g_HV2HV.szSrcUserCode,"");
	pDestUser=GetpUserByCode( g_HV2HV.szDestUserCode,"");
	if(NULL ==pSrcUser || NULL==pDestUser)
	{
		return ;
	}
	
	pLift=GetpLiftByLiftID(pSrcUser->cLiftID);
	if(NULL ==pLift)
	{
		return ;
	}
	
	if(pSrcUser->cLevel>pDestUser->cLevel)
	{
		nFunCode=FC_LF_STOP_DOWN;
	}
	else
	{
		nFunCode=FC_LF_STOP_UP;
	}
	
	*pData=MOX2_LOCATION_TYPE_HV;
	pData++;
	
	*pData=pLift->cGroup;
	pData++;
	*pData=MOX2_LIFT_INDEX_BROADCAST;
	pData++;
	*pData=pSrcUser->cDoorType;
	pData++;
	*pData=sIdentity>>8;
	pData++;
	*pData=sIdentity & 0xff;
	pData++;
	*pData=pSrcUser->cLevel;
	pData++;
	

	DA[0]=pLift->cDA1;
	DA[1]=pLift->cDA2;
	DA[2]=pLift->cDA3;
	
	nAppTempLen = PckCommApp( nFunCode, 
		Data, nDataLen , 
		AppTemp);
	SendData2RS485(AppTemp, nAppTempLen, SA, DA,NEED_RSP,++g_Seq);
	SetProtocolStatus(FC_LF_A_B,pMsg->dwParam);
	
	g_HV2HV.nStatus=MOX2_HV2HV_STATUS_WAIT_STOP_ACK;
	return ;
}


static void MOX2SendData2485(MXMSG * pMsg, BOOL bNeedRsp)
{
	unsigned char	AppTemp[1024];
	int				nAppTempLen = 0;
	BYTE DA[3] = {0};
	BYTE SA[3] = {0};

	memcpy(DA,pMsg->szDestDev,3);
	
	//nDataLen = *((unsigned char*)pmsg->pParam);
	// MsgGet.pParam store self ip address


	nAppTempLen = PckCommApp( pMsg->dwMsg, 
			pMsg->pParam, pMsg->wDataLen , 
			AppTemp);

	SendData2RS485(AppTemp, nAppTempLen, SA, DA,bNeedRsp, ++g_Seq);
	
	if (FC_LF_HEARTBEAT != pMsg->dwMsg)
	{
		SetProtocolStatus(pMsg->dwMsg,pMsg->dwParam);
	}	

}
static void SetProtocolStatus(	DWORD	nMsg,DWORD	dwIP)
{
		g_LCAProtocol.dwTick	= GetTickCount();
		g_LCAProtocol.nMsg		= nMsg;
		if(dwIP!=0)
		{
			g_LCAProtocol.dwIP		= dwIP;
		}
		g_LCAProtocol.Seq=g_Seq;
		g_LCAProtocol.nStatus	= LCA_STATUS_WAITING_ACK;
}


void
LCAMox2WaitingAckPro(void)
{
	static unsigned char pDataRecv[256] = {0};
	static int nDataLen = 0;
	short nReadLen = 0;

	
	
	//unsigned char ucRetCode = 0;
//	int nMOXDataLen = 0;

	if (	g_LCAProtocol.nStatus	== LCA_STATUS_WAITING_ACK 
			&& (GetTickCount() - g_LCAProtocol.dwTick > MOX2_WAIT_ACK_TIME))
	{
		if(MOX2_HV2HV_STATUS_WAIT_STOP_ACK==g_HV2HV.nStatus || MOX2_HV2HV_STATUS_WAIT_ULOCK_ACK==g_HV2HV.nStatus )
		{
			g_HV2HV.nStatus=MOX2_HV2HV_STATUS_IDLE;
		}
	
	
		if (FC_LF_STATE == g_LCAProtocol.nMsg)
		{
			/*if (g_BnkChkState.bankstate[g_BnkChkState.checkpos] < MIT_BIGFAULT_COUNT)
			{
				g_BnkChkState.bankstate[g_BnkChkState.checkpos]++;
			}
			
			if (g_BnkChkState.checkpos == g_LTConfig.banksize - 1)
			{
				g_BnkChkState.checkpos = 0;
			}
			else
			{
				g_BnkChkState.checkpos++;
			}*/
		}

		SendErrorACK2Target(g_LCAProtocol.nMsg, g_LCAProtocol.dwIP, 0xff);
		g_LCAProtocol.dwTick	= GetTickCount();
		g_LCAProtocol.nStatus	= LCA_STATUS_IDLE;
		nDataLen = 0;
		printf("RX: Time Out\n");
	}
	
	nReadLen = ReadMox485Data(&pDataRecv[nDataLen], 255-nDataLen);
	if (nReadLen > 0 )
	{
		nDataLen += nReadLen;
		printf("nReadLen=%d\n",nDataLen);
	}

	if (nDataLen >= MOX2_MIN_DATA_LEN && IsCpltMOX2Commfrm(pDataRecv, &nDataLen))
	//if (nDataLen >= MOX2_MIN_DATA_LEN )
	{
		/*if (FC_LF_STATE == g_LCAProtocol.nMsg)
		{
			g_BnkChkState.bankstate[g_BnkChkState.checkpos] = 0;
			
			if (g_BnkChkState.checkpos == g_LTConfig.banksize - 1)
			{
				g_BnkChkState.checkpos = 0;
			}
			else
			{
				g_BnkChkState.checkpos++;
			}
		}*/
		

printf("....ok\n");
		if(MOX2_HV2HV_STATUS_WAIT_STOP_ACK==g_HV2HV.nStatus || MOX2_HV2HV_STATUS_WAIT_ULOCK_ACK==g_HV2HV.nStatus )
		{
			MOX2HV2HVProc();
		}
		else
		{
			SendErrorACK2Target(g_LCAProtocol.nMsg, g_LCAProtocol.dwIP, 0x00);
			g_LCAProtocol.nStatus	= LCA_STATUS_IDLE;
		}
		g_LCAProtocol.dwTick	= GetTickCount();
		nDataLen = 0;
	}
	/*
	if (nMitDataLen > 0)
	{
		g_dwStateCount++;
	}
	
	if (GetTickCount() - g_dwStateTick > MIT_LIFTSTATE_TIME)
	{
		if (g_dwStateCount > 0)
		{
			printf("Lift state is normal.\n");
			g_dwStateCount = 0;
		}
		else
		{
			printf("Lift state is abnormal.\n");
		}
		
		g_dwStateTick = GetTickCount();
	}	
	*/
}

static
BOOL
IsCpltMOX2Commfrm(unsigned char* pBuf, int* pnLen)
{
	BOOL				bCpltFrm		= FALSE;
	int					nValidHeadPos	= 0;
	unsigned short		nLen			= 0;
	int					i;
	// Check minimum length
	
	if (*pnLen < MOX2_MIN_DATA_LEN)
	{
		return 0;
	}
	for (i = 0; i <= *pnLen - MOX2_MIN_DATA_LEN; i++)
	{
		// Find STX
		if ( (BACP_COM_LY_STX_B0	== *(pBuf + i + 1))	&&
			 (BACP_COM_LY_STX_B1	== *(pBuf + i + 0)))	
		{
			// Save first valid head position
			if (0 == nValidHeadPos)
			{
				nValidHeadPos = i;
			}
			nLen = MAKEWORD(*(pBuf + i + 4), *(pBuf + i + 3));

			// Check LEN, VER, ETX
			if ( (nLen <= *pnLen - i)													&&
				 (BACP_COM_VER == *(pBuf + i + 2))		&&	
				 (BACP_COM_LY_ETX_B0 == *(pBuf + i + nLen - 1))	&&
				 (BACP_COM_LY_ETX_B1 == *(pBuf + i + nLen - 2))	&&
				 (IsCorrectMoxCheckSum((unsigned char* )(pBuf + nValidHeadPos + OFFSET_BACP_SERIAL_VER),
				 (nLen - LEN_BACP_SERIAL_STX - LEN_BACP_SERIAL_ETX - LEN_BACP_SERIAL_CHK),
				((unsigned char *)(pBuf + nValidHeadPos + nLen - LEN_BACP_SERIAL_ETX - LEN_BACP_SERIAL_CHK))))
				  )
			{
				if(g_LCAProtocol.nStatus	== LCA_STATUS_WAITING_ACK  )
				{
					if((*(pBuf + nValidHeadPos +OFFSET_BACP_SERIAL_OPT) & COM_LY_OPT_REQ_FLAG) )
					{
						if(*(pBuf + nValidHeadPos +OFFSET_BACP_SERIAL_SEQ)== g_LCAProtocol.Seq)
						{
							bCpltFrm = TRUE;
						}
					}
					else
					{
						bCpltFrm = TRUE;
					}
				}
				break;
			}
			else
			{
				nLen = 0;
			}
		}
	}

	if (bCpltFrm)
	{
		//*pnLen = nLen;
		if (i != 0)
		{
			memmove(pBuf, (pBuf + i), *pnLen);
		}
	}
	else
	{
		*pnLen -= nValidHeadPos;
		if (nValidHeadPos != 0)
		{
			memmove(pBuf, (pBuf + nValidHeadPos), *pnLen);
		}
	}
	return bCpltFrm;
}


static
BOOL
IsCorrectMoxCheckSum(unsigned char* pBuf, unsigned short len, unsigned char * chksum)
{
	BOOL				bCorrectChk		= FALSE;
	unsigned short		value			= 0;
	unsigned char			temp			= 0;
	unsigned short		i				= 0;
	unsigned short		CheckSum			= 0;
	CheckSum=(*chksum)*0x100+(*(chksum+1));
	
	
	
	for (i = 0; i < len; i++)
	{
		
		temp = (unsigned char)(*(pBuf + i));
		value += temp;
	}
	
	if (value == CheckSum) 
	{
		bCorrectChk = TRUE;
	}
	else
	{
		bCorrectChk = FALSE;
	}	
	
	//printf("value=%d,CheckSum=%d\n",value,CheckSum);
	return bCorrectChk;
}

static
VOID
SendErrorACK2Target(DWORD dwMsg, DWORD dwIP, unsigned char ucRetCode)
{
	MXMSG  msgSend;

	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LT_MOX_V2;
	msgSend.dwDestMd	= MXMDID_LCA;

	if (dwMsg < 0x8000)
	{
		msgSend.dwMsg		= dwMsg + 0x8000;
	}

	msgSend.dwParam		= dwIP;
	msgSend.pParam		= malloc(1);
	*msgSend.pParam		= ucRetCode;
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Mox_CallLiftA2B()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
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

 MOXUSER * GetpUserByCode(char * szUserCode,char * szDoorCode)
{
	int i;
	for(i=0;i<g_m2ccmt.nUserCnt;i++)
	{
		if(0==strlen(szDoorCode) || 0==strlen(g_m2ccmt.pUser[i].szDoorCode))
		{
			if(0 == strcmp(szUserCode, g_m2ccmt.pUser[i].szDNSCode))
			{
				return &g_m2ccmt.pUser[i];
			}
			if(0 == strcmp(szUserCode, g_m2ccmt.pUser[i].szAMTCode))
			{
				return &g_m2ccmt.pUser[i];
			}
		}
		else
		{
			if(strlen(szDoorCode))
			{
				if(0 == strcmp(szUserCode, g_m2ccmt.pUser[i].szDNSCode) && 0 == strcmp(szDoorCode, g_m2ccmt.pUser[i].szDoorCode))
				{
					return &g_m2ccmt.pUser[i];
				}
				if(0 == strcmp(szUserCode, g_m2ccmt.pUser[i].szAMTCode) && 0 == strcmp(szDoorCode, g_m2ccmt.pUser[i].szDoorCode))
				{
					return &g_m2ccmt.pUser[i];
				}
			}
		}
		
	}
	return NULL;
}

 MOXLIFT	* GetpLiftByLiftID(unsigned char cLiftID)
{
//	int i;
	
	if(cLiftID>g_m2ccmt.nLiftCnt || cLiftID==0)
	{
		return NULL;
	}
	
	return &g_m2ccmt.pLift[cLiftID-1];
}


 MOXLIFT	* GetpLiftByCardReaderStationNum(unsigned char cStationNum,unsigned char * pcLiftIndex)
{
	int i;
	for(i=0;i<g_m2ccmt.nCardReaderCnt;i++)
	{
		if(cStationNum==g_m2ccmt.pCardReader[i].cStationNum)
		{
			*pcLiftIndex=g_m2ccmt.pCardReader[i].cLiftIndex;
			return GetpLiftByLiftID(g_m2ccmt.pCardReader[i].cLiftID);
		}
	}
	return NULL;
}




 MOXDOOR * GetpDoorByCode(char * szDoorCode)
{
	int i;
	if(0==strlen(szDoorCode))
	{
		return NULL;
	}
	for(i=0;i<g_m2ccmt.nDoorCnt;i++)
	{
		if(0 == strcmp(szDoorCode, g_m2ccmt.pDoor[i].szDoorCode))
		{
			return &g_m2ccmt.pDoor[i];
		}	
	}
	return NULL;
}


static void MOX2SendHeartBreak(void)
{
	unsigned char	AppTemp[1024];
	int				nAppTempLen = 0;
	unsigned char nDataLen = 7;
	BYTE DA[3] = {0,0,1};
	BYTE SA[3] = {0};
	unsigned char Data[nDataLen];
	int i;
	
	Data[0]=MOX2_LOCATION_TYPE_GM;//LOCATION
	Data[2]=MOX2_LIFT_INDEX_BROADCAST;//LIFTIDX
	memset(&Data[3],0,4);

	for(i=0;i<g_m2ccmt.nLiftCnt;i++)
	{
		DA[0]=g_m2ccmt.pLift[i].cDA1;
		DA[1]=g_m2ccmt.pLift[i].cDA2;
		DA[2]=g_m2ccmt.pLift[i].cDA3;
		Data[1]=g_m2ccmt.pLift[i].cGroup;;//GROUP
		nAppTempLen = PckCommApp( FC_LF_HEARTBEAT, 
			Data, nDataLen , 
			AppTemp);
		SendData2RS485(AppTemp, nAppTempLen, SA, DA,NOT_RSP,++g_Seq);
	}
}


static int
PckCommApp(unsigned short nFunCode,  unsigned char* pData, int nDataLen, unsigned char* pFrm)
{
	int		nFrmLen		= 0;
	int		nLenPos		= 0;

	// FUNCTION CODE

	*(pFrm + nFrmLen + 1) = LOBYTE(nFunCode);
	*(pFrm + nFrmLen + 0) = HIBYTE(nFunCode);
	
	nFrmLen += LEN_COMM_APP_LY_FC;
	// LEN
	nLenPos = nFrmLen;
	nFrmLen += LEN_COMM_APP_LY_LEN;

	// DATA
	if (nDataLen > 0)
	{
		memcpy((pFrm + nFrmLen), pData, nDataLen);
		nFrmLen += nDataLen;
	}

	// Fill LEN
	*(pFrm + nLenPos + 1) = LOBYTE(nFrmLen);
	*(pFrm + nLenPos + 0) = HIBYTE(nFrmLen);

	return nFrmLen;
}

static unsigned char
SendData2RS485(unsigned char* pAppBuf, int nAppBufLen, PBYTE pSA, PBYTE pDA, BOOL bNeedRsp,unsigned char usSeq)
{	

	unsigned char pDataSend[256] = {0};
	//unsigned char pDataRecv[256] = {0};
	short nDataLen = 0;


	nDataLen = PackCommRqtEx(pAppBuf, nAppBufLen, pSA, pDA, bNeedRsp,usSeq, pDataSend);


	WriteMox485Data((char *)pDataSend, nDataLen);
	//AddData2Buf(pDataSend, nDataLen);

	return 0;
}

static int
PackCommRqtEx(unsigned char* pAppBuf, int nAppBufLen, PBYTE pSA, PBYTE pDA, BOOL bNeedRsp, unsigned char nSeq, unsigned char* pFrm)
{
	int				nFrmLen	= 0;
	int				nLenPos	= OFFSET_COM_LY_PACKAGE_LEN;
//	unsigned int	nSeq	= GetNextRqtSeq();
	unsigned short  usChk = 0;
	int i = 0;
	// STX
	*(pFrm + nFrmLen + 1) = BACP_COM_LY_STX_B0;
	*(pFrm + nFrmLen + 0) = BACP_COM_LY_STX_B1;

	nFrmLen += LEN_COM_STX;
	// VER
	*(pFrm + nFrmLen) = CURRENT_COM_NET_LAY_VER;
	nFrmLen += LEN_COM_VER;

	// LEN
	nFrmLen += LEN_COM_LEN;

	// SEQ

	*(pFrm + nFrmLen + 0) = nSeq;
			
	nFrmLen += LEN_COM_SEQ;


	// OPT
	if (bNeedRsp)
	{
		*(pFrm + nFrmLen) = COM_LY_OPT_REQ_ON ;
	}
	else
	{
		*(pFrm + nFrmLen) = COM_LY_OPT_REQ_OFF ;
	}
	nFrmLen += LEN_COM_OPT; 

	memcpy(pFrm + nFrmLen, pDA, 3);
	nFrmLen += LEN_COM_DA;

	memcpy(pFrm + nFrmLen, pSA, 3);
	nFrmLen += LEN_COM_SA;
	// APP
	memcpy((pFrm + nFrmLen), pAppBuf, nAppBufLen);
	nFrmLen += nAppBufLen;
	
	// check num
	nFrmLen += 2;

	// ETX
	*(pFrm + nFrmLen + 1) = BACP_COM_LY_ETX_B0;
	*(pFrm + nFrmLen + 0) = BACP_COM_LY_ETX_B1;

	nFrmLen += LEN_COM_ETX;

	// Fill LEN 
	*(pFrm + nLenPos + 1) = LOBYTE(nFrmLen);
	*(pFrm + nLenPos + 0) = HIBYTE(nFrmLen);


	usChk = 0;
	for (i = LEN_COM_STX; i < nFrmLen - 4; i++)
	{
		usChk += *(pFrm + i);
	}

	*(pFrm + nFrmLen - 3) = LOBYTE(usChk);
	*(pFrm + nFrmLen - 4) = HIBYTE(usChk);

	return nFrmLen;
}




