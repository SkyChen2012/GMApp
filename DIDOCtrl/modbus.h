/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	modbus.h
**
**	AUTHOR:		Jason Wang
**
**	DATE:		2012-04-10
**
**	FILE DESCRIPTION:
**		The header of modbus.c
**
**	FUNCTIONS:
**
**	NOTES:
**
*/
#ifndef __MODBUS_H
#define __MODBUS_H
#include "MXCommon.h"

/************** SYSTEM INCLUDE FILES **************************************************/
 
 
/************** USER INCLUDE FILES ***************************************************/
 
/************** DEFINES **************************************************************/
 
/************** TYPEDEFS *************************************************************/
 
/************** STRUCTURES ***********************************************************/
 
/************** EXTERNAL DECLARATIONS ************************************************/
extern BOOL modbus_init(void);
extern void set_serial_para(unsigned int baud_rate, BYTE data_bits, BYTE parity_flag, BYTE stop_bits);
extern void send_cmd_to_DI_serial(BYTE stop_code, BYTE cmd, BYTE start_addr, BYTE count);
extern void send_cmd_to_DO_serial(BYTE stop_code, BYTE cmd, BYTE start_addr, BYTE count, unsigned short value);
extern unsigned int wait_slave_ack_serial(BYTE *buf, unsigned int buf_len, unsigned int time_out);
extern BOOL analysis_ack_serial(BYTE *buf, unsigned int buf_len, unsigned int *value); 
//!!!  It is H/H++ file specific, nothing should be defined here
 
/************** ENTRY POINT DECLARATIONS *********************************************/
 
/************** LOCAL DECLARATIONS ***************************************************/
 
/*************************************************************************************/
 

#endif /* _MODBUS_H */


