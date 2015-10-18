/************** SYSTEM INCLUDE FILES **************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXTypes.h"
#include "BacpSerial.h"

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

static unsigned short
CalcCheckSum(unsigned char* pBuf, unsigned short len)
{
	unsigned short		value			= 0;
	unsigned char			temp			= 0;
	unsigned short		i				= 0;

	for (i = 0; i < len; i++)
	{
		temp = (unsigned char)(*(pBuf + i));
		value += temp;
	}

	return value;
}

BOOL
IsCorrectCheckSum(unsigned char* pBuf, unsigned short len, unsigned char chksum)
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

	value = ~value + 1;

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

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsValidBacpSerialFrm
**
**	DESCRIPTION:	
**			Check whether the frame is valid
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
IsValidBacpSerialFrm(unsigned char* pBuf, int* pnLen)
{
	BOOL				bCpltFrm		= FALSE;
	int					nValidHeadPos	= 0;
	unsigned short		nLen			= 0;
	int					i;

	// Check minimum length
	if (*pnLen < MIN_LEN_BACP_SERIAL)
	{
		return bCpltFrm;
	}

	for (i = 0; i <= *pnLen - MIN_LEN_BACP_SERIAL; i++)
	{
		// Find STX
		if ( (BACP_SERIAL_STX_B0 == *(pBuf + i + 0))	&&
			 (BACP_SERIAL_STX_B1 == *(pBuf + i + 1)))
		{
			// Save first valid head position
			if (0 == nValidHeadPos)
			{
				nValidHeadPos = i;
			}
			nLen = MAKEWORD(*(pBuf + i + OFFSET_BACP_SERIAL_LEN + 0), *(pBuf + i + OFFSET_BACP_SERIAL_LEN + 1));
			
			// Check LEN, VER, CHK, ETX
			if ( (nLen <= *pnLen - i)													&&
				 (CURRENT_BACP_SERIAL_VER == *(pBuf + i + OFFSET_BACP_SERIAL_VER))		&&	
				 (BACP_SERIAL_ETX_B0 == *(pBuf + i + nLen - LEN_BACP_SERIAL_ETX + 0))	&&
				 (BACP_SERIAL_ETX_B1 == *(pBuf + i + nLen - LEN_BACP_SERIAL_ETX + 1))	&&
				 (IsCorrectCheckSum((unsigned char* )(pBuf + nValidHeadPos + OFFSET_BACP_SERIAL_VER),
				 	(nLen - LEN_BACP_SERIAL_STX - LEN_BACP_SERIAL_ETX - LEN_BACP_SERIAL_CHK),
				 	(*(pBuf + nValidHeadPos + nLen - LEN_BACP_SERIAL_ETX - LEN_BACP_SERIAL_CHK)))) )
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
**	FUNCTION NAME:	PckBacpSerialRqt
**
**	DESCRIPTION:	
**			Packet Serial request frame
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
PckBacpSerialRqt(unsigned char* pAppBuf, int nAppBufLen, unsigned int nSeq, unsigned char* pFrm)
{
	return PckBacpSerialRqtEx(pAppBuf, nAppBufLen, TRUE, nSeq, pFrm);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PckBacpSerialRqtEx
**
**	DESCRIPTION:	
**			Packet Serial request frame
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
PckBacpSerialRqtEx(unsigned char* pAppBuf, int nAppBufLen, BOOL bNeedRsp, unsigned char nSeq, unsigned char* pFrm)
{
	int				nFrmCursor	= 0;
	int				nLenPos		= OFFSET_BACP_SERIAL_LEN;
	int				i			= 0;
	unsigned short	checksum	= 0;
	int				nChkPos		= 0;
//	unsigned int		nSeq		= GetNextRqtSeq();

	// STX
	*(pFrm + nFrmCursor + 0) = BACP_SERIAL_STX_B0;
	*(pFrm + nFrmCursor + 1) = BACP_SERIAL_STX_B1;
	nFrmCursor += LEN_BACP_SERIAL_STX;
	// VER
	*(pFrm + nFrmCursor) = CURRENT_BACP_SERIAL_VER;
	nFrmCursor += LEN_BACP_SERIAL_VER;
	// LEN
	nFrmCursor += LEN_BACP_SERIAL_LEN;
	// SEQ
	*(pFrm + nFrmCursor) = nSeq;
	nFrmCursor += LEN_BACP_SERIAL_SEQ;
	// OPT
	if (bNeedRsp)
	{
		*(pFrm + nFrmCursor) = BACP_SERIAL_OPT_CON_ON | BACP_SERIAL_OPT_REQ;
	}
	else
	{
		*(pFrm + nFrmCursor) = BACP_SERIAL_OPT_CON_OFF | BACP_SERIAL_OPT_REQ;
	}
	nFrmCursor += LEN_BACP_SERIAL_OPT;
	// ADDR
	*(pFrm + nFrmCursor + 0) = BACP_SERIAL_ADDR_DEFAULT;
	*(pFrm + nFrmCursor + 1) = BACP_SERIAL_ADDR_DEFAULT;	
	*(pFrm + nFrmCursor + 2) = BACP_SERIAL_ADDR_DEFAULT;
	*(pFrm + nFrmCursor + 3) = BACP_SERIAL_ADDR_DEFAULT;	
	*(pFrm + nFrmCursor + 4) = BACP_SERIAL_ADDR_DEFAULT;
	*(pFrm + nFrmCursor + 5) = BACP_SERIAL_ADDR_DEFAULT;	
	nFrmCursor += LEN_BACP_SERIAL_ADDR;
	// APP
	memcpy((pFrm + nFrmCursor), pAppBuf, nAppBufLen);
	nFrmCursor += nAppBufLen;
	// CHK
	nChkPos = nFrmCursor;
	nFrmCursor += LEN_BACP_SERIAL_CHK;
	// ETX
	*(pFrm + nFrmCursor + 0) = BACP_SERIAL_ETX_B0;
	*(pFrm + nFrmCursor + 1) = BACP_SERIAL_ETX_B1;
	nFrmCursor += LEN_BACP_SERIAL_ETX;

	// Fill LEN 
	*(pFrm + nLenPos + 0) = LOBYTE(nFrmCursor);
	*(pFrm + nLenPos + 1) = HIBYTE(nFrmCursor);

	// Fill CHK
	checksum = CalcCheckSum((pFrm + OFFSET_BACP_SERIAL_VER), (MIN_LEN_BACP_SERIAL - LEN_BACP_SERIAL_STX - LEN_BACP_SERIAL_CHK - LEN_BACP_SERIAL_ETX + nAppBufLen));
	*(pFrm + nChkPos + 0) = LOBYTE(checksum);
	*(pFrm + nChkPos + 1) = HIBYTE(checksum);

	return nFrmCursor;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PckBacpSerialRsp
**
**	DESCRIPTION:	
**			Packet Serial response frame
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
PckBacpSerialRsp(unsigned char* pRqtFrm, int nRqtFrmLen, unsigned char* pRspFrm)
{
	unsigned char nSeq;

	if (NULL == pRspFrm)
	{
		return 0;
	}

	memcpy(&nSeq, (pRqtFrm + OFFSET_BACP_SERIAL_SEQ), LEN_BACP_SERIAL_SEQ);
		
	return PckBacpSerialRspEx(nSeq, pRspFrm);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PckBacpSerialRspEx
**
**	DESCRIPTION:	
**			Packet Serial response frame
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
PckBacpSerialRspEx(unsigned char nSeq, unsigned char* pRspFrm)
{
	unsigned short		checksum		= 0;
	int					nRspFrmLen		= RSP_LEN_BACP_SERIAL;
	int					nSeqPos			= OFFSET_BACP_SERIAL_SEQ;	
	static unsigned char	PreDefRspFrm[RSP_LEN_BACP_SERIAL]	= {
		// STX
		BACP_SERIAL_STX_B0, BACP_SERIAL_STX_B1,
		// VER 
		CURRENT_BACP_SERIAL_VER, 
		// LEN
		LOBYTE(RSP_LEN_BACP_SERIAL), HIBYTE(RSP_LEN_BACP_SERIAL),
		// SEQ
		0x00,
		// OPT
		BACP_SERIAL_OPT_CON_OFF | BACP_SERIAL_OPT_RSP,
		// ADDR
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		// CHK
		0x00, 0x00,
		// ETX
		BACP_SERIAL_ETX_B0, BACP_SERIAL_ETX_B1};

	if (NULL == pRspFrm)
	{
		return nRspFrmLen;
	}

	memcpy(pRspFrm, PreDefRspFrm, nRspFrmLen);

	// SEQ
	*(pRspFrm + nSeqPos) = nSeq;
	
	// CHK
	checksum = CalcCheckSum((pRspFrm + OFFSET_BACP_SERIAL_VER), (RSP_LEN_BACP_SERIAL - LEN_BACP_SERIAL_STX - LEN_BACP_SERIAL_CHK - LEN_BACP_SERIAL_ETX));
	*(pRspFrm + RSP_LEN_BACP_SERIAL - LEN_BACP_SERIAL_CHK - LEN_BACP_SERIAL_ETX + 0) = LOBYTE(checksum);
	*(pRspFrm + RSP_LEN_BACP_SERIAL - LEN_BACP_SERIAL_CHK - LEN_BACP_SERIAL_ETX + 1) = HIBYTE(checksum);

	return nRspFrmLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsBacpSerialRqtFrm
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
IsBacpSerialRqtFrm(unsigned char* pFrm, int nFrmLen)
{
	return (!IsBacpSerialRspFrm(pFrm, nFrmLen));
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsBacpSerialRspFrm
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
IsBacpSerialRspFrm(unsigned char* pFrm, int nFrmLen)
{
	BOOL	bRetCode	= FALSE;

	if (BACP_SERIAL_OPT_RSP == ((*(pFrm + OFFSET_BACP_SERIAL_OPT)) & BACP_SERIAL_OPT_RSP))
	{
		bRetCode = TRUE;
	}

	return bRetCode;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UnpckBacpSerial
**
**	DESCRIPTION:	
**			Unpack BACP Serial layer frame				
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
UnpckBacpSerial(unsigned char* pFrm, int nFrmLen, BacpSerialBscFld* pBscFld, unsigned char* pApp)
{
	int				nAppLen	= 0;
	unsigned char*	pAppIn	= NULL;

	nAppLen = UnpckBacpSerialEx(pFrm, nFrmLen, pBscFld, &pAppIn);
	if (nAppLen > 0)
	{
		memcpy(pApp, pAppIn, nAppLen);
	}

	return nAppLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UnpckBacpSerialEx
**
**	DESCRIPTION:	
**			Unpack BACP Serial layer frame				
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
UnpckBacpSerialEx(unsigned char* pFrm, int nFrmLen, BacpSerialBscFld* pBscFld, unsigned char** ppApp)
{
	int		nAppLen	= 0;

	*ppApp	= NULL;
	
	// STX
	pBscFld->Stx[0]	= (BYTE)*(pFrm + OFFSET_BACP_SERIAL_STX + 0);
	pBscFld->Stx[1]	= *(pFrm + OFFSET_BACP_SERIAL_STX + 1);
	// VER
	pBscFld->Ver	= *(pFrm + OFFSET_BACP_SERIAL_VER);
	// LEN
	pBscFld->Len	= MAKEWORD(*(pFrm + OFFSET_BACP_SERIAL_LEN + 0), *(pFrm + OFFSET_BACP_SERIAL_LEN + 1));
	// SEQ
	pBscFld->Seq	= *(pFrm + OFFSET_BACP_SERIAL_SEQ);
	// OPT
	pBscFld->Opt	= *(pFrm + OFFSET_BACP_SERIAL_OPT);
	// ADDR
	pBscFld->Da1	= *(pFrm + OFFSET_BACP_SERIAL_ADDR + 0);
	pBscFld->Da2	= *(pFrm + OFFSET_BACP_SERIAL_ADDR + 1);
	pBscFld->Da3	= *(pFrm + OFFSET_BACP_SERIAL_ADDR + 2);
	pBscFld->Sa1	= *(pFrm + OFFSET_BACP_SERIAL_ADDR + 3);
	pBscFld->Sa2	= *(pFrm + OFFSET_BACP_SERIAL_ADDR + 4);
	pBscFld->Sa3	= *(pFrm + OFFSET_BACP_SERIAL_ADDR + 5);
	// APP
	if (pBscFld->Len > MIN_LEN_BACP_SERIAL)
	{
		nAppLen = pBscFld->Len - MIN_LEN_BACP_SERIAL;
		*ppApp = pFrm + OFFSET_BACP_SERIAL_APP;
	}
	// CHK
	pBscFld->Chk	= MAKEWORD(*(pFrm + pBscFld->Len - LEN_BACP_SERIAL_ETX - LEN_BACP_SERIAL_CHK + 0), *(pFrm + pBscFld->Len - LEN_BACP_SERIAL_ETX -LEN_BACP_SERIAL_CHK + 1));
	// ETX
	pBscFld->Etx[0]	= *(pFrm + pBscFld->Len - LEN_BACP_SERIAL_ETX + 0);
	pBscFld->Etx[1]	= *(pFrm + pBscFld->Len - LEN_BACP_SERIAL_ETX + 1);

	return nAppLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsBacpNetOptConOn
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
IsBacpSerialOptConOn(unsigned char ucOpt)
{
	BOOL	bRetCode	= FALSE;

	if ((ucOpt & BACP_SERIAL_OPT_CON_ON) == BACP_SERIAL_OPT_CON_ON)
	{
		bRetCode = TRUE;
	}

	return bRetCode;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DebugBacpSerialFrm
**
**	DESCRIPTION:	
**			Print Serial layer frame info
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
DebugBacpSerialFrm(unsigned char* pFrm, int nFrmLen)
{
	unsigned short	nLen = MAKEWORD(*(pFrm + OFFSET_BACP_SERIAL_LEN + 0), *(pFrm + OFFSET_BACP_SERIAL_LEN + 1));
	int				i;

	printf("BACP Serial Layer Frame Debug ......\n");
	printf("STX: %02X %02X\n", 
		*(pFrm + OFFSET_BACP_SERIAL_STX + 0),
		*(pFrm + OFFSET_BACP_SERIAL_STX + 1));	
	printf("VER: %02X\n", *(pFrm + OFFSET_BACP_SERIAL_VER));	
	printf("LEN: %02X %02X\n", 
		*(pFrm + OFFSET_BACP_SERIAL_LEN + 0),
		*(pFrm + OFFSET_BACP_SERIAL_LEN + 1));	
	printf("SEQ: %02X\n", *(pFrm + OFFSET_BACP_SERIAL_SEQ));	
	printf("OPT: %02X\n", *(pFrm + OFFSET_BACP_SERIAL_OPT));	
	printf("ADDR: %02X %02X %02X %02X %02X %02X\n", 
		*(pFrm + OFFSET_BACP_SERIAL_ADDR + 0),
		*(pFrm + OFFSET_BACP_SERIAL_ADDR + 1),
		*(pFrm + OFFSET_BACP_SERIAL_ADDR + 2),
		*(pFrm + OFFSET_BACP_SERIAL_ADDR + 3),
		*(pFrm + OFFSET_BACP_SERIAL_ADDR + 4),
		*(pFrm + OFFSET_BACP_SERIAL_ADDR + 5));
	
	printf("APP: ");
	for (i = 0; i < nLen - MIN_LEN_BACP_SERIAL; i++)
	{
		printf("%02X ", *(pFrm + OFFSET_BACP_SERIAL_APP + i));
	}
	printf("\n");

	printf("CHK: %02X %02X\n", 
		*(pFrm + nLen - LEN_BACP_SERIAL_CHK - LEN_BACP_SERIAL_ETX + 0),
		*(pFrm + nLen - LEN_BACP_SERIAL_CHK - LEN_BACP_SERIAL_ETX + 1));

	printf("ETX: %02X %02X\n", 
		*(pFrm + nLen - LEN_BACP_SERIAL_ETX + 0),
		*(pFrm + nLen - LEN_BACP_SERIAL_ETX + 1));	
}

