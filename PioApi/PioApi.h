/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	PioApi.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		03 - Nov - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef PIO_API_H
#define PIO_API_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/

#define	PIO_DEV_NAME				"/dev/mox_pio"

#define PIO_TYPE_UNLOCK						1
#define PIO_TYPE_DO_1						2
#define PIO_TYPE_DO_2						3
#define PIO_TYPE_DI1						4
#define PIO_TYPE_DI2						5
#define PIO_TYPE_PHOTOSENSITIVE	  			6
#define PIO_TYPE_CCD_LED					7
#define PIO_TYPE_KEY_LED					8
#define PIO_TYPE_TAMPER_ALARM	   			9
#define PIO_TYPE_CCD_POWER			  		10
#define PIO_TYPE_OLED_POWER			  		11
#define PIO_TYPE_HID_HOLD					12
#define PIO_TYPE_AMP_PWR					13
#define PIO_TYPE_BEEP_CONTROL				14
#define RESET_SWITCH_DI                     15
#define PIO_TYPE_SHELL_KEYBOARD_POWER		16
#define PIO_TYPE_SHELL_LCD_POWER			17
#define PIO_TYPE_SHELL_READCARD_POWER		18




/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern int		PIOInit();
extern void		PIOExit();
extern int		PIORead(int nType);
extern void		PIOWrite(int nType, unsigned char ucValue);
/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // PIO_API_H


