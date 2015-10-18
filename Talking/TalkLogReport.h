/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	TalkLogReport.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		01 - Sep - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
** 
*/
#ifndef TALKREPORT_H
#define TALKREPORT_H
/************** SYSTEM INCLUDE FILES **************************************************************************/
#include <time.h>
#include "ModuleTalk.h"
#include "MXTypes.h"

/************** USER INCLUDE FILES ****************************************************************************/


/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/
#define IRIS_CODE_LEN   19

#define STATUS_CALL		1
#define CS_NEWCALL		0
#define CS_COMPLETE		1

#define STATUS_MONITOR	2
#define US_FAIL			0
#define US_SUCCESSFUL	1

#define STATUS_UNLOCK	3
#define MS_FAIL			0
#define MS_SUCCESSFUL	1

#define LEN_LOGDATA     58

#define TL_IDLE			0
#define TL_BUSY			1

#define MAX_TALK_LOG_COUNT	10

/************** TYPEDEFS *************************************************************/

typedef enum _ACTIONTYPE{
	TYPE_CALLOUT = 1,
	TYPE_MONITOR,
	TYPE_UNLOCK
}ACTIONTYPE;

typedef enum _ACTIONSTEP{
	STEP_START	= 1,
	STEP_PICK,
	STEP_END
}ACTIONSTEP;

typedef struct _TLDATA
{
	UCHAR		Type;
	UCHAR		state;
	time_t		TimeStart;
	time_t		TimePick; //Only for call
	time_t		TimeEnd;
	char        ProCode[IRIS_CODE_LEN];
	char        ConCode[IRIS_CODE_LEN];
	BOOL		bHavePic;
	INT			nPicLen;
	CHAR*		PicBuf;
}TLDATA;

typedef	struct _TLHEADER
{
	UINT			dwVersion;
	UINT			nCurCnt;
	UINT			nMaxCnt;
}TLHEADER;


typedef struct _MANTL
{
	TLHEADER	TlHeader;
	TLDATA		TlDataHeader[MAX_TALK_LOG_COUNT];
}MANTL;


typedef struct _TLINFO
{
	UINT		nStatus;
	DWORD		dwTickCount;
	BOOL		bHavePic;
	INT			nPicLen;
	CHAR*		PicBuf;
}TLINFO;

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

extern VOID RecordLogData(ACTIONTYPE ActionType, ACTIONSTEP ActionStep, GMTALKINFO *pGMTalkInfo);

extern TLINFO g_TLInfo;

extern VOID ConvertFromTime_t(UCHAR *pOUTTime, time_t *pTime_tTime);
extern BOOL IsMCOnline();
extern BOOL		IsReportIdle();

extern VOID TlLogInit();
extern VOID	TlLogExit();

extern	VOID	TLTimeOutCtrl();
extern	BOOL	TlLogProc(MXMSG* pMsg);

extern	VOID	TlReport();
/***********************************************************************************/

#endif //TALKREPORT_H


