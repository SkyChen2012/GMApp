/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	BacpNet.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		09 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef BACPNET_H
#define BACPNET_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

#include "Bacp.h"

/************** DEFINES ***************************************************************************************/

#define	BACP_VER_10							0x10
#define	BACP_VER_20							0x20

#if	BACP_DWORD_ORDER == 0
	#define	BACP_NET_LY_STX_B3				0x34
	#define	BACP_NET_LY_STX_B2				0x23
	#define	BACP_NET_LY_STX_B1				0x54
	#define	BACP_NET_LY_STX_B0				0x34
	#define	BACP_NET_LY_ETX_B3				0x34
	#define	BACP_NET_LY_ETX_B2				0x54
	#define	BACP_NET_LY_ETX_B1				0x23
	#define	BACP_NET_LY_ETX_B0				0x43
#elif BACP_DWORD_ORDER == 1
	#define	BACP_NET_LY_STX_B0				0x34
	#define	BACP_NET_LY_STX_B1				0x23
	#define	BACP_NET_LY_STX_B2				0x54
	#define	BACP_NET_LY_STX_B3				0x34
	#define	BACP_NET_LY_ETX_B0				0x34
	#define	BACP_NET_LY_ETX_B1				0x54
	#define	BACP_NET_LY_ETX_B2				0x23
	#define	BACP_NET_LY_ETX_B3				0x43
#endif

#define	LEN_BACP_NET_LY_STX					4
#define	LEN_BACP_NET_LY_VER					1
#define	LEN_BACP_NET_LY_LEN					2
#define	LEN_BACP_NET_LY_SEQ					4
#define	LEN_BACP_NET_LY_OPT					1
#define	LEN_BACP_NET_LY_ETX					4
#define	MIN_LEN_BACP_NET_LY					(LEN_BACP_NET_LY_STX + LEN_BACP_NET_LY_VER + LEN_BACP_NET_LY_LEN + LEN_BACP_NET_LY_SEQ + LEN_BACP_NET_LY_OPT + LEN_BACP_NET_LY_ETX)
#define	RSP_LEN_BACP_NET_LY					MIN_LEN_BACP_NET_LY

#define	OFFSET_BACP_NET_LY_STX				0
#define	OFFSET_BACP_NET_LY_VER				4
#define	OFFSET_BACP_NET_LY_LEN				5
#define	OFFSET_BACP_NET_LY_SEQ				7
#define	OFFSET_BACP_NET_LY_OPT				11
#define	OFFSET_BACP_NET_LY_APP				12
#define	OFFSET_BACP_NET_LY_ETX				


#define	CURRENT_BACP_NET_LAY_VER			BACP_VER_20

#define	BACP_NET_LY_OPT_CON_ON				0x01
#define	BACP_NET_LY_OPT_CON_OFF				0x00
#define	BACP_NET_LY_OPT_REQ					0x00
#define	BACP_NET_LY_OPT_RSP					0x02


/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

// Network layer frame basic field
typedef	struct _BacpNetBscFld
{
	unsigned char				Stx[LEN_BACP_NET_LY_STX];		// STX
	unsigned char				Ver;							// VER
	unsigned short				Len;							// LEN
	unsigned int				Seq;							// SEQ
	unsigned char				Opt;							// OPT
	unsigned char				Etx[LEN_BACP_NET_LY_ETX];		// ETX
} BacpNetBscFld;

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern BOOL		IsCpltBacpNetFrm(unsigned char* pBuf, int* pnLen);
extern int		PckBacpNetRqt(unsigned char* pAppBuf, int nAppBufLen, unsigned int nSeq, unsigned char* pFrm);
extern int		PckBacpNetRqtEx(unsigned char* pAppBuf, int nAppBufLen, BOOL bNeedRsp, unsigned int nSeq, unsigned char* pFrm);
extern int		PckBacpNetRsp(unsigned char* pRqtFrm, int nRqtFrmLen, unsigned char* pRspFrm);
extern int		PckBacpNetRspEx(unsigned int nSeq, unsigned char* pRspFrm);
extern void		DebugBacpNetFrm(unsigned char* pFrm, int nFrmLen);

extern BOOL		IsBacpNetRqtFrm(unsigned char* pFrm, int nFrmLen);
extern BOOL		IsBacpNetRspFrm(unsigned char* pFrm, int nFrmLen);

extern int		UnpckBacpNet(unsigned char* pFrm, int nFrmLen, BacpNetBscFld* pBscFld, unsigned char* pApp);
extern int		UnpckBacpNetEx(unsigned char* pFrm, int nFrmLen, BacpNetBscFld* pBscFld, unsigned char** ppApp);
extern BOOL		IsBacpNetOptConOn(unsigned char ucOpt);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // BACPNET_H
