/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	BacpNet.c
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		09 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				IsCpltBacpNetFrm
**				PckBacpNetRqt
**				PckBacpNetRqtEx
**				PckBacpNetRsp
**				PckBacpNetRspEx
**				IsBacpNetRqtFrm
**				IsBacpNetRspFrm
**				UnpckBacpNet
**				UnpckBacpNetEx
**				IsBacpNetOptConOn
**				DebugBacpNetFrm
**				
**				GetNextRqtSeq
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
#include "BacpNet.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

//static unsigned int			g_nBacpRqtSeq	= 0x00;
//static unsigned int			GetNextRqtSeq();

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetNextRqtSeq
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Get next sequence of request
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				sequence of request
**	NOTES:
**			
*/
//static unsigned int
//GetNextRqtSeq()
//{
//	return g_nBacpRqtSeq++;
//}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsCpltBacpNetFrm
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
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
BOOL
IsCpltBacpNetFrm(unsigned char* pBuf, int* pnLen)
{
	BOOL				bCpltFrm		= FALSE;
	int					nValidHeadPos	= 0;
	unsigned short		nLen			= 0;
	int					i;

	// Check minimum length
	if (*pnLen < MIN_LEN_BACP_NET_LY)
	{
		return bCpltFrm;
	}

	for (i = 0; i <= *pnLen - MIN_LEN_BACP_NET_LY; i++)
	{
		// Find STX
		if ( (BACP_NET_LY_STX_B0 == *(pBuf + i + 0))	&&
			 (BACP_NET_LY_STX_B1 == *(pBuf + i + 1))	&&
			 (BACP_NET_LY_STX_B2 == *(pBuf + i + 2))	&&
			 (BACP_NET_LY_STX_B3 == *(pBuf + i + 3)))	
		{
			// Save first valid head position
			if (0 == nValidHeadPos)
			{
				nValidHeadPos = i;
			}
#if BACP_WORD_ORDER == 0
			nLen = MAKEWORD(*(pBuf + i + OFFSET_BACP_NET_LY_LEN + 0), *(pBuf + i + OFFSET_BACP_NET_LY_LEN + 1));
#else
			nLen = MAKEWORD(*(pBuf + i + OFFSET_BACP_NET_LY_LEN + 1), *(pBuf + i + OFFSET_BACP_NET_LY_LEN + 0));
#endif			
			// Check LEN, VER, ETX
			if ( (nLen <= *pnLen - i)													&&
				 (CURRENT_BACP_NET_LAY_VER == *(pBuf + i + OFFSET_BACP_NET_LY_VER))		&&	
				 (BACP_NET_LY_ETX_B0 == *(pBuf + i + nLen - LEN_BACP_NET_LY_ETX + 0))	&&
				 (BACP_NET_LY_ETX_B1 == *(pBuf + i + nLen - LEN_BACP_NET_LY_ETX + 1))	&&
				 (BACP_NET_LY_ETX_B2 == *(pBuf + i + nLen - LEN_BACP_NET_LY_ETX + 2))	&&
				 (BACP_NET_LY_ETX_B3 == *(pBuf + i + nLen - LEN_BACP_NET_LY_ETX + 3)))
			{
				bCpltFrm = TRUE;
				break;
			}
		}
	}

	if (bCpltFrm)
	{
		*pnLen = nLen;
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

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PckBacpNetRqt
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Packet network request frame
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pAppBuf		[IN]		unsigned char*
**				nAppBufLen	[IN]		int
**				nSeq		[IN]		unsinged int
**				pFrm		[OUT]		unsigned char*
**	RETURNED VALUE:	
**				frame length
**	NOTES:
**			
*/
int
PckBacpNetRqt(unsigned char* pAppBuf, int nAppBufLen, unsigned int nSeq, unsigned char* pFrm)
{
	return PckBacpNetRqtEx(pAppBuf, nAppBufLen, TRUE, nSeq, pFrm);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PckBacpNetRqtEx
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Packet network request frame
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pAppBuf		[IN]		unsigned char*
**				nAppBufLen	[IN]		int
**				bNeedRsp	[IN]		BOOL	TRUE if need network layer response
**				nSeq		[IN]		unsinged int
**				pFrm		[OUT]		unsigned char*
**	RETURNED VALUE:	
**				frame length
**	NOTES:
**			
*/
int
PckBacpNetRqtEx(unsigned char* pAppBuf, int nAppBufLen, BOOL bNeedRsp, unsigned int nSeq, unsigned char* pFrm)
{
	int				nFrmLen	= 0;
	int				nLenPos	= OFFSET_BACP_NET_LY_LEN;
//	unsigned int	nSeq	= GetNextRqtSeq();

	// STX
	*(pFrm + nFrmLen + 0) = BACP_NET_LY_STX_B0;
	*(pFrm + nFrmLen + 1) = BACP_NET_LY_STX_B1;
	*(pFrm + nFrmLen + 2) = BACP_NET_LY_STX_B2;
	*(pFrm + nFrmLen + 3) = BACP_NET_LY_STX_B3;
	nFrmLen += LEN_BACP_NET_LY_STX;
	// VER
	*(pFrm + nFrmLen) = CURRENT_BACP_NET_LAY_VER;
	nFrmLen += LEN_BACP_NET_LY_VER;
	// LEN
	nFrmLen += LEN_BACP_NET_LY_LEN;
	// SEQ
#if BACP_DWORD_ORDER == 0
	*(pFrm + nFrmLen + 0) = BLOBYTE(nSeq);
	*(pFrm + nFrmLen + 1) = BHIBYTE(nSeq);
	*(pFrm + nFrmLen + 2) = ALOBYTE(nSeq);
	*(pFrm + nFrmLen + 3) = AHIBYTE(nSeq);
#elif BACP_DWORD_ORDER == 1
	*(pFrm + nFrmLen + 0) = AHIBYTE(nSeq);
	*(pFrm + nFrmLen + 1) = ALOBYTE(nSeq);
	*(pFrm + nFrmLen + 2) = BHIBYTE(nSeq);
	*(pFrm + nFrmLen + 3) = BLOBYTE(nSeq);
#endif			
	nFrmLen += LEN_BACP_NET_LY_SEQ;
	// OPT
	if (bNeedRsp)
	{
		*(pFrm + nFrmLen) = BACP_NET_LY_OPT_CON_ON | BACP_NET_LY_OPT_REQ;
	}
	else
	{
		*(pFrm + nFrmLen) = BACP_NET_LY_OPT_CON_OFF | BACP_NET_LY_OPT_REQ;
	}
	nFrmLen += LEN_BACP_NET_LY_OPT;
	// APP
	memcpy((pFrm + nFrmLen), pAppBuf, nAppBufLen);
	nFrmLen += nAppBufLen;
	// ETX
	*(pFrm + nFrmLen + 0) = BACP_NET_LY_ETX_B0;
	*(pFrm + nFrmLen + 1) = BACP_NET_LY_ETX_B1;
	*(pFrm + nFrmLen + 2) = BACP_NET_LY_ETX_B2;
	*(pFrm + nFrmLen + 3) = BACP_NET_LY_ETX_B3;
	nFrmLen += LEN_BACP_NET_LY_ETX;

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
**	FUNCTION NAME:	PckBacpNetRsp
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Packet network response frame
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pRqtFrm		[IN]		unsigned char*
**				nRqtFrmLen	[IN]		int
**				pRspFrm		[OUT]		unsigned char*
**	RETURNED VALUE:	
**				frame length
**	NOTES:
**			
*/
int
PckBacpNetRsp(unsigned char* pRqtFrm, int nRqtFrmLen, unsigned char* pRspFrm)
{
	unsigned int nSeq;

	if (NULL == pRspFrm)
	{
		return 0;
	}

	memcpy(&nSeq, (pRqtFrm + OFFSET_BACP_NET_LY_SEQ), LEN_BACP_NET_LY_SEQ);
		
	return PckBacpNetRspEx(nSeq, pRspFrm);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PckBacpNetRspEx
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Packet network response frame
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nSeq		[IN]		unsigned int
**				pRspFrm		[OUT]		unsigned char*
**	RETURNED VALUE:	
**				frame length
**	NOTES:
**			
*/
int
PckBacpNetRspEx(unsigned int nSeq, unsigned char* pRspFrm)
{
	int						nRspFrmLen							= RSP_LEN_BACP_NET_LY;
	int						nSeqPos								= OFFSET_BACP_NET_LY_SEQ;	
	static unsigned char	PreDefRspFrm[RSP_LEN_BACP_NET_LY]	= {
		// STX
		BACP_NET_LY_STX_B0, BACP_NET_LY_STX_B1, BACP_NET_LY_STX_B2, BACP_NET_LY_STX_B3,
		// VER 
		CURRENT_BACP_NET_LAY_VER, 
		// LEN
#if BACP_WORD_ORDER == 0
		LOBYTE(RSP_LEN_BACP_NET_LY), HIBYTE(RSP_LEN_BACP_NET_LY),
#else
		HIBYTE(RSP_LEN_BACP_NET_LY), LOBYTE(RSP_LEN_BACP_NET_LY),
#endif			
		// SEQ
		0x00, 0x00, 0x00, 0x00,
		// OPT
		BACP_NET_LY_OPT_CON_OFF | BACP_NET_LY_OPT_RSP,
		// ETX
		BACP_NET_LY_ETX_B0, BACP_NET_LY_ETX_B1, BACP_NET_LY_ETX_B2, BACP_NET_LY_ETX_B3};

	if (NULL == pRspFrm)
	{
		return nRspFrmLen;
	}

	memcpy(pRspFrm, PreDefRspFrm, nRspFrmLen);
	
#if BACP_DWORD_ORDER == 0
	*(pRspFrm + nSeqPos + 0) = BLOBYTE(nSeq);
	*(pRspFrm + nSeqPos + 1) = BHIBYTE(nSeq);
	*(pRspFrm + nSeqPos + 2) = ALOBYTE(nSeq);
	*(pRspFrm + nSeqPos + 3) = AHIBYTE(nSeq);
#elif BACP_DWORD_ORDER == 1
	*(pRspFrm + nSeqPos + 0) = AHIBYTE(nSeq);
	*(pRspFrm + nSeqPos + 1) = ALOBYTE(nSeq);
	*(pRspFrm + nSeqPos + 2) = BHIBYTE(nSeq);
	*(pRspFrm + nSeqPos + 3) = BLOBYTE(nSeq);
#endif			
	
	return nRspFrmLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsBacpNetRqtFrm
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Judge whether the frame is request frame
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pFrm		[IN]		unsigned char*
**				nFrmLen		[IN]		int
**	RETURNED VALUE:	
**				TRUE if the frame is request frame
**	NOTES:
**		
*/
BOOL
IsBacpNetRqtFrm(unsigned char* pFrm, int nFrmLen)
{
	return (!IsBacpNetRspFrm(pFrm, nFrmLen));
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsBacpNetRspFrm
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Judge whether the frame is response frame
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pFrm		[IN]		unsigned char*
**				nFrmLen		[IN]		int
**	RETURNED VALUE:	
**				TRUE if the frame is response frame
**	NOTES:
**		
*/
BOOL
IsBacpNetRspFrm(unsigned char* pFrm, int nFrmLen)
{
	BOOL	bRetCode	= FALSE;

	if (BACP_NET_LY_OPT_RSP == (*(pFrm + OFFSET_BACP_NET_LY_OPT) & BACP_NET_LY_OPT_RSP))
	{
		bRetCode = TRUE;
	}

	return bRetCode;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UnpckBacpNet
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Unpack BACP network layer frame				
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pFrm		[IN]		unsigned char*
**				nFrmLen		[IN]		int
**				pBscFld		[OUT]		BacpNetBscFld*
**				pApp		[OUT]		unsinged char*
**	RETURNED VALUE:	
**				Application data length
**	NOTES:
**		Application data is copyed to pApp buffer
*/
int
UnpckBacpNet(unsigned char* pFrm, int nFrmLen, BacpNetBscFld* pBscFld, unsigned char* pApp)
{
	int				nAppLen	= 0;
	unsigned char*	pAppIn	= NULL;

	nAppLen = UnpckBacpNetEx(pFrm, nFrmLen, pBscFld, &pAppIn);
	if (nAppLen > 0)
	{
		memcpy(pApp, pAppIn, nAppLen);
	}

	return nAppLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UnpckBacpNetEx
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Unpack BACP network layer frame				
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pFrm		[IN]		unsigned char*
**				nFrmLen		[IN]		int
**				pBscFld		[OUT]		BacpNetBscFld*
**				ppApp		[OUT]		unsinged char**
**	RETURNED VALUE:	
**				Application data length
**	NOTES:
**		Application data pointer is stored in *ppApp 
*/
int
UnpckBacpNetEx(unsigned char* pFrm, int nFrmLen, BacpNetBscFld* pBscFld, unsigned char** ppApp)
{
	int		nAppLen	= 0;

	*ppApp	= NULL;
	
	// STX
	pBscFld->Stx[0]	= (BYTE)*(pFrm + OFFSET_BACP_NET_LY_STX + 0);
	pBscFld->Stx[1]	= *(pFrm + OFFSET_BACP_NET_LY_STX + 1);
	pBscFld->Stx[2]	= *(pFrm + OFFSET_BACP_NET_LY_STX + 2);
	pBscFld->Stx[3]	= *(pFrm + OFFSET_BACP_NET_LY_STX + 3);
	// VER
	pBscFld->Ver	= *(pFrm + OFFSET_BACP_NET_LY_VER);
	// LEN
#if BACP_WORD_ORDER == 0
	pBscFld->Len	= MAKEWORD(*(pFrm + OFFSET_BACP_NET_LY_LEN + 0), *(pFrm + OFFSET_BACP_NET_LY_LEN + 1));
#else
	pBscFld->Len	= MAKEWORD(*(pFrm + OFFSET_BACP_NET_LY_LEN + 1), *(pFrm + OFFSET_BACP_NET_LY_LEN + 0));
#endif
	// SEQ
#if BACP_DWORD_ORDER == 0
	pBscFld->Seq	= MAKEDWORD(*(pFrm + OFFSET_BACP_NET_LY_SEQ + 3), *(pFrm + OFFSET_BACP_NET_LY_SEQ + 2), *(pFrm + OFFSET_BACP_NET_LY_SEQ + 1), *(pFrm + OFFSET_BACP_NET_LY_SEQ + 0));
#elif BACP_DWORD_ORDER == 1
	pBscFld->Seq	= MAKEDWORD(*(pFrm + OFFSET_BACP_NET_LY_SEQ + 0), *(pFrm + OFFSET_BACP_NET_LY_SEQ + 1), *(pFrm + OFFSET_BACP_NET_LY_SEQ + 2), *(pFrm + OFFSET_BACP_NET_LY_SEQ + 3));
#endif			
	// OPT
	pBscFld->Opt	= *(pFrm + OFFSET_BACP_NET_LY_OPT);
	// APP
	if (pBscFld->Len > MIN_LEN_BACP_NET_LY)
	{
		nAppLen = pBscFld->Len - MIN_LEN_BACP_NET_LY;
		*ppApp = pFrm + OFFSET_BACP_NET_LY_APP;
	}
	// ETX
	pBscFld->Etx[0]	= *(pFrm + pBscFld->Len - LEN_BACP_NET_LY_ETX + 0);
	pBscFld->Etx[1]	= *(pFrm + pBscFld->Len - LEN_BACP_NET_LY_ETX + 1);
	pBscFld->Etx[2]	= *(pFrm + pBscFld->Len - LEN_BACP_NET_LY_ETX + 2);
	pBscFld->Etx[3]	= *(pFrm + pBscFld->Len - LEN_BACP_NET_LY_ETX + 3);

	return nAppLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsBacpNetOptConOn	
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Check whether CONFIRM flag is on
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				ucOpt		[IN]		unsigned char
**	RETURNED VALUE:	
**				TRUE if CONFIRM flag on
**	NOTES:
**			
*/
BOOL
IsBacpNetOptConOn(unsigned char ucOpt)
{
	BOOL	bRetCode	= FALSE;

	if ((ucOpt & BACP_NET_LY_OPT_CON_ON) == BACP_NET_LY_OPT_CON_ON)
	{
		bRetCode = TRUE;
	}

	return bRetCode;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DebugBacpNetFrm
**	AUTHOR:			Jerry Huang
**	DATE:			10 - Oct - 2006
**
**	DESCRIPTION:	
**			Print network layer frame info
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pFrm		[IN]		unsigned char*
**				nFrmLen		[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
DebugBacpNetFrm(unsigned char* pFrm, int nFrmLen)
{
#if BACP_WORD_ORDER == 0
	unsigned short	nLen = MAKEWORD(*(pFrm + OFFSET_BACP_NET_LY_LEN + 0), *(pFrm + OFFSET_BACP_NET_LY_LEN + 1));
#else
	unsigned short	nLen = MAKEWORD(*(pFrm + OFFSET_BACP_NET_LY_LEN + 1), *(pFrm + OFFSET_BACP_NET_LY_LEN + 0));
#endif			
	int				i;

	printf("BACP NetWork Layer Frame Debug ......\n");
	printf("STX: %02X %02X %02X %02X\n", 
		*(pFrm + OFFSET_BACP_NET_LY_STX + 0),
		*(pFrm + OFFSET_BACP_NET_LY_STX + 1),
		*(pFrm + OFFSET_BACP_NET_LY_STX + 2),
		*(pFrm + OFFSET_BACP_NET_LY_STX + 3));	
	printf("VER: %02X\n", *(pFrm + OFFSET_BACP_NET_LY_VER));	
	printf("LEN: %02X %02X\n", 
		*(pFrm + OFFSET_BACP_NET_LY_LEN + 0),
		*(pFrm + OFFSET_BACP_NET_LY_LEN + 1));	
	printf("SEQ: %02X %02X %02X %02X\n", 
		*(pFrm + OFFSET_BACP_NET_LY_SEQ + 0),
		*(pFrm + OFFSET_BACP_NET_LY_SEQ + 1),
		*(pFrm + OFFSET_BACP_NET_LY_SEQ + 2),
		*(pFrm + OFFSET_BACP_NET_LY_SEQ + 3));	
	printf("OPT: %02X\n", *(pFrm + OFFSET_BACP_NET_LY_OPT));	

	printf("APP: ");
	for (i = 0; i < nLen - MIN_LEN_BACP_NET_LY; i++)
	{
		printf("%02X ", *(pFrm + OFFSET_BACP_NET_LY_APP + i));
	}
	printf("\n");

	printf("ETX: %02X %02X %02X %02X\n", 
		*(pFrm + nLen - LEN_BACP_NET_LY_ETX + 0),
		*(pFrm + nLen - LEN_BACP_NET_LY_ETX + 1),
		*(pFrm + nLen - LEN_BACP_NET_LY_ETX + 2),
		*(pFrm + nLen - LEN_BACP_NET_LY_ETX + 3));	
}
