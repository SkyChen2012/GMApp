/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MoxModnet.h
**
**	AUTHOR:		Alan Huang
**
**	DATE:		12 - Nov - 2002
**
**	FILE DESCRIPTION:
**			Mox Modnet definition files
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef DIAGTELNETD_H
#define DIAGTELNETD_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/
 
#define	DIAG_ITEM_COUNT		25
#define	MAX_LOG_BUFF		48
#define MAX_LOG_MSG_LEN		126
/************** TYPEDEFS **************************************************************************************/
#define DIAG_ETH_SENT					0
#define DIAG_ETH_SENT_ERR				1
#define DIAG_ETH_RCV					2
#define DIAG_ETH_RCV_ERR				3


#define DIAG_A_SENT						4
#define DIAG_A_SENT_ERR					5
#define DIAG_A_RCV						6
#define DIAG_A_RCV_ERR					7
#define DIAG_A_RCV_RATE					8
#define DIAG_A_SENT_RATE				9

#define DIAG_V_SENT						10
#define DIAG_V_SENT_ERR					11
#define DIAG_V_RCV						12
#define DIAG_V_RCV_ERR					13
#define DIAG_V_SENT_RATE				14
#define DIAG_V_RCV_RATE					15


#define DIAG_V_FRM_SENT					16
#define DIAG_V_FRM_SENT_ERR				17
#define DIAG_V_FRM_RCV					18
#define DIAG_V_FRM_RCV_ERR				19
#define DIAG_V_RCV_FRM_RATE				20
#define DIAG_V_SENT_FRM_RATE			21

#define DIAG_MM_POS						22

#define DIAG_THREAD_COUNT				23
#define DIAG_SRLREAD_COUNT				24


/************** STRUCTURES ************************************************************************************/


/************** GLOBAL VARIABLE DEFINITIONS *******************************************************************/

//!!!  It is C/C++ file specific, nothing should be defined here

/************** EXTERNAL DECLARATIONS *************************************************************************/


extern DWORD	dwDiag[DIAG_ITEM_COUNT];

extern int		nSleepTick;

extern BOOL	DiagPutOneLog(CHAR* pMsg);

extern void	TelnetInit();

/**************************************************************************************************************/
#endif // DIAGTELNETD_H
