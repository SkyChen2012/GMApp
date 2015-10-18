/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	LiftCtrlConversion.h
**
**	AUTHOR:		Mark Qian
**
**	DATE:		21 - Jan - 2010
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef LIFTCONTROLCONVERSION_H
#define LIFTCONTROLCONVERSION_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/
#define SLEEPTICK						2

#define TIMEOUT_SENDING_WAITING		1000
#define TIMEOUT_RECEIVING_WAITING		5000

#define TIME_INTERVAL_SENDING_INIT		10000
#define TIME_INTERVAL_SENDING_CHECK		30000

#define CHECK_REPEAT_TIMES				10

#define SERIAL_BUF_LEN				22

/************** STRUCTURES ************************************************************************************/
typedef struct _HitachiData
{
	unsigned char stx;
	unsigned char length[3];
	unsigned char command[2];
	unsigned char event[2];
	unsigned char code[2];
	unsigned char reserved[2];
	unsigned char buildingNum[2];
	unsigned char roomNum[4];
	unsigned char hallNum[2];
	unsigned char etx;
	unsigned char sum;
}HitachiData;

typedef enum _SENDING_STATE
{
	SENDING_STATE_SENDING_ENQ = 1,
	SENDING_STATE_WAITING_ENQ_ACK_1,
	SENDING_STATE_WAITING_ENQ_ACK_2,
	SENDING_STATE_WAITING_ENQ_ACK_3,
	SENDING_STATE_WAITING_ENQ_ACK_4,
	SENDING_STATE_SENDING_DATA,
	SENDING_STATE_WAITING_DATA_ACK_1,
	SENDING_STATE_WAITING_DATA_ACK_2,
	SENDING_STATE_WAITING_DATA_ACK_3,
	SENDING_STATE_WAITING_DATA_ACK_4,
	SENDING_STATE_SUCCESS,
	SENDING_STATE_FAIL
}SENDING_STATE;

typedef enum _RECEIVING_STATE
{
	RECEIVING_STATE_START = 1,
	RECEIVING_STATE_WAITING_ENQ,
	RECEIVING_STATE_WAITING_DATA,
	RECEIVING_STATE_SUCCESS,
	RECEIVING_STATE_FAIL
}RECEIVING_STATE;

typedef enum _TOTAL_STATE
{
	TOTAL_STATE_WAITING_INIT = 1,
	TOTAL_STATE_SENDING_INIT,
	TOTAL_STATE_RECEIVING_INIT_RESPONSE,
	TOTAL_STATE_SENDING_CHECK,
	TOTAL_STATE_RECEIVING_CHECK_RESPONSE,
	TOTAL_STATE_SENDING_DATA
}TOTAL_STATE;

static const unsigned char cmd_Stx = 0x02;
static const unsigned char cmd_Etx = 0x03;
static const unsigned char cmd_Enq = 0x05;
static const unsigned char cmd_Ack = 0x06;
static const unsigned char cmd_Nak = 0x15;

static const unsigned char cmd_InitReq[] = {
	0x02, //stx
	0x30, 0x31, 0x36, // data length
	0x30, 0x30, // command
	0x30, 0x30, // event
	0x30, 0x30, // code
	0x30, 0x30, // reserved
	0x30, 0x30, // Building Number
	0x30, 0x30, 0x30, 0x30, // Room Number
	0x30, 0x30, // Hall Number
	0x03, // etx
	0x9A // sum
};
static const unsigned char cmd_InitResp[] = {
	0x02, //stx
	0x30, 0x31, 0x36, // data length
	0x30, 0x31, // command
	0x30, 0x30, // event
	0x30, 0x30, // code
	0x30, 0x30, // reserved
	0x30, 0x30, // Building Number
	0x30, 0x30, 0x30, 0x30, // Room Number
	0x30, 0x30, // Hall Number
	0x03, // etx
	0x9B // sum
};
static const unsigned char cmd_CheckReq[] = {
	0x02, //stx
	0x30, 0x31, 0x36, // data length
	0x33, 0x30, // command
	0x30, 0x30, // event
	0x30, 0x30, // code
	0x30, 0x30, // reserved
	0x30, 0x30, // Building Number
	0x30, 0x30, 0x30, 0x30, // Room Number
	0x30, 0x30, // Hall Number
	0x03, // etx
	0x9D // sum
};
static const unsigned char cmd_CheckResp[] = {
	0x02, //stx
	0x30, 0x31, 0x36, // data length
	0x33, 0x31, // command
	0x30, 0x30, // event
	0x30, 0x30, // code
	0x30, 0x30, // reserved
	0x30, 0x30, // Building Number
	0x30, 0x30, 0x30, 0x30, // Room Number
	0x30, 0x30, // Hall Number
	0x03, // etx
	0x9E // sum
};


typedef struct _HitachiCodeMapTable
{
	char szCode[20];
	char szDoorCode[20];
	unsigned char ucType;
	char szBuildingNum[3];
	char szRoomNum[5];
	char szHallNum[3];
}HCCMT;

typedef struct LiftControlHitachi
{
	int	nCount;
	HCCMT * pHCCMT;
}LCHC;

#define HCCMT_FILE	"/mox/rdwr/hccmt.ini"
#define HCCMT_SECTION	"hccmt"
#define COUNT_KEY		"count"

#define HCCMT_KEY_CODE		"code"
#define HCCMT_KEY_TYPE		"type"
#define HCCMT_KEY_BUILDNUM	"buildingnum"
#define HCCMT_KEY_ROOMNUM	"roomnum"
#define HCCMT_KEY_HALLNUM	"hallnum"
#define HCCMT_KEY_DOORCODE	"doorcode"
//#define HCCMT_KEY_IP		"IP"
/************** EXTERNAL DECLARATIONS *************************************************************************/

extern void		LCAConversionMdInit(void);
extern void		LCConversionThreadFun(void);
extern void		LCConversionMdExit(void);


/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // LIFTCONTROLCONVERSION_H


