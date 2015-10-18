/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MXMdId.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		13 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef MXMDID_H
#define MXMDID_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/

#define	MXMDID_NULL				0x00		// NULL
#define	MXMDID_TALKING			0x01		// Talking
#define	MXMDID_ETH				0x02		// Ethernet
#define	MXMDID_MM				0x03		// Multi-Media
#define	MXMDID_SA				0x04		// SecurityAlarm
#define MXMDID_SMG				0x05		// ShortMessage
#define MXMDID_PU				0X07		// PasswordUnlock
#define MXMDID_PSW				0x08		// password
#define MXMDID_AB				0x09		// Address Book
#define MXMDID_KEY				0x0A		// key
#define MXMDID_UART				0x0B		// UART
#define MXMDID_IOCTRL			0x0C		// IOCTRL
#define MXMDID_AS				0x0D		// Access System
#define MXMDID_PARASET			0x0E		// Set Parameters
#define MXMDID_PUBLIC			0X10		// Public function

#define MXMDID_LC				0x11		// LiftControl
#define MXMDID_LCA				0x12		// LiftControl Agent

#define MXMDID_HITACHI			0x13		// Hitachi LiftControl
#define MXMDID_YUV2JPG			0x14		// IOCTRL
#define MXMDID_LT_MOX_V1		0x15		// MOX	lift control
#define MXMDID_LT_MITSUBISHI	0X16		// Mitsubishi Lift Control

#define MXMDID_ACC				0x17		// Access Client
#define MXMDID_ACA				0x18		// Access Agent
#define MXMDID_LT_MOX_V2		0x19		// MOX	lift control
#define MXMDID_MODNET			0x20		// control DI/DO module through modnet
#define MXMDID_MODBUS			0x21
#define MXMDID_LT_BY_DO_MODULE	0x22
#define MXMDID_RTC				0x23


/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // MXMDID_H

