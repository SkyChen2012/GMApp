/// Copyright MOX Group Products , Australia
/// FILE DESCRIPTION:
/// read card CSN in car
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

#include "TelnetLog.h"
#include "PioApi.h"
#include "DiagTelnetd.h"
#include "InCarCardProc.h"
#include "RS485.h"

/************** DEFINES **************************************************************/

#define DEBUG_INCARCARD

/************** TYPEDEFS *************************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static INT nFdCarUart = 0;
static BYTE	UartRcvData[MAX_UART_DATA_LEN] = { 0 };
static INT  nUartRcvDataLen = 0;
static BYTE	UartSndData[MAX_UART_DATA_LEN] = { 0 };
static INT  nUartSndDataLen = 0;
static DWORD OverTimeTicket = 0;
#define READINCARCSN_TIMEOUT 500  //500ms 针对485异常数据
static INT serial_port_open(INT nPort);

static INT ReadInCarCSN(BYTE* pOutBuf,unsigned char * pStationNum);
static void SendInCarCardUnlock(BYTE* pCSN,unsigned char cStationNum);

static BOOL IsCompletePacket(BYTE* pBuffer, INT* pnTotalLen, INT* pnLen);
static INT UnpckSerialEx(BYTE* pFrm, INT nFrmLen, SerialPacket* pSerPck, BYTE** ppApp);
static INT UnpckAppEx(BYTE* pApp, INT nAppLen, AppPacket* pAppPck, BYTE** ppData);
static INT PckAppEx(AppPacket* pAppPck, BYTE* pData, INT nDataLen, BYTE* AppPktBuf);
static INT PckSerialEx(SerialPacket* pSerPck, BYTE* pApp, INT nAppLen, BYTE* SerialPktBuf);
static unsigned short CalCheckSum(BYTE* pBuf, INT nLen);

/*************************************************************************************/

/// <summary>
/// 
/// </summary>
/// <param>void</param>
/// <returns>void</returns>
VOID
InCarCardInit(VOID)
{
	BYTE	TmpUartData[1024]		= { 0 };	
	INT		nTmpUartLen				= 0;

	if(access(SPEFILE, F_OK) == 0)
		nFdCarUart = serial_port_open(RS485_PORT_2);
	else
		nFdCarUart = serial_port_open(RS485_PORT_1);
	
	TmpUartData[0] = 1;
	TmpUartData[1] = 2;
	TmpUartData[2] = 3;

	nTmpUartLen = 3;

	write(nFdCarUart, TmpUartData, nTmpUartLen);
#ifdef DEBUG_INCARCARD
	printf("nFdCarUart = %d\n", nFdCarUart);
#endif // DEBUG_INCARCARD
}

/// <summary>
/// 
/// </summary>
/// <param>void</param>
/// <returns>void</returns>
VOID
InCarCardProc(VOID)
{
	BYTE InCarCSN[CSN_LEN] = { 0 };
	int nCSNLen = 0;
	int nFindPos = 0;
	BOOL bFindCSN = FALSE;
	unsigned char cStationNum;
	nCSNLen = ReadInCarCSN(InCarCSN,&cStationNum);

	if (CSN_LEN != nCSNLen)
	{
		return;
	}	

	SendInCarCardUnlock(InCarCSN,cStationNum);
}

/// <summary>
/// 
/// </summary>
/// <param>void</param>
/// <returns>void</returns>
VOID
InCarCardExit(VOID)
{
}

/// <summary>
/// 
/// </summary>
/// <param name="cardInfo">[IN][CardInfo]</param>
/// <returns>void</returns>
static
void
SendInCarCardUnlock(BYTE* pCSN,unsigned char cStationNum)
{
	MXMSG msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LC;
	msgSend.dwDestMd	= MXMDID_LC;
	msgSend.dwMsg		= FC_LF_UNLOCK;
	
	msgSend.pParam = malloc(1 + CSN_LEN);
	//msgSend.dwParam=cStationNum;
	if (NULL == msgSend.pParam)
	{
		printf(" %s: ERROR, malloc memory failed.\n", __FUNCTION__);
		return;
	}
	
	*(msgSend.pParam) = CSN_LEN+2;
	memcpy(msgSend.pParam + 1, pCSN, CSN_LEN);
	memcpy(msgSend.pParam + 1+CSN_LEN, &cStationNum, 1);
	
	MxPutMsg(&msgSend);
}

/// <summary>
/// 
/// </summary>
/// <param name="byRet">[IN][BYTE]</param>
/// <returns>VOID</returns>
VOID
SendCardUnlockACK2CR(BYTE byRet)
{
	unsigned short CheckSum = 0;
	int i = 0;

	if (0x00 == byRet)
	{
		UartSndData[OFFSET_SERIAL_APP + OFFSET_APP_DATA + 1] = RESULT_OK;
		UartSndData[OFFSET_SERIAL_APP + OFFSET_APP_DATA + 2] = LED_IND_OK;
		UartSndData[OFFSET_SERIAL_APP + OFFSET_APP_DATA + 3] = BEEP_IND_OK;
		CheckSum = CalCheckSum(&UartSndData[OFFSET_SERIAL_VER], nUartSndDataLen - 6/*STX CHK ETX*/);
		UartSndData[nUartSndDataLen - 4/*CHK ETX*/] = HIBYTE(CheckSum);
		UartSndData[nUartSndDataLen - 4/*CHK ETX*/ + 1] = LOBYTE(CheckSum);

		write(nFdCarUart, UartSndData, nUartSndDataLen);
	}
	else
	{
		UartSndData[OFFSET_SERIAL_APP + OFFSET_APP_DATA + 1] = RESULT_FAIL;
		UartSndData[OFFSET_SERIAL_APP + OFFSET_APP_DATA + 2] = LED_IND_ERR;
		UartSndData[OFFSET_SERIAL_APP + OFFSET_APP_DATA + 3] = BEEP_IND_ERR;
		CheckSum = CalCheckSum(&UartSndData[OFFSET_SERIAL_VER], nUartSndDataLen - 6/*STX CHK ETX*/);
		UartSndData[nUartSndDataLen - 4/*CHK ETX*/] = HIBYTE(CheckSum);
		UartSndData[nUartSndDataLen - 4/*CHK ETX*/ + 1] = LOBYTE(CheckSum);
		
		write(nFdCarUart, UartSndData, nUartSndDataLen);
	}
#ifdef DEBUG_INCARCARD
	printf("Send read card CSN ack to uart data len %d: ", nUartSndDataLen);
	for(i = 0; i < nUartSndDataLen; i++)
	{
		printf("%02x ", UartSndData[i]);
	}
	printf("\n");
#endif // DEBUG_INCARCARD
}

/// <summary>
/// 
/// </summary>
/// <param name="pOutBuf">[IN][BYTE*]</param>
/// <returns>int</returns>
static
INT
ReadInCarCSN(BYTE* pOutBuf,unsigned char * pStationNum)
{
	int		nReadUartLen = 0;		
	
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
	
	nReadUartLen = read(nFdCarUart, UartRcvData + nUartRcvDataLen, MAX_UART_DATA_LEN - nUartRcvDataLen);

	if (nReadUartLen > 0)
	{	
		if(nUartRcvDataLen + nReadUartLen >= MAX_UART_DATA_LEN)
		{
			nUartRcvDataLen = 0;
			OverTimeTicket = 0;
			printf("===================\n===================\n===================\n");
			printf("Uart data error!\n");
			return 0;
		}
		if(0 == OverTimeTicket)
		{
			OverTimeTicket = GetTickCount();
		}
#ifdef DEBUG_INCARCARD	
		printf("%s: Serial Read len %d: ", __FUNCTION__, nReadUartLen);
		for (i = nUartRcvDataLen; i < nUartRcvDataLen + nReadUartLen; i++)
		{
			printf("%02x ", UartRcvData[i]);
		}
		printf("\n");
#endif // DEBUG_INCARCARD
		nUartRcvDataLen += nReadUartLen;
	}

	if (IsCompletePacket(UartRcvData, &nUartRcvDataLen, &nPacketLen))
	{
		OverTimeTicket = 0;
		memcpy(PacketData, UartRcvData, nPacketLen);		
#ifdef DEBUG_INCARCARD
		if (nPacketLen > 0)
		{
		 	printf("Serial Read len %d: ", nPacketLen);
		 	for(i = 0; i < nPacketLen; i++)
		 	{
		 		printf("%02x ", PacketData[i]);
		 	}
		 	printf("\n");
		}
#endif // DEBUG_INCARCARD
		if (nUartRcvDataLen >= nPacketLen)
		{
			memcpy(UartRcvData, UartRcvData + nPacketLen, nUartRcvDataLen - nPacketLen);
			nUartRcvDataLen = nUartRcvDataLen - nPacketLen;
		}

		nAppLen = UnpckSerialEx(PacketData, nPacketLen, &SerialPck, &pApp);
	
		if (nAppLen >= MIN_APP_LENGTH)
		{
			nInDataLen = UnpckAppEx(pApp, nAppLen, &AppPck, &pInData);

			if (	nInDataLen >= MIN_DATA_LENGTH
					&& AppPck.FuncCat == HIBYTE(FC_AC_REPORT_CSN) 
					&& AppPck.FuncCod == LOBYTE(FC_AC_REPORT_CSN))
			{	
				nTempLen = CSN_LEN - pInData[2];
				memset(pOutBuf, 0, nTempLen);
				*pStationNum=pInData[0];
				memcpy(pOutBuf + nTempLen, &pInData[3], pInData[2]);
				nReadCardLen = CSN_LEN;
#ifdef DEBUG_INCARCARD			
				printf("Card len %d: ", nReadCardLen);
				for (i = 0; i < nReadCardLen; i++)
				{
					printf("%02x ", pOutBuf[i]);
				}
				printf("\n");
#endif // DEBUG_INCARCARD

				memset(UartSndData, 0, MAX_UART_DATA_LEN);
				nUartSndDataLen = 0;

				pInData[1] = 0;
				pInData[2] = 0;
				pInData[3] = 0;
				nInDataLen = 4;

				nAppLen = PckAppEx(&AppPck, pInData, nInDataLen, pApp);
				nUartSndDataLen = PckSerialEx(&SerialPck, pApp, nAppLen, UartSndData);
			}
		}					
		
		nRetCardLen = nReadCardLen;
	}
	if(OverTimeTicket)
	{
		DWORD tmp1 = GetTickCount(),tmp=0;
		tmp = tmp1- OverTimeTicket;
		
		if( tmp > READINCARCSN_TIMEOUT)
		{
			nUartRcvDataLen = 0;
			OverTimeTicket = 0;
			memset(UartRcvData,0,MAX_UART_DATA_LEN);
			printf("===================\n===================\n===================\n");
			printf("Read in car CSN overtime!%ld\n",tmp);
			return 0;
		}	
	}


	return nRetCardLen;
}

/// <summary>
/// 
/// </summary>
/// <param name="pBuffer">[IN][BYTE*]</param>
/// <param name="pnTotalLen">[IN][INT*]</param>
/// <param name="pnLen">[IN][INT*]</param>
/// <returns>bool</returns>
static
BOOL
IsCompletePacket(BYTE* pBuffer, INT* pnTotalLen, INT* pnLen)
{
	INT		nIndex;
	INT		nValidHeadPos = 0;
	BOOL	bComplete = FALSE;
	DWORD	dwSerlSta, dwSerialLen, dwFunCode, nAppLen, dwSerlEnd;
	unsigned short checksum = 0;

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
			if (nValidHeadPos == 0)
			{
				nValidHeadPos = nIndex;
			}

			dwSerialLen = MAKEWORD(pBuffer[nIndex + OFFSET_SERIAL_LEN + 1], pBuffer[nIndex + OFFSET_SERIAL_LEN]);
			if (nIndex + dwSerialLen <= *pnTotalLen)
			{
				checksum = MAKEWORD(pBuffer[nIndex + dwSerialLen - 3], pBuffer[nIndex + dwSerialLen - 4]);
				if (checksum == CalCheckSum(&pBuffer[nIndex + OFFSET_SERIAL_VER], dwSerialLen - 6))
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

/// <summary>
/// 
/// </summary>
/// <param name="pBuf">[IN][BYTE*]</param>
/// <param name="nLen">[IN][INT]</param>
/// <returns>DWORD</returns>
static
unsigned short
CalCheckSum(BYTE* pBuf, INT nLen)
{
	INT i = 0;
	DWORD sum = 0;

	for (i = 0; i < nLen; i++)
	{
		sum += pBuf[i];
	}

	return sum;
}

/// <summary>
/// 
/// </summary>
/// <param name="pFrm">[IN][BYTE*]</param>
/// <param name="nFrmLen">[IN][INT]</param>
/// <param name="pSerPck">[IN][SerialPacket*]</param>
/// <param name="ppApp">[IN][BYTE**]</param>
/// <returns>INT</returns>
static
INT
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

/// <summary>
/// 
/// </summary>
/// <param name="pApp">[IN][BYTE*]</param>
/// <param name="nAppLen">[IN][INT]</param>
/// <param name="pAppPck">[IN][AppPacket*]</param>
/// <param name="ppData">[IN][BYTE**]</param>
/// <returns>INT</returns>
static
INT
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

/// <summary>
/// 
/// </summary>
/// <param name="pAppPck">[IN][AppPacket*]</param>
/// <param name="pData">[IN][BYTE*]</param>
/// <param name="nDataLen">[IN][INT]</param>
/// <returns>INT</returns>
static
INT
PckAppEx(AppPacket* pAppPck, BYTE* pData, INT nDataLen, BYTE* AppPktBuf)
{
	// Function Category
	AppPktBuf[OFFSET_APP_FUNCCAT] = HIBYTE(FC_ACK_AC_REPORT_CSN);
	// Function Code
	AppPktBuf[OFFSET_APP_FUNCCOD] = LOBYTE(FC_ACK_AC_REPORT_CSN);
	// Length
	AppPktBuf[OFFSET_APP_LENGTH + 0] = HIBYTE(MIN_APP_LENGTH + nDataLen);
	AppPktBuf[OFFSET_APP_LENGTH + 1] = LOBYTE(MIN_APP_LENGTH + nDataLen);
	// DATA
	memcpy(AppPktBuf + OFFSET_APP_DATA, pData, nDataLen);
	
	return MIN_APP_LENGTH + nDataLen;
}

/// <summary>
/// 
/// </summary>
/// <param name="pSerPck">[IN][SerialPacket*]</param>
/// <param name="pApp">[IN][BYTE*]</param>
/// <param name="nAppLen">[IN][INT]</param>
/// <returns>INT</returns>
static
INT
PckSerialEx(SerialPacket* pSerPck, BYTE* pApp, INT nAppLen, BYTE* SerialPktBuf)
{
	// STX
	SerialPktBuf[OFFSET_SERIAL_STX + 0] = HIBYTE(pSerPck->Stx);
	SerialPktBuf[OFFSET_SERIAL_STX + 1] = LOBYTE(pSerPck->Stx);
	// VER
	SerialPktBuf[OFFSET_SERIAL_VER] = pSerPck->Ver;
	// LEN
	SerialPktBuf[OFFSET_SERIAL_LEN + 0] = HIBYTE(MIN_SERIAL_PCKLEN + nAppLen);
	SerialPktBuf[OFFSET_SERIAL_LEN + 1] = LOBYTE(MIN_SERIAL_PCKLEN + nAppLen);
	// SEQ
	SerialPktBuf[OFFSET_SERIAL_SEQ] = pSerPck->Seq;
	// OPT
	SerialPktBuf[OFFSET_SERIAL_OPT] = 0;
	// DA1
	SerialPktBuf[OFFSET_SERIAL_DA1] = pSerPck->Sa1;
	// DA2
	SerialPktBuf[OFFSET_SERIAL_DA2] = pSerPck->Sa2;
	// DA3
	SerialPktBuf[OFFSET_SERIAL_DA3] = pSerPck->Sa3;
	// SA1
	SerialPktBuf[OFFSET_SERIAL_SA1] = pSerPck->Da1;
	// SA2
	SerialPktBuf[OFFSET_SERIAL_SA2] = pSerPck->Da2;
	// SA3
	SerialPktBuf[OFFSET_SERIAL_SA3] = pSerPck->Da3;
	// APP
	memcpy(SerialPktBuf + OFFSET_SERIAL_APP, pApp, nAppLen);

	//ETX
	SerialPktBuf[MIN_SERIAL_PCKLEN + nAppLen - 2] = HIBYTE(pSerPck->Etx);
	SerialPktBuf[MIN_SERIAL_PCKLEN + nAppLen - 1] = LOBYTE(pSerPck->Etx);
	
	return MIN_SERIAL_PCKLEN + nAppLen;	
}

/// <summary>
/// 
/// </summary>
/// <param name="nPort">[IN][INT]</param>
/// <returns>INT</returns>
static
INT 
serial_port_open(INT nPort)
{
	char			tcPort[16];
	struct termios	oldtio;
	struct termios	newtio;	
	int				nFd = -1;
	
	sprintf(tcPort, "/dev/ttymxc%d", nPort);
	
	
	nFd = open(tcPort, O_RDWR |O_NOCTTY | O_NONBLOCK);
	
	
	if (nFd < 0) 
	{
		printf("Uart Open Error\n");
		return -1;
	}
	
	tcgetattr(nFd,&newtio); 
	
	cfsetispeed(&newtio, B9600);
	cfsetospeed(&newtio, B9600);
	
	newtio.c_cflag &= ~CSIZE; /* Mask the character size bits */
	newtio.c_cflag |= CS8;    /* Select 8 data bits */
	
	newtio.c_cflag &= ~PARENB;//mode:8N1
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_cflag &= ~CSIZE;
	
	newtio.c_cflag |= CS8;
	
	newtio.c_cflag |= (CLOCAL | CREAD);
	
	newtio.c_cflag &= ~CRTSCTS; /*disable hardware flow control*/
	newtio.c_iflag &= ~(IXON | IXOFF | IXANY); /*disable software flow control */
	
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /*Input*/
	newtio.c_oflag &= ~OPOST; /*Output*/
	
	newtio.c_iflag &=~(INLCR|IGNCR|ICRNL);
    newtio.c_oflag &=~(ONLCR|OCRNL);

	
	newtio.c_cc[VTIME] = 0;//timeout = 2seconds
	newtio.c_cc[VMIN] = 0;
	tcflush(nFd, TCIFLUSH);
	//tcflush(nFd, TCIOFLUSH);
	
	tcsetattr(nFd, TCSANOW, &newtio);	
	return nFd;
}
