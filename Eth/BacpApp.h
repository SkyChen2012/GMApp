/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	BacpApp.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		10 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef BACPAPP_H
#define BACPAPP_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

#include "Bacp.h"

/************** DEFINES ***************************************************************************************/


#define	LEN_BACP_APP_LY_LEN			2
#define	LEN_BACP_APP_LY_FC			2
#define	LEN_BACP_APP_LY_SRC_DEV		MAX_LEN_DEV_CODE
#define	LEN_BACP_APP_LY_DEST_DEV	MAX_LEN_DEV_CODE

#define	MIN_LEN_BACP_APP_LAY		(LEN_BACP_APP_LY_LEN + LEN_BACP_APP_LY_FC + LEN_BACP_APP_LY_SRC_DEV + LEN_BACP_APP_LY_DEST_DEV)

#define	OFFSET_BACP_APP_LY_FC		0
#define	OFFSET_BACP_APP_LY_LEN		2
#define	OFFSET_BACP_APP_LY_SRC_DEV	4
#define	OFFSET_BACP_APP_LY_DEST_DEV	23
#define	OFFSET_BACP_APP_LY_DATA		42

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern int		PckBacpApp(unsigned short nFunCode, char* pSrcDev, char* pDestDev, unsigned char* pData, int nDataLen, unsigned char* pFrm);
extern int		UnpckBacpApp(unsigned short* pnFunCode, char* pSrcDev, char* pDestDev, unsigned char* pData, int* pnDataLen, unsigned char* pFrm, int nFrmLen);
extern int		UnpckBacpAppEx(unsigned short* pnFunCode, char** ppSrcDev, char** ppDestDev, unsigned char** ppData, int* pnDataLen, unsigned char* pFrm, int nFrmLen);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // BACPAPP_H
