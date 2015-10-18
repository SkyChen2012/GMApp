/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Dispatch.h
**
**	AUTHOR:		Jerry Huang
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

#ifndef DISPATCH_H
#define DISPATCH_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

#include "MXTypes.h"
#include <pthread.h>
#include "MXList.h"

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/

#define	MAX_LEN_DEV_CODE			19

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

typedef	struct _MXMSG
{
	char							szSrcDev[MAX_LEN_DEV_CODE + 1];
	char							szDestDev[MAX_LEN_DEV_CODE + 1];
	DWORD							dwSrcMd;
	DWORD							dwDestMd;
	DWORD							dwMsg;
	DWORD							dwParam;
	WORD							wDataLen;
	unsigned char*					pParam;
	
} MXMSG, *PMXMSG;



typedef struct _HASH_Version_Result
{	
	UCHAR  hash_Version;
	UCHAR  hash_Result
} HASH_VR;



// Internal MSG structuct
typedef	struct _MXMSGInt
{
	MXList							List;
	MXMSG							Msg;
} MXMSGInt;

// Module callback function
typedef	struct _MdCbFun
{
	DWORD (*GetStatus)();	
} MdCbFun;

// Module structure
typedef	struct	_MdInfo
{
	MXList							List;
	DWORD							dwID;
	MdCbFun							CbFun;
	pthread_mutex_t					Mutex;
	MXListHead						MsgHead;
	unsigned short					nMsgCnt;
} MdInfo;

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern void			DpcInit();
extern void			DpcExit();
extern BOOL			DpcAddMd(DWORD dwID, const MdCbFun* pFun);
extern void			DpcRmMd(DWORD dwID);
extern void			DpcPrintAllMd();
extern BOOL			MxPutMsg(const MXMSG* pMsg);
extern BOOL			MxPutMsgEx(const char* szSrcDev, const char* szDestDev, DWORD dwSrcMd, DWORD dwDestMd, DWORD dwMsg, DWORD dwParam, unsigned char* pParam);
extern BOOL			MxGetMsg(MXMSG* pMsg);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // DISPATCH_H
