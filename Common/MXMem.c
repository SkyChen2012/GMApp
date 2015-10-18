/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Dspch.c
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
**				MXAlloc		
**				
**
**				
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************/

#include <stdlib.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMem.h"

/************** DEFINES **************************************************************/

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
**	FUNCTION NAME:	MXAlloc
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			alloc memory
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nSize		[IN]		unsigned int
**	RETURNED VALUE:	
**				memory pointer that alloced
**	NOTES:
**			
*/
void*
MXAlloc(unsigned int nSize)
{
	return (void*) calloc(nSize, 1);
}

