
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	CardRead.h
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

#ifndef CARDREAD_H
#define CARDREAD_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"

/************** DEFINES ***************************************************************************************/

#define	LEN_SERIAL_STX				2
#define	LEN_SERIAL_VER				1
#define	LEN_SERIAL_LEN				2
#define	LEN_SERIAL_SEQ				1
#define	LEN_SERIAL_OPT				1
#define	LEN_SERIAL_DA1				1
#define	LEN_SERIAL_DA2				1
#define	LEN_SERIAL_DA3				1
#define	LEN_SERIAL_SA1				1
#define	LEN_SERIAL_SA2				1
#define	LEN_SERIAL_SA3				1
#define	LEN_SERIAL_APP				
#define	LEN_SERIAL_CHK				2
#define	LEN_SERIAL_ETX				2

#define MIN_SERIAL_PCKLEN			17
#define MAX_SERIAL_PCKLEN			29

#define	OFFSET_SERIAL_STX				0
#define	OFFSET_SERIAL_VER				2
#define	OFFSET_SERIAL_LEN				3
#define	OFFSET_SERIAL_SEQ				5
#define	OFFSET_SERIAL_OPT				6
#define	OFFSET_SERIAL_DA1				7
#define	OFFSET_SERIAL_DA2				8
#define	OFFSET_SERIAL_DA3				9
#define	OFFSET_SERIAL_SA1				10
#define	OFFSET_SERIAL_SA2				11
#define	OFFSET_SERIAL_SA3				12
#define	OFFSET_SERIAL_APP				13
#define	OFFSET_SERIAL_FUNCOD			14
#define	OFFSET_SERIAL_APPLEN			15
#define	OFFSET_SERIAL_CHK
#define	OFFSET_SERIAL_ETX		

#define LEN_APP_FUNCCAT					1
#define LEN_APP_FUNCCOD					1
#define LEN_APP_LENGTH					2
#define LEN_APP_DATA		

#define MIN_APP_LENGTH					4

#define OFFSET_APP_FUNCCAT				0
#define OFFSET_APP_FUNCCOD				1
#define OFFSET_APP_LENGTH				2
#define OFFSET_APP_DATA					4

#define SERIAL_START					0X5434
#define SERIAL_END						0X3454

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

// App Packet 
typedef struct _AppPacket
{
	BYTE				FuncCat;						// Function Category
	BYTE				FuncCod;						// Function Code
	WORD				Length;							// Length
	BYTE*				Data;							// DATA
}AppPacket;

// Serial Packet
typedef	struct _SerialPacket
{
	WORD				Stx;							// STX
	BYTE				Ver;							// VER
	WORD				Len;							// LEN
	BYTE				Seq;							// SEQ
	BYTE				Opt;							// OPT
	BYTE				Da1;							// DA1
	BYTE				Da2;							// DA2
	BYTE				Da3;							// DA3
	BYTE				Sa1;							// SA1
	BYTE				Sa2;							// SA2
	BYTE				Sa3;							// SA3
	AppPacket			App;							// APP
	WORD				Chk;							// CHK
	WORD				Etx;							// ETX
} SerialPacket;


/************** EXTERNAL DECLARATIONS *************************************************************************/

extern VOID	  ReadCardInit(VOID);
extern INT	  ReadCSN(CHAR* pOutBuf, BYTE* pCardMode);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // CARDREAD_H
