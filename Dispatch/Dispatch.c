/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Dispatch.c
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		25 - Sep - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				DpcInit	
**				DpcExit
**				DpcAddMd
**				DpcRmMd
**				DpcPrintAllMd	
**				MxPutMsg
**				MxPutMsgEx
**				MxGetMsg
**				
**				DpcFindMd
**				DpcRmAllMd
**				DpcRmMdInt		
**
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
/************** USER INCLUDE FILES ***************************************************/

#include <MXTypes.h>
#include <MXMem.h>
#include <MXList.h>
#include "Dispatch.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static MXListHead			MdHead	= {NULL, NULL};

static MdInfo*				DpcFindMd(DWORD dwID);
static void					DpcRmAllMd();
static void					DpcRmMdInt(MdInfo* pMdInfo);

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DpcInit
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Dispatch intialize
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
DpcInit()
{
	MdHead.pHead = NULL;
	MdHead.pTail = NULL;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DpcExit
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Dispatch exit
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
DpcExit()
{
	DpcRmAllMd();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DpcAddMd
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Add one module to dispatch
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				dwID		[IN]		DWORD
**				pCbFun		[IN]		const MdCbFun*
**	RETURNED VALUE:	
**				TRUE if succeed, otherwise FALSE
**	NOTES:
**			
*/
BOOL
DpcAddMd(DWORD dwID, const MdCbFun* pCbFun)
{
	MdInfo*	pMdInfo;

	pMdInfo	= DpcFindMd(dwID);
	if (pMdInfo != NULL)
	{
		return FALSE;
	}
	
	pMdInfo = MXNew(MdInfo);
	if (NULL == pMdInfo)
	{
		return FALSE;
	}

	pMdInfo->dwID = dwID;
	pMdInfo->nMsgCnt = 0;
	pMdInfo->MsgHead.pHead = NULL;
	pMdInfo->MsgHead.pTail = NULL;
	pthread_mutex_init(&pMdInfo->Mutex, NULL);
	if (pCbFun != NULL)
	{
		memcpy(&pMdInfo->CbFun, pCbFun, sizeof (MdCbFun));
	}
	else
	{
		memset(&pMdInfo->CbFun, 0, sizeof (MdCbFun));
	}

	MXListAdd(&MdHead, &pMdInfo->List);

	return TRUE;	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DpcRmMd
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Remove one module from dispatch
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				dwID		[IN]		DWORD
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
DpcRmMd(DWORD dwID)
{
	MdInfo*	pMdInfo	= DpcFindMd(dwID);

	DpcRmMdInt(pMdInfo);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MxPutMsg
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Put one message to module queue
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMsg		[IN]		const MXMSG*
**	RETURNED VALUE:	
**				TRUE if succeed, otherwise FALSE
**	NOTES:
**			
*/
BOOL
MxPutMsg(const MXMSG* pMsg)
{
	MXMSGInt*	pMsgInt	= NULL;
	MdInfo*		pMdInfo = NULL;
	
	pMdInfo = DpcFindMd(pMsg->dwDestMd);
	if (NULL == pMdInfo)
	{
		return FALSE;
	}

	pMsgInt = MXNew(MXMSGInt);
	if (NULL == pMsgInt)
	{
		return FALSE;
	}

//	printf("MxPutMsg : alloc %08x\n", (unsigned int) pMsgInt);

	memcpy(&pMsgInt->Msg, pMsg, sizeof (MXMSG));
	pthread_mutex_lock(&pMdInfo->Mutex);
	MXListAdd(&pMdInfo->MsgHead, &pMsgInt->List);
	pMdInfo->nMsgCnt++;
	pthread_mutex_unlock(&pMdInfo->Mutex);

	return TRUE;	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MxPutMsgEx
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Put one message to module queue
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				szSrcDev	[IN]		const char*
**				szDestDev	[IN]		const char*
**				dwSrcMd		[IN]		DWORD
**				dwDestMd	[IN]		DWORD
**				dwMsg		[IN]		DWORD
**				dwParam		[IN]		DWORD
**				pParam		[IN]		unsigned char*
**	RETURNED VALUE:	
**				TRUE if succeed, otherwise FALSE
**	NOTES:
**		21 - Oct - 2006: Add pParam	
*/
BOOL
MxPutMsgEx(const char* szSrcDev, const char* szDestDev, DWORD dwSrcMd, DWORD dwDestMd, DWORD dwMsg, DWORD dwParam, unsigned char* pParam)
{
	MXMSG	Msg;

	strcpy(Msg.szSrcDev, szSrcDev);
	strcpy(Msg.szDestDev, szDestDev);
	Msg.dwSrcMd = dwSrcMd;
	Msg.dwDestMd = dwDestMd;
	Msg.dwMsg = dwMsg;
	Msg.dwParam = dwParam;
	Msg.pParam = pParam;

	return MxPutMsg(&Msg);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MxGetMsg
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Get one message from module queue
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMsg		[OUT]		MXMSG*
**	RETURNED VALUE:	
**				TRUE if succeed, otherwise FALSE
**	NOTES:
**			
*/
BOOL
MxGetMsg(MXMSG* pMsg)
{
	MXMSGInt*	pMsgInt	= NULL;
	MdInfo*		pMdInfo = NULL;
	
	pMdInfo = DpcFindMd(pMsg->dwDestMd);
	if ((NULL == pMdInfo) || (0 == pMdInfo->nMsgCnt))
	{
		return FALSE;
	}

	pMsgInt = (MXMSGInt*) pMdInfo->MsgHead.pHead;
	memcpy(pMsg, &pMsgInt->Msg, sizeof (MXMSG));
	pthread_mutex_lock(&pMdInfo->Mutex);
	MXListRm(&pMdInfo->MsgHead, &pMsgInt->List);
	pMdInfo->nMsgCnt--;
	pthread_mutex_unlock(&pMdInfo->Mutex);

	MXFree(pMsgInt);
//	printf("MxPutMsg : free %08x\n", (unsigned int) pMsgInt);
	
	return TRUE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DpcFindMd
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Find one module
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				dwID		[IN]		DWORD
**	RETURNED VALUE:	
**				MdInfo* pointer or NULL
**	NOTES:
**			
*/
static MdInfo*
DpcFindMd(DWORD dwID)
{
	MdInfo*		pMdInfo	= NULL;
	MXList*		pNext;

	pNext = MdHead.pHead;
	while (pNext != NULL)
	{
		pMdInfo = (MdInfo*) pNext;
		if (pMdInfo->dwID == dwID)
		{
			break;
		}
		pNext = pNext->pNext;
		pMdInfo = NULL;
	}

	return pMdInfo;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DpcRmAllMd
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Remove all module
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
DpcRmAllMd()
{
	MdInfo*		pMdInfo	= NULL;
	MXList*		pNext;

	pNext = MdHead.pHead;
	while (pNext != NULL)
	{
		pMdInfo = (MdInfo*) pNext;
		pNext = pNext->pNext;
		DpcRmMdInt(pMdInfo);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DpcRmMdInt
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Remove one module from dispatch (internal)
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMdInfo		[IN]		MdInfo*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
DpcRmMdInt(MdInfo* pMdInfo)
{
	MXMSG	Msg;

	if (pMdInfo != NULL)
	{
		if (pMdInfo->nMsgCnt > 0)
		{
			while (MxGetMsg(&Msg));
		}
		pthread_mutex_destroy(&pMdInfo->Mutex);
		MXListRm(&MdHead, &pMdInfo->List);
		MXFree(pMdInfo);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DpcPrintAllMd
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Print all module information
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
DpcPrintAllMd()
{
	MdInfo*	pMdInfo	= NULL;
	MXList*		pNext;

	printf("Dispatch: Module List Debug ....\n");
	pNext = MdHead.pHead;
	while (pNext != NULL)
	{
		pMdInfo = (MdInfo*) pNext;
		printf("Addr %08X, ID %u, Prev %08X, Next %08X\n",
			(unsigned int) pMdInfo,
			(unsigned int) pMdInfo->dwID,
			(unsigned int) pNext->pPrev,
			(unsigned int) pNext->pNext);
		pNext = pNext->pNext;
	}
	printf("Dispatch: Module List Debug End\n\n");
}

