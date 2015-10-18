/*hdr
**
**	Copyright Mox Group
**
**	FILE NAME:	hash.h
**
**	AUTHOR:		Denny Du
**
**	DATE:		26 - 12 - 2014
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/
#ifndef HASH_H
#define HASH_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/
#include "AccessCommon.h"

/************** DEFINES ***************************************************************************************/

#define HASH_SUPPORT_GM_FILE_SYSTEM	//门口机和MC版本宏开关


#ifndef		U8
#define		U8			unsigned char
#endif
#ifndef		U16
#define		U16			unsigned short
#endif
#ifndef		U32
#define		U32			unsigned int
#endif

#define SYNCDATABUFSIZE	0x500000	//5MB

#define	BUF_TYPE_HASH_FUNC	0
#define	BUF_TYPE_HASH_LIST	1
#define	BUF_TYPE_HASH_CARD	2
#define	BUF_TYPE_HASH_INFO	3

#define DOWNLOAD_FILENAME_CARD_HASH_FUNC	"HASHFUNC"	//hash桶
#define DOWNLOAD_FILENAME_CARD_HASH_LIST	"HASHLIST"	//冲突列表
#define DOWNLOAD_FILENAME_CARD_HASH_DATA	"HASHDATA"	//卡片数据
#define DOWNLOAD_FILENAME_CARD_HASH_INFO	"HASHINFO"	//索引信息

#define TYPE_NORMAL_CARD    2	//门禁卡
#define TYPE_PATROL_CARD	3	//巡更卡
#define TYPE_LIFT_CARD		4	//梯控卡

#ifdef HASH_SUPPORT_GM_FILE_SYSTEM
//门口机哈希算法文件存储
#define AHASHFILE	"/mox/rdwr/HASH"
#endif

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/
#ifdef HASH_SUPPORT_GM_FILE_SYSTEM
//门口机版本
extern HANDLE *OpenCardDataBase(void);
extern BOOL CloseCardDataBase(HANDLE * pCardHandle);
extern BOOL WriteCardInfo(HANDLE * pCardHandle,CDINFO *Card,U8 CardType);
extern BOOL ReadCard(HANDLE * pCardHandle,BYTE *CSN,U8 CardType,CDINFO *Card);
extern BOOL DeleteCard(HANDLE * pCardHandle,BYTE *CSN,U8 CardType);
extern int GetCardNumber(HANDLE * pCardHandle, int nCardType);
extern BOOL GetCardBuffInfo(HANDLE * pCardHandle,int nBufType, BYTE **pCardBuff,int *pCardFileSize);
extern BOOL SetCardBuffInfo(HANDLE * pCardHandle,int nBufType, BYTE *pCardBuff,int pCardFileSize,int pCardFlieOffset);
#else
//MC版本
extern HANDLE *OpenCardDataBase(BYTE *pCardBuff,int pCardBuffSize,int DeviceType);
extern BOOL CloseCardDataBase(HANDLE *pCardHandle);
extern BOOL ClearCardDataBase(HANDLE *pCardHandle);
extern BOOL AddCard(HANDLE *pCardHandle,int nCardType,void *pCardInfo,int nCardNumber);
extern int GetCardNumber(HANDLE * pCardHandle, int nCardType);
extern BOOL GetCardBuffInfo(HANDLE * pCardHandle,int nBufType, BYTE **pCardBuff,int *pCardFileSize);
extern BOOL GetCardInfo(HANDLE * pCardHandle, int nCardType,int *nCardNumber,void * pCardInfo);
#endif

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // HASH_H

