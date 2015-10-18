/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	AMT.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		25 - Dec - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	
**
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "Dispatch.h"
#include "IniFile.h"
#include "MXCommon.h"
#include "MenuParaProc.h"
#include "BacpNetCtrl.h"
#include "AMT.h"

/************** DEFINES **************************************************************/
//#define DEBUG_AMT

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

extern	UINT	GetTalkDeviceCode(void);

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static BOOL CompareDevCode(char *pInCode,char *pTableCode);

BYTE *AMTBuf = NULL;



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadAMT
**	AUTHOR:		   Jeff Wang
**	DATE:		23 - Mar - 2009
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Anything need to be noticed.
*/
VOID 
LoadAMT()
{
	FILE * fd			= NULL;
	DWORD dwATMVersion = 0;
	DWORD ATMLen	= 0;

	if (AMTBuf)
	{
		free(AMTBuf);
		AMTBuf = NULL;
	}

	if ((fd = fopen(AMTFILE, "r+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fread(&dwATMVersion, 4, (size_t)1, fd);
		
		fseek(fd, 4, SEEK_SET);
		fread(&ATMLen, 4, (size_t)1, fd);		
		
		if(dwATMVersion < ATM_VERSION_4 || ATMLen > 0x200000)
		{
			printf("ATM version or Len old\n");		
			fclose(fd);			
			return;
		}
		
		AMTBuf = (UCHAR*)malloc(ATMLen + 8);
		fseek(fd, 0, SEEK_SET);		
		fread(AMTBuf, ATMLen + 8, (size_t)1, fd);

#ifdef DEBUG_AMT
		printf("Load AMT success\n");
#endif

		fclose(fd);
	}
	else
	{
		printf("File open error\n");
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	WriteData2AMT
**	AUTHOR:		   Jeff Wang
**	DATE:		25 - Dec - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Anything need to be noticed.
*/
void 
WriteData2AMT(UCHAR *pBuf, INT nBufLen)
{
	FILE * fd			= NULL;

	if ((fd = fopen(AMTFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		
		fwrite(pBuf, nBufLen, (size_t)1, fd);
		
#ifdef DEBUG_AMT
		printf("write AMT success\n");
#endif

		fclose(fd);
	}
	else
	{
		printf("File open error\n");
	}

	/***************Copy the data to AMT buffer***********/

	LoadAMT();
	GetTalkDeviceCode();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:
**	AUTHOR:		   Jeff Wang
**	DATE:		25 - Dec - 2008
**
**	DESCRIPTION:	
** 
**
**	ARGUMENTS:		
**				EthResolvInfo*	 pResolvInfo
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Find information from AMT
*/

void 
FdFromAMTResolv(EthResolvInfo *pResolvInfo)
{  
	DWORD dwATMVersion  = 0;
	UCHAR  ATMHeadItem   = 0;
	DWORD  ATMLen	     = 0;

	DWORD dwDevIP		 = 0;
	
	AMT_Type800 at1_Content;
	AMT_Type900 at2_Content;
	AMT_TypeMC at3_Content;
	DWORD	dwNexAddr    = 0;
	DWORD	dwItemOffset   = 0;
	UCHAR	nCountIP	=	0; 
	int j,i,k = 0;

	UCHAR nType = 0;
	WORD SetTypeCount = 0;	
	
	memset(&at2_Content, 0, sizeof(AMT_Type900));

	if (NULL == AMTBuf) 
	{
		printf("AMT empty\n");
		return;
	}

	memcpy(&dwATMVersion, AMTBuf, 4);
	memcpy(&ATMLen, AMTBuf+4, 4);

#ifdef DEBUG_AMT
	printf("ATMLen = %d,dwATMVersion=%x\n", ATMLen, dwATMVersion);
#endif

	if(dwATMVersion < ATM_VERSION_4 || ATMLen > 0x200000)
	{
		printf("ATM version or Len old\n");		
		return;
	}
	
	memcpy(&ATMHeadItem, AMTBuf+8, 1);

#ifdef DEBUG_AMT
	printf("****ATMHeadItem = %d****\n", ATMHeadItem);
	printf("pResolvInfo->nQueryMethod = %d\n", pResolvInfo->nQueryMethod);
#endif

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
	
		if (nType == ATM_TYPE_MC) 
		{
			for(j=0;j<SetTypeCount;j++)
			{
				memcpy((char *)(&at3_Content), AMTBuf + 8 + dwItemOffset + j * ATM_TYPE1_V4_MCLEN, ATM_TYPE1_V4_MCLEN);				
#ifdef DEBUG_AMT
				printf("*************************AMT Data\n");
				
				printf("code = %s\n", at3_Content.DevCode);
				printf("IP Address :");
				for (k = 0; k < 4; k++)
				{
					printf("%02X ", at3_Content.DevIP[k]);
				}
				printf("\n");
#endif
				if (HO_QUERY_METHOD_CODE == pResolvInfo->nQueryMethod)
				{
					if(CompareDevCode((char*)(pResolvInfo->szDevCode),(char *)at3_Content.DevCode))
					{
						memcpy(&pResolvInfo->nIP, (unsigned char *)(at3_Content.DevIP), sizeof(DWORD));
						memcpy((char*)pResolvInfo->szDevCode, (char *)at3_Content.DevCode, MAX_LEN_DEV_CODE + 1);
						pResolvInfo->nType = nType;
						return;
					}
				}
				else if (HO_QUERY_METHOD_IP == pResolvInfo->nQueryMethod)
				{
					memcpy(&dwDevIP, (char *)at3_Content.DevIP, sizeof(DWORD));
					
					if (dwDevIP == pResolvInfo->nIP) 
					{
						strcpy(pResolvInfo->szDevCode, (char*)at3_Content.DevCode);
						pResolvInfo->nType = nType;
						return;
					}
				}
			}
			
		}
		else if(nType == ATM_TYPE_EHV || nType == ATM_TYPE_EGM)//enter here 
		{
			memcpy((char *)(&at2_Content), AMTBuf+dwItemOffset +8, 28);
			nCountIP = at2_Content.IPCOUNT;
			memcpy((char *)(&at2_Content) + 28, AMTBuf+dwItemOffset +8 + 28, nCountIP * 4);
			memcpy((char *)(&at2_Content) + 108, AMTBuf+dwItemOffset +8 + 28 + nCountIP * 4, 24);
			//printf("********************* at2_Content.Level: %d\n", at2_Content.Level);

			for( j = 0 ; j < SetTypeCount ; j++)
			{
				if (HO_QUERY_METHOD_CODE == pResolvInfo->nQueryMethod)
				{
					if(CompareDevCode((char*)(pResolvInfo->szDevCode),(char *)at2_Content.DevCode))
					{
						memcpy(&pResolvInfo->nIP, (unsigned char *)(at2_Content.pDevIP), sizeof(DWORD));
						memcpy((char*)pResolvInfo->szDevCode, (char *)at2_Content.DevCode, MAX_LEN_DEV_CODE + 1);
						pResolvInfo->nType = nType;
						pResolvInfo->Level = at2_Content.Level;
						return;
					}
				}
				else if (HO_QUERY_METHOD_IP == pResolvInfo->nQueryMethod)//enter here 
				{
					memcpy(&dwDevIP, (char *)at2_Content.pDevIP, sizeof(DWORD));
					if (dwDevIP == pResolvInfo->nIP) 
					{
						strcpy(pResolvInfo->szDevCode, (char*)at2_Content.DevCode);
						pResolvInfo->nType = nType;
						pResolvInfo->Level = at2_Content.Level;
						return;
					}
					else if (at2_Content.IPCOUNT > 1 && nType == ATM_TYPE_EHV)
					{
						for(k=1;k<at2_Content.IPCOUNT;k++)
						{
							memcpy(&dwDevIP, (unsigned char*)(at2_Content.pDevIP) + 4 * k ,sizeof(DWORD));
							if (dwDevIP == pResolvInfo->nIP) 
							{
								strcpy(pResolvInfo->szDevCode, (char*)at2_Content.DevCode);
								pResolvInfo->nType = nType;
								pResolvInfo->Level = at2_Content.Level;
								return;
							}
						}
					}
				}
				
				if ((j + 1) < SetTypeCount)
				{
					memcpy(&dwNexAddr,at2_Content.NexAddress,4);

					memcpy((char *)(&at2_Content), AMTBuf+dwNexAddr +8, 28);
					nCountIP = at2_Content.IPCOUNT;
					memcpy((char *)(&at2_Content) + 28, AMTBuf+dwNexAddr +8 + 28, nCountIP * 4);
					memcpy((char *)(&at2_Content) + 108, AMTBuf+dwNexAddr +8 + 28 + nCountIP * 4, 24);
				}
			}
		} 
		else if(nType == ATM_TYPE_HV)
		{

			for(j=0;j<SetTypeCount;j++)
			{
				memcpy((char *)(&at1_Content), AMTBuf + 8 + dwItemOffset + j * ATM_TYPE1_V4_LEN, ATM_TYPE1_V4_LEN);
#ifdef DEBUG_AMT
				printf("*************************AMT Data\n");
				
				printf("code = %s\n", at1_Content.DevCode);
				printf("IP Address :");
				for (k = 0; k < 4; k++)
				{
					printf("%02X ", at1_Content.DevIP[k]);
				}
				printf("\n");
#endif
				
				if (HO_QUERY_METHOD_CODE == pResolvInfo->nQueryMethod)
				{
					if(CompareDevCode((char*)(pResolvInfo->szDevCode),(char *)at1_Content.DevCode))
					{
						memcpy(&pResolvInfo->nIP, (unsigned char *)(at1_Content.DevIP), sizeof(DWORD));
						memcpy((char*)pResolvInfo->szDevCode, (char *)at1_Content.DevCode, MAX_LEN_DEV_CODE + 1);
						pResolvInfo->nType = nType;
						return;
					}
				}
				else if (HO_QUERY_METHOD_IP == pResolvInfo->nQueryMethod)
				{
					memcpy(&dwDevIP, (char *)at1_Content.DevIP, sizeof(DWORD));

					if (dwDevIP == pResolvInfo->nIP) 
					{
						strcpy(pResolvInfo->szDevCode, (char*)at1_Content.DevCode);
						pResolvInfo->nType = nType;
						return;
					}
				}
			}
		} 
	}
}   

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:
**	AUTHOR:		 
**	DATE:		 
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
CompareDevCode(char *pInCode,char *pTableCode)
{
	int TableCodeLen, InCodeLen = 0;
	
	char pInCodeNew[20] = {0};
	
#ifdef COMPAREDEVCODE	
	printf("pTableCode = %s\n", pTableCode);
	printf("pInCode = %s\n", pInCode);
#endif

	TableCodeLen = strlen(pTableCode);
	InCodeLen	 = strlen(pInCode);

	
	if (0 == strcmp(pTableCode, pInCode)) 
	{
		return TRUE;	
	}
	

	if (InCodeLen < g_SysConfig.VldCodeDigits) 
	{
		if (0 == strcmp(pTableCode, pInCode)) 
		{
			return TRUE;	
		}
		else
		{
			//add 0 if inputting code is less than VldCodeDigit 1
			if (1 == (g_SysConfig.VldCodeDigits - InCodeLen)) 
			{
				pInCodeNew[0] = 0x30;
				memcpy(&pInCodeNew[1],pInCode,InCodeLen);
				InCodeLen ++;				
#ifdef COMPAREDEVCODE	
				printf("pInCodeNew = %s\n", pInCodeNew);
				printf("&pTableCode[InCodeLen-g_SysConfig.VldCodeDigits] = %s\n", &pTableCode[TableCodeLen-g_SysConfig.VldCodeDigits]);
#endif
				if (0 == strcmp(pInCodeNew, &pTableCode[TableCodeLen-g_SysConfig.VldCodeDigits])) 
				{
					strcpy(pInCode,pTableCode);
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
		}
	}
	else
	{
#ifdef COMPAREDEVCODE	
		printf("pInCode = %s\n", pInCode);
		printf("&pTableCode[InCodeLen-g_SysConfig.VldCodeDigits] = %s\n", &pTableCode[TableCodeLen-g_SysConfig.VldCodeDigits]);
#endif
		if (0 == strcmp(pInCode, &pTableCode[TableCodeLen-g_SysConfig.VldCodeDigits])) 
		{
			strcpy(pInCode,pTableCode);
			return TRUE;	
		}
		else
		{
			return FALSE;
		}
	}
	

/*
	if (InCodeLen < g_SysConfig.VldCodeDigits) 
	{
		if (0 == strcmp(pTableCode, pInCode)) 
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
#ifdef COMPAREDEVCODE	
		printf("pInCode = %s\n", pInCode);
		printf("&pTableCode[InCodeLen-g_SysConfig.VldCodeDigits] = %s\n", &pTableCode[TableCodeLen-g_SysConfig.VldCodeDigits]);
#endif
		if (0 == strcmp(pInCode, &pTableCode[TableCodeLen-g_SysConfig.VldCodeDigits])) 
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



