/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	DelLocalNumModeWnd.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		25 - Aug - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	
**
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************/



/************** USER INCLUDE FILES ***************************************************/


/************** DEFINES **************************************************************/


/************** TYPEDEFS *************************************************************/

typedef enum _DELLOCALNUMMODESTEP
{
	STEP_ETR_LOCALNUM = 1,
	STEP_LOCALNUM_NOTEXIST,
	STEP_CONFIRM,
	STEP_DEL_SUCCESS
}DELLOCALNUMMODESTEP;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

extern void CreateDelLocalNumModeWnd(HWND hwndParent);

/************** ENTRY POINT DECLARATIONS *********************************************/
