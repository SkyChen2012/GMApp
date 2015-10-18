/*
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SecurityAlarm.h
**
**	AUTHOR:		Mike Zhang
**
**	DATE:		13 - Dec - 2010
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef SECURITYALARM_H
#define SECURITYALARM_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/


/************** ENTRY POINT DECLARATIONS **********************************************************************/


/************** DEFINES ***************************************************************************************/

#define	WM_SA_LOG_ADDNEW					WM_USER + 26
#define WM_SA_WND_FLASH						WM_USER + 28

#define	SA_ZONE_MAX_NUM						8
#define	SA_PASSWD_KEY_NUM					10

#define	ALARM_TYPE_IMMEIDATE				1
#define	ALARM_TYPE_MONITOR					4
#define	ALARM_TYPE_DELAY					2
#define	ALARM_TYPE_FOLLOW					3

#define	ALARM_TYPE_CNT						4


#define ALARM_TYPE_DISASSEMBLY				5	//
#define	ALARM_TYPE_HIJACK					8
#define	ALARM_TYPE_ENABLE					6
#define	ALARM_TYPE_DISABLE					7


#define	ALARM_TYPE_DOOR_OPEN_TIMEOUT		11

#define ALARM_TYPE_EMERGENCY_HELP			12

#define ALARM_TYPE_GM_INFRARED				20

#define	SA_ALARM_MAX_NUM					8

#define	SA_DEV_NAME							"/dev/mox_security_alarm"

#define	SA_SET_CONFIG_NOTIFY_ENABLE_SA		0
#define	SA_SET_CONFIG_NOTIFY_DISABLE_SA		1
#define	SA_SET_CONFIG_NOTIFY_MODIFY_ZONE	2

#define	ALARM_STATUS_DETECT					0
#define	ALARM_STATUS_ALARM					1
#define	ALARM_STATUS_DETECTED_ALARM			2

#define	ZONE_RESERVED						0

#define	FOLLOW_ALARM_STATUS_INVALID			0
#define	FOLLOW_ALARM_STATUS_OUT				1
#define	FOLLOW_ALARM_STATUS_IN				2


#define SHV_LIST_BUF_LEN					150

#define SAL_CHANGE_NO						0x00
#define SAL_CHANGE_ADD						0x01
#define SAL_CHANGE_READ						0x02
#define SAL_CHANGE_DEL						0x03		

#define SAL_CHANGE_ENABLE					0x04
#define SAL_CHANGE_DISABLE					0x05


#define SAL_STA_ORIGINAL				0x00
#define SAL_STA_WAIT_SALLIST			0x01
#define SAL_STA_WAIT_TEXT				0x02
#define SAL_STA_WAIT_DEL_ACK			0x03
#define SAL_STA_WAIT_CNT				0x04
#define SAL_STA_TIMEOUT					0X10


#define SA_SET_ENABLE					1
#define SA_SET_DISABLE					2

#define	MC_ACK_TIMEOUT					2000
#define	SA_REPORT_TIME				(1000 * 5)


#define ALARM_DELAY_TIMEOUT						1000

#define MAX_ALARM_TYPE					3


//#define ALARM_TYPE_LOCAL				2
//#define ALARM_TYPE_OUTER				3
//#define ALARM_TYPE_CONFIRM_OUTER		4
/************** TYPEDEFS **************************************************************************************/
   
/************** STRUCTURES ************************************************************************************/
typedef	struct tagSAOpenStruct
{
	int					nSARunStatus;
	BOOL				bNeedReport;
	BOOL				bWaitAck;
	BOOL				bAlarmAlter;
	BOOL				bAlarmSigLastSta;
	DWORD				dwDelayTick;
} SASubMan;



typedef	struct tagSAManStruct
{
	int					nSARunStatus;
	DWORD				SAReportTime[6];	//	the time when received the message
	DWORD				dwSendTick;
	SASubMan			SADetailMan[MAX_ALARM_TYPE];
} SAManStruct;






#define	WAIT_MC_ACK_IDX_INVALID			-1
#define	WAIT_MC_ACK_IDX_HIJACK			0x101
#define	WAIT_MC_ACK_IDX_ENABLE			0x102
#define	WAIT_MC_ACK_IDX_DISABLE			0x103

#define	ALARM_RUN_NORMAL				0
#define	ALARM_RUN_TRIGGGER				1

#define	SUB_ALARM_NORMAL				0
#define	SUB_ALARM_ON_DELAY				1
#define	SUB_ALARM_DELAY					2
#define	SUB_ALARM_OFF_DELAY				3
#define	SUB_ALARM_RUN					4



/************** GLOBAL VARIABLE DEFINITIONS *******************************************************************/


/************** EXTERNAL DECLARATIONS *************************************************************************/

extern int				SecurityAlarmInit(void);
extern void				SecurityAlarmExit(void);
extern void				SecurityAlarmProcess(void);

/**************************************************************************************************************/
#endif // SECURITYALARM_H

