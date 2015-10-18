/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	BacpSerialApp.h
**
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef BACPSERIALAPP_H
#define BACPSERIALAPP_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

#include "Bacp.h"

/************** DEFINES ***************************************************************************************/
#define	BACP_APP_SERIAL_RES_OK			0x0

#define	LEN_BACP_APP_SERIAL_LEN			2
#define	LEN_BACP_APP_SERIAL_CAT			1
#define	LEN_BACP_APP_SERIAL_FC			1
#define	LEN_BACP_APP_SERIAL_RES			1

#define	MIN_LEN_BACP_APP_SERIAL			(LEN_BACP_APP_SERIAL_CAT + LEN_BACP_APP_SERIAL_FC + LEN_BACP_APP_SERIAL_LEN)
#define	MIN_LEN_BACP_APP_SERIAL_RESPONSE	(MIN_LEN_BACP_APP_SERIAL + LEN_BACP_APP_SERIAL_RES)

#define	OFFSET_BACP_APP_SERIAL_CAT		0
#define	OFFSET_BACP_APP_SERIAL_FC		1
#define	OFFSET_BACP_APP_SERIAL_LEN		2
#define	OFFSET_BACP_APP_SERIAL_DATA		4
#define	OFFSET_BACP_APP_SERIAL_RES		4

/************** TYPEDEFS **************************************************************************************/

// App frame basic field
typedef	struct _BacpSerialAppFld
{
	unsigned char				FunCat;
	unsigned char				FunCode;
	unsigned short			Len;
} BacpSerialAppFld;

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern int		PckSerialBacpApp(unsigned char nFunCategory, unsigned char nFunCode, unsigned char* pData, int nDataLen, unsigned char* pFrm);
extern int		UnpckBacpSerialApp(unsigned char* pnFunCategory, unsigned char* pnFunCode, unsigned char* pData, unsigned short* pnDataLen, unsigned char* pFrm, int nFrmLen);
extern int		UnpckBacpSerialAppEx(unsigned char* pnFunCategory, unsigned char* pnFunCode, unsigned char** ppData, unsigned short* pnDataLen, unsigned char* pFrm, int nFrmLen);
extern void	DebugBacpSerialAppFrm(unsigned char* pFrm, int nFrmLen);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // BACPSERIALAPP_H
