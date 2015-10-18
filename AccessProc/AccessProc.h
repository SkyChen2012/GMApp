
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AccessProc.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		13 - April - 2009
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef ACCESSPROC_H
#define ACCESSPROC_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"
#include "MXCommon.h"
#include "AccessCommon.h"

/************** DEFINES ***************************************************************************************/

#define AST_VERSION_LEN		4
#define AST_FUNVLD_LEN		1

#define MODE_ROOMCODE_PASSWORD_SET		0x01
#define MODE_ROOMCODE_PASSWORD_CLR		0xFE

#define MODE_CARD_ONLY_SET				0x02
#define MODE_CARD_ONLY_CLR				0xFD

#define MODE_CARD_PASSWORD_SET			0x04
#define MODE_CARD_PASSWORD_CLR			0xFB

#define GATE_STA_TIME					200

#define ST_ACC_ORIGINAL					0x00000000
#define ST_ACC_WAIT_SWIPECARD_ACK		0x00000001

#define ST_ACC_RDPWD_ORIGINAL			0x00000005
#define ST_ACC_WAIT_RDPASSWORD_ACK		0x00000006
#define ST_ACC_WAIT_CHECKRDPWD_ACK		0x00000007
#define ST_ACC_ENTERRDNEWPWD			0x00000008
#define ST_ACC_WAIT_MODRDPWD_ACK		0x00000009
#define ST_ACC_CSNPWD_ORIGINAL			0x0000000A
#define ST_ACC_WAIT_CSNPWD_ACK          0x0000000B

#define ST_ACC_UNLOCK_ORIGINAL			0x00000010
#define ST_ACC_WAIT_UNLOCK_ACK			0x00000011

#define UNLOCKTYPE_PATROL				1
#define UNLOCKTYPE_MANUAL				2
#define UNLOCKTYPE_TALK					3
#define UNLOCKTYPE_CARDPWD				4
#define UNLOCKTYPE_RDPWD				5
#define UNLOCKTYPE_FIREALM				6


/************** TYPEDEFS **************************************************************************************/

typedef enum _ASWORKSTATUS 
{
	STATUS_OPEN = 1,
	STATUS_PWDMOD,
	STATUS_ADD_CARD,
	STATUS_ADD_PATROL_CARD,
	STATUS_ADD_AUTHORIZE_CARD,
	STATUS_DEL_CARD_MODE,
	STATUS_DEL_LOCALNUM_MODE,
	STATUS_DEL_RD_MODE,
	STATUS_GATEOPEN_OVERTIME_ALARM,
	STATUS_IVDCARD_SWIPE_ALARM,
	STATUS_INFRARED_ALARM,
	STATUS_CARD_STOP,
	STATUS_DEL_CARD,
	STATUS_DEL_PATROL_CARD,
	STATUS_DEL_AUTHORIZE_CARD,
}ASWORKSTATUS;

typedef struct _ASINFO
{
	ASWORKSTATUS	ASWorkStatus;
	ASTHEAD			ASTHead;
	CHAR*			MXASBuf;
}ASINFO;

typedef struct _ACCINFO
{
	char	szACCCode[RD_CODE_LEN];
	char	szACACode[RD_CODE_LEN];
	DWORD	dwACCState;
	DWORD	dwACAIP;	
	DWORD	dwTickCount;
	BYTE	ACCData[256];
}ACCINFO;

typedef struct _CSNPWDINFO
{
	char	szACCCode[RD_CODE_LEN];
	char	szACACode[RD_CODE_LEN];
	DWORD	dwACCState;
	DWORD	dwACAIP;	
	DWORD	dwTickCount;
	char	szCallCode[RD_CODE_LEN];
}CSNPWDINFO;

typedef struct _RDPWDINFO
{
	char	szACCCode[RD_CODE_LEN];
	char	szACACode[RD_CODE_LEN];
	DWORD	dwACCState;
	DWORD	dwACAIP;	
	DWORD	dwTickCount;
	char	szCallCode[RD_CODE_LEN];
}RDPWDINFO;

typedef struct _UnlockINFO
{
	char	szACCCode[RD_CODE_LEN];
	char	szACACode[RD_CODE_LEN];
	DWORD	dwACCState;
	DWORD	dwACAIP;	
	DWORD	dwTickCount;
}UnlockINFO;

/************** STRUCTURES ************************************************************************************/


/************** EXTERNAL DECLARATIONS *************************************************************************/

extern void AccessProc();
extern void AccessInit();
extern void AccessExit();

extern VOID SendUnlockGate2ACC(BYTE byUnlockType);

extern ASINFO	g_ASInfo;
extern ASINFO	g_APInfo;
extern ASINFO	g_AAInfo;
extern ACCINFO	g_ACCInfo;
extern BOOL     g_bTamperAlarmOn;
extern ASINFO	g_AS_HVInfo;

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // ACCESSPROC_H
