/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	DelRdModeWnd.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		27 - April - 2009
**
**	FILE DESCRIPTION:
**  Delete Card by its Resident's Number				
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

typedef enum _DELRDMODESTEP
{
	STEP_ETR_RDNUM = 1,
	STEP_DEL_SUCCESS,
	STEP_DEL_FAIL,
	STEP_CONFIRM,
	STEP_DEL_NO_CARD
}DELRDMODESTEP;
/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

extern void CreateDelRdModeWnd(HWND hwndParent);

/************** ENTRY POINT DECLARATIONS *********************************************/
