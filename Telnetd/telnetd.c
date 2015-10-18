/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MoxModnet.c
**
**	AUTHOR:		Alan Huang
**
**	DATE:		12 - Nov - 2001
**
**	FILE DESCRIPTION:
**		Mox Modnet process 
**
**	FUNCTIONS:
**		
**
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/
#define MOX_MODNET
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "dsys0soc.h"
#include "telnetd.h"
#include "DiagTelnetd.h"
#include "MXTypes.h"

/************** DEFINES ***************************************************************************************/

/************** TYPEDEFS **************************************************************************************/


/************** EXTERNAL DECLARATIONS *************************************************************************/

/************** ENTRY POINT DECLARATIONS **********************************************************************/


typSOC_ID	MoxTelnetdSocket = INVALID_SOCKET;

MOXTELNETDCONN*		MoxTelnetdConn[MAX_TELNETD_CONN_NUM];
SHORT		nTelnetdConnect = 0;

// export funcions
SHORT	MoxTelnetdInit();
SHORT	MoxTelnetdProcess();
SHORT	MoxTelnetdExit();

// local funcions.
SHORT	ProcessTelnetd(SHORT nClientIndex);
SHORT	TelnetdDestroyClient(SHORT nClientIndex);
SHORT	MoxTelnetdCheckConnect();
void 	ParseTelnetInput(SHORT nClientIndex);
//int	FindUdpSlot(SOCKADDR_IN FromAddr);


/************** LOCAL DECLARATIONS ****************************************************************************/


/**************************************************************************************************************/
DWORD	
dsysGetTick()
{
	return GetTickCount();
}
/*hdr
**
**	Copyright Mox Product, Australia
**
**	FUNCTION NAME:	MoxTelnetdInit(), create telnetd server.
**
**	AUTHOR:		Alan Huang
**
**	DATE:		12 - Nov - 2002
**
**	DESCRIPTION:
**			Init MoxModnet
**
**	ARGUMENTS:
**		none
**			description	- 
**			type		- 
**			range		-
**
**	RETURNED VALUE:
*r			description	- error code
**			type		- SHORT
**			range		- 0: no error, other: error
**
**	PRE-CONDITIONS:
**
**	POST-CONDITIONS:
**
**	PSEUDO CODE:
**
**	BEGIN
**	END
**
**	NOTES: 
*/

SHORT
MoxTelnetdInit()
{
	SHORT		nRet = 0;
	typSOC_ADD	LocalAddr;
	//typSOC_ADD	LocalAddrUdp;
	SHORT		iLoop;

	dsysSocInit();
	
	nRet = dsysSocAddressSet(NULL, MOX_TELNETD_PORT, &LocalAddr);
	if (nRet != NO_ERROR)
	{
		return nRet;
	}

	nRet = dsysSocCreate(&MoxTelnetdSocket, ISA_SOC_NONBLOCKING);
	if (nRet != NO_ERROR)
	{
		return nRet;
	}

	nRet = dsysSocOptionSet(&MoxTelnetdSocket, ISA_SOC_OPT_REUSEADDR, 1);
	if (nRet != NO_ERROR)
	{
		return nRet;
	}

	nRet = dsysSocBind(&MoxTelnetdSocket, &LocalAddr);
	if (nRet != NO_ERROR)
	{
		return nRet;
	}

	nRet = dsysSocListen(&MoxTelnetdSocket, MAX_TELNETD_CONN_NUM);
	if (nRet != NO_ERROR)
	{
		return nRet;
	}
	for (iLoop = 0; iLoop < MAX_TELNETD_CONN_NUM; iLoop++)
	{
		MoxTelnetdConn[iLoop] = NULL;
	}

	return nRet;
}
 
/*hdr
**
**	Copyright Mox Product, Australia
**
**	FUNCTION NAME:	MoxTelnetdProcess
**
**	AUTHOR:		Alan Huang
**
**	DATE:		12 - Nov - 2002
**
**	DESCRIPTION:
**			Read request from socket
**
**	ARGUMENTS:
*p		none
**			description	- 
**			type		- 
**			range		-
**
**	RETURNED VALUE:
*r			description	- error code
**			type		- SHORT
**			range		- 0: no error, other: error
**
**	PRE-CONDITIONS:
**
**	POST-CONDITIONS:
**
**	PSEUDO CODE:
**
**	BEGIN
**	END
**
**	NOTES:
*/

SHORT
MoxTelnetdProcess()
{
	SHORT		nRet = 0;
	SHORT		nCount;

	MoxTelnetdCheckConnect();

	for (nCount = 0; nCount < MAX_TELNETD_CONN_NUM; nCount++)
	{
		if (MoxTelnetdConn[nCount] != NULL)
		{
			nRet = ProcessTelnetd(nCount);
			if (nRet != NO_ERROR)
			{
				// client lost.
				TelnetdDestroyClient(nCount);
			}
		}
	}

	return nRet;
}

 
/*hdr
**
**	Copyright Mox Product Australia
**
**	FUNCTION NAME:	MoxTelnetdExit()
**
**	AUTHOR:		Alan Huang
**
**	DATE:		12 - Nov - 2002
**
**	DESCRIPTION:
**			Exit MoxTelnetd, destroy server and all connected client
**
**	ARGUMENTS:
*p		none
**			description	- 
**			type		- 
**			range		-
**
**	RETURNED VALUE:
*r			description	- error code
**			type		- SHORT
**			range		- 0: no error, other: error
**
**	PRE-CONDITIONS:
**
**	POST-CONDITIONS:
**
**	PSEUDO CODE:
**
**	BEGIN
**	END
**
**	NOTES:
*/

SHORT
MoxTelnetdExit()
{
	SHORT	nRet = 0;
	SHORT	nCount;

	if (ISA_SOC_ISINVALID(&MoxTelnetdSocket) == FALSE)
	{
		dsysSocClose(&MoxTelnetdSocket, TRUE);
	}
	for (nCount = 0; nCount < MAX_TELNETD_CONN_NUM; nCount++)
	{
		if (MoxTelnetdConn[nCount] != NULL)
		{
 			TelnetdDestroyClient(nCount);
 		}
	}
	
	dsysSocExit();

	return nRet;
}

/*hdr
**
**	Copyright Mox Product, Australia
**
**	FUNCTION NAME:	MoxTelnetdCheckConnect()
**
**	AUTHOR:		Alan Huang
**
**	DATE:		12 - Nov - 2002
**
**	DESCRIPTION:
**			check client connection, build a client for the connection.
**
**	ARGUMENTS:
*p		None
**			description	-  
**			type		-  
**			range		-  
**
**	RETURNED VALUE:
*r			description	- error code
**			type		- SHORT
**			range		- 0: a new client built, other: no new client built
**
**	PRE-CONDITIONS:
**
**	POST-CONDITIONS:
**
**	PSEUDO CODE:
**
**	BEGIN
**	END
**
**	NOTES:
*/

SHORT
MoxTelnetdCheckConnect()
{
	SHORT		nRet = -1;
	typSOC_ID	IncomeSocket = INVALID_SOCKET;
	typSOC_ADD	PeerAddr;
	SHORT		iLoop;
	DWORD		dwTick = 0xFFFFFFFF;
	SHORT		nToFree = 0;
	if (dsysSocAccept(&MoxTelnetdSocket, &IncomeSocket, &PeerAddr) == 1)
	{
		if (nTelnetdConnect >= (MAX_TELNETD_CONN_NUM - 1))
		{
			// if cable lost server can not detect error.
//printf("connect full %d\n", nMdnConnect);
			for (iLoop = 0; iLoop < MAX_TELNETD_CONN_NUM; iLoop++)
			{
				if (MoxTelnetdConn[iLoop] != NULL)
				{
					if (MoxTelnetdConn[iLoop]->dwLastTick < dwTick)
					{
						dwTick = MoxTelnetdConn[iLoop]->dwLastTick;
						nToFree = iLoop;
					}
				}
			}
			if (nToFree < MAX_TELNETD_CONN_NUM)
			{
printf("connect full, destroy %d\n", nToFree);
				TelnetdDestroyClient(nToFree);
			}
							
		}
		for (iLoop = 0; iLoop < MAX_TELNETD_CONN_NUM; iLoop++)
		{
			if (MoxTelnetdConn[iLoop] == NULL)
			{
				MoxTelnetdConn[iLoop]= (MOXTELNETDCONN*)malloc(sizeof(MOXTELNETDCONN));
				if(MoxTelnetdConn[iLoop] == NULL)
				{
					printf("MoxModnetConn mem allocate faile\n");
					nRet = -1;
					break;
				}
				MoxTelnetdConn[iLoop]->Socket = IncomeSocket;
				MoxTelnetdConn[iLoop]->bInUse = TRUE;
				MoxTelnetdConn[iLoop]->nRecvLen = 0;
				MoxTelnetdConn[iLoop]->dwLastTick = dsysGetTick();
				IncomeSocket = INVALID_SOCKET;
				MoxTelnetdConn[iLoop]->nStat = 0;

				nTelnetdConnect++;
				nRet = 0;
				ShowHint(iLoop);
				break;
			}
		}
	}

	if (ISA_SOC_ISINVALID(&IncomeSocket) == FALSE)
	{
		// close connection if client could not create.
		dsysSocClose(&IncomeSocket, TRUE);
	}
	return nRet;
}

/*hdr
**
**	Copyright Mox Product, Australia
**
**	FUNCTION NAME:	TelnetdDestroyClient()
**
**	AUTHOR:		Alan Huang
**
**	DATE:		12 - Nov - 2002
**
**	DESCRIPTION:
**			free a client resource.
**
**	ARGUMENTS:
*p		nClientIndex
**			description	- client index
**			type		- SHORT
**			range		- 0 - (MAX_MODNET_CONN_NUM - 1)
**
**	RETURNED VALUE:
*r			description	- error code
**			type		- SHORT
**			range		- 0: no error, other: error
**
**	PRE-CONDITIONS:
**
**	POST-CONDITIONS:
**
**	PSEUDO CODE:
**
**	BEGIN
**	END
**
**	NOTES:
*/

SHORT
TelnetdDestroyClient(SHORT nClientIndex)
{
	SHORT	nRet = 0;

	if (MoxTelnetdConn[nClientIndex] != NULL)
	{
		if (ISA_SOC_ISINVALID(&MoxTelnetdConn[nClientIndex]->Socket) == FALSE)
		{
			dsysSocClose(&MoxTelnetdConn[nClientIndex]->Socket, TRUE);
		}
		free(MoxTelnetdConn[nClientIndex]);
		MoxTelnetdConn[nClientIndex] = NULL;
		nTelnetdConnect--;
	}

	FlagShowLogData = FALSE;
	
	return nRet;
}

/*hdr
**
**	Copyright Mox Product, Australia
**
**	FUNCTION NAME:	ProcessTelnetd()
**
**	AUTHOR:		Alan Huang
**
**	DATE:		12 - Nov - 2002
**
**	DESCRIPTION:
**			read request, executive the request and build reply.
**
**	ARGUMENTS:
*p		nClientIndex
**			description	- client index
**			type		- SHORT
**			range		- 0 - (MAX_MODNET_CONN_NUM - 1)
**
**	RETURNED VALUE:
*r			description	- error code
**			type		- SHORT
**			range		- 0: no error, other: error
**
**	PRE-CONDITIONS:
**
**	POST-CONDITIONS:
**
**	PSEUDO CODE:
**
**	BEGIN
**	END
**
**	NOTES:
**			Read request
**			parse received data with Modnet protocol
**			IF get a request
**				exective the request
**				build request.
**			END		
*/

SHORT
ProcessTelnetd(SHORT nClientIndex)
{
	SHORT	nRet = NO_ERROR;
	int	nReceived;
	SHORT	nCheck;
 
	if (MoxTelnetdConn[nClientIndex] != NULL)
	{
		// get and parse the command
		nCheck = dsysSocCheckReceive(&MoxTelnetdConn[nClientIndex]->Socket, 0);
 		if (nCheck == BAD_RET)
		{
			nRet = BAD_RET;
		}
		if (nCheck == 1)
		{
			if (MoxTelnetdConn[nClientIndex]->nRecvLen >= MAX_TELNET_BUFF)
			{
				MoxTelnetdConn[nClientIndex]->nRecvLen = 0;
			}
			nReceived = dsysSocReceive(&MoxTelnetdConn[nClientIndex]->Socket,
							&MoxTelnetdConn[nClientIndex]->RecvBuff[MoxTelnetdConn[nClientIndex]->nRecvLen],
							MAX_TELNET_BUFF - MoxTelnetdConn[nClientIndex]->nRecvLen);
			
 			if (nReceived == BAD_RET)
			{
 				nRet = BAD_RET;
			}
			if (nReceived > 0)
			{
				MoxTelnetdConn[nClientIndex]->dwLastTick = dsysGetTick();

				MoxTelnetdConn[nClientIndex]->nRecvLen += nReceived;
				ParseTelnetInput(nClientIndex);
			}
		}
		// feed msg to client;
		BuildFeedData(nClientIndex);
		
		if (MoxTelnetdConn[nClientIndex]->nSendLen > 0 
				&& dsysSocCheckConnect(&MoxTelnetdConn[nClientIndex]->Socket, 0) == 1)
		{
			dsysSocSend(	&MoxTelnetdConn[nClientIndex]->Socket,
							MoxTelnetdConn[nClientIndex]->SendBuff,
							MoxTelnetdConn[nClientIndex]->nSendLen);
		}
		else
		{
		}
	}
	return nRet;
}

/*hdr
**
**	Copyright Mox Product, Australia
**
**	FUNCTION NAME:	ParseTelnetInput()
**
**	AUTHOR:		Alan Huang
**
**	DATE:		12 - Nov - 2002
**
**	DESCRIPTION:
**			read request, executive the request .
**
**	ARGUMENTS:
*p		nClientIndex
**			description	- client index
**			type		- SHORT
**			range		- 0 - (MAX_MODNET_CONN_NUM - 1)
**
**	RETURNED VALUE:
*r			description	- error code
**			type		- SHORT
**			range		- 0: no error, other: error
**
**	PRE-CONDITIONS:
**
**	POST-CONDITIONS:
**
**	PSEUDO CODE:
**
**	BEGIN
**	END
**
**	NOTES:
**			END		
*/

void
ParseTelnetInput(SHORT nClientIndex)
{
		switch (MoxTelnetdConn[nClientIndex]->RecvBuff[0])
		{		
			case KEY_LOG:
			{
				FlagShowLogData = TRUE;
				nSleepTick = 500;
				MoxTelnetdConn[nClientIndex]->nStat = KEY_LOG;
				break;
			}
			case KEY_STATISTIC:
			{
				FlagShowLogData = FALSE;
				nSleepTick = 3000;
				MoxTelnetdConn[nClientIndex]->nStat = KEY_STATISTIC;
				break;
			}
			case KEY_RESET:
			{
				FlagShowLogData = FALSE;
				//MoxTelnetdConn[nClientIndex]->nStat = KEY_STATISTIC;
				memset(dwDiag, 0, sizeof(WORD) * DIAG_ITEM_COUNT);
				break;
			}
			default:
			{
				MoxTelnetdConn[nClientIndex]->nStat = 0;
				ShowHint(nClientIndex);
				FlagShowLogData = FALSE;
				break;
			}
		}
		MoxTelnetdConn[nClientIndex]->nRecvLen = 0;

}
	 
