/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	BacpNetComm.c
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		16 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				BacpNetInit
**				BacpNetProcess
**				BacpNetExit
**				SendAppFrm2Net		
**		
**				BacpNetFreeMXList
**						
**				BacpNetPSProcess
**				SendAppFrm2PSNet
**				SendPSNetFrm2Port
**				RecvPSNetFrmRspFromPort
**				GetBacpNePSRun
**				NewBacpAppBuf
**				
**				BacpNetSPProcess
**				DoSPNetFrm
**				RecvSPNetFrmFromPort
**				SendSPNetFrm2Port
**				
**				BufDebug
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

#include "MXMdId.h"
#include "MXMsg.h"
#include "MXTypes.h"
#include "MXList.h"
#include "BacpNet.h"
#include "BacpApp.h"
#include "BacpNetComm.h"
#include "Eth.h"

/************** DEFINES **************************************************************/

#define	BACP_NET_COMM_MEM_DEBUG
//#define	BACP_NET_COMM_PS_DEBUG
//#define	BACP_NET_COMM_SP_DEBUG
//#define	BACP_APP_COMM_PS_CMD_DEBUG
//#define	BACP_APP_COMM_PS_VIDEO_DEBUG
//#define	BACP_APP_COMM_PS_AUDIO_DEBUG
//#define	BACP_APP_COMM_SP_CMD_DEBUG
//#define	BACP_APP_COMM_SP_VIDEO_DEBUG
//#define	BACP_APP_COMM_SP_AUDIO_DEBUG

//#define	BACP_NET_CMD_NEED_ACK

#define	BACP_NET_MAX_CNT					3
#define	BACP_NET_MASTER_CNT					3
#define	BACP_NET_SLAVE_CNT					3

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

typedef	struct	_BacpNetRunManStruct  
{
	int				nCnt;
	BacpNetRun*		pRun;	
} BacpNetRunManStruct;

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

// BACP Network and port run management
static BacpNetRunManStruct		NetRunMan;	
// Send network frame use this buffer
static unsigned char*			pPrePSNetFrm		= NULL;

static void					BacpNetFreeMXList(MXListHead* pHead);

static void					BacpNetPSProcess(BacpNetPSRun* pPS);
static void					SendAppFrm2PSNet(BacpNetPSRun* pPS, char* pSrcDev, char* pDestDev, unsigned char* pAppFrm, int nAppFrmLen, int nCbId, unsigned int nDestIPAddr);
static void					SendPSNetFrm2Port(BacpNetPSRun* pPS, unsigned char* pNetFrm, int nNetFrmLen);
static BOOL					RecvPSNetFrmRspFromPort(BacpNetPSRun* pPS);
static BacpNetPSRun*		GetBacpNePSRun(int nFunc);
static BacpAppBuf*			NewBacpAppBuf(char* pSrcDev, char* pDestDev, unsigned char* pAppFrm, int nAppFrmLen, int nCbId, unsigned int nDestIPAddr);

static void					BacpNetSPProcess(BacpNetSPRun* pSP);
static void					DoSPNetFrm(BacpNetSPRun* pSP);
static int					RecvSPNetFrmFromPort(BacpNetSPRun* pSP);
static void					SendSPNetFrm2Port(BacpNetSPRun* pSP, unsigned char* pNetFrm, int nNetFrmLen);

static void					RecvRequestIVOP();
static void					BufDebug(unsigned char* pBuf, int nBufLen);

extern int SndPacket(UINT  * pSkt, PBYTE pSd, int nLen, struct sockaddr_in * pAddrTo); /*0 - false; 1 - OK*/

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BacpNetFreeMXList
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
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
BacpNetFreeMXList(MXListHead* pHead)
{
	MXList*					pNext;
	MXList*					pCur;

	pNext = pHead->pHead;

	while (pNext != NULL)
	{
		pCur = pNext;
		pNext = pNext->pNext;
		free(pCur);
#ifdef BACP_NET_COMM_MEM_DEBUG
		printf("Eth: free memory %08X\n", (unsigned int) pCur);
#endif
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BacpNetInit
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			BACP network initialize
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				bMaster		[IN]		BOOL	MasterHV/SlaveHV
**	RETURNED VALUE:	
**				None
**	NOTES:
**		23 - Oct - 2006: Cmd not need ack	
*/
void
BacpNetInit(BOOL bMaster)
{
	int						i;
	BacpNetRun*				pRun;
	int						nBlockFlag	= 1;
	struct sockaddr_in		LocalAddr;
	int						nLocalAddrLen	= sizeof (struct sockaddr_in);
	int nSendBuf=32*1024;//…Ë÷√Œ™32K
	
	//int nSendBuf=0;
	
	pPrePSNetFrm = malloc(MAX_PS_NET_FRM_LEN);
	if (pPrePSNetFrm != NULL)
	{
#ifdef BACP_NET_COMM_MEM_DEBUG
		printf("Eth: malloc memory %08X\n", (unsigned int) pPrePSNetFrm);
#endif
	}
	else
	{
		printf("Eth: malloc pPrePSNetFrm fail ...\n");
	}

	if (bMaster)
	{
		NetRunMan.nCnt	= BACP_NET_MASTER_CNT;
	}
	else
	{
		NetRunMan.nCnt	= BACP_NET_SLAVE_CNT;
	}

	NetRunMan.pRun = (BacpNetRun*) malloc (sizeof (BacpNetRun) * NetRunMan.nCnt);
	if (NetRunMan.pRun != NULL)
	{
#ifdef BACP_NET_COMM_MEM_DEBUG
		printf("Eth: malloc memory %08X\n", (unsigned int) NetRunMan.pRun);
#endif
	}
	else 
	{
		printf("Eth: BacpNetRunManInit malloc fail\n");
		NetRunMan.nCnt = 0;
		return;
	}

	memset(NetRunMan.pRun, 0, sizeof (BacpNetRun) * NetRunMan.nCnt);

	LocalAddr.sin_family		= AF_INET;
	LocalAddr.sin_addr.s_addr	= htonl(INADDR_ANY);

	if (bMaster)
	{
		for (i = 0; i < NetRunMan.nCnt; i++)
		{
			pRun = NetRunMan.pRun + i;
			
			pRun->PS.AppBufHead.pHead	= NULL;
			pRun->PS.AppBufHead.pTail	= NULL;
			pRun->PS.dwTimeout			= NET_ACK_TIMEOUT;
			pRun->PS.bNeedAck			= FALSE;
			pRun->PS.nStatus			= NET_PS_STATUS_ORIGINAL;
			pRun->PS.SockFd.bUseSock	= FALSE;
			pRun->PS.nCurSeq			= 0;
			pRun->PS.nLastSeq			= 0;
			
			pRun->SP.SockFd.bUseSock	= FALSE;
			pRun->SP.NetBufHead.pHead	= NULL;
			pRun->SP.NetBufHead.pTail	= NULL;
			pRun->SP.nRecvBufLen		= 0;
			
			switch (i)
			{
			case 0:
				// P->S
				pRun->PS.nFunc				= NET_DATA_TYPE_CMD;
				pRun->PS.nDestUDPPort		= SVR_PORT_CMD;
#ifdef	BACP_NET_CMD_NEED_ACK
				pRun->PS.bNeedAck			= TRUE;
#endif	
				pRun->PS.SockFd.bUseSock	= TRUE;
				pRun->PS.SockFd.Fd.Sock		= socket(AF_INET, SOCK_DGRAM, 0);
				LocalAddr.sin_port			= htons(CLT_PORT_CMD);
				bind(pRun->PS.SockFd.Fd.Sock, (struct sockaddr*) &LocalAddr, nLocalAddrLen);
				ioctl(pRun->PS.SockFd.Fd.Sock, FIONBIO, &nBlockFlag);
				// S->P
				pRun->SP.nFunc				= NET_DATA_TYPE_CMD;
				pRun->SP.SockFd.bUseSock	= TRUE;
				pRun->SP.SockFd.Fd.Sock		= socket(AF_INET, SOCK_DGRAM, 0);
				LocalAddr.sin_port			= htons(SVR_PORT_CMD);
				bind(pRun->SP.SockFd.Fd.Sock, (struct sockaddr*) &LocalAddr, nLocalAddrLen);
				ioctl(pRun->SP.SockFd.Fd.Sock, FIONBIO, &nBlockFlag);
				break;
				
			case 1:
				// P->S
				pRun->PS.nFunc				= NET_DATA_TYPE_VIDEO;
				pRun->PS.nDestUDPPort		= SVR_PORT_VIDEO;
				pRun->PS.SockFd.bUseSock	= TRUE;
				pRun->PS.SockFd.Fd.Sock		= socket(AF_INET, SOCK_DGRAM, 0);
				setsockopt(pRun->PS.SockFd.Fd.Sock,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
				//fcntl(pRun->PS.SockFd.Fd.Sock, F_SETFL, O_NDELAY);
				
				
				LocalAddr.sin_port			= htons(CLT_PORT_VIDEO);
				bind(pRun->PS.SockFd.Fd.Sock, (struct sockaddr*) &LocalAddr, nLocalAddrLen);
				ioctl(pRun->PS.SockFd.Fd.Sock, FIONBIO, &nBlockFlag);
				// S->P
				pRun->SP.nFunc				= NET_DATA_TYPE_VIDEO;
				pRun->SP.SockFd.bUseSock	= TRUE;
				pRun->SP.SockFd.Fd.Sock		= socket(AF_INET, SOCK_DGRAM, 0);
				setsockopt(pRun->SP.SockFd.Fd.Sock,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
				//fcntl(pRun->SP.SockFd.Fd.Sock, F_SETFL, O_NDELAY);
				
				LocalAddr.sin_port			= htons(SVR_PORT_VIDEO);
				bind(pRun->SP.SockFd.Fd.Sock, (struct sockaddr*) &LocalAddr, nLocalAddrLen);
				ioctl(pRun->SP.SockFd.Fd.Sock, FIONBIO, &nBlockFlag);
				break;
				
			case 2:
				// P->S
				pRun->PS.nFunc				= NET_DATA_TYPE_AUDIO;
				pRun->PS.nDestUDPPort		= SVR_PORT_AUDIO;
				pRun->PS.SockFd.bUseSock	= TRUE;
				pRun->PS.SockFd.Fd.Sock		= socket(AF_INET, SOCK_DGRAM, 0);
				LocalAddr.sin_port			= htons(CLT_PORT_AUDIO);
				bind(pRun->PS.SockFd.Fd.Sock, (struct sockaddr*) &LocalAddr, nLocalAddrLen);
				ioctl(pRun->PS.SockFd.Fd.Sock, FIONBIO, &nBlockFlag);
				// S->P
				pRun->SP.nFunc				= NET_DATA_TYPE_AUDIO;
				pRun->SP.SockFd.bUseSock	= TRUE;
				pRun->SP.SockFd.Fd.Sock		= socket(AF_INET, SOCK_DGRAM, 0);
				LocalAddr.sin_port			= htons(SVR_PORT_AUDIO);
				bind(pRun->SP.SockFd.Fd.Sock, (struct sockaddr*) &LocalAddr, nLocalAddrLen);
				ioctl(pRun->SP.SockFd.Fd.Sock, FIONBIO, &nBlockFlag);
				break;
				
			default:
				break;
			}
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BacpNetProcess
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**				BACP network process
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
BacpNetProcess()
{
	int						i;
	BacpNetPSRun*			pPS				= NULL;
	BacpNetSPRun*			pSP				= NULL;
	
	for (i = 0; i < NetRunMan.nCnt; i++)
	{
		pPS = &NetRunMan.pRun[i].PS;
		BacpNetPSProcess(pPS);

		pSP = &NetRunMan.pRun[i].SP;
		BacpNetSPProcess(pSP);
	}
	RecvRequestIVOP();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BacpNetExit
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			BACP network exit
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
BacpNetExit()
{
	int						i;
	BacpNetRun*				pRun;

	if (NetRunMan.pRun != NULL)
	{
		for (i = 0; i < NetRunMan.nCnt; i++)
		{
			pRun = NetRunMan.pRun + i;
			if (pRun->PS.SockFd.bUseSock)
			{
				close(pRun->PS.SockFd.Fd.Sock);
			}

			BacpNetFreeMXList(&pRun->PS.AppBufHead);

			if (pRun->SP.SockFd.bUseSock)
			{
				close(pRun->SP.SockFd.Fd.Sock);
			}

			BacpNetFreeMXList(&pRun->SP.NetBufHead);
		}

		free(NetRunMan.pRun);
#ifdef BACP_NET_COMM_MEM_DEBUG
		printf("Eth: free memory %08X\n", (unsigned int) NetRunMan.pRun);
#endif
		NetRunMan.pRun = NULL;
		NetRunMan.nCnt = 0;
	}

	if (pPrePSNetFrm != NULL)
	{
		free(pPrePSNetFrm);
#ifdef BACP_NET_COMM_MEM_DEBUG
		printf("Eth: free %08X\n", (unsigned int) pPrePSNetFrm);
#endif
		pPrePSNetFrm = NULL;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BacpNetPSProcess
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			BACP network primary -> standby process
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pPS			[IN/OUT]	BacpNetPSRun*	
**	RETURNED VALUE:	
**				None
**	NOTES:
**		21 - Oct - 2006: Add callback when send ok or fail		
**
*/
static void
BacpNetPSProcess(BacpNetPSRun* pPS)
{
	DWORD					dwCur;
	BacpAppBuf*				pBacpAppBuf;

	if (NET_PS_STATUS_ORIGINAL == pPS->nStatus)
	{
		if (pPS->AppBufHead.pHead != NULL)
		{
			pBacpAppBuf = (BacpAppBuf*) pPS->AppBufHead.pHead;

			SendAppFrm2PSNet(	pPS,
				pBacpAppBuf->Src,
				pBacpAppBuf->Dest, 
				pBacpAppBuf->pAppFrm, 
				pBacpAppBuf->nAppFrmLen, 
				pBacpAppBuf->nCbId,
				pBacpAppBuf->nDestIPAddr);

			MXListRm(&pPS->AppBufHead, &pBacpAppBuf->List);
			free(pBacpAppBuf);

#ifdef BACP_NET_COMM_MEM_DEBUG
			printf("Eth: free memory %08X\n", (unsigned int) pBacpAppBuf);
#endif
		}
	}
	else if (NET_PS_STATUS_WAIT_FOR_ACK == pPS->nStatus)
	{
		if (RecvPSNetFrmRspFromPort(pPS))
		{
			pPS->nStatus = NET_PS_STATUS_ORIGINAL;
			DoNetSendCb(pPS->nCbId, TRUE);
		}
		else
		{
			dwCur = GetTickCount();
			if (dwCur - pPS->dwSendTick >= pPS->dwTimeout)
			{
				// not get ack, notify the module id
				DoNetSendCb(pPS->nCbId, FALSE);
				pPS->nStatus = NET_PS_STATUS_ORIGINAL;
				printf("Not get ack...\n");
			}
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendAppFrm2Net
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Send app frame to network
**
**	ARGUMENTS:	ARGNAME			DRIECTION	TYPE	DESCRIPTION
**				nInnerFlag		[IN]		int		// INNER, OUTER
**				nFunc			[IN]		int		// CMD, VIDEO, AUDIO
**				pSrcDev			[IN]		char*
**				pDestDev		[IN]		char*
**				pAppFrm			[IN]		unsigned char*
**				nAppFrmLen		[IN]		int
**				nCbId			[IN]		int
**				nDestIPAddr		[IN]		unsigned int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
SendAppFrm2Net(int nFunc, char* pSrcDev, char* pDestDev, unsigned char* pAppFrm, int nAppFrmLen, int nCbId, unsigned int nDestIPAddr)
{
	BacpNetPSRun*			pPS				= NULL;

	pPS = GetBacpNePSRun(nFunc);
	if (NULL == pPS)
	{
		return;
	}

	SendAppFrm2PSNet(pPS, pSrcDev, pDestDev, pAppFrm, nAppFrmLen, nCbId, nDestIPAddr);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendAppFrm2Net
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Send app frame to network
**
**	ARGUMENTS:	ARGNAME			DRIECTION	TYPE	DESCRIPTION
**				pPS				[IN/OUT]	BacpNetPSRun*
**				pSrcDev			[IN]		char*	
**				pDestDev		[IN]		char*
**				pAppFrm			[IN]		unsigned char*
**				nAppFrmLen		[IN]		int
**				nCbId			[IN]		int
**				nDestIPAddr		[IN]		unsigned int
**	RETURNED VALUE:	
**				None
**	NOTES:
**		If there is a network frame waiting for ack, 
**		put the application frame to list. 
**		Then the application frame will be sent in 
**		BacpNetPSProcess function
**
**		If not need ack, not to change NET_PS_STATUS_WAIT_FOR_ACK
**
*/
static void					
SendAppFrm2PSNet(BacpNetPSRun* pPS, char* pSrcDev, char* pDestDev, unsigned char* pAppFrm, int nAppFrmLen, int nCbId, unsigned int nDestIPAddr)
{
	int			nNetFrmLen		= 0;
	BacpAppBuf* pBacpAppBuf		= NULL;
	unsigned char * pSendAppBuf=NULL;
	
	
	pSendAppBuf=malloc(MAX_PS_NET_FRM_LEN);
	if (NET_PS_STATUS_ORIGINAL == pPS->nStatus)
	{
		if (nAppFrmLen + MIN_LEN_BACP_NET_LY <= MAX_PS_NET_FRM_LEN)
		{
			nNetFrmLen = PckBacpNetRqtEx(pAppFrm, nAppFrmLen, pPS->bNeedAck, pPS->nCurSeq, pSendAppBuf);
			pPS->nLastSeq = pPS->nCurSeq;
			pPS->nCurSeq++;
			pPS->nDestIPAddr = nDestIPAddr;
			
#ifdef BACP_APP_COMM_PS_CMD_DEBUG
			if (NET_DATA_TYPE_CMD == pPS->nFunc)
			{
				printf("Eth: APP Send, P -> S (CMD) length %d\n", nAppFrmLen); 
				BufDebug(pAppFrm, nAppFrmLen);
			}
#endif
#ifdef BACP_APP_COMM_PS_VIDEO_DEBUG
			if (NET_DATA_TYPE_VIDEO == pPS->nFunc)
			{
				printf("Eth: APP Send, P -> S (VIDEO) length %d\n", nAppFrmLen); 
				BufDebug(pAppFrm, nAppFrmLen);
			}
#endif
#ifdef BACP_APP_COMM_PS_AUDIO_DEBUG
			if (NET_DATA_TYPE_AUDIO == pPS->nFunc)
			{
				printf("Eth: APP Send, P -> S (AUDIO) length %d\n", nAppFrmLen); 
				BufDebug(pAppFrm, nAppFrmLen);
			}
#endif
			SendPSNetFrm2Port(pPS, pSendAppBuf, nNetFrmLen);
			
			if (pPS->bNeedAck)
			{
				pPS->dwSendTick = GetTickCount();
				pPS->nCbId = nCbId;
				pPS->nStatus = NET_PS_STATUS_WAIT_FOR_ACK;
			}
		}
		else
		{
			printf("Eth: pSendAppBuf %08X buffer size is less\n", (unsigned int) pSendAppBuf);
		}
	}
	else if (NET_PS_STATUS_WAIT_FOR_ACK == pPS->nStatus)
	{
		pBacpAppBuf = NewBacpAppBuf(pSrcDev, pDestDev, pAppFrm, nAppFrmLen, nCbId, nDestIPAddr);
		if (pBacpAppBuf != NULL)
		{
			MXListAdd(&pPS->AppBufHead, &pBacpAppBuf->List);
		}
	}
	free(pSendAppBuf);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendPSNetFrm2Port
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Send network frame to port (P->S)
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pPS			[IN]		BacpNetPSRun*
**				pNetFrm		[IN]		unsigned char*
**				nNetFrmLen	[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**		HPI not finished	
*/
static void
SendPSNetFrm2Port(BacpNetPSRun* pPS, unsigned char* pNetFrm, int nNetFrmLen)
{
	struct sockaddr_in		RemoteAddr;
	int						nRemoteAddrLen;
	int nSendRslt= 0;
	
	
	if (pPS->SockFd.bUseSock)
	{
		RemoteAddr.sin_family		= AF_INET;
		RemoteAddr.sin_addr.s_addr	= htonl(pPS->nDestIPAddr);
		RemoteAddr.sin_port			= htons(pPS->nDestUDPPort);
		nRemoteAddrLen = sizeof (struct sockaddr_in);	
		nSendRslt=sendto(	pPS->SockFd.Fd.Sock, 
			pNetFrm, nNetFrmLen, 0, 
			(struct sockaddr*) &RemoteAddr, nRemoteAddrLen);
#ifdef BACP_NET_COMM_PS_DEBUG
		printf("Eth: Send, P -> S, ip %d.%d.%d.%d, port %d\n", 
			(int) ((pPS->nDestIPAddr >> 24) & 0xFF), 
			(int) ((pPS->nDestIPAddr >> 16) & 0xFF), 
			(int) ((pPS->nDestIPAddr >> 8) & 0xFF), 
			(int) (pPS->nDestIPAddr & 0xFF), 
			pPS->nDestUDPPort);
		BufDebug(pNetFrm, nNetFrmLen);
#endif
	}
	else
	{
		// use HPI
	}
	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RecvPSNetFrmRspFromPort
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Receive network reponse frame from port (P->S)
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pPS			[IN]		BacpNetPSRun*
**	RETURNED VALUE:	
**				TRUE if get last send frame's response
**	NOTES:
**		HPI not finished	
*/
static BOOL
RecvPSNetFrmRspFromPort(BacpNetPSRun* pPS)
{
	unsigned char			RecvBuf[RSP_LEN_BACP_NET_LY * 2];
	int						nRecvBufLen	= 0;
	struct sockaddr_in		RemoteAddr;
	int						nRemoteAddrLen;
	BacpNetBscFld			BscFld;
	BOOL					bRetCode	= FALSE;
	unsigned char*			pApp;

	if (pPS->SockFd.bUseSock)
	{
		nRemoteAddrLen = sizeof (struct sockaddr_in);
		nRecvBufLen = recvfrom(	pPS->SockFd.Fd.Sock, 
			RecvBuf, 
			(RSP_LEN_BACP_NET_LY * 2),
			0, 
			(struct sockaddr*) &RemoteAddr, 
			&nRemoteAddrLen);

		while (nRecvBufLen > 0) 
		{
#ifdef BACP_NET_COMM_PS_DEBUG
			printf("Eth: Recv, P -> S, port %d\n", 
				ntohs(RemoteAddr.sin_port));
			BufDebug(RecvBuf, nRecvBufLen);
#endif
			if (	(RemoteAddr.sin_addr.s_addr == htonl(pPS->nDestIPAddr))	&& 
					(RemoteAddr.sin_port == htons(pPS->nDestUDPPort))	&&
					(IsCpltBacpNetFrm(RecvBuf, &nRecvBufLen))			&&
					(IsBacpNetRspFrm(RecvBuf, nRecvBufLen)))
			{
				UnpckBacpNetEx(RecvBuf, nRecvBufLen, &BscFld, &pApp);
				if (BscFld.Seq == pPS->nLastSeq)
				{
					bRetCode = TRUE;
					break;
				}
				else
				{
					nRecvBufLen -= RSP_LEN_BACP_NET_LY;
				}
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		// hpi
	}

	return bRetCode;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetBacpNePSRun
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			According CMD/VIDEO/AUDIO flag to find BacpNetPSRun struct
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nFunc		[IN]		int
**	RETURNED VALUE:	
**				BacpNetPSRun struct pointer
**	NOTES:
**			
*/
static BacpNetPSRun*
GetBacpNePSRun(int nFunc)
{
	BacpNetPSRun* pRun	= NULL;
	int			i;

	for (i = 0; i < NetRunMan.nCnt; i++)
	{
		if (NetRunMan.pRun[i].PS.nFunc == nFunc)
		{
			pRun = &NetRunMan.pRun[i].PS;
			break;
		}
	}

	return pRun;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NewBacpAppBuf
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Store app info to BacpAppBuf
**
**	ARGUMENTS:	ARGNAME			DRIECTION	TYPE	DESCRIPTION
**				pSrcDev			[IN]		char*
**				pDestDev		[IN]		char* 
**				pAppFrm			[IN]		unsigned char* 
**				nAppFrmLen		[IN]		int
**				nCbId			[IN]		int
**				nDestIPAddr		[IN]		unsigned int
**	RETURNED VALUE:	
**				BacpAppBuf* pointer
**	NOTES:
**			
*/
static BacpAppBuf*			
NewBacpAppBuf(char* pSrcDev, char* pDestDev, unsigned char* pAppFrm, int nAppFrmLen, int nCbId, unsigned int nDestIPAddr)
{
	BacpAppBuf*		pBacpAppBuf	= NULL;

	pBacpAppBuf = (BacpAppBuf*) malloc(sizeof (BacpAppBuf) + nAppFrmLen);
	if (pBacpAppBuf != NULL)
	{
		pBacpAppBuf->pAppFrm = (unsigned char*) pBacpAppBuf + sizeof (BacpAppBuf);
		strcpy(pBacpAppBuf->Src, pSrcDev);
		strcpy(pBacpAppBuf->Dest, pDestDev);
		pBacpAppBuf->nCbId = nCbId;
		pBacpAppBuf->nAppFrmLen = nAppFrmLen;
		memcpy(pBacpAppBuf->pAppFrm, pAppFrm, nAppFrmLen);
		pBacpAppBuf->nDestIPAddr = nDestIPAddr;
#ifdef BACP_NET_COMM_MEM_DEBUG
		printf("Eth: malloc memory %08X\n", (unsigned int) pBacpAppBuf);
#endif
	}
	else
	{
		printf("Eth: NewBacpAppBuf malloc fail\n");
	}

	return pBacpAppBuf;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BacpNetSPProcess
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			BACP network standby -> primary process
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pSP			[IN/OUT]	BacpNetSPRun*	
**	RETURNED VALUE:	
**				None
**	NOTES:
**
*/
static void
BacpNetSPProcess(BacpNetSPRun* pSP)
{
	RecvSPNetFrmFromPort(pSP);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DoSPNetFrm
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			analyse network frame, call application layer function (S->P)
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pSP			[IN/OUT]	BacpNetSPRun*		
**	RETURNED VALUE:	
**				None
**	NOTES:
**		30 - Nov - 2006: fix bug, clear the frame that it is not request			
*/
static void
DoSPNetFrm(BacpNetSPRun* pSP)
{
	BacpNetBscFld			BscFld;
	unsigned char*			pApp;
	int						nAppLen;
	unsigned char			RspFrm[RSP_LEN_BACP_NET_LY];
	int						nRspFrmLen;
	
	if (IsCpltBacpNetFrm(pSP->RecvBuf, &pSP->nRecvBufLen))
	{	
		if (IsBacpNetRqtFrm(pSP->RecvBuf, pSP->nRecvBufLen))
		{
			nAppLen = UnpckBacpNetEx(pSP->RecvBuf, pSP->nRecvBufLen, &BscFld, &pApp);			

			if(nAppLen >= pSP->nRecvBufLen)
			{
				printf("%s,%d,nAppLen:%d,nRecvBufLen:%d\n",__func__,__LINE__,nAppLen,pSP->nRecvBufLen);
				pSP->nRecvBufLen = 0;
				return;
			}

	#ifdef BACP_APP_COMM_SP_CMD_DEBUG
			if (NET_DATA_TYPE_CMD == pSP->nFunc)
			{
				printf("Eth: APP Recv, S -> P (CMD) length %d\n", nAppLen); 
				BufDebug(pApp, nAppLen);
			}
	#endif
	#ifdef BACP_APP_COMM_SP_VIDEO_DEBUG
			if (NET_DATA_TYPE_VIDEO == pSP->nFunc)
			{
				printf("Eth: APP Recv, S -> P (VIDEO) length %d\n", nAppLen); 
				BufDebug(pApp, nAppLen);
			}
	#endif
	#ifdef BACP_APP_COMM_SP_AUDIO_DEBUG
			if (NET_DATA_TYPE_AUDIO == pSP->nFunc)
			{
				printf("Eth: APP Recv, S -> P (AUDIO) length %d\n", nAppLen); 			
				BufDebug(pApp, nAppLen);
			}
	#endif
			// Send network response
			if (IsBacpNetOptConOn(BscFld.Opt))
			{
				nRspFrmLen = PckBacpNetRspEx(BscFld.Seq, RspFrm);
				SendSPNetFrm2Port(pSP, RspFrm, nRspFrmLen);
			}
			// Call App layer function
			DoAppFrm(pSP->nFunc, pApp, nAppLen, pSP->nSrcIPAddr);
		}
		// Clear received buffer
		pSP->nRecvBufLen = 0;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RecvSPNetFrmFromPort
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Receive network frame, call network layer function (S->P)	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pSP			[IN/OUT]	BacpNetSPRun*		
**	RETURNED VALUE:	
**				None
**	NOTES:
**		HPI not finished	
*/
static int
RecvSPNetFrmFromPort(BacpNetSPRun* pSP)
{
	unsigned char			RecvBuf[MAX_SP_NET_FRM_LEN];
	int						nRecvBufLen	= 0;
	struct sockaddr_in		RemoteAddr;
	int						nRemoteAddrLen;
	unsigned int			nSrcIPAddr	= 0L;
	unsigned short			nSrcUDPPort	= 0;

	if (pSP->nRecvBufLen >= MAX_SP_NET_FRM_LEN)
	{
		pSP->nRecvBufLen = 0;
	}

	if (pSP->SockFd.bUseSock)
	{
		nRemoteAddrLen = sizeof (struct sockaddr_in);
		nRecvBufLen = recvfrom(	pSP->SockFd.Fd.Sock, 
			RecvBuf, 
			MAX_SP_NET_FRM_LEN, 
			0, 
			(struct sockaddr*) &RemoteAddr,
			&nRemoteAddrLen);

		if (nRecvBufLen > 0)
		{
#ifdef BACP_NET_COMM_SP_DEBUG
			printf("Eth: Recv, S -> P, port %d\n", 
				ntohs(RemoteAddr.sin_port));
			BufDebug(RecvBuf, nRecvBufLen);
#endif
			nSrcIPAddr = ntohl(RemoteAddr.sin_addr.s_addr);
			nSrcUDPPort = ntohs(RemoteAddr.sin_port);
		}
	}
	else
	{
		// HPI
	}

	if (nRecvBufLen > 0)
	{
		if ( (pSP->nRecvBufLen > 0)				&& 
			 (nSrcIPAddr == pSP->nSrcIPAddr)	&&
			 (nSrcUDPPort == pSP->nSrcUDPPort))
		{
			if (nRecvBufLen + pSP->nRecvBufLen > MAX_SP_NET_FRM_LEN)
			{
				memcpy(pSP->RecvBuf, RecvBuf, nRecvBufLen);
				pSP->nRecvBufLen = nRecvBufLen;
			}
			else
			{
				memcpy((pSP->RecvBuf + pSP->nRecvBufLen), RecvBuf, nRecvBufLen);
				pSP->nRecvBufLen += nRecvBufLen;
			}
		}
		else
		{
			memcpy(pSP->RecvBuf, RecvBuf, nRecvBufLen);
			pSP->nRecvBufLen = nRecvBufLen;
			pSP->nSrcIPAddr = nSrcIPAddr;
			pSP->nSrcUDPPort = nSrcUDPPort;
		}

		DoSPNetFrm(pSP);
	}
	
	return 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendSPNetFrm2Port
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Send network frame (S->P)	
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pSP			[IN/OUT]	BacpNetSPRun*	
**				pNetFrm		[IN]		unsigned char*
**				nNetFrmLen	[IN]		int	
**	RETURNED VALUE:	
**				None
**	NOTES:
**		HPI not finished	
*/
static void
SendSPNetFrm2Port(BacpNetSPRun* pSP, unsigned char* pNetFrm, int nNetFrmLen)
{
	struct sockaddr_in		RemoteAddr;
	int						nRemoteAddrLen;

	if (pSP->SockFd.bUseSock)
	{
		RemoteAddr.sin_family		= AF_INET;
		RemoteAddr.sin_addr.s_addr	= htonl(pSP->nSrcIPAddr);
		RemoteAddr.sin_port			= htons(pSP->nSrcUDPPort);
		nRemoteAddrLen				= sizeof (struct sockaddr_in);	

		sendto(	pSP->SockFd.Fd.Sock, 
			pNetFrm, nNetFrmLen, 0,
			(struct sockaddr*) &RemoteAddr,
			nRemoteAddrLen);
#ifdef BACP_NET_COMM_SP_DEBUG
		printf("Eth: Send (Confirm), S -> P, port %d\n", 			
			ntohs(RemoteAddr.sin_port));
			BufDebug(pNetFrm, nNetFrmLen);
#endif
	}
	else
	{
		// HPI
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RecvRequestIVOP
**	AUTHOR:			Jeff Wang
**	DATE:			24 - Nov - 2008
**
**	DESCRIPTION:	
**			Receive and process I frame request
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
RecvRequestIVOP()
{
	MXMSG	MsgSend;

	unsigned char			RecvBuf[RSP_LEN_BACP_NET_LY * 2];
	int						nRecvBufLen	= 0;
	struct sockaddr_in		RemoteAddr;
	int						nRemoteAddrLen;	
	
	unsigned char*			pAppFrm		= NULL;
	int						nAppFrmLen	= 0;
	BacpNetBscFld			BscFld;
	
	BacpNetPSRun*			pPS			= NULL;
	
	char*					pSrcDev = NULL;
	char*					pDestDev = NULL;
	unsigned short			nFunCode = 0;
	unsigned char*			pRcvData = NULL;
	int						nRcvDataLen = 0;	

	pPS = &NetRunMan.pRun[1].PS;
	
	if (pPS->SockFd.bUseSock)
	{
		nRemoteAddrLen = sizeof (struct sockaddr_in);
		nRecvBufLen = recvfrom(	pPS->SockFd.Fd.Sock, 
			RecvBuf, 
			(RSP_LEN_BACP_NET_LY * 2),
			0, 
			(struct sockaddr*) &RemoteAddr, 
			&nRemoteAddrLen);
		
		if (nRecvBufLen > 0) 
		{
			nAppFrmLen = UnpckBacpNetEx(RecvBuf, nRecvBufLen, &BscFld, &pAppFrm);

			UnpckBacpAppEx(	&nFunCode, 
				&pSrcDev, &pDestDev, &pRcvData, 
				&nRcvDataLen, pAppFrm, nAppFrmLen);
			if (FC_AV_REQUEST_VIDEO_IVOP == nFunCode)
			{
				strcpy(MsgSend.szSrcDev, pSrcDev);
				strcpy(MsgSend.szDestDev, pDestDev);
					
				MsgSend.dwDestMd	= MXMDID_MM;
				MsgSend.dwSrcMd		= MXMDID_ETH;
				MsgSend.dwMsg		= nFunCode;	
						
				MxPutMsg(&MsgSend);
				
//				printf("Eth:FC_AV_REQUEST_VIDEO_IVOP\n");				
			}
		}
	}	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BufDebug
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Oct - 2006
**
**	DESCRIPTION:	
**			Debug buffer
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pBuf		[IN]		unsigned char*
**				int			[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
BufDebug(unsigned char* pBuf, int nBufLen)
{
	int		i;

	for (i = 0; i < nBufLen; i++)
	{
		printf("%02X ", pBuf[i]);
	}
	printf("\n");
}
