/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	NumberPanel.h
**
**	AUTHOR:		Harry Qian
**
**	DATE:		13 - Nov - 2007
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef NUMBERPANEL_H
#define NUMBERPANEL_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

#include <windows.h>

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/





#define NUMBER_MENU_CNT 12


#define IDC_NUMBER_0						"number/number_0.bmp"
#define IDC_NUMBER_1						"number/number_1.bmp"
#define IDC_NUMBER_2						"number/number_2.bmp"
#define IDC_NUMBER_3						"number/number_3.bmp"
#define IDC_NUMBER_4						"number/number_4.bmp"
#define IDC_NUMBER_5						"number/number_5.bmp"
#define IDC_NUMBER_6						"number/number_6.bmp"
#define IDC_NUMBER_7						"number/number_7.bmp"
#define IDC_NUMBER_8						"number/number_8.bmp"
#define IDC_NUMBER_9						"number/number_9.bmp"
#define IDC_NUMBER_DEL						"number/number_clear.bmp"
#define IDC_NUMBER_CLEAR					"number/number_clearall.bmp"


#define NUMBER_0_ID			0
#define NUMBER_1_ID			1
#define NUMBER_2_ID			2
#define NUMBER_3_ID			3
#define NUMBER_4_ID			4
#define NUMBER_5_ID			5
#define NUMBER_6_ID			6
#define NUMBER_7_ID			7	
#define NUMBER_8_ID			8
#define NUMBER_9_ID			9
#define NUMBER_CL_ID		10		//clear all
#define NUMBER_DL_ID		11		//delete one






#define NUMBER_0_X		0
#define NUMBER_0_Y		210

#define NUMBER_1_X		0
#define NUMBER_1_Y		140

#define NUMBER_2_X		70
#define NUMBER_2_Y		140

#define NUMBER_3_X		140
#define NUMBER_3_Y		140

#define NUMBER_4_X		0
#define NUMBER_4_Y		70

#define NUMBER_5_X		70
#define NUMBER_5_Y		70

#define NUMBER_6_X		140
#define NUMBER_6_Y		70

#define NUMBER_7_X		0
#define NUMBER_7_Y		0

#define NUMBER_8_X		70
#define NUMBER_8_Y		0

#define NUMBER_9_X		140
#define NUMBER_9_Y		0

#define NUMBER_DL_X		140//570		//CANCEL
#define NUMBER_DL_Y		210

#define NUMBER_CL_X		70//	640		//CONFIRM
#define NUMBER_CL_Y		210

#define NUMBER_OFFSET		5

#define NUMBER_LONG			65	


typedef	void (*NumProc)(HWND hWnd, int nKey);

typedef	struct _NUMBERMODULE
{
	BOOL	bValid;
	HWND	hWnd;
	int		xPos;
	int		yPos;
	NumProc lpProc;
} NUMBERMODULE;


/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern	void	CreateNumberPanel(HWND hWnd, int xPos, int yPos, NumProc lpNumerProc);
extern	void	DeleteNumerPanel(HWND hWnd);
extern	void	DrawNumberPanel(HWND hWnd, HDC Hdc);
extern	void	NumPostionPro(HWND hWnd, int nXPos, int nYPos);

extern void		TouchNumberInput(UINT *pLen, char * pNum, UINT nMaxLen, UINT nID);
extern void		TouchNumberBack(UINT *pLen, char * pNum);
extern void		TouchNumerClear(UINT *pLen, char * pNum, int nMaxLen);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // NUMBERPANEL_H



















