/*
*hdr
**
**	Copyright MOX Products
**
**	FILE NAME:	MXTypes.h
**
**	AUTHOR:		Gao Guanjun
**
**	DATE:		08 - Oct - 2006
**
**	FILE DESCRIPTION:
**			Mox standard header file
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/
#ifndef _H_MOXTYPES
#define _H_MOXTYPES
/************** SYSTEM INCLUDE FILES **************************************************************************/

//Nothing should be included here

/************** USER INCLUDE FILES ****************************************************************************/

//Nothing should be included here

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/

#define MAX_PATH          260

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef BAD_RET
#define BAD_RET              -1
#endif

#ifndef NO_ERROR
#define NO_ERROR            0
#endif

#define	NOT		!
#define	AND		&&
#define	OR		||

//Get the maximize value from two given values
#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

//Get the minimize value from two given values
#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//Get the value of bit i from byte a, from low to high
/*
for (int i = 0; i < 8; i++)
{
	TRACE("%d ", BIT(247, i));
}
out put is: 11101111
*/
#ifndef BIT
#define BIT(a,i)				(((a) >> (i)) & (1))
#endif

#ifndef __SUPPORT_WIEGAND_CARD_READER__ /*AaronWu, For support wiegand card reader*/
#define __SUPPORT_WIEGAND_CARD_READER__
#endif

#ifndef __SUPPORT_PROTOCOL_900_1A__ /*AaronWu, For support protocol 900.1a*/
#define __SUPPORT_PROTOCOL_900_1A__
#endif


#ifndef _WINDEF_H

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

#endif	// _WINDEF_H

#define MAKEDWORD(a_h, a_l, b_h, b_l)			((DWORD)((MAKEWORD(a_l, a_h) << 16) | (MAKEWORD(b_l, b_h))))
#define	AHIBYTE(d)								((BYTE) (((d) >> 24) & 0xFF))
#define	ALOBYTE(d)								((BYTE) (((d) >> 16) & 0xFF))
#define	BHIBYTE(d)								((BYTE) (((d) >> 8) & 0xFF))
#define	BLOBYTE(d)								((BYTE) ((d) & 0xFF))

#ifndef	PACKED
#define	PACKED	__attribute__((packed, aligned(1)))
#endif	// PACKED

/************** TYPEDEFS **************************************************************************************/

#ifndef _WINDEF_H

#ifndef LONG
typedef signed long				LONG;		//LONG
#endif

#ifndef DWORD
typedef unsigned long       DWORD;		//DWORD
#endif

#ifndef INT
typedef signed int                 INT;		//INT
#endif

#ifndef  UINT
typedef unsigned int        UINT;		//UINT
#endif

#ifndef  CHAR
typedef signed char				CHAR;		//CHAR
#endif

#ifndef UCHAR
typedef unsigned char		UCHAR;		//UCHAR
#endif

#ifndef BYTE
typedef UCHAR				BYTE;		//BYTE
#endif

#ifndef PBYTE
typedef BYTE*				   PBYTE;		//PBYTE
#endif

#ifndef BOOL
typedef int                 BOOL;		//BOOL
#endif

#ifndef SHORT
typedef signed short				SHORT;		//SHORT
#endif

#ifndef USHORT
typedef unsigned short		USHORT;		//USHORT
#endif

#ifndef WORD
typedef unsigned short      WORD;		//WORD
#endif

#ifndef FLOAT
typedef float               FLOAT;		//FLOAT
#endif

#ifndef DOUBLE
typedef double				DOUBLE;		//DBOUBLE				
#endif

#ifndef VOID
typedef void				VOID;		//VOID
#endif

#ifndef HANDLE
typedef VOID*				HANDLE;		//HANDLE
#endif

#ifndef LPARAM
typedef LONG				LPARAM;		//LPARAM
#endif

#ifndef WPARAM
typedef UINT				WPARAM;		//WPARAM
#endif

#ifndef LRESULT
typedef LONG				LRESULT;	//LRESULT
#endif

#endif	// _WINDEF_H

/************** STRUCTURES ************************************************************************************/

/************** GLOBAL VARIABLE DEFINITIONS *******************************************************************/

//!!!  It is C/C++ file specific, nothing should be defined here

/************** EXTERNAL DECLARATIONS *************************************************************************/

/**************************************************************************************************************/
#endif	//#define _H_MOXTYPES

