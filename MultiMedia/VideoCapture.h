/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Multimedia.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		17 - Jun - 2009
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef VIDEOCAPTURE_H
#define VIDEOCAPTURE_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/
#define	MAX_VIDEOCAP_DATA_SIZE				(300 * 1024)	//(150 * 1024)


/************** TYPEDEFS **************************************************************************************/


/************** STRUCTURES ************************************************************************************/


/************** GLOBAL VARIABLE DEFINITIONS *******************************************************************/

//!!!  It is C/C++ file specific, nothing should be defined here

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern INT	GetVCapData(BYTE* pOutData);
extern VOID	MmVideoInit();
extern void MmVideoStart(void);
extern void MmVideoStop(void);
/**************************************************************************************************************/
#endif // VIDEOCAPTURE_H





















