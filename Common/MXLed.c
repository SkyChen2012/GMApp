/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MXLed.c
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		23 - Nov - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**						
**				
**
**				
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************/

#include <windows.h>
#include <stdlib.h>

/************** USER INCLUDE FILES ***************************************************/

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
**	FUNCTION NAME:	LightLed
**	AUTHOR:			Jerry Huang
**	DATE:			23 - Nov - 2006
**
**	DESCRIPTION:	
**			Light LED
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nKeyValue	[IN]		UINT
**				bLight		[IN]		BOOL
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
LightLed(UINT nKeyValue, BOOL bLight)
{
	if ((nKeyValue >= VK_F1) && (nKeyValue <= VK_F10))
	{
		if (bLight)
		{
			MoxKbLed((nKeyValue - VK_F1 + 1), 1);
		}
		else
		{
			MoxKbLed((nKeyValue - VK_F1 + 1), 0);
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LightMsgLed
**	AUTHOR:			Jerry Huang
**	DATE:			23 - Nov - 2006
**
**	DESCRIPTION:	
**			Light message LED
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				bLight		[IN]		BOOL
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
LightMsgLed(BOOL bLight)
{
	if (bLight)
	{
		MoxKbLed(11, 1);
	}
	else
	{
		MoxKbLed(11, 0);
	}
}

