/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AccessCommon.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		27 - May - 2008
**
**	FILE DESCRIPTION:
**
**
**	FUNCTIONS:
**
**	NOTES:
** 
*/
/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"

#include "AccessCommon.h"
#include "AccessProc.h"
#include "CardProc.h"
#include "rtc.h"
#include "PwdUlk.h"
#include "MenuParaProc.h"
#include "hash.h"
#include "EGMLCAgent.h"


/************** DEFINES **************************************************************/
//#define ACCESS_COMMON_DEBUG

/************** TYPEDEFS *************************************************************/

extern HANDLE*   g_HandleHashCard ;//卡片存储的句柄

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/
CPTINFO	g_CardPWDInfo = 
{	
	STATUS_OPEN,
		
	{
		PWD_VERSION,
			0
	},
	
	NULL
};

//static BYTE	g_CardType = TYPE_NORMAL_CARD;


#ifdef DEBUG_AS_COMMON
static VOID AsPrintAllCd();

#endif




/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadCdInfoFromMem
**	AUTHOR:		   Jeff Wang
**	DATE:		7 - July - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			AST version number is 0x00010103
*/



VOID
LoadCdInfoFromMem()
{
	CDINFO_OLD		CdInfo;
	CDINFO_OLD	*pCdInfo;

	FILE * fd			= NULL;
	FILE * fd2			= NULL;
	struct stat			pStat;
	memset(&g_ASInfo.ASTHead, 0, sizeof(ASTHEAD));
    if(NULL == g_ASInfo.MXASBuf) return;
	memset(g_ASInfo.MXASBuf, 0, MAX_AS_CD_CNT * sizeof(CDINFO_OLD));
	//printf("sizeof(CDINFO) = %d\n", sizeof(CDINFO));
	if ((fd = fopen(ASTFILE, "r+")) != NULL)
	{
		stat(ASTFILE, &pStat);
		fseek(fd, 0, SEEK_SET);
		fread(&g_ASInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);
        printf("load success,version : %08x\n",g_ASInfo.ASTHead.VersionNum);
		fseek(fd, 0, SEEK_CUR);
		fread(g_ASInfo.MXASBuf, pStat.st_size-sizeof(ASTHEAD), (size_t)1, fd);
		fclose(fd);
	}
	else
	{

		printf("AST file open error,create the file\n");
		if ((fd = fopen(ASTFILE, "w+")) != NULL)
		{
			g_ASInfo.ASTHead.VersionNum = AST_VERSION;
			g_ASInfo.ASTHead.nCardCnt = 0;
			printf("write success\n");
			fseek(fd, 0, SEEK_SET);
			fwrite(&g_ASInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);
			
			fclose(fd);
		}
		else
		{

			printf("AST file open error,Write error\n");
			return;
		}
	}
		
	if( g_ASInfo.ASTHead.VersionNum > AST_VERSION )		
	{	
		printf("Version error\n");
		return;
	}
#ifdef ACCESS_COMMON_DEBUG	
	printf("%s,Card count=%d\n",__FUNCTION__,g_ASInfo.ASTHead.nCardCnt);
#endif
	if (g_ASInfo.ASTHead.nCardCnt > MAX_AS_CD_CNT)
	{
		printf("Card count error\n");
		return;		
	}
}

VOID
LoadPatrolCdInfoFromMem(VOID)
{
	CDINFO_OLD		CdInfo;
	CDINFO_OLD		*pCdInfo;
	
	FILE * fd			= NULL;
	FILE * fd2			= NULL;
	struct stat			pStat;
	
	memset(&g_APInfo.ASTHead, 0, sizeof(ASTHEAD));
	memset(g_APInfo.MXASBuf, 0, MAX_AS_PCD_CNT * sizeof(CDINFO_OLD));

	if ((fd = fopen(APTFILE, "r+")) != NULL)
	{
		stat(APTFILE, &pStat);
		fseek(fd, 0, SEEK_SET);
		fread(&g_APInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);
		
		fseek(fd, 0, SEEK_CUR);
		fread(g_APInfo.MXASBuf, pStat.st_size-sizeof(ASTHEAD), (size_t)1, fd);
		
		fclose(fd);
	}
	else
	{
		
		printf("APT file open error,create the file\n");
		if ((fd = fopen(APTFILE, "w+")) != NULL)
		{
			g_APInfo.ASTHead.VersionNum = APT_VERSION;
			g_APInfo.ASTHead.nCardCnt = 0;
			
			fseek(fd, 0, SEEK_SET);
			fwrite(&g_APInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);
			
			fclose(fd);
		}
		else
		{
			
			printf("APT file open error,Write error\n");
			return;
		}
	}
	
	if( g_APInfo.ASTHead.VersionNum > APT_VERSION )		
	{	
		printf("Version error\n");
		return;
	}
#ifdef ACCESS_COMMON_DEBUG	
	printf("%s,Card count=%d\n",__FUNCTION__,g_APInfo.ASTHead.nCardCnt);
#endif
	if (g_APInfo.ASTHead.nCardCnt > MAX_AS_PCD_CNT)
	{
		printf("Card count error\n");
		return;		
	}
	
}

VOID
LoadAuthorizeCdInfoFromMem(VOID)
{
	CDINFO_OLD CdInfo;
	CDINFO_OLD *pCdInfo;
	
	FILE * fd			= NULL;
	FILE * fd2			= NULL;
	struct stat			pStat;
	
	memset(&g_AAInfo.ASTHead, 0, sizeof(ASTHEAD));
	memset(g_AAInfo.MXASBuf, 0, MAX_AS_ACD_CNT * sizeof(CDINFO_OLD));

	if ((fd = fopen(AATFILE, "r+")) != NULL)
	{
		stat(AATFILE, &pStat);
		fseek(fd, 0, SEEK_SET);
		fread(&g_AAInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);
		
		fseek(fd, 0, SEEK_CUR);
		fread(g_AAInfo.MXASBuf, pStat.st_size-sizeof(ASTHEAD), (size_t)1, fd);
		
		fclose(fd);
	}
	else
	{
		
		printf("AAT file open error,create the file\n");
		if ((fd = fopen(AATFILE, "w+")) != NULL)
		{
			g_AAInfo.ASTHead.VersionNum = AAT_VERSION;
			g_AAInfo.ASTHead.nCardCnt = 0;
			
			fseek(fd, 0, SEEK_SET);
			fwrite(&g_AAInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);
			
			fclose(fd);
		}
		else
		{
			
			printf("AAT file open error,Write error\n");
			return;
		}
	}
	
	if( g_AAInfo.ASTHead.VersionNum > AAT_VERSION )		
	{	
		printf("Version error\n");
		return;
	}
#ifdef ACCESS_COMMON_DEBUG	
	printf("%s,Card count=%d\n",__FUNCTION__,g_AAInfo.ASTHead.nCardCnt);
#endif
	if (g_AAInfo.ASTHead.nCardCnt > MAX_AS_ACD_CNT)
	{
		printf("Card count error\n");
		return;		
	}
	
}

BOOL
SaveCardInfo2Mem(int type)
{
	switch(type)
	{
		case STATUS_DEL_CARD:
			//SaveCdInfo2Mem();
			break;
		case STATUS_DEL_PATROL_CARD:
			//SavePatrolCdInfo2Mem();
			break;
		case STATUS_DEL_AUTHORIZE_CARD:
			SaveAuthorizeCdHVInfo2Mem();
			break;
		default:
			break;

	}
	return TRUE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SaveCdInfo2Mem
**	AUTHOR:		   Jeff Wang
**	DATE:		7 - July - 2008
**
**	DESCRIPTION:
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/

VOID 
SaveCdInfo2Mem()
{
	FILE * fd			= NULL;
    if(NULL == g_ASInfo.MXASBuf) return;
	if ((fd = fopen(ASTFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fwrite(&g_ASInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);

		fseek(fd, 0, SEEK_CUR);
		fwrite(g_ASInfo.MXASBuf, g_ASInfo.ASTHead.nCardCnt * sizeof(CDINFO_OLD), (size_t)1, fd);

#ifdef DEBUG_AS_COMMON		
		printf("Save:Card Count = %d\n", g_ASInfo.ASTHead.nCardCnt);
#endif		
		fclose(fd);
	}
	else
	{
		printf("AST file open error\n");
	}
}

VOID 
SavePatrolCdInfo2Mem(VOID)
{
	FILE * fd			= NULL;
	
	if ((fd = fopen(APTFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fwrite(&g_APInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);
		
		fseek(fd, 0, SEEK_CUR);
		fwrite(g_APInfo.MXASBuf, g_APInfo.ASTHead.nCardCnt * sizeof(CDINFO_OLD), (size_t)1, fd);
		
#ifdef DEBUG_AS_COMMON		
		printf("Save:Card Count = %d\n", g_APInfo.ASTHead.nCardCnt);
#endif		
		fclose(fd);
	}
	else
	{
		printf("APT file open error\n");
	}
}

VOID 
SaveAuthorizeCdInfo2Mem(VOID)
{
	FILE * fd			= NULL;
	
	if ((fd = fopen(AATFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fwrite(&g_AAInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);
		
		fseek(fd, 0, SEEK_CUR);
		fwrite(g_AAInfo.MXASBuf, g_AAInfo.ASTHead.nCardCnt * sizeof(CDINFO_OLD), (size_t)1, fd);
		
#ifdef DEBUG_AS_COMMON		
		printf("%s:Card Count = %d\n", __FUNCTION__,g_AAInfo.ASTHead.nCardCnt);
#endif		
		fclose(fd);
	}
	else
	{
		printf("AAT file open error\n");
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsLocalNumExist
**	AUTHOR:		   Jeff Wang
**	DATE:		10 - Sep - 2008
**
**	DESCRIPTION:	
**		Find whether the resient is exist in AMT
**
**	ARGUMENTS:		
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**	
*/
BOOL 
IsLocalNumExist(DWORD nLocalNum)
{
/*	CDINFO*				pCdInfo	= NULL;
	CHAR*				p = NULL;
	INT					nRdCount = 0;

	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
	
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return FALSE;
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt)
	{
		pCdInfo = (CDINFO*)p;
		if (pCdInfo->LocalNum == nLocalNum)
		{
			return TRUE;
		}			
		p += sizeof(CDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}	

	p = g_APInfo.MXASBuf;
	while (nRdCount < g_APInfo.ASTHead.nCardCnt)
	{
		pCdInfo = (CDINFO*)p;
		if (pCdInfo->LocalNum == nLocalNum)
		{
			return TRUE;
		}			
		p += sizeof(CDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}	*/
	return FALSE;
}

BOOL
IsValidCard(BYTE* pCSN)
{	
	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
	if(IsCdExist(pCSN))
	{
		return TRUE;
	}
	if(IsPatrolCdExist(pCSN))
	{
		return TRUE;
	}
    if(isAST_HVFileExisted())
    {
        if(IsAuthorizeCdHVExist(pCSN))
    	{
    		return TRUE;
    	}
    }
    else
    {
        if(IsAuthorizeCdExist(pCSN))
    	{
    		return TRUE;
    	}
    }
	return FALSE;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsCdExist
**	AUTHOR:			Jeff Wang
**	DATE:			14 - Oct - 2008
**
**	DESCRIPTION:	
**			Get one card information from table by card number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pCSN		[IN]		CHAR*
**	
**	RETURNED VALUE:	
**				Return TRUE if exist, otherwise FALSE
**	NOTES:
**			
*/
BOOL
IsCdExist(BYTE* pCSN)
{
	CDINFO_OLD*	pCdInfo		= NULL;
	CDINFO  pCdHashinfo;
	CHAR*	    p				= NULL;
	INT				nRdCount = 0;

	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);

	if(ReadCard(g_HandleHashCard,pCSN,TYPE_NORMAL_CARD,&pCdHashinfo) == FALSE)
	{
		DEBUG_CARD_PRINTF("%s,%d,TYPE_NORMAL_CARD Hash Card non-existed\r\n",__func__,__LINE__);
	}
	else
	{
		DEBUG_CARD_PRINTF("%s,%d,TYPE_NORMAL_CARDcard found\n",__func__,__LINE__); 
		return TRUE;
	}
/*  Old Version Not supported
 	p = g_ASInfo.MXASBuf;
       if(p == NULL) return FALSE;
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO_OLD*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			return TRUE;
		}
		p += sizeof(CDINFO_OLD);
		nRdCount++;
		pCdInfo = NULL;
	}
*/
	return FALSE;
}

BOOL
IsPatrolCdExist(BYTE* pCSN)
{
	CDINFO_OLD*	pCdInfo		= NULL;
	CDINFO pCdHashInfo;
	CHAR*	    p				= NULL;
	INT				nRdCount = 0;

	printf("%s,%d\r\n",__func__,__LINE__);

/**/
	if(ReadCard(g_HandleHashCard,pCSN,TYPE_PATROL_CARD,&pCdHashInfo) == FALSE)
	{
		printf("%s,%d,TYPE_PATROL_CARD Card non-existed\r\n",__func__,__LINE__);
	}
	else
	{
		printf("%s,%d,TYPE_PATROL_CARD found\n",__func__,__LINE__); 
		return TRUE;
	}
/*     Old Version Not supported

	p = g_APInfo.MXASBuf;

	while (nRdCount < g_APInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO_OLD*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			return TRUE;
		}
		p += sizeof(CDINFO_OLD);
		nRdCount++;
		pCdInfo = NULL;
	}
*/
	return FALSE;
}
/*
BOOL
IsAuthorizeCdExist(BYTE* pCSN)
{
	CDINFO*	pCdInfo		= NULL;
	CHAR*	    p				= NULL;
	INT				nRdCount = 0;
	
	p = g_AAInfo.MXASBuf;

	while (nRdCount < g_AAInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			return TRUE;
		}
		p += sizeof(CDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}

	return FALSE;
}
*/
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsCdInRoom
**	AUTHOR:			Mike Zhang
**	DATE:			17 - Jan - 2011
**
**	DESCRIPTION:	
**			Get one card information from table by card number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pCSN		[IN]		CHAR*
**	
**	RETURNED VALUE:	
**				Return TRUE if exist, otherwise FALSE
**	NOTES:
**			
*/
BOOL 
IsCdInRoom(BYTE* pRdCode)
{
	CDINFO*	pCdInfo		= NULL;
	CHAR*	    p				= NULL;
	INT				nRdCount = 0;
	
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return FALSE;
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		if (0 == strcmp(pRdCode, pCdInfo->RdNum))
		{
			return TRUE;
		}
		p += sizeof(CDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}

	return FALSE;
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsGroupNumExist
**	AUTHOR:		   Jeff Wang
**	DATE:		27 - April - 2009
**
**	DESCRIPTION:	
**		Find whether the Group Number is registered
**
**	ARGUMENTS:		
**				
**	RETURNED VALUE:	
**				TRUE if registered, else FALSE
**	NOTES:
**			
*/
BOOL 
IsGroupNumExist(BYTE GroupNum)
{
/*	CDINFO*				pCdInfo	= NULL;
	CHAR*				  p = NULL;
	INT						   nRdCount = 0;
	
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return FALSE;
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt)
	{
		pCdInfo = (CDINFO*)p;		
		if (GroupNum == pCdInfo->GroupNum)
		{
			return TRUE;
		}			
		p += sizeof(CDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}*/
	return FALSE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsAddCd
**	AUTHOR:			Jeff Wang
**	DATE:			10 - Oct - 2008
**
**	DESCRIPTION:	
**			Add one card to resident
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**						[IN]		const MXMSG*
**	RETURNED VALUE:	
**				TRUE if succeed, otherwise FALSE
**	NOTES:
**			
*/
BOOL
AsAddCd(CDINFO_OLD* pCdInfo)
{
	CDINFO p_newCdInfo;
	if(pCdInfo->CardType == TYPE_NORMAL_CARD || pCdInfo->CardType == TYPE_PATROL_CARD )
		{
		if(ConversionCdInfoOld2CdInfo(&p_newCdInfo,pCdInfo))
		{
			AsPrintCd(&p_newCdInfo);
			if(IsCdExist(&p_newCdInfo.CSN))
			{
				printf("Card is exist\n");
				return FALSE;
			}
			if (FALSE == WriteCardInfo(g_HandleHashCard,&p_newCdInfo,p_newCdInfo.CardType))
			{
				printf("%s,%d,File ADD One new  Card\n",__func__,__LINE__);		
				return FALSE;
			}

			printf("%s,%d,Sucess ADD One new  Card\n",__func__,__LINE__);	
	    	return TRUE;
		}
	}
	printf("%s,%d,File ADD One new Card,This card : %d\n",__func__,__LINE__,pCdInfo->CardType);	

			if(g_ASInfo.MXASBuf == NULL) return FALSE;
			if (g_ASInfo.ASTHead.nCardCnt + 1 < MAX_AS_CD_CNT)
			{
#ifdef ACCESS_COMMON_DEBUG			
				printf("%s success\n",__FUNCTION__);
#endif
				memcpy(g_ASInfo.MXASBuf + g_ASInfo.ASTHead.nCardCnt * sizeof(CDINFO_OLD),
					pCdInfo, sizeof (CDINFO_OLD));
				g_ASInfo.ASTHead.nCardCnt++;
				return TRUE;
			}
			else
			{
				printf("Card Info buffer full\n");
				return FALSE;
			}
		
}

BOOL
AsAddPatrolCd(CDINFO_OLD* pCdInfo)
{

	CDINFO p_newCdInfo;
	if(pCdInfo->CardType == TYPE_PATROL_CARD)
	{
		if(ConversionCdInfoOld2CdInfo(&p_newCdInfo,pCdInfo))
		{
			AsPrintCd(&p_newCdInfo);
			if(IsPatrolCdExist(&p_newCdInfo.CSN))
			{
				printf("Card is exist\n");
				return FALSE;
			}
			if (FALSE == WriteCardInfo(g_HandleHashCard,&p_newCdInfo,p_newCdInfo.CardType))
			{
				printf("%s,%d,File ADD One new PatrolCd Card\n",__func__,__LINE__);		
				return FALSE;
			}

			printf("%s,%d,Sucess ADD One new PatrolCd Card\n",__func__,__LINE__);	
	    	return TRUE;
		}
	}
	printf("%s,%d,File ADD One new PatrolCd Card,This card : %d\n",__func__,__LINE__,pCdInfo->CardType);	
	return FALSE;
/*
	if(IsPatrolCdExist(pCdInfo->CSN))
	{
		printf("Patrol Card is exist\n");
		return FALSE;
	}
	
	if (g_APInfo.ASTHead.nCardCnt + 1 < MAX_AS_PCD_CNT)
	{
#ifdef ACCESS_COMMON_DEBUG			
		printf("%s success\n",__FUNCTION__);
#endif
		memcpy(g_APInfo.MXASBuf + g_APInfo.ASTHead.nCardCnt * sizeof(CDINFO_OLD),
			pCdInfo, sizeof (CDINFO_OLD));
		g_APInfo.ASTHead.nCardCnt++;
		return TRUE;
	}
	else
	{
		printf("Patrol Card Info buffer full\n");
		return FALSE;
	}
*/

}

/*
BOOL
AsAddAuthorizeCd(CDINFO* pCdInfo)
{
	if(IsAuthorizeCdExist(pCdInfo->CSN))
	{
		printf("Card is exist\n");
        AsRmCd(pCdInfo->CSN);
        AsRmPatrolCd(pCdInfo->CSN);
		return FALSE;
	}
	
	if (g_AAInfo.ASTHead.nCardCnt + 1 < MAX_AS_ACD_CNT)
	{
#ifdef ACCESS_COMMON_DEBUG		
		printf("%s success\n",__FUNCTION__);
#endif
		memcpy(g_AAInfo.MXASBuf + g_AAInfo.ASTHead.nCardCnt * sizeof(CDINFO),
			pCdInfo, sizeof (CDINFO));
		g_AAInfo.ASTHead.nCardCnt++;
        AsRmCd(pCdInfo->CSN);
        AsRmPatrolCd(pCdInfo->CSN);
		return TRUE;
	}
	else
	{
		printf("Card Info buffer full\n");
		return FALSE;
	}

}
*/

INT
RmCard(CHAR* pCSN)
{
	if(AsRmCd(pCSN))
	{
		return STATUS_DEL_CARD;
	}
	if(AsRmPatrolCd(pCSN))
	{
		return STATUS_DEL_PATROL_CARD;
	}
	if(AsRmAuthorizeCdHV(pCSN))
	{
		return STATUS_DEL_AUTHORIZE_CARD;
	}
	return -1;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsRmCd
**	AUTHOR:			Jeff Wang
**	DATE:			10 - Oct - 2008
**
**	DESCRIPTION:	
**			Remove one card
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pCSN		  [IN]		CHAR*
**
**	RETURNED VALUE:	
**				TRUE if succeed, otherwise FALSE
**	NOTES:
**			
*/
BOOL
AsRmCd(CHAR* pCSN)
{
	CDINFO*  pCdInfo = NULL;
	CHAR*	   p = NULL;
	INT            nRdCount = 0;

	if(DeleteCard(g_HandleHashCard,pCSN,TYPE_NORMAL_CARD))
		{

		DEBUG_CARD_PRINTF("%s__%d DeleteCard success! ",__func__,__LINE__);

	}
	DEBUG_CARD_PRINTF("%s__%d faile success! ",__func__,__LINE__);

	return FALSE;
/*
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return FALSE;
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			while (nRdCount + 1 < g_ASInfo.ASTHead.nCardCnt)
			{
				memcpy(p, p + sizeof(CDINFO), sizeof(CDINFO));
				p += sizeof(CDINFO);
				nRdCount++;
			}
			g_ASInfo.ASTHead.nCardCnt--;			
			return TRUE;
		}
		else
		{
			p += sizeof(CDINFO);
			nRdCount++;
			pCdInfo = NULL;
		}
	}
	return FALSE;	
	*/
}

BOOL
AsRmPatrolCd(CHAR* pCSN)
{
	CDINFO_OLD*  pCdInfo = NULL;
	CHAR*	   p = NULL;
	INT            nRdCount = 0;


	if(DeleteCard(g_HandleHashCard,pCSN,TYPE_PATROL_CARD))
		{

		DEBUG_CARD_PRINTF("%s__%d DeleteCard success! ",__func__,__LINE__);

	}
	DEBUG_CARD_PRINTF("%s__%d faile success! ",__func__,__LINE__);

return FALSE;


/*	
	p = g_APInfo.MXASBuf;
	while (nRdCount < g_APInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO_OLD*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			while (nRdCount + 1 < g_APInfo.ASTHead.nCardCnt)
			{
				memcpy(p, p + sizeof(CDINFO_OLD), sizeof(CDINFO_OLD));
				p += sizeof(CDINFO_OLD);
				nRdCount++;
			}
			g_APInfo.ASTHead.nCardCnt--;			
			return TRUE;
		}
		else
		{
			p += sizeof(CDINFO_OLD);
			nRdCount++;
			pCdInfo = NULL;
		}
	}
	return FALSE;	
	*/
}

/*
BOOL
AsRmAuthorizeCd(CHAR* pCSN)
{
	CDINFO*  pCdInfo = NULL;
	CHAR*	   p = NULL;
	INT            nRdCount = 0;
	
	p = g_AAInfo.MXASBuf;
	while (nRdCount < g_AAInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			while (nRdCount + 1 < g_AAInfo.ASTHead.nCardCnt)
			{
				memcpy(p, p + sizeof(CDINFO), sizeof(CDINFO));
				p += sizeof(CDINFO);
				nRdCount++;
			}
			g_AAInfo.ASTHead.nCardCnt--;			
			return TRUE;
		}
		else
		{
			p += sizeof(CDINFO);
			nRdCount++;
			pCdInfo = NULL;
		}
	}
	return FALSE;	
}
*/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsGetCdInfobyCd
**	AUTHOR:			Jeff Wang
**	DATE:			10 - Oct - 2008
**
**	DESCRIPTION:	
**			Get one card information from table by card number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pCSN		[IN]		BYTE*
**	
**	RETURNED VALUE:	
**				Return card information if succeed, otherwise NULL
**	NOTES:
**			
*/
CDINFO_OLD*
AsGetCdInfobyCd(BYTE* pCSN)
{
	CDINFO_OLD*	pCdInfo	= NULL;
	CHAR*	p = NULL;
	INT		nRdCount = 0;
	int i;

	for(i=0; i < CSN_LEN; i++)
	{
		printf(" %02x", pCSN[i]);
	}
	
	printf("\n");
	//access card	
#ifdef ACCESS_COMMON_DEBUG		
	printf("access card count:%d\n",g_ASInfo.ASTHead.nCardCnt);
#endif
	p = g_ASInfo.MXASBuf;
	if(p == NULL) return pCdInfo;
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO_OLD*)p;
		if (pCdInfo)
		{            
			if (CSNCompare(pCSN, pCdInfo->CSN))
			{
#ifdef ACCESS_COMMON_DEBUG		
			printf("find access card,type=%d \n",pCdInfo->CardType);
#endif				
				if(pCdInfo->CardType != TYPE_NORMAL_CARD)
				{
					printf("==========%s,type error:%d not equal TYPE_NORMAL_CARD===============\n",__FUNCTION__,pCdInfo->CardType);
				}

				return pCdInfo;
			}
		}
		p += sizeof(CDINFO_OLD);
		nRdCount++;
		pCdInfo = NULL;
	}
	//patrol card
#ifdef ACCESS_COMMON_DEBUG		
	printf("patrol card count:%d\n",g_APInfo.ASTHead.nCardCnt);
#endif
	nRdCount = 0;

	p = g_APInfo.MXASBuf;
    if(p == NULL) return pCdInfo;
	while (nRdCount < g_APInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO_OLD*)p;
		if (pCdInfo)
		{
			if (CSNCompare(pCSN, pCdInfo->CSN))
			{
#ifdef ACCESS_COMMON_DEBUG		
			printf("find patrol card,type=%d \n",pCdInfo->CardType);
#endif				
				if(pCdInfo->CardType != TYPE_PATROL_CARD)
				{
					printf("==========%s,type error:%d not equal TYPE_PATROL_CARD===============\n",__FUNCTION__,pCdInfo->CardType);
				}

				return pCdInfo;
			}
		}
		p += sizeof(CDINFO_OLD);
		nRdCount++;
		pCdInfo = NULL;
	}

	//Authorize card
#ifdef ACCESS_COMMON_DEBUG		
		printf("authorize card count:%d\n",g_AAInfo.ASTHead.nCardCnt);
#endif
		nRdCount = 0;
		p = g_AAInfo.MXASBuf;
        if(p == NULL) return pCdInfo;
		while (nRdCount < g_AAInfo.ASTHead.nCardCnt) 
		{

			pCdInfo = (CDINFO_OLD*)p;
			if (pCdInfo)
			{
				if (CSNCompare(pCSN, pCdInfo->CSN))
				{
#ifdef ACCESS_COMMON_DEBUG		
			printf("find authorize card,type=%d \n",pCdInfo->CardType);
#endif					
					if(pCdInfo->CardType != TYPE_AUTHORIZE_CARD)
					{
						printf("==========%s,type error:%d not equal TYPE_AUTHORIZE_CARD===============\n",__FUNCTION__,pCdInfo->CardType);
					}
					return (pCdInfo);
				}
			}
			p += sizeof(CDINFO_OLD);
			nRdCount++;
			pCdInfo = NULL;
		}

	
	return NULL;
}	





/*
CDINFO*
AsGetPatrolCdInfobyCd(CHAR* pCSN)
{
	CDINFO*	pCdInfo	= NULL;
	CHAR*	   p = NULL;
	INT            nRdCount = 0;
	
	p = g_APInfo.MXASBuf;
	
	while (nRdCount < g_APInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			if (pCdInfo)
			{
				return pCdInfo;
			}
		}
		p += sizeof(CDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}
	
	return NULL;
}	
*/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsGetCdInfobyLocalNum
**	AUTHOR:			Jeff Wang
**	DATE:			10 - Oct - 2008
**
**	DESCRIPTION:	
**			Get one card information from table by ID
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nLocalNum		[IN]		WORD
**
**	RETURNED VALUE:	
**				Return card information if succeed, otherwise NULL
**	NOTES:
**			
*/
CDINFO*
AsGetCdInfobyLocalNum(DWORD nLocalNum)
{
	CDINFO*	pCdInfo	= NULL;
/*	CHAR*	   p = NULL;
	INT            nRdCount = 0;
	
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return FALSE;
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		if (nLocalNum == pCdInfo->LocalNum)
		{
			if (pCdInfo)
			{
				return pCdInfo;
			}
		}
		p += sizeof(CDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}
*/
	return NULL;
}	


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsRmAllCd
**	AUTHOR:			Jeff Wang
**	DATE:			10 - Oct - 2008
**
**	DESCRIPTION:	
**			Remove all resident
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
VOID
AsRmAllCd()
{
    if(g_ASInfo.MXASBuf == NULL) return;
	memset(g_ASInfo.MXASBuf, 0, g_ASInfo.ASTHead.nCardCnt * sizeof(CDINFO));
	g_ASInfo.ASTHead.nCardCnt = 0;	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsRmCdByRdNum
**	AUTHOR:			Jeff Wang
**	DATE:			27 - April - 2009
**
**	DESCRIPTION:	
**			Remove Cards by Resident's Number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
INT
AsRmCdByRdNum(CHAR* pRdNum, INT nRdNumLen)
{
	CDINFO*		pCdInfo	= NULL;
	CHAR*		p = NULL;
	CHAR*		p1 = NULL;
	INT         nRdCount = 0;
	INT         nRdCount1 = 0;
	INT			nCount = 0;
	
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return pCdInfo;
	nRdNumLen = strlen(pRdNum);
	
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		if (CodeCompare(pRdNum, pCdInfo->RdNum, nRdNumLen))
		{
			nCount++;
			p1 = p;
			nRdCount1 = nRdCount;
			while (nRdCount1 + 1 < g_ASInfo.ASTHead.nCardCnt) 
			{
				memcpy(p1, p1+sizeof(CDINFO), sizeof(CDINFO));
				p1 += sizeof(CDINFO);
				nRdCount1++;
			}
			g_ASInfo.ASTHead.nCardCnt--;
		}
		else
		{
			p += sizeof(CDINFO);
			nRdCount++;
			pCdInfo = NULL;
		}
	}

	if (nCount > 0)
	{
		return TYPE_NORMAL_CARD	;
	}
/*
	nCount = 0;
	while (nRdCount < g_APInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		if (CodeCompare(pRdNum, pCdInfo->RdNum, nRdNumLen))
		{
			nCount++;
			p1 = p;
			nRdCount1 = nRdCount;
			while (nRdCount1 + 1 < g_APInfo.ASTHead.nCardCnt) 
			{
				memcpy(p1, p1+sizeof(CDINFO), sizeof(CDINFO));
				p1 += sizeof(CDINFO);
				nRdCount1++;
			}
			g_APInfo.ASTHead.nCardCnt--;
		}
		else
		{
			p += sizeof(CDINFO);
			nRdCount++;
			pCdInfo = NULL;
		}
	}

	if (nCount > 0)
	{
		return TYPE_PATROL_CARD;
	}
*/
	return 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsRmCdbyLocalNum
**	AUTHOR:			Jeff Wang
**	DATE:			27 - April - 2009
**
**	DESCRIPTION:	
**			Remove one card information from table by ID
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nLocalNum	[IN]		WORD
**
**	RETURNED VALUE:	
**				Return TRUE if succeed, otherwise FALSE
**	NOTES:
**			
*/
BOOL
AsRmCdbyLocalNum(DWORD nLocalNum)
{
/*	CDINFO*	pCdInfo	= NULL;
	CHAR*	   p = NULL;
	INT            nRdCount = 0;
	CHAR*	   p1 = NULL;
	INT            nRdCount1 = 0;
	
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return FALSE;
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		if (nLocalNum == pCdInfo->LocalNum)
		{
			p1	=	p;
			nRdCount1	=	nRdCount;
			while (nRdCount1 + 1 < g_ASInfo.ASTHead.nCardCnt)
			{
				memcpy(p1, p1 + sizeof(CDINFO), sizeof(CDINFO));
				p1 += sizeof(CDINFO);
				nRdCount1++;
			}
			g_ASInfo.ASTHead.nCardCnt--;

			return TRUE;
		}
		p += sizeof(CDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}
*/
	return FALSE;
}	


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsPrintAllCd
**	AUTHOR:			Jeff Wang
**	DATE:			07 - Apr - 2009
**
**	DESCRIPTION:	
**			Print all module information
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
#ifdef DEBUG_AS_COMMON
static VOID
AsPrintAllCd()
{
	CDINFO*	pCdInfo	= NULL;
	CHAR*	   p = NULL;
	INT            nRdCount = 0;
	INT				i = 0;

	printf("As: Cd Info Debug ....\n");
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return;
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt)
	{
		pCdInfo = (CDINFO*) p;

		for(i=0; i<CSN_LEN; i++)
		{
			printf(" %02x", pCdInfo->CSN[i]);
		}
		printf("\n");
//		printf("Card Local Num: %d\n", pCdInfo->LocalNum);
		printf("Rd Num: %s\n", pCdInfo->RdNum);
		printf("Card Type: %d\n", pCdInfo->CardType);
		printf("Card Status: %d\n", pCdInfo->CardStatus);
//		printf("Card VldStartTime: 0x%4x\n", pCdInfo->ValidStartTime);
//		printf("Card VldEndTime:  0x%4x\n", pCdInfo->ValidEndTime);

		p += sizeof(CDINFO);
		nRdCount++;
	}
	printf("As: Cd Info Debug ....End\n");
}
#endif


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsPrintCd
**	AUTHOR:			Jeff Wang
**	DATE:			07 - Apr - 2009
**
**	DESCRIPTION:	
**			Print all module information
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
VOID
AsPrintCd(CDINFO* pCdInfo)
{
#ifdef DEBUG_AS_COMMON	
	INT				i = 0;

	if (NULL == pCdInfo)
	{
		return;
	}

	for(i=0; i < CSN_LEN; i++)
	{
		printf(" %02x", pCdInfo->CSN[i]);
	}
	printf("\n");

	printf(" Card Password: %s\n Rd Num: %s\n Card Type: %d\n Card Status: %d\n",
				pCdInfo->UlkPwd,
				pCdInfo->RdNum,
				pCdInfo->CardType,
				pCdInfo->CardStatus);
#endif	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CSNCompare
**	AUTHOR:			Jeff Wang
**	DATE:		21 - Oct - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/
BOOL 
CSNCompare(BYTE *pInCode, BYTE *pTableCode)
{
	INT	i = 0;
#ifdef __SUPPORT_WIEGAND_CARD_READER__
	INT CompareIndex = 0;
#endif

	if ((NULL == pInCode)||(NULL == pTableCode))
	{
		printf("Lenth Error\n");
		return FALSE;
	}
#ifdef __SUPPORT_WIEGAND_CARD_READER__
	
	if(g_ASPara.WiegandBitNum > 0 && g_ASPara.WiegandBitNum < 40)
	{
        CompareIndex = (40 - g_ASPara.WiegandBitNum)/8;
	}
	for(i = CompareIndex; i < CSN_LEN; i++)
#else
	for(i = 0; i < CSN_LEN; i++)
#endif
	{
		if (pInCode[i] != pTableCode[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	JudgeCardInfo
**	AUTHOR:			Jeff Wang
**	DATE:		11 - May - 2009
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				pCdInfo		[IN]		CDINFO*
**
**	RETURNED VALUE:	
**				0: Card Authorized
**				1: Expired
**				2: Out of Time Slice
**				3: Card disabled
**	NOTES:
**		Whether Card is in the time slice is not designed yet	
*/
BYTE 
JudgeCardInfo(CDINFO* pCdInfo)
{
	time_t tmCurTime = 0;

	U32 ValidStartTime = (U32)(U32)(pCdInfo->ValidStartTime[0] | 
									pCdInfo->ValidStartTime[1]<<8 | 
									pCdInfo->ValidStartTime[2]<<16 | 
									pCdInfo->ValidStartTime[3]<<24);
	U32 ValidEndTime	=(U32)(U32)(pCdInfo->ValidEndTime[0] | 
									pCdInfo->ValidEndTime[1]<<8 | 
									pCdInfo->ValidEndTime[2]<<16 | 
									pCdInfo->ValidEndTime[3]<<24);

	tmCurTime = GetRtcTime();
	if (CARD_STATUS_DISABLED == pCdInfo->CardStatus) 
	{
		return CARD_INFO_DISABLED;
	}
	else if ((tmCurTime > ValidEndTime && 0 != ValidEndTime)
			|| (tmCurTime < ValidStartTime && 0 != ValidStartTime)) 
	{
		return CARD_INFO_EXPIRED;
	}
	else
	{
		return CARD_INFO_AUTHORIZED;
	}
}




/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UpdateASTfromAMT
**	AUTHOR:			Mike Zhang
**	DATE:		28 - Jul - 2010
**
**	DESCRIPTION:	
**		Update resident information acording to AMT
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/
VOID 
UpdateASTfromAMT()
{
	INT		nCountEHV = 0;	
	CDINFO*	pCdInfo	= NULL;
	BOOL	bFind = FALSE;
	INT		i              = 0;
	INT		nRdCount = 0;
	CHAR	*p = NULL;
	CHAR*	p1 = NULL;
	INT     nRdCount1 = 0;
	
	//char    ASTBuffer[1024] = {0};	
	char  *  pASTBuffer=NULL;
	nCountEHV = FdEHVfromAMT(&pASTBuffer);
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return;
	//Delete inexistent resident in the AST
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		
		for(i = 0; i < nCountEHV; i++)
		{
			if(CodeCompare(pASTBuffer + i * RD_CODE_LEN, pCdInfo->RdNum, strlen(pASTBuffer + i * RD_CODE_LEN)))
			{
				bFind = TRUE;
				break;
			}
		}
		if (!bFind) 
		{
			p1 = p;
			nRdCount1 = nRdCount;
			while (nRdCount1 + 1 < g_ASInfo.ASTHead.nCardCnt) 
			{
				memcpy(p1, p1+sizeof(CDINFO), sizeof(CDINFO));
				p1 += sizeof(CDINFO);
				nRdCount1++;
			}
			g_ASInfo.ASTHead.nCardCnt--;
		}
		else
		{
			p += sizeof(CDINFO);
			nRdCount++;
			pCdInfo = NULL;
			bFind = FALSE;
		}
		
	}
	free(pASTBuffer);
	SaveCdInfo2Mem();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SaveCdPwdInfo2Mem
**	AUTHOR:		   Mike Zhang
**	DATE:		4 - Jan - 2011
**
**	DESCRIPTION:
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/

VOID 
SaveCdPwdInfo2Mem()
{
	FILE * fd			= NULL;
	
	if ((fd = fopen(CPTFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fwrite(&g_CardPWDInfo.PWDHead, sizeof(CDPWDHEAD), (size_t)1, fd);
		
		fseek(fd, 0, SEEK_CUR);
		fwrite(g_CardPWDInfo.MXPWDBuf, g_CardPWDInfo.PWDHead.nCardCnt * sizeof(CDPWDINFO), (size_t)1, fd);
		
#ifdef DEBUG_AS_COMMON		
		printf("Save:Card Password Count = %d\n", g_CardPWDInfo.PWDHead.nCardCnt);
#endif		
		fclose(fd);
	}
	else
	{
		printf("CPT file open error\n");
	}
}	



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsAddPwd
**	AUTHOR:			Mike Zhang
**	DATE:			04 - Jan - 2011
**
**	DESCRIPTION:	
**			Add one card to resident
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**						[IN]		const MXMSG*
**	RETURNED VALUE:	
**				TRUE if succeed, otherwise FALSE
**	NOTES:
**			
*/
BOOL
AsAddPwd(CDPWDINFO* pCdInfo)
{
	
	if (g_CardPWDInfo.PWDHead.nCardCnt + 1 < MAX_AS_CD_CNT)
	{
		memcpy(g_CardPWDInfo.MXPWDBuf + g_CardPWDInfo.PWDHead.nCardCnt * sizeof(CDPWDINFO),
			pCdInfo, sizeof (CDPWDINFO));
		g_CardPWDInfo.PWDHead.nCardCnt++;
		SaveCdPwdInfo2Mem();
		return TRUE;
	}
	else
	{
		printf("Card Password Info buffer full\n");
		return FALSE;
	}
	
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsRmCdPwd
**	AUTHOR:			Mike Zhang
**	DATE:			04 - Jan - 2011
**
**	DESCRIPTION:	
**			Remove one card
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pCSN		  [IN]		CHAR*
**
**	RETURNED VALUE:	
**				TRUE if succeed, otherwise FALSE
**	NOTES:
**			
*/
BOOL
AsRmCdPwd(CHAR* pCSN)
{
	CDPWDINFO*  pCdInfo = NULL;
	CHAR*	   p = NULL;
	INT        nRdCount = 0;
	
	p = g_CardPWDInfo.MXPWDBuf;
	
#ifdef CARD_DEBUG	
	printf("AsRmCd CardCnt = %d\n",g_CardPWDInfo.PWDHead.nCardCnt);
#endif
	while (nRdCount < g_CardPWDInfo.PWDHead.nCardCnt) 
	{
		pCdInfo = (CDPWDINFO*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			while (nRdCount + 1 < g_ASInfo.ASTHead.nCardCnt)
			{
				memcpy(p, p + sizeof(CDPWDINFO), sizeof(CDPWDINFO));
				p += sizeof(CDPWDINFO);
				nRdCount++;
			}
			g_CardPWDInfo.PWDHead.nCardCnt--;
			SaveCdPwdInfo2Mem();
			
#ifdef CARD_DEBUG			
			printf("RM a Card PWD pCSN = %s\n",pCSN);
#endif
			return TRUE;
		}
		else
		{
			p += sizeof(CDPWDINFO);
			nRdCount++;
			pCdInfo = NULL;
		}
	}
	return FALSE;	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsModCdPwdbyCsn
**	AUTHOR:			Mike Zhang
**	DATE:			10 - Jan - 2011
**
**	DESCRIPTION:	
**			Get one card information from table by card number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pCSN		[IN]		BYTE*
**	
**	RETURNED VALUE:	
**				Return card information if succeed, otherwise NULL
**	NOTES:
**			
*/
BOOL
AsModCdPwdbyCsn(BYTE* pCSN, INT nCSNLen,CHAR* pNewPwd, INT nPwdLen)
{
	CDPWDINFO*	pCdInfo	= NULL;
	CDPWDINFO	CdInfo = {0};
	CHAR*	   p = NULL;
	INT        nRdCount = 0;
	int        nRet = 0;
	
	p = g_CardPWDInfo.MXPWDBuf;
	
	while (nRdCount < g_CardPWDInfo.PWDHead.nCardCnt) 
	{
		pCdInfo = (CDPWDINFO*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			if (pCdInfo)
			{
				memset(pCdInfo->UlkPwd,0,PWD_LEN);
				memcpy(pCdInfo->UlkPwd,pNewPwd,nPwdLen);
				nRet = 1;
				break;
			}
		}
		p += sizeof(CDPWDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}
	if (nRet) 
	{
		SaveCdPwdInfo2Mem();		
	}
	else
	{
		memset(CdInfo.CSN,0,CSN_LEN);
		memcpy(CdInfo.CSN,pCSN,nCSNLen);
		memset(CdInfo.UlkPwd,0,PWD_LEN);
		memcpy(CdInfo.UlkPwd,pNewPwd,nPwdLen);
		AsAddPwd(&CdInfo);
	}
	
	return TRUE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsDelCdPwdbyRd
**	AUTHOR:			Mike Zhang
**	DATE:			04 - Jan - 2011
**
**	DESCRIPTION:	
**			Get one card information from table by card number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pCSN		[IN]		CHAR*
**	
**	RETURNED VALUE:	
**				Return card information if succeed, otherwise NULL
**	NOTES:
**			
*/
BOOL
AsDelCdPwdbyRd(CHAR* pRdCode, INT nRdNumLen)
{
	CDINFO*	pCdInfo	= NULL;
	CHAR*	   p = NULL;
	INT        nRdCount = 0;
	int        nRet = 0;
		
	p = g_ASInfo.MXASBuf;
    if(p == NULL) return FALSE;
	nRdNumLen = strlen(pRdCode);
	
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO*)p;
		if (CodeCompare(pRdCode, pCdInfo->RdNum, nRdNumLen))
		{
			if (pCdInfo)
			{
				AsRmCdPwd(pCdInfo->CSN);
				nRet = 1;
			}
		}
		p += sizeof(CDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}
	
	return nRet;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsCdPwdExist
**	AUTHOR:			Mike Zhang
**	DATE:			04 - Jan - 2011
**
**	DESCRIPTION:	
**			Get one card information from table by card number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pCSN		[IN]		CHAR*
**	
**	RETURNED VALUE:	
**				Return TRUE if exist, otherwise FALSE
**	NOTES:
**			
*/
BOOL
IsCdPwdExist(BYTE* pCSN, CHAR *PwdStr)
{
	CDPWDINFO*	pCdInfo	= NULL;
	CHAR*	    p		= NULL;
	INT		nRdCount    = 0;
	
	p = g_CardPWDInfo.MXPWDBuf;
	
	while (nRdCount < g_CardPWDInfo.PWDHead.nCardCnt) 
	{
		pCdInfo = (CDPWDINFO*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			memcpy(PwdStr,pCdInfo->UlkPwd,PWD_LEN);
			return TRUE;
		}
		p += sizeof(CDPWDINFO);
		nRdCount++;
		pCdInfo = NULL;
	}
	
	return FALSE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BOOL UkPwdCompare
**	AUTHOR:			Mike Zhang
**	DATE:		29 - Mar - 2010
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/
BOOL 
UkPwdCompare(CHAR *pSavePwd,CHAR *pIputPwd ,INT InPwdLen)
{
	int SavePwdLen = 0;
	
	if ((NULL == pIputPwd)||(NULL == pSavePwd))
	{
		printf("Lenth 0\n");
		return 0;
	}
	
	SavePwdLen = strlen(pSavePwd);
	printf("InPwdLen = %d\n", InPwdLen);
	printf("SavePwdLen = %d\n", SavePwdLen);
	printf("pIputPwd = %s\n", pIputPwd);
	printf("pSavePwd = %s\n",pSavePwd);
	
	if (SavePwdLen != InPwdLen) 
	{
		printf("Len Error\n");
		return 0;
	}
		
	if (0 == memcmp(pIputPwd, pSavePwd, InPwdLen))
	{
		printf("UkPwdCompare OK\n");	
		return 1;
	}
	else
	{
		printf("UkPwdComparecompare Fail\n");
		return 0;
	}
}

#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
CDINFO_HV_t* AsGetCdHVinfobyIndex(UINT index)//OK
{
	
	CDINFO_HV_t*	pCdInfo	= NULL;
	CHAR*	        p = NULL;
	INT             nRdCount = 0;
	UINT	        offset = 0;
    if(g_AS_HVInfo.MXASBuf == NULL) return pCdInfo;
	if(index < g_AS_HVInfo.ASTHead.nCardCnt)
	{
		p = g_AS_HVInfo.MXASBuf;
		offset = sizeof(CDINFO_HV_t)*(g_AS_HVInfo.ASTHead.nCardCnt - index - 1);

	}
	else if(index < g_AS_HVInfo.ASTHead.nCardCnt + g_AAInfo.ASTHead.nCardCnt)
	{
		p = g_AAInfo.MXASBuf;
		offset = sizeof(CDINFO_HV_t)*(g_AS_HVInfo.ASTHead.nCardCnt + g_AAInfo.ASTHead.nCardCnt - index - 1);
	}

	pCdInfo = (CDINFO_HV_t*)(p + offset);	
	return pCdInfo;
}
#endif

/************************************************************************************************
**FunctionName    : AsRmAuthorizeCdHV
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BOOL AsRmAuthorizeCdHV(CHAR* pCSN)
{
	CDINFO_HV_t*    pCdInfo = NULL;
	CHAR*	        p = NULL;
	INT             nRdCount = 0;
	
	p = g_AAInfo.MXASBuf;
	while (nRdCount < g_AAInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO_HV_t*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			while (nRdCount + 1 < g_AAInfo.ASTHead.nCardCnt)
			{
				memcpy(p, p + sizeof(CDINFO_HV_t), sizeof(CDINFO_HV_t));
				p += sizeof(CDINFO_HV_t);
				nRdCount++;
			}
			g_AAInfo.ASTHead.nCardCnt--;			
			return TRUE;
		}
		else
		{
			p += sizeof(CDINFO_HV_t);
			nRdCount++;
			pCdInfo = NULL;
		}
	}
	return FALSE;	
}

/************************************************************************************************
**FunctionName    : SaveAuthorizeCdHVInfo2Mem
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
void SaveAuthorizeCdHVInfo2Mem(void)
{
	FILE * fd = NULL;
	
	if ((fd = fopen(AATFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
        g_AAInfo.ASTHead.VersionNum = AAT_VERSION;
		fwrite(&g_AAInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);		
		fseek(fd, 0, SEEK_CUR);
		fwrite(g_AAInfo.MXASBuf, g_AAInfo.ASTHead.nCardCnt * sizeof(CDINFO_HV_t), (size_t)1, fd);		
		fclose(fd);
	}
	else
	{
		printf("AAT file open error\n");
	}
}

/************************************************************************************************
**FunctionName    : LoadAuthorizeCdHVInfoFromMem
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
void LoadAuthorizeCdHVInfoFromMem(void)
{	
	FILE* Fd = NULL;
	if((Fd = fopen(AATFILE,"r+")) != NULL)
	{
		fseek(Fd,0,SEEK_SET);
		fread(&g_AAInfo.ASTHead,sizeof(ASTHEAD),(size_t)1,Fd);		
		fseek(Fd,0,SEEK_CUR);
		fread(g_AAInfo.MXASBuf,g_AAInfo.ASTHead.nCardCnt * sizeof(CDINFO_HV_t),(size_t)1,Fd);		
		fclose(Fd);
	}
    else
    {
        printf("AATFILE open failed!\n");
    }
}

/************************************************************************************************
**FunctionName    : AsRmCdHV
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BOOL AsRmCdHV(CHAR* pCSN)//OK
{
	CDINFO_HV_t*    pCdInfo = NULL;
	CHAR*	        p = NULL;
	INT             nRdCount = 0;
	
	p = g_AS_HVInfo.MXASBuf;
    if(p == NULL) return FALSE;
	while (nRdCount < g_AS_HVInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO_HV_t*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			while (nRdCount + 1 < g_AS_HVInfo.ASTHead.nCardCnt)
			{
				memcpy(p, p + sizeof(CDINFO_HV_t), sizeof(CDINFO_HV_t));
				p += sizeof(CDINFO_HV_t);
				nRdCount++;
			}
			g_AS_HVInfo.ASTHead.nCardCnt--;			
			return TRUE;
		}
		else
		{
			p += sizeof(CDINFO_HV_t);
			nRdCount++;
			pCdInfo = NULL;
		}
	}
	return FALSE;	
}

/************************************************************************************************
**FunctionName    : SaveCdHVInfo2Mem
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
void SaveCdHVInfo2Mem(void)
{
	FILE * fd = NULL;
    DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
	if(NULL == g_AS_HVInfo.MXASBuf) return;
	if ((fd = fopen(AST_HVFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fwrite(&g_AS_HVInfo.ASTHead, sizeof(ASTHEAD), (size_t)1, fd);
		fseek(fd, 0, SEEK_CUR);
		fwrite(g_AS_HVInfo.MXASBuf, g_AS_HVInfo.ASTHead.nCardCnt * sizeof(CDINFO_HV_t), (size_t)1, fd);
		fclose(fd);
	}
	else
	{
		DEBUG_CARD_PRINTF("%s,%d,AST_HV file open error\n",__func__,__LINE__);
	}
}

/************************************************************************************************
**FunctionName    : LoadCdHVInfoFromFile
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
void LoadCdHVInfoFromFile(void)
{
	FILE* Fd = NULL;
	if ((Fd = fopen(AST_HVFILE,"r+")) != NULL)
	{
		fseek(Fd,0,SEEK_SET);
		fread(&g_AS_HVInfo.ASTHead,sizeof(ASTHEAD),(size_t)1,Fd);
		fseek(Fd,0,SEEK_CUR);
		fread(g_AS_HVInfo.MXASBuf,g_AS_HVInfo.ASTHead.nCardCnt * sizeof(CDINFO_HV_t),(size_t)1,Fd);
		fclose(Fd);
	}
	else
	{
		printf("AST_HVFILE open failed!\n");
	}
}

/************************************************************************************************
**FunctionName    : AsAddAuthorizeCdHV
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BOOL AsAddAuthorizeCdHV(CDINFO_HV_t* pCdInfo)
{
	if(IsAuthorizeCdHVExist(pCdInfo->CSN))
	{
		printf("Card is exist\n");       
        AsRmCdHV(pCdInfo->CSN);
        AsRmPatrolCd(pCdInfo->CSN);
		return FALSE;
	}
	
	if (g_AAInfo.ASTHead.nCardCnt + 1 < MAX_AS_ACD_CNT)
	{
        printf("AsAddAuthorizeCdHV\n");
		memcpy(g_AAInfo.MXASBuf + g_AAInfo.ASTHead.nCardCnt * sizeof(CDINFO_HV_t),
			pCdInfo, sizeof (CDINFO_HV_t));
		g_AAInfo.ASTHead.nCardCnt++;
        
        AsRmCdHV(pCdInfo->CSN);
        AsRmPatrolCd(pCdInfo->CSN);
		return TRUE;
	}
	else
	{
		printf("Card Info buffer full\n");
		return FALSE;
	}

}

/************************************************************************************************
**FunctionName    : AsAddAuthorizeCd
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BOOL AsAddAuthorizeCd(CDINFO_OLD* pCdInfo)
{
    if(IsAuthorizeCdExist(pCdInfo->CSN))
	{
		printf("Card is exist\n");        
        AsRmCd(pCdInfo->CSN);
        AsRmPatrolCd(pCdInfo->CSN);
		return FALSE;
	}
	
	if (g_AAInfo.ASTHead.nCardCnt + 1 < MAX_AS_ACD_CNT)
	{
		memcpy(g_AAInfo.MXASBuf + g_AAInfo.ASTHead.nCardCnt * sizeof(CDINFO_OLD),
			pCdInfo, sizeof (CDINFO_OLD));
		g_AAInfo.ASTHead.nCardCnt++;
        printf("add auth card success , cnt: %u\n",g_AAInfo.ASTHead.nCardCnt);
        AsRmCd(pCdInfo->CSN);
        AsRmPatrolCd(pCdInfo->CSN);
		return TRUE;
	}
	else
	{
		printf("Card Info buffer full\n");
		return FALSE;
	}
}

/************************************************************************************************
**FunctionName    : AsAddCdHV
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BOOL AsAddCdHV(CDINFO_HV_t* pCdInfo)
{	
    if(g_AS_HVInfo.MXASBuf == NULL) return FALSE;
	if (g_AS_HVInfo.ASTHead.nCardCnt + 1 < MAX_AS_CD_CNT)
	{
		memcpy(g_AS_HVInfo.MXASBuf + g_AS_HVInfo.ASTHead.nCardCnt * sizeof(CDINFO_HV_t),
			pCdInfo, sizeof (CDINFO_HV_t));
		g_AS_HVInfo.ASTHead.nCardCnt++;
		return TRUE;
	}
	else
	{
		printf("Card Info buffer full\n");
		return FALSE;
	}

}

/************************************************************************************************
**FunctionName    : AsGetCdHVInfobyCd
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
CDINFO_HV_t* AsGetCdHVInfobyCd(BYTE* pCSN)
{
    CDINFO_HV_t*	pCdInfo	= NULL;
	CHAR*	        p = NULL;
	INT             nRdCount = 0;
    
	p = g_AS_HVInfo.MXASBuf;
    if(p == NULL) return pCdInfo;
	while (nRdCount < g_AS_HVInfo.ASTHead.nCardCnt) 
	{
		if(NULL != (pCdInfo = (CDINFO_HV_t*)p))
		{
			if (CSNCompare(pCSN, pCdInfo->CSN))
			{
				return pCdInfo;
			}
		}
		p += sizeof(CDINFO_HV_t);
		nRdCount++;
		pCdInfo = NULL;
	}
    
    pCdInfo = NULL;
    nRdCount = 0;
	p = g_AAInfo.MXASBuf;
    if(p == NULL) return pCdInfo;
	while (nRdCount < g_AAInfo.ASTHead.nCardCnt) 
	{
		if(NULL != (pCdInfo = (CDINFO_HV_t*)p))
		{
			if (CSNCompare(pCSN, pCdInfo->CSN))
			{					
				return pCdInfo;
			}
		}
		p += sizeof(CDINFO_HV_t);
		nRdCount++;
		pCdInfo = NULL;
	}
    return pCdInfo;
}

/************************************************************************************************
**FunctionName    : IsAuthorizeCdHVExist
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BOOL IsAuthorizeCdHVExist(BYTE* pCSN)
{
	CDINFO_HV_t*	pCdInfo	= NULL;
	CHAR*	        p		= NULL;
	INT				nRdCount = 0;
	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
	p = g_AAInfo.MXASBuf;

	while (nRdCount < g_AAInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO_HV_t*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			return TRUE;
		}
		p += sizeof(CDINFO_HV_t);
		nRdCount++;
		pCdInfo = NULL;
	}

	return FALSE;
}

/************************************************************************************************
**FunctionName    : IsAuthorizeCdExist
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BOOL IsAuthorizeCdExist(BYTE* pCSN)
{
    CDINFO_HV_t*	pCdInfo	= NULL;
	CHAR*	        p		= NULL;
	INT				nRdCount = 0;
	
	p = g_AAInfo.MXASBuf;
	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);

	while (nRdCount < g_AAInfo.ASTHead.nCardCnt) 
	{
		pCdInfo = (CDINFO_OLD*)p;
		if (CSNCompare(pCSN, pCdInfo->CSN))
		{
			return TRUE;
		}
		p += sizeof(CDINFO_OLD);
		nRdCount++;
		pCdInfo = NULL;
	}

	return FALSE;
}

/************************************************************************************************
**FunctionName    : isAST_HVFileExisted
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BOOL isAST_HVFileExisted(void)
{
    BOOL bRet = FALSE;
    if((access(AST_HVFILE,0)) == 0)
    {
        bRet = TRUE;
    }
	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
    return bRet;
}

/************************************************************************************************
**FunctionName    : convertNormalCardFileData
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
void convertNormalCardFileData(void)
{
    FILE* Fd = NULL;
    WORD wIter = 0;
    WORD wTotal = 0;
    CHAR* pOldFormateData = g_ASInfo.MXASBuf;//g_ASInfo.MXASBuf has already been allocated 
    CHAR* pNewFormateData = g_AS_HVInfo.MXASBuf;
    CHAR* pNewFormateDataB = pNewFormateData;
    //create a file which we want
    if((Fd = fopen(AST_HVFILE,"w+")) != NULL)
    {
        fseek(Fd,0,SEEK_SET);
        //both heads are same,so we can use memcpy() call
        memcpy(&g_AS_HVInfo.ASTHead,&g_ASInfo.ASTHead,sizeof(ASTHEAD));
        fwrite(&g_AS_HVInfo.ASTHead,sizeof(ASTHEAD),(size_t)1,Fd);               
        wTotal = g_AS_HVInfo.ASTHead.nCardCnt;
        printf("normal card cnt : %u\n",wTotal);
        for(wIter = 0 ; wIter < wTotal ; wIter++)
        {
            memcpy(pNewFormateData,pOldFormateData,sizeof(CDINFO));
            pNewFormateData += sizeof(CDINFO_HV_t);
            pOldFormateData += sizeof(CDINFO);
        }
        fseek(Fd,0,SEEK_CUR);
        //save data
        fwrite(pNewFormateDataB,g_AS_HVInfo.ASTHead.nCardCnt * sizeof(CDINFO_HV_t),(size_t)1,Fd);
        fclose(Fd);
    }
    
    pOldFormateData = pNewFormateData = pNewFormateDataB;
    Fd = NULL;
    //we don't need g_ASInfo.MXASBuf any more
    free(g_ASInfo.MXASBuf);
    g_ASInfo.MXASBuf = NULL;
}

/************************************************************************************************
**FunctionName    : convertAuthCardFileData
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
void convertAuthCardFileData(void)
{
    BOOL bRet = FALSE;
    FILE* Fd = NULL;
    WORD wIter = 0;
    WORD wTotal = 0;
    CHAR* pOldFormateData = g_AAInfo.MXASBuf;//g_AAInfo.MXASBuf has already been allocated  
    CHAR* pNewFormateData = (CHAR*)malloc(MAX_AS_ACD_CNT * sizeof(CDINFO_HV_t));    
    CHAR* pNewFormateDataB = pNewFormateData;    
    if(NULL == pNewFormateData) {return;}
    memset(pNewFormateData,0,g_AAInfo.ASTHead.nCardCnt * sizeof(CDINFO_HV_t));
    printf("converting normal card.\n");
    if((Fd = fopen(AATFILE,"w+")) != NULL)
    {
        fseek(Fd,0,SEEK_SET);
        //both heads are same,so we can use memcpy() call
        fwrite(&g_AAInfo.ASTHead,sizeof(ASTHEAD),(size_t)1,Fd);
        wTotal = g_AAInfo.ASTHead.nCardCnt;
        printf("auth card cnt : %u\n",wTotal);
        for(wIter = 0 ; wIter < wTotal ; wIter++)
        {
            memcpy(pNewFormateData,pOldFormateData,sizeof(CDINFO));
            pNewFormateData += sizeof(CDINFO_HV_t);
            pOldFormateData += sizeof(CDINFO);
        }
        fseek(Fd,0,SEEK_CUR);
        //save data
        fwrite(pNewFormateDataB,g_AAInfo.ASTHead.nCardCnt * sizeof(CDINFO_HV_t),(size_t)1,Fd);
        fclose(Fd);
    }
    free(g_AAInfo.MXASBuf);
    g_AAInfo.MXASBuf = pNewFormateDataB;
    pOldFormateData = pNewFormateData = pNewFormateDataB;
    Fd = NULL;
}




/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SaveOrSynchronousCardHashInfo2Mem
**	AUTHOR:		   Mike Zhang
**	DATE:		4 - Jan - 2011
**
**	DESCRIPTION:
**		同步or 保存card hash <--> flash  	
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/

VOID 
SaveOrSynchronousCardHashInfo2Mem(void)
{
	SaveCardHashInfo2Mem();
	LoadCardHashInfoFromMem();
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SaveCardHashInfo2Mem
**	AUTHOR:		   Mike Zhang
**	DATE:		4 - Jan - 2011
**
**	DESCRIPTION:
**		保存数据card hash <--> flash  	
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/
//extern UCHAR   SyncDataBuf[SYNCDATABUFSIZE];
VOID 
SaveCardHashInfo2Mem(void)
{
/*	FILE * fd = NULL;
	
	if ((fd = fopen(AHASHFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fwrite(&SyncDataBuf[0], SYNCDATABUFSIZE , (size_t)1, fd);		
		fclose(fd);
		printf("AHASHFILE file open sucess\n");
	}
	else
	{
		printf("AHASHFILE file open error\n");
	}
*/	
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadCardHashInfoFromMem
**	AUTHOR:		   Mike Zhang
**	DATE:		4 - Jan - 2011
**
**	DESCRIPTION:
**		读取数据card hash <--> flash  	
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/

VOID 
LoadCardHashInfoFromMem(void)
{
/*	CDINFO		CdInfo;
	CDINFO	*pCdInfo;

	FILE * fd			= NULL;
	FILE * fd2			= NULL;
	struct stat			pStat;

	printf("AHASHFILE file open \n");		
	if ((fd = fopen(AHASHFILE, "r+")) != NULL)
	{
		stat(AHASHFILE, &pStat);
		fseek(fd, 0, SEEK_SET);
		fread(&SyncDataBuf[0], SYNCDATABUFSIZE, (size_t)1, fd);

		fclose(fd);
	}
	else
	{

		printf("AHASHFILE file open error,create the file\n");
		if ((fd = fopen(AHASHFILE, "w+")) != NULL)
		{
			ClearAllCardData();
			printf("write create the file success\n");
			fclose(fd);
		}
		else
		{

			printf("ASTFILE file open error,Write error\n");
			return;
		}
	}
*/
	printf("AHASHFILE file open \n");	
}

/************************************************************************************************
**FunctionName    : isAHASHFILEExisted
**Function        : 
**InputParameters : 
**OuputParameters : 
**ReturnedValue   : 
************************************************************************************************/
BOOL isAHASHFILEExisted(void)
{
    BOOL bRet = FALSE;
    if((access(AHASHFILE,0)) == 0)
    {
        bRet = TRUE;
    }
    return bRet;
}











/**
 * Conversion Card Information    卡片数据格式转换，CDONFO转化为HASHCDINFO
 *  
 * @param CdInfo_old 		转化前的数据格式
 * @param CDINFO			转化后的数据格式
 *
 * @return 0 转换成功； -1 转换失败 
 */

/**/
BOOL ConversionCdInfoOld2CdInfo(CDINFO *CdInfo,CDINFO_OLD*CdInfo_old)
{	
	int i = 0;
	
	printf("%s,%d\r\n",__func__,__LINE__);

	
	if(CdInfo_old == NULL)
		{
		return FALSE;
	}
		CdInfo->CSN[0] = 				CdInfo_old->CSN[0] ;
		CdInfo->CSN[1] = 				CdInfo_old->CSN[1] ;
		CdInfo->CSN[2] = 				CdInfo_old->CSN[2] ;
		CdInfo->CSN[3] = 				CdInfo_old->CSN[3] ;
		CdInfo->CSN[4] = 				CdInfo_old->CSN[4] ;
		CdInfo->CardType = 			CdInfo_old->CardType ;
		CdInfo->CardMode=			CdInfo_old->CardMode ;
		CdInfo->CardStatus=			CdInfo_old->CardStatus	;
		CdInfo->GateNumber =		CdInfo_old->GateNumber	;
		for( i = 0;i<RD_CODE_LEN; i++)
		{
			CdInfo->RdNum[i]=				CdInfo_old->RdNum[i];
		}
		for( i = 0;i<PWD_LEN; i++)
		{
			CdInfo->UlkPwd[i] = 			CdInfo_old->UlkPwd[i];
		}
		CdInfo->UnlockTimeSliceID = CdInfo_old->UlkTimeSlice;
		CdInfo->ValidEndTime[0] = 		CdInfo_old->VldEndTime>>0;
		CdInfo->ValidEndTime[1] = 		CdInfo_old->VldEndTime>>8;
		CdInfo->ValidEndTime[2] = 		CdInfo_old->VldEndTime>>16;
		CdInfo->ValidEndTime[3] = 		CdInfo_old->VldEndTime>>24;

		CdInfo->ValidStartTime[0] = 	CdInfo_old->VldStartTime>>0;
		CdInfo->ValidStartTime[1] = 	CdInfo_old->VldStartTime>>8;
		CdInfo->ValidStartTime[2] = 	CdInfo_old->VldStartTime>>16;
		CdInfo->ValidStartTime[3] = 	CdInfo_old->VldStartTime>>24;

	return TRUE;
	
}

/*
BOOL ConversionLfCard2HashLfCard  (HASH_LFCARD* HashLfCard ,LFCard* LfCard)
{	
	int i = 0;
	printf("%s,%d\r\n",__func__,__LINE__);
	if(LfCard == NULL)
		{
		return FALSE;
	}	
		HashLfCard->CSN[0] = 				LfCard->CSN[0] ;
		HashLfCard->CSN[1] = 				LfCard->CSN[1] ;
		HashLfCard->CSN[2] = 				LfCard->CSN[2] ;
		HashLfCard->CSN[3] = 				LfCard->CSN[3] ;
		HashLfCard->CSN[4] = 				LfCard->CSN[4] ;
		for( i = 0;i<=RD_CODE_LEN; i++)
		{
			HashLfCard->Code[i]=				LfCard->Code[i];
		}
		HashLfCard->bank	=				LfCard->bank;
		HashLfCard->node	=				LfCard->node;
		HashLfCard->byBackDoorLen	=		LfCard->byBackDoorLen;
		HashLfCard->byFrontDoorLen	=		LfCard->byFrontDoorLen;
		HashLfCard->bAdmin			=		LfCard->bAdmin;
		for( i = 0;i<=7; i++)
		{
			HashLfCard->dlBackUnlockLevel[i]	=	LfCard->dlBackUnlockLevel>>(8*i);
			HashLfCard->dlFrontUnlockLevel[i]	= 	LfCard->dlFrontUnlockLevel>>(8*i);
		}

	return TRUE;
	
}

/*
BOOL UPdata_ALT( void )
{
	HASH_LFCARD	pCdInfo;//新的卡片结构
	LFCard* pCdInfoOld = NULL;//旧的卡片结构
	
	CHAR*	    p				= NULL;
	INT				nRdCount = 0;

	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
	p = g_APInfo.MXASBuf;

	while (nRdCount < g_APInfo.ASTHead.nCardCnt) 
	{
		pCdInfoOld = (CDINFO_OLD*)p;
		
		if (ConversionCdInfoOld2CdInfo(&pCdInfo,pCdInfoOld))
		{
			WriteCardInfo(g_HandleHashCard,&pCdInfo,pCdInfo.CardType);
			AsPrintCd(&pCdInfo);
		}
		p += sizeof(CDINFO_OLD);
		nRdCount++;
		pCdInfoOld = NULL;
	}

	return TRUE;
}
*/



BOOL UPdata_APT( void )
{
	CDINFO	pCdInfo;//新的卡片结构
	CDINFO_OLD* pCdInfoOld = NULL;//旧的卡片结构
	
	CHAR*	    p				= NULL;
	INT				nRdCount = 0;

	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
	p = g_APInfo.MXASBuf;

	while (nRdCount < g_APInfo.ASTHead.nCardCnt) 
	{
		pCdInfoOld = (CDINFO_OLD*)p;
		
		if (ConversionCdInfoOld2CdInfo(&pCdInfo,pCdInfoOld))
		{
			WriteCardInfo(g_HandleHashCard,&pCdInfo,pCdInfo.CardType);
			AsPrintCd(&pCdInfo);
		}
		p += sizeof(CDINFO_OLD);
		nRdCount++;
		pCdInfoOld = NULL;
	}
	g_APInfo.MXASBuf = NULL;
	g_APInfo.ASTHead.nCardCnt = 0;	
	system("rm -fr /mox/rdwr/APT");
	printf("[%s,%d]  rm APT ~~~~\r\n",__func__,__LINE__);

	return TRUE;
}


BOOL UPdata_AST( void )
{
	CDINFO	pCdInfo;//新的卡片结构
	CDINFO_OLD* pCdInfoOld = NULL;//旧的卡片结构
	
	CHAR*	    p				= NULL;
	INT				nRdCount = 0;

	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
	p = g_ASInfo.MXASBuf;
	
	while (nRdCount < g_ASInfo.ASTHead.nCardCnt) 
	{
		pCdInfoOld = (CDINFO_OLD*)p;
		if (ConversionCdInfoOld2CdInfo(&pCdInfo,pCdInfoOld))
		{
			WriteCardInfo(g_HandleHashCard,&pCdInfo,pCdInfo.CardType);
			AsPrintCd(&pCdInfo);
		}
		p += sizeof(CDINFO_OLD);
		nRdCount++;
		pCdInfoOld = NULL;
	}

	//g_ASInfo.MXASBuf = NULL;
	g_ASInfo.ASTHead.nCardCnt = 0;

	system("rm -fr /mox/rdwr/AST");
	printf("[%s,%d]  rm AST ~~~~\r\n",__func__,__LINE__);


	return TRUE;
}



BOOL UPdata_AAT( void )
{
	CDINFO	pCdInfo;//新的卡片结构
	CDINFO_OLD* pCdInfoOld = NULL;//旧的卡片结构
	
	CHAR*	    p				= NULL;
	INT				nRdCount = 0;

	DEBUG_CARD_PRINTF("%s,%d\r\n",__func__,__LINE__);
	p = g_AAInfo.MXASBuf;

	while (nRdCount < g_AAInfo.ASTHead.nCardCnt) 
	{
		pCdInfoOld = (CDINFO_OLD*)p;
		if (ConversionCdInfoOld2CdInfo(&pCdInfo,pCdInfoOld))
		{
			WriteCardInfo(g_HandleHashCard,&pCdInfo,pCdInfo.CardType);
			AsPrintCd(&pCdInfo);
		}
		p += sizeof(CDINFO_OLD);
		nRdCount++;
		pCdInfoOld = NULL;
	}
	system("rm -fr /mox/rdwr/AAT");

	return TRUE;
}




//conversion
/**
 * Conversion Card Information    卡片数据格式转换，CDONFO转化为HASHCDINFO
 *  
 * @param CdInfo 		转化前的数据格式
 * @param HashCdInfo	转化后的数据格式
 *
 * @return 0 转换成功； -1 转换失败 
 */

/*int ConversionCdInfo2HashCdInfo(CDINFO *CdInfo,HASH_CDINFO *HashCdInfo)
{
	printf("%s,%d\r\n",__func__,__LINE__);
	if(CdInfo == NULL)
		{
		return -1;
	}
	HashCdInfo->CSN 				= CdInfo->CSN;
	HashCdInfo->CardMode 		= CdInfo->CardMode;
	HashCdInfo->CardStatus		= CdInfo->CardStatus;
	HashCdInfo->GateNumber		= CdInfo->GateNumber;
	HashCdInfo->ResidentCode		= CdInfo->RdNum;
	HashCdInfo->UnlockPassword	= CdInfo->UlkPwd;
	HashCdInfo->UnlockTimeSliceID= CdInfo->UlkTimeSlice;
	HashCdInfo->ValidEndTime		= CdInfo->VldEndTime;
	HashCdInfo->ValidStartTime	= CdInfo->VldStartTime;

	return 0;
	
}
*/

/**
 * Conversion Card Information    卡片数据格式转换，CDONFO转化为HVCdInfo
 *  
 * @param CdInfo 		转化前的数据格式
 * @param HVCdInfo		转化后的数据格式
 *
 * @return 0 转换成功； -1 转换失败 
 */

/*int ConversionCdInfo2HVCdInfo(CDINFO * CdInfo,CDINFO_HV_t* HVCdInfo)
{
	printf("%s,%d\r\n",__func__,__LINE__);
	if(CdInfo == NULL)
		{
		return -1;
	}
 	CdInfo->CSN			= CdInfo->CSN;
	HVCdInfo->GroupNum		= CdInfo->GateNumber;
	HVCdInfo->CardType		= CdInfo->CardType;
	HVCdInfo->LocalNum		= CdInfo->LocalNum;
	HVCdInfo->CardMode 		= CdInfo->CardMode;
	HVCdInfo->CardStatus		= CdInfo->CardStatus;
	HVCdInfo->GateNumber		= CdInfo->GateNumber;
	HVCdInfo->RdNum			= CdInfo->RdNum;
	HVCdInfo->UlkPwd			= CdInfo->UlkPwd;
	HVCdInfo->UlkTimeSlice	= CdInfo->UlkTimeSlice;
	HVCdInfo->VldEndTime		= CdInfo->VldEndTime;
	HVCdInfo->VldStartTime	= CdInfo->VldStartTime;

	return 0;
	
}
*/

/**
 * Conversion Card Information    卡片数据格式转换，HVCdInfo转化为HashCdInfo
 *  
 * @param HVCdInfo 		转化前的数据格式
 * @param HashCdInfo	转化后的数据格式
 *
 * @return 0 转换成功； -1 转换失败 
 */

/*int ConversionHVCdInfo2HashCdInfo(CDINFO_HV_t * HVCdInfo,HASH_CDINFO * HashCdInfo)
{
	printf("%s,%d\r\n",__func__,__LINE__);
	if(CdInfo == NULL)
		{
		return -1;
	}
	shCdInfo->CSN 				= HVCdInfo->CSN;
	HashCdInfo->CardMode 		= HVCdInfo->CardMode;
	HashCdInfo->CardStatus		= HVCdInfo->CardStatus;
	HashCdInfo->GateNumber		= HVCdInfo->GateNumber;
	HashCdInfo->ResidentCode		= HVCdInfo->RdNum;
	HashCdInfo->UnlockPassword	= HVCdInfo->UlkPwd;
	HashCdInfo->UnlockTimeSliceID= HVCdInfo->UlkTimeSlice;
	HashCdInfo->ValidEndTime		= HVCdInfo->VldEndTime;
	HashCdInfo->ValidStartTime	= HVCdInfo->VldStartTime;

	return 0;

}
*/




