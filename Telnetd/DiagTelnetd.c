// Modbus.cpp : Defines the entry point for the console application.
//
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "MXCommon.h"
#include "MXTypes.h"
#include "dsys0soc.h"
#include "DiagTelnetd.h"
#include "telnetd.h"
#include "MenuParaProc.h"

int		nDiagItem = DIAG_ITEM_COUNT;
DWORD	dwDiag[DIAG_ITEM_COUNT] = {0};
CHAR	chDiagName[DIAG_ITEM_COUNT][32]={
"ETH_SENT",
"ETH_SENT_ERR",
"ETH_RCV",
"ETH_RCV_ERR",

"A_SENT",
"A_SENT_ERR",
"A_RCV",
"A_RCV_ERR",
"A_RCV_RATE",
"A_SENT_RATE",

"V_SENT",
"V_SENT_ERR",
"V_RCV",
"V_RCV_ERR",
"V_SEND_RATE",
"V_RCV_RATE",

"V_FRM_SENT",
"V_FRM_SENT_ERR",
"V_FRM_RCV",
"V_FRM_RCV_ERR",
"V_RCV_FRM_RATE",
"V_SENT_FRM_RATE",
"DIAG_MM_POS",
"DIAG_THREAD_COUNT",
"DIAG_SRLREAD_COUNT"
};

char chLog[MAX_LOG_BUFF][MAX_LOG_MSG_LEN];
int nLogStart = 0;
int nLogEnd = 0;
DWORD	dwLastTick = 0;

void 	BuildDiagTable(CHAR* chSenderBuffer);
BOOL	DiagPutOneLog(CHAR* pMsg);
BOOL 	DiagGetOneLog(CHAR* pMsg);

int		nSleepTick = 3000;
pthread_t TelnetWork;

static void	DiagTelnetdThreadFun(void* arg);

void
TelnetInit()
{
	if ((pthread_create(&TelnetWork, NULL, DiagTelnetdThreadFun, NULL)) != 0)	
	{
		printf("Telnet: create thread fail\n");
	}
}

static void
DiagTelnetdThreadFun(void* arg)
{
	printf("Telnet Server server startup!\n");
	memset(dwDiag, 0, sizeof(DWORD) * DIAG_ITEM_COUNT);
	if (0 == MoxTelnetdInit())
	{
		while (1)
		{
			MoxTelnetdProcess();
			usleep(nSleepTick * 1000);
		}
		pthread_exit(0);
		MoxTelnetdExit();
	}	
}

void
ShowHint(SHORT nClientIndex)
{
	char    chHints[150] = { 0 };
	/*char	chHints1[]= {
	"EGM 900.1 Firmware version:""\n\r"
	"[s] Diagnostic Data\n\r"
	"[l] realtime log\n\r"
	"[p] Pure log\n\r"
	"[r] Reset Diagnostic data\n\r"};
	printf("strlen(chHints) = %d\n", strlen(chHints));*/

	strcat(chHints, "EGM 900.1 Firmware version:");
	strcat(chHints, g_SysInfo.SoftwareVersion);
	strcat(chHints, "\n\r");
	strcat(chHints, "[s] Diagnostic Data\n\r");
	strcat(chHints, "[l] realtime log\n\r");
	strcat(chHints, "[p] Pure log\n\r");
	strcat(chHints, "[r] Reset Diagnostic data\n\r");

	if (dsysSocCheckConnect(&MoxTelnetdConn[nClientIndex]->Socket, 0) == 1)
	{
		dsysSocSend(	&MoxTelnetdConn[nClientIndex]->Socket,
						chHints,
						strlen(chHints));
	}		
}

void
BuildFeedData(SHORT nClientIndex)
{
	UCHAR sequence[10];
	sequence[0] = 27; //ESC;
	sequence[1] = 91; //LSB;
	sequence[2] = 49;	//Ascii Code of 1
	sequence[3] = 74; //SE;
	
	MoxTelnetdConn[nClientIndex]->nSendLen = 0;
	
	if (KEY_STATISTIC == MoxTelnetdConn[nClientIndex]->nStat)
	{
		memcpy(MoxTelnetdConn[nClientIndex]->SendBuff, sequence, 4);
		BuildDiagTable((CHAR*)&MoxTelnetdConn[nClientIndex]->SendBuff[4]);
		MoxTelnetdConn[nClientIndex]->nSendLen = strlen((CHAR*)MoxTelnetdConn[nClientIndex]->SendBuff);			
	}
	if (KEY_LOG == MoxTelnetdConn[nClientIndex]->nStat)
	{
		if (TRUE == DiagGetOneLog((CHAR*)MoxTelnetdConn[nClientIndex]->SendBuff))
		{
			strcat((CHAR*)MoxTelnetdConn[nClientIndex]->SendBuff, "\n\r");
			MoxTelnetdConn[nClientIndex]->nSendLen = strlen((CHAR*)MoxTelnetdConn[nClientIndex]->SendBuff);
		}
	}	
}

void 
BuildDiagTable(CHAR* chSenderBuffer)
{
	int		nCount = 0;
	CHAR	chItem[100] = {0};

	*chSenderBuffer = 0;
	for (nCount = 0; nCount < nDiagItem; nCount++)
	{
		sprintf(chItem, "%s: %d", chDiagName[nCount], dwDiag[nCount]); 
		strcat(chSenderBuffer, chItem);
		if (nCount%2)
		{
			strcat(chSenderBuffer, "\n\r");
		}
		else
		{
			strcat(chSenderBuffer, "\t\t");
		}
	}
	strcat(chSenderBuffer, "\n\r");

}

BOOL
DiagPutOneLog(CHAR* pMsg)
{
	//int nCount;
	DWORD	dwTick = dsysGetTick();
	
	pMsg[MAX_LOG_MSG_LEN-1]=0;
	
	nLogEnd++;
	if (nLogEnd == MAX_LOG_BUFF)
	{
		nLogEnd = 0;
	}
	//sprintf(chLog[nLogEnd], " t:%d ", (dwTick - dwLastTick));
	//dwLastTick = dsysGetTick();
	sprintf(chLog[nLogEnd], " t:%d ", dwTick );	
	strcat(chLog[nLogEnd], pMsg);
	if (nLogStart == nLogEnd)
	{
		nLogStart++;
		if (nLogStart == MAX_LOG_BUFF)
		{
		  nLogStart = 0;
		 } 
	}		

	return TRUE;
}


BOOL 
DiagGetOneLog(CHAR* pMsg)
{
	if (nLogStart == nLogEnd)
	{
		return FALSE;
	}
	nLogStart++;	
	if (nLogStart == MAX_LOG_BUFF)
	{
		nLogStart = 0;
	}
	
	strcpy(pMsg, chLog[nLogStart]);

	return TRUE;
}

