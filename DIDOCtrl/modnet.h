/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	modnet.h
**
**	AUTHOR:		Jason Wang
**
**	DATE:		2012-04-10
**
**	FILE DESCRIPTION:
**		The header of modnet.c 
**
**	FUNCTIONS:
**
**	NOTES:
**
*/
#ifndef __MODNET_H
#define __MODNET_H
#include "MXCommon.h"

/************** SYSTEM INCLUDE FILES **************************************************/
 
 
/************** USER INCLUDE FILES ***************************************************/
 
/************** DEFINES **************************************************************/
#define modnet_udp_port 503 
#define COIL_ON 1
#define COIL_OFF 0
#define ACK_TIME_OUT 0xffffffff
#define BUF_LEN 256 


/************** TYPEDEFS *************************************************************/
 
/************** STRUCTURES ***********************************************************/
 
/************** EXTERNAL DECLARATIONS ************************************************/
extern BOOL modnet_init(void);
extern void send_cmd_to_DI_by_eth(unsigned int rip, unsigned short rport, BYTE cmd, unsigned short start_addr, unsigned short count);
extern void send_cmd_to_DO_by_eth(unsigned int rip, unsigned short rport, BYTE cmd, unsigned short start_addr, unsigned short count, unsigned short value); 
extern unsigned int wait_slave_ack_eth(BYTE *buf, unsigned int buf_len, unsigned int *rip, unsigned short *rport, unsigned int time_out);
extern BOOL analysis_ack_eth(BYTE *buf, unsigned int buf_len, unsigned int rip, unsigned short rport, unsigned int *value);

//!!!  It is H/H++ file specific, nothing should be defined here
 
/************** ENTRY POINT DECLARATIONS *********************************************/
 
/************** LOCAL DECLARATIONS ***************************************************/
 
/*************************************************************************************/
 

#endif

