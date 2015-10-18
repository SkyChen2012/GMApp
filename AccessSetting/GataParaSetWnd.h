/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	GateParaSetWnd.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		22 - Aug - 2008
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

typedef enum _TIMEALARMSTEP
{
	STEP_MAIN_WND = 1,
	STEP_TIME_SET,
	STEP_TIME_SET_PROMPT,
	STEP_ALARM_SET,
	STEP_ALARM_SET_PROMPT,
	STEP_CONTACTOR_SET,
	STEP_CONTACTOR_SET_PROMPT
}TIMEALARMSTEP;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

extern void CreateGateParaSetWnd(HWND hwndParent);

/************** ENTRY POINT DECLARATIONS *********************************************/



/*************************************************************************************/
