/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	BacpNetCtrl.c
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		25 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				BacpNetCtrlInit	
**				BacpNetCtrlExit	
**				NetCtrlFreeMXList
**				NetCtrlRscInit
**				NetCtrlRscExit	
**				NetCtrlLoadResolv
**				NetCtrlResolvCacheFind
**				NetCtrlSendPS
**				NetCtrlRcvSP
**				AnalyseResolvData
**				NetCtrlResolvNet
**				BacpNetCtrlResolv
**				PrintResolvInfo
**				PrintResolvCache
**				
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/timeb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <termios.h>
#include <errno.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXTypes.h"
#include "MXList.h"
#include "MXMem.h"
#include "MXMsg.h"
#include "MXCommon.h"

#include "BacpNet.h"
#include "BacpApp.h"
#include "BacpNetComm.h"
#include "BacpNetCtrl.h"
#include "ModuleTalk.h"
#include "TalkLogReport.h"
#include "MenuParaProc.h"
#include "AMT.h"

/************** DEFINES **************************************************************/

//#define	BACP_NET_CTRL_MEM_DEBUG
//#define	BACP_NET_CTRL_SP_DEBUG
//#define	BACP_NET_CTRL_PS_DEBUG
//#define	BACP_NET_CTRL_RESOLV_RESULT
//#define	BACP_NET_CTRL_TIME_DEBUG
//#define	BACP_NET_CTRL_RESOLV_DEBUG
//#define CALL_INFO_DNS_DEBUG

//#define DNS_INFO_DEBUG

#define	HO_QUERY_METHOD_CODE_DATA_LEN		(MAX_LEN_DEV_CODE + 1)

#define	HO_QUERY_TIMEOUT					200	// ms

#define	HO_QUERY_NOT_FIND					1
#define	HO_QUERY_FIND						0
#define	HO_QUERY_RESULT_LEN					1
#define	HO_QUERY_NAME_LEN_LEN				1
#define	HO_QUERY_ADDR_LEN_LEN				1
#define	HO_QUERY_CODE_LEN_LEN				1
#define	HO_QUERY_IPCNT_LEN					1

#define	DTM_GET_TIMEOUT						200	// ms


//#define  AB_DEBUG

#define ICMP_BUFF_SIZE						1024 
/************** TYPEDEFS *************************************************************/

typedef	struct _NetCtrlRunManStruct
{
	BacpNetCtrlRun		CtrlCltRun;
	MXListHead			CacheHead;
	
} NetCtrlRunManStruct;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

extern char*		GetSelfOuterDevCode();

/************** LOCAL DECLARATIONS ***************************************************/

static	NetCtrlRunManStruct			NetCtrlRunMan;

static void			NetCtrlFreeMXList(MXListHead* pHead);
static void			NetCtrlRscInit(BacpNetCtrlRun* pRun, BOOL bUseSock);
static void			NetCtrlRscExit(BacpNetCtrlRun* pRun);
static void			NetCtrlLoadResolv(MXListHead* pHead);
static BOOL			NetCtrlResolvCacheFind(const MXListHead* pHead, EthResolvInfo* pResolvInfo);
static BOOL			NetCtrlResolvNet(NetCtrlRunManStruct* pMan, EthResolvInfo* pResolvInfo);
static void			NetCtrlSendPS(BacpNetCtrlPSRun* pPS, unsigned char* pAppFrm, int nAppFrmLen, unsigned int nDestIPAddr);
static int			NetCtrlRcvSP(BacpNetCtrlSPRun* pSP, unsigned char* pAppFrm, int nMaxAppFrmLen, unsigned int* pnSrcIPAddr);
static BOOL			AnalyseResolvData(NetCtrlRunManStruct* pMan, EthResolvInfo* pResolvInfo, unsigned char* pData, int nDataLen);
//static void			PrintResolvInfo(EthResolvInfo* pResolvInfo);
//static void			PrintResolvCache(MXListHead* pHead);
static void			NetCtrlLoadIP();

static int			DNSBuildHo800(char* pResponseMsg, AMT_Type800 * at1_Content, UCHAR DevType);
static void			ProcessDNSCommand(char* pCommandMsg, int nCommandMsgLen, WORD nFncCode, char* pResponseMsg, int* pnResponseLen);
static int			DNSBuildHo900(char* pResponseMsg, AMT_Type900 * at2_Content, UCHAR DevType);
static int			DNSHoQuery(char* pCommandMsg, char* pResponseMsg, int* pnResponseLen);

static	CHAR *g_pAMTBuf = NULL;
static	INT		  g_nAMTLen = 0;

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BacpNetCtrlInit
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			Network control intialize
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				bMaster		[IN]		BOOL
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
BacpNetCtrlInit(BOOL bMaster)
{
//	EthResolvInfo	ResolvInfo;
//	BOOL			bRetCode;

	NetCtrlRscInit(&NetCtrlRunMan.CtrlCltRun, TRUE);

	NetCtrlRunMan.CacheHead.pHead	= NULL;
	NetCtrlRunMan.CacheHead.pTail	= NULL;

	// Load resolve configuration from file
	NetCtrlLoadResolv(&NetCtrlRunMan.CacheHead);

	NetCtrlLoadIP();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BacpNetCtrlExit
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			Network control exit
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				void
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
BacpNetCtrlExit()
{
	NetCtrlFreeMXList(&NetCtrlRunMan.CacheHead);
	NetCtrlRscExit(&NetCtrlRunMan.CtrlCltRun);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetCtrlFreeMXList
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			Free buffer stored in list
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pHead		[IN]		MXListHead*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
NetCtrlFreeMXList(MXListHead* pHead)
{
	MXList*					pNext;
	MXList*					pCur;

	pNext = pHead->pHead;

	while (pNext != NULL)
	{
		pCur = pNext;
		pNext = pNext->pNext;
		free(pCur);
#ifdef BACP_NET_CTRL_MEM_DEBUG
		printf("Eth: free memory %08X\n", (unsigned int) pCur);
#endif
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetCtrlRscInit
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			Network control resource intialize
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pRun		[IN/OUT]	BacpNetCtrlRun*
**				bUseSock	[IN]		BOOL
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
NetCtrlRscInit(BacpNetCtrlRun* pRun, BOOL bUseSock)
{
	int						nBlockFlag	= 1;
	struct sockaddr_in		LocalAddr;
	int						nLocalAddrLen	= sizeof (struct sockaddr_in);
	
	LocalAddr.sin_family		= AF_INET;
	LocalAddr.sin_addr.s_addr	= htonl(INADDR_ANY);
	
	pRun->PS.bNeedAck			= FALSE;
	pRun->PS.nCurSeq			= 0;
	if (bUseSock)
	{
		pRun->PS.SockFd.bUseSock	= TRUE;
		pRun->PS.SockFd.Fd.Sock		= socket(AF_INET, SOCK_DGRAM, 0);
		LocalAddr.sin_port			= htons(CLT_PORT_CTRL);
		bind(pRun->PS.SockFd.Fd.Sock, (struct sockaddr*) &LocalAddr, nLocalAddrLen);
		ioctl(pRun->PS.SockFd.Fd.Sock, FIONBIO, &nBlockFlag);
		pRun->PS.nDestUDPPort		= SVR_PORT_CTRL;
		
		pRun->SP.SockFd.bUseSock	= TRUE;
		pRun->SP.SockFd.Fd.Sock		= socket(AF_INET, SOCK_DGRAM, 0);
		LocalAddr.sin_port			= htons(SVR_PORT_CTRL);
		bind(pRun->SP.SockFd.Fd.Sock, (struct sockaddr*) &LocalAddr, nLocalAddrLen);
		ioctl(pRun->SP.SockFd.Fd.Sock, FIONBIO, &nBlockFlag);
	}
	else
	{
		pRun->PS.SockFd.bUseSock	= FALSE;
		pRun->SP.SockFd.bUseSock	= FALSE;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetCtrlRscExit
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			Network control resource exit
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pRun		[IN/OUT]	BacpNetCtrlRun*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
NetCtrlRscExit(BacpNetCtrlRun* pRun)
{
	if (pRun->PS.SockFd.bUseSock)
	{
		close(pRun->PS.SockFd.Fd.Sock);
	}

	if (pRun->SP.SockFd.bUseSock)
	{
		close(pRun->SP.SockFd.Fd.Sock);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetCtrlLoadResolv
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			Network control load resolve items to cache
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pHead		[IN/OUT]	MXListHead*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
NetCtrlLoadResolv(MXListHead* pHead)
{
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetCtrlResolvCacheFind
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			find the resolve information from cache
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pHead		[IN]		MXListHead*
**				pResolvInfo	[IN/OUT]	EthResolvInfo*
**	RETURNED VALUE:	
**				TRUE if find
**	NOTES:
**			
*/
static BOOL
NetCtrlResolvCacheFind(const MXListHead* pHead, EthResolvInfo* pResolvInfo)
{
	BOOL					bFind	= FALSE;
	MXList*					pNext;
	MXList*					pCur;
	ResolvItem*				pResolv	= NULL;
	int						i;

	pNext = pHead->pHead;

	while (pNext != NULL)
	{
		pCur = pNext;
		pNext = pNext->pNext;

		pResolv = (ResolvItem*) pCur;

		if (HO_QUERY_METHOD_CODE == pResolvInfo->nQueryMethod)
		{
			if (strcmp(pResolvInfo->szDevCode, pResolv->szDevCode) == 0)
			{
				bFind = TRUE;
				break;
			}
		}
		else if (HO_QUERY_METHOD_IP == pResolvInfo->nQueryMethod)
		{
			for (i = 0; i < pResolv->nIPIDCnt; i++)
			{
				if (pResolv->IPIDArr[i].nIPAddr == pResolvInfo->nIP)
				{
					bFind = TRUE;
					break;
				}
			}
			
			if (bFind)
			{
				break;
			}
		}
	}
	
	if (bFind)
	{
		strcpy(pResolvInfo->szDevCode, pResolv->szDevCode);
		strcpy(pResolvInfo->szName, pResolv->szName);
		strcpy(pResolvInfo->szAddr, pResolv->szAddr);
		for (i = 0; i < pResolv->nIPIDCnt; i++)
		{
			pResolvInfo->IPIDArr[i].dwID = pResolv->IPIDArr[i].dwID;
			pResolvInfo->IPIDArr[i].nIPAddr = pResolv->IPIDArr[i].nIPAddr;
		}
		pResolvInfo->nIPIDCnt = pResolv->nIPIDCnt;
		pResolvInfo->nType = pResolv->nType;
	}

	return bFind;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetCtrlSendPS
**	AUTHOR:			Jerry Huang
**	DATE:			16 - Nov - 2006
**
**	DESCRIPTION:	
**			PS send frame
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pPS			[IN]		BacpNetCtrlPSRun*
**				pAppFrm		[IN]		unsigned char*
**				nAppFrmLen	[IN]		int
**				nDestIPAddr	[IN]		unsigned int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
NetCtrlSendPS(BacpNetCtrlPSRun* pPS, unsigned char* pAppFrm, int nAppFrmLen, unsigned int nDestIPAddr)
{
	struct sockaddr_in		RemoteAddr;
	int						nRemoteAddrLen;
	unsigned char			NetFrm[MAX_PS_NET_FRM_LEN];
	int						nNetFrmLen	= 0;

	nNetFrmLen = PckBacpNetRqtEx(pAppFrm, nAppFrmLen, FALSE, pPS->nCurSeq, NetFrm);
	pPS->nDestIPAddr = nDestIPAddr;
	
	if (pPS->SockFd.bUseSock)
	{
		RemoteAddr.sin_family		= AF_INET;
		RemoteAddr.sin_addr.s_addr	= htonl(pPS->nDestIPAddr);
		RemoteAddr.sin_port			= htons(pPS->nDestUDPPort);
		nRemoteAddrLen = sizeof (struct sockaddr_in);	
		sendto(	pPS->SockFd.Fd.Sock, 
			NetFrm, nNetFrmLen, 0, 
			(struct sockaddr*) &RemoteAddr, nRemoteAddrLen);
#ifdef BACP_NET_CTRL_PS_DEBUG
		if (nNetFrmLen > 0)
		{
			printf("Eth: Ctrl Send, P -> S, ip %s, port %d\n", 
				inet_aton(RemoteAddr.sin_addr),
				ntohs(RemoteAddr.sin_port));
		}
#endif
	}

	pPS->nCurSeq++;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetCtrlRcvSP
**	AUTHOR:			Jerry Huang
**	DATE:			16 - Nov - 2006
**
**	DESCRIPTION:	
**			SP receive frame
**
**	ARGUMENTS:	ARGNAME			DRIECTION	TYPE	DESCRIPTION
**				pSP				[IN]		BacpNetCtrlSPRun*
**				pAppFrm			[OUT]		unsigned char*
**				nMaxAppFrmLen	[IN]	int
**				pnDestIPAddr	[OUT]		unsigned int*
**	RETURNED VALUE:	
**				Application frame length
**	NOTES:
**			
*/
static int
NetCtrlRcvSP(BacpNetCtrlSPRun* pSP, unsigned char* pAppFrm, int nMaxAppFrmLen, unsigned int* pnSrcIPAddr)
{
	unsigned char			RecvBuf[MAX_SP_NET_FRM_LEN];
	int						nRecvBufLen	= 0;
	struct sockaddr_in		RemoteAddr;
	int						nRemoteAddrLen;
	BacpNetBscFld			BscFld;
	int						nAppFrmLen	= 0;
	
	unsigned char			RspFrm[RSP_LEN_BACP_NET_LY];
	int						nRspFrmLen;

	if (pSP->SockFd.bUseSock)
	{
		nRemoteAddrLen = sizeof (struct sockaddr_in);
		nRecvBufLen = recvfrom(	pSP->SockFd.Fd.Sock, 
			RecvBuf, 
			MAX_SP_NET_FRM_LEN, 
			0,
			(struct sockaddr*) &RemoteAddr,
			&nRemoteAddrLen);

#ifdef BACP_NET_CTRL_SP_DEBUG
		if (nRecvBufLen > 0)
		{
			printf("Eth: Ctrl Recv, S -> P, port %d\n", 
			ntohs(RemoteAddr.sin_port));
		}
#endif

		if ( (nRecvBufLen > 0)							&& 
			 (IsCpltBacpNetFrm(RecvBuf, &nRecvBufLen))	&& 
			 (IsBacpNetRqtFrm(RecvBuf, nRecvBufLen)))
		{
			nAppFrmLen = UnpckBacpNet(RecvBuf, nRecvBufLen, &BscFld, pAppFrm);
			*pnSrcIPAddr = ntohl(RemoteAddr.sin_addr.s_addr);
			// Send network response
			if (IsBacpNetOptConOn(BscFld.Opt))
			{
				nRspFrmLen = PckBacpNetRspEx(BscFld.Seq, RspFrm);
				sendto(	pSP->SockFd.Fd.Sock, 
					RspFrm, nRspFrmLen, 
					0, (struct sockaddr*) &RemoteAddr, nRemoteAddrLen);
			}
		}
	}

	return nAppFrmLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AnalyseResolvData
**	AUTHOR:			Jerry Huang
**	DATE:			16 - Nov - 2006
**
**	DESCRIPTION:	
**			Analyse resolved data information
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMan		[IN]		NetCtrlRunManStruct*
**				pResolvInfo	[OUT]		EthResolvInfo*
**				pData		[IN]		unsigned char*
**				nDataLen	[OUT]		int
**	RETURNED VALUE:	
**				TRUE if ok
**	NOTES:
**			
*/
static BOOL
AnalyseResolvData(NetCtrlRunManStruct* pMan, EthResolvInfo* pResolvInfo, unsigned char* pData, int nDataLen)
{
	BOOL					bRet	= FALSE;
	unsigned char			nNameLen ;
	int						nNameLenPos;
	unsigned char			nAddrLen;
	int						nAddrLenPos;
	unsigned char			nCodeLen;
	int						nCodeLenPos;
	unsigned char			nIPCnt;
	int						nIPCntPos;
	int						nTypePos;
	int						i;
	ResolvItem*				pResolv;
	char					szTemp[20] = {0};

	if (HO_QUERY_NOT_FIND == pData[0])
	{
		printf("the dns result is no find...\n");
		return bRet;
	}
	
	nNameLenPos = HO_QUERY_RESULT_LEN;
	nNameLen = *(pData + nNameLenPos);

	pResolvInfo->szName[0] =0;
	if (nNameLen > 0)
	{
		memcpy(pResolvInfo->szName, (pData + nNameLenPos + 1), nNameLen);
		*(pResolvInfo->szName + nNameLen) = '\0';
	}

	nAddrLenPos = HO_QUERY_RESULT_LEN + 
		HO_QUERY_NAME_LEN_LEN + nNameLen;
	nAddrLen = *(pData + nAddrLenPos);
	// Skip Address field
	pResolvInfo->szAddr[0] = 0;
	if (nAddrLen > 0)
	{
		memcpy(pResolvInfo->szAddr, (pData + nAddrLenPos + 1), nAddrLen);
		*(pResolvInfo->szAddr + nAddrLen) = '\0';
	}	

	nCodeLenPos = HO_QUERY_RESULT_LEN + 
		HO_QUERY_NAME_LEN_LEN + nNameLen + 
		HO_QUERY_ADDR_LEN_LEN + nAddrLen;
	nCodeLen = *(pData + nCodeLenPos);

//	pResolvInfo->szDevCode[0] = 0;
	if (pResolvInfo->nQueryMethod == HO_QUERY_METHOD_CODE)
	{
		if (nCodeLen > 0)
		{
	//		memcpy(pResolvInfo->szDevCode, (pData + nCodeLenPos + 1), nCodeLen);
			memcpy(szTemp, (pData + nCodeLenPos + 1), nCodeLen);
			if (strcmp(pResolvInfo->szDevCode, szTemp))
			{
				printf("the code:%s do not equal the query code:%s...\n",szTemp, pResolvInfo->szDevCode);
				return bRet;
			}
		//	*(pResolvInfo->szDevCode + nCodeLen) = '\0';
		}		
	}
	else
	{
		pResolvInfo->szDevCode[0] = 0;
		if (nCodeLen > 0)
		{
			memcpy(pResolvInfo->szDevCode, (pData + nCodeLenPos + 1), nCodeLen);
			*(pResolvInfo->szDevCode + nCodeLen) = '\0';
		}
	}


	nIPCntPos = HO_QUERY_RESULT_LEN + 
		HO_QUERY_NAME_LEN_LEN + nNameLen + 
		HO_QUERY_ADDR_LEN_LEN + nAddrLen +
		HO_QUERY_CODE_LEN_LEN + nCodeLen;
	nIPCnt = *(pData + nIPCntPos);
	for (i = 0; i < nIPCnt; i++)
	{
		if (i >= MAX_IPID_NUM_PER_ETH_DEVICE)
		{
			continue;
		}
		pResolvInfo->IPIDArr[i].nIPAddr = MAKEDWORD(*(pData + nIPCntPos + 1 + i * 4 + 0), *(pData + nIPCntPos + 1 + i * 4 + 1), *(pData + nIPCntPos + 1 + i * 4 + 2), *(pData + nIPCntPos + 1 + i * 4 + 3));
		pResolvInfo->IPIDArr[i].dwID = i;
		if (HO_QUERY_METHOD_IP == pResolvInfo->nQueryMethod)
		{
			if (pResolvInfo->nIP == pResolvInfo->IPIDArr[i].nIPAddr)
			{
				bRet = TRUE;
			}
		}
	}

	if (HO_QUERY_METHOD_IP == pResolvInfo->nQueryMethod && !bRet)
	{
		return bRet;
	}

	pResolvInfo->nIPIDCnt = i;

	nTypePos = HO_QUERY_RESULT_LEN + 
		HO_QUERY_NAME_LEN_LEN + nNameLen + 
		HO_QUERY_ADDR_LEN_LEN + nAddrLen +
		HO_QUERY_CODE_LEN_LEN + nCodeLen + 
		HO_QUERY_IPCNT_LEN + nIPCnt * 4;
	pResolvInfo->nType = *(pData + nTypePos);

	pResolv = MXNew(ResolvItem);
	if (pResolv != NULL)
	{
#ifdef BACP_NET_CTRL_MEM_DEBUG
		printf("Eth: malloc memory %08X\n", (unsigned int) pResolv);
#endif

		strcpy(pResolv->szName, pResolvInfo->szName);
		strcpy(pResolv->szAddr, pResolvInfo->szAddr);
		strcpy(pResolv->szDevCode, pResolvInfo->szDevCode);
		for (i = 0; i < pResolvInfo->nIPIDCnt; i++)
		{
			pResolv->IPIDArr[i].nIPAddr = pResolvInfo->IPIDArr[i].nIPAddr;
			pResolv->IPIDArr[i].dwID = pResolvInfo->IPIDArr[i].dwID;
		}
		pResolv->nIPIDCnt = pResolvInfo->nIPIDCnt;
		pResolv->nType = pResolvInfo->nType;
		pResolv->bForever = TRUE;

		MXListAdd(&pMan->CacheHead, (MXList*) pResolv);
	}

#ifdef	BACP_NET_CTRL_RESOLV_RESULT
	PrintResolvInfo(pResolvInfo);
#endif

	bRet = TRUE;
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetCtrlResolvNet
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			find the resolve information from name server
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMan		[IN]		NetCtrlRunManStruct*
**				pResolvInfo	[IN/OUT]	EthResolvInfo*
**	RETURNED VALUE:	
**				TRUE if find
**	NOTES:
**			
*/
static BOOL
NetCtrlResolvNet(NetCtrlRunManStruct* pMan, EthResolvInfo* pResolvInfo)
{
	int						i = 0;
	BOOL					bRet	= FALSE;
	BacpNetNameServer*		pNameServer = NULL;

	unsigned char			AppFrm[MAX_APP_FRM_LEN] = {0};
	int						nAppFrmLen	= 0;
	unsigned char			Data[HO_QUERY_METHOD_CODE_DATA_LEN] = {0};
	int						nDataLen	= 0;

	DWORD					dwSendTick = 0;
	DWORD					dwCurTick = 0;

	unsigned int			nSrcIPAddr = 0;

	char*					pSrcDev = NULL;
	char*					pDestDev = NULL;
	unsigned short			nFunCode = 0;
	unsigned char*			pRcvData = NULL;
	int						nRcvDataLen = 0;

	char					szDevCode[MAX_LEN_DEV_CODE + 1]	=	{0};

	unsigned int			nInIPAddr = 0;
	struct sockaddr_in		RemoteAddr;
	BOOL			bQuit = FALSE;
	memset(Data, 0, HO_QUERY_METHOD_CODE_DATA_LEN);

	if (HO_QUERY_METHOD_CODE == pResolvInfo->nQueryMethod)
	{
		*(Data + nDataLen) = HO_QUERY_METHOD_CODE;
		nDataLen += 1;
		strcpy((char*) (Data + nDataLen), pResolvInfo->szDevCode);
		nDataLen += MAX_LEN_DEV_CODE;
	}
	else 
	{
		*(Data + nDataLen) = HO_QUERY_METHOD_IP;
		nDataLen += 1;
		nInIPAddr = htonl(pResolvInfo->nIP);
		memcpy((Data + nDataLen), &nInIPAddr, sizeof (nInIPAddr));
		nDataLen += sizeof (nInIPAddr);
	}

	for (i = 0; i < pMan->CtrlCltRun.nNameServerCnt; i++)
	{
		do {
			nDataLen = sizeof (struct sockaddr_in);
			nAppFrmLen = recvfrom(pMan->CtrlCltRun.SP.SockFd.Fd.Sock, 
				AppFrm, 
				MAX_SP_NET_FRM_LEN, 
				0,
				(struct sockaddr*) &RemoteAddr,
				&nDataLen);	
			if (nAppFrmLen <= 0)
			{
				break;
			}
		} while(TRUE);

		bQuit = FALSE;
		pNameServer = pMan->CtrlCltRun.NameServer + i;
		nAppFrmLen = PckBacpApp(	FC_HO_QUERY, 
			szDevCode, 
			pNameServer->szDevCode, 
			Data, nDataLen, AppFrm);
	
		NetCtrlSendPS(	&pMan->CtrlCltRun.PS, 
			AppFrm, nAppFrmLen, 
			pNameServer->nIPAddr);

#ifdef BACP_NET_CTRL_RESOLV_DEBUG
	printf("Eth: Ctrl, send FC_HO_QUERY command to MC i =%d.\n", i);	
#endif

		dwSendTick = GetTickCount();

		do {
				do 
				{
					nAppFrmLen = NetCtrlRcvSP(	&pMan->CtrlCltRun.SP, 
						AppFrm, MAX_APP_FRM_LEN, &nSrcIPAddr);
					if (nAppFrmLen > 0) 
					{ 
						break; 
					}
					dwCurTick = GetTickCount();
					if ( dwCurTick - dwSendTick > HO_QUERY_TIMEOUT) 
					{
						bQuit =	TRUE;
						break;
					}
				} while(TRUE);
			
				if (nAppFrmLen <= 0)
				{
					bQuit = TRUE;
				}
				
				if (bQuit)
				{
					break;
				}
				UnpckBacpAppEx(	&nFunCode, 
					&pSrcDev, &pDestDev, &pRcvData, 
					&nRcvDataLen, AppFrm, nAppFrmLen);
				if (FC_ACK_HO_QUERY == nFunCode)
				{
					bRet = AnalyseResolvData(pMan, pResolvInfo, pRcvData, nRcvDataLen);
					if (bRet)
					{
						bQuit = TRUE;
						break;
					}
				}			
		} while(!bQuit);
		
	}
	
	return bRet;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetCtrlResolvNet
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			find the resolve information from name server
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMan		[IN]		NetCtrlRunManStruct*
**				pResolvInfo	[IN/OUT]	EthResolvInfo*
**	RETURNED VALUE:	
**				TRUE if find
**	NOTES:
**			
*/

BOOL
NetReqModuleCount(BYTE nType)
{
	int						i;
//	int						iLoop = 0;
	BOOL					bRet	= FALSE;
	BacpNetNameServer*		pNameServer = NULL;
	NetCtrlRunManStruct* pMan = &NetCtrlRunMan;

	unsigned char			AppFrm[MAX_APP_FRM_LEN] = {0};
	int						nAppFrmLen	= 0;
	unsigned char			Data[HO_QUERY_METHOD_CODE_DATA_LEN] = {0};
	int						nDataLen	= 0;

	DWORD					dwSendTick = 0;
	DWORD					dwCurTick = 0;

	unsigned int			nSrcIPAddr = 0;

	char*					pSrcDev = NULL;
	char*					pDestDev = NULL;
	unsigned short			nFunCode = 0;
	unsigned char*			pRcvData = NULL;
	int						nRcvDataLen = 0;

	char					szDevCode[MAX_LEN_DEV_CODE + 1]	=	{0};


	memset(Data, 0, HO_QUERY_METHOD_CODE_DATA_LEN);

	*Data = nType;
	nDataLen = 1;

	for (i = 0; i < pMan->CtrlCltRun.nNameServerCnt; i++)
	{
		pNameServer = pMan->CtrlCltRun.NameServer + i;
		nAppFrmLen = PckBacpApp(	FC_HO_GET_COUNT, 
			szDevCode, 
			pNameServer->szDevCode, 
			Data, nDataLen, AppFrm);
#ifdef CALL_INFO_DNS_DEBUG
		printf("the FC:%x, devcode:%s, destCode:%s,\n", FC_HO_GET_COUNT, szDevCode, pNameServer->szDevCode);
		for(iLoop = 0; iLoop < nDataLen; iLoop++)
		{
			printf("%02x  ", Data[iLoop]);
		}
#endif



		NetCtrlSendPS(	&pMan->CtrlCltRun.PS, 
			AppFrm, nAppFrmLen, 
			pNameServer->nIPAddr);
#ifdef CALL_INFO_DNS_DEBUG
		printf("the dns ip:%x.the port:%x\n", pNameServer->nIPAddr, pMan->CtrlCltRun.PS.nDestUDPPort);
#endif
		dwSendTick = GetTickCount();
		do 
		{
			nAppFrmLen = NetCtrlRcvSP(	&pMan->CtrlCltRun.SP, 
				AppFrm, MAX_APP_FRM_LEN, &nSrcIPAddr);
			if (nAppFrmLen > 0) 
			{ 
				break; 
			}
			dwCurTick = GetTickCount();
		} while(dwCurTick - dwSendTick <= HO_QUERY_TIMEOUT);

		if (nAppFrmLen <= 0)
		{
			continue;
		}
		
		UnpckBacpAppEx(	&nFunCode, 
			&pSrcDev, &pDestDev, &pRcvData, 
			&nRcvDataLen, AppFrm, nAppFrmLen);
		if (FC_ACK_HO_GET_COUNT == nFunCode)
		{
//			bRet = GetModuleInfo(pRcvData, nRcvDataLen);
			if (bRet)
			{
				break;
			}
		}
	}
	
	return bRet;
}


BOOL
NetReqModuleInfo(BYTE nType, unsigned short iIndex, BYTE nCnt, BYTE nOrderBy)
{
	int						i;
	BOOL					bRet	= FALSE;
	BacpNetNameServer*		pNameServer = NULL;
	NetCtrlRunManStruct* pMan = &NetCtrlRunMan;
	
	unsigned char			AppFrm[MAX_APP_FRM_LEN] = {0};
	int						nAppFrmLen	= 0;
	unsigned char			Data[HO_QUERY_METHOD_CODE_DATA_LEN] = {0};
	int						nDataLen	= 0;
	
	DWORD					dwSendTick = 0;
	DWORD					dwCurTick = 0;
	
	unsigned int			nSrcIPAddr = 0;
	
	char*					pSrcDev = NULL;
	char*					pDestDev = NULL;
	unsigned short			nFunCode = 0;
	unsigned char*			pRcvData = NULL;
	int						nRcvDataLen = 0;
	
	char					szDevCode[MAX_LEN_DEV_CODE + 1]	=	{0};
	
	
	memset(Data, 0, HO_QUERY_METHOD_CODE_DATA_LEN);
	
	*((unsigned short *)Data) = iIndex;
	*(Data + 2) = nCnt;
	*(Data + 3) = nType;
	*(Data + 4) = nOrderBy;
	nDataLen = 5;
#ifdef CALL_INFO_DNS_DEBUG
	printf("### query the data: ncnt;%d, index:%d, ntype:%d, orderby:%d.\n", nCnt, iIndex, nType, nOrderBy);
#endif
	for (i = 0; i < pMan->CtrlCltRun.nNameServerCnt; i++)
	{
		pNameServer = pMan->CtrlCltRun.NameServer + i;
		nAppFrmLen = PckBacpApp(	FC_HO_GET, 
			szDevCode, 
			pNameServer->szDevCode, 
			Data, nDataLen, AppFrm);
		
		NetCtrlSendPS(	&pMan->CtrlCltRun.PS, 
			AppFrm, nAppFrmLen, 
			pNameServer->nIPAddr);
		
		
		dwSendTick = GetTickCount();
		do 
		{
			nAppFrmLen = NetCtrlRcvSP(	&pMan->CtrlCltRun.SP, 
				AppFrm, MAX_APP_FRM_LEN, &nSrcIPAddr);
			if (nAppFrmLen > 0) 
			{ 
				break; 
			}
			dwCurTick = GetTickCount();
		} while(dwCurTick - dwSendTick <= HO_QUERY_TIMEOUT);
		
		if (nAppFrmLen <= 0)
		{
			continue;
		}
		
		UnpckBacpAppEx(	&nFunCode, 
			&pSrcDev, &pDestDev, &pRcvData, 
			&nRcvDataLen, AppFrm, nAppFrmLen);
		if (FC_ACK_HO_GET == nFunCode)
		{
//			bRet = GetModuleData(pRcvData, nRcvDataLen);
			if (bRet)
			{
				break;
			}
		}
	}
	
	return bRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BacpNetCtrlResolv
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			find the resolve information
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pResolvInfo	[IN/OUT]	EthResolvInfo*
**	RETURNED VALUE:	
**				TRUE if find
**	NOTES:
**			
*/


BOOL
BacpNetCtrlResolv(EthResolvInfo* pResolvInfo)
{
	BOOL					bFind	= FALSE;

	// Judge whether it is self?

	bFind = NetCtrlResolvCacheFind(&NetCtrlRunMan.CacheHead, pResolvInfo);
	if (bFind)
	{
		return bFind;
	}

	bFind = NetCtrlResolvNet(&NetCtrlRunMan, pResolvInfo);

	if (bFind)
	{
		return bFind;
	}

	return bFind;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PrintResolvInfo
**	AUTHOR:			Jerry Huang
**	DATE:			16 - Nov - 2006
**
**	DESCRIPTION:	
**			Print resolve infor
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pResolvInfo	[IN/OUT]	EthResolvInfo*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
/*
static void
PrintResolvInfo(EthResolvInfo* pResolvInfo)
{
	int			i;

	printf("Eth: DNS\n"
		"	Device Code %s\n"
		"	Name %s\n" 
		"	Type %d\n", 
		pResolvInfo->szDevCode,
		pResolvInfo->szName,
		pResolvInfo->nType);

	for (i = 0; i < pResolvInfo->nIPIDCnt; i++)
	{
		printf("	IP %02d.%02d.%02d.%02d, ID %ld\n",
			(int) ((pResolvInfo->IPIDArr[i].nIPAddr >> 24) & 0xFF), 
			(int) ((pResolvInfo->IPIDArr[i].nIPAddr >> 16) & 0xFF), 
			(int) ((pResolvInfo->IPIDArr[i].nIPAddr >> 8) & 0xFF), 
			(int) (pResolvInfo->IPIDArr[i].nIPAddr & 0xFF), 
			pResolvInfo->IPIDArr[i].dwID);
	}
}
*/
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PrintResolvCache
**	AUTHOR:			Jerry Huang
**	DATE:			17 - Nov - 2006
**
**	DESCRIPTION:	
**			Print resolv cache
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pHead		[IN]		MXListHead*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
/*
static void
PrintResolvCache(MXListHead* pHead)
{
	MXList*					pNext;
	MXList*					pCur;
	ResolvItem*				pResolv;
	int						i;

	pNext = pHead->pHead;

	while (pNext != NULL)
	{
		pCur = pNext;
		pNext = pNext->pNext;

		pResolv = (ResolvItem*) pCur;
		
		printf("Eth: DNS Cache\n"
			"	Device Code %s\n"
			"	Name %s\n" 
			"	Type %d\n", 
			pResolv->szDevCode,
			pResolv->szName,
			pResolv->nType);

		for (i = 0; i < pResolv->nIPIDCnt; i++)
		{
			printf("	IP %02d.%02d.%02d.%02d, ID %ld\n",
				(int) ((pResolv->IPIDArr[i].nIPAddr >> 24) & 0xFF), 
				(int) ((pResolv->IPIDArr[i].nIPAddr >> 16) & 0xFF), 
				(int) ((pResolv->IPIDArr[i].nIPAddr >> 8) & 0xFF), 
				(int) (pResolv->IPIDArr[i].nIPAddr & 0xFF), 
				pResolv->IPIDArr[i].dwID);
		}
	}
}
*/
extern UCHAR g_bDefaultLocalIPAddr[4];

DWORD	
GetSelfIP()
{
	return g_SysConfig.IPAddr;
	//DWORD SelfIP = 0;
	//memcpy(&SelfIP, g_bDefaultLocalIPAddr, sizeof(DWORD));
	//return SelfIP;
	//return NetCtrlRunMan.CtrlCltRun.nSelfIPAddr;
}

BOOL
GetSelfInfo(unsigned int* pIPAddr, char* pszDevCode)
{
	EthResolvInfo		ResolvInfo;

	pszDevCode[0] = 0;
	*pIPAddr = NetCtrlRunMan.CtrlCltRun.nSelfIPAddr;

	ResolvInfo.nIP = NetCtrlRunMan.CtrlCltRun.nSelfIPAddr;
	ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;

	if (BacpNetCtrlResolv(&ResolvInfo))
	{
		*pIPAddr = NetCtrlRunMan.CtrlCltRun.nSelfIPAddr;
		strcpy(pszDevCode, ResolvInfo.szDevCode);
	}

	return TRUE;
}


BOOL
GetMCInfo(unsigned int* pIPAddr, char* pszDevCode)
{
	EthResolvInfo		ResolvInfo;

	pszDevCode[0] = 0;
	*pIPAddr = NetCtrlRunMan.CtrlCltRun.nMCIPAddr;

	ResolvInfo.nIP = NetCtrlRunMan.CtrlCltRun.nMCIPAddr;
	ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;

	if (BacpNetCtrlResolv(&ResolvInfo))
	{
		*pIPAddr = ResolvInfo.IPIDArr[0].nIPAddr;
		strcpy(pszDevCode, ResolvInfo.szDevCode);
	}

	return TRUE;
}

BOOL
GetGMInfo(unsigned int* pIPAddr, char* pszDevCode)
{
	EthResolvInfo		ResolvInfo;

	pszDevCode[0] = 0;
	*pIPAddr = NetCtrlRunMan.CtrlCltRun.nGMIPAddr;

	ResolvInfo.nIP = NetCtrlRunMan.CtrlCltRun.nGMIPAddr;
	ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;

	if (BacpNetCtrlResolv(&ResolvInfo))
	{
		*pIPAddr = ResolvInfo.IPIDArr[0].nIPAddr;
		strcpy(pszDevCode, ResolvInfo.szDevCode);
	}

	return TRUE;
}

static void
NetCtrlLoadIP()
{
	NetCtrlRunMan.CtrlCltRun.nNameServerCnt = 1;
}

//extern void	EthHVLog(char* pMsg);

void
NetCtrlUpdateTime()
{
	int						i;
	BacpNetNameServer*		pNameServer;

	unsigned char			AppFrm[MAX_APP_FRM_LEN];
	int						nAppFrmLen	= 0;

	DWORD					dwSendTick;
	DWORD					dwCurTick;

	unsigned int			nSrcIPAddr;

	char*					pSrcDev;
	char*					pDestDev;
	unsigned short			nFunCode;
	unsigned char*			pRcvData;
	int						nRcvDataLen;

	char					szDevCode[MAX_LEN_DEV_CODE + 1]	=	{0};

	NetCtrlRunManStruct*	pMan		= &NetCtrlRunMan;

	for (i = 0; i < pMan->CtrlCltRun.nNameServerCnt; i++)
	{
		pNameServer = pMan->CtrlCltRun.NameServer + i;

		if ((g_TalkInfo.bMCStatus) && (0 == i)) 
		{
			pNameServer->nIPAddr = g_TalkInfo.dwMCIP;
		}

		nAppFrmLen = PckBacpApp(	FC_DTM_GET, 
			szDevCode, 
			szDevCode, 
			NULL, 0, AppFrm);

#ifdef BACP_NET_CTRL_TIME_DEBUG
		printf("Eth: Ctrl, send FC_DTM_GET command to MC\n");
#endif
	
		NetCtrlSendPS(	&pMan->CtrlCltRun.PS, 
			AppFrm, nAppFrmLen, 
			pNameServer->nIPAddr);

		dwSendTick = GetTickCount();
		do 
		{
			nAppFrmLen = NetCtrlRcvSP(	&pMan->CtrlCltRun.SP, 
				AppFrm, MAX_APP_FRM_LEN, &nSrcIPAddr);
			if (nAppFrmLen > 0) 
			{ 
				break; 
			}
			dwCurTick = GetTickCount();
		} while(dwCurTick - dwSendTick <= DTM_GET_TIMEOUT);

		if (nAppFrmLen <= 0)
		{
#ifdef BACP_NET_CTRL_TIME_DEBUG
			printf("Eth: Ctrl, not get FC_ACK_DTM_GET command from MC\n");
#endif
			continue;
		}
		
		UnpckBacpAppEx(	&nFunCode, 
			&pSrcDev, &pDestDev, &pRcvData, 
			&nRcvDataLen, AppFrm, nAppFrmLen);
		if (FC_ACK_DTM_GET == nFunCode)
		{
#ifdef BACP_NET_CTRL_TIME_DEBUG
			printf("Eth: Ctrl, get FC_ACK_DTM_GET command from MC\n");
#endif		
			break;
		}
	}
}
/*
void
SetTimeFromMC(unsigned char* pTime)
{
	struct tm		ltm;	// local time
	time_t			Sec;

	if (pTime == NULL)
	{
		return;
	}

	memset(&ltm, 0, sizeof (ltm));

	ltm.tm_year = pTime[0] + 2000 - 1900;
	ltm.tm_mon = pTime[1] - 1;
	ltm.tm_mday = pTime[2];
	ltm.tm_hour = pTime[3];
	ltm.tm_min = pTime[4];
	ltm.tm_sec = pTime[5];
	
	Sec = mktime(&ltm);

//	MoxSetRealTime(Sec, 0);
}
*/
BOOL
NetReqAddrBookCount(void * pNode, PBYTE* pCode, BYTE bCodeLen)
{
	int						i;
//	int						iLoop = 0;
	BOOL					bRet	= FALSE;
	BacpNetNameServer*		pNameServer = NULL;
	NetCtrlRunManStruct* pMan = &NetCtrlRunMan;
	
	unsigned char			AppFrm[MAX_APP_FRM_LEN] = {0};
	int						nAppFrmLen	= 0;
	unsigned char			Data[HO_QUERY_METHOD_CODE_DATA_LEN] = {0};
	int						nDataLen	= 0;
	
	DWORD					dwSendTick = 0;
	DWORD					dwCurTick = 0;
	
	unsigned int			nSrcIPAddr = 0;
	
	char*					pSrcDev = NULL;
	char*					pDestDev = NULL;
	unsigned short			nFunCode = 0;
	unsigned char*			pRcvData = NULL;
	int						nRcvDataLen = 0;
	
	char					szDevCode[20]	=	{0};
	
	
	memset(Data, 0, HO_QUERY_METHOD_CODE_DATA_LEN);
	
	memcpy(Data, pCode, bCodeLen);
	nDataLen = 19;
	Data[nDataLen] = HO_QUERY_ADDRBOOK;
	nDataLen += 1;
	
	for (i = 0; i < pMan->CtrlCltRun.nNameServerCnt; i++)
	{
		pNameServer = pMan->CtrlCltRun.NameServer + i;
		nAppFrmLen = PckBacpApp(	FC_HO_GETCHILDREN_COUNT, 
			szDevCode, 
			pNameServer->szDevCode, 
			Data, nDataLen, AppFrm);
#ifdef AB_DEBUG
		printf("the FC:%x, devcode:%s, destCode:%s,\n", FC_HO_GET_COUNT, szDevCode, pNameServer->szDevCode);
		for(iLoop = 0; iLoop < nDataLen; iLoop++)
		{
			printf("%02x  ", Data[iLoop]);
		}
		printf(" \n");
#endif


		NetCtrlSendPS(	&pMan->CtrlCltRun.PS, 
			AppFrm, nAppFrmLen, 
			pNameServer->nIPAddr);
#ifdef CALL_INFO_DNS_DEBUG
		printf("the dns ip:%x.the port:%x\n", pNameServer->nIPAddr, pMan->CtrlCltRun.PS.nDestUDPPort);
#endif
		dwSendTick = GetTickCount();
		do 
		{
			nAppFrmLen = NetCtrlRcvSP(	&pMan->CtrlCltRun.SP, 
				AppFrm, MAX_APP_FRM_LEN, &nSrcIPAddr);
			if (nAppFrmLen > 0) 
			{ 

				break; 
			}
			dwCurTick = GetTickCount();
		} while(dwCurTick - dwSendTick <= HO_QUERY_TIMEOUT);
		
		if (nAppFrmLen <= 0)
		{
			continue;
		}
		
		UnpckBacpAppEx(	&nFunCode, 
			&pSrcDev, &pDestDev, &pRcvData, 
			&nRcvDataLen, AppFrm, nAppFrmLen);

		if (FC_ACK_HO_GETCHILDRENCOUNT == nFunCode)
		{
//			bRet = GetUserGroupInfo(pNode, pRcvData, pCode, bCodeLen);
			if (bRet)
			{
				break;
			}
		}
	}
	
	return bRet;
}

BOOL
NetReqAddrInfo(void * pNode, unsigned char * pCode, BYTE bCodeLen, unsigned short iIndex, BYTE nCnt, BYTE nOrderBy)
{
	int						i;
	BOOL					bRet	= FALSE;
	BacpNetNameServer*		pNameServer = NULL;
	NetCtrlRunManStruct* pMan = &NetCtrlRunMan;
	
	unsigned char			AppFrm[MAX_APP_FRM_LEN] = {0};
	int						nAppFrmLen	= 0;
	unsigned char			Data[24] = {0};
	int						nDataLen	= 0;
	
	DWORD					dwSendTick = 0;
	DWORD					dwCurTick = 0;
	
	unsigned int			nSrcIPAddr = 0;
	
	char*					pSrcDev = NULL;
	char*					pDestDev = NULL;
	unsigned short			nFunCode = 0;
	unsigned char*			pRcvData = NULL;
	int						nRcvDataLen = 0;
	
	char					szDevCode[MAX_LEN_DEV_CODE + 1]	=	{0};
	
	
	memset(Data, 0, HO_QUERY_METHOD_CODE_DATA_LEN);
	
	memcpy(Data, pCode, bCodeLen);
	nDataLen = MAX_LEN_DEV_CODE;
	*((unsigned short *)(Data + nDataLen)) = iIndex;
	nDataLen += 2;
	*(Data + nDataLen) = nCnt;
	nDataLen += 1;
	*(Data + nDataLen) = HO_QUERY_ADDRBOOK;
	nDataLen += 1;
	*(Data + nDataLen) = nOrderBy;
	nDataLen += 1;

#ifdef AB_DEBUG
	printf("\n AddressBook: query the data: ncnt;%d, index:%d, orderby:%d.\n", nCnt, iIndex,  nOrderBy);
	for(i = 0; i < nDataLen; i++)
	{
		printf("%02x  ", Data[i]);
	}
	printf(" \n");
#endif

	for (i = 0; i < pMan->CtrlCltRun.nNameServerCnt; i++)
	{
		pNameServer = pMan->CtrlCltRun.NameServer + i;
		nAppFrmLen = PckBacpApp(	FC_HO_GETCHILDREN, 
			szDevCode, 
			pNameServer->szDevCode, 
			Data, nDataLen, AppFrm);
		
		NetCtrlSendPS(	&pMan->CtrlCltRun.PS, 
			AppFrm, nAppFrmLen, 
			pNameServer->nIPAddr);
		
		
		dwSendTick = GetTickCount();
		do 
		{
			nAppFrmLen = NetCtrlRcvSP(	&pMan->CtrlCltRun.SP, 
				AppFrm, MAX_APP_FRM_LEN, &nSrcIPAddr);
			if (nAppFrmLen > 0) 
			{ 

				break; 
			}
			dwCurTick = GetTickCount();
		} while(dwCurTick - dwSendTick <= HO_QUERY_TIMEOUT);
		
		if (nAppFrmLen <= 0)
		{
			continue;
		}
		
		UnpckBacpAppEx(	&nFunCode, 
			&pSrcDev, &pDestDev, &pRcvData, 
			&nRcvDataLen, AppFrm, nAppFrmLen);

		if (FC_ACK_HO_GETCHILDREN == nFunCode)
		{
//			bRet = GetAddrUserData(pNode, pRcvData, pCode, bCodeLen);
			if (bRet)
			{
				break;
			}
		}
	}
	
	return bRet;
}


void
CheckMCStatus()
{
	int						i;
	BacpNetNameServer*		pNameServer;
	
	unsigned char			AppFrm[MAX_APP_FRM_LEN];
	int						nAppFrmLen	= 0;
	
	DWORD					dwSendTick;
	DWORD					dwCurTick;
	
	unsigned int			nSrcIPAddr;

//	char*					pSrcDev;
//	char*					pDestDev;
//	unsigned short			nFunCode;
//	unsigned char*			pRcvData;
//	int						nRcvDataLen;
	
	char					szDevCode[MAX_LEN_DEV_CODE + 1]	=	{0};
	
	NetCtrlRunManStruct*	pMan		= &NetCtrlRunMan;

	if (!g_TalkInfo.bMCStatus) 
	{
		return;
	}

	for (i = 0; i < pMan->CtrlCltRun.nNameServerCnt; i++)
	{
		pNameServer = pMan->CtrlCltRun.NameServer + i;
		
		if ((g_TalkInfo.bMCStatus) && (0 == i)) 
		{
			pNameServer->nIPAddr = g_TalkInfo.dwMCIP;
		}

		nAppFrmLen = PckBacpApp(	FC_DTM_GET, 
			szDevCode, 
			szDevCode, 
			NULL, 0, AppFrm);
		
#ifdef BACP_NET_CTRL_TIME_DEBUG
		printf("Eth: Ctrl, send FC_DTM_GET command to MC\n");
#endif
		
		NetCtrlSendPS(	&pMan->CtrlCltRun.PS, 
			AppFrm, nAppFrmLen, 
			pNameServer->nIPAddr);
		
		dwSendTick = GetTickCount();
		do 
		{
			nAppFrmLen = NetCtrlRcvSP(	&pMan->CtrlCltRun.SP, 
				AppFrm, MAX_APP_FRM_LEN, &nSrcIPAddr);
			if (nAppFrmLen > 0) 
			{ 
				break; 
			}
			dwCurTick = GetTickCount();
		} while(dwCurTick - dwSendTick <= DTM_GET_TIMEOUT);
		
		if (nAppFrmLen <= 0)
		{
#ifdef BACP_NET_CTRL_TIME_DEBUG
			printf("Eth: Ctrl, not get FC_ACK_DTM_GET command from MC\n");
#endif
			g_TalkInfo.bMCStatus = FALSE;
			continue;
		}
		g_TalkInfo.bMCStatus = TRUE;		
	}
}

/****************************DNS Service*****************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME: ProcessIcmp
**	AUTHOR:		 Alan Huang
**	DATE:		 19-Oct-2007
**
**	DESCRIPTION:	process the control and management protocol
** 
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			mainly for DNS service by now
**			Risk: not considering the max length of the buffer.
*/

void 
ProcessIcmp()
{
    unsigned char	chResponseMsg[ICMP_BUFF_SIZE] =  {0};
    int		nResponseMsgLen = 0;


//	BOOL					bRet	= FALSE;
//	BacpNetNameServer*		pNameServer = NULL;
	NetCtrlRunManStruct* pMan = &NetCtrlRunMan;
	
	unsigned char			AppFrm[MAX_APP_FRM_LEN] = {0};
	int						nAppFrmLen	= 0;
//	unsigned char			Data[HO_QUERY_METHOD_CODE_DATA_LEN] = {0};
//	int						nDataLen	= 0;	
	
	unsigned int			nSrcIPAddr = 0;
	
	char*					pSrcDev = NULL;
	char*					pDestDev = NULL;
	unsigned short			nFunCode = 0;
	unsigned char*			pRcvData = NULL;
	int						nRcvDataLen = 0;
	
	char					szDevCode[20]	=	{0};

	nAppFrmLen = NetCtrlRcvSP(	&pMan->CtrlCltRun.SP, 
		AppFrm, MAX_APP_FRM_LEN, &nSrcIPAddr);
	
	if (nAppFrmLen > 0)
	{
		UnpckBacpAppEx(	&nFunCode, 
			&pSrcDev, &pDestDev, &pRcvData, 
			&nRcvDataLen, AppFrm, nAppFrmLen);

#ifdef DNS_INFO_DEBUG
		printf("nFunCode = 0x%x\n", nFunCode);
#endif

		switch (nFunCode)
		{
			case FC_HO_QUERY:
			case FC_HO_GET_COUNT:
			case FC_HO_GET:
			{
				ProcessDNSCommand((char*)pRcvData, nRcvDataLen, nFunCode, (char*)chResponseMsg, &nResponseMsgLen);
				break;
			}
			default:
			{
				break;
			}
		}
		if (nResponseMsgLen > 0)
		{
			nFunCode += 0x8000;
			nAppFrmLen = PckBacpApp(nFunCode, 
				szDevCode, 
				szDevCode, 
				chResponseMsg, nResponseMsgLen, AppFrm);			
			
			NetCtrlSendPS(	&pMan->CtrlCltRun.PS, 
				AppFrm, nAppFrmLen, 
				nSrcIPAddr);
 		}		
	}		
}

// for pCommand, start from src and dest, function code and len is not included.
static void
ProcessDNSCommand(char* pCommandMsg, int nCommandMsgLen, WORD nFncCode, char* pResponseMsg, int* pnResponseLen)
{
	DWORD dwATMVersion  = 0;
	UCHAR  AMTHeadItem   = 0;
	DWORD  ATMLen	     = 0;
	AMT_Type800	*at1_Content = NULL;
	AMT_Type900 at2_Content;

	DWORD	dwNexAddr    = 0;
	UCHAR	nCountIP	=	0; 
	
	DWORD	dwItemOffset   = 0;
	int j = 0;
	int	i = 0;
	UCHAR nType = 0;
	WORD SetTypeCount = 0;
	UCHAR	nDeviceTypeAsk = 0;	
	UCHAR	nResult = 1;
	int		nResponseLen = 0;
	// get home owner
	WORD		nhoIndexFrm = 0;
	UCHAR		nhoCount = 0;
	UCHAR		nhoType = 0;
	UCHAR		nhoOrder = 0;	
	int			nRet = 0;

	FILE * fd			= NULL;
	char szName[255]	= {0};
	struct stat s;
	
	strcpy(szName, AMTFILE);
	if ((fd = fopen(szName, "r+")) == NULL)
	{
		printf("DNS: Open AMT file error\n");
		return;
	}
	if ( -1 == stat(AMTFILE, &s))
	{
		printf("AMT file Information error\n");
		return;
	}	
	if (s.st_size > 0)
	{
		g_nAMTLen = s.st_size;
		g_pAMTBuf = (UCHAR*)malloc(g_nAMTLen);
		if (NULL == g_pAMTBuf)
		{
			printf("AMT buffer alloc error\n");
		}
		fread(g_pAMTBuf,g_nAMTLen,1,fd);
	}	
	fclose(fd);
	fd = NULL;

	memcpy(&dwATMVersion, (unsigned char *)(g_pAMTBuf),4);
	memcpy(&ATMLen, (unsigned char *)(g_pAMTBuf + 4),4);
	if(dwATMVersion < ATM_VERSION_4)
	{
		printf("ATM version old\n");
		return;
	}
	if(ATMLen > 0x200000)
	{
		printf("ATM Len error\n");
		return;
	}
	memcpy(&AMTHeadItem,(unsigned char *)(g_pAMTBuf+8),1);
		
	switch (nFncCode)
	{
		case FC_HO_QUERY:
		{
#ifdef DNS_INFO_DEBUG
			printf("FC_HO_QUERY\n");
#endif

			// find AMT content,
			if (AMTHeadItem > 0)
			{
				nResult = DNSHoQuery(pCommandMsg,pResponseMsg, &nResponseLen);
			}
			//parse  MC/GM.
			//parse  Ehv.
			//complete response.			  
			break;
		}
		case FC_HO_GET_COUNT:
		{
			nDeviceTypeAsk = pCommandMsg[0];
#ifdef DNS_INFO_DEBUG
			printf("FC_HO_GET_COUNT\n");
			printf("nDeviceTypeAsk = %d\n", nDeviceTypeAsk);
			printf("AMTHeadItem = %d\n", AMTHeadItem);
#endif
			if (AMTHeadItem > 0)
			{
				for (i = 0; i < AMTHeadItem; i++)
				{
					nType = *((unsigned char *)(g_pAMTBuf+9+i*ATM_HEAD_LEN_V4));		
					memcpy(&SetTypeCount,(unsigned char *)(g_pAMTBuf+9+i*ATM_HEAD_LEN_V4 + 1), 2);
					if (ETH_DEVICE_TYPE_ETH_CGM == nDeviceTypeAsk)
					{
						if (ATM_TYPE_GM == nType)
						{
							nResult = 1;
						}						
					}
/*
					if (ETH_DEVICE_TYPE_ETH_UGM == nDeviceTypeAsk)
					{						
						if (ATM_TYPE_GM == nType)
						{
							nResult = 0;
						}						
					}
*/
					if (ETH_DEVICE_TYPE_EHV == nDeviceTypeAsk)
					{						
						if (ATM_TYPE_EHV == nType)
						{
							nResult = 0;
						}						
					}
					
					if (ETH_DEVICE_TYPE_ETH_UGM == nDeviceTypeAsk)
					{		
						if (ATM_TYPE_EGM == nType)
						{
							nResult = 0;
						}						
					}
					if (ETH_DEVICE_TYPE_MC == nDeviceTypeAsk)
					{		
						if (ATM_TYPE_MC == nType)
						{
							nResult = 0;
						}						
					}
					
					if (0 == nResult)
					{
						break;
					}					
				}								
			}
			
			nResponseLen = 0;
			pResponseMsg[nResponseLen] = nResult;
			nResponseLen++;
			pResponseMsg[nResponseLen] = nDeviceTypeAsk;
			nResponseLen++;		
			// why only UCHAR for ount??? to be check.
			memcpy(&pResponseMsg[nResponseLen], &SetTypeCount, 2);
			
#ifdef DNS_INFO_DEBUG
			printf("type %d, count %d\n", nDeviceTypeAsk, SetTypeCount);				
#endif
			nResponseLen += 2;			
			break;
		}
		case FC_HO_GET:
		{
#ifdef DNS_INFO_DEBUG
			printf("FC_HO_GET nCommandMsgLen = %d\n",nCommandMsgLen);
#endif
			memcpy((CHAR*)&nhoIndexFrm, (CHAR*)&pCommandMsg[0], 2);
#ifdef DNS_INFO_DEBUG
			printf("nhoIndexFrm = %d\n",nhoIndexFrm);
#endif
			nhoCount = pCommandMsg[2];
#ifdef DNS_INFO_DEBUG
			printf("nhoCount = %d\n",nhoCount);
#endif
			nhoType = pCommandMsg[3];
#ifdef DNS_INFO_DEBUG
			printf("nhoType = %d\n",nhoType);
#endif
			nhoOrder = pCommandMsg[4];
#ifdef DNS_INFO_DEBUG
			printf("nhoOrder = %d\n",nhoOrder);
#endif
			// I have no order.
			
			nResponseLen = 6;
			if (ETH_DEVICE_TYPE_ETH_UGM == nhoType)
			{						
				for (i = 0; i < AMTHeadItem; i++)
				{
					nType = *((unsigned char *)(g_pAMTBuf+9+i*ATM_HEAD_LEN_V4));		
					memcpy(&SetTypeCount,(unsigned char *)(g_pAMTBuf+9+i*ATM_HEAD_LEN_V4 + 1), 2);
//					printf("get ho, SetTypeCount = %d\n", SetTypeCount); 
					
					memcpy(&dwItemOffset,(unsigned char *)(g_pAMTBuf+9+i*ATM_HEAD_LEN_V4 + 3),4);
					if (ATM_TYPE_EGM == nType)
					{
						
						if (nhoIndexFrm + nhoCount > SetTypeCount)
						{
							nhoCount = SetTypeCount - nhoIndexFrm;	
						} 
						
#ifdef DNS_INFO_DEBUG
						printf("ATM_TYPE_EGM == nType: nhoCount = %d\n",nhoCount);
#endif
						memcpy((char *)(&at2_Content), AMTBuf+dwItemOffset +8, 28);
						nCountIP = at2_Content.IPCOUNT;
						memcpy((char *)(&at2_Content) + 28, AMTBuf+dwItemOffset +8 + 28, nCountIP * 4);
						memcpy((char *)(&at2_Content) + 108, AMTBuf+dwItemOffset +8 + 28 + nCountIP * 4, 24);
						
						for (j = nhoIndexFrm; j < (nhoIndexFrm + nhoCount); j++)
						{
						//	printf("get one content\n");
// 							at1_Content = (AMT_Type800 *)(g_pAMTBuf+dwItemOffset+8+j*ATM_TYPE1_V4_LEN);														
//							at2_Content = (AMT_Type900 *)(g_pAMTBuf+dwItemOffset+8+j*ATM_TYPE1_V4_LEN);														
							//put one to response,
//							nRet = DNSBuildHo800(&pResponseMsg[nResponseLen], at1_Content, nType);
#ifdef DNS_INFO_DEBUG
							printf("DNSBuildHo900 START\n");
#endif
							nRet = DNSBuildHo900(&pResponseMsg[nResponseLen], &at2_Content, nType);
#ifdef DNS_INFO_DEBUG
							printf("DNSBuildHo900 END nRet = %d\n",nRet);
#endif
							// rearrange the response buffer.	
							nResponseLen += nRet;
							nResult = 0;
							
							if ((j + 1) < (nhoIndexFrm + nhoCount))
							{
								memcpy(&dwNexAddr,&at2_Content.NexAddress,4);
								
								memcpy((char *)(&at2_Content), AMTBuf+dwNexAddr +8, 28);
								nCountIP = at2_Content.IPCOUNT;
								memcpy((char *)(&at2_Content) + 28, AMTBuf+dwNexAddr +8 + 28, nCountIP * 4);
								memcpy((char *)(&at2_Content) + 108, AMTBuf+dwNexAddr +8 + 28 + nCountIP * 4, 24);
							}
							
						}
					}
				}
				//complete the response
			}
			else
			{
				nResult = 1;
			}
			pResponseMsg[0] = nResult;
			pResponseMsg[1] = nhoType;
			pResponseMsg[2] = nhoOrder;
			memcpy(&pResponseMsg[3], &nhoIndexFrm, 2);
			pResponseMsg[5] = nhoCount;
			break;
		}
		default:
		{
			break;
		}
	}
	*pnResponseLen = nResponseLen;	
	free(g_pAMTBuf);
	g_pAMTBuf = NULL;
	g_nAMTLen = 0;
}

static int
DNSBuildHo800(char* pResponseMsg, AMT_Type800 * at1_Content, UCHAR DevType)
{
	char* 	pResponseMsgStart = pResponseMsg;
	// name
	*pResponseMsg = strlen((char*)at1_Content->Name);	
	pResponseMsg++;
	strcpy(pResponseMsg, (char*)at1_Content->Name);
	pResponseMsg += strlen((char*)at1_Content->Name); 
	// addreee, no address, use name as address
	*pResponseMsg = strlen((char*)at1_Content->Name);
	pResponseMsg++;
	strcpy(pResponseMsg, (char*)at1_Content->Name);
	pResponseMsg += strlen((char*)at1_Content->Name); 
	// call code
	*pResponseMsg = strlen((char*)at1_Content->DevCode);
	pResponseMsg++;
	strcpy(pResponseMsg, (char*)at1_Content->DevCode);
	pResponseMsg += strlen((char*)at1_Content->DevCode); 
	// IP
	*pResponseMsg = 1;
	pResponseMsg++;
	memcpy(pResponseMsg, (char*)at1_Content->DevIP,4);
	pResponseMsg += 4;

	// dev type
	if (ATM_TYPE_GM == DevType) 
	{
		*pResponseMsg = ETH_DEVICE_TYPE_UGM;
	}
	pResponseMsg++;

	
	return (pResponseMsg - pResponseMsgStart);	
}

static int
DNSBuildHo900(char* pResponseMsg, AMT_Type900 * at2_Content, UCHAR DevType)
{
	char* 	pResponseMsgStart = pResponseMsg;

	// name
	*pResponseMsg = strlen((char*)at2_Content->Name);
	pResponseMsg++;
	strcpy(pResponseMsg, (char*)at2_Content->Name);
	pResponseMsg += strlen((char*)at2_Content->Name);
#ifdef DNS_INFO_DEBUG
	printf("at2_Content->Name = %s\n",at2_Content->Name);
#endif	
	// addreee, no address, use the name as address so that EHV can display it
	*pResponseMsg = strlen((char*)at2_Content->Name);
	pResponseMsg++;
	strcpy(pResponseMsg, (char*)at2_Content->Name);
	pResponseMsg += strlen((char*)at2_Content->Name); 	
	// call code
	*pResponseMsg = strlen((char*)at2_Content->DevCode);
	pResponseMsg++;
	strcpy(pResponseMsg, (char*)at2_Content->DevCode);
	pResponseMsg += strlen((char*)at2_Content->DevCode); 

#ifdef DNS_INFO_DEBUG
	printf("at2_Content->DevCode = %s\n",at2_Content->DevCode);
#endif	
	
	if (ATM_TYPE_EHV == DevType) 
	{
		// IP	
		*pResponseMsg = at2_Content->IPCOUNT;
		pResponseMsg++;
		memcpy(pResponseMsg, (char *)(at2_Content->pDevIP), 4 * at2_Content->IPCOUNT);		
		
		pResponseMsg += 4 * at2_Content->IPCOUNT;

		// dev type
		*pResponseMsg = ETH_DEVICE_TYPE_EHV;
	}
	else if (ATM_TYPE_EGM == DevType) 
	{
		// IP	
		*pResponseMsg = at2_Content->IPCOUNT;
		pResponseMsg++;
		memcpy(pResponseMsg, (char *)(at2_Content->pDevIP), 4 * at2_Content->IPCOUNT);		
		
		pResponseMsg += 4 * at2_Content->IPCOUNT;
		
		// dev type
		*pResponseMsg = ETH_DEVICE_TYPE_ETH_UGM;
	}
	
	pResponseMsg++;
	
	return (pResponseMsg - pResponseMsgStart);	
}

static int	
DNSHoQuery(char* pCommandMsg, char* pResponseMsg, int* pnResponseLen)
{
	UCHAR	nMode = 0;
	DWORD	dwDevIP = 0;
	DWORD	dwTempIP = 0;
	UCHAR	chCallCode[19] = {0};
	AMT_Type800 *at1_Content = NULL;
	AMT_Type900 at2_Content = {0};
	UCHAR  	AMTHeadItem   = 0;
	int		i = 0;
	int		j = 0;
	int		ipCount = 0;
	UCHAR   nType = 0;
	WORD  SetTypeCount = 0;
	DWORD	dwNexAddr    = 0;
	DWORD	dwItemOffset   = 0;
	
	int		bFound = 0;
	int		nRet = 0;
	UCHAR 	nResult = 1;
	DWORD	dwPointer = 0;
	UCHAR 	nCountIP;
	
	nMode = pCommandMsg[0];
	
	if (1 == nMode)
	{//Code Mode
		memcpy(chCallCode, &pCommandMsg[1], 19);		
	}
	if (2 == nMode)
	{//IP Mode
		memcpy((char*)&dwDevIP, &pCommandMsg[1], 4);
	}

#ifdef DNS_INFO_DEBUG
	printf("nMode  = %d\n", nMode);
#endif
	
	// find AMT content, caller insure the table is valid and not empty
	memcpy(&AMTHeadItem,(unsigned char *)(g_pAMTBuf+8),1);
	for (i = 0; i < AMTHeadItem; i++)
	{
		nType = *((unsigned char *)(g_pAMTBuf+9+i*ATM_HEAD_LEN_V4));
		dwPointer = g_pAMTBuf+9+i*ATM_HEAD_LEN_V4 + 1;		
		memcpy(&SetTypeCount,(unsigned char *)dwPointer, 2);
		memcpy(&dwItemOffset,(unsigned char *)(g_pAMTBuf+9+i*ATM_HEAD_LEN_V4 + 3),4);
		if (!(ATM_TYPE_EHV == nType
			||ATM_TYPE_EGM == nType))
		{
			for (j = 0; j < SetTypeCount; j++)
			{
				at1_Content = (AMT_Type800 *)(g_pAMTBuf+dwItemOffset+8+j*ATM_TYPE1_V4_LEN);
				if (1 == nMode)
				{
					// call code.
					if (0 == strcmp((char*)chCallCode, (char*)at1_Content->DevCode))
					{
						bFound = 1;
						break;
					}
				}
				if (2 == nMode)
				{
				}
				if (bFound == 1)
				{
					break;
				}
				
			}			
			if (1 == bFound)
			{
				nRet = DNSBuildHo800(&pResponseMsg[1], at1_Content, nType);
				nResult = 0;
				break;
			}
									
		}
		else
		{
			memcpy(&at2_Content, (UCHAR*)(g_pAMTBuf+dwItemOffset +8), 28);
			nCountIP = at2_Content.IPCOUNT;
			memcpy((UCHAR*)(&at2_Content) + 28, (UCHAR*)(g_pAMTBuf+dwItemOffset +8 + 28), nCountIP * 4);
			
			strcpy((char*)at2_Content.Name, 
					(char*)(g_pAMTBuf+dwItemOffset +8 + 28 + 4 * at2_Content.IPCOUNT));
			for (j = 0; j < SetTypeCount; j++)
			{
				if (1 == nMode)
				{
					// call code.
					if (0 == strcmp((char*)chCallCode, (char*)at2_Content.DevCode))
					{
						bFound = 1;
						break;
					}
				}
				if (2 == nMode)
				{
					for (ipCount = 0; ipCount < at2_Content.IPCOUNT; ipCount++)
					{
						memcpy(&dwTempIP, (char*)(at2_Content.pDevIP) + 4 * ipCount, 4);
				//			printf("find by IP %x %x, %x\n", dwDevIP, dwTempIP, *(DWORD*)(g_pAMTBuf+dwItemOffset +8 + 28 ));
						if (dwDevIP == dwTempIP)
						{
				//			printf("found by IP %x %x\n", dwDevIP, at2_Content.pDevIP[ipCount]);
							bFound = 1;
							break;
						}
					}
				}
				if (bFound == 1)
				{
					break;
				}
				memcpy(&dwNexAddr, at2_Content.NexAddress, 4);
				if (dwNexAddr != 0)
				{
					memcpy(&at2_Content, (UCHAR*)(g_pAMTBuf+dwNexAddr +8), 28);
					nCountIP = at2_Content.IPCOUNT;
					memcpy((UCHAR*)(&at2_Content) + 28, (UCHAR*)(g_pAMTBuf+dwNexAddr +8 + 28), nCountIP * 4);
					strcpy((char*)at2_Content.Name, 
						(char*)(dwNexAddr + g_pAMTBuf + 8 + 28 + 4 * at2_Content.IPCOUNT));
				}
				else
				{
					break;
				}
				
			}			
			if (1 == bFound)
			{
				nRet = DNSBuildHo900(&pResponseMsg[1], &at2_Content, nType);
				nResult = 0;
				break;
			}
		}
		
	}
	
	*pnResponseLen = 0;				
	pResponseMsg[*pnResponseLen] = nResult;
	(*pnResponseLen )++;
	*pnResponseLen += nRet;	
	return 0;	
}


