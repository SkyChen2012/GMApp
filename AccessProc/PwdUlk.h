
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	PwdUlk.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		07 - Apr - 2009
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef PWDULK_H
#define PWDULK_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"
#include "MXCommon.h"

/************** DEFINES ***************************************************************************************/

#define AST_VERSION_LEN		4
#define AST_FUNVLD_LEN		1

#define MODE_ROOMCODE_PASSWORD_SET		0x01
#define MODE_ROOMCODE_PASSWORD_CLR		0xFE

#define MODE_CARD_ONLY_SET				0x02
#define MODE_CARD_ONLY_CLR				0xFD

#define MODE_CARD_PASSWORD_SET			0x04
#define MODE_CARD_PASSWORD_CLR			0xFB

#define PUT_RDCODE_LEN	19

/************** TYPEDEFS **************************************************************************************/

typedef enum _PUSTATUS
{
	STATUS_PASSWORD_MODIFY = 1,
	STATUS_UNLOCK_DOOR
}PUSTATUS;

typedef struct _PUTHEAD 
{
	DWORD			VersionNum;
	DWORD			nResiCnt;
	DWORD			nCdPwdLen;
}PUTHEAD;

typedef struct _PUINFO
{
	PUTHEAD			PUTHead;
	CHAR*			MXPUBuf;
}PUINFO;

typedef struct _RDINFO 
{
	BYTE			bPwdCfg;
	char			RdCode[RD_CODE_LEN];
	char			RdPwd[PWD_LEN];
}RDINFO;

/************** STRUCTURES ************************************************************************************/


/************** EXTERNAL DECLARATIONS *************************************************************************/

extern VOID PwdUlkProc();

extern VOID PwdUlkInit();

extern PUINFO	g_PUInfo;

extern BOOL IsRdExist(CHAR *pRdCode, INT nRdLen);

extern VOID LoadRdInfoFromMem();
extern VOID SaveRdInfo2Mem();

extern RDINFO* AsGetRdInfobyRd(CHAR *pRdCode, INT RdCodeLen);
extern BOOL AsRmRdByRd(CHAR *pRdCode, INT RdCodeLen);
extern VOID UpdateRdfromAMT();
extern VOID UpdateRdDefaultPwd();

extern RDINFO*	PUGetRdInfobyRd(CHAR *pRdCode, INT RdCodeLen);
extern BOOL	PUPasswordCompare(CHAR *pInPwd,  INT InCodeLen);

extern BOOL CodeCompare(CHAR *pInCode, CHAR *pTableCode, INT InCodeLen);

extern INT FdEHVfromAMT(char ** ppPUTBuffer);

extern BOOL PUGetPwdbyRd(CHAR *PwdStr,CHAR *pRdCode, INT RdCodeLen);


/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // PWDULK_H
