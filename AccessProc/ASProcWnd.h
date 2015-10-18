
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	ASProcWnd.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		08 - Sep - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef ASPROCWND_H
#define ASPROCWND_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/


/************** DEFINES ***************************************************************************************/

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

extern void CreateASCtrlWnd(HWND hwndParent);
extern HWND g_hWndAS;
extern HWND GetASProcWnd(void);

/**************************************************************************************************************/
#endif // ASPROCWND_H
