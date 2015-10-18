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

#ifndef MOXMODNET_H
#define MOXMODNET_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/
#include "MXTypes.h"
#include "dsys0soc.h"
/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/
 

#define	MAX_TELNETD_CONN_NUM		0x02	// the last conn is for relay write request from standby to active
#define MOX_TELNETD_PORT			23


// buffer lengths

#define	MAX_TELNET_BUFF		2048	// this could acoommodate 3 packet
#define	MAX_PACK_LEN		300	// 
#define	MAX_RECV_LEN		350	// 

#define KEY_STATISTIC	's'
#define KEY_LOG			'l'
#define KEY_RESET		'r'


// defines for UDP
/************** TYPEDEFS **************************************************************************************/

typedef struct MOXTELNETDCONN
{
	typSOC_ID	Socket;
	BOOL		bInUse;
	UCHAR		RecvBuff[MAX_TELNET_BUFF];
	UCHAR		SendBuff[MAX_TELNET_BUFF];
	SHORT		nRecvLen;
	SHORT		nSendLen;
	DWORD		dwLastTick;
	
	SHORT		nStat;	//

} MOXTELNETDCONN;



/************** STRUCTURES ************************************************************************************/


/************** GLOBAL VARIABLE DEFINITIONS *******************************************************************/

//!!!  It is C/C++ file specific, nothing should be defined here

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern MOXTELNETDCONN*		MoxTelnetdConn[MAX_TELNETD_CONN_NUM];
extern SHORT	MoxTelnetdInit();
extern SHORT	MoxTelnetdProcess();
extern SHORT	MoxTelnetdExit();

extern BOOL FlagShowLogData;

/**************************************************************************************************************/
#endif // MOXMODNET_H
