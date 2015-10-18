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

#define HASH_SUPPORT_GM_FILE_SYSTEM	//�ſڻ���MC�汾�꿪��


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

#define DOWNLOAD_FILENAME_CARD_HASH_FUNC	"HASHFUNC"	//hashͰ
#define DOWNLOAD_FILENAME_CARD_HASH_LIST	"HASHLIST"	//��ͻ�б�
#define DOWNLOAD_FILENAME_CARD_HASH_DATA	"HASHDATA"	//��Ƭ����
#define DOWNLOAD_FILENAME_CARD_HASH_INFO	"HASHINFO"	//������Ϣ

#define TYPE_NORMAL_CARD    2	//�Ž���
#define TYPE_PATROL_CARD	3	//Ѳ����
#define TYPE_LIFT_CARD		4	//�ݿؿ�

#ifdef HASH_SUPPORT_GM_FILE_SYSTEM
//�ſڻ���ϣ�㷨�ļ��洢
#define AHASHFILE	"/mox/rdwr/HASH"
#endif

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/
#ifdef HASH_SUPPORT_GM_FILE_SYSTEM
//�ſڻ��汾
extern HANDLE *OpenCardDataBase(void);
extern BOOL CloseCardDataBase(HANDLE * pCardHandle);
extern BOOL WriteCardInfo(HANDLE * pCardHandle,CDINFO *Card,U8 CardType);
extern BOOL ReadCard(HANDLE * pCardHandle,BYTE *CSN,U8 CardType,CDINFO *Card);
extern BOOL DeleteCard(HANDLE * pCardHandle,BYTE *CSN,U8 CardType);
extern int GetCardNumber(HANDLE * pCardHandle, int nCardType);
extern BOOL GetCardBuffInfo(HANDLE * pCardHandle,int nBufType, BYTE **pCardBuff,int *pCardFileSize);
extern BOOL SetCardBuffInfo(HANDLE * pCardHandle,int nBufType, BYTE *pCardBuff,int pCardFileSize,int pCardFlieOffset);
#else
//MC�汾
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

