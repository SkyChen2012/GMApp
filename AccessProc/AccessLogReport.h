/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AccessLogReport.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		06 - May - 2009
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
** 
*/
#ifndef ACCESSLOGREPORT_H
#define ACCESSLOGREPORT_H
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

#define AL_IDLE								0
#define AL_BUSY								1

#define TYPE_NULL							0
#define TYPE_SWIPE_CARD						1
#define TYPE_GATE_OPEN_MANUAL				2
#define TYPE_GATE_OPEN						3
#define TYPE_GATE_CLOSE						4
#define TYPE_GATE_OPEN_NOTALARM				5
#define TYPE_GATE_OPEN_OVERTIME_ALARM		6
#define TYPE_INVALID_CARD_ALARM				7
#define TYPE_SWIPE_CARD_V2					10
#define TYPE_FORCE_UNLOCK                   12

#define TYPE_GATE_STATUS					0xF0

#define ASLOG_EVENTTYPE_OFFSET				0
#define ASLOG_EVENTTYPE_LEN					1

#define ASLOG_EVENTTIME_OFFSET				(ASLOG_EVENTTYPE_OFFSET + ASLOG_EVENTTYPE_LEN)
#define ASLOG_EVENTTIME_LEN					4

#define ASLOG_EVENTID_OFFSET				(ASLOG_EVENTTIME_OFFSET + ASLOG_EVENTTIME_LEN)
#define ASLOG_EVENTID_LEN					4

#define	ASLOG_COMMON_LEN					(ASLOG_EVENTTYPE_LEN + ASLOG_EVENTTIME_LEN + ASLOG_EVENTID_LEN)

#define ASLOG_DATALEN_OFFSET				(ASLOG_EVENTID_OFFSET + ASLOG_EVENTID_LEN)
#define ASLOG_DATALEN_LEN					2

#define ASLOG_DATA_OFFSET					(ASLOG_DATALEN_OFFSET + ASLOG_DATALEN_LEN)

//Swipe Card
#define SWIPECARD_CDMODE_OFFSET				0
#define SWIPECARD_CDMODE_LEN				1

#define SWIPECARD_CSN_LEN_OFFSET			(SWIPECARD_CDMODE_OFFSET + SWIPECARD_CDMODE_LEN)
#define SWIPECARD_CSN_LEN_LEN				1

#define SWIPECARD_CSN_OFFSET				(SWIPECARD_CSN_LEN_OFFSET + SWIPECARD_CSN_LEN_LEN)
#define SWIPECARD_CSN_LEN					(CSN_LEN)

#define SWIPECARD_READERPORT_OFFSET			(SWIPECARD_CSN_OFFSET + SWIPECARD_CSN_LEN)
#define SWIPECARD_READERPORT_LEN			1

#define SWIPECARD_READERID_OFFSET			(SWIPECARD_READERPORT_OFFSET + SWIPECARD_READERPORT_LEN)
#define SWIPECARD_READERID_LEN				1

#define SWIPECARD_GATEID_OFFSET				(SWIPECARD_READERID_OFFSET + SWIPECARD_READERID_LEN)
#define SWIPECARD_GATEID_LEN				1

#define SWIPECARD_INOUT_OFFSET				(SWIPECARD_GATEID_OFFSET + SWIPECARD_GATEID_LEN)
#define SWIPECARD_INOUT_LEN					1

#define SWIPECARD_RESULT_OFFSET				(SWIPECARD_INOUT_OFFSET + SWIPECARD_INOUT_LEN)
#define SWIPECARD_RESULT_LEN				1

#define	SWIPECARD_CARDID_OFFSET				(SWIPECARD_RESULT_OFFSET + SWIPECARD_RESULT_LEN)
#define SWIPECARD_CARDID_LEN				4

#define SWIPECARD_DATA_LEN					(SWIPECARD_CDMODE_LEN + SWIPECARD_CSN_LEN_LEN + SWIPECARD_CSN_LEN + SWIPECARD_READERPORT_LEN + SWIPECARD_READERID_LEN + SWIPECARD_GATEID_LEN + SWIPECARD_INOUT_LEN + SWIPECARD_RESULT_LEN + SWIPECARD_CARDID_LEN)

//Gate Open Manual
#define MANUALOPEN_BUTTONID_OFFSET			0
#define MANUALOPEN_BUTTONID_LEN				1

#define MANUALOPEN_GATEID_OFFSET			(MANUALOPEN_BUTTONID_OFFSET + MANUALOPEN_BUTTONID_LEN)
#define MANUALOPEN_GATEID_LEN				1

#define MANUALOPEN_DATA_LEN					(MANUALOPEN_BUTTONID_LEN + MANUALOPEN_GATEID_LEN)

//Door Status
#define DOORSTATUS_GATEID_OFFSET			0
#define DOORSTATUS_GATEID_LEN				1

#define DOORSTATUS_DATA_LEN					(DOORSTATUS_GATEID_LEN)

//iris 900.1 status
#define SWIPECARD_UNLOCK						0
#define SWIPECARD_EXPIRED						1
#define SWIPECARD_NOT_IN_TIME_SLICE				2
#define SWIPECARD_INVALID						3
#define SWIPECARD_CARD_PATROL_UNLOCK			4
#define SWIPECARD_CARD_DISABLED					5
#define SWIPECARD_ENTER_PASSWORD				6	//10
#define SWIPECARD_CARD_AUTHORIZE				7
#define	SWIPECARD_PATROL_NOT_IN_TIME_SLICE	    8
#define	SWIPECARD_PATROL_DISABLED			    9

//not standard stats
#define	SWIPECARD_PASSWORD_USER					100
#define SWIPECARD_PASSWORD_ERROR				101	//6
#define SWIPECARD_PASSWORD_NOT_ENTER			102	//7
#define SWIPECARD_OTHERS						103	//8
#define SWIPECARD_FUN_DISABLE					104	//11
#define SWIPECARD_CARD_PATROL					105	//7
#define SWIPECARD_CARD_PATROL_ENTER_PASSWORD	106

//request
#define SWIPECARD_REQUEST_VERIFY_CSN            0
#define SWIPECARD_REQUEST_VERIFY_PWD            1
#define SWIPECARD_REQUEST_SAVE_NEW_PWD          2

//response
#define CSN_PWD_MOD_INVALID                 0
#define CSN_PWD_MOD_EXPIRED                 1
#define CSN_PWD_MOD_NOT_IN_TIME_SLICE       2
#define CSN_PWD_MOD_DISABLED                3
#define CSN_PWD_MOD_AUTHORIZING             4
#define CSN_PWD_MOD_CARD_EXIST              5
#define CSN_PWD_MOD_PWD_CORRECT             6
#define CSN_PWD_MOD_PATROL                  7
#define CSN_PWD_MOD_FUN_DISABLED            8
#define CSN_PWD_MOD_PWD_ERROR               9

#define GATE_ID_DEFAULT						1
#define READER_PORT_DEFAULT					1
#define READER_ID_DEFAULT					1
#define BUTTON_ID_DEFAULT					1

#define INOUT_DEFAULT						1 //1: In, 2: Out

#define MAX_AS_LOG_COUNT					500
#define EVENT_DATA_LEN						SWIPECARD_DATA_LEN

/************** TYPEDEFS *************************************************************/


typedef	struct _ALHEADER
{
	UINT			dwVersion;		//Log version
	UINT			nCurCnt;		// the current counts of record
	UINT			nMaxCnt;		// the max counts of record
}ALHEADER;


typedef struct _ALDATA
{
	BOOL			bValid;
	BYTE			EventType;
	time_t			EventTime;
	UINT			EventID;
	WORD			DataLen;
	BYTE			pData[EVENT_DATA_LEN];
}ALDATA;

typedef struct _MANASLOG
{
	ALHEADER	AlHeader;
	ALDATA		AlDataHeader[MAX_AS_LOG_COUNT];
}MANASLOG;


typedef struct _ASLOGINFO
{
	UINT	nStatus;
	DWORD	dwTickCount;
	UINT	nCurEventID;
}ASLOGINFO;

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//extern	VOID	RecordSwipeCardLog(CHAR* pCSN, BYTE ReaderID, BYTE GateID, BYTE InOut, BYTE Result);
//extern	VOID	RecordManualOpenLog(BYTE ButtonID, BYTE GateID);
//extern	VOID	RecordDoorStsLog(BYTE LogType, BYTE GateID);

extern	VOID	AsLogReportProc();
extern	VOID	AsLogReportTimeOut();

extern VOID	AsLogInit();
extern BOOL	AsLogProc(MXMSG* pMsg);
extern VOID	AsLogReport();

extern	ASLOGINFO	g_AsLogInfo;

extern	VOID	RecordSwipeCardLog(BYTE CardMode, CHAR* pCSN, BYTE ReaderPort, BYTE ReaderID, BYTE GateID, BYTE InOut, BYTE Result, UINT CardID);
extern	VOID	RecordOpenMnlLog(BYTE ButtonID, BYTE GateID);
extern	VOID	RecordDoorStsLog(BYTE GateSts, BYTE GateID);
extern	VOID	RecordAsAlarmLog(BYTE AlarmType);
#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
extern ALDATA* GetLastSwipCardLog();
extern VOID ClearLastSwipeCardInfo();
extern VOID SetLastSwipeCardInfo();
extern BOOL HaveLastSwipeCardLog();
extern UINT GetSwipeCardLogCntByGate();
extern ALDATA* GetSwipeCardLogByIndex(USHORT index);
extern void SaveALog2Mem();
extern void LoadALogFromMem();
#endif

extern void sendForceUlkLog2MC(void);
/************** ENTRY POINT DECLARATIONS *********************************************/


/************** LOCAL DECLARATIONS ***************************************************/

#endif //ACCESSLOGREPORT_H


