
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	ACAgentProc.h
**
**	AUTHOR:		WaydeZeng
**
**	DATE:		28 - February - 2011
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef ACAGENTPROC_H
#define ACAGENTPROC_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"
#include "MXCommon.h"
#include "AccessCommon.h"

/************** DEFINES ***************************************************************************************/


/************** TYPEDEFS **************************************************************************************/


/************** STRUCTURES ************************************************************************************/

typedef struct _UNLOCKCTRL
{
	DWORD dwUnlockState;
	DWORD dwTickCount;
}UNLOCKCTRL;

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern VOID ACAgentInit(VOID);
extern VOID ACAgentProc(VOID);
extern VOID ACAgentExit(VOID);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // ACAGENTPROC_H
