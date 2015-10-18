/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AMT.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		25 - Dec - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	
**
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES *****************************************/


/************** USER INCLUDE FILES *********************************************/
#include "MXTypes.h"
#include "BacpNetCtrl.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS ***********************************************************/

typedef struct
{      
	UCHAR	SettingType;
	WORD	SettingTypeCount;
	UCHAR	OffSet1Addr[4];
	UCHAR	OffSetNAddr[4];
}ATM_HEAD_V4;

typedef struct
{
	UCHAR	PreAddress[4];
	UCHAR	NexAddress[4];
	UCHAR	DevCode[19];
	UCHAR   DevIP[4];
	UCHAR	DevPhyAddr[3];
	UCHAR	Name[20];
	UCHAR	Reserved[4];
}AMT_Type800;

typedef struct
{
	UCHAR	PreAddress[4];
	UCHAR	NexAddress[4];
	UCHAR	DevCode[19];
	UCHAR	IPCOUNT;
	DWORD   pDevIP[20];
	UCHAR	Name[20];
	SHORT	Level;
	UCHAR	Reserved[2];
}AMT_Type900;

typedef struct
{
	UCHAR	PreAddress[4];
	UCHAR	NexAddress[4];
	UCHAR	DevCode[20];
	UCHAR   DevIP[4];
	UCHAR	Name[20];
	UCHAR	Reserved[4];
}AMT_TypeMC;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here
extern void WriteData2AMT(UCHAR *pBuf, INT nBufLen);
extern void FdFromAMTResolv(EthResolvInfo *pResolvInfo);
extern void LoadAMT();
extern BYTE *AMTBuf;

/************** ENTRY POINT DECLARATIONS *********************************************/





