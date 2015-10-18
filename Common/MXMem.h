/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Dspch.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		25 - Sep - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef MXMEM_H
#define MXMEM_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/

#define		MXNew(type)		((type*) MXAlloc(sizeof (type)))
#define		MXFree(ptr)		free((void*) ptr)

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern void*				MXAlloc(unsigned int nSize);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // MXMEM_H

