/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	TelnetLog.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		19 - Nov - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef TELNETLOG_H
#define TELNETLOG_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"

/************** DEFINES ***************************************************************************************/

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern	void	TelLogInput(MXMSG *pMsg);
extern	void	TelInputCSN(BYTE* CSN, int nCardLen);
extern	void	TelSrlData(UCHAR* pSrlData, INT nDataLen);

extern	VOID TelLogStr(CHAR* pLogData);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // TELNETLOG_H
