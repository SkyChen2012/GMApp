/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Multimedia.h
**
**	AUTHOR:		Betty Gao
**
**	DATE:		19 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef FM1182_H
#define FM1182_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

//Nothing should be included here

/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/


/************** TYPEDEFS **************************************************************************************/
   
/************** STRUCTURES ************************************************************************************/


/************** GLOBAL VARIABLE DEFINITIONS *******************************************************************/

//!!!  It is C/C++ file specific, nothing should be defined here

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern int	SendtoFM1182(UCHAR* pBuffer, int nLen);
extern void	SetFM1182SpkVolume(USHORT nVolume, int nMode);
extern void	SetMicVolForHdb();
extern void	RetMicVolForHdb();
extern int	ReadFM1182Reg(USHORT nAddr, USHORT* nValue);

/**************************************************************************************************************/
#endif // FM1182_H
