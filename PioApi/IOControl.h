/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	IOControl.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		05 - Sep - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef IOCONTROL_H
#define IOCONTROL_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/
#define		KEY_LED_ON		0// 1: Open  0:close
#define		KEY_LED_OFF		1
#define		CCD_LED_ON		1// 0: Open  1:close
#define		CCD_LED_OFF		0
//#define		DO_ON			1
//#define		DO_OFF			0
#define		DO_ON			0
#define		DO_OFF			1

#define	FUN_DI_DISABLED			0
#define	FUN_MANUAL_SWITCH		1
#define	FUN_GATE_OPEN_TIMEOVER	2
#define	FUN_PLAY_NOTE				3
#define	FUN_FIREALARM_LINKAGE	4
#define	FUN_INFRARED_ALARM		5


#define	FUN_DO_DISABLED			0
#define	FUN_LINKAGE_UNLOCK		1
#define	FUN_DO_LIFTCTRL_UP		2
#define	FUN_DO_LIFTCTRL_DOWN	3
#define	FUN_DO_ALARM_OUTPUT		4


#define DI_LOW_LEVEL	0
#define DI_HIGH_LEVEL	1
#define DI_NO_DETECT	2

#define CCD_LED_ON_DELAY_TIME  2500

/*add for shell GM*/
#define		SHELL_KEYBOARD_ON	1
#define		SHELL_KEYBOARD_OFF	0
#define		SHELL_LCD_ON	1
#define		SHELL_LCD_OFF	0
#define		SHELL_READCARD_ON	1
#define		SHELL_READCARD_OFF	0


/************** TYPEDEFS **************************************************************************************/
typedef struct
{
    DWORD LastTickCount;
    unsigned char UsedState;
    unsigned char Border;
    DWORD TotalVal;
    DWORD CurPosVal;
    DWORD CurNegVal;
}DETECT_TIMER_t;
/************** STRUCTURES ************************************************************************************/
typedef struct _DOTimerInfo
{
	BOOL		used;
	DWORD	 	pulseTimerTick;
}DOTimerInfo;

/************** EXTERNAL DECLARATIONS *************************************************************************/
extern VOID	IOCtrlInit(VOID);
extern VOID IOCtrlProc(VOID);
extern VOID CCDLedCtrl(UCHAR LedStatus);
extern BOOL IsPhotoSensitiveDetect();
extern VOID KeyLedCtrl(UCHAR LedStatus);

extern BOOL DIDetect(INT nIOType);
extern BYTE ReadDI1();
extern BYTE ReadDI2();
extern BOOL IsFireAlarmOn();	//for Shell GM
extern void KeyboardPwrCtrl(UCHAR KbStatus); //for Shell GM
extern void LcdPwrCtrl(UCHAR LcdStatus); //for Shell GM
extern void ReadCardPwrCtrl(UCHAR ReadCardStatus); //for Shell GM
extern BYTE ReadDI_V2(DETECT_TIMER_t* pTimer,INT uDIType,BOOL (*FUN)(INT));

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // IOCONTROL_H

