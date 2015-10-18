/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AddResiWnd.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		21 - Aug - 2008
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

#ifndef ADDCARDWND_H
#define ADDCARDWND_H

/************** SYSTEM INCLUDE FILES **************************************************/



/************** USER INCLUDE FILES *********************************************/


/************** DEFINES **************************************************************/

#define		MAX_CARD_ID		99999
#define		MIN_CARD_ID		0

#define CD_LOCALNUM_LEN										5+1

/************** TYPEDEFS *************************************************************/

typedef enum _ADDCARDSTEP
{
	STEP_ADD_CSN = 1,
		STEP_ADD_CARD_REGISTERED,
		STEP_ADD_LOCAL_NUM,
		STEP_ADD_LOCAL_NUM_REGISTERED,
		STEP_ADD_RD_NUM,
		STEP_ADD_RD_NUM_REGISTERED,
		STEP_ADD_VLDSTART_TIME,
		STEP_ADD_VLDEND_TIME,
		STEP_ADD_SUCCESS,
		STEP_ADD_FAIL,
}ADDCARDSTEP;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

extern VOID CreateAddCardWnd(HWND hwndParent);
extern VOID ShowAddCardWnd(BYTE* pCSN, BYTE CardMode);
extern DWORD ConvertStr2ID(CHAR *pIntStr);


/*************************************************************************************/
#endif //ADDCARDWND_H