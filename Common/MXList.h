/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MXList.h
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

#ifndef MXLIST_H
#define MXLIST_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

// General list structure
typedef	struct _MXList
{
	struct _MXList*				pNext;		// Next Item
	struct _MXList*				pPrev;		// Prev Item
} MXList;

// Genernal list head struct
typedef	struct _MXListHead
{
	MXList*							pHead;		// First Item
	MXList*							pTail;		// Last Item
} MXListHead;

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern void			MXListAdd(MXListHead* pHead, MXList* pItem);
extern void			MXListRm(MXListHead* pHead, MXList* pItem);
extern void			FreeMXListMem(MXListHead* pHead, char* pszDesc);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // MXLIST_H

