
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	CardRead.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		13 - April - 2009
**
**	FILE DESCRIPTION:
**
**
**	FUNCTIONS:
**
**	NOTES:
** 
*/
/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <errno.h>
#include <sys/timeb.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>  
#include <termios.h>

/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"

#include "AccessCommon.h"
#include "TelnetLog.h"
#include "PioApi.h"
#include "CardRead.h"
#include "DiagTelnetd.h"
#include "MenuParaProc.h"

/************** DEFINES **************************************************************/

//#define HID_CARDREADER
#define MOX_CARDREADER


#define		HID_PORT		2
#define		DATAREDINTERVALTIME  50
#define     DATAPKTDATATIME				500

/************** TYPEDEFS *************************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static INT nFdUart = 0;

static INT serial_port_open(INT nPort);

static VOID	CSNNotifyAck(CHAR* pRcvData);

static INT UnpckSerialEx(BYTE* pFrm, INT nFrmLen, SerialPacket* pSerPck, BYTE** ppApp);
static INT UnpckAppEx(BYTE* pApp, INT nAppLen, AppPacket* pAppPck, BYTE** ppData);

static BOOL IsCompletePacket(BYTE* pBuffer, INT* pnTotalLen, INT* pnLen);
static DWORD CheckSum(BYTE* pBuf, INT nLen);
/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadCardInit
**	AUTHOR:		Jeff Wang
**	DATE:			10 - April - 2009
**
**	DESCRIPTION:	
**			Read Card Number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pOutBuf		[IN]		CHAR*		Buffer
**	RETURNED VALUE:	
**				read length
**	NOTES:
**			
*/
VOID
ReadCardInit(VOID)
{
	PIOWrite(PIO_TYPE_HID_HOLD, 1);
	
	nFdUart = serial_port_open(HID_PORT);

	printf("nFdUart = %d\n", nFdUart);
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadCSN
**	AUTHOR:		Jeff Wang
**	DATE:			10 - April - 2009
**
**	DESCRIPTION:	
**			Read Card Number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pOutBuf		[IN]		CHAR*		Buffer
**	RETURNED VALUE:	
**				read length
**	NOTES:
**			
*/
INT
ReadCSN(CHAR* pOutBuf, BYTE* pCardMode)
{
	BYTE	TmpUartData[MAX_CDDATA_READ_LEN]		= { 0 };	
	INT		nTmpUartLen								= 0;

	static BYTE	UartRcvData[MAX_UART_DATA_LEN]		= { 0 };
	static INT  nUartRcvDataLen							= 0;	
	
	BYTE	PacketData[MAX_PACKET_DATA_LEN]			= { 0 };	
	INT		nPacketLen								= 0;

	INT				i			 = 0;
	INT				nReadCardLen = 0;
	INT				nRetCardLen	 = 0;
	BYTE*			pApp		 = NULL;
	INT				nAppLen		 = 0;
	SerialPacket	SerialPck;
	AppPacket		AppPck;
	BYTE*			pInData		 = NULL;
	INT				nInDataLen   = 0;
	INT				nTempLen     = 0;
	INT				nIndex		 = 0;
	
	nTmpUartLen = read(nFdUart, TmpUartData, MAX_UART_DATA_LEN - nUartRcvDataLen);
	dwDiag[DIAG_SRLREAD_COUNT]++;

	if (nTmpUartLen > 0)
	{	
		memcpy(UartRcvData + nUartRcvDataLen, TmpUartData, nTmpUartLen);
		nUartRcvDataLen += nTmpUartLen;

		TelSrlData(TmpUartData, nTmpUartLen);
	}
	memset(TmpUartData, 0, MAX_CDDATA_READ_LEN);
	nTmpUartLen = 0;

	if (IsCompletePacket(UartRcvData, &nUartRcvDataLen, &nPacketLen) == TRUE)
	{

#ifdef MOX_CARDREADER
		memcpy(PacketData, UartRcvData, nPacketLen);		

		if (nPacketLen > 0)
		{
		 	printf("Serial Read len %d: ", nPacketLen);
		 	for(i = 0; i < nPacketLen; i++)
		 	{
		 		printf("%02x ", PacketData[i]);
		 	}
		 	printf("\n");
		}

		if (nUartRcvDataLen >= nPacketLen)
		{
			memcpy(UartRcvData, UartRcvData + nPacketLen, nUartRcvDataLen - nPacketLen);
			nUartRcvDataLen = nUartRcvDataLen - nPacketLen;
		}

		nAppLen = UnpckSerialEx(PacketData, nPacketLen, &SerialPck, &pApp);
		if (nAppLen > 4)
		{
			nInDataLen = UnpckAppEx(pApp, nAppLen, &AppPck, &pInData);
		}			
		if (nInDataLen > 2)
		{	
			*pCardMode = pInData[1];

			nTempLen = CSN_LEN - pInData[2];
			memset(pOutBuf, 0x00, nTempLen);
			memcpy(pOutBuf + nTempLen, &pInData[3], pInData[2]);
			nReadCardLen = CSN_LEN;
			
			TelInputCSN(pOutBuf, nReadCardLen);
			printf("Card len %d: ",nReadCardLen);
#ifdef __SUPPORT_WIEGAND_CARD_READER__	
			printf("WiegandBitNum=%d,WiegandReverse=%d\n",g_ASPara.WiegandBitNum,g_ASPara.WiegandReverse);
			if(g_ASPara.WiegandBitNum > 0 && g_ASPara.WiegandBitNum <= 40)
			{
				if(pInData[2] != g_ASPara.WiegandBitNum/8)
					printf("Should be set Wiegand %d!\n",pInData[2]*8+2);
				if(g_ASPara.WiegandReverse)
				{
					char tmpChar = 0;
					int Start=0, End=0,j = CSN_LEN-1;
					Start = CSN_LEN - g_ASPara.WiegandBitNum/8;
					End = g_ASPara.WiegandBitNum/8;
					for(i = Start; i < End; i++,j--)
					{
						if(i >= j)
							break;
						tmpChar = pOutBuf[i];
						pOutBuf[i] = pOutBuf[j];
						pOutBuf[j] = tmpChar;
					}
				}
			}
#endif
			for(i = 0; i < nReadCardLen; i++)
			{
				printf("%02x ", pOutBuf[i]);
			}
			printf("\n");
		}
		
		memset(PacketData, 0, MAX_PACKET_DATA_LEN);
		nPacketLen = 0;
		
		nRetCardLen = nReadCardLen;
#endif

	}

	return nRetCardLen;
}

static BOOL
IsCompletePacket(BYTE* pBuffer, INT* pnTotalLen, INT* pnLen)
{
	INT		nIndex;
	INT		nValidHeadPos = 0;
	BOOL	bComplete = FALSE;
	DWORD	dwSerlSta, dwSerialLen, dwFunCode, nAppLen, checksum, dwSerlEnd;

	*pnLen   = 0;

	if (*pnTotalLen < MIN_SERIAL_PCKLEN)
	{
		return FALSE;
	}

	for (nIndex = 0; nIndex < *pnTotalLen - MIN_SERIAL_PCKLEN; nIndex++)
	{
		dwSerlSta = MAKEWORD(pBuffer[nIndex + OFFSET_SERIAL_STX + 1], pBuffer[nIndex + OFFSET_SERIAL_STX]);
		if (dwSerlSta == SERIAL_START)
		{
			// Save first valid head position
			if (nValidHeadPos == 0)
			{
				nValidHeadPos = nIndex;
			}

			dwSerialLen = MAKEWORD(pBuffer[nIndex + OFFSET_SERIAL_LEN + 1], pBuffer[nIndex + OFFSET_SERIAL_LEN]);
			if (nIndex + dwSerialLen <= *pnTotalLen)
			{
				checksum = MAKEWORD(pBuffer[nIndex + dwSerialLen - 3], pBuffer[nIndex + dwSerialLen - 4]);
				if (checksum == CheckSum(&pBuffer[nIndex + OFFSET_SERIAL_VER], dwSerialLen - 6))
				{
					dwSerlEnd = MAKEWORD(pBuffer[nIndex + dwSerialLen - 1], pBuffer[nIndex + dwSerialLen - 2]);
					if (dwSerlEnd == SERIAL_END)
					{
						bComplete = TRUE;
						*pnLen   = dwSerialLen;
						break;
					}					
				}	
			}
		}
	}

	if (bComplete == TRUE)
	{
		if (nIndex != 0)
		{
			memcpy(pBuffer, pBuffer + nIndex, *pnTotalLen - nIndex);
			*pnTotalLen -= nIndex;
		}		
	}	
	else
	{
		if (nValidHeadPos != 0)
		{
			memcpy(pBuffer, pBuffer + nValidHeadPos, *pnTotalLen - nValidHeadPos);
			*pnTotalLen -= nValidHeadPos;
		}
	}

	return bComplete;
}

static DWORD CheckSum(BYTE* pBuf, INT nLen)
{
	INT i = 0;
	DWORD sum = 0;

	for (i = 0; i < nLen; i++)
	{
		sum += pBuf[i];
	}

	return sum;
}

static INT
UnpckSerialEx(BYTE* pFrm, INT nFrmLen, SerialPacket* pSerPck, BYTE** ppApp)
{
	INT nAppLen = 0;

	// STX
	pSerPck->Stx = MAKEWORD(*(pFrm + OFFSET_SERIAL_STX + 1), *(pFrm + OFFSET_SERIAL_STX + 0));
	// VER
	pSerPck->Ver = pFrm[OFFSET_SERIAL_VER];
	// LEN
	pSerPck->Len = MAKEWORD(*(pFrm + OFFSET_SERIAL_LEN + 1), *(pFrm + OFFSET_SERIAL_LEN + 0));
	// SEQ
	pSerPck->Seq = pFrm[OFFSET_SERIAL_SEQ];
	// OPT
	pSerPck->Opt = pFrm[OFFSET_SERIAL_OPT];
	// DA1
	pSerPck->Da1 = pFrm[OFFSET_SERIAL_DA1];
	// DA2
	pSerPck->Da2 = pFrm[OFFSET_SERIAL_DA2];
	// DA3
	pSerPck->Da3 = pFrm[OFFSET_SERIAL_DA3];
	// SA1
	pSerPck->Sa1 = pFrm[OFFSET_SERIAL_SA1];
	// SA2
	pSerPck->Sa2 = pFrm[OFFSET_SERIAL_SA2];
	// SA3
	pSerPck->Sa3 = pFrm[OFFSET_SERIAL_SA3];
	// APP
	if(pSerPck->Len > MIN_SERIAL_PCKLEN)
	{
		nAppLen = pSerPck->Len - MIN_SERIAL_PCKLEN;
		*ppApp = pFrm + OFFSET_SERIAL_APP;
	}
	// CHK
	pSerPck->Chk = MAKEWORD(*(pFrm + pSerPck->Len - LEN_SERIAL_CHK - LEN_SERIAL_ETX + 1), *(pFrm + pSerPck->Len - LEN_SERIAL_CHK - LEN_SERIAL_ETX + 0));
	//ETX
	pSerPck->Etx = MAKEWORD(*(pFrm + pSerPck->Len - LEN_SERIAL_ETX + 1), *(pFrm + pSerPck->Len - LEN_SERIAL_ETX + 0));

	return nAppLen;
}

static INT
UnpckAppEx(BYTE* pApp, INT nAppLen, AppPacket* pAppPck, BYTE** ppData)
{
	INT nDataLen = 0;

	// Function Category
	pAppPck->FuncCat = pApp[OFFSET_APP_FUNCCAT];
	// Function Code
	pAppPck->FuncCod = pApp[OFFSET_APP_FUNCCOD];
	// Length
	pAppPck->Length = MAKEWORD(*(pApp + OFFSET_APP_LENGTH + 1), *(pApp + OFFSET_APP_LENGTH + 0));
	// DATA
	if(pAppPck->Length > MIN_APP_LENGTH)
	{
		nDataLen = pAppPck->Length - MIN_APP_LENGTH;
		*ppData = pApp + OFFSET_APP_DATA;
	}

	return nDataLen;
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	serial_port_open
**	AUTHOR:			Alan Huang / Jerry Huang
**	DATE:			23 - Jan - 2007
**
**	DESCRIPTION:	
**			Open serial port
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nPort		[IN]		int		port number
**	RETURNED VALUE:	
**				port handle if success, otherwise -1
**	NOTES:
**			
*/
static INT 
serial_port_open(INT nPort)
{
	char			tcPort[16];
	//struct termios	oldtio;
	struct termios	newtio;	
	int				nFd = -1;
	
	sprintf(tcPort, "/dev/ttymxc%d", nPort);
	
	nFd = open(tcPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (nFd < 0) 
	{
		printf("Uart Open Error\n");
		return -1;
	}
	
	/* save current port settings */
	tcgetattr(nFd , &newtio); 

	//HID serial port set
#ifdef HID_CARDREADER
	newtio.c_iflag				 |= INPCK;
	newtio.c_oflag				&= ~OPOST;
	newtio.c_lflag				 &= ~(ICANON | ECHO | ECHOE | ISIG);
	newtio.c_cc[VTIME]	 = 0;
	newtio.c_cc[VMIN]	  = 1;
	newtio.c_cflag		= B57600 
		|PARENB 
		&~PARODD
		&~CSTOPB
		&~CSIZE
		|CS8
		|CLOCAL
		|CREAD;
#endif

#ifdef 	MOX_CARDREADER    
    newtio.c_iflag		|= INPCK;
    newtio.c_oflag      &= ~OPOST;
    newtio.c_lflag		&= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_cc[VTIME]	= 0;
	newtio.c_cc[VMIN]	= 1;
	newtio.c_cflag		= B9600 
		&~PARENB 
		&~CSTOPB
		&~CSIZE
		|CS8
		|CLOCAL
		|CREAD;    
    newtio.c_iflag      &= ~(IXON | IXOFF | IXANY);/*disable input flow controling*/
#endif
	newtio.c_iflag &=~(INLCR|IGNCR|ICRNL);
    newtio.c_oflag &=~(ONLCR|OCRNL);

    tcflush(nFd  , TCIFLUSH);
    tcsetattr(nFd, TCSANOW, &newtio);
	
	return nFd;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CSNNotifyAck
**	AUTHOR:			Alan Huang / Jerry Huang
**	DATE:			21 - Aor - 2009
**
**	DESCRIPTION:	
**			Send CSN NOTIFY ACK to Card Read
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pRcvData		[IN]		int		Receive data
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID
CSNNotifyAck(CHAR* pRcvData)
{
	CHAR	WrDataBuf[20] = { 0 };
	size_t      WrDataLen       = 7;
	size_t		nWritten			  = 0;
	
	WrDataBuf[0] = 0x02;
	WrDataBuf[1] = pRcvData[1];
	WrDataBuf[2] = 0x01;
	WrDataBuf[3] = 0x01;
	WrDataBuf[4] = 0x00;
	WrDataBuf[5] = WrDataBuf[1] + WrDataBuf[2] + WrDataBuf[3] + WrDataBuf[4];
	WrDataBuf[6] = 0x03;

	nWritten = write(nFdUart, WrDataBuf, WrDataLen);

	if (-1 == nWritten)
	{
		printf("Serial Ack Send Error\n");
	}
	else
	{
		printf("Serial Ack Send Success\n");
	}
}
