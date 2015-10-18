/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	LCAPro_mox_V1.h
**
**	AUTHOR:		Harry Qian
**
**	DATE:		28 - Sep - 2010
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef LCAPRO_MOX_V1_H
#define LCAPRO_MOX_V1_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/
#define	LEN_COMM_APP_LY_LEN			2
#define	LEN_COMM_APP_LY_FC			2
#define NOT_RSP				0
#define NEED_RSP				1

#define	BACP_COM_LY_STX_B1				0x54
#define	BACP_COM_LY_STX_B0				0x34
#define BACP_COM_VER					0x20

#define	BACP_COM_LY_ETX_B1				0x34
#define	BACP_COM_LY_ETX_B0				0x54

#define OFFSET_COM_LY_PACKAGE_LEN		3

#define LEN_COM_STX				2
#define	LEN_COM_VER				1
#define	LEN_COM_LEN					2
#define	LEN_COM_SEQ					1
#define	LEN_COM_OPT					1
#define LEN_COM_DA					3
#define LEN_COM_SA					3
#define LEN_COM_CHK					2
#define	LEN_COM_ETX					2

#define	COM_VER_10							0x10
#define	COM_VER_20							0x20

#define	CURRENT_COM_NET_LAY_VER			COM_VER_20


#define	COM_LY_OPT_REQ_ON					0x01
#define	COM_LY_OPT_REQ_OFF					0x00
#define COM_LY_OPT_REQ_FLAG					0x02	
#define COM_LY_OPT_SA						0x04


#define MCMT_FILE	"/mox/rdwr/mcmt.ini"
#define MCMT_SECTION	"mcmt"
#define KEY_USER_COUNT		"usercnt"
#define KEY_LIFT_COUNT		"liftcnt"

#define KEY_DEVICECODE		"code"
#define KEY_TYPE			"type"
#define KEY_LIFTID			"liftid"
#define KEY_LAYER			"layer"

#define	KEY_DA1				"da1"
#define	KEY_DA2				"da2"
#define	KEY_DA3				"da3"

#define MIN_DATA_LEN		20	// 4 + 4 + 1
#define ACK_ERRORTIP_OFFSET	17 
/************** STRUCTURES ************************************************************************************/

typedef struct _MOXCodeMapTable
{
	unsigned char ucType;
	unsigned char ucLiftID;
	unsigned char ucLayer;
	char szCode[20];
}MCMT;

typedef struct _MOXLIFTDA
{
	unsigned char da1;
	unsigned char da2;
	unsigned char da3;
}MLDA;



typedef struct LiftControlMOX
{
	int		nUsrCnt;
	int		nLiftCnt;
	MCMT *	pMCMT;
	MLDA *  pLFDA;
}LCMOX;


typedef struct _MOXRETRYInfo
{
	char			pData[256];
	unsigned char	nLen;
	unsigned char	nRetryCnt;
}RETRYINFO;


#define MOX_COM_RETRY_NUM	2
#define SERIAL_COMM_TIMEOUT	1000
/************** EXTERNAL DECLARATIONS *************************************************************************/

extern DWORD	GetLiftIDbyLayer(int nLayer);
extern DWORD	GetLiftIDbyCode(const char * pChar);
extern BYTE		GetLayerbyCode(const char * pChar);

extern void		LCAMOXProtoInit(void);
extern void		LCAMoxThreadFun(void);
extern void		LCAMOXProtoExit(void);

extern void		LCAMoxWaitingAckPro(void);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif //






















