/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Eth.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		10 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef ETH_H
#define ETH_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

#include <windows.h>

/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"
#include "MXList.h"
#include "Dispatch.h"
#include "Bacp.h"
#include "BacpNet.h"
#include "BacpApp.h"
#include "BacpNetComm.h"

/************** DEFINES ***************************************************************************************/

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern void		EthInit();
extern void		EthExit();
extern void		DoAppFrm(int nFunc, unsigned char* pAppFrm, int nAppFrmLen, unsigned int nSrcIPAddr);
extern void		DoNetSendCb(int nCbId, BOOL bSuccess);
extern void		RequestVideoIOVP(unsigned int nDestIPAddr);

extern UINT		GetRecvPackNum(void);
extern UINT		GetSendPackNum(void);
extern void	MxSendAV2Eth(int nType, unsigned char* pBuf, int nBufLen);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // ETH_H
