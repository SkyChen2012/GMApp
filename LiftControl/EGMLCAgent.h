/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	EGMLCAgent.h
**
**	AUTHOR:		Harry Qian
**
**	DATE:		22 - Jul - 2010
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef EGMLCAGENT_H
#define EGMLCAGENT_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/

#define MAX_SERIAL_BUF_LEN		1024

/************** TYPEDEFS **************************************************************************************/
#define LIFTCONTROL_NONE	0
#define LIFTCONTROL_UP	1
#define LIFTCONTROL_DOWN	2

#define MIN_GROUND_LEVEL	-8
#define MAX_GROUND_LEVEL	55

#define LEVEL_NUMBER_FORMAT	0
#define LEVEL_IP_FORMAT		1
#define LEVEL_CODE_FORMAT	2
#define LEVEL_CSN_FORMAT	3

#define BACPSERIALAPP_TO_VALUE	3000

#define ALT_FILE		"/mox/rdwr/ALT"
#define CLT_FILE		"/mox/rdwr/CLT"

#define MAX_LF_CARD_CNT	50000
#define MAX_LF_RD_CNT	10000

#define LCA_STATUS_IDLE				0X00
#define LCA_STATUS_WAITING_ACK		0x01

#define LC_INFO_VERSION_OFFSET      4
#define LC_INFO_COUNT_OFFSET        4
#define LC_INFO_CARD_LENGTH_OFFSET  4

#define LC_DEBUG

#define LIFT_FRONT_DOOR		0
#define LIFT_BACK_DOOR		1
#define LIFT_FB_DOOR		2

#define CALL_LIFT_METHOD_DEF		0
#define CALL_LIFT_METHOD_DOORDOOR	1
#define CALL_LIFT_METHOD_LIFTUNLOCK	2//轿厢解锁
/************** STRUCTURES ************************************************************************************/

// General list structure
/************** STRUCTURES ************************************************************************************/
typedef struct _ALCTFileListInfo
{
	BYTE	CSN[CSN_LEN];
	unsigned char GroundNum;
	unsigned char	LowLevels[4];
	unsigned char	HighLevels[4];
}ALCTFileListRecord;

typedef struct _LevelInfo
{
	unsigned char format;
	signed char 	level;
}LevelInfo;


typedef struct _LiftStatsIndo
{
	DWORD  dwIP;	// the ip of the device wants to show the lift state

}LIFTSTATS;

typedef struct LCAProtocolMan
{
	int		nStatus;
	int		nComPort;
	BOOL	bNeedRsp;
	DWORD	dwTick;
	DWORD	nMsg;
	DWORD	dwIP;
	unsigned char Seq;
	char	SA[3];
	char	DA[3];
}LCAPROTOCOL;

typedef struct _LiftCtrlCMDInfo
{
	BOOL			bValid;
	int				nCmdId;
	char							szSrcDev[MAX_LEN_DEV_CODE + 1];
	char							szDestDev[MAX_LEN_DEV_CODE + 1];
	DWORD			dwIP;	// the ip of the device wants to show the lift state
	UINT			usCMD;
	unsigned char * pData;
	int				nDataLen;
	DWORD			dwTimeStart;
}LCCMD;

typedef struct _LFCard
{
	BYTE CSN[5]	;
	char Code[20]	;
	BYTE bank	;
	BYTE node	;
	BYTE byDoorType	;
	CHAR byDestFloor	;
	BYTE byFrontDoorLen	;
	BYTE byBackDoorLen	;
	unsigned long long dlFrontUnlockLevel	;
	unsigned long long dlBackUnlockLevel	;
	BYTE bAdmin	;
}PACKED LFCard ;

typedef struct _LCINFO
{
	int versionnum;
	int	nCount;
	int nCardInfoLen;
	LFCard * pLFCard;
}LCINFO;

typedef struct _CodeInfoV0
{
	char Code[20];
	BYTE bank;
	BYTE node;
	BYTE byFloor;
	BYTE byFBDoor;//前后门
}CodeInfo_V0;

typedef struct _CodeInfoV1
{
	char Code[20];
	BYTE bank;
	BYTE node;
	BYTE byFloor;
	BYTE byFBDoor;//前后门
	BYTE byMethod;
}CodeInfo_V1,CodeInfo;

typedef struct _CODELIFT
{
	int versionnum;
	int	nCount;
	int nCodeInfoLen;
	CodeInfo * pCodeInfo;
}CODELIFT;

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern LCAPROTOCOL	g_LCAProtocol;
extern VOID LoadLCInfo(VOID);

extern void		EGMLCAMdInit(void);
extern void		EGMLCAFuncProc(void);
extern void		EGMLCAMdExit(void);
extern  LFCard * FindLFCardByCSN(BYTE * szCSN);
/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif //






















