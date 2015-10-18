#ifndef BACPSERIAL_H
#define BACPSERIAL_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

#include "Bacp.h"

/************** DEFINES ***************************************************************************************/

#define	BACP_SERIAL_VER_10							0x10
#define	BACP_SERIAL_VER_20							0x20

#define	BACP_SERIAL_ADDR_DEFAULT					0x00

//#if	BACP_DWORD_ORDER == 0
	#define	BACP_SERIAL_STX_B1				0x54
	#define	BACP_SERIAL_STX_B0				0x34
	#define	BACP_SERIAL_ETX_B1				0x34
	#define	BACP_SERIAL_ETX_B0				0x54
//#elif BACP_DWORD_ORDER == 1
//	#define	BACP_SERIAL_STX_B0				0x34
//	#define	BACP_SERIAL_STX_B1				0x54
//	#define	BACP_SERIAL_ETX_B0				0x54
//	#define	BACP_SERIAL_ETX_B1				0x34
//#endif

#define	LEN_BACP_SERIAL_STX					2
#define	LEN_BACP_SERIAL_VER					1
#define	LEN_BACP_SERIAL_LEN					2
#define	LEN_BACP_SERIAL_SEQ					1
#define	LEN_BACP_SERIAL_OPT					1
#define	LEN_BACP_SERIAL_ADDR					6
#define	LEN_BACP_SERIAL_CHK					2
#define	LEN_BACP_SERIAL_ETX					2
#define	MIN_LEN_BACP_SERIAL					(LEN_BACP_SERIAL_STX + LEN_BACP_SERIAL_VER + LEN_BACP_SERIAL_LEN + LEN_BACP_SERIAL_SEQ + LEN_BACP_SERIAL_OPT + LEN_BACP_SERIAL_ADDR + LEN_BACP_SERIAL_CHK + LEN_BACP_SERIAL_ETX)//17
#define	RSP_LEN_BACP_SERIAL					MIN_LEN_BACP_SERIAL

#define	OFFSET_BACP_SERIAL_STX				0
#define	OFFSET_BACP_SERIAL_VER				2
#define	OFFSET_BACP_SERIAL_LEN				3
#define	OFFSET_BACP_SERIAL_SEQ				5
#define	OFFSET_BACP_SERIAL_OPT				6
#define	OFFSET_BACP_SERIAL_ADDR				7
#define	OFFSET_BACP_SERIAL_APP				13


#define	CURRENT_BACP_SERIAL_VER				BACP_SERIAL_VER_20

#define	BACP_SERIAL_OPT_CON_ON				0x01
#define	BACP_SERIAL_OPT_CON_OFF				0x00
#define	BACP_SERIAL_OPT_REQ					0x00
#define	BACP_SERIAL_OPT_RSP					0x02


/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

// Serial layer frame basic field
typedef	struct _BacpSerialBscFld
{
	unsigned char				Stx[LEN_BACP_SERIAL_STX];		// STX
	unsigned char				Ver;							// VER
	unsigned short			Len;							// LEN
	unsigned char				Seq;							// SEQ
	unsigned char				Opt;							// OPT
	unsigned char				Da1;
	unsigned char				Da2;
	unsigned char				Da3;
	unsigned char				Sa1;
	unsigned char				Sa2;
	unsigned char				Sa3;
	unsigned short			Chk;
	unsigned char				Etx[LEN_BACP_SERIAL_ETX];		// ETX
} BacpSerialBscFld;

/************** EXTERNAL DECLARATIONS *************************************************************************/
extern BOOL IsCorrectCheckSum(unsigned char* pBuf, unsigned short len, unsigned char chksum);
extern BOOL		IsValidBacpSerialFrm(unsigned char* pBuf, int* pnLen);
extern int		PckBacpSerialRqt(unsigned char* pAppBuf, int nAppBufLen, unsigned int nSeq, unsigned char* pFrm);
extern int		PckBacpSerialRqtEx(unsigned char* pAppBuf, int nAppBufLen, BOOL bNeedRsp, unsigned char nSeq, unsigned char* pFrm);
extern int		PckBacpSerialRsp(unsigned char* pRqtFrm, int nRqtFrmLen, unsigned char* pRspFrm);
extern int		PckBacpSerialRspEx(unsigned char nSeq, unsigned char* pRspFrm);
extern void		DebugBacpSerialFrm(unsigned char* pFrm, int nFrmLen);

extern BOOL		IsBacpSerialRqtFrm(unsigned char* pFrm, int nFrmLen);
extern BOOL		IsBacpSerialRspFrm(unsigned char* pFrm, int nFrmLen);

extern int		UnpckBacpSerial(unsigned char* pFrm, int nFrmLen, BacpSerialBscFld* pBscFld, unsigned char* pApp);
extern int		UnpckBacpSerialEx(unsigned char* pFrm, int nFrmLen, BacpSerialBscFld* pBscFld, unsigned char** ppApp);
extern BOOL		IsBacpSerialOptConOn(unsigned char ucOpt);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // BACPSERIAL_H
