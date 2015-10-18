/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SystemInfoAPI.c
**
**	AUTHOR:		Mark	Qian
**
**	DATE:		21 - Jan - 2010
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
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
#include "LiftCtrlConversion.h"
#include	"IniFile.h"
/************** DEFINES **************************************************************/
//#define LCMT_DEBUG
//#define LC_DEBUG
//static unsigned int GetHCCMTIndexByIPCode(DWORD dwIP,const char * szCode);

static TOTAL_STATE g_TotalState = TOTAL_STATE_SENDING_INIT;
static SENDING_STATE g_SendingState = SENDING_STATE_SENDING_ENQ;
static RECEIVING_STATE g_ReceivingState = RECEIVING_STATE_START;

static unsigned char g_SerialInputBuf[SERIAL_BUF_LEN];
static unsigned char g_SerialInputDataBuf[SERIAL_BUF_LEN];

static HitachiData g_SerialOutputDataBuf;
static LCHC		g_LcHMT;


static int g_CheckTimes = 0;
static DWORD g_TotalTick			= 0;
static DWORD g_SendingTick		= 0;
static DWORD g_ReceivingTick		= 0;

static int g_ReadSerialLen			= 0;

static BOOL g_DataEnable 			= FALSE;

static MXMSG	g_MsgRecev;
//static MXMSG	g_MsgSend;

pthread_t 	LiftCtrlConversionWork;

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static void LoadHCCMT(void);
static BOOL HitRead485Data4Char(unsigned char DestChar);

void
LCAConversionMdInit(void)
{
	if (LC_HITACHI_MODE == g_SysConfig.LC_ComMode)
	{
		g_TotalTick = GetTickCount();
		g_CheckTimes = 0;
		g_DataEnable = FALSE;
		g_TotalState = TOTAL_STATE_SENDING_INIT;
		g_SendingState = SENDING_STATE_SENDING_ENQ;
		g_ReceivingState = RECEIVING_STATE_START;

		DpcAddMd(MXMDID_HITACHI, NULL);
		LoadHCCMT();
		Write485Data_temp((char *)("0"), 2);
	}
}

void
LCConversionMdExit(void)
{
	if (LC_HITACHI_MODE == g_SysConfig.LC_ComMode) 
	{	
		DpcRmMd(MXMDID_HITACHI);
	}
}

static void
LoadHCCMT(void)
{
	const char * pChar = NULL;
	int nCount = 0;
	int i = 0;
	char szSec[20] = {0};

	g_LcHMT.nCount = 0;
	g_LcHMT.pHCCMT = NULL;

	OpenIniFile(HCCMT_FILE);
	
	nCount = ReadInt(HCCMT_SECTION, COUNT_KEY, 0);

	if (nCount > 0 && nCount < 10001)
	{
		g_LcHMT.nCount = nCount;
		g_LcHMT.pHCCMT = malloc(sizeof(HCCMT) * nCount);
		memset(g_LcHMT.pHCCMT, 0, sizeof(HCCMT) * nCount);

		if (g_LcHMT.pHCCMT == NULL)
		{
			printf("Error: the g_LcHMT.pHCCMT can not be NULL...\n");
			CloseIniFile();
			return;
		}

		for (i = 0; i < nCount; i++)
		{
			WatchDog();
			snprintf(szSec, sizeof (szSec), "user%d", i + 1);

			pChar = ReadString(szSec, HCCMT_KEY_CODE, "");
			strcpy(g_LcHMT.pHCCMT[i].szCode, pChar);
#ifdef LCMT_DEBUG
			printf("[%s]\n%s=%s\n", szSec, HCCMT_KEY_CODE, g_LcHMT.pHCCMT[i].szCode);
#endif
			g_LcHMT.pHCCMT[i].ucType = ReadInt(szSec, HCCMT_KEY_TYPE, 0);

#ifdef LCMT_DEBUG
			printf("%s=%d\n", HCCMT_KEY_TYPE, g_LcHMT.pHCCMT[i].ucType);
#endif

			if (ETH_DEVICE_TYPE_EHV == g_LcHMT.pHCCMT[i].ucType) 
			{
				pChar = ReadString(szSec, HCCMT_KEY_BUILDNUM, "");
				if (strlen(pChar) == 2)
				{
					strcpy(g_LcHMT.pHCCMT[i].szBuildingNum, pChar);
				}
				else
				{	
					printf("the value of [%s]:%s is error: %s.\n", szSec, HCCMT_KEY_BUILDNUM, pChar);
				}
#ifdef LCMT_DEBUG
			printf("%s=%s\n",  HCCMT_KEY_BUILDNUM, g_LcHMT.pHCCMT[i].szBuildingNum);
#endif
				pChar = ReadString(szSec, HCCMT_KEY_ROOMNUM, "");
				if (strlen(pChar) == 4)
				{
					strcpy(g_LcHMT.pHCCMT[i].szRoomNum, pChar);
				}
				else
				{	
					printf("the value of [%s]:%s is error: %s.\n", szSec, HCCMT_KEY_ROOMNUM, pChar);
				}
				
				pChar = ReadString(szSec, HCCMT_KEY_HALLNUM, "");
				if (strlen(pChar) == 2)
				{
					strcpy(g_LcHMT.pHCCMT[i].szHallNum, pChar);
				}
				else
				{	
					printf("the value of [%s]:%s is error: %s.\n", szSec, HCCMT_KEY_HALLNUM, pChar);
				}
					
				
				
				pChar = ReadString(szSec, HCCMT_KEY_DOORCODE, "");
				strcpy(g_LcHMT.pHCCMT[i].szDoorCode, pChar);
				
				
#ifdef LCMT_DEBUG
			printf("%s=%s\n",  HCCMT_KEY_ROOMNUM, g_LcHMT.pHCCMT[i].szRoomNum);
			printf("%s=%s\n",  HCCMT_KEY_HALLNUM, g_LcHMT.pHCCMT[i].szHallNum);
#endif
			}
			else
			{
				pChar = ReadString(szSec, HCCMT_KEY_HALLNUM, "");
				if (strlen(pChar) == 2)
				{
					strcpy(g_LcHMT.pHCCMT[i].szHallNum, pChar);
				}
				else
				{	
					printf("the value of [%s]:%s is error: %s.\n", szSec, HCCMT_KEY_HALLNUM, pChar);
				}
#ifdef LCMT_DEBUG
			printf("%s=%s\n",  HCCMT_KEY_HALLNUM, g_LcHMT.pHCCMT[i].szHallNum);
#endif
			}
		}
	}
	else
	{
#ifdef LCMT_DEBUG
			printf("ERROR: the g_LcHMT.pHCCMT is NULL, can not find hccmt.ini, count=%d\n", g_LcHMT.nCount);
#endif
	}

	CloseIniFile();

}

/*static unsigned char 
GetUserType(const char * szCode)
{
	int i = 0;
	if (g_LcHMT.pHCCMT == NULL)
	{
		return 0xff;
	}

	for (i = 0; i < g_LcHMT.nCount; i++)
	{
		if (!strcmp(szCode, g_LcHMT.pHCCMT[i].szCode))
		{
			return g_LcHMT.pHCCMT[i].ucType;
		}
	}
	return 0;
}*/


static const char * 
GetBuildingNum(DWORD dwIP,const char * szCode)
{
	int i = 0;
	if (g_LcHMT.pHCCMT == NULL)
	{
		return NULL;
	}
	/*i=GetHCCMTIndexByIPCode(dwIP,szCode);
	if(i)
	{
		return g_LcHMT.pHCCMT[i-1].szBuildingNum;
	}*/
	for (i = 0; i < g_LcHMT.nCount; i++)
	{
		if (!strcmp(szCode, g_LcHMT.pHCCMT[i].szCode) )
		{
			return g_LcHMT.pHCCMT[i].szBuildingNum;
		}
	}
	return NULL;
}

static const char * 
GetRoomNum(DWORD dwIP,const char * szCode)
{
	int i = 0;

	if (g_LcHMT.pHCCMT == NULL)
	{
		return NULL;
	}
	/*i=GetHCCMTIndexByIPCode(dwIP,szCode);
	if(i)
	{
		return g_LcHMT.pHCCMT[i-1].szRoomNum;
	}*/
	for (i = 0; i < g_LcHMT.nCount; i++)
	{
		if (!strcmp(szCode, g_LcHMT.pHCCMT[i].szCode)) 
		{
			return g_LcHMT.pHCCMT[i].szRoomNum;
		}
	}
	return NULL;
}

static const char * 
GetHallNum(const char * szDoorCode,const char * szCode)
{
	int i = 0;
printf("GetHallNum szDoorCode=%s,szCode=%s\n",szDoorCode,szCode);
	if (g_LcHMT.pHCCMT == NULL)
	{
		return NULL;
	}
	/*i=GetHCCMTIndexByIPCode(dwIP,szCode);
	if(i)
	{
		return g_LcHMT.pHCCMT[i-1].szHallNum;
	}*/
	
	for (i = 0; i < g_LcHMT.nCount; i++)
	{
		if ((!strcmp(szCode, g_LcHMT.pHCCMT[i].szCode)) && (!strcmp(szDoorCode, g_LcHMT.pHCCMT[i].szDoorCode)))
		{
			printf("Get Hall Num By DoorCode\n");
			return g_LcHMT.pHCCMT[i].szHallNum;
		}
	}
	for (i = 0; i < g_LcHMT.nCount; i++)
	{
		if ((!strcmp(szCode, g_LcHMT.pHCCMT[i].szCode)) && (strlen(g_LcHMT.pHCCMT[i].szDoorCode)==0))
		{
			printf("Get Hall Num\n");
			return g_LcHMT.pHCCMT[i].szHallNum;
		}
	}
	
	return NULL;
}



static BOOL
IsValidInputDataBuf()
{
	int i = 0;
	unsigned char value = 0;
	int length = SERIAL_BUF_LEN - 1;

	if (g_SerialInputDataBuf[0] != cmd_Stx)
	{
#ifdef LC_DEBUG	
		printf("**************** g_SerialInputDataBuf[0] != cmd_Stx: %02X\n", g_SerialInputDataBuf[0]);
#endif
		return FALSE;
	}
	if (g_SerialInputDataBuf[20] != cmd_Etx)
	{
#ifdef LC_DEBUG	
		printf("**************** g_SerialInputDataBuf[20] != cmd_Etx: %02X\n", g_SerialInputDataBuf[20]);
#endif
		return FALSE;
	}		

	value = 0;
	for (i = 1; i < length; i++)
	{	
		//printf("%02X ", g_SerialInputDataBuf[i]);
		value += g_SerialInputDataBuf[i];
	}
	//printf("%02X\n", g_SerialInputDataBuf[21]);
	if (value != g_SerialInputDataBuf[length])
	{
#ifdef LC_DEBUG	
		printf("**************** value != g_SerialInputDataBuf[length]: %02X, %02X\n", g_SerialInputDataBuf[length], value);

#endif
		return FALSE;
	}	
	else return TRUE;
}

static BOOL
PckOutputDataBuf(DWORD dwIP,unsigned char mode, unsigned char nValidCodeDigits, char SrcRdNum[RD_CODE_LEN], char DestRdNum[RD_CODE_LEN])
{
	int i = 0;
	int srcDigits = 0;
	int destDigits = 0;
	unsigned char value = 0;
	int length = SERIAL_BUF_LEN - 1;
	const char * pCharBuildingNum = NULL;
	const char * pCharRoomNum = NULL;
	const char * pCharHallNum = NULL;
	
#ifdef LCMT_DEBUG
printf("%s: the mode=%d. the validcodedigit=%d.\n src=%s. the dest=%s.\n",
	   __FUNCTION__, mode, nValidCodeDigits, SrcRdNum, DestRdNum);
#endif
	if(strlen(DestRdNum)==0)
	{
		return FALSE;
	}
	srcDigits = strlen(SrcRdNum);
	if (nValidCodeDigits < srcDigits) srcDigits = nValidCodeDigits;

	destDigits = strlen(DestRdNum);
	if (nValidCodeDigits < destDigits) destDigits = nValidCodeDigits;	

	
	
	if (HO_QUERY_METHOD_EHV != mode) 
	{
		if(strlen(SrcRdNum)==0)
		{
			return FALSE;
		}
		pCharHallNum = GetHallNum((const char *)SrcRdNum,(const char *)DestRdNum);
		if (pCharHallNum == NULL) 
		{
			printf("Error: can not get the dest device hall number...\n");
			return FALSE;
		}
	}

	
	pCharBuildingNum = GetBuildingNum(dwIP,(const char *)DestRdNum);
	if (pCharBuildingNum == NULL) 
	{
		printf("Error: can not get the dest device building number...\n");
		return FALSE;
	}
	pCharRoomNum = GetRoomNum(dwIP,(const char *)DestRdNum);
	if (pCharRoomNum == NULL) 
	{
		printf("Error: can not get the dest device room number...\n");
		return FALSE;
	}
	
	g_SerialOutputDataBuf.stx = cmd_Stx;
	
	g_SerialOutputDataBuf.length[0] = '0';
	g_SerialOutputDataBuf.length[1] = '1';
	g_SerialOutputDataBuf.length[2] = '6';
printf("mode=%d\n",mode);
	switch(mode)
	{
		//case HO_QUERY_METHOD_HOME:
	//	case HO_QUERY_METHOD_OUTSIDE:
		case HO_QUERY_METHOD_CSN:
		case HO_QUERY_METHOD_CALLCODE:
			g_SerialOutputDataBuf.command[0] = '1';
			g_SerialOutputDataBuf.command[1] = '9';
			break;
		case HO_QUERY_METHOD_EHV:
			g_SerialOutputDataBuf.command[0] = '1';
			g_SerialOutputDataBuf.command[1] = '8';
			break;
		default:
#ifdef LC_DEBUG
			printf("**************** PckOutputDataBuf: Error Hitachi mode!\n");
#endif
			break;
	}
	
	g_SerialOutputDataBuf.event[0] = '0';
	g_SerialOutputDataBuf.event[1] = '1';

	switch(mode)
	{
		//case HO_QUERY_METHOD_HOME:
		case HO_QUERY_METHOD_CALLCODE:
			g_SerialOutputDataBuf.code[0] = '0';
			g_SerialOutputDataBuf.code[1] = '1';
			break;
		//case HO_QUERY_METHOD_OUTSIDE:
		case HO_QUERY_METHOD_CSN:
			g_SerialOutputDataBuf.code[0] = '0';
			g_SerialOutputDataBuf.code[1] = '1';
			break;
		case HO_QUERY_METHOD_EHV:
			g_SerialOutputDataBuf.code[0] = '0';
			g_SerialOutputDataBuf.code[1] = '0';
			break;
		default:
#ifdef LC_DEBUG
			printf("**************** PckOutputDataBuf: Error Hitachi mode!\n");
#endif
			break;
	}
	
	g_SerialOutputDataBuf.reserved[0] = '0';
	g_SerialOutputDataBuf.reserved[1] = '0';

	/*pCharBuildingNum = GetBuildingNum(dwIP,(const char *)DestRdNum);
	if (pCharBuildingNum == NULL) 
	{
		printf("Error: can not get the dest device building number...\n");
	}
	else*/
	{
		g_SerialOutputDataBuf.buildingNum[0] = pCharBuildingNum[0];
		g_SerialOutputDataBuf.buildingNum[1] = pCharBuildingNum[1];
#ifdef LCMT_DEBUG
		printf("the dest=%s, the buildingnum=%s.\n", DestRdNum, pCharBuildingNum);
#endif
	}

	/*pCharRoomNum = GetRoomNum(dwIP,(const char *)DestRdNum);
	if (pCharRoomNum == NULL) 
	{
		printf("Error: can not get the dest device room number...\n");
	}
	else*/
	{
		g_SerialOutputDataBuf.roomNum[0] = pCharRoomNum[0];
		g_SerialOutputDataBuf.roomNum[1] = pCharRoomNum[1];
		g_SerialOutputDataBuf.roomNum[2] = pCharRoomNum[2];
		g_SerialOutputDataBuf.roomNum[3] = pCharRoomNum[3];
#ifdef LCMT_DEBUG
		printf("the dest=%s, the roomNum=%s.\n", DestRdNum, pCharRoomNum);
#endif

	}

	if (HO_QUERY_METHOD_EHV == mode) 
	{
		g_SerialOutputDataBuf.hallNum[0] = '0';
		g_SerialOutputDataBuf.hallNum[1] = '0';		
	}
	else
	{
	/*	pCharHallNum = GetHallNum(dwIP,(const char *)DestRdNum);
		if (pCharHallNum == NULL) 
		{
			printf("Error: can not get the dest device hall number...\n");
		}
		else*/
		{
			g_SerialOutputDataBuf.hallNum[0] = pCharHallNum[0];
			g_SerialOutputDataBuf.hallNum[1] = pCharHallNum[1];
		}
	}

	g_SerialOutputDataBuf.etx = cmd_Etx;

	value = 0;
	for (i = 1; i < length; i++) value += ((unsigned char *)(&(g_SerialOutputDataBuf)))[i];	
	g_SerialOutputDataBuf.sum = value;
	return TRUE;
}


/*
static void
PckOutputDataBuf(unsigned char mode, unsigned char nValidCodeDigits, char SrcRdNum[RD_CODE_LEN], char DestRdNum[RD_CODE_LEN])
{
	int i = 0;
	int srcDigits = 0;
	int destDigits = 0;
	unsigned char value = 0;
	int length = SERIAL_BUF_LEN - 1;
printf("%s: the mode=%d. the validcodedigit=%d.\n src=%s. the dest=%s.\n",
	   __FUNCTION__, mode, nValidCodeDigits, SrcRdNum, DestRdNum);

	srcDigits = strlen(SrcRdNum);
	if (nValidCodeDigits < srcDigits) srcDigits = nValidCodeDigits;

	destDigits = strlen(DestRdNum);
	if (nValidCodeDigits < destDigits) destDigits = nValidCodeDigits;	

	g_SerialOutputDataBuf.stx = cmd_Stx;
	
	g_SerialOutputDataBuf.length[0] = '0';
	g_SerialOutputDataBuf.length[1] = '1';
	g_SerialOutputDataBuf.length[2] = '6';

	switch(mode)
	{
		case HO_QUERY_METHOD_HOME:
		case HO_QUERY_METHOD_OUTSIDE:
			g_SerialOutputDataBuf.command[0] = '1';
			g_SerialOutputDataBuf.command[1] = '9';
			break;
		case HO_QUERY_METHOD_EHV:
			g_SerialOutputDataBuf.command[0] = '1';
			g_SerialOutputDataBuf.command[1] = '8';
			break;
		default:
#ifdef LC_DEBUG
			printf("**************** PckOutputDataBuf: Error Hitachi mode!\n");
#endif
			break;
	}
	
	g_SerialOutputDataBuf.event[0] = '0';
	g_SerialOutputDataBuf.event[1] = '1';

	switch(mode)
	{
		case HO_QUERY_METHOD_HOME:
			g_SerialOutputDataBuf.code[0] = '0';
			g_SerialOutputDataBuf.code[1] = '1';
			break;
		case HO_QUERY_METHOD_OUTSIDE:
			g_SerialOutputDataBuf.code[0] = '0';
			g_SerialOutputDataBuf.code[1] = '3';
			break;
		case HO_QUERY_METHOD_EHV:
			g_SerialOutputDataBuf.code[0] = '0';
			g_SerialOutputDataBuf.code[1] = '0';
			break;
		default:
#ifdef LC_DEBUG
			printf("**************** PckOutputDataBuf: Error Hitachi mode!\n");
#endif
			break;
	}
	
	g_SerialOutputDataBuf.reserved[0] = '0';
	g_SerialOutputDataBuf.reserved[1] = '0';

	printf("#################### destDigits: %d\n", destDigits);

	switch(destDigits)
	{
		case 0:
			g_SerialOutputDataBuf.buildingNum[0] = '0';
			g_SerialOutputDataBuf.buildingNum[1] = '1';
			g_SerialOutputDataBuf.roomNum[0] = '0';
			g_SerialOutputDataBuf.roomNum[1] = '0';
			g_SerialOutputDataBuf.roomNum[2] = '0';
			g_SerialOutputDataBuf.roomNum[3] = '0';
			break;
		case 1:
			g_SerialOutputDataBuf.buildingNum[0] = '0';
			g_SerialOutputDataBuf.buildingNum[1] = '1';
			g_SerialOutputDataBuf.roomNum[0] = '0';
			g_SerialOutputDataBuf.roomNum[1] = '0';
			g_SerialOutputDataBuf.roomNum[2] = '0';
			g_SerialOutputDataBuf.roomNum[3] = DestRdNum[0];
			break;
		case 2:
			g_SerialOutputDataBuf.buildingNum[0] = '0';
			g_SerialOutputDataBuf.buildingNum[1] = '1';
			g_SerialOutputDataBuf.roomNum[0] = '0';
			g_SerialOutputDataBuf.roomNum[1] = '0';
			g_SerialOutputDataBuf.roomNum[2] = DestRdNum[0];
			g_SerialOutputDataBuf.roomNum[3] = DestRdNum[1];
			break;
		case 3:
			g_SerialOutputDataBuf.buildingNum[0] = '0';
			g_SerialOutputDataBuf.buildingNum[1] = '1';
			g_SerialOutputDataBuf.roomNum[0] = '0';
			g_SerialOutputDataBuf.roomNum[1] = DestRdNum[0];
			g_SerialOutputDataBuf.roomNum[2] = DestRdNum[1];
			g_SerialOutputDataBuf.roomNum[3] = DestRdNum[2];
			break;
		case 4:
			g_SerialOutputDataBuf.buildingNum[0] = '0';
			g_SerialOutputDataBuf.buildingNum[1] = '1';
			g_SerialOutputDataBuf.roomNum[0] = DestRdNum[0];
			g_SerialOutputDataBuf.roomNum[1] = DestRdNum[1];
			g_SerialOutputDataBuf.roomNum[2] = DestRdNum[2];
			g_SerialOutputDataBuf.roomNum[3] = DestRdNum[3];
			break;
		case 5:
			g_SerialOutputDataBuf.buildingNum[0] = '0';
			g_SerialOutputDataBuf.buildingNum[1] = '1';
			if (DestRdNum[0] > g_SerialOutputDataBuf.buildingNum[1]) 
				g_SerialOutputDataBuf.buildingNum[1] = DestRdNum[0];
			g_SerialOutputDataBuf.roomNum[0] = DestRdNum[1];
			g_SerialOutputDataBuf.roomNum[1] = DestRdNum[2];
			g_SerialOutputDataBuf.roomNum[2] = DestRdNum[3];
			g_SerialOutputDataBuf.roomNum[3] = DestRdNum[4];
			break;
		default:
			g_SerialOutputDataBuf.buildingNum[0] = '0';
			g_SerialOutputDataBuf.buildingNum[1] = '1';
			if ((DestRdNum[destDigits - 5] > g_SerialOutputDataBuf.buildingNum[1]) || (DestRdNum[destDigits - 6] > g_SerialOutputDataBuf.buildingNum[0])) 
			g_SerialOutputDataBuf.buildingNum[1] = DestRdNum[destDigits - 5];
			g_SerialOutputDataBuf.buildingNum[0] = DestRdNum[destDigits - 6];
			g_SerialOutputDataBuf.roomNum[0] = DestRdNum[destDigits - 4];
			g_SerialOutputDataBuf.roomNum[1] = DestRdNum[destDigits - 3];
			g_SerialOutputDataBuf.roomNum[2] = DestRdNum[destDigits - 2];
			g_SerialOutputDataBuf.roomNum[3] = DestRdNum[destDigits - 1];	

			break;
	}

	if (HO_QUERY_METHOD_EHV == mode)
	{
		g_SerialOutputDataBuf.hallNum[0] = '0';
		g_SerialOutputDataBuf.hallNum[1] = '0';
	}
	else
	{
		switch(srcDigits)
		{
			case 0:
				g_SerialOutputDataBuf.hallNum[0] = '0';
				g_SerialOutputDataBuf.hallNum[1] = '1';
				break;
			case 1:
				g_SerialOutputDataBuf.hallNum[0] = '0';
				g_SerialOutputDataBuf.hallNum[1] = '1';
				if (SrcRdNum[0] > g_SerialOutputDataBuf.hallNum[1]) g_SerialOutputDataBuf.hallNum[1] = SrcRdNum[0];
				break;
			default:
				g_SerialOutputDataBuf.hallNum[0] = '0';
				g_SerialOutputDataBuf.hallNum[1] = '1';
				if ((SrcRdNum[srcDigits - 1] > g_SerialOutputDataBuf.hallNum[1]) || (SrcRdNum[srcDigits - 2] > g_SerialOutputDataBuf.hallNum[0]))
				g_SerialOutputDataBuf.hallNum[1] = SrcRdNum[srcDigits - 1];
				g_SerialOutputDataBuf.hallNum[0] = SrcRdNum[srcDigits - 2];
				break;
		}
	}
	
	g_SerialOutputDataBuf.etx = cmd_Etx;

	value = 0;
	for (i = 1; i < length; i++) value += ((unsigned char *)(&(g_SerialOutputDataBuf)))[i];	
	g_SerialOutputDataBuf.sum = value;
}
*/

static BOOL HitRead485Data4Char(unsigned char DestChar)
{
    int nReadSerialLen;
    do
    {
        nReadSerialLen = Read485Data(g_SerialInputBuf, 1);
        if(nReadSerialLen>0)
        {
            if(DestChar == g_SerialInputBuf[0])
            {
                return TRUE;
            }
        }
    }
    while(nReadSerialLen>0);
    return FALSE;
}

static SENDING_STATE
LiftCtrlConversionSendingData(SENDING_STATE SeningState,const unsigned char *pSendingData)
{
	int nReadSerialLen;

	switch(g_SendingState)
	{
		case SENDING_STATE_SENDING_ENQ:
			printf("start to send quest................\n");
			Write485Data((CHAR *)(&cmd_Enq), sizeof(cmd_Enq));
			g_SendingState = SENDING_STATE_WAITING_ENQ_ACK_1;
			g_SendingTick = GetTickCount();
			break;
			
		case SENDING_STATE_WAITING_ENQ_ACK_1:
            if(HitRead485Data4Char(cmd_Ack)==TRUE)
            {
                g_SendingState = SENDING_STATE_SENDING_DATA;
            }
            else
            {
                if ((GetTickCount() - g_SendingTick) > TIMEOUT_SENDING_WAITING)
                {
                    Write485Data((CHAR *)(&cmd_Enq), sizeof(cmd_Enq));
					g_SendingState = SENDING_STATE_WAITING_ENQ_ACK_2;
					g_SendingTick = GetTickCount();
                }
            }
			break;		
			
		case SENDING_STATE_WAITING_ENQ_ACK_2:
			 if(HitRead485Data4Char(cmd_Ack)==TRUE)
            {
                g_SendingState = SENDING_STATE_SENDING_DATA;
            }
            else
            {
                if ((GetTickCount() - g_SendingTick) > TIMEOUT_SENDING_WAITING)
                {
                    Write485Data((CHAR *)(&cmd_Enq), sizeof(cmd_Enq));
					g_SendingState = SENDING_STATE_WAITING_ENQ_ACK_3;
					g_SendingTick = GetTickCount();
                }
            }
			break;	
			
		case SENDING_STATE_WAITING_ENQ_ACK_3:
			if(HitRead485Data4Char(cmd_Ack)==TRUE)
            {
                g_SendingState = SENDING_STATE_SENDING_DATA;
            }
            else
            {
                if ((GetTickCount() - g_SendingTick) > TIMEOUT_SENDING_WAITING)
                {
                    Write485Data((CHAR *)(&cmd_Enq), sizeof(cmd_Enq));
					g_SendingState = SENDING_STATE_WAITING_ENQ_ACK_4;
					g_SendingTick = GetTickCount();
                }
            }
			break;
			
		case SENDING_STATE_WAITING_ENQ_ACK_4:
            if(HitRead485Data4Char(cmd_Ack)==TRUE)
            {
                g_SendingState = SENDING_STATE_SENDING_DATA;
            }
            else
            {
                if ((GetTickCount() - g_SendingTick) > TIMEOUT_SENDING_WAITING)
                {
                   g_SendingState = SENDING_STATE_FAIL;
                }
            }
        
			break;
			
		case SENDING_STATE_SENDING_DATA:
		{
			Write485Data((CHAR *)pSendingData, SERIAL_BUF_LEN);
			g_SendingState = SENDING_STATE_WAITING_DATA_ACK_1;
			g_SendingTick = GetTickCount();
			break;
		}
			
		case SENDING_STATE_WAITING_DATA_ACK_1:
		{
            if(HitRead485Data4Char(cmd_Ack)==TRUE)
            {
                g_SendingState = SENDING_STATE_SUCCESS;
            }
            else
            {
                if ((GetTickCount() - g_SendingTick) > TIMEOUT_SENDING_WAITING)
                {
                    Write485Data((CHAR *)pSendingData, SERIAL_BUF_LEN);
					g_SendingState = SENDING_STATE_WAITING_DATA_ACK_2;
					g_SendingTick = GetTickCount();
                }
            }
			break;
		}
		
		case SENDING_STATE_WAITING_DATA_ACK_2:
		{
			 if(HitRead485Data4Char(cmd_Ack)==TRUE)
            {
                g_SendingState = SENDING_STATE_SUCCESS;
            }
            else
            {
                if ((GetTickCount() - g_SendingTick) > TIMEOUT_SENDING_WAITING)
                {
                    Write485Data((CHAR *)pSendingData, SERIAL_BUF_LEN);
					g_SendingState = SENDING_STATE_WAITING_DATA_ACK_3;
					g_SendingTick = GetTickCount();
                }
            }
			break;
		}
			
		case SENDING_STATE_WAITING_DATA_ACK_3:
		{
			 if(HitRead485Data4Char(cmd_Ack)==TRUE)
            {
                g_SendingState = SENDING_STATE_SUCCESS;
            }
            else
            {
                if ((GetTickCount() - g_SendingTick) > TIMEOUT_SENDING_WAITING)
                {
                    Write485Data((CHAR *)pSendingData, SERIAL_BUF_LEN);
					g_SendingState = SENDING_STATE_WAITING_DATA_ACK_4;
					g_SendingTick = GetTickCount();
                }
            }
			break;
		}
			
		case SENDING_STATE_WAITING_DATA_ACK_4:
		{
            if(HitRead485Data4Char(cmd_Ack)==TRUE)
            {
                g_SendingState = SENDING_STATE_SUCCESS;
            }
            else
            {
                if ((GetTickCount() - g_SendingTick) > TIMEOUT_SENDING_WAITING)
                {
                    g_SendingState = SENDING_STATE_FAIL;
                }
            }
			break;
		}
	
		case SENDING_STATE_SUCCESS:
			g_SendingTick = 0;
			break;
			
		case SENDING_STATE_FAIL:
			g_SendingTick = 0;
			break;
			
		default:
			g_SendingState = SENDING_STATE_FAIL;
#ifdef LC_DEBUG
			printf("************** Wrong Hitachi Sending State!\n");
#endif
			break;
	}
	return g_SendingState;
}

static RECEIVING_STATE
LiftCtrlConversionReceivingData(SENDING_STATE ReceivingState)
{
	int nReadSerialLen;
//	int i = 0;

	switch(g_ReceivingState)
	{
		case RECEIVING_STATE_START:
		case RECEIVING_STATE_WAITING_ENQ:
			if (RECEIVING_STATE_START == g_ReceivingState)
			{
				g_ReceivingState = RECEIVING_STATE_WAITING_ENQ;
				g_ReceivingTick = GetTickCount();
			}

			if ((GetTickCount() - g_ReceivingTick) > TIMEOUT_RECEIVING_WAITING)
			{
				printf("wait data time out....aa1\n");
				g_ReceivingState = RECEIVING_STATE_START;
				g_TotalState = TOTAL_STATE_SENDING_INIT;	
				printf("the receiving init response fail...adfadfasdfa..\n");
				return g_ReceivingState;
			}	
			
            if(HitRead485Data4Char(cmd_Enq)==TRUE)
            {
               	printf("Get a enquest command....\n");
                Write485Data((CHAR *)(&cmd_Ack), sizeof(cmd_Ack));
                usleep(200 * 1000);
                g_ReceivingState = RECEIVING_STATE_WAITING_DATA;
                g_ReceivingTick = GetTickCount();
                nReadSerialLen = 0;
            }
          
			break;

		case RECEIVING_STATE_WAITING_DATA:
			if ((GetTickCount() - g_ReceivingTick) > TIMEOUT_RECEIVING_WAITING)
			{
				printf("wait data time out....2\n");
				g_ReceivingState = RECEIVING_STATE_FAIL;
				g_ReceivingTick = 0;
				return g_ReceivingState;
			}
			
		//	nReadSerialLen = Read485Data(g_SerialInputDataBuf , SERIAL_BUF_LEN);
			nReadSerialLen = Read485Data(g_SerialInputDataBuf + g_ReadSerialLen, SERIAL_BUF_LEN - g_ReadSerialLen);
		///////////////
	//		printf("********* nReadSerialLen: %d\n", nReadSerialLen);	
			if (nReadSerialLen > 0)
			{
				if ((1 == nReadSerialLen) && (cmd_Enq == g_SerialInputDataBuf[0]))
				{
					Write485Data((CHAR *)(&cmd_Ack), sizeof(cmd_Ack));
				}
				else
				{
					g_ReadSerialLen += nReadSerialLen;
					if (SERIAL_BUF_LEN == g_ReadSerialLen)
					{
						g_ReadSerialLen = 0;
						if (IsValidInputDataBuf())
						{
							Write485Data((CHAR *)(&cmd_Ack), sizeof(cmd_Ack));
							g_ReceivingState = RECEIVING_STATE_SUCCESS;
						}
						else
						{
							Write485Data((CHAR *)(&cmd_Nak), sizeof(cmd_Nak));
						}
					}
				}
				printf("get data lenth=%d.\n", nReadSerialLen);
			}
			else if (nReadSerialLen < 0)
			{
				printf("read data errno=%d....2\n", errno);
			}

			break;

		case RECEIVING_STATE_SUCCESS:
			g_ReceivingTick = 0;
			break;		

		case RECEIVING_STATE_FAIL:
			printf("receiveing state fail...\n");
	//		g_ReceivingState = RECEIVING_STATE_START;
		//	g_ReceivingTick = 0;
			break;
			
		default:
#ifdef LC_DEBUG
			printf("************** Wrong Hitachi Receiving State: %d\n", g_ReceivingState);
#endif
			g_ReceivingState = RECEIVING_STATE_FAIL;
			break;		
	}		
	return g_ReceivingState;	
}

static SHORT
LiftCtrlConversionProcess()
{
	SHORT		nRet = 0;
//	SHORT		nCount;

	switch (g_TotalState)
	{
		case TOTAL_STATE_WAITING_INIT:
		{
			g_ReceivingState = LiftCtrlConversionReceivingData(g_ReceivingState);
			if (RECEIVING_STATE_SUCCESS == g_ReceivingState)
			{
				g_ReceivingState = RECEIVING_STATE_START;
				if (0 == memcmp(cmd_InitResp, g_SerialInputDataBuf, SERIAL_BUF_LEN))
				{
					g_TotalState = TOTAL_STATE_SENDING_INIT;
					printf("********************* TOTAL_STATE_WAITING_INIT over\n");
					g_TotalTick = GetTickCount();
					g_TotalTick -=  TIME_INTERVAL_SENDING_INIT;
				}
				else g_TotalState = TOTAL_STATE_WAITING_INIT;
			}
			else if (RECEIVING_STATE_FAIL == g_ReceivingState)
			{
				g_ReceivingState = RECEIVING_STATE_START;
				g_TotalState = TOTAL_STATE_WAITING_INIT;					
			}
			break;
		}	
		case TOTAL_STATE_SENDING_INIT:
		{
			g_CheckTimes = 0;
			if ((GetTickCount() - g_TotalTick) < TIME_INTERVAL_SENDING_INIT) return nRet;
			
			g_SendingState = LiftCtrlConversionSendingData(g_SendingState, cmd_InitReq);
			if (SENDING_STATE_SUCCESS == g_SendingState)
			{
				g_SendingState = SENDING_STATE_SENDING_ENQ;
				g_TotalState = TOTAL_STATE_RECEIVING_INIT_RESPONSE;
				printf("********************* TOTAL_STATE_SENDING_INIT over\n");
				g_TotalTick = GetTickCount();
			}
			else if (SENDING_STATE_FAIL == g_SendingState)
			{
				g_SendingState = SENDING_STATE_SENDING_ENQ;
				g_TotalState = TOTAL_STATE_SENDING_INIT;	
				g_ReadSerialLen = 0;
				g_TotalTick = GetTickCount();
			}
			break;
		}
		case TOTAL_STATE_RECEIVING_INIT_RESPONSE:
		{
			g_ReceivingState = LiftCtrlConversionReceivingData(g_ReceivingState);
			if (RECEIVING_STATE_SUCCESS == g_ReceivingState)
			{
				g_ReceivingState = RECEIVING_STATE_START;
				if (0 == memcmp(cmd_InitResp, g_SerialInputDataBuf, SERIAL_BUF_LEN))
				{
					g_TotalState = TOTAL_STATE_SENDING_CHECK;
					printf("********************* TOTAL_STATE_RECEIVING_INIT_RESPONSE over\n");
					g_TotalTick = GetTickCount();
					g_TotalTick -=  TIME_INTERVAL_SENDING_CHECK;
				}
				else g_TotalState = TOTAL_STATE_SENDING_INIT;
			}
			else if (RECEIVING_STATE_FAIL == g_ReceivingState)
			{
				g_ReceivingState = RECEIVING_STATE_START;
				g_TotalState = TOTAL_STATE_SENDING_INIT;	
				printf("the receiving init response fail.....\n");
			}
			break;
		}
		case TOTAL_STATE_SENDING_CHECK:
		{
			if ((GetTickCount() - g_TotalTick) < TIME_INTERVAL_SENDING_CHECK) return nRet;

			g_SendingState = LiftCtrlConversionSendingData(g_SendingState, cmd_CheckReq);
			if (SENDING_STATE_SUCCESS == g_SendingState)
			{
				g_SendingState = SENDING_STATE_SENDING_ENQ;
				g_TotalState = TOTAL_STATE_RECEIVING_CHECK_RESPONSE;
				printf("********************* TOTAL_STATE_SENDING_CHECK over\n");
				g_ReadSerialLen = 0;
				g_TotalTick = GetTickCount();
			}
			else if (SENDING_STATE_FAIL == g_SendingState)
			{
				g_SendingState = SENDING_STATE_SENDING_ENQ;
				g_TotalState = TOTAL_STATE_SENDING_CHECK;			
				g_CheckTimes++;
				g_DataEnable = FALSE;
				g_TotalTick = GetTickCount();
			}

			if (g_CheckTimes > CHECK_REPEAT_TIMES)
			{
				g_TotalState = TOTAL_STATE_WAITING_INIT;			
				g_CheckTimes = 0;
				g_DataEnable = FALSE;
				g_TotalTick = GetTickCount();				
			}
			break;
		}
		case TOTAL_STATE_RECEIVING_CHECK_RESPONSE:
		{
			g_ReceivingState = LiftCtrlConversionReceivingData(g_ReceivingState);
			if (RECEIVING_STATE_SUCCESS == g_ReceivingState)
			{
				g_ReceivingState = RECEIVING_STATE_START;
				if (0 == memcmp(cmd_CheckResp, g_SerialInputDataBuf, SERIAL_BUF_LEN))
				{
					g_TotalState = TOTAL_STATE_SENDING_DATA;
					printf("********************* TOTAL_STATE_RECEIVING_CHECK_RESPONSE over\n");
					g_CheckTimes = 0;
					g_DataEnable = TRUE;
				}
				else
				{
					g_TotalState = TOTAL_STATE_SENDING_CHECK;
					g_CheckTimes++;
					g_DataEnable = FALSE;						
				}
			}
			else if (RECEIVING_STATE_FAIL == g_ReceivingState)
			{
				g_ReceivingState = RECEIVING_STATE_START;
				g_TotalState = TOTAL_STATE_SENDING_CHECK;
				g_CheckTimes++;
				g_DataEnable = FALSE;
			}

			if (g_CheckTimes > CHECK_REPEAT_TIMES)
			{
				g_TotalState = TOTAL_STATE_WAITING_INIT;			
				g_CheckTimes = 0;
				g_DataEnable = FALSE;
				g_TotalTick = GetTickCount();				
			}			
			break;
		}
		case TOTAL_STATE_SENDING_DATA:
		{
			if (TRUE == g_DataEnable)
			{
				memset(&g_MsgRecev, 0, sizeof(g_MsgRecev));
				g_MsgRecev.dwDestMd	= MXMDID_HITACHI;
				g_MsgRecev.pParam		= NULL;				

				if (MxGetMsg(&g_MsgRecev))
				{
					switch(g_MsgRecev.dwMsg)
					{
						case FC_LF_STOP:
							if(PckOutputDataBuf(g_MsgRecev.dwParam,g_MsgRecev.pParam[0],
								g_MsgRecev.pParam[1],
								g_MsgRecev.szSrcDev,
								g_MsgRecev.szDestDev))
								{
									g_DataEnable = FALSE;
								}
							break;	
						case FC_LF_A_B:
							if(PckOutputDataBuf(g_MsgRecev.dwParam,g_MsgRecev.pParam[0],
								g_MsgRecev.pParam[1],
								g_MsgRecev.szSrcDev,
								g_MsgRecev.szDestDev))
								{
									g_DataEnable = FALSE;
								}
							break;						
						default:
								break;
					}
					break;

				}	
				else
				{
					if ((GetTickCount() - g_TotalTick) >= TIME_INTERVAL_SENDING_CHECK)
					{
						g_TotalState = TOTAL_STATE_SENDING_CHECK;
						g_CheckTimes = 0;
						g_DataEnable = FALSE;
					}
				}
			}
			else
			{
				g_SendingState = LiftCtrlConversionSendingData(g_SendingState, (unsigned char *)(&g_SerialOutputDataBuf));
				if (SENDING_STATE_SUCCESS == g_SendingState)
				{
					g_SendingState = SENDING_STATE_SENDING_ENQ;
					g_TotalState = TOTAL_STATE_SENDING_DATA;
					g_CheckTimes = 0;
					g_DataEnable = TRUE;
						
					if ((GetTickCount() - g_TotalTick) >= TIME_INTERVAL_SENDING_CHECK)
					{
						g_TotalState = TOTAL_STATE_SENDING_CHECK;
						g_CheckTimes = 0;
						g_DataEnable = FALSE;
					}
				}
				else if (SENDING_STATE_FAIL == g_SendingState)
				{
					g_SendingState = SENDING_STATE_SENDING_ENQ;
					g_TotalState = TOTAL_STATE_SENDING_CHECK;
					g_CheckTimes = 0;
					g_DataEnable = FALSE;
				}					
			}
			
			if (g_CheckTimes >= CHECK_REPEAT_TIMES)
			{
				g_TotalState = TOTAL_STATE_WAITING_INIT;			
				g_CheckTimes = 0;
				g_DataEnable = FALSE;
				g_TotalTick = GetTickCount();				
			}
			break;
		}
		default:
		{
#ifdef LC_DEBUG
			printf("************ Wrong Hitachi g_TotalState!\n");
#endif
			g_TotalState = TOTAL_STATE_SENDING_INIT;
			break;
		}
	}

	return nRet;
}

void
LCConversionThreadFun(void)
{
#ifdef LC_DEBUG
	//printf("%s: LiftCtrlConversion thread startup!time=%d\n", __FUNCTION__, GetTickCount());
#endif
	memset(&g_MsgRecev, 0, sizeof(g_MsgRecev));
	g_MsgRecev.dwDestMd	= MXMDID_HITACHI;
	g_MsgRecev.pParam		= NULL;

 	if (!g_DataEnable) 
	{
		if (MxGetMsg(&g_MsgRecev)) 
		{
			printf("%s get a message, but EGM discard the message.\n", __FUNCTION__);
		}
	}
	LiftCtrlConversionProcess();
}


/*static unsigned int GetHCCMTIndexByIPCode(DWORD dwIP,const char * szCode)
{
	int i = 0;
	if (g_LcHMT.pHCCMT == NULL)
	{
		return 0xff;
	}
	for (i = 0; i < g_LcHMT.nCount; i++)
	{
		if (!strcmp(szCode, g_LcHMT.pHCCMT[i].szCode) && dwIP==g_LcHMT.pHCCMT[i].dwIP )
		{
			return i+1;
		}
	}
	return 0;
}
*/


