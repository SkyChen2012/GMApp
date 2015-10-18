/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	EGMLiftControl.c
**
**	AUTHOR:		Harry	Qian
**
**	DATE:		22 - Jul - 2010
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				Lift control module 
**					
**				
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

/************** USER INCLUDE FILES ***************************************************/

#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"
#include "ModuleTalk.h"
#include "BacpNetCtrl.h"
#include "Multimedia.h"
#include "TalkEventWnd.h"
#include "TalkLogReport.h"
#include "MenuParaProc.h"
#include "AMT.h"
#include "PioApi.h"
#include "IOControl.h"
#include "RS485.h"
#include "BacpSerial.h"
#include "BacpSerialApp.h"
#include "EGMLiftControl.h"
#include "EGMLCAgent.h"
#include "AccessCommon.h"
#include "InCarCardProc.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/
/*typedef struct EGMLCMdstr
{
	DWORD dwLCAIP;

}EGMLCMDINFO;*/

/************** LOCAL DECLARATIONS ***************************************************/

//static EGMLCMDINFO	g_LCMan;

static unsigned char g_CardGround	= 0;
static unsigned char g_CardHighLevelsInfo[4];
static unsigned char g_CardLowLevelsInfo[4];

static unsigned char g_UnlockHighLevelsCount	= 0;
static unsigned char g_UnlockLowLevelsCount	= 0;



static void	SendEGMLCQuery2LCA(MXMSG * pmsg);
static void SendEGMLCInCarUnlock2LCA(MXMSG * pmsg);
static void	LiftCtrlDOPro(MXMSG * pmsg);

/*************************************************************************************/

void
EGMLCMdInit(void)
{
	DpcAddMd(MXMDID_LC, NULL);

	InCarCardInit();
}

void
EGMLCMdExit(void)
{
	DpcRmMd(MXMDID_LC);
}


void
EGMLCMdProcess(void)
{
	MXMSG	msgRecev;
	memset(&msgRecev, 0, sizeof(msgRecev));
	msgRecev.dwDestMd	= MXMDID_LC;
	msgRecev.pParam		= NULL;

	InCarCardProc();

	if (MxGetMsg(&msgRecev))
	{
		switch(msgRecev.dwMsg) 
		{
		case MXMSG_GM_CALL_LIFT:	// From gm , when unlock the door.
			printf("%s get the MXMSG_GM_CALL_LIFT, g_SysConfig.LCEnable = %d.\n", __FUNCTION__, g_SysConfig.LCEnable);
			if (g_SysConfig.LCEnable)//	 LiftControl ON or OFF
			{
				printf("g_SysConfig.LCMode = %d.\n", g_SysConfig.LCMode);
				if (g_SysConfig.LCMode == LC_DO_MODE)
				{
                    LiftCtrlDOPro(&msgRecev);
				}
				else
				{
					SendEGMLCQuery2LCA(&msgRecev);
				}
			}
			break;
		case FC_LF_UNLOCK:
		{
			printf("%s get the FC_LF_UNLOCK, g_SysConfig.LCEnable = %d.\n", __FUNCTION__, g_SysConfig.LCEnable);

			if (g_SysConfig.LCEnable)//	 LiftControl ON or OFF
			{
				printf("the lcmode=%d.\n", g_SysConfig.LCMode);

				SendEGMLCInCarUnlock2LCA(&msgRecev);
			}

			break;
		}
		case FC_ACK_LF_A_B:
			printf("%s get the FC_ACK_LF_A_B, g_SysConfig.LCEnable = %d.\n", __FUNCTION__, g_SysConfig.LCEnable);
			break;
		case FC_ACK_LF_UNLOCK:
			printf("%s get the FC_ACK_LF_UNLOCK, g_SysConfig.LCEnable = %d.\n", __FUNCTION__, g_SysConfig.LCEnable);

			SendCardUnlockACK2CR(*(msgRecev.pParam));
			break;
		default:
			break;
		}
		DoClearResource(&msgRecev);
	}
}

static void 
SendDO2IOCtrl(DWORD dwComm)
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LC;
	msgSend.dwDestMd	= MXMDID_IOCTRL;

	msgSend.dwMsg		= dwComm;
	msgSend.dwParam		= 0;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
}



static void
LCDOModeProc(int direction)
{	
	if (LIFTCONTROL_UP == direction)
	{
		if (FUN_DO_LIFTCTRL_UP == g_SysConfig.DO1)
		{
			SendDO2IOCtrl(COMM_UNLOCK_DO1);
		}
		else if (FUN_DO_LIFTCTRL_UP == g_SysConfig.DO2)
		{
			SendDO2IOCtrl(COMM_UNLOCK_DO2);
		}
	}
	else if (LIFTCONTROL_DOWN == direction)
	{
		if (FUN_DO_LIFTCTRL_DOWN == g_SysConfig.DO1)
		{
			SendDO2IOCtrl(COMM_UNLOCK_DO1);
		}
		else if (FUN_DO_LIFTCTRL_DOWN == g_SysConfig.DO2)
		{
			SendDO2IOCtrl(COMM_UNLOCK_DO2);
		}
	}	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadCardLCInfo
**	AUTHOR:		    Michael Ma
**	DATE:		    5 - Sep - 2012
**
**	DESCRIPTION:
**				    
**
**	ARGUMENTS:		TYPE        ARGNAME     DESCRIPTION
**	                BYTE        CSN
**	RETURNED VALUE:	
**                  BOOL
**	NOTES:
**	                get information of corresponding card in ALT file that every card has one 
**                  corresponding resident with a specified level number.
*/
static 
BOOL 
LoadCardLCInfo(BYTE CSN[CSN_LEN])
{
	FILE * fd	= NULL;
	BOOL bLCFileSearchResult = FALSE;
	unsigned long nOneCard = 0;
	
	if ((fd = fopen(ALTFILE, "r+")) != NULL)
	{
        int nCardSize = 0;
        LFCard sOneCard;
        
        fseek(fd,LC_INFO_VERSION_OFFSET,SEEK_SET);
        fread(&nCardSize,sizeof(int),(size_t)1,fd);
        for(nOneCard = 0 ; nOneCard < nCardSize ; nOneCard++)
        {
            memset(&sOneCard,0,sizeof(LFCard));
            fseek(fd,LC_INFO_VERSION_OFFSET+LC_INFO_COUNT_OFFSET+LC_INFO_CARD_LENGTH_OFFSET+nOneCard*sizeof(LFCard),SEEK_SET);
            fread(&sOneCard,sizeof(LFCard),(size_t)1,fd);
            if(CSNCompare(CSN,sOneCard.CSN))
            {
                g_CardGround = sOneCard.byDestFloor;
                bLCFileSearchResult = TRUE;
                break;
            }
        }  

#if 0 //original codes and local variables disabled by [MichaelMa]
		fseek(fd, 0, SEEK_SET);
		fread(&g_LCFileListCount, 4, (size_t)1, fd);

		for (i = 0; i < g_LCFileListCount; i++)
		{
			lFileCursor = 4 + i * sizeof(ALCTFileListRecord);
			fseek(fd, lFileCursor, SEEK_SET);
			fread(&LCFileListRecord, sizeof(ALCTFileListRecord), (size_t)1, fd);

			if (CSNCompare(CSN, LCFileListRecord.CSN))
			{
				bLCFileSearchResult = TRUE;
				g_CardGround = LCFileListRecord.GroundNum;
				memcpy(g_CardLowLevelsInfo, LCFileListRecord.LowLevels, 4);
				memcpy(g_CardHighLevelsInfo, LCFileListRecord.HighLevels, 4);
				
				g_UnlockLowLevelsCount = 0;
				for (j = 0; j < 4; j++)
				{
					while(LCFileListRecord.LowLevels[j])
					{
						g_UnlockLowLevelsCount++;
						LCFileListRecord.LowLevels[j] &= LCFileListRecord.LowLevels[j] - 1;
					}
				}
				g_UnlockHighLevelsCount = 0;
				for (j = 0; j < 4; j++)
				{
					while(LCFileListRecord.HighLevels[j])
					{
						g_UnlockHighLevelsCount++;
						LCFileListRecord.HighLevels[j] &= LCFileListRecord.HighLevels[j] - 1;
					}
				}
				break;
			}
		}
#endif 
        memset(&sOneCard,0,sizeof(LFCard));
		fclose(fd);
	}
    else
    {
        printf("ALT file open failed,it may not exit!\n");
    }

	return bLCFileSearchResult;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LiftCtrlDOPro
**	AUTHOR:		    Michael Ma
**	DATE:		    5 - Sep - 2012
**
**	DESCRIPTION:
**				
**
**	ARGUMENTS:		
**	                MXMSG * pmsg
**	RETURNED VALUE:	
**                  NULL
**	NOTES:
**	                Handling lift controling  when the mode is IO. 
*/
static 
void
LiftCtrlDOPro(MXMSG * pmsg)
{
	EthResolvInfo 	ResolvInfo;
	unsigned char GetGroundLevelMode;
	DWORD IP;
	CHAR RdNum[RD_CODE_LEN];
	BYTE CSN[CSN_LEN];
	SHORT DestLevel;
	SHORT SrcLevel;
	int direction;

	//if (g_SysConfig.LC_ComMode == -1)
	//{
	//	SendDO2IOCtrl(COMM_UNLOCK_DO1);
	//}
	//else
	//{
		if (NULL != pmsg->pParam)
		{
			memcpy(&GetGroundLevelMode, pmsg->pParam, sizeof(GetGroundLevelMode));
			switch(GetGroundLevelMode)
			{
				case HO_QUERY_METHOD_IP:
					memcpy(&IP, pmsg->pParam + sizeof(GetGroundLevelMode), sizeof(IP));
					break;
				case HO_QUERY_METHOD_CODE:
					memcpy(RdNum, pmsg->pParam + sizeof(GetGroundLevelMode), RD_CODE_LEN);
					break;
				case HO_QUERY_METHOD_CSN:
					memcpy(CSN, pmsg->pParam + 1 + RD_CODE_LEN/*sizeof(GetGrboundLevelMode)*/, CSN_LEN);
					break;
                case HO_QUERY_METHOD_CALLCODE :
                    memcpy(RdNum, pmsg->pParam + 1 + DEVICE_CODE_LEN, RD_CODE_LEN);
                    break;
				default:
					break;
			}
		}
		else
		{
			return;
		}

        //Get self level number
		memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
		ResolvInfo.Level = 0xffff;
		ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;
		ResolvInfo.nIP = ChangeIPFormat(GetSelfIP());
		FdFromAMTResolv(&ResolvInfo);
		if (0xffff == ResolvInfo.Level)
		{
#ifdef LC_DEBUG
			printf("*************** No Self IP in AMT! Check whether self is in AMT.\n");
#endif
			return;
		}
		else
		{
			SrcLevel = ResolvInfo.Level;
		}

		// Get target level number
		if (HO_QUERY_METHOD_CSN == GetGroundLevelMode)
		{
			if (!LoadCardLCInfo(CSN))
			{
				return;
			}
			else 
            {
                DestLevel = (signed char)g_CardGround;
                printf("DestLevel : %d\n",DestLevel);
			}
        }
        else if(HO_QUERY_METHOD_CALLCODE == GetGroundLevelMode) 
        {
            memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
			ResolvInfo.Level = 0xffff;
            ResolvInfo.nQueryMethod = HO_QUERY_METHOD_CODE;
            memcpy(ResolvInfo.szDevCode,RdNum,DEVICE_CODE_LEN);
            FdFromAMTResolv(&ResolvInfo);
            if (0xffff == ResolvInfo.Level)
            {
                printf("***** can't find this resident in AMT file! *****\n");
            }
            else
            {
                DestLevel = ResolvInfo.Level;
            }
        }
		else
		{
			memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
			ResolvInfo.Level = 0xffff;
			if (HO_QUERY_METHOD_IP == GetGroundLevelMode)
			{
				ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;
				ResolvInfo.nIP = ChangeIPFormat(IP);
			}
			else if (HO_QUERY_METHOD_CODE == GetGroundLevelMode)
			{
				ResolvInfo.nQueryMethod = HO_QUERY_METHOD_CODE;
				memcpy(ResolvInfo.szDevCode, RdNum, RD_CODE_LEN);
			}
			else 
			{
				return;
			}
			FdFromAMTResolv(&ResolvInfo);
			
			if (0xffff == ResolvInfo.Level) return;
			else
			{
				DestLevel = ResolvInfo.Level;
			}
		}
        
        if (SrcLevel > DestLevel) 
        {
            direction = LIFTCONTROL_DOWN;
		}
        else if(SrcLevel < DestLevel)
        {
            direction = LIFTCONTROL_UP;
        }
        else 
        {
            direction = LIFTCONTROL_NONE;
        }
        printf("direction : %d\n",direction);
		LCDOModeProc(direction);	
	//}
}


static void
SendEGMLCQuery2LCA(MXMSG * pmsg)
{
	MXMSG	msgSend ;
	unsigned short nDataLen = 4;// 2(src) + 2(dest)
//	DWORD dwIP = 0;
//	DWORD	dwTemp = 0;
	unsigned char GetGroundLevelMode;
	memset(&msgSend, 0, sizeof(MXMSG));

	msgSend.dwSrcMd		= MXMDID_LC;
	msgSend.dwDestMd	= MXMDID_ETH;
	strcpy(msgSend.szSrcDev, pmsg->szSrcDev);
	strcpy(msgSend.szDestDev, g_TalkInfo.szTalkDevCode);
    
	msgSend.dwMsg		= FC_LF_A_B;
	msgSend.dwParam		= g_SysConfig.LCAIP;
#ifdef LC_DEBUG
	printf("the LCAIP = %lx.\n", g_SysConfig.LCAIP);
#endif
		if (NULL != pmsg->pParam)
		{
			memcpy(&GetGroundLevelMode, pmsg->pParam, sizeof(GetGroundLevelMode));
#ifdef LC_DEBUG
			printf("the GetGroundLevelMode = %d.\n", GetGroundLevelMode);
#endif			
			switch(GetGroundLevelMode)
			{
				case HO_QUERY_METHOD_RDCODE:
					nDataLen = 1 + 1 + RD_CODE_LEN + 1 + RD_CODE_LEN;
					msgSend.pParam		= malloc(nDataLen + 2);
					
					memcpy(msgSend.pParam, &nDataLen, 2);
					msgSend.pParam[2] = HO_QUERY_METHOD_RDCODE;
					msgSend.pParam[3] = LEVEL_CODE_FORMAT;
					memcpy(msgSend.pParam + 4, pmsg->pParam + 1, RD_CODE_LEN);
					msgSend.pParam[4 + RD_CODE_LEN] = LEVEL_CODE_FORMAT;
					memcpy(msgSend.pParam + 4 + RD_CODE_LEN + 1, pmsg->pParam + 1 + RD_CODE_LEN, RD_CODE_LEN);
					
					break;
				case HO_QUERY_METHOD_CALLCODE:
					nDataLen = 1 + 1 + RD_CODE_LEN + 1 + RD_CODE_LEN;
					msgSend.pParam = malloc(nDataLen + 2);
					
					memcpy(msgSend.pParam, &nDataLen, 2);
					msgSend.pParam[2] = HO_QUERY_METHOD_CALLCODE;
					msgSend.pParam[3] = LEVEL_CODE_FORMAT;
					memcpy(msgSend.pParam + 4, pmsg->pParam + 1, RD_CODE_LEN);
					msgSend.pParam[4 + RD_CODE_LEN] = LEVEL_CODE_FORMAT;
					memcpy(msgSend.pParam + 4 + RD_CODE_LEN + 1, pmsg->pParam + 1 + RD_CODE_LEN, RD_CODE_LEN);
					
					break;
				case HO_QUERY_METHOD_CSN:
					nDataLen = 1 + 1 + RD_CODE_LEN + 1 + CSN_LEN;
					msgSend.pParam		= malloc(nDataLen + 2);
					memcpy(msgSend.pParam, &nDataLen, 2);
					
					msgSend.pParam[2] = HO_QUERY_METHOD_CSN;
					msgSend.pParam[3] = LEVEL_CODE_FORMAT;
					memcpy(msgSend.pParam + 4, pmsg->pParam + 1, RD_CODE_LEN);	                            //RD
					msgSend.pParam[4 + RD_CODE_LEN] = LEVEL_CSN_FORMAT;
					memcpy(msgSend.pParam + 4 + RD_CODE_LEN + 1, pmsg->pParam + 1 + RD_CODE_LEN, CSN_LEN);	//CSN	
					
					break;
				default:
					break;
			}
		}
		else
		{
			return;
		}

	MxPutMsg(&msgSend);	
}

static void
SendEGMLCInCarUnlock2LCA(MXMSG * pmsg)
{
	MXMSG	msgSend ;
	unsigned short nDataLen = 0;
//	int offset = 0;

	memset(&msgSend, 0, sizeof(MXMSG));
	
	msgSend.dwSrcMd		= MXMDID_LC;
	msgSend.dwDestMd	= MXMDID_ETH;
	
	msgSend.dwMsg		= pmsg->dwMsg;
	msgSend.dwParam		= g_SysConfig.LCAIP;
#ifdef LC_DEBUG
	printf("the LCAIP = %lx.\n", g_SysConfig.LCAIP);
#endif
	if (NULL != pmsg->pParam)
	{
		nDataLen = *(pmsg->pParam);
		msgSend.pParam = malloc(sizeof(unsigned short) + nDataLen);
		memcpy(msgSend.pParam, &nDataLen, sizeof(unsigned short));
		memcpy(msgSend.pParam + sizeof(unsigned short), pmsg->pParam + 1, nDataLen);
	}
	else
	{
		return;
	}
	
	MxPutMsg(&msgSend);	
}