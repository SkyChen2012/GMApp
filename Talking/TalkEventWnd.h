/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MXList.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		20 - Aug - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				
**								
**	NOTES:
** 
*/



/************** USER INCLUDE FILES ***************************************************/

#include <windows.h>

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

typedef enum _CALLSTAUTS
{
	CALLING_DB_EHV = 1,
	CALLING_DB_ERROR_CODE,
	CALLING_DB_MC,
	CALLING_GM,
	CALLED	
}CALLSTATUS;


/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

extern void CreateTalkWnd(HWND hwndParent);
extern void	ShowTalkWnd(void);
extern void	WndHideTalkWindow(void);
extern void ShowTalkUlkWnd(void);
extern void HideTalkUlkWnd(void);
extern void SetCalloutKey(char *pKeyBuff);
extern BOOL CheckRecallKey();
extern BOOL GetLeavePhotoStatus();
extern DWORD GetRecallKey();
extern void ClearRecallKey();
extern BOOL isDBCallKeyPressed(WPARAM wParam);

extern CALLSTATUS g_CallStatus;
extern HWND g_hWNDTalk;

