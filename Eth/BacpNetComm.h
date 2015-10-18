/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	BacpNetComm.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		16 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef BACPNETCOMM_H
#define BACPNETCOMM_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

#include <windows.h>

/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"
#include "MXList.h"
#include "Dispatch.h"
#include "Bacp.h"
#include "BacpNet.h"
#include "BacpApp.h"

/************** DEFINES ***************************************************************************************/

#define	NET_INNER_FLAG				0
#define	NET_OUTER_FLAG				1

#define	NET_DATA_TYPE_CMD			0
#define	NET_DATA_TYPE_VIDEO			1
#define	NET_DATA_TYPE_AUDIO			2
#define NET_DATA_TYPE_DNS			3

typedef	enum
{
	NET_PS_STATUS_ORIGINAL	= 0,
	NET_PS_STATUS_WAIT_FOR_ACK
} NET_PS_STATUS;

#define	NET_ACK_TIMEOUT				200	// ms

#define	SVR_PORT_CMD				0x7F0
#define	CLT_PORT_CMD				0x7F1
#define	SVR_PORT_VIDEO				0x7F2
#define	CLT_PORT_VIDEO				0x7F3
#define	SVR_PORT_AUDIO				0x7F4
#define	CLT_PORT_AUDIO				0x7F5
#define	SVR_PORT_DNS				0x7F8
#define	CLT_PORT_DNS				0x7F9



#define	MAX_PS_NET_FRM_LEN			(1024 * 3)
#define	MAX_SP_NET_FRM_LEN			(1024 * 3)
#define	MAX_APP_FRM_LEN				(1024 * 3)

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

typedef	union
{
	char						szDev[MAX_LEN_DEV_CODE + 1];
	DWORD						dwID;
} BacpAppDevCode;

typedef	struct _BacpAppBuf
{
	MXList						List;
	char*						Src;
	char*						Dest;
	int							nCbId;
	int							nAppFrmLen;
	unsigned char*				pAppFrm;
	unsigned int				nDestIPAddr;
} BacpAppBuf;

typedef	struct _EthSockFd
{
	BOOL						bUseSock;		// TRUE use socket, FALSE use HPI
	union
	{
		int						Sock;			// Socket fd
	} Fd;
} EthSockFd;

// Primary -> Secondary
typedef	struct _BacpNetPSRun
{
//	int							nInnerFlag;
	int							nFunc;
	EthSockFd					SockFd;
	BOOL						bNeedAck;
	NET_PS_STATUS				nStatus;
	DWORD						dwSendTick;
	DWORD						dwTimeout;
	unsigned int				nDestIPAddr;
	unsigned short				nDestUDPPort;
	MXListHead					AppBufHead;	
	int							nCbId;	// The id that to notify, after communication ok or fail
	unsigned int				nCurSeq;
	unsigned int				nLastSeq;
} BacpNetPSRun;

typedef	struct _BacpNetSPBuf
{
	MXList						List;
	unsigned char				RecvBuf[MAX_SP_NET_FRM_LEN];
	int							nRecvBufLen;
	unsigned int				nSrcIPAddr;
	unsigned short				nSrcUDPPort;
	DWORD						dwRecvTick;
} BacpNetSPBuf;
// Secondary -> Primary
typedef	struct _BacpNetSPRun
{
//	int							nInnerFlag;
	int							nFunc;
	EthSockFd					SockFd;
	MXListHead					NetBufHead;

	unsigned char				RecvBuf[MAX_SP_NET_FRM_LEN];
	int							nRecvBufLen;
	unsigned int				nSrcIPAddr;
	unsigned short				nSrcUDPPort;

} BacpNetSPRun;

typedef	struct _BacpNetRun
{
	BacpNetPSRun		PS;
	BacpNetSPRun		SP;
} BacpNetRun;

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern void			BacpNetInit(BOOL bMaster);
extern void			BacpNetProcess();
extern void			BacpNetExit();
extern void			SendAppFrm2Net(int nFunc, char* pSrcDev, char* pDestDev, unsigned char* pAppFrm, int nAppFrmLen, int nCbId, unsigned int nDestIPAddr);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // BACPNETCOMM_H

