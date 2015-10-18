/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SALogReport.c
**
**	AUTHOR:		Mike Zhang
**
**	DATE:		20 - Dec - 2010
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
** 
*/
#ifndef SALOGREPORT_H
#define SALOGREPORT_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

#include <time.h>

/************** USER INCLUDE FILES ****************************************************************************/

#include "MXCommon.h"
#include "MXTypes.h"
#include "AccessCommon.h"
#include "Dispatch.h"

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/

#define SAL_IDLE								0
#define SAL_BUSY								1

#define MAX_SA_LOG_COUNT					500

/************** TYPEDEFS *************************************************************/


typedef	struct _SALHEADER
{
	UINT			dwVersion;		//Log version
	UINT			nCurCnt;		// the current counts of record
	UINT			nMaxCnt;		// the max counts of record
}SALHEADER;


typedef struct _SALDATA
{
	BOOL			bValid;
	BYTE			EventType;
	time_t			EventTime;
	UINT			EventID;
	WORD			DataLen;
	BYTE*			pData;
}SALDATA;

typedef struct _MANSALOG
{
	SALHEADER	SAlHeader;
	SALDATA		SAlDataHeader[MAX_SA_LOG_COUNT];
}MANSALOG;


typedef struct _SALOGINFO
{
	UINT	nStatus;
	DWORD	dwTickCount;
	UINT	nCurEventID;
}SALOGINFO;

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//extern	VOID	RecordSwipeCardLog(CHAR* pCSN, BYTE ReaderID, BYTE GateID, BYTE InOut, BYTE Result);
//extern	VOID	RecordManualOpenLog(BYTE ButtonID, BYTE GateID);
//extern	VOID	RecordDoorStsLog(BYTE LogType, BYTE GateID);

extern	VOID	SALogReportProc();
extern	VOID	SALogReportTimeOut();

extern VOID	SALogInit();
extern BOOL	SALogProc(MXMSG* pMsg);
extern VOID	SALogReport();

extern	SALOGINFO	g_SALogInfo;

extern	VOID	RecordAlarmLog(BYTE AlarmType);

/************** ENTRY POINT DECLARATIONS *********************************************/


/************** LOCAL DECLARATIONS ***************************************************/

#endif //ACCESSLOGREPORT_H


