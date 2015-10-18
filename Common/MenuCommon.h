/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MenuCommon.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		28 - May - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef MENUCOMMON_H
#define MENUCOMMON_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/

#include "MXCommon.h"

/************** DEFINES ***************************************************************************************/
#define MENU_LEN 50

#define MENU_STATE_NORMAL			0
#define MENU_STATE_DISABLE			1
#define MENU_STATE_SELECT			2
#define MENU_STATE_VISIBLE				4
#define MENU_STATE_DEFAULT 			(MENU_STATE_VISIBLE|MENU_STATE_NORMAL)
#define MENU_STATE_DEFAULT_SELECT	(MENU_STATE_DEFAULT|MENU_STATE_SELECT)


/************** TYPEDEFS **************************************************************************************/

typedef struct _MENUINFO
{
	BOOL bSelected;
	HEBREW_STR_ID	HebrewStrID;
	CHAR MenuEnglishName[MENU_LEN];
	CHAR MenuChineseName[MENU_LEN];
	VOID (*Menufun)(); //Function of selected menu, may create new menu or execute menu funciton
}MENUINFO;

typedef struct _MENUINFOEX
{
	INT  iID;
	UINT iState;
	HEBREW_STR_ID	HebrewStrID;
	CHAR MenuEnglishName[MENU_LEN];
	CHAR MenuChineseName[MENU_LEN];
	VOID (*Menufun)(); //Function of selected menu, may create new menu or execute menu funciton
}MENUINFOEX;


typedef enum _MENUENTERSTEP
{
	MENU_INPUT_PASSWORD = 1,
	MENU_PASSWORD_ERROR,
	MENU_ENTER_MENU
}MENUENTERSTEP;

/************** STRUCTURES ************************************************************************************/



/************** EXTERNAL DECLARATIONS *************************************************************************/
extern void KillAllChildWnd(HWND hWnd);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // MENUCOMMON_H
