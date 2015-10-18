/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	BacpNetCtrl.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		25 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef BACPNETCTRL_H
#define BACPNETCTRL_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

#include <windows.h>

/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"
#include "MXList.h"
#include "Dispatch.h"
#include "Bacp.h"
#include "BacpNet.h"
#include "BacpApp.h"
#include "BacpNetComm.h"

/************** DEFINES ***************************************************************************************/

#define	SVR_PORT_CTRL				0x7F8
#define	CLT_PORT_CTRL				0x7F9

#define	MAX_NAME_SERVER_NUM			2

#define	DEVICE_NAME_LENGTH			19
#define MAX_ADDR_LEN				200

#define	ETH_DEVICE_TYPE_SHV			1
#define	ETH_DEVICE_TYPE_CGM			2
#define	ETH_DEVICE_TYPE_UGM			3
#define	ETH_DEVICE_TYPE_EHV			4
#define	ETH_DEVICE_TYPE_MC			5
#define	ETH_DEVICE_TYPE_ETH_UGM			128
#define	ETH_DEVICE_TYPE_ETH_CGM			129
					
#define	MAX_IPID_NUM_PER_ETH_DEVICE		32

#define	HO_QUERY_METHOD_CODE				0x01
#define	HO_QUERY_METHOD_IP					0x02
#define	HO_QUERY_METHOD_CSN					0x03
#define	HO_QUERY_METHOD_EHV					0x04
#define	HO_QUERY_METHOD_HOME				0x05
#define	HO_QUERY_METHOD_OUTSIDE				0x06
#define	HO_QUERY_METHOD_CALLCODE			0x07
#define	HO_QUERY_METHOD_RDCODE				0x08
#define	HO_QUERY_METHOD_AC					0x09

#define HO_QUERY_ADDRBOOK						0xFF


#define LC_DO_MODE								0x00
#define LC_COMM_MODE							0X01


#define LC_MOX_MODE_V1						0x00
#define LC_HITACHI_MODE						0x01
#define LC_MITSUBISHI_MODE					0x02
#define LC_MOX_MODE_V2						0x03
#define LC_BY_DO_MODULE						0x04

#define LC_RUN_MODE_CALL					0X00	// Call lift
#define LC_RUN_MODE_CALL_UNLOCK				0X01	// call lift & unlocl
#define LC_RUN_MODE_AUTO					0X02	// auto running
/************** TYPEDEFS **************************************************************************************/

typedef	struct _BacpNetIPID
{
	unsigned int				nIPAddr;//IP address
	DWORD						dwID;	//THE HV id from index 0;
} BacpNetIPID;

typedef	struct _BacpNetNameServer
{
	unsigned int				nIPAddr;
	char						szDevCode[MAX_LEN_DEV_CODE + 1];

} BacpNetNameServer;

typedef	struct _ResolvItem
{
	MXList						List;

	char						szName[DEVICE_NAME_LENGTH + 1];
	char						szAddr[MAX_ADDR_LEN];
	char						szDevCode[MAX_LEN_DEV_CODE + 1];
	int							nType;
	BacpNetIPID					IPIDArr[MAX_IPID_NUM_PER_ETH_DEVICE];
	int							nIPIDCnt;
	
	BOOL						bForever;
	DWORD						nExpires;

} ResolvItem;

typedef	struct _EthResolvInfo
{
	unsigned int				nIP;
	char						szDevCode[MAX_LEN_DEV_CODE + 1];
	char						szAddr[MAX_ADDR_LEN];
	char						szName[DEVICE_NAME_LENGTH + 1];
	int							nType;
	BacpNetIPID					IPIDArr[MAX_IPID_NUM_PER_ETH_DEVICE];
	int							nIPIDCnt;
	SHORT					Level;

	unsigned char				nQueryMethod;	// 1 by code, 2 bye ip
	
} EthResolvInfo;

// Primary -> Secondary
typedef	struct _BacpNetCtrlPSRun
{
	EthSockFd					SockFd;
	BOOL						bNeedAck;
	unsigned int				nDestIPAddr;
	unsigned short				nDestUDPPort;
	unsigned int				nCurSeq;
} BacpNetCtrlPSRun;

// Secondary -> Primary
typedef	struct _BacpNetCtrlSPRun
{
	EthSockFd					SockFd;
} BacpNetCtrlSPRun;

typedef	struct _BacpNetCtrlRun
{
	BacpNetCtrlPSRun			PS;
	BacpNetCtrlSPRun			SP;

	BacpNetNameServer			NameServer[MAX_NAME_SERVER_NUM];
	int							nNameServerCnt;
	
	unsigned int				nGMIPAddr;
	unsigned int				nMCIPAddr;
	unsigned int				nSelfIPAddr;

} BacpNetCtrlRun;

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern	void		BacpNetCtrlInit(BOOL bMaster);
extern	void		BacpNetCtrlExit();
extern	BOOL		BacpNetCtrlResolv(EthResolvInfo* pResolvInfo);
extern	DWORD		GetSelfIP();
extern	BOOL			GetSelfInfo(unsigned int* pIPAddr, char* pszDevCode);
extern	BOOL			GetMCInfo(unsigned int* pIPAddr, char* pszDevCode);
extern	BOOL			GetGMInfo(unsigned int* pIPAddr, char* pszDevCode);
extern	void			NetCtrlUpdateTime();
extern	void			SetTimeFromMC(unsigned char* pTime);

extern BOOL			NetReqModuleCount(BYTE nType);
extern BOOL			NetReqModuleInfo(BYTE nType, unsigned short iIndex, BYTE nCnt, BYTE nOrderBy);

extern BOOL		NetReqAddrBookCount(void * pNode, PBYTE* pCode, BYTE bCodeLen);
extern	BOOL	NetReqAddrInfo(void * pNode, unsigned char * pCode, BYTE bCodeLen, unsigned short iIndex, BYTE nCnt, BYTE nOrderBy);

extern void		CheckMCStatus();

extern	void	ProcessIcmp();

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // BACPNETCTRL_H
