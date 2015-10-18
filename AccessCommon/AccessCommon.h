
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AccessCommon.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		13 - April - 2009
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef ACCESSCOMMON_H
#define ACCESSCOMMON_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"
#include "MXCommon.h"


/************** DEFINES ***************************************************************************************/
#define CARD_DEBUG

#ifdef CARD_DEBUG
  #define DEBUG_CARD_PRINTF(...)  printf( __VA_ARGS__)
#else
	#define DEBUG_CARD_PRINTF(...)  {} 
#endif

#define DEBUG_AS_COMMON

#define AST_VERSION_LEN		4
#define AST_FUNVLD_LEN		1

#define MODE_ROOMCODE_PASSWORD_SET		0x01
#define MODE_ROOMCODE_PASSWORD_CLR		0xFE

#define MODE_CARD_ONLY_SET				0x02
#define MODE_CARD_ONLY_CLR				0xFD

#define MODE_CARD_PASSWORD_SET			0x04
#define MODE_CARD_PASSWORD_CLR			0xFB

#define AST_VERSION						0x00010103
#define APT_VERSION						0x00010103
#define AAT_VERSION						0x00010103


#define TYPE_AUTHORIZE_CARD							1
#define TYPE_NORMAL_CARD        				  	2
#define TYPE_PATROL_CARD						    3
#define TYPE_LIFT_CARD								4

#define		RD_NUM_DEFAULT					   "00"
#define		GROUP_NUM_DEFAULT				   0
#define		CARD_LOCALNUM_DEFAULT						1
#define		CARD_PASSWORD_DEFAULT		"66666666"

#define CARD_MODE_DEFAULT		1
#define STATION_NUMBER_DEFAULT		1
#define GATE_NUMBER_DEFAULT		1

#define CARD_VLD_ENDTIME_DEFAULT 0xFFFF

#define MAX_AS_CD_CNT						50000
#define MAX_AS_PCD_CNT						10000
#define MAX_AS_ACD_CNT						1000


#define CARD_STATUS_ENABLED				0
#define CARD_STATUS_DISABLED			1

#define PWD_VERSION								0x00000000


#define CARD_INFO_AUTHORIZED			0 // Card AUthorized
#define CARD_INFO_EXPIRED				1 // Expired
#define CARD_INFO_OUTOFTIMESLICE		2 // Out of Time Slice
#define CARD_INFO_DISABLED				3 // Card disabled

/************** TYPEDEFS **************************************************************************************/

typedef struct _CDPWDINFO
{	
	BYTE		CSN[CSN_LEN]	;
	CHAR		UlkPwd[PWD_LEN]	;
}PACKED CDPWDINFO;


typedef struct _ASTHEAD 
{
	DWORD			VersionNum;
	DWORD			nCardCnt;
}ASTHEAD,CDPWDHEAD;

typedef struct _HASH_CDINFO
{	
	BYTE CSN[CSN_LEN];
	BYTE CardType;
	CHAR RdNum[RD_CODE_LEN];
	CHAR UlkPwd[PWD_LEN];
	BYTE CardStatus;	
	WORD GateNumber;
	BYTE CardMode;
	BYTE UnlockTimeSliceID;
	BYTE ValidStartTime[TIME_LEN];
	BYTE ValidEndTime[TIME_LEN];
	BYTE bAdmin;
	BYTE Reserved[19];
}PACKED CDINFO;


typedef struct _CDINFO_HV
{	
	BYTE			CSN[CSN_LEN];
	DWORD		    LocalNum;	        //Card Local Number 
	CHAR			UlkPwd[PWD_LEN];
	CHAR			RdNum[RD_CODE_LEN];
	BYTE			GroupNum;
	BYTE			CardType;
	BYTE			CardStatus;
	BYTE			CardMode;	        //IC or ID Card
	WORD			GateNumber;
	BYTE			UlkTimeSlice;
	time_t			VldStartTime;
	time_t			VldEndTime;
	DWORD			Reserved[4];
    char            strCardHolderName[20];
    char            strRemark[20];
}PACKED CDINFO_HV_t;


typedef struct _CDINFO_OLD
{	
	BYTE			CSN[CSN_LEN]	;
	DWORD		    LocalNum		;	//Card Local Number 
	CHAR			UlkPwd[PWD_LEN]	;
	CHAR			RdNum[RD_CODE_LEN]	;
	BYTE			GroupNum	;
	BYTE			CardType	;
	BYTE			CardStatus	;
	BYTE			CardMode	;	//IC or ID Card
	WORD			GateNumber	;
	BYTE			UlkTimeSlice	;
	time_t			VldStartTime	;
	time_t			VldEndTime	;
	DWORD			Reserved[4]	;
}PACKED CDINFO_OLD;


typedef struct _CPTINFO
{
	int	PWDWorkStatus;
	CDPWDHEAD		PWDHead;
	CHAR*			MXPWDBuf;
}CPTINFO;

/************** STRUCTURES ************************************************************************************/


/************** EXTERNAL DECLARATIONS *************************************************************************/
extern CPTINFO	g_CardPWDInfo;
//Benson


extern BOOL IsLocalNumExist(DWORD nLocalNum);
extern BOOL IsCdExist(BYTE* pCSN);
extern BOOL IsCdInRoom(BYTE* pRdCode);
extern BOOL IsCdPwdExist(BYTE* pCSN, CHAR *PwdStr);


extern VOID LoadCdInfoFromMem();
extern VOID SaveCdInfo2Mem();
extern BOOL SaveCardInfo2Mem(int type);

extern VOID LoadPatrolCdInfoFromMem(VOID);
extern VOID LoadAuthorizeCdInfoFromMem(VOID);
extern VOID SavePatrolCdInfo2Mem(VOID);
extern VOID SaveAuthorizeCdInfo2Mem(VOID);


extern BOOL AsAddCd(CDINFO_OLD* pCdInfo);
extern BOOL AsRmCd(CHAR* pCSN);
extern INT RmCard(CHAR* pCSN);
extern VOID	AsRmAllCd();

extern CDINFO_OLD* AsGetCdInfobyCd(BYTE* pCSN);


extern CDINFO* AsGetCdInfobyLocalNum(DWORD nLocalNum);

extern CDINFO* AsGetPatrolCdInfobyCd(CHAR* pCSN);

extern INT AsRmCdByRdNum(CHAR* pRdNum, INT nRdNumLen);
extern BOOL	AsRmCdbyLocalNum(DWORD nLocalNum);
extern BOOL IsGroupNumExist(BYTE GroupNum);

extern BOOL AsAddPwd(CDPWDINFO* pCdInfo);
extern BOOL AsRmCdPwd(CHAR* pCSN);
extern BOOL AsModCdPwdbyCsn(BYTE* pCSN, INT nCSNLen,CHAR* pNewPwd, INT nPwdLen);
extern BOOL AsDelCdPwdbyRd(CHAR* pRdCode, INT nRdNumLen);

extern BOOL UkPwdCompare(CHAR *pSavePwd,CHAR *pIputPwd ,INT InPwdLen);



extern BOOL IsPatrolCdExist(BYTE* pCSN);
//extern BOOL IsAuthorizeCdExist(BYTE* pCSN);
extern BOOL IsValidCard(BYTE* pCSN);

extern BOOL isAHASHFILEExisted(void);

extern VOID ClearCSN();

extern BOOL CSNCompare(BYTE *pInCSN, BYTE *pTableCSN);
extern VOID UpdateASTfromAMT();


#ifdef DEBUG_AS_COMMON
extern VOID AsPrintCd(CDINFO* pCdInfo);
#endif

extern BYTE	JudgeCardInfo(CDINFO* pCdInfo);

extern VOID SaveCdPwdInfo2Mem();
#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
extern CDINFO_HV_t* AsGetCdHVinfobyIndex(UINT index);
#endif

/*
    卡                                                      卡数据结构
    MC添加的用户卡存放在AST文件中                           CDINFO
    HV和授权功能卡加的用户卡存放在AST_HV文件中              CDINFO_HV_t
    HV和IRIS Editor添加的授权功能卡存放在AAT文件中          CDINFO_HV_t
    GM添加的授权功能和用户卡                                当前这种方式存在问题，不使用
*/
extern BOOL isAST_HVFileExisted(void);
extern void LoadCdHVInfoFromFile(void);
extern void LoadAuthorizeCdHVInfoFromMem(void);
extern CDINFO_HV_t* AsGetCdHVInfobyCd(BYTE* pCSN);
extern BOOL AsRmCdHV(CHAR* pCSN);
extern void SaveCdHVInfo2Mem(void);
extern void SaveAuthorizeCdHVInfo2Mem(void);
extern BOOL AsAddCdHV(CDINFO_HV_t* pCdInfo);
extern BOOL AsAddAuthorizeCdHV(CDINFO_HV_t* pCdInfo);
extern BOOL IsAuthorizeCdHVExist(BYTE* pCSN);
extern void convertAuthCardFileData(void);
extern void convertNormalCardFileData(void);
extern BOOL AsAddAuthorizeCd(CDINFO_OLD* pCdInfo);
extern BOOL IsAuthorizeCdExist(BYTE* pCSN);
extern VOID SaveOrSynchronousCardHashInfo2Mem(void);
extern VOID LoadCardHashInfoFromMem(void);
extern VOID SaveCardHashInfo2Mem(void);

//从老的卡片转移到新的卡片存储机制
extern BOOL ConversionCdInfoOld2CdInfo(CDINFO *CdInfo,CDINFO_OLD*CdInfo_old);
extern BOOL UPdata_APT(void);
extern BOOL UPdata_AST( void );

//extern BOOL ConversionLfCard2HashLfCard(HASH_LFCARD *HashLfCard, LFCard*LfCard);


/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // ACCESSCOMMON_H
