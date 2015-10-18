/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	FileConfigure.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		08 - May - 2009
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
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "Dispatch.h"
#include "MXCommon.h"
#include "FileConfigure.h"
#include "MenuParaProc.h"
#include "ParaSetting.h"
#include "EGMLCAgent.h"
#include "LiftCtrlByDOModule.h"
#include "AccessProc.h"
#include "hash.h"


/************** DEFINES **************************************************************/

#define DEBUG_DWLD_FILE
#define DEBUG_UPLD_FILE

/************** TYPEDEFS *************************************************************/

typedef struct _DWNLDPKTINFO
{
	BYTE	PktFlag;
	WORD	PktIndex;
}DWNLDPKTINFO;


typedef	struct _DWNLDFILEINFO 
{
	UINT	FileLen;
	CHAR	FileName[TITLE_BUF_LEN];
}DWNLDFILEINFO;

typedef struct _UPLDPKTINFO
{
	WORD	PktIndex;
	UINT	nTotalSndLen;
}UPLDPKTINFO;


/************** STRUCTURES ***********************************************************/

extern HANDLE*   g_HandleHashCard ;//卡片存储的句柄
/************** EXTERNAL DECLARATIONS ************************************************/


//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/


/************** LOCAL DECLARATIONS ***************************************************/


/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DwnldFileProc
**	AUTHOR:			Jeff Wang
**	DATE:			08 - May - 2009
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
BOOL
DwnldFileProc(BYTE* pIn, UINT nInLen)
{
	static DWNLDPKTINFO	 LastDwnldPktInfo;
	static DWNLDFILEINFO DwnldFileInfo;
	static UINT			 nTotalRcvFileLen = 0;

	DWNLDPKTINFO	CurDwnldFileInfo;
	UINT			CurPktLen	=	0;
	BOOL			bRet	=	FALSE;
	FILE*			fd			= NULL;

	CHAR			FilePath[TITLE_BUF_LEN * 2] = { 0 };
	CHAR			StrCmd[TITLE_BUF_LEN * 2] = { 0 };

	memcpy(&CurDwnldFileInfo.PktFlag,	pIn + DWNLDFILE_PKTFLAG_OFFSET, DWNLDFILE_PKTFLAG_LEN);
	memcpy(&CurDwnldFileInfo.PktIndex,	pIn + DWNLDFILE_PKTINDEX_OFFSET, DWNLDFILE_PKTINDEX_LEN);
	
	switch(CurDwnldFileInfo.PktFlag) 
	{
	case PKTFLAG_FIRST:
		{
			memcpy(&DwnldFileInfo.FileLen, pIn + DWNLDFILE_FILELEN_OFFSET, DWNLDFILE_FILELEN_LEN);
			strcpy(DwnldFileInfo.FileName, pIn + DWNLDFILE_FILEDATA_OFFSET);

#ifdef DEBUG_DWLD_FILE
			printf("Dld File Len = %d\n", DwnldFileInfo.FileLen);
			printf("Dld File Name = %s\n", DwnldFileInfo.FileName);			
#endif
			if((DEVICE_CORAL_DB == g_DevFun.DevType) && 
				((0 == strcmp(DwnldFileInfo.FileName,"AST"))
				||(0 == strcmp(DwnldFileInfo.FileName,"APT"))	
				||(0 == strcmp(DwnldFileInfo.FileName,"ALT"))
				||(0 == strcmp(DwnldFileInfo.FileName,"HASHFUNC"))
				||(0 == strcmp(DwnldFileInfo.FileName,"HASHLIST"))
				||(0 == strcmp(DwnldFileInfo.FileName,"HASHDATA"))
				||(0 == strcmp(DwnldFileInfo.FileName,"HASHINFO"))))
			{
				printf("%s,%d\r\n",__func__,__LINE__);
				bRet	=	FALSE;
				break;
			}


			if((strcmp(DwnldFileInfo.FileName,"AST") == 0)||(0 == strcmp(DwnldFileInfo.FileName,"APT"))){
				printf("%s,%d error :File Name = %s  STM 32 not support \r\n",__func__,__LINE__,DwnldFileInfo.FileName);
				bRet	=	FALSE;
				break;
			}
			
			printf("%s,%d DwnldFileInfo.FileName = %s\r\n",__func__,__LINE__,DwnldFileInfo.FileName);


			//清空卡片数据
			if((DwnldFileInfo.FileLen == 0)&&(0 == strcmp(DwnldFileInfo.FileName,"HASHFUNC")))
				{
					printf("^_^!!! %s--%d  clean card  data g_HandleHashCard.addr = %p\n",__func__,__LINE__,g_HandleHashCard);					
					CloseCardDataBase(g_HandleHashCard);
					system("rm -fr /mox/rdwr/HASH");
					printf("^_^!!! %s--%d  clean card  data \n",__func__,__LINE__);
					g_HandleHashCard = OpenCardDataBase();//初始化 hashcard	
					printf("^_^!!! %s--%d  New g_HandleHashCard.addr = %p\n",__func__,__LINE__,g_HandleHashCard);
					memset(&LastDwnldPktInfo, 0, sizeof(DWNLDPKTINFO));
					memset(&DwnldFileInfo,	  0, sizeof(DWNLDFILEINFO));
					nTotalRcvFileLen = 0;
					bRet	=	TRUE;	
			}else if (DwnldFileInfo.FileLen > 0) 
			{
				nTotalRcvFileLen	=	0;
				memcpy(&LastDwnldPktInfo, &CurDwnldFileInfo, sizeof(DWNLDPKTINFO));
				
				bRet	=	TRUE;
			}
			else	
			{
				memset(&LastDwnldPktInfo, 0, sizeof(DWNLDPKTINFO));
				memset(&DwnldFileInfo,	  0, sizeof(DWNLDFILEINFO));
				nTotalRcvFileLen = 0;

				bRet	=	FALSE;
			}
		}
		break;
		
	case PKTFLAG_MIDDLE:
		{
			if (DwnldFileInfo.FileLen > 0
				&& CurDwnldFileInfo.PktIndex - LastDwnldPktInfo.PktIndex == 1
				) 
			{
				memcpy(&CurPktLen, pIn + DWNLDFILE_FILELEN_OFFSET, sizeof(UINT));
				memcpy(&DownloadDataBuf[nTotalRcvFileLen], pIn + DWNLDFILE_FILEDATA_OFFSET, CurPktLen);
				nTotalRcvFileLen += CurPktLen;
				memcpy(&LastDwnldPktInfo, &CurDwnldFileInfo, sizeof(DWNLDPKTINFO));
				
				bRet	=	TRUE;
			}
			else
			{
				memset(&LastDwnldPktInfo, 0, sizeof(DWNLDPKTINFO));
				memset(&DwnldFileInfo,	  0, sizeof(DWNLDFILEINFO));
				nTotalRcvFileLen = 0;
				
				bRet	=	FALSE;
			}
		}
		break;
		
	case PKTFLAG_LAST:
		{
			memcpy(&CurPktLen, pIn + DWNLDFILE_FILELEN_OFFSET, sizeof(UINT));

#ifdef DEBUG_DWLD_FILE				
			printf("nTotalRcvFileLen = %d\n", nTotalRcvFileLen);
			printf("CurPktLen = %d\n", CurPktLen);
			printf("CurDwnldFileInfo.PktIndex = %d\n", CurDwnldFileInfo.PktIndex);
			printf("LastDwnldPktInfo.PktIndex = %d\n", LastDwnldPktInfo.PktIndex);
#endif
			if (DwnldFileInfo.FileLen == nTotalRcvFileLen + CurPktLen
				&& CurDwnldFileInfo.PktIndex - LastDwnldPktInfo.PktIndex == 1
				) 
			{
				memcpy(&DownloadDataBuf[nTotalRcvFileLen], pIn + DWNLDFILE_FILEDATA_OFFSET, CurPktLen);
				nTotalRcvFileLen += CurPktLen;
				
#ifdef DEBUG_DWLD_FILE				
				printf("********nTotalRcvFileLen finished = %d ****************\n", nTotalRcvFileLen);
#endif

				if (DwnldFileInfo.FileLen > 0 && strcmp(DwnldFileInfo.FileName, ""))
				{
					if (strcmp(DwnldFileInfo.FileName,DOWNLOAD_FILENAME_CARD_HASH_FUNC)== 0)
					{
						printf("^_^!!! %s--%d  clean card  data \n",__func__,__LINE__);
						CloseCardDataBase(g_HandleHashCard);
						system("rm -fr /mox/rdwr/HASH");
						printf("^_^!!! %s--%d  clean card  data \n",__func__,__LINE__);
						g_HandleHashCard = OpenCardDataBase();//初始化 hashcard	
						SetCardBuffInfo(g_HandleHashCard,0,DownloadDataBuf,nTotalRcvFileLen,0);
					}else if (strcmp(DwnldFileInfo.FileName,DOWNLOAD_FILENAME_CARD_HASH_LIST)== 0)
					{
						SetCardBuffInfo(g_HandleHashCard,1,DownloadDataBuf,nTotalRcvFileLen,0);
					}else if (strcmp(DwnldFileInfo.FileName,DOWNLOAD_FILENAME_CARD_HASH_DATA)== 0)
					{
						SetCardBuffInfo(g_HandleHashCard,2,DownloadDataBuf,nTotalRcvFileLen,0);
					}else if (strcmp(DwnldFileInfo.FileName,DOWNLOAD_FILENAME_CARD_HASH_INFO)== 0)
					{
						SetCardBuffInfo(g_HandleHashCard,3,DownloadDataBuf,nTotalRcvFileLen,0);
					}else
					{
						sprintf(StrCmd, "rm -fr /mox/rdwr/%s", DwnldFileInfo.FileName);
						system(StrCmd);
						
						sprintf(FilePath, "/mox/rdwr/%s", DwnldFileInfo.FileName);

						if ((fd = fopen(FilePath, "w+")) != NULL)
						{
							fseek(fd, 0, SEEK_SET);					
							fwrite(DownloadDataBuf, nTotalRcvFileLen, (size_t)1, fd);					
							fclose(fd);

							/*if (strcmp(FilePath, ASTFILE) == 0)
							{
								LoadCdInfoFromMem();
							}
							if (strcmp(FilePath, APTFILE) == 0)
							{
								LoadPatrolCdInfoFromMem();
							}*/
							if (strcmp(FilePath, AATFILE) == 0)	//add for Authorize Card
							{
								LoadAuthorizeCdInfoFromMem();
							}
							if (strcmp(FilePath, ALTFILE) == 0)
							{
								LoadLCInfo();
							}
							if (strcmp(FilePath, DOLTFILE) == 0)
							{
								modbus_LT_para_init();
							}						
#ifdef DEBUG_DWLD_FILE				
							printf("File DownLoad Success\n");				
#endif					
						}
						else
						{
#ifdef DEBUG_DWLD_FILE				
							printf("File open error\n");
#endif					
						}

					}
					bRet	=	TRUE;
				}
				else
				{
					bRet	=	FALSE;
				}
			}
			else
			{
				bRet	=	FALSE;
			}

			memset(&LastDwnldPktInfo, 0, sizeof(DWNLDPKTINFO));
			memset(&DwnldFileInfo,	  0, sizeof(DWNLDFILEINFO));
			nTotalRcvFileLen = 0;
		}
		break;
	default:
		break;
	}

	return bRet;	
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UpldFileProc
**	AUTHOR:			Jeff Wang
**	DATE:			08 - May - 2009
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pInOut		[OUT]		BYTE*
**	
**	RETURNED VALUE:	
**				Return the data lenth
**	NOTES:
**			
*/



UINT
UpldFileProc(BYTE* pInOut)
{
	static  UPLDPKTINFO	LastUpldPktInfo;	

	FILE*				fd			= NULL;
	struct	stat		pStat;

	WORD			CurIndex	=	0;
	UINT			StaPos		=	0;
	
	WORD			FileNameLen =	0;
	CHAR			FileName[TITLE_BUF_LEN] = { 0 };

	CHAR			FilePath[TITLE_BUF_LEN * 2] = { 0 };

	WORD			nPktLen		=	0;
	UINT			nRetLen		=	0;

	BYTE* 			hashCardData = NULL;

	

#ifdef DEBUG_UPLD_FILE
	UINT i = 0;	
	for(i=0;i<15;i++)
	{
		printf("pInOut[%d] = %d\n", i, pInOut[i]);
	}
#endif
	
	memcpy(&CurIndex,		pInOut + UPLDFILE_INDEX_OFFSET, UPLDFILE_INDEX_LEN);					
	memcpy(&StaPos,			pInOut + UPLDFILE_STAPOS_OFFSET, UPLDFILE_STAPOS_LEN);
	memcpy(&FileNameLen,	pInOut + UPLDFILE_FILENAMELEN_OFFSET, UPLDFILE_FILENAMELEN_LEN);
	strcpy(FileName,		pInOut + UPLDFILE_FILENAME_OFFSET);

	
#ifdef DEBUG_UPLD_FILE
	printf("CurIndex = %d\n", CurIndex);
	printf("StaPos = %d\n", StaPos);
	printf("FileName = %s\n", FileName);
#endif
	


if((DEVICE_CORAL_DB == g_DevFun.DevType) && 
		((0 == strcmp(FileName,"AST"))
		||(0 == strcmp(FileName,"APT"))
		||(0 == strcmp(FileName,"ALT"))
		||(0 == strcmp(FileName,"HASHDATA"))))
	{
		printf("%s,%d\r\n",__func__,__LINE__);
		return -1;
	}
	
	sprintf(FilePath, "/mox/rdwr/%s", FileName);

	if((strcmp(FileName,"AST") == 0)||(0 == strcmp(FileName,"APT")))
		{
			printf("%s,%d error:FileName = %s STM 32 not support \r\n",__func__,__LINE__,FileName);
			return -1;
		}


#ifdef DEBUG_UPLD_FILE
	printf("FilePath = %s\n", FilePath);
#endif

	memcpy(pInOut + UPLDFILE_INDEX_OFFSET, &CurIndex, UPLDFILE_INDEX_LEN);
	nRetLen += UPLDFILE_INDEX_LEN;

	if((0 == strcmp(FileName,"HASHDATA")))
	{
		GetCardBuffInfo(g_HandleHashCard,2,&hashCardData,&pStat.st_size);
		if (pStat.st_size > 0)
				{
					memcpy(pInOut + UPLDFILE_FILELEN_OFFSET, &pStat.st_size, UPLDFILE_FILELEN_LEN);
					nRetLen += UPLDFILE_FILELEN_LEN;
					*(pInOut + UPLDFILE_RESULT_OFFSET) = FILE_OK;
					nRetLen += UPLDFILE_RESULT_LEN;
					
					if (pStat.st_size >= LastUpldPktInfo.nTotalSndLen + PKT_PER_LEN)
					{
						nPktLen = PKT_PER_LEN;
					}
					else
					{
						nPktLen =	(WORD)(pStat.st_size - LastUpldPktInfo.nTotalSndLen);
					}		
					memcpy(pInOut + UPLDFILE_PKTLEN_OFFSET, &nPktLen, UPLDFILE_PKTLEN_LEN);
					nRetLen += UPLDFILE_PKTLEN_LEN;		

					memcpy(pInOut + UPLDFILE_FILEDATA_OFFSET, (hashCardData + LastUpldPktInfo.nTotalSndLen) ,nPktLen);
					LastUpldPktInfo.nTotalSndLen += nPktLen;
					nRetLen += nPktLen;
					printf("%s,%d \r\n",__func__,__LINE__);
#ifdef DEBUG_UPLD_FILE
					printf("Data Packet finished1\n");
					printf("nRetLen = %d\n", nRetLen);
#endif			


					if (pStat.st_size == LastUpldPktInfo.nTotalSndLen)
					{
#ifdef DEBUG_UPLD_FILE
						printf("File send finished1\n");
#endif		
						memset(&LastUpldPktInfo, 0, sizeof(UPLDPKTINFO));
					}
				}
				else
				{
					memcpy(pInOut + UPLDFILE_FILELEN_OFFSET, &pStat.st_size, UPLDFILE_FILELEN_LEN);
					nRetLen += UPLDFILE_FILELEN_LEN;
					*(pInOut + UPLDFILE_RESULT_OFFSET) = FILE_NOT_EXIST;
					nRetLen += UPLDFILE_RESULT_LEN;
		
#ifdef DEBUG_UPLD_FILE
					printf("Data Packet finished2\n");
#endif			
				}
	}
	else if ((fd = fopen(FilePath, "r+")) != NULL)
	{
		if (-1 == stat(FilePath, &pStat)) 
		{
			printf("AST file Information error\n");
			return 0;
		}

#ifdef DEBUG_UPLD_FILE
		printf("Size = %d\n", pStat.st_size);
#endif
		if (pStat.st_size > 0)
		{
			memcpy(pInOut + UPLDFILE_FILELEN_OFFSET, &pStat.st_size, UPLDFILE_FILELEN_LEN);
			nRetLen += UPLDFILE_FILELEN_LEN;
			*(pInOut + UPLDFILE_RESULT_OFFSET) = FILE_OK;
			nRetLen += UPLDFILE_RESULT_LEN;
			
			if (pStat.st_size >= LastUpldPktInfo.nTotalSndLen + PKT_PER_LEN)
			{
				nPktLen	= PKT_PER_LEN;
			}
			else
			{
				nPktLen	=	(WORD)(pStat.st_size - LastUpldPktInfo.nTotalSndLen);
			}		
			
			LastUpldPktInfo.nTotalSndLen += nPktLen;
			memcpy(pInOut + UPLDFILE_PKTLEN_OFFSET, &nPktLen, UPLDFILE_PKTLEN_LEN);
			nRetLen += UPLDFILE_PKTLEN_LEN;
			
			fseek(fd, StaPos, SEEK_SET);
			fread(pInOut + UPLDFILE_FILEDATA_OFFSET, nPktLen, 1, fd);
			nRetLen += nPktLen;

#ifdef DEBUG_UPLD_FILE
			printf("Data Packet finished1\n");
			printf("nRetLen = %d\n", nRetLen);
#endif			
			
			if (pStat.st_size == LastUpldPktInfo.nTotalSndLen)
			{
#ifdef DEBUG_UPLD_FILE
				printf("File send finished1\n");
#endif		
				memset(&LastUpldPktInfo, 0, sizeof(UPLDPKTINFO));
			}
		}
		else
		{
			memcpy(pInOut + UPLDFILE_FILELEN_OFFSET, &pStat.st_size, UPLDFILE_FILELEN_LEN);
			nRetLen += UPLDFILE_FILELEN_LEN;
			*(pInOut + UPLDFILE_RESULT_OFFSET) = FILE_NOT_EXIST;
			nRetLen += UPLDFILE_RESULT_LEN;

#ifdef DEBUG_UPLD_FILE
			printf("Data Packet finished2\n");
#endif			
		}
		
		fclose(fd);
	}
	else
	{
#ifdef DEBUG_UPLD_FILE
		printf("%s file open error\n");
#endif
		
		memset(pInOut + UPLDFILE_FILELEN_OFFSET, 0, UPLDFILE_FILELEN_LEN);
		nRetLen += UPLDFILE_FILELEN_LEN;

        if((!strcmp(CLT_FILE,FilePath)) || (!strcmp(VAT_FILE,FilePath)) || (!strcmp(PUTFILE,FilePath))) //added by [MichaelMa] at 21.8.2012 fot bug 9848,9337
        {
            *(pInOut + UPLDFILE_RESULT_OFFSET) = FILE_OK;
        }                                               
        else                                             
        {
            *(pInOut + UPLDFILE_RESULT_OFFSET) = FILE_OPEN_ERROR; 
        }
        
		//*(pInOut + UPLDFILE_RESULT_OFFSET) = FILE_OPEN_ERROR;
		nRetLen += UPLDFILE_RESULT_LEN;
	}
#ifdef DEBUG_UPLD_FILE
	printf("nRetLen = %d\n", nRetLen);
#endif
	
	return nRetLen;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RemoveFileProc
**	AUTHOR:			Jeff Wang
**	DATE:			11 - May - 2009
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pIn			[IN]		BYTE*
**	
**	RETURNED VALUE:	
**				TRUE if Removed, else FALSE
**	NOTES:
**			
*/
BOOL
RemoveFileProc(BYTE* pIn)
{
	WORD			FileNameLen =	0;
	CHAR			FileName[TITLE_BUF_LEN] = { 0 };
	CHAR			FilePath[TITLE_BUF_LEN * 2] = { 0 };
	CHAR			CmdBUf[TITLE_BUF_LEN * 2] = { 0 };
	FILE*			fd = NULL;
	
//	memcpy(&FileNameLen,	pIn + UPLDFILE_FILENAMELEN_OFFSET, UPLDFILE_FILENAMELEN_LEN);
	memcpy(&FileNameLen,	pIn + 0, UPLDFILE_FILENAMELEN_LEN);
	
	if (FileNameLen <= 0 || FileNameLen > 256) 
	{
		printf("RemoveFileProc ERROR: FileNameLen = %d\n",FileNameLen);
		return FALSE;
	}

#ifdef DEBUG_DWLD_FILE
	printf("RemoveFileProc FileNameLen = %d\n",FileNameLen);
#endif

//	memcpy(&FileName,		pIn + UPLDFILE_FILENAME_OFFSET, FileNameLen);
	memcpy(&FileName,		pIn + 2, FileNameLen);
	FileName[FileNameLen]	=	'\0';

	if (0 == strcmp(FileName,"")) 
	{
		printf("RemoveFileProc ERROR: FileName Empty\n");
		return FALSE;
	}
	

#ifdef DEBUG_DWLD_FILE
	printf("RemoveFileProc FileName = %s\n",FileName);
#endif
	
	if (FileNameLen > 0 && strcmp(FileName, ""))
	{
		sprintf(FilePath, "/mox/rdwr/%s", FileName);
		
		if(fd = (fopen(FilePath, "r+") != NULL))
		{
			sprintf(CmdBUf, "rm -rf /mox/rdwr/%s", FileName);
			system(CmdBUf);
			return TRUE;		
		}
		else
		{
			printf("File Open Error\n");
			return FALSE;
		}
	}
	
	return FALSE;
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DwnldOneCardProc
**	AUTHOR:			Benson Chen
**	DATE:			27 - Jan - 2015
**
**	DESCRIPTION:	
**		MC uses this function code to add a card to the access controller or GM. 
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pInOut		[OUT]		BYTE*
**	
**	RETURNED VALUE:	
**				Return the sucess(true) or fail(false) 
**	NOTES:
**			
*/
BOOL
DwnldOneCardProc(BYTE* pIn, UINT nInLen ,unsigned short nFunCode)

{
	BYTE	pVersion;  	//Version
	BYTE	CardType = 0;
	CDINFO	pCdInfo ;		
	UINT	nRet = 0;
	switch(nFunCode)
	{
		case FC_AC_MC_ADD_ONE_CARD:	CardType = TYPE_NORMAL_CARD;break;

		case FC_PC_MC_ADD_ONE_CARD:	CardType = TYPE_PATROL_CARD;break;

		default : 
			return FALSE;
	}
	memcpy(&pVersion,	pIn + DWNLDOREDITONECARD_VERSION_OFFSET, DWNLDOREDITONECARD_VERSION_LEN);

	memcpy(&pCdInfo.CSN,pIn + DWNLDOREDITONECARD_CSN_OFFSET,DWNLDOREDITONECARD_CSN_LEN);
	
	switch(nFunCode)
	{
		case FC_AC_MC_ADD_ONE_CARD:	CardType = TYPE_NORMAL_CARD;break;

		case FC_PC_MC_ADD_ONE_CARD:	CardType = TYPE_PATROL_CARD;break;

		default : 
			return FALSE;
	}
	memset(&pCdInfo,0,sizeof(pCdInfo));
	memcpy(&pCdInfo.CSN,pIn + DWNLDOREDITONECARD_CSN_OFFSET,DWNLDOREDITONECARD_CSN_LEN);
	memcpy(&pCdInfo.RdNum,pIn + DWNLDOREDITONECARD_RESIDENTCODE_OFFSET,DWNLDOREDITONECARD_RESIDENTCODE_LEN);
	memcpy(&pCdInfo.CardStatus,pIn + DWNLDOREDITONECARD_CARDSTATUS_OFFSET,DWNLDOREDITONECARD_CARDSTATUS_LEN);
	memcpy(&pCdInfo.CardMode,pIn + DWNLDOREDITONECARD_CARDMODE_OFFSET,DWNLDOREDITONECARD_CARDMODE_LEN);
	memcpy(&pCdInfo.GateNumber,pIn + DWNLDOREDITONECARD_GATENUMBER_OFFSET,DWNLDOREDITONECARD_GATENUMBER_LEN);
	memcpy(&pCdInfo.UnlockTimeSliceID,pIn + DWNLDOREDITONECARD_UNLOCKTIMESLICEID_OFFSET,DWNLDOREDITONECARD_UNLOCKTIMESLICEID_LEN);
	memcpy(&pCdInfo.ValidStartTime,pIn + DWNLDOREDITONECARD_VALIDSTARTTIME_OFFSET,DWNLDOREDITONECARD_VALIDSTARTTIME_LEN);
	memcpy(&pCdInfo.ValidEndTime,pIn + DWNLDOREDITONECARD_VALIDENDTIME_OFFSET,DWNLDOREDITONECARD_VALIDENDTIME_LEN);
	memcpy(&pCdInfo.CardType,&CardType,DWNLDOREDITONECARD_CARDTYPE_LEN);
	memcpy(&pCdInfo.bAdmin,pIn + DWNLDOREDITONECARD_ISADMIN_OFFSET,DWNLDOREDITONECARD_ISADMIN_LEN);
	
	AsPrintCd(&pCdInfo);

	if (FALSE == WriteCardInfo(g_HandleHashCard,&pCdInfo,CardType))
	{
		printf("%s,%d,File DwnldOneCardProc Card\n",__func__,__LINE__);		
		return FALSE;
	}

	printf("%s,%d,Sucess DwnldOneCardProc Card\n",__func__,__LINE__);	
	
	return TRUE;
	
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DeleteOneCardProc
**	AUTHOR:			Benson Chen
**	DATE:			27 - Jan - 2015
**
**	DESCRIPTION:	
**		MC uses this command to delete an access card in the  GM.  
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pInOut		[OUT]		BYTE*
**	
**	RETURNED VALUE:	
**				Return the sucess(true) or fail(false) 
**	NOTES:
**			
*/

BOOL
DeleteOneCardProc(BYTE* pIn, UINT nInLen,unsigned short nFunCode)
{

	BYTE	pVersion;	//Version
	BYTE	CSN[CSN_LEN]	;		
	//UINT	nRet = 0;
	BYTE	CardType;
	switch(nFunCode)
	{
		case FC_AC_MC_DEL_ONE_CARD: CardType = TYPE_NORMAL_CARD;break;

		case FC_PC_MC_DEL_ONE_CARD:	CardType = TYPE_PATROL_CARD;break;

		default : 
			break;
	}
	memcpy(&pVersion,	pIn + DELETEONECARD_VERSION_OFFSET, DELETEONECARD_VERSION_LEN);

	/*if(g_AHASHCARDInfo.ASTHead.VersionNum == pVersion)
	{
		return FALSE;
	}*/	
	
	memcpy(&CSN,pIn + DWNLDOREDITONECARD_CSN_OFFSET,DWNLDOREDITONECARD_CSN_LEN);

	if( DeleteCard(g_HandleHashCard,&CSN,CardType) == FALSE)
	{
		printf("File DeleteOneCardProc Card\n");

		return FALSE;
	}
	
	printf("Sucess DeleteOneCardProc Card\n");


	return TRUE;

}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EditOneCardProc
**	AUTHOR:			Benson Chen
**	DATE:			27 - Jan - 2015
**
**	DESCRIPTION:	
**		MC uses this command to edit an access card to the  GM. 
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pInOut		[OUT]		BYTE*
**	
**	RETURNED VALUE:	
**				Return the sucess(true) or fail(false) 
**	NOTES:
**			
*/

BOOL
EditOneCardProc(BYTE* pIn, UINT nInLen,unsigned short nFunCode)
{

	BYTE	pVersion;  	//Version
	CDINFO	pCdInfo;		
	UINT	nRet = 0;
	BYTE 	CardType = 0;
	

	memcpy(&pVersion,	pIn + DWNLDOREDITONECARD_VERSION_OFFSET, DWNLDOREDITONECARD_VERSION_LEN);

	/*if(g_AHASHCARDInfo.ASTHead.VersionNum == pVersion)
	{
		return FALSE;
	}*/
	switch(nFunCode)
	{
		case FC_AC_MC_EDIT_ONE_CARD:CardType = TYPE_NORMAL_CARD;break;

		case FC_PC_MC_EDIT_ONE_CARD:CardType = TYPE_PATROL_CARD;break;

		default : 
			return FALSE;
	}

	memcpy(&pCdInfo.CSN,pIn + DWNLDOREDITONECARD_CSN_OFFSET,DWNLDOREDITONECARD_CSN_LEN);

	if( DeleteCard(g_HandleHashCard,&pCdInfo.CSN,CardType)== FALSE)
	{
		printf("File EditOneCardProc Card\n");

		return FALSE;
	}
	//SaveOrSynchronousCardHashInfo2Mem();
	memset(&pCdInfo,0,sizeof(pCdInfo));
	memcpy(&pCdInfo.CSN,pIn + DWNLDOREDITONECARD_CSN_OFFSET,DWNLDOREDITONECARD_CSN_LEN);
	memcpy(&pCdInfo.RdNum,pIn + DWNLDOREDITONECARD_RESIDENTCODE_OFFSET,DWNLDOREDITONECARD_RESIDENTCODE_LEN);
	memcpy(&pCdInfo.CardStatus,pIn + DWNLDOREDITONECARD_CARDSTATUS_OFFSET,DWNLDOREDITONECARD_CARDSTATUS_LEN);
	memcpy(&pCdInfo.CardMode,pIn + DWNLDOREDITONECARD_CARDMODE_OFFSET,DWNLDOREDITONECARD_CARDMODE_LEN);
	memcpy(&pCdInfo.GateNumber,pIn + DWNLDOREDITONECARD_GATENUMBER_OFFSET,DWNLDOREDITONECARD_GATENUMBER_LEN);
	memcpy(&pCdInfo.UnlockTimeSliceID,pIn + DWNLDOREDITONECARD_UNLOCKTIMESLICEID_OFFSET,DWNLDOREDITONECARD_UNLOCKTIMESLICEID_LEN);
	memcpy(&pCdInfo.ValidStartTime,pIn + DWNLDOREDITONECARD_VALIDSTARTTIME_OFFSET,DWNLDOREDITONECARD_VALIDSTARTTIME_LEN);
	memcpy(&pCdInfo.ValidEndTime,pIn + DWNLDOREDITONECARD_VALIDENDTIME_OFFSET,DWNLDOREDITONECARD_VALIDENDTIME_LEN);
	memcpy(&pCdInfo.CardType,&CardType,DWNLDOREDITONECARD_CARDTYPE_LEN);
	memcpy(&pCdInfo.bAdmin,pIn + DWNLDOREDITONECARD_ISADMIN_OFFSET,DWNLDOREDITONECARD_ISADMIN_LEN);
	
	printf("CardType : %d\n",CardType);

	

	if (FALSE == WriteCardInfo(g_HandleHashCard,&pCdInfo,CardType))
	{
		printf("File EditOneCardProc Card\n");
		return FALSE;
	}
	AsPrintCd(&pCdInfo);
	printf("Sucess EditOneCardProc Card\n");
	return TRUE;


}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UpldOneCardProc
**	AUTHOR:			Benson Chen
**	DATE:			27 - Jan - 2015
**
**	DESCRIPTION:	
**			MC uses this function code to query an access card from the access controller or GM.
**		
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pInOut		[OUT]		BYTE*
**	
**	RETURNED VALUE:	
**				Return the data lenth
**	NOTES:
**			
*/
UINT
UpldOneCardProc(BYTE* pInOut,unsigned short nFunCode)
{
	
	BYTE	pVersion = 1;	//Version
	CDINFO	pCdInfo;
	BYTE 	CardType = 0;
	
	UINT			nRetLen		=	0;
	memcpy(&pVersion,	pInOut + DWNLDOREDITONECARD_VERSION_OFFSET, DWNLDOREDITONECARD_VERSION_LEN);

	/*if(g_AHASHCARDInfo.ASTHead.VersionNum == pVersion)
	{
		return nRetLen;
	}*/

	
	memcpy(&pCdInfo.CSN,pInOut + DWNLDOREDITONECARD_CSN_OFFSET,DWNLDOREDITONECARD_CSN_LEN);
	switch(nFunCode)
	{
		case FC_AC_MC_QUERY_CARD:	CardType = TYPE_NORMAL_CARD;break;

		case FC_PC_MC_QUERY_CARD:	CardType = TYPE_PATROL_CARD;break;

		default : 
			return nRetLen;
	}

	//AsPrintCd(&pCdInfo);
	memset(&pCdInfo,0,sizeof(pCdInfo));
	if(ReadCard(g_HandleHashCard,&pCdInfo.CSN,CardType,&pCdInfo) == FALSE)
	{
		printf("File Read Card\n");
		
		memcpy(pInOut + DWNLDOREDITONECARD_VERSION_OFFSET,&pVersion, DWNLDOREDITONECARD_VERSION_LEN);
		nRetLen += DWNLDOREDITONECARD_VERSION_LEN;
		memcpy(pInOut + DWNLDOREDITONECARD_CSN_OFFSET,&pCdInfo.CSN,DWNLDOREDITONECARD_CSN_LEN);
		nRetLen += DWNLDOREDITONECARD_CSN_LEN;
		
		return nRetLen;
	}

	memcpy(pInOut + DWNLDOREDITONECARD_VERSION_OFFSET,&pVersion, DWNLDOREDITONECARD_VERSION_LEN);
	nRetLen += DWNLDOREDITONECARD_VERSION_LEN;
	memcpy(pInOut + DWNLDOREDITONECARD_CSN_OFFSET,&pCdInfo.CSN,DWNLDOREDITONECARD_CSN_LEN);
	nRetLen += DWNLDOREDITONECARD_CSN_LEN;
	memcpy(pInOut + DWNLDOREDITONECARD_RESIDENTCODE_OFFSET,&pCdInfo.RdNum,DWNLDOREDITONECARD_RESIDENTCODE_LEN);
	nRetLen += DWNLDOREDITONECARD_RESIDENTCODE_LEN;
	//memcpy(&pCdInfo.CardType,pIn + DWNLDOREDITONECARD_CARDTYPE_OFFSET,DWNLDOREDITONECARD_CARDTYPE_LEN);
	memcpy(pInOut + DWNLDOREDITONECARD_CARDSTATUS_OFFSET,&pCdInfo.CardStatus,DWNLDOREDITONECARD_CARDSTATUS_LEN);
	nRetLen += DWNLDOREDITONECARD_CARDSTATUS_LEN;
	memcpy(pInOut + DWNLDOREDITONECARD_CARDMODE_OFFSET,&pCdInfo.CardMode,DWNLDOREDITONECARD_CARDMODE_LEN);
	nRetLen += DWNLDOREDITONECARD_CARDMODE_LEN;
	memcpy(pInOut + DWNLDOREDITONECARD_GATENUMBER_OFFSET,&pCdInfo.GateNumber,DWNLDOREDITONECARD_GATENUMBER_LEN);
	nRetLen += DWNLDOREDITONECARD_GATENUMBER_LEN;
	memcpy(pInOut + DWNLDOREDITONECARD_UNLOCKTIMESLICEID_OFFSET,&pCdInfo.UnlockTimeSliceID,DWNLDOREDITONECARD_UNLOCKTIMESLICEID_LEN);
	nRetLen += DWNLDOREDITONECARD_UNLOCKTIMESLICEID_LEN;
	memcpy(pInOut + DWNLDOREDITONECARD_VALIDSTARTTIME_OFFSET,&pCdInfo.ValidStartTime,DWNLDOREDITONECARD_VALIDSTARTTIME_LEN);
	nRetLen += DWNLDOREDITONECARD_VALIDSTARTTIME_LEN;
	memcpy(pInOut + DWNLDOREDITONECARD_VALIDENDTIME_OFFSET,&pCdInfo.ValidEndTime,DWNLDOREDITONECARD_VALIDENDTIME_LEN);
	nRetLen += DWNLDOREDITONECARD_VALIDENDTIME_LEN;
	memcpy(pInOut + DWNLDOREDITONECARD_ISADMIN_OFFSET,&pCdInfo.bAdmin,DWNLDOREDITONECARD_ISADMIN_LEN);
	nRetLen += DWNLDOREDITONECARD_ISADMIN_LEN;

	AsPrintCd(&pCdInfo);
	
	printf("nRetLen = %d\n", nRetLen);

	return nRetLen;
}


