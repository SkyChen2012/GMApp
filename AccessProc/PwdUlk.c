
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	PwdUlk.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		07 - Apr - 2009
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
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <errno.h>
#include <sys/timeb.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>  
#include <termios.h>

/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXTypes.h"
#include "MXCommon.h"

#include "MenuParaProc.h"
#include "PwdUlkWnd.h"
#include "PwdUlk.h"
#include "ParaSetting.h"
#include "PwdModifyWnd.h"
#include "AMT.h"
/************** DEFINES **************************************************************/

#define DEBUG_AS_INIT
//#define DEBUG_AS_PROC

#define PUT_VERSION			0x10000000
#define MAX_PU_RD_CNT		10000

/************** TYPEDEFS *************************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static BOOL RDCompare(char *pInCode, char *pTableCode, int InCodeLen);

BOOL CodeCompare(CHAR *pInResidentCode, CHAR *pTableResidentCode, INT InCodeLenth);

void LoadRdInfoFromMem();
void SaveRdInfo2Mem();

PUINFO	g_PUInfo = 
{	
	{
		PUT_VERSION,
		0,
		0
	},
	NULL
};

//char  *PUTBuffer	=	NULL;

#ifdef DEBUG_AS_PROC
static void	PUPrintAllRd();
#endif

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PwdUlkProc
**	AUTHOR:		   Jeff Wang
**	DATE:		08 - April - 2009
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
PwdUlkProc()
{
	;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PwdUlkInit
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
PwdUlkInit()
{
	g_PUInfo.MXPUBuf = (CHAR*)malloc(MAX_PU_RD_CNT * sizeof(RDINFO));

	if (NULL == g_PUInfo.MXPUBuf) 
	{
		printf("RD Buffer Init Error\n");
		return;
	}
	memset(g_PUInfo.MXPUBuf, 0, MAX_PU_RD_CNT * sizeof(RDINFO));

	LoadRdInfoFromMem();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadRdInfoFromMem
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
LoadRdInfoFromMem()
{
	FILE * fd			= NULL;
	struct stat			pStat;;

	if ((fd = fopen(PUTFILE, "r+")) != NULL)
	{
		stat(PUTFILE, &pStat);

		fseek(fd, 0, SEEK_SET);
		fread(&g_PUInfo.PUTHead, sizeof(PUTHEAD), (size_t)1, fd);

		fseek(fd, 0, SEEK_CUR);
		fread(g_PUInfo.MXPUBuf, pStat.st_size-sizeof(PUTHEAD), (size_t)1, fd);

		fclose(fd);
	}
	else
	{
		printf("PUT file open error\n");
		return;
	}	
	
// 	if( g_PUInfo.PUTHead.VersionNum > PUT_VERSION )		
// 	{	
// 		memset(g_PUInfo.MXPUBuf, 0, pStat.st_size);
// 		printf("Version error\n");		
// 		return;
// 	}

	if (g_PUInfo.PUTHead.nResiCnt > MAX_PU_RD_CNT)
	{
		memset(g_PUInfo.MXPUBuf, 0, pStat.st_size);
		printf("Resident count error\n");		
		return;		
	}

#ifdef DEBUG_AS_PROC
	printf("Load:ResiCnt = %d\n", g_PUInfo.PUTHead.nResiCnt);
	PUPrintAllRd();
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SaveRdInfo2Mem
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
SaveRdInfo2Mem()
{
	FILE * fd			= NULL;

	if ((fd = fopen(PUTFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fwrite(&g_PUInfo.PUTHead, sizeof(PUTHEAD), (size_t)1, fd);

		fseek(fd, 0, SEEK_CUR);
		fwrite(g_PUInfo.MXPUBuf, g_PUInfo.PUTHead.nResiCnt * sizeof(RDINFO), (size_t)1, fd);

#ifdef DEBUG_AS_PROC
		printf("Save:ResiCnt = %d\n", g_PUInfo.PUTHead.nResiCnt);
#endif
		
		fclose(fd);
	}
	else
	{
		printf("PUT file Save error\n");
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsRdExist
**	AUTHOR:		   Jeff Wang
**	DATE:		10 - Sep - 2008
**
**	DESCRIPTION:	
**		Find whether the resient is exist in PUT
**
**	ARGUMENTS:		
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			Any thing need to be noticed.
*/
BOOL 
IsRdExist(CHAR *pRdCode, INT nRdLen)
{
	RDINFO  *pRdInfo	= NULL;
	CHAR      *p			    = NULL;
	INT		       nRdCount	              =	0;
	
	p = g_PUInfo.MXPUBuf;

	while(nRdCount < g_PUInfo.PUTHead.nResiCnt)
	{
		pRdInfo = (RDINFO*)p;		
		if (RDCompare(pRdCode, pRdInfo->RdCode, nRdLen))
		{
			return TRUE;
		}
			
		p += sizeof(RDINFO);
		nRdCount++;
		pRdInfo = NULL;
	}	
	return FALSE;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PUExit
**	AUTHOR:			Jeff Wang
**	DATE:			08 - April - 2009
**
**	DESCRIPTION:	
**			PU resident information exit
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
VOID
PUExit()
{
	PURmAllRd();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PUAddRd
**	AUTHOR:			Jeff Wang
**	DATE:		   10 - Oct - 2008
**
**	DESCRIPTION:	
**			Add one resident code to table
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				RdInfoMan		[IN]		RDINFOMAN
**	RETURNED VALUE:	
**				TRUE if succeed, otherwise FALSE
**	NOTES:
**			
*/
BOOL
PUAddRd(CHAR* pRdCode, INT nRdcodeLen)
{
	RDINFO	RdInfo;
	
	memset(&RdInfo, 0, sizeof(RDINFO));
	memcpy(RdInfo.RdCode, pRdCode, nRdcodeLen);
	memcpy(RdInfo.RdPwd, g_PUPara.ResiPwdDefault, strlen(g_PUPara.ResiPwdDefault));
	RdInfo.bPwdCfg	= 0;

	if (g_PUInfo.PUTHead.nResiCnt +1 <= MAX_PU_RD_CNT)
	{
		memcpy(g_PUInfo.MXPUBuf + g_PUInfo.PUTHead.nResiCnt * sizeof(RDINFO),
						&RdInfo, sizeof(RDINFO));

	}
	g_PUInfo.PUTHead.nResiCnt++;	
	
	return TRUE;	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PUGetRdInfobyRd
**	AUTHOR:			Jeff Wang
**	DATE:			10 - Oct - 2008
**
**	DESCRIPTION:	
**			Find resident information by resident code
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pRdcode		[IN]		char*
**				RdcodeLen		[IN]		int
**	RETURNED VALUE:	
**				RDINFOMAN* pointer or NULL
**	NOTES:
**			
*/
RDINFO*
PUGetRdInfobyRd(CHAR *pRdCode, INT RdCodeLen)
{
	RDINFO*	pRdInfo	= NULL;
	INT			nRdCount = 0;
	CHAR	*p = NULL;

	p = g_PUInfo.MXPUBuf;
	while (nRdCount < g_PUInfo.PUTHead.nResiCnt)
	{
		pRdInfo = (RDINFO*) p;
		if (RDCompare(pRdCode, pRdInfo->RdCode, RdCodeLen))
		{
			return pRdInfo;
		}
		p += sizeof(RDINFO);
		nRdCount++;
		pRdInfo = NULL;
	}

	return NULL;
}

BOOL 
PUPasswordCompare(CHAR *pInPwd,  INT InCodeLen)
{
	RDINFO*	pRdInfo	= NULL;
	INT			nRdCount = 0;
	CHAR	*p = NULL;

	if (NULL == pInPwd)
	{
		printf("Lenth Error\n");
		return FALSE;
	}

	p = g_PUInfo.MXPUBuf;
	while (nRdCount < g_PUInfo.PUTHead.nResiCnt)
	{
		pRdInfo = (RDINFO*) p;
		if (IsRdExist(pRdInfo->RdCode, strlen(pRdInfo->RdCode)) && CodeCompare(pInPwd, pRdInfo->RdPwd, InCodeLen))
		{
			return TRUE;
		}
		p += sizeof(RDINFO);
		nRdCount++;

	}

	return FALSE;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PURmRd
**	AUTHOR:			Jeff Wang
**	DATE:			10 - Oct - 2008
**
**	DESCRIPTION:	
**			Remove one Resident from table
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMdInfo		[IN]		MdInfo*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
VOID
PURmRd(CHAR *pRdCode, INT RdCodeLen)
{
	RDINFO*	pRdInfo	= NULL;
	INT			nRdCount = 0;
	CHAR	*p = NULL;
	INT		j = 0;
	CHAR	*p1 = NULL;

	
	p = g_PUInfo.MXPUBuf;
	
	while (nRdCount < g_PUInfo.PUTHead.nResiCnt)
	{
		pRdInfo = (RDINFO*) p;
		if (CodeCompare(pRdCode, pRdInfo->RdCode, RdCodeLen))
		{
			j	=	nRdCount;
			p1	=	p;
			while (j + 1 < g_PUInfo.PUTHead.nResiCnt) 
			{
				memcpy(p1, p1 + sizeof(RDINFO), sizeof(RDINFO));
				p1 += sizeof(RDINFO);
				j++;
			}
			g_PUInfo.PUTHead.nResiCnt--;

			break;
		}
		p += sizeof(RDINFO);
		nRdCount++;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PURmAllRd
**	AUTHOR:			Jeff Wang
**	DATE:			10 - Oct - 2008
**
**	DESCRIPTION:	
**			Remove one Resident from table
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMdInfo		[IN]		MdInfo*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
PURmAllRd()
{
	memset(g_PUInfo.MXPUBuf, 0, g_PUInfo.PUTHead.nResiCnt * sizeof(RDINFO));
	g_PUInfo.PUTHead.nResiCnt = 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DpcPrintAllMd
**	AUTHOR:			Jerry Huang
**	DATE:			25 - Sep - 2006
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
#ifdef DEBUG_AS_PROC
static void
PUPrintAllRd()
{
	RDINFO *pRdInfo	= NULL;
	INT			  nRdCount = 0;
	CHAR	  *p = NULL;

	printf("As: Rd Info Debug ....\n");
	p = g_PUInfo.MXPUBuf;

	while (nRdCount < g_PUInfo.PUTHead.nResiCnt)
	{
		pRdInfo = (RDINFO*) p;
		printf("RdCode %s, RdPwd %s, PwdCfg %d\n",
			(UINT) pRdInfo->RdCode,
			(UINT) pRdInfo->RdPwd,
			(UINT) pRdInfo->bPwdCfg);

		p += sizeof(RDINFO);
		nRdCount++;
	}
	printf("As: Rd Debug ....End\n");
}
#endif

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	FdEHVfromAMT
**	AUTHOR:			Jeff Wang
**	DATE:		25 - Mar - 2008
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
INT 
FdEHVfromAMT(char ** ppPUTBuffer)
{	
	BYTE   ATMHeadItem	=	0;
	BYTE	nType   	= 	0;
	WORD	SetTypeCount= 	0;
	WORD  nCountEHV 	= 	0;
	WORD	nCountMC  	= 	0;
	UCHAR	nCountIP	=	0; 	
	CHAR	*pMCBuf 	= 	NULL;
	CHAR 	*pEHVBuf	= 	NULL;
	CHAR	*pMCBufDef  = 	NULL;
	DWORD  dwATMVersion= 	0;
	DWORD 	ATMLen		=	0;
	DWORD	dwItemOffset=	0;
	DWORD	dwNexAddr 	= 	0;
	int 	i,j           = 0;
	BOOL	bMCConfigured = TRUE;
	
	AMT_Type900 atEHV_Content;

	INT	RetCnt	=	0;

	memset(&atEHV_Content, 0, sizeof(AMT_Type900));	

	if (NULL == AMTBuf) 
	{
		printf("AMT empty\n");
		return;
	}
	
	memcpy(&dwATMVersion, AMTBuf, 4);
	memcpy(&ATMLen, AMTBuf+4, 4);
	
	if(dwATMVersion < ATM_VERSION_4 || ATMLen > 0x200000)
	{
		printf("ATM version or Len old\n");		
		return;
	}
	
	memcpy(&ATMHeadItem, AMTBuf+8, 1);	
	
	if(ATMHeadItem <= 0)
	{
		printf("ATM item empety\n");
		return;
	}
	
	for(i=0;i<ATMHeadItem;i++)
	{
		memcpy(&nType, AMTBuf+9+i*ATM_HEAD_LEN_V4, 1);
		memcpy(&SetTypeCount, AMTBuf+9+i*ATM_HEAD_LEN_V4+1, 2);
		memcpy(&dwItemOffset, AMTBuf+9+i*ATM_HEAD_LEN_V4+3, 4);		
		
		if(nType == ATM_TYPE_EHV || nType == ATM_TYPE_MC)
		{
			memcpy((char *)(&atEHV_Content), AMTBuf+dwItemOffset +8, 28);
			nCountIP = atEHV_Content.IPCOUNT;
			memcpy((char *)(&atEHV_Content) + 28, AMTBuf+dwItemOffset +8 + 28, nCountIP * 4);
			memcpy((char *)(&atEHV_Content) + 108, AMTBuf+dwItemOffset +8 + 28 + nCountIP * 4, 24);
			
			for(j=0;j<SetTypeCount;j++)
			{
				if(0 == strcmp(atEHV_Content.DevCode, "00"))
				{
					bMCConfigured = FALSE;
				}
				
				if ((j + 1) < SetTypeCount)
				{
					memcpy(&dwNexAddr,atEHV_Content.NexAddress,4);
				//	printf("dwNexAddr=%d\n",dwNexAddr);
					memcpy((char *)(&atEHV_Content), AMTBuf+dwNexAddr +8, 28);
					nCountIP = atEHV_Content.IPCOUNT;
					memcpy((char *)(&atEHV_Content) + 28, AMTBuf+dwNexAddr +8 + 28, nCountIP * 4);
					memcpy((char *)(&atEHV_Content) + 108, AMTBuf+dwNexAddr +8 + 28 + nCountIP * 4, 24);
				}
			}
		} 
	}	
	for(i=0;i<ATMHeadItem;i++)
	{
		memcpy(&nType, AMTBuf+9+i*ATM_HEAD_LEN_V4, 1);
		memcpy(&SetTypeCount, AMTBuf+9+i*ATM_HEAD_LEN_V4+1, 2);
		memcpy(&dwItemOffset, AMTBuf+9+i*ATM_HEAD_LEN_V4+3, 4);	
		if(nType == ATM_TYPE_EHV)
		{
			pEHVBuf = (CHAR*)malloc(SetTypeCount * RD_CODE_LEN);

			memset(pEHVBuf, 0, SetTypeCount * RD_CODE_LEN);			
			
			memcpy((char *)(&atEHV_Content), AMTBuf+dwItemOffset +8, 28);
			nCountIP = atEHV_Content.IPCOUNT;
			memcpy((char *)(&atEHV_Content) + 28, AMTBuf+dwItemOffset +8 + 28, nCountIP * 4);
			memcpy((char *)(&atEHV_Content) + 108, AMTBuf+dwItemOffset +8 + 28 + nCountIP * 4, 24);
			
			for(j=0;j<SetTypeCount;j++)
			{			
				strcpy((CHAR*)(pEHVBuf + j*RD_CODE_LEN), atEHV_Content.DevCode);
			//	printf("j=%d,atEHV_Content.DevCode=%s,Strlen=%d\n",j,atEHV_Content.DevCode,strlen(atEHV_Content.DevCode));
				if ((j + 1) < SetTypeCount)
				{
					memcpy(&dwNexAddr,atEHV_Content.NexAddress,4);
			//	printf("dwNexAddr=%d\n",dwNexAddr);
					memcpy((char *)(&atEHV_Content), AMTBuf+dwNexAddr +8, 28);
					nCountIP = atEHV_Content.IPCOUNT;
					memcpy((char *)(&atEHV_Content) + 28, AMTBuf+dwNexAddr +8 + 28, nCountIP * 4);
					memcpy((char *)(&atEHV_Content) + 108, AMTBuf+dwNexAddr +8 + 28 + nCountIP * 4, 24);
				}
			}
			nCountEHV = SetTypeCount;
		}
		if (nType == ATM_TYPE_MC) 
		{			
			pMCBuf = (CHAR*)malloc(SetTypeCount * RD_CODE_LEN);
			memset(pMCBuf, 0, SetTypeCount * RD_CODE_LEN);
			
			memcpy((char *)(&atEHV_Content), AMTBuf+dwItemOffset +8, 28);
			nCountIP = atEHV_Content.IPCOUNT;
		//	printf("nCountIP=%d\n",nCountIP);
			memcpy((char *)(&atEHV_Content) + 28, AMTBuf+dwItemOffset +8 + 28, nCountIP * 4);
			memcpy((char *)(&atEHV_Content) + 108, AMTBuf+dwItemOffset +8 + 28 + nCountIP * 4, 24);
			
			for(j=0;j<SetTypeCount;j++)
			{
				strcpy((CHAR*)(pMCBuf + j*RD_CODE_LEN), atEHV_Content.DevCode);

				if ((j + 1) < SetTypeCount)
				{
					memcpy(&dwNexAddr,atEHV_Content.NexAddress,4);
				
					memcpy((char *)(&atEHV_Content), AMTBuf+dwNexAddr +8, 28);
					nCountIP = atEHV_Content.IPCOUNT;
					memcpy((char *)(&atEHV_Content) + 28, AMTBuf+dwNexAddr +8 + 28, nCountIP * 4);
					memcpy((char *)(&atEHV_Content) + 108, AMTBuf+dwNexAddr +8 + 28 + nCountIP * 4, 24);
				}
			}
			nCountMC = SetTypeCount;
		}
	}
	
	for(i = 0; i< nCountMC; i++)
	{
		if (0 == strcmp(pMCBuf + i * RD_CODE_LEN, "00"))
		{
			bMCConfigured = FALSE;
		}
	}
	
	if (bMCConfigured) 
	{
		pMCBufDef = (char*)malloc(RD_CODE_LEN);
		memset(pMCBufDef, 0, RD_CODE_LEN);
		strcpy(pMCBufDef, "00");
	}

	if (bMCConfigured)
	{
		*ppPUTBuffer = (char*)malloc((nCountEHV + nCountMC + 1) * RD_CODE_LEN);

		memset(*ppPUTBuffer, 0, (nCountEHV + nCountMC + 1) * RD_CODE_LEN);

		memcpy(*ppPUTBuffer, pEHVBuf, nCountEHV*RD_CODE_LEN);

		memcpy(*ppPUTBuffer + nCountEHV*RD_CODE_LEN, pMCBuf, nCountMC * RD_CODE_LEN);
		memcpy(*ppPUTBuffer + (nCountEHV + nCountMC)*RD_CODE_LEN, pMCBufDef, RD_CODE_LEN);
		
		RetCnt	=	nCountEHV + nCountMC + 1;
	}
	else
	{
		*ppPUTBuffer = (char*)malloc((nCountEHV + nCountMC) * RD_CODE_LEN);
		memset(*ppPUTBuffer, 0, (nCountEHV + nCountMC) * RD_CODE_LEN);
		
		memcpy(*ppPUTBuffer, pEHVBuf, nCountEHV*RD_CODE_LEN);
		memcpy(*ppPUTBuffer + nCountEHV*RD_CODE_LEN, pMCBuf, nCountMC * RD_CODE_LEN);
		
		RetCnt	=	nCountEHV + nCountMC;
	}
#ifdef DEBUG_AS_PROC
	for(i=0; i< RetCnt;i++)
		printf("AMT Rd Code: %s\n", *ppPUTBuffer + i * RD_CODE_LEN);
#endif	

	free(pEHVBuf);
	free(pMCBuf);
	free(pMCBufDef);
	return RetCnt;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UpdateRdfromAMT
**	AUTHOR:			Jeff Wang
**	DATE:		13 - Oct - 2008
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
UpdateRdfromAMT()
{
	INT		nCountEHV = 0;	
	RDINFO*	pRdInfo = NULL;	
	BOOL	bFind = FALSE;
	INT		i              = 0;
	INT		nRdCount = 0;
	CHAR	*p = NULL;
	
	//char    PUTBuffer[1024] = {0};
	char * pPUTBuffer=NULL;
	nCountEHV = FdEHVfromAMT(&pPUTBuffer);
	
	p = g_PUInfo.MXPUBuf;

	//Delete inexistent resident in the PUT
	while (nRdCount < g_PUInfo.PUTHead.nResiCnt) 
	{
		pRdInfo = (RDINFO*)p;

		for(i = 0; i < nCountEHV; i++)
		{
			if(CodeCompare(pPUTBuffer + i * RD_CODE_LEN, pRdInfo->RdCode, strlen(pPUTBuffer + i * RD_CODE_LEN)))
			{
				bFind = TRUE;
				break;
			}
		}
		if (!bFind) 
		{
			printf("Remove RD: %s\n", pRdInfo->RdCode);
			PURmRd(pRdInfo->RdCode, strlen(pRdInfo->RdCode));
		}

		p += sizeof(RDINFO);
		nRdCount++;
		pRdInfo = NULL;
		bFind = FALSE;
	}

	//Add new resident to PUT	
	for(i = 0; i < nCountEHV; i++)
	{
		p = g_PUInfo.MXPUBuf;
		nRdCount = 0;
		
		while (nRdCount < g_PUInfo.PUTHead.nResiCnt) 
		{
			pRdInfo = (RDINFO*)p;
			if(CodeCompare(pPUTBuffer + i * RD_CODE_LEN, pRdInfo->RdCode, strlen(pPUTBuffer + i * RD_CODE_LEN)))
			{
				bFind = TRUE;
				break;
			}
			
			p += sizeof(RDINFO);
			nRdCount++;
			pRdInfo = NULL;
		}
		if (!bFind) 
		{
			if (PUAddRd(pPUTBuffer + i * RD_CODE_LEN, strlen(pPUTBuffer + i * RD_CODE_LEN))) 
			{
				printf("Add RD: %s\n", pPUTBuffer + i * RD_CODE_LEN);
			}
		}
		bFind = FALSE;
	}
    SaveRdInfo2Mem();	
	free(pPUTBuffer);
//	PUTBuffer = NULL;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UpdateRdDefaultPwd
**	AUTHOR:			Jeff Wang
**	DATE:		23 - Oct - 2008
**
**	DESCRIPTION:	
**		Update resident default password
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/
void 
UpdateRdDefaultPwd()
{
	RDINFO *pRdInfo = NULL;	
	CHAR	 *p				= NULL;
	INT			nRdCount = 0;

	p= g_PUInfo.MXPUBuf;

	//Delete inexistent resident in the AST
	while (nRdCount < g_PUInfo.PUTHead.nResiCnt) 
	{
		pRdInfo = (RDINFO*)p;

		if (!(pRdInfo->bPwdCfg))
		{
			strcpy(pRdInfo->RdPwd, g_PUPara.ResiPwdDefault);
		}
		p += sizeof(RDINFO);
		nRdCount++;
		pRdInfo = NULL;
	}

    SaveRdInfo2Mem();
	printf("Default password update\n");
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RDCompare
**	AUTHOR:			Jeff Wang
**	DATE:		5 - Dec - 2008
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
static BOOL 
RDCompare(char *pInCode, char *pTableCode, int InCodeLen)
{
	int TableCodeLen = 0;

	EthResolvInfo		ResolvInfo;
	
	memset(&ResolvInfo,0,sizeof(EthResolvInfo));

	memcpy(ResolvInfo.szDevCode, pInCode, InCodeLen);
//	printf("1 ResolvInfo.szDevCode = %s\n",ResolvInfo.szDevCode);
	
	ResolvInfo.nQueryMethod	= HO_QUERY_METHOD_CODE;
	FdFromAMTResolv(&ResolvInfo);	
	
//	printf("ResolvInfo.nType = %d\n",ResolvInfo.nType);
	
	if (ATM_TYPE_NODEVICE != ResolvInfo.nType)
	{
//		printf("2 ResolvInfo.szDevCode = %s\n",ResolvInfo.szDevCode);
//		printf("pTableCode = %s\n",pTableCode);
		if (0 == strcmp(ResolvInfo.szDevCode,pTableCode)) 
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
		
/*	

	if ((NULL == pInCode)||(NULL == pTableCode))
	{
		return FALSE;
	}

	TableCodeLen = strlen(pTableCode);

	if (InCodeLen < g_SysConfig.VldCodeDigits)
	{
		if (InCodeLen != TableCodeLen) 
		{
			return FALSE;
		}

		if (0 == memcmp(pInCode, pTableCode, InCodeLen)) 
		{
			return TRUE;	
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		if (0 == memcmp(pInCode, &(pTableCode[TableCodeLen-g_SysConfig.VldCodeDigits]), g_SysConfig.VldCodeDigits)) 
		{
			return TRUE;	
		}
		else
		{
			return FALSE;
		}
	}
*/

}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	BOOL CodeCompare
**	AUTHOR:			Jeff Wang
**	DATE:		25 - Mar - 2008
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
CodeCompare(CHAR *pInCode, CHAR *pTableCode, INT InCodeLen)
{
	int TableCodeLen = 0;

	if ((NULL == pInCode)||(NULL == pTableCode))
	{
		printf("Lenth Error\n");
		return FALSE;
	}

	TableCodeLen = strlen(pTableCode);
	//printf("TableCodeLen = %d\n", TableCodeLen);
	if (InCodeLen != TableCodeLen) 
	{
		//printf("Len Error\n");
		return FALSE;
	}

#ifdef CODE_COMPARE_DEBUG
	printf("pInCode = %s\n", pInCode);
	printf("pTableCode = %s\n",pTableCode);
#endif

	if (0 == memcmp(pInCode, pTableCode, InCodeLen))
	{
		//printf("compare OK\n");	
		return TRUE;
	}
	else
	{
		//printf("compare Fail\n");
		return FALSE;
	}
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PUGetPwdbyRd
**	AUTHOR:			Mike Zhang
**	DATE:			04 - Jan - 2011
**
**	DESCRIPTION:	
**			Find resident information by resident code
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pRdcode		[IN]		char*
**				RdcodeLen		[IN]		int
**	RETURNED VALUE:	
**				RDINFOMAN* pointer or NULL
**	NOTES:
**			
*/
BOOL
PUGetPwdbyRd(CHAR *PwdStr,CHAR *pRdCode, INT RdCodeLen)
{
	RDINFO*	pRdInfo	= NULL;
	INT		nRdCount = 0;
	CHAR	*p = NULL;
	
	p = g_PUInfo.MXPUBuf;

	RdCodeLen = strlen(pRdCode);
	
	
	while (nRdCount < g_PUInfo.PUTHead.nResiCnt)
	{
		pRdInfo = (RDINFO*) p;
		if (RDCompare(pRdCode, pRdInfo->RdCode, RdCodeLen))
		{
			memcpy(PwdStr,pRdInfo->RdPwd,PWD_LEN);
			return TRUE;
			
		}
		p += sizeof(RDINFO);
		nRdCount++;
		pRdInfo = NULL;
	}
	
	return FALSE;
}






















