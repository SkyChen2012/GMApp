/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SAAlarmWnd.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		27 - Nov - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef SAALARMWND_H
#define SAALARMWND_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/

#define ALARM_TYPE_TAMPER				0
#define ALARM_TYPE_OPEN_OVERTIME		1
#define ALARM_TYPE_INFRARED				2
//#define ALARM_INVALID_CARD				3



#define ALA_SHOW_TAMPER                  0
#define ALA_SHOW_OPEN                  	 1
#define ALA_SHOW_INFRARED	             2
#define ALA_SHOW_TAMPER_AND_OPEN         3
#define ALA_SHOW_TAMPER_AND_INFRARED     4
#define ALA_SHOW_OPEN_AND_INFRARED		 5
#define ALA_SHOW_ALL					 6
#define ALA_WND_CONFIRM                  7
#define ALA_WND_DESTROY                  8


/************** TYPEDEFS **************************************************************************************/
#define PSW_KEY_ORIGIN_X		550
#define PSW_KEY_ORIGIN_Y		80

#define PSW_TITLE_LEFT_SET		30
#define PSW_MAX_LEN				300
/************** STRUCTURES ************************************************************************************/

/************** GLOBAL VARIABLE DEFINITIONS *******************************************************************/

//!!!  It is C/C++ file specific, nothing should be defined here

/************** EXTERNAL DECLARATIONS *************************************************************************/
extern HWND	g_HwndAsAlarm;

//extern	HWND		CreateSAAlarmWnd(HWND hwndParent, UINT nParam);
extern void			CreateAsAlarmWnd(int nAlaType);

extern void CloseSAConfirmWnd(void);
extern void UpdateAlarmConfirmWnd(int Type);


/**************************************************************************************************************/
#endif // SAALARMWND_H

