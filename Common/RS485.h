
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

#ifndef RS485_H
#define RS485_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"

/************** DEFINES ***************************************************************************************/
#define		RS485_PORT_1		1
#define		RS485_PORT_2		3

#define		LF_SERIAL_INI_FILE	"/mox/rdwr/liftserial.ini"
#define		LF_SERIAL_INI_FILE1	"/mox/rdwr/liftserial1.ini"

#define		SEC_LCA				"LCA"
#define		KEY_COMPORT			"COM"
#define		DEFAULT_COMPORT		RS485_PORT_2

#define		KEY_BAUDRATE		"baudrate"
#define		DEFAULT_BAUDRATE	9600

#define		KEY_DATABIT			"databit"
#define		DEFAULT_DATABIT		8			

#define		KEY_PARITY			"parity"
#define		DEFAULT_PARITY		2			//0 means none, 1 means Odd parity, 2 mean even parity

#define		KEY_STOPBIT			"stopbit"
#define		DEFAULT_STOPBIT		1			// 0 means 1, 1 means 1.5 , 2 means 2

#define		PARITY_NONE	0
#define		PARITY_ODD	1
#define		PARITY_EVEN	2


/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/


/************** EXTERNAL DECLARATIONS *************************************************************************/

extern VOID Rs485Init(VOID);
extern VOID	Write485Data(CHAR* pOutBuf, unsigned int Length);
extern int Read485Data(unsigned char* pInBuf, unsigned int Length);

extern VOID	Write485Data_temp(CHAR* pOutBuf, unsigned int Length);

extern VOID WriteMox485Data(CHAR* pOutBuf, unsigned int Length);
extern int ReadMox485Data(unsigned char* pInBuf, unsigned int Length);


/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // RS485_H
