/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	LCAPro_mitsubishi.c
**
**	AUTHOR:		Wayde Zeng
**
**	DATE:		9 - March - 2011
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				lift control for MOX protocol
**					
**				
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

/************** USER INCLUDE FILES ***************************************************/

#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"
#include "ModuleTalk.h"
#include "BacpNetCtrl.h"
#include "Multimedia.h"
#include "TalkEventWnd.h"
#include "TalkLogReport.h"
#include "MenuParaProc.h"
#include "AMT.h"
#include "PioApi.h"
#include "IOControl.h"
#include "RS485.h"
#include "BacpSerial.h"
#include "BacpSerialApp.h"
#include "EGMLiftControl.h"
#include "EGMLCAgent.h"
#include "LCAPro_mox_V1.h"
#include "LCAPro_mitsubishi.h"
#include "IniFile.h"
/************** DEFINES **************************************************************/

#define MIT_DEBUG

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static LTCONFIG	g_LTConfig;
static VAFLOOR	g_VAFloor;
static BANKCHECKSTATE g_BnkChkState;

static BYTE		g_byIndex = 0;
static DWORD	g_dwHeartBeatTick;
static DWORD	g_dwStateTick;
static DWORD	g_dwStateCount;

static VOID		LoadLTConfig(VOID);
static VOID		LoadVAFloor(VOID);
static VOID		BankCheckStateInit(VOID);
static BOOL		MitPreprocessMsg(MXMSG* msgRecv);
static VOID		LCAMitProcessMsg(MXMSG * pmsg);
static VOID		LCAMitIdlePro(MXMSG * pmsg);
static int		IsCpltMitCommfrm(unsigned char* pBuf, int* pnLen, int* pDataLen);
static VOID		SendErrorACK2Target(DWORD dwMsg, DWORD dwIP, unsigned char ucRetCode);
static BOOL		ConvertVAFloor(CHAR AFloor, CHAR* pVFloor);

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMitProtoInit()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**				initialize the MOX protocol process
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
VOID
LCAMitProtoInit(VOID)
{
	if (LC_MITSUBISHI_MODE == g_SysConfig.LC_ComMode)
	{
		DpcAddMd(MXMDID_LT_MITSUBISHI, NULL);

		LoadLTConfig();
		LoadVAFloor();
		BankCheckStateInit();

		g_dwHeartBeatTick = GetTickCount();
		g_dwStateTick = GetTickCount();
		g_dwStateCount = 0;
	}
}
BOOL IsMitSCPLFlag(void)
{
	return g_LTConfig.bSCPLFlag;
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadLTConfig()
**	AUTHOR:			Wayde Zeng
**	DATE:			13 - Dec - 2010
**
**	DESCRIPTION:	
**				load system configuration
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
LoadLTConfig(VOID)
{
	//const char* pChar = NULL;
	int nCount = 0;
	int i = 0;
	char szSec[20] = { 0 };
	
	g_LTConfig.banksize = 0;
	memset(g_LTConfig.bank, 0, BANK_MAXSIZE);

	OpenIniFile(LFCONFIG_FILE);
	g_LTConfig.bSCPLFlag=ReadBool(LFCONFIG_SECTION, LFCONFIG_KEY_SWPL_FLAG, FALSE);
	printf("g_LTConfig.bSCPLFlag=%d\n",g_LTConfig.bSCPLFlag);
	nCount = ReadInt(LFCONFIG_SECTION, LFCONFIG_KEY_BANK_COUNT, 1);
	
	if (nCount > 0 && nCount <= BANK_MAXSIZE)
	{
		g_LTConfig.banksize = nCount;
		
		for (i = 0; i < nCount; i++)
		{
			snprintf(szSec, sizeof (szSec), "BANK%d", i + 1);
			g_LTConfig.bank[i] = ReadInt(szSec, LFCONFIG_KEY_BANK, 0);
		}
	}
	else
	{
		printf("ERROR: can not find CFT.ini, count=%d\n", g_LTConfig.banksize);
	}
	nCount = ReadInt(LFCONFIG_SECTION, LFCONFIG_KEY_CARDREADER_COUNT, 0);
	if (nCount > 0 && nCount <= CARDREADER_MAXSIZE)
	{
		g_LTConfig.CardReaderSize = nCount;
		
		for (i = 0; i < nCount; i++)
		{
			snprintf(szSec, sizeof (szSec), "CARDREADER%d", i + 1);
			g_LTConfig.CardReader[i].bBank = ReadInt(szSec, LFCONFIG_KEY_BANK, 0);
			g_LTConfig.CardReader[i].bNode = ReadInt(szSec, LFCONFIG_KEY_NODE, 0);
			g_LTConfig.CardReader[i].bStationNum = ReadInt(szSec, LFCONFIG_KEY_STATIONNUM, 0);
		}
	}
	else
	{
		printf("ERROR: can not find CFT.ini, count=%d\n", g_LTConfig.banksize);
	}
	
	CloseIniFile();
}

LTCARDREADER * GetLiftInfoByStationNum(BYTE bStationNum)
{
	int i;
	for (i = 0; i < g_LTConfig.CardReaderSize; i++)
	{
		if(bStationNum==g_LTConfig.CardReader[i].bStationNum)
		{
			return &g_LTConfig.CardReader[i];
		}
	}
	return NULL;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadVAFloor()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
LoadVAFloor(VOID)
{
	FILE* fd = NULL;
	struct stat pStat;
	//int i = 0;
	//int j = 0;
	//char szSec[20] = { 0 };
	
	g_VAFloor.versionnum = 0;
	g_VAFloor.nCount = 0;
	g_VAFloor.nPCVALen = 0;
	g_VAFloor.pPCVA = (PCVA*)malloc(MAX_FLOOR_NUM * sizeof(PCVA));
	memset(g_VAFloor.pPCVA, 0, MAX_FLOOR_NUM * sizeof(PCVA));
	
	if ((fd = fopen(VAT_FILE, "r+")) != NULL)
	{
		stat(VAT_FILE, &pStat);
		fseek(fd, 0, SEEK_SET);
		fread(&g_VAFloor.versionnum, sizeof(int), (size_t)1, fd);
		fseek(fd, 0, SEEK_CUR);
		fread(&g_VAFloor.nCount, sizeof(int), (size_t)1, fd);
		fseek(fd, 0, SEEK_CUR);
		fread(&g_VAFloor.nPCVALen, sizeof(int), (size_t)1, fd);
		fseek(fd, 0, SEEK_CUR);
		fread(g_VAFloor.pPCVA, pStat.st_size - sizeof(int) - sizeof(int) - sizeof(int), (size_t)1, fd);
		
		fclose(fd);
	}
	else
	{
		printf("VAT file open error\n");
		return;
	}
	
	if (g_VAFloor.nCount > MAX_FLOOR_NUM)
	{
		printf("Lift floor count error\n");
		return;		
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BankCheckStateInit()
**	AUTHOR:			Wayde Zeng
**	DATE:			13 - Dec - 2010
**
**	DESCRIPTION:	
**				initialize check state of bank
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
BankCheckStateInit(VOID)
{
	int i = 0;

	for (i = 0; i < BANK_MAXSIZE; i++)
	{
		g_BnkChkState.bankstate[i] = 0;
		g_BnkChkState.FaultTick[i] = GetTickCount();
	}

	g_BnkChkState.checkpos = 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMitProtoExit()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
VOID
LCAMitProtoExit(VOID)
{
	if (LC_MITSUBISHI_MODE == g_SysConfig.LC_ComMode) 
	{
		DpcRmMd(MXMDID_LT_MITSUBISHI);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMitThreadFun()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
VOID
LCAMitThreadFun(VOID)
{
	MXMSG msgRecv;

	memset(&msgRecv, 0, sizeof(MXMSG));
	msgRecv.dwDestMd	= MXMDID_LT_MITSUBISHI;
	msgRecv.pParam		= NULL;

	if (MxGetMsg(&msgRecv)) 
	{
		if (MitPreprocessMsg(&msgRecv))
		{
			LCAMitProcessMsg(&msgRecv);
		}
		
		DoClearResource(&msgRecv);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMitProcessMsg()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
LCAMitProcessMsg(MXMSG * pmsg)
{
	LCAMitIdlePro(pmsg);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PckCommApp_Mit()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
int
PckCommApp_Mit(unsigned short nFunCode,  unsigned char* pData, int nDataLen, unsigned char* pFrm)
{
	int nFrmLen = 0;
//	int nLenPos = 0;
	unsigned char nMitCmd = 0;
	BYTE byGateLen = 0;
	BYTE byFrontLen = 0;
	BYTE byBackLen = 0;
	unsigned long long dlFrontUnlockLevel = 0;
	unsigned long long dlBackUnlockLevel = 0; 
	CHAR cLevel = 0;
	CHAR vLevel = 0;
	unsigned long long dlFrontLevel = 0;
	unsigned long long dlFrontValue = 0;
	unsigned long long dlBackLevel = 0;
	unsigned long long dlBackValue = 0;
	CHAR cALevel = 0;
	CHAR cBLevel = 0;
	CHAR cVALevel = 0;
	CHAR cVBLevel = 0;

	// FUNCTION CODE
	switch(nFunCode) 
	{
		case FC_LF_STOP_DOWN:
		case FC_LF_STOP_UP:
		case FC_LF_STOP: 
			break;
		case FC_LF_A_B:
			nMitCmd = MIT_PROTOCOL_A_B;

			cALevel = *(pData + 1);
			cBLevel = *(pData + 3);
			ConvertVAFloor(cALevel, &cVALevel);
			ConvertVAFloor(cBLevel, &cVBLevel);
			*(pData + 1) = cVALevel;
			*(pData + 3) = cVBLevel;
	
			break;
		case FC_LF_UNLOCK:
			if (nDataLen > 4)
			{
				byGateLen = *(pData + 2);
				byFrontLen = byGateLen & 0x0F;
				byBackLen = (byGateLen & 0xF0) >> 4;
				memcpy(&dlFrontUnlockLevel, pData + 3, byFrontLen);
				memcpy(&dlBackUnlockLevel, pData + 3 + byFrontLen, byBackLen);

				if (byFrontLen > 0)
				{
					for (cLevel = 0; cLevel < 64; cLevel++)
					{
						if (ConvertVAFloor(cLevel - 8, &vLevel))
						{
							dlFrontValue = ((unsigned long long)1 << vLevel) * ((dlFrontUnlockLevel & ((unsigned long long)1 << cLevel)) >> cLevel);
							dlFrontLevel = dlFrontLevel | dlFrontValue;
						}
					}
					memcpy(pData + 3, &dlFrontLevel, byFrontLen);
				}

				if (byBackLen > 0)
				{
					for (cLevel = 0; cLevel < 64; cLevel++)
					{
						if (ConvertVAFloor(cLevel - 8, &vLevel))
						{
							dlBackValue = ((unsigned long long)1 << vLevel) * ((dlBackUnlockLevel & ((unsigned long long)1 << cLevel)) >> cLevel);
							dlBackLevel = dlBackLevel | dlBackValue;
						}
					}
					memcpy(pData + 3 + byFrontLen, &dlBackLevel, byBackLen);
				}

				nMitCmd = MIT_PROTOCOL_INCAR_M;
			}
			else
			{
				cLevel = *(pData + 2);
				if(cLevel==0xff)
				{
					*(pData + 2) = cLevel;
				}
				else
				{
					ConvertVAFloor(cLevel, &vLevel);
					*(pData + 2) = vLevel;
				}
				nMitCmd = MIT_PROTOCOL_INCAR_A;
			}
			break;
		case FC_LF_STATE_NOTIFY:
			break;
		case FC_LF_STATE:
			break;
		case FC_LF_HEARTBEAT:
			nMitCmd = MIT_PROTOCOL_HEARTBEAT;
			break;
		default:
			break;
	}

	if (FC_LF_STATE == nFunCode)
	{
		nFrmLen = 0;
	}
	else if (FC_LF_HEARTBEAT == nFunCode)
	{
		*(pFrm + nFrmLen) = nMitCmd;
		nFrmLen += LEN_MIT_APP_LY_FC;

		*(pFrm + nFrmLen) = 0x00;
		nFrmLen += 1;

		*(pFrm + nFrmLen) = 0x00;
		nFrmLen += 1;
	}
	else
	{
		*(pFrm + nFrmLen) = nMitCmd;
		nFrmLen += LEN_MIT_APP_LY_FC;
		// LEN
		*(pFrm + nFrmLen) = ++g_byIndex;
		nFrmLen += LEN_MIT_APP_LY_INDEX;
		// DATA
		if (nDataLen > 0)
		{
			memcpy((pFrm + nFrmLen), pData, nDataLen);
			nFrmLen += nDataLen;
		}
	}
	
	return nFrmLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PackCommRqtExMit()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
int
PackCommRqtExMit(unsigned char* pAppBuf, int nAppBufLen, unsigned char ucBnk, unsigned char ucNod , unsigned char* pFrm)
{
	int nFrmLen	= 0;
	unsigned char usChk = 0;
	int i = 0;

	// STX
	*(pFrm + nFrmLen + 0) = MIT_PACK_STX;
	nFrmLen += LEN_MIT_STX;

	// BNK
	*(pFrm + nFrmLen) = ucBnk;
	nFrmLen += LEN_MIT_BNK;

	// NOD
	*(pFrm + nFrmLen) = ucNod;
	nFrmLen += LEN_MIT_NOD;
	
	// DATA LEN
	*(pFrm + nFrmLen) = nAppBufLen;
	nFrmLen += LEN_MIT_DATALEN;

	// DATA
	memcpy(pFrm + nFrmLen, pAppBuf, nAppBufLen);
	nFrmLen += nAppBufLen;

	// CHK
	usChk = 0;
	for (i = 1; i < nFrmLen; i++)
	{
		usChk += *(pFrm + i);
	}
	usChk = ~usChk + 1;
	*(pFrm + nFrmLen) = usChk;
	nFrmLen += LEN_MIT_CHK;	

	// ETX
	*(pFrm + nFrmLen) = MIT_PACK_ETX;
	nFrmLen += LEN_MIT_ETX;
	
	return nFrmLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendData2RS485_Mit()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
unsigned char
SendData2RS485_Mit(unsigned char* pAppBuf, int nAppBufLen, unsigned char ucBnk, unsigned char ucNod)
{	
	unsigned char pDataSend[256] = {0};
	short nDataLen = 0;

	nDataLen = PackCommRqtExMit(pAppBuf, nAppBufLen, ucBnk, ucNod, pDataSend);

	WriteMox485Data((char *)pDataSend, nDataLen);

	return 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMitIdlePro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
LCAMitIdlePro(MXMSG * pmsg)
{
	unsigned char AppTemp[1024];
	int nAppTempLen = 0;
	unsigned char nDataLen = 0;
//	char pDA[3] = {0};
	unsigned char ucRetCode = 0;
	unsigned char ucBnk = 0;
	unsigned char ucNod	= 0;

	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}

	// fill the BNK & NOD
	ucBnk = *((unsigned char*)pmsg->pParam);
	ucNod = *((unsigned char*)pmsg->pParam + 1);
	nDataLen = *((unsigned char*)pmsg->pParam + 2);	

	nAppTempLen = PckCommApp_Mit( (unsigned short) pmsg->dwMsg, 
			pmsg->pParam + 3, nDataLen, 
			AppTemp);

	ucRetCode = SendData2RS485_Mit(AppTemp, nAppTempLen, ucBnk, ucNod);


//	if (FC_LF_HEARTBEAT != pmsg->dwMsg)
	{
		g_LCAProtocol.nStatus	= LCA_STATUS_WAITING_ACK;
		g_LCAProtocol.dwTick	= GetTickCount();
		g_LCAProtocol.nMsg		= pmsg->dwMsg;
		g_LCAProtocol.dwIP		= pmsg->dwParam;
	}				
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAMitWaitingAckPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
VOID
LCAMitWaitingAckPro(VOID)
{
	static unsigned char pDataRecv[256] = {0};
	static int nDataLen = 0;
	short nReadLen = 0;
//	unsigned char ucRetCode = 0;
	int nMitDataLen = 0;
	BOOL bCheckData=FALSE;
    
	if ( g_LCAProtocol.nStatus == LCA_STATUS_WAITING_ACK && (GetTickCount() - g_LCAProtocol.dwTick > MIT_WAIT_ACK_TIME) )
	{
		if (FC_LF_STATE == g_LCAProtocol.nMsg)
		{
			if (g_BnkChkState.bankstate[g_BnkChkState.checkpos] < MIT_BIGFAULT_COUNT)
			{
				g_BnkChkState.bankstate[g_BnkChkState.checkpos]++;
			}
			
			if (g_BnkChkState.checkpos == g_LTConfig.banksize - 1)
			{
				g_BnkChkState.checkpos = 0;
			}
			else
			{
				g_BnkChkState.checkpos++;
			}
		}

		SendErrorACK2Target(g_LCAProtocol.nMsg, g_LCAProtocol.dwIP, 0xff);
		g_LCAProtocol.dwTick	= GetTickCount();
		g_LCAProtocol.nStatus	= LCA_STATUS_IDLE;
		nDataLen = 0;
	}
	
	nReadLen = ReadMox485Data(&pDataRecv[nDataLen], 255 - nDataLen);
	if (nReadLen > 0 )
	{
		nDataLen += nReadLen;
	}

	if (nDataLen >= MIN_MIT_DATA_LEN && IsCpltMitCommfrm(pDataRecv, &nDataLen, &nMitDataLen))
	{
		bCheckData=TRUE;
		if (FC_LF_STATE == g_LCAProtocol.nMsg)
		{
			g_BnkChkState.bankstate[g_BnkChkState.checkpos] = 0;
			
			if (g_BnkChkState.checkpos == g_LTConfig.banksize - 1)
			{
				g_BnkChkState.checkpos = 0;
			}
			else
			{
				g_BnkChkState.checkpos++;
			}
		}

		SendErrorACK2Target(g_LCAProtocol.nMsg, g_LCAProtocol.dwIP, 0x00);
		g_LCAProtocol.nStatus	= LCA_STATUS_IDLE;
		g_LCAProtocol.dwTick	= GetTickCount();
		nDataLen = 0;
	}
	
	if (bCheckData > 0)
	{
		g_dwStateCount++;
	}
	
	if (GetTickCount() - g_dwStateTick > MIT_LIFTSTATE_TIME)
	{
		if (g_dwStateCount > 0)
		{
			//printf("Lift state is normal.\n");
			g_dwStateCount = 0;
		}
		else
		{
			printf("Lift state is abnormal.\n");
		}
		
		g_dwStateTick = GetTickCount();
	}

	
	if (g_LCAProtocol.nStatus == LCA_STATUS_IDLE)
	{
		MXMSG	msgSend;
	
		memset(&msgSend, 0, sizeof(MXMSG));
		msgSend.dwSrcMd		= MXMDID_LT_MITSUBISHI;
		msgSend.dwDestMd	= MXMDID_LT_MITSUBISHI;
	
		msgSend.dwMsg	= FC_LF_HEARTBEAT;
		msgSend.pParam	= NULL;

		if (MitPreprocessMsg(&msgSend))
		{
			LCAMitProcessMsg(&msgSend);
		}
		if(msgSend.pParam)
			free(msgSend.pParam);
	}

}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendErrorACK2Target()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
SendErrorACK2Target(DWORD dwMsg, DWORD dwIP, unsigned char ucRetCode)
{
	MXMSG  msgSend;

	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LT_MITSUBISHI;
	msgSend.dwDestMd	= MXMDID_LCA;

	if (dwMsg < 0x8000)
	{
		msgSend.dwMsg		= dwMsg + 0x8000;
	}
	msgSend.dwParam		= dwIP;
	msgSend.pParam		= malloc(1);
	*msgSend.pParam		= ucRetCode;
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsCpltMitCommfrm()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
int
IsCpltMitCommfrm(unsigned char* pBuf, int* pnLen, int* pDataLen)
{
	BOOL bCpltFrm = FALSE;
	int nValidHeadPos = 0;
	unsigned char nLen = 0;
	int i = 0;

	// Check minimum length
	if (*pnLen < MIN_MIT_DATA_LEN)
	{
		return 0;
	}

	for (i = 0; i <= *pnLen - MIN_MIT_DATA_LEN; i++)
	{
		// Find STX
		if (MIT_PACK_STX	== *(pBuf + i + 0))	
		{
			// Save first valid head position
			if (0 == nValidHeadPos)
			{
				nValidHeadPos = i;
			}

			nLen = *(pBuf + i + 3);

			// Check LEN, VER, ETX
			if (	(nLen <= *pnLen - i) 
					&& (MIT_PACK_ETX == *(pBuf + i + nLen + 5))
					&& (IsCorrectCheckSum((unsigned char* )(pBuf + i + 1), (nLen + 3), (*(pBuf + i + 4 + nLen)))))
			{
				*pDataLen = nLen;
				bCpltFrm = TRUE;
				break;
			}
			else
			{
				nLen = 0;
			}
		}
	}

	if (bCpltFrm)
	{
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
**	FUNCTION NAME:	MitPreprocessMsg()
**	AUTHOR:			Wayde Zeng
**	DATE:			14 - Jan - 2011
**
**	DESCRIPTION:	
**				preprocess message of Mitsubishi
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
MitPreprocessMsg(MXMSG* msgRecv)
{
	switch(msgRecv->dwMsg)
	{
		case FC_LF_STATE:
		{
			if (g_BnkChkState.bankstate[g_BnkChkState.checkpos] < MIT_BIGFAULT_COUNT)
			{
				if (g_BnkChkState.bankstate[g_BnkChkState.checkpos] > 0)
				{
#ifdef MIT_DEBUG
					printf("%s small fault: bank%d state: %d\n", __FUNCTION__, g_BnkChkState.checkpos, g_BnkChkState.bankstate[g_BnkChkState.checkpos]);
#endif
				}
				
				msgRecv->pParam		= malloc(3);
				if (NULL == msgRecv->pParam)
				{
					printf(" %s: ERROR, malloc memory failed.\n", __FUNCTION__);
					return FALSE;
				}
				
				*(msgRecv->pParam) = g_LTConfig.bank[g_BnkChkState.checkpos];
			//	*(msgRecv->pParam + 1) = 0xFF;
				*(msgRecv->pParam + 1) = 0x80;
				*(msgRecv->pParam + 2) = 0;
			}
			else if (4 == g_BnkChkState.bankstate[g_BnkChkState.checkpos])
			{
				if (GetTickCount() - g_BnkChkState.FaultTick[g_BnkChkState.checkpos] > MIT_BIGFAULT_TIME)
				{
#ifdef MIT_DEBUG
					printf("%s big fault: bank%d state: %d\n", __FUNCTION__, g_BnkChkState.checkpos, g_BnkChkState.bankstate[g_BnkChkState.checkpos]);
#endif					
					msgRecv->pParam		= malloc(3);
					if (NULL == msgRecv->pParam)
					{
						printf(" %s: ERROR, malloc memory failed.\n", __FUNCTION__);
						return FALSE;
					}
					
					*(msgRecv->pParam) = g_LTConfig.bank[g_BnkChkState.checkpos];
					//*(msgRecv->pParam + 1) = 0xFF;
					*(msgRecv->pParam + 1) = 0x80;
					*(msgRecv->pParam + 2) = 0;

					g_BnkChkState.FaultTick[g_BnkChkState.checkpos] = GetTickCount();
				}
				else
				{
					SendErrorACK2Target(msgRecv->dwMsg, msgRecv->dwParam, 0xff);
					return FALSE;
				}
			}
			break;
		}
		case FC_LF_HEARTBEAT:
		{
			DWORD tmp = GetTickCount() - g_dwHeartBeatTick ;
			if ( tmp > MIT_HEARTBEAT_TIME)
			{
				g_dwHeartBeatTick = GetTickCount();
				msgRecv->pParam		= malloc(3);
				if (NULL == msgRecv->pParam)
				{
					printf(" %s: ERROR, malloc memory failed.\n", __FUNCTION__);
					return FALSE;
				}
				
				*(msgRecv->pParam) = 0x7F;
				*(msgRecv->pParam + 1) = 0xFF;
				//*(msgRecv->pParam + 1) = 0x80;
				*(msgRecv->pParam + 2) = 0;
			}
			else
			{
				return FALSE;
			}
			break;
		}		
		default:
		{
			break;
		}
	}

	return TRUE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ConvertVAFloor()
**	AUTHOR:			Wayde Zeng
**	DATE:			9 - March - 2011
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
ConvertVAFloor(CHAR AFloor, CHAR* pVFloor)
{
	int i = 0;

	for (i = 0; i < g_VAFloor.nCount; i++)
	{
		if (g_VAFloor.pPCVA[i].AFloor == AFloor)
		{
			*pVFloor = g_VAFloor.pPCVA[i].VFloor;
			return TRUE;
		}
	}

	*pVFloor = AFloor;
	return FALSE;
}


