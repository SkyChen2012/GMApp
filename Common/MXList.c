/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MXList.c
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
**				MXListAdd	
**				MXListRm
**				
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************/

#include <stdlib.h>
#include <stdio.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXTypes.h"
#include "MXList.h"
#include "MXMem.h"

/************** DEFINES **************************************************************/

//#define FREE_MEM_DEBUG

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MXListAdd
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Add an element to list tail
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pHead		[IN/OUT]	MXListHead*
**				pItem		[IN/OUT]	MXList*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
MXListAdd(MXListHead* pHead, MXList* pItem)
{
	if (pHead->pTail != NULL)
	{
		pItem->pPrev = pHead->pTail;
		pHead->pTail->pNext = pItem;
	}
	else
	{
		pItem->pPrev = NULL;
	}

	pItem->pNext = NULL;
	pHead->pTail = pItem;
	if (NULL == pHead->pHead)
	{
		pHead->pHead = pItem;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MXListRm
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			Remove an element from list
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pHead		[IN/OUT]	MXListHead*
**				pItem		[IN/OUT]	MXList*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
MXListRm(MXListHead* pHead, MXList* pItem)
{
	if (pItem->pNext != NULL)
	{
		pItem->pNext->pPrev = pItem->pPrev;
	}
	if (pItem->pPrev != NULL)
	{
		pItem->pPrev->pNext = pItem->pNext;
	}
	if (pHead->pHead == pItem)
	{
		pHead->pHead = pItem->pNext;
	}
	if (pHead->pTail == pItem)
	{
		pHead->pTail = pItem->pPrev;
	}
	pItem->pNext = pItem->pPrev = NULL;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	FreeMXListMem
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Jun - 2007
**
**	DESCRIPTION:	
**			Free MXList memory
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pHead		[IN/OUT]	MXListHead*
**				pszDesc		[IN]		char*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
FreeMXListMem(MXListHead* pHead, char* pszDesc)
{
	MXList*				pMXList;
	MXList*				pTmp;

	pMXList = pHead->pHead;
	while (pMXList != NULL)
	{
		pTmp = pMXList;
		pMXList = pMXList->pNext;
		MXFree(pTmp);
#ifdef FREE_MEM_DEBUG
		printf("MXFree memory %x", (unsigned int) pTmp);
		if (pszDesc != NULL)
		{
			printf(" (%s)", pszDesc);
		}
		printf("\n");
#endif
	}

	pHead->pHead = NULL;
	pHead->pTail = NULL;
}
