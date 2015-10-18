/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	LCAPro_mitsubishi.h
**
**	AUTHOR:		Wayde Zeng
**
**	DATE:		9 - March - 2011
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef LCAPRO_MITSUBISHI_H
#define LCAPRO_MITSUBISHI_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/

#define LEN_MIT_APP_LY_FC		1
#define	LEN_MIT_APP_LY_INDEX	1	

#define MIT_PACK_STX			0x02
#define LEN_MIT_STX				1
#define	LEN_MIT_BNK				1
#define	LEN_MIT_NOD				1
#define LEN_MIT_DATALEN			1
#define LEN_MIT_CHK				1
#define LEN_MIT_ETX				1
#define MIT_PACK_ETX			0x03
#define MIN_MIT_DATA_LEN		6

#define BANK_MAXSIZE			8
#define CARDREADER_MAXSIZE			15
#define MAX_FLOOR_NUM			256
#define MIT_BIGFAULT_COUNT		4

#define MIT_WAIT_ACK_TIME		200
#define MIT_BIGFAULT_TIME		1000
#define MIT_LIFTSTATE_TIME		4000
#define MIT_HEARTBEAT_TIME		5000

#define MIT_PROTOCOL_INCAR_A	0x09
#define MIT_PROTOCOL_INCAR_M	0x0A
#define MIT_PROTOCOL_A_B		0x01
#define MIT_PROTOCOL_HEARTBEAT	0x11
#define MIT_PROTOCOL_STATE_ACK	0x91

#define LFCONFIG_FILE						"/mox/rdwr/LFConfig.ini"//电梯信息(bank nod)
#define LFCONFIG_SECTION					"LFConfig"
#define LFCONFIG_KEY_BANK_COUNT				"BankCnt"
#define LFCONFIG_KEY_SWPL_FLAG				"SCPLFlag"
#define LFCONFIG_KEY_CARDREADER_COUNT		"CardReader"
#define LFCONFIG_KEY_BANK					"bank"
#define LFCONFIG_KEY_STATIONNUM				"stationnum"
#define LFCONFIG_KEY_NODE					"node"



/************** STRUCTURES ************************************************************************************/

// configuration

typedef struct _LTCARDREADER
{
	BYTE 	bBank;
	BYTE	bNode;
	BYTE	bStationNum;
}LTCARDREADER;


typedef struct _LTCONFIG
{
	BYTE bank[BANK_MAXSIZE];
	int banksize;
	LTCARDREADER	CardReader[CARDREADER_MAXSIZE];
	int CardReaderSize;
	BOOL bSCPLFlag;//同一个门刷不同的卡招不同的电梯
}LTCONFIG;

// bank state check
typedef struct _BANKCHECKSTATE
{
	int bankstate[BANK_MAXSIZE];
	int checkpos;
	DWORD FaultTick[BANK_MAXSIZE];
}BANKCHECKSTATE;

typedef struct _PCVA
{
	CHAR AFloor;
	CHAR VFloor;
}PCVA;

typedef struct _VAFLOOR
{
	int versionnum;
	int	nCount;
	int nPCVALen;
	PCVA * pPCVA;
}VAFLOOR;

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern VOID LCAMitProtoInit(VOID);
extern VOID	LCAMitThreadFun(VOID);
extern VOID LCAMitProtoExit(VOID);

extern VOID LCAMitWaitingAckPro(VOID);
extern LTCARDREADER * GetLiftInfoByStationNum(BYTE bStationNum);
extern BOOL IsMitSCPLFlag(void);
/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif //LCAPRO_MITSUBISHI_H






















