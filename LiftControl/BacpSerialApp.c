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
#include "BacpSerial.h"
#include "BacpSerialApp.h"

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
**	FUNCTION NAME:	PckBacpSerialApp
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
PckSerialBacpApp(unsigned char nFunCategory, unsigned char nFunCode, unsigned char* pData, int nDataLen, unsigned char* pFrm)
{
	int		nFrmLen	= 0;
	int		nLenPos		= 0;

	// FUNCTION CATEGORY
	*(pFrm + nFrmLen) = nFunCategory;
	nFrmLen += LEN_BACP_APP_SERIAL_CAT;	
	// FUNCTION CODE
	*(pFrm + nFrmLen) = nFunCode;
	nFrmLen += LEN_BACP_APP_SERIAL_FC;
	// LEN
	nLenPos = nFrmLen;
	nFrmLen += LEN_BACP_APP_SERIAL_LEN;
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
**	FUNCTION NAME:	PckBacpSerialApp
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
UnpckBacpSerialApp(unsigned char* pnFunCategory, unsigned char* pnFunCode, unsigned char* pData, unsigned short* pnDataLen, unsigned char* pFrm, int nFrmLen)
{
	unsigned char*	pDataTmp;
	int				nFrmLenInner;

	nFrmLenInner = UnpckBacpSerialAppEx(pnFunCategory, pnFunCode, &pDataTmp, pnDataLen, pFrm, nFrmLen);
	if (nFrmLenInner > 0)
	{
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
**	FUNCTION NAME:	UnpckBacpSerialAppEx
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
UnpckBacpSerialAppEx(unsigned char* pnFunCategory, unsigned char* pnFunCode, unsigned char** ppData, unsigned short* pnDataLen, unsigned char* pFrm, int nFrmLen)
{
	int		nFrmLenInner	= 0;

	if (nFrmLen < MIN_LEN_BACP_APP_SERIAL)
	{
		return -1;
	}

	*pnDataLen	= 0;

	// FUNTION CATEGORY
	*pnFunCategory	= *(pFrm + OFFSET_BACP_APP_SERIAL_CAT);
	// FUNTION CODE
	*pnFunCode		= *(pFrm + OFFSET_BACP_APP_SERIAL_FC);
	// LEN
	nFrmLenInner	= MAKEWORD(*(pFrm + OFFSET_BACP_APP_SERIAL_LEN + 0), *(pFrm + OFFSET_BACP_APP_SERIAL_LEN + 1));
	if ((nFrmLenInner < nFrmLen) || (nFrmLenInner < MIN_LEN_BACP_APP_SERIAL))
	{
		return -1;
	}
	// DATA
	*ppData = (unsigned char*) (pFrm + OFFSET_BACP_APP_SERIAL_DATA);
	*pnDataLen = (unsigned short)(nFrmLenInner - MIN_LEN_BACP_APP_SERIAL);
	printf("************ nFrmLenInner = %d, pnDataLen = %d\n", nFrmLenInner, *pnDataLen);

	return nFrmLenInner;
}

void
DebugBacpSerialAppFrm(unsigned char* pFrm, int nFrmLen)
{
	unsigned short	nLen = MAKEWORD(*(pFrm + LEN_BACP_APP_SERIAL_LEN + 0), *(pFrm + LEN_BACP_APP_SERIAL_LEN + 1));
	int				i;

	printf("BACP Serial APP Layer Frame Debug ......\n");
	printf("CATEGORY: %02X\n", *(pFrm + OFFSET_BACP_APP_SERIAL_CAT));	
	printf("FUNCTION CODE: %02X\n", *(pFrm + OFFSET_BACP_APP_SERIAL_FC));	
	printf("LEN: %02X %02X\n", 
		*(pFrm + OFFSET_BACP_APP_SERIAL_LEN + 0),
		*(pFrm + OFFSET_BACP_APP_SERIAL_LEN + 1));
	printf("APP: ");
	for (i = 0; i < nLen - MIN_LEN_BACP_APP_SERIAL; i++)
	{
		printf("%02X ", *(pFrm + OFFSET_BACP_APP_SERIAL_DATA + i));
	}
	printf("\n");
}



