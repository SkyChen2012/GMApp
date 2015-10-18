/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	BacpApp.c
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		10 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				PckBacpApp
**				UnpckBacpApp				
**				UnpckBacpAppEx
**
**				
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXTypes.h"
#include "MXList.h"
#include "MXMem.h"
#include "Dispatch.h"
#include "Bacp.h"
#include "BacpNet.h"
#include "BacpApp.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PckBacpApp
**	AUTHOR:			Jerry Huang
**	DATE:			12 - Oct - 2006
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
PckBacpApp(unsigned short nFunCode, char* pSrcDev, char* pDestDev, unsigned char* pData, int nDataLen, unsigned char* pFrm)
{
	int		nFrmLen		= 0;
	int		nLenPos		= 0;

	// FUNCTION CODE
#if BACP_WORD_ORDER == 0
	*(pFrm + nFrmLen + 0) = LOBYTE(nFunCode);
	*(pFrm + nFrmLen + 1) = HIBYTE(nFunCode);
#else
	*(pFrm + nFrmLen + 0) = HIBYTE(nFunCode);
	*(pFrm + nFrmLen + 1) = LOBYTE(nFunCode);
#endif	
	nFrmLen += LEN_BACP_APP_LY_FC;
	// LEN
	nLenPos = nFrmLen;
	nFrmLen += LEN_BACP_APP_LY_LEN;
	// SOURCE DEVICE CODE
	memset((pFrm + nFrmLen), 0, LEN_BACP_APP_LY_SRC_DEV);
	memcpy((pFrm + nFrmLen), pSrcDev, strlen(pSrcDev));
	nFrmLen += LEN_BACP_APP_LY_SRC_DEV;
	// DESTATION DEVICE CODE
	memset((pFrm + nFrmLen), 0, LEN_BACP_APP_LY_DEST_DEV);
	memcpy((pFrm + nFrmLen), pDestDev, strlen(pDestDev));
	nFrmLen += LEN_BACP_APP_LY_DEST_DEV;
	// DATA
	if (nDataLen > 0)
	{
		memcpy((pFrm + nFrmLen), pData, nDataLen);
		nFrmLen += nDataLen;
	}

	// Fill LEN
#if BACP_WORD_ORDER == 0
	*(pFrm + nLenPos + 0) = LOBYTE(nFrmLen);
	*(pFrm + nLenPos + 1) = HIBYTE(nFrmLen);
#else
	*(pFrm + nLenPos + 0) = HIBYTE(nFrmLen);
	*(pFrm + nLenPos + 1) = LOBYTE(nFrmLen);
#endif	

	return nFrmLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PckBacpApp
**	AUTHOR:			Jerry Huang
**	DATE:			12 - Oct - 2006
**
**	DESCRIPTION:	
**			Unpack BACP application data
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pnFunCode	[OUT]		unsigned short*
**				pSrcDev		[OUT]		char*
**				pDestDev	[OUT]		char*	
**				pData		[OUT]		unsigned char*
**				pnDataLen	[OUT]		int*
**				pFrm		[IN]		unsigned char*
**				nFrmLen		[IN]		int
**	RETURNED VALUE:	
**				error -1 or real BACP application packet length
**	NOTES:
**			
*/
int
UnpckBacpApp(unsigned short* pnFunCode, char* pSrcDev, char* pDestDev, unsigned char* pData, int* pnDataLen, unsigned char* pFrm, int nFrmLen)
{
	char*			pSrcDevTmp;
	char*			pDestDevTmp;
	unsigned char*	pDataTmp;
	int				nFrmLenInner;

	nFrmLenInner = UnpckBacpAppEx(pnFunCode, &pSrcDevTmp, &pDestDevTmp, &pDataTmp, pnDataLen, pFrm, nFrmLen);
	if (nFrmLenInner > 0)
	{
		strcpy(pSrcDev, pSrcDevTmp);
		strcpy(pDestDev, pDestDevTmp);
		if (*pnDataLen > 0)
		{
			memcpy(pData, pDataTmp, *pnDataLen);
		}
	}

	return nFrmLenInner;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UnpckBacpAppEx
**	AUTHOR:			Jerry Huang
**	DATE:			12 - Oct - 2006
**
**	DESCRIPTION:	
**			Unpack BACP application data
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pnFunCode	[OUT]		unsigned short*
**				ppSrcDev	[OUT]		char**
**				ppDestDev	[OUT]		char**	
**				ppData		[OUT]		unsigned char**
**				pnDataLen	[OUT]		int*
**				pFrm		[IN]		unsigned char*
**				nFrmLen		[IN]		int
**	RETURNED VALUE:	
**				error -1 or real BACP application packet length
**	NOTES:
**			
*/
int
UnpckBacpAppEx(unsigned short* pnFunCode, char** ppSrcDev, char** ppDestDev, unsigned char** ppData, int* pnDataLen, unsigned char* pFrm, int nFrmLen)
{
	int		nFrmLenInner	= 0;

	if (nFrmLen < MIN_LEN_BACP_APP_LAY)
	{
		return -1;
	}

	*pnDataLen	= 0;
	
	// FUNTION CODE
#if BACP_WORD_ORDER == 0
	*pnFunCode		= MAKEWORD(*(pFrm + OFFSET_BACP_APP_LY_FC + 0), *(pFrm + OFFSET_BACP_APP_LY_FC + 1));
#else
	*pnFunCode		= MAKEWORD(*(pFrm + OFFSET_BACP_APP_LY_FC + 1), *(pFrm + OFFSET_BACP_APP_LY_FC + 0));
#endif
	// LEN
#if BACP_WORD_ORDER == 0
	nFrmLenInner	= MAKEWORD(*(pFrm + OFFSET_BACP_APP_LY_LEN + 0), *(pFrm + OFFSET_BACP_APP_LY_LEN + 1));
#else
	nFrmLenInner	= MAKEWORD(*(pFrm + OFFSET_BACP_APP_LY_LEN + 1), *(pFrm + OFFSET_BACP_APP_LY_LEN + 0));
#endif

	if ((nFrmLenInner < nFrmLen) || (nFrmLenInner < MIN_LEN_BACP_APP_LAY))
	{
		return -1;
	}

	// SOURCE DEVICE CODE
	*ppSrcDev = (char*) (pFrm + OFFSET_BACP_APP_LY_SRC_DEV);
	// DESTINATION DEVICE CODE
	*ppDestDev = (char*) (pFrm + OFFSET_BACP_APP_LY_DEST_DEV);
	// DATA
	*ppData = (unsigned char*) (pFrm + OFFSET_BACP_APP_LY_DATA);
	*pnDataLen = nFrmLenInner - MIN_LEN_BACP_APP_LAY;

	return nFrmLenInner;
}
