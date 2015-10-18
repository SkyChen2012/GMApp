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
#include "IniFile.h"
/************** DEFINES **************************************************************/

//#define MCMT_DEBUG

static RETRYINFO g_RetryInfo;

/************** TYPEDEFS *************************************************************/




/************** STRUCTURES ***********************************************************/

/************** LOCAL DECLARATIONS ***************************************************/

LCMOX				g_mcmt;

static void		LoadMoxMapTable(void);
static void		LCAMoxProcessMsg(MXMSG * pmsg);
static void		SendErrorACK2Target(DWORD dwMsg, DWORD dwIP, unsigned char ucRetCode);
static void		LCAMoxIdlePro(MXMSG * pmsg);
static int		IsCpltMoxCommfrm(unsigned char* pBuf, int* pnLen);
static void		AddData2Buf(PBYTE pData, BYTE nLen);
static void		ReSendCOMData(void);
static void		ResetCOMBuf(void);
static BOOL		IsCorrectMoxCheckSum(unsigned char* pBuf, unsigned short len, unsigned char chksum);

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
LCAMOXProtoInit(void)
{
	if (LC_MOX_MODE_V1 == g_SysConfig.LC_ComMode)
	{
		DpcAddMd(MXMDID_LT_MOX_V1, NULL);

		LoadMoxMapTable();

		g_RetryInfo.nLen = 0;
		memset(g_RetryInfo.pData, 0, sizeof(g_RetryInfo.pData));
		g_RetryInfo.nRetryCnt = 0;
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadMoxMapTable()
**	AUTHOR:			Harry Qian
**	DATE:			26 - Sep - 2010
**
**	DESCRIPTION:	
**				load the MOX map table of lift control
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
LoadMoxMapTable(void)
{
	const char * pChar = NULL;
	int nCount = 0;
	int i = 0;
	char szSec[20] = {0};

	g_mcmt.nUsrCnt = 0;
	g_mcmt.nLiftCnt = 0;
	g_mcmt.pMCMT = NULL;

	OpenIniFile(MCMT_FILE);
	
	nCount = ReadInt(MCMT_SECTION, KEY_USER_COUNT, 0);
#ifdef MCMT_DEBUG
		printf("[%s]\n%s=%d\n", MCMT_SECTION, KEY_USER_COUNT, nCount);
#endif
	if (nCount > 0 && nCount < 10001)
	{
		g_mcmt.nUsrCnt = nCount;
		g_mcmt.pMCMT = malloc(sizeof(MCMT) * nCount);
		memset(g_mcmt.pMCMT, 0, sizeof(MCMT) * nCount);

		if (g_mcmt.pMCMT == NULL)
		{
			printf("Error: the g_mcmt.pMCMT can not be NULL...\n");
			CloseIniFile();
			return;
		}

		for (i = 0; i < nCount; i++)
		{
			snprintf(szSec, sizeof (szSec), "user%d", i + 1);

			pChar = ReadString(szSec, KEY_DEVICECODE, "");
			strcpy(g_mcmt.pMCMT[i].szCode, pChar);
#ifdef MCMT_DEBUG
			printf("[%s]\n%s=%s\n", szSec, KEY_DEVICECODE, g_mcmt.pMCMT[i].szCode);
#endif
			g_mcmt.pMCMT[i].ucType = ReadInt(szSec, KEY_TYPE, 0);
			g_mcmt.pMCMT[i].ucLiftID = ReadInt(szSec, KEY_LIFTID, 0);
			g_mcmt.pMCMT[i].ucLayer = ReadInt(szSec, KEY_LAYER, 0);

		}
	
	}

	nCount = ReadInt(MCMT_SECTION, KEY_LIFT_COUNT, 0);
#ifdef MCMT_DEBUG
		printf("[%s]\n%s=%d\n", MCMT_SECTION, KEY_LIFT_COUNT, nCount);
#endif
	if (nCount > 0 && nCount < 9)// assume max lift group is 8.
	{
		g_mcmt.nLiftCnt = nCount;
		g_mcmt.pLFDA = malloc(sizeof(MLDA) * nCount);
		memset(g_mcmt.pLFDA, 0, sizeof(MLDA) * nCount);

		if (g_mcmt.pLFDA == NULL)
		{
			printf("Error: the g_mcmt.pLFDA can not be NULL...\n");
			CloseIniFile();
			return;
		}

		for (i = 0; i < nCount; i++)
		{
			snprintf(szSec, sizeof (szSec), "lift%d", i + 1);
			g_mcmt.pLFDA[i].da1 = ReadInt(szSec, KEY_DA1, 0);
			g_mcmt.pLFDA[i].da2 = ReadInt(szSec, KEY_DA2, 0);
			g_mcmt.pLFDA[i].da3 = ReadInt(szSec, KEY_DA3, 0);

		}
	}

	CloseIniFile();
}

DWORD
GetLiftIDbyLayer(int nLayer)
{
	int i = 0;
	DWORD dwRet = 0;
	int nLiftID = 0;
	if (g_mcmt.nUsrCnt > 0)
	{
		for (i = 0; i < g_mcmt.nUsrCnt; i++)
		{
			if (nLayer == g_mcmt.pMCMT[i].ucLayer) 
			{
				nLiftID = g_mcmt.pMCMT[i].ucLiftID - 1;
				dwRet = MAKEDWORD(0, g_mcmt.pLFDA[nLiftID].da1, g_mcmt.pLFDA[nLiftID].da2, g_mcmt.pLFDA[nLiftID].da3);
				return dwRet;		
			}
		}
	}
	return dwRet;
}


DWORD
GetLiftIDbyCode(const char * pChar)
{
	int i = 0;
	DWORD dwRet = 0;
	int nLiftID = 0;
	if (pChar == NULL)
	{
		return 0;
	}
	if (g_mcmt.nUsrCnt > 0)
	{
		for (i = 0; i < g_mcmt.nUsrCnt; i++)
		{
#ifdef MCMT_DEBUG
			printf("[%s]: %s vs %s\n", __FUNCTION__, pChar, g_mcmt.pMCMT[i].szCode);
#endif
			if (!strcmp(pChar, g_mcmt.pMCMT[i].szCode)) 
			{
				nLiftID = g_mcmt.pMCMT[i].ucLiftID - 1;
				if (nLiftID < g_mcmt.nLiftCnt)
				{
					dwRet = MAKEDWORD(0, g_mcmt.pLFDA[nLiftID].da1, g_mcmt.pLFDA[nLiftID].da2, g_mcmt.pLFDA[nLiftID].da3);
					return dwRet;
				}
			}
		}
	}
	return dwRet;
}

BYTE
GetLayerbyCode(const char * pChar)
{
	int i = 0;
	BYTE nRet = 0;
	int nLiftID = 0;
	if (pChar == NULL)
	{
		return 0;
	}
	if (g_mcmt.nUsrCnt > 0)
	{
		for (i = 0; i < g_mcmt.nUsrCnt; i++)
		{
			if (!strcmp(pChar, g_mcmt.pMCMT[i].szCode)) 
			{
				nRet = g_mcmt.pMCMT[i].ucLayer;
				return nRet;
			}
		}
	}
	return nRet;
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
LCAMOXProtoExit(void)
{
	if (LC_MOX_MODE_V1 == g_SysConfig.LC_ComMode) 
	{
		DpcRmMd(MXMDID_LT_MOX_V1);
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
LCAMoxThreadFun(void)
{
	MXMSG	g_MsgRecev;

	memset(&g_MsgRecev, 0, sizeof(g_MsgRecev));
	g_MsgRecev.dwDestMd		= MXMDID_LT_MOX_V1;
	g_MsgRecev.pParam		= NULL;

	if (MxGetMsg(&g_MsgRecev)) 
	{
		LCAMoxProcessMsg(&g_MsgRecev);
		DoClearResource(&g_MsgRecev);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMoxProcessMsg()
**	AUTHOR:			Harry Qian
**	DATE:			16 - Sep - 2010
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
static void
LCAMoxProcessMsg(MXMSG * pmsg)
{
	LCAMoxIdlePro(pmsg);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PckCommApp
**	AUTHOR:			Harry Qian 
**	DATE:			28 - Sep - 2010
**
**	DESCRIPTION:	
**			Pack BACP application data
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nFunCode	[IN]		unsigned short
**				pSrcDev		[IN]		char*
**				pDestDev	[IN]		char*	
**				pData		[IN]		unsigned char*
**				nDataLen	[IN]		int
**				pFrm		[OUT]		unsigned char*
**	RETURNED VALUE:	
**				packet length
**	NOTES:
**			
*/
int
PckCommApp(unsigned short nFunCode,  unsigned char* pData, int nDataLen, unsigned char* pFrm)
{
	int		nFrmLen		= 0;
	int		nLenPos		= 0;

	// FUNCTION CODE

	*(pFrm + nFrmLen + 0) = LOBYTE(nFunCode);
	*(pFrm + nFrmLen + 1) = HIBYTE(nFunCode);
	
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
	*(pFrm + nLenPos + 0) = LOBYTE(nFrmLen);
	*(pFrm + nLenPos + 1) = HIBYTE(nFrmLen);

	return nFrmLen;
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PackCommRqtEx
**	AUTHOR:			Harry Qian 
**	DATE:			17 - Sep - 2010
**
**	DESCRIPTION:	
**			Packet serial request frame
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pAppBuf		[IN]		unsigned char*
**				nAppBufLen	[IN]		int
**				pSA			[IN]		source station number
**				pDA			[IN]		dest station number
**				bNeedRsp	[IN]		BOOL	TRUE if need network layer response
**				nSeq		[IN]		unsinged int
**				pFrm		[OUT]		unsigned char*
**	RETURNED VALUE:	
**				frame length
**	NOTES:
**			
*/
int
PackCommRqtEx(unsigned char* pAppBuf, int nAppBufLen, PBYTE pSA, PBYTE pDA, BOOL bNeedRsp, unsigned short nSeq, unsigned char* pFrm)
{
	int				nFrmLen	= 0;
	int				nLenPos	= OFFSET_COM_LY_PACKAGE_LEN;
//	unsigned int	nSeq	= GetNextRqtSeq();
	unsigned short  usChk = 0;
	int i = 0;
	// STX
	*(pFrm + nFrmLen + 0) = BACP_COM_LY_STX_B0;
	*(pFrm + nFrmLen + 1) = BACP_COM_LY_STX_B1;

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
		*(pFrm + nFrmLen) = COM_LY_OPT_REQ_ON | COM_LY_OPT_SA;
	}
	else
	{
		*(pFrm + nFrmLen) = COM_LY_OPT_REQ_OFF | COM_LY_OPT_SA;
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
	*(pFrm + nFrmLen + 0) = BACP_COM_LY_ETX_B0;
	*(pFrm + nFrmLen + 1) = BACP_COM_LY_ETX_B1;

	nFrmLen += LEN_COM_ETX;

	// Fill LEN 
	*(pFrm + nLenPos + 0) = LOBYTE(nFrmLen);
	*(pFrm + nLenPos + 1) = HIBYTE(nFrmLen);


	usChk = 0;
	for (i = LEN_COM_STX; i < nFrmLen - 4; i++)
	{
		usChk += *(pFrm + i);
	}

	*(pFrm + nFrmLen - 4) = LOBYTE(usChk);
	*(pFrm + nFrmLen - 3) = HIBYTE(usChk);

	return nFrmLen;
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendData2RS485
**	AUTHOR:			Harry Qian 
**	DATE:			28 - Sep - 2010
**
**	DESCRIPTION:	
**			send data to RS485 and receive the response packet.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pAppBuf		[IN]		unsigned char*
**				nAppBufLen	[IN]		int
**				pSA			[IN]		source station number
**				pDA			[IN]		dest station number
**				bNeedRsp	[IN]		BOOL	TRUE if need network layer response
**				nSeq		[IN]		unsinged int
**				pFrm		[OUT]		unsigned char*
**	RETURNED VALUE:	
**				frame length
**	NOTES:
**			
*/
unsigned char
SendData2RS485(unsigned char* pAppBuf, int nAppBufLen, PBYTE pSA, PBYTE pDA)
{	
	static 	unsigned short usSeq = 0;
	unsigned char pDataSend[256] = {0};
	unsigned char pDataRecv[256] = {0};
	short nDataLen = 0;


	nDataLen = PackCommRqtEx(pAppBuf, nAppBufLen, pSA, pDA, NOT_RSP, ++usSeq, pDataSend);


	WriteMox485Data(pDataSend, nDataLen);
	AddData2Buf(pDataSend, nDataLen);

	return 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMoxIdlePro()
**	AUTHOR:			Harry Qian
**	DATE:			16 - Sep - 2010
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
static void
LCAMoxIdlePro(MXMSG * pmsg)
{
	unsigned char	AppTemp[1024];
	int				nAppTempLen = 0;
	unsigned char nDataLen = 0;
	char pDA[3] = {0};
	unsigned char ucRetCode = 0;

	switch(pmsg->dwMsg) 
	{
	case FC_LF_STOP_DOWN:
	case FC_LF_STOP_UP:
	case FC_LF_STOP:  //LOWORD
	case FC_LF_A_B:
	case FC_LF_UNLOCK:
		nDataLen = *((unsigned char*)pmsg->pParam);
		// MsgGet.pParam store self ip address


		nAppTempLen = PckCommApp( (unsigned short) pmsg->dwMsg, 
				pmsg->pParam + 1, nDataLen - 4, 
				AppTemp);

		ucRetCode = SendData2RS485(AppTemp, nAppTempLen, &pDA, &pDA);

		g_LCAProtocol.nStatus	= LCA_STATUS_WAITING_ACK;
		g_LCAProtocol.dwTick	= GetTickCount();
		g_LCAProtocol.nMsg		= pmsg->dwMsg;
		g_LCAProtocol.dwIP		= pmsg->dwParam;
		break;
	case FC_LF_STATE_NOTIFY:
	case FC_LF_STATE:
		nAppTempLen = PckCommApp( (unsigned short) pmsg->dwMsg, 
				NULL, 0, 
				AppTemp);

		ucRetCode = SendData2RS485(AppTemp, nAppTempLen, &pDA, &pDA);

		g_LCAProtocol.nStatus	= LCA_STATUS_WAITING_ACK;
		g_LCAProtocol.dwTick	= GetTickCount();
		g_LCAProtocol.nMsg		= pmsg->dwMsg;
		g_LCAProtocol.dwIP		= pmsg->dwParam;
		break;
	default:
		break;
	}
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMoxWaitingAckPro()
**	AUTHOR:			Harry Qian
**	DATE:			16 - Sep - 2010
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
void
LCAMoxWaitingAckPro(void)
{
	static unsigned char pDataRecv[256] = {0};
	static int nDataLen = 0;
	short nReadLen = 0;
	unsigned char ucRetCode = 0;

	if (g_LCAProtocol.nStatus	== LCA_STATUS_WAITING_ACK && GetTickCount() > g_LCAProtocol.dwTick + SERIAL_COMM_TIMEOUT)
	{
		printf("ReSend the data to com port:\n");
		if (--g_RetryInfo.nRetryCnt > 0) 
		{
			ReSendCOMData();
			g_LCAProtocol.dwTick	= GetTickCount();
		}
		else
		{
			SendErrorACK2Target(g_LCAProtocol.nMsg, g_LCAProtocol.dwIP, 0xff);
			g_LCAProtocol.dwTick	= GetTickCount();
			g_LCAProtocol.nStatus	= LCA_STATUS_IDLE;
			nDataLen = 0;
			ResetCOMBuf();
		}

	}

	nReadLen = ReadMox485Data(&pDataRecv[nDataLen], 255 - nDataLen);
	if (nReadLen > 0 )
	{
		nDataLen += nReadLen;
	}

	if (nDataLen >= MIN_DATA_LEN && IsCpltMoxCommfrm(pDataRecv, &nDataLen))
	{
#ifdef MCMT_DEBUG
		printf("The data rece is valid....the nDataLen=%x.\n", nDataLen);		
#endif
		if (nDataLen < MIN_DATA_LEN)
		{
			ucRetCode = 0XFF;
		}
		else
		{
			ucRetCode =  pDataRecv[ACK_ERRORTIP_OFFSET];
		}

		SendErrorACK2Target(g_LCAProtocol.nMsg, g_LCAProtocol.dwIP, ucRetCode);
		g_LCAProtocol.nStatus	= LCA_STATUS_IDLE;
		g_LCAProtocol.dwTick	= GetTickCount();
		nDataLen = 0;
	}
	
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMoxRecvCOM()
**	AUTHOR:			Harry Qian
**	DATE:			16 - Sep - 2010
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
void
LCAMoxRecvCOM(void)
{
	static unsigned char pDataRecv[256] = {0};
	static int nDataLen = 0;
	short nReadLen = 0;
	unsigned char ucRetCode = 0;

	nReadLen = ReadMox485Data(&pDataRecv[nDataLen], 255 - nDataLen);
	if (nReadLen > 0 )
	{
		nDataLen += nReadLen;
	}

	if (nDataLen >= MIN_DATA_LEN && IsCpltMoxCommfrm(pDataRecv, &nDataLen))
	{
#ifdef MCMT_DEBUG
		printf("The data rece is valid....the nDataLen=%x.\n", nDataLen);
#endif
		if (nDataLen < MIN_DATA_LEN)
		{
			ucRetCode = 0XFF;
		}
		else
		{
			ucRetCode =  pDataRecv[ACK_ERRORTIP_OFFSET];
		}

	//	SendErrorACK2Target(g_LCAProtocol.nMsg, g_LCAProtocol.dwIP, ucRetCode);
	//	g_LCAProtocol.nStatus	= LCA_STATUS_IDLE;
	//	g_LCAProtocol.dwTick	= GetTickCount();
	//	nDataLen = 0;
	}
	
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendErrorACK2Target()
**	AUTHOR:			Harry Qian
**	DATE:			28 - Sep - 2010
**
**	DESCRIPTION:	
**			Send the error code to APP module
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
SendErrorACK2Target(DWORD dwMsg, DWORD dwIP, unsigned char ucRetCode)
{
	MXMSG  msgSend;

	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LT_MOX_V1;
	msgSend.dwDestMd	= MXMDID_LCA;

	if (dwMsg < 0x8000)
	{
		msgSend.dwMsg		= dwMsg + 0x8000;
	}

	msgSend.dwParam		= dwIP;
	msgSend.pParam		= malloc(1);
	*msgSend.pParam		= ucRetCode;
	MxPutMsg(&msgSend);
	printf("return the error ACK=%d.\n", ucRetCode);

}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsCpltMoxCommfrm
**	AUTHOR:			Harry Qian
**	DATE:			9 - Oct - 2010
**
**	DESCRIPTION:	
**			Check whether the frame is complete
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pBuf		[IN/OUT]	unsigned char*
**				pnLen		[IN/OUT]	int*
**	RETURNED VALUE:	
**				TRUE if valid, otherwise FALSE
**	NOTES:
**			
*/
static int
IsCpltMoxCommfrm(unsigned char* pBuf, int* pnLen)
{
	BOOL				bCpltFrm		= FALSE;
	int					nValidHeadPos	= 0;
	unsigned short		nLen			= 0;
	int					i;
	// Check minimum length
	if (*pnLen < MIN_DATA_LEN)
	{

		return 0;
	}

	for (i = 0; i <= *pnLen - MIN_DATA_LEN; i++)
	{
		// Find STX
		if ( (BACP_COM_LY_STX_B0	== *(pBuf + i + 0))	&&
			 (BACP_COM_LY_STX_B1	== *(pBuf + i + 1)))	
		{
			// Save first valid head position
			if (0 == nValidHeadPos)
			{
				nValidHeadPos = i;
			}

			nLen = MAKEWORD(*(pBuf + i + 3), *(pBuf + i + 4));

			// Check LEN, VER, ETX
			if ( (nLen <= *pnLen - i)													&&
				 (BACP_COM_VER == *(pBuf + i + 2))		&&	
				 (BACP_COM_LY_ETX_B0 == *(pBuf + i + nLen - 2))	&&
				 (BACP_COM_LY_ETX_B1 == *(pBuf + i + nLen - 1))	&&
				 (IsCorrectMoxCheckSum((unsigned char* )(pBuf + nValidHeadPos + OFFSET_BACP_SERIAL_VER),
				 (nLen - LEN_BACP_SERIAL_STX - LEN_BACP_SERIAL_ETX - LEN_BACP_SERIAL_CHK),
				(*(unsigned short*)(pBuf + nValidHeadPos + nLen - LEN_BACP_SERIAL_ETX - LEN_BACP_SERIAL_CHK))))
				  )
			{

				bCpltFrm = TRUE;
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

static void
AddData2Buf(PBYTE pData, BYTE nLen)
{
	memcpy(g_RetryInfo.pData, pData, nLen);
	g_RetryInfo.nLen = nLen;
	g_RetryInfo.nRetryCnt = MOX_COM_RETRY_NUM;
}

static void
ReSendCOMData(void)
{
	WriteMox485Data(g_RetryInfo.pData, g_RetryInfo.nLen);
}



static void
ResetCOMBuf(void)
{
	memset(g_RetryInfo.pData, 0, sizeof(g_RetryInfo.pData));
	g_RetryInfo.nLen = 0;
	g_RetryInfo.nRetryCnt = 0;
}

static
BOOL
IsCorrectMoxCheckSum(unsigned char* pBuf, unsigned short len, unsigned char chksum)
{
	BOOL				bCorrectChk		= FALSE;
	unsigned char		value			= 0;
	unsigned char			temp			= 0;
	unsigned short		i				= 0;
	
	for (i = 0; i < len; i++)
	{
		
		temp = (unsigned char)(*(pBuf + i));
		value += temp;
	}
	
	if (value == chksum) 
	{
		bCorrectChk = TRUE;
	}
	else
	{
		bCorrectChk = FALSE;
	}	
	
	return bCorrectChk;
}







