/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MoxSysTime.c
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		19 - May - 2006
**
**	FILE DESCRIPTION:
**				Time functions
**
**	FUNCTIONS:
**				MoxSysToHc
**				MoxTimeInit
**				MoxTimeExit
**				MoxGetLocalTickCount
**				MoxGetGlobalTickCount
**				MoxSetGlobalTick
**				MoxSetRealTime
**				MoxTimeWait
**
**	NOTES:
**
*/

/************** SYSTEM INCLUDE FILES **************************************************/
// __SUPPORT_PROTOCOL_900_1A__
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/************** USER INCLUDE FILES ***************************************************/
#include "MXCommon.h"
#include "PioApi.h"
#include "MenuParaProc.h"
#include "IOControl.h"
#include "TalkEventWnd.h"

/************** DEFINES **************************************************************/
#define DI_STATUS_NORMAL 		0	//Õý³£
#define DI_STATUS_TRIGGER 		1	//´¥·¢
/************** TYPEDEFS *************************************************************/
/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/
static int _DIRead(int);
static int _DOClear(int);
static int _DOSet(int);
static int _DOGetRealID(int id);
static int _DOUnlockClear();
static int _DOUnlockSet();

static int _DIGetRealID(int id);
static void _DIPlaySound(int id);
static int _DI1Read();
static int _DI2Read();

int MOX_DEV_DIRead(int id){
//	printf("MOX_DEV_DIRead %d\n",id);
	return _DIRead(id);
}

int MOX_DEV_DOClear(int id){
	printf("MOX_DEV_DOClear %d\n",id);
	return _DOClear(id);
}

int MOX_DEV_DOSet(int id){
	printf("MOX_DEV_DOSet %d\n",id);
	return _DOSet(id);
}
/************** LOCAL FUNCTION ***************************************************/
static int _DOGetRealID(int id)
{
	switch(id)
	{
		case 1:
			return PIO_TYPE_DO_1;
		case 2:
			return PIO_TYPE_DO_2;
		case 201:
			return PIO_TYPE_UNLOCK;
		default:
			break;
	}
	return -1;
}

static int _DIGetRealID(int id)
{
	switch(id)
	{
		case 1:
			return PIO_TYPE_DI1;
		case 2:
			return PIO_TYPE_DI2;
		default:
			break;
	}
	return -1;
}

static int _DIRead(int id)
{
	int nStatus = DI_STATUS_NORMAL;
	int nID = _DIGetRealID(id);
	int nFun = DIFUN_NULL;
	if(nID == -1)	
	{
		printf("invalid port id, please check!\n");
		return -1;
	}
	
	switch(nID)
	{
		case PIO_TYPE_DI1:
			nStatus = _DI1Read();
			if(nStatus == DI_STATUS_TRIGGER)
				nFun = GetDIFunction(DIPORT_1);
			break;
		case PIO_TYPE_DI2:
			nStatus = _DI2Read();
			if(nStatus == DI_STATUS_TRIGGER)
				nFun = GetDIFunction(DIPORT_2);
			break;
		default:
			break;
	}

	if(nFun == DIFUN_PLAYSOUND)
		_DIPlaySound(nID);
	
//	printf("[DI%d][%d]\n",id,nStatus);
	return nStatus;
}

static int _DOClear(int id)
{
	int nID = _DOGetRealID(id);
	if(nID == -1)	
	{
		printf("DO prot ID error, please check setting!\n");
		return -1;
	}

	if(nID == PIO_TYPE_UNLOCK)
		return _DOUnlockClear();
		
	if(GetDOMode(id) == DOMT_NOPEN) 
		PIOWrite(nID,DO_OFF);
	else
		PIOWrite(nID,DO_ON);

	return 0;
}

static int _DOSet(int id)
{
	int nID = _DOGetRealID(id);
	if(nID == -1)	
	{
		printf("DO prot ID error, please check setting!\n");
		return -1;
	}
	
	if(nID == PIO_TYPE_UNLOCK)
		return _DOUnlockSet();

	if(GetDOMode(id) == DOMT_NCLOSE) 
		PIOWrite(nID,DO_OFF);
	else
		PIOWrite(nID,DO_ON);

	return 1;
}

static int _DOUnlockClear()
{
	printf("DO unlock clear!\n");
	PIOWrite(PIO_TYPE_UNLOCK, DO_OFF);
	return 0;
}

static int _DOUnlockSet()
{
	printf("DO unlock set!\n");
	PIOWrite(PIO_TYPE_UNLOCK, DO_ON);
	return 1;
}


static DWORD g_Tick = 0;
#define SOUND_PLAY_WIDTH	(1000 * 3)
static void _DIPlaySound(int id)
{
	DWORD nTick = GetTickCount();

	if(nTick - g_Tick < SOUND_PLAY_WIDTH)
		return;

	g_Tick = nTick;

	switch(id)
	{
		case PIO_TYPE_DI1:
			StartPlayDI1ANote();
			break;
		case PIO_TYPE_DI2:
			StartPlayDI2ANote();
			break;
		default:
			break;
	}
	
}

static int _DI1Read()
{
	int nStatus = DI_STATUS_NORMAL;
	BYTE nResult;
	static int perStatus = DI_STATUS_NORMAL;
	
	nResult = ReadDI1();
	if( nResult == DI_HIGH_LEVEL)
		nStatus = DI_STATUS_TRIGGER;

	if(nResult != DI_NO_DETECT)
		perStatus = nStatus;
	else
		nStatus = perStatus;
	
	return nStatus;
}

static int _DI2Read()
{
	int nStatus = DI_STATUS_NORMAL;
	BYTE nResult;
	static int perStatus = DI_STATUS_NORMAL;
	
	nResult = ReadDI2();
	if( nResult == DI_HIGH_LEVEL)
		nStatus = DI_STATUS_TRIGGER;

	if(nResult != DI_NO_DETECT)
		perStatus = nStatus;
	else
		nStatus = perStatus;
	
	return nStatus;

}


