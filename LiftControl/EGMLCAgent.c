/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	EGMLCAgent.c
**
**	AUTHOR:		Harry	Qian
**
**	DATE:		22 - Jul - 2010
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				
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
#include "LiftCtrlConversion.h"
#include "LCAPro_mox_V1.h"
#include "LCAPro_mitsubishi.h"
#include "AccessCommon.h"
#include "LCAPro_mox_V2.h"
#include "LiftCtrlByDOModule.h"
/************** DEFINES **************************************************************/
//#define LCA_DEBUG


#define MAX_CMD_NUM		100

/************** TYPEDEFS *************************************************************/

static BOOL		bLCAExist;
static LCCMD	g_CMDBuff[MAX_CMD_NUM];
static int		g_CmdIndex;
static LCINFO	g_LCInfo;
static CODELIFT	g_CodeLift;
static DWORD	g_dwHeartBeatTick;

LCAPROTOCOL	g_LCAProtocol;

static VOID LoadCodeLift(VOID);

static void Mox_CallLiftA2B(MXMSG* pmsg);
static void Mox_CallLiftStop(MXMSG* pmsg);
static void Mox_CallLiftUnlock(MXMSG* pmsg);

static void Hitachi_CallLiftA2B(MXMSG* pmsg);
static void Hitachi_CallLiftStop(MXMSG* pmsg);
//static void Hitachi_CallLiftStopUp(MXMSG* pmsg);
//static void Hitachi_CallLiftStopDown(MXMSG* pmsg);
static void Hitachi_CallLiftUnlock(MXMSG* pmsg);

static BOOL CmdBufferEmpty(VOID);
//static void AddCmd2Buffer(int nCmdId, DWORD dwIP, unsigned short usCmd);
static LCCMD * GetBlankCmdBuff(void);
static LCCMD * GetCmdFromBuff(void);
static void EGMLCACmdAckPro(MXMSG * pmsg);
static LCCMD * CheckCmdExist(int nCmdId);
static void LCASendErrorACK2LC(MXMSG * pmsg);
static void AddMsg2Buffer(MXMSG* pMsg);

static void Mox_LFStateRsq(MXMSG* pmsg);
static void GetLFStats_Mox(MXMSG* pmsg);

static void Mox_OnlyCallStop(MXMSG* pmsg);
static void Mox_OnlyUnlockPro(MXMSG* pmsg);
static void LCASendLiftRunningInfo(LCCMD * pCmd, MXMSG * pmsg);
static LCCMD *RmCmdFromBuffer(LCCMD * pCmd);
static void HCLF_A_BPro(MXMSG *pmsg);
static void HCLFStopPro(MXMSG *pmsg);
static void HCLF_UnlockPro(MXMSG *pmsg);
static void HCLFStateRsq(MXMSG *pmsg);
static void HCGetLFStats(MXMSG *pmsg);
static void HCLFCmdAckPro(MXMSG *pmsg);
static void HCLFHeartBeat(MXMSG *pmsg);

static void Mit_CallLiftA2B(MXMSG* pmsg);
static void Mit_CallLiftUnlock(MXMSG* pmsg);
static void Mit_LFStateRsq(MXMSG* pmsg);
static void GetLFStats_Mit(MXMSG* pmsg);
static void Mit_LFHeartBeat(MXMSG* pmsg);

static VOID	LCASendStateCheck(VOID);
static VOID	LCAHeartBeat(VOID);
static void DOM_CallLiftUnlock(MXMSG* pmsg);
static void DOM_CallLiftStop(MXMSG* pmsg);
static void DOM_CallLiftA2B(MXMSG* pmsg);

static VOID LCASendErrorACK2Target(DWORD dwMsg, DWORD dwCmdID, unsigned char ucRetCode);
static VOID LCAWaitingAckPro(VOID);
static VOID LCAProtocolPro(VOID);
static BOOL RMCMDByMsg(MXMSG * pmsg);
static void Mit_CallLiftStop(MXMSG* pmsg);
//static LFCard * FindLFCardByCSN(BYTE * szCSN);
static void MOX2_LFHeartBeat(MXMSG* pmsg);
static void Mox2_CallLiftA2B(MXMSG* pmsg);
static void MOX2_CallLiftStop(MXMSG* pmsg);
static void MOX2_CallLiftUnlock(MXMSG* pmsg);
static void MOX2_SendHV2HVMsg(MXMSG* pmsg,char * szSrcUserCode,char * szDestUsetCode);
/************** STRUCTURES ***********************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EGMLCAMdInit()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				intialize the lift control agent module.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
void
EGMLCAMdInit(void)
{
	DpcAddMd(MXMDID_LCA, NULL);
	memset(g_CMDBuff, 0, sizeof(LCCMD) * MAX_CMD_NUM);

	if (g_SysConfig.bLCAgent)
	{
		LCAConversionMdInit();
		LCAMOXProtoInit();
		LCAMOX2ProtoInit();
		LCAMitProtoInit();
		bLCAExist = TRUE;

		g_LCInfo.versionnum = 0;
		g_LCInfo.nCount = 0;
		g_LCInfo.nCardInfoLen = 0;
		g_LCInfo.pLFCard = (LFCard*)malloc(MAX_LF_CARD_CNT * sizeof(LFCard));
		memset(g_LCInfo.pLFCard, 0, MAX_LF_CARD_CNT * sizeof(LFCard));

		g_CodeLift.versionnum = 0;
		g_CodeLift.nCount = 0;
		g_CodeLift.nCodeInfoLen = 0;
		g_CodeLift.pCodeInfo = (CodeInfo*)malloc(MAX_LF_RD_CNT * sizeof(CodeInfo));
		memset(g_CodeLift.pCodeInfo, 0, MAX_LF_RD_CNT * sizeof(CodeInfo));

		LoadLCInfo();
		LoadCodeLift();

		g_dwHeartBeatTick = GetTickCount();
		g_LCAProtocol.nStatus = LCA_STATUS_IDLE;
	}
	else
	{
		bLCAExist = FALSE;
	}
	
	printf("%s: the LC agent exist = %d. LCAIP=%lx\n", __FUNCTION__, g_SysConfig.bLCAgent, g_SysConfig.LCAIP);	

}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EGMLCAMdExit()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				exit the lift control agent module.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
void
EGMLCAMdExit(void)
{
	DpcRmMd(MXMDID_LCA);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EGMLCAFuncProc()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				process the command for lift control request.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
void
EGMLCAFuncProc(void)
{
	MXMSG	msgRecev;
	LCCMD*	cmdCur;
	MXMSG	msgCmd;

	memset(&msgRecev, 0, sizeof(msgRecev));
	msgRecev.dwDestMd	= MXMDID_LCA;
	msgRecev.pParam		= NULL;

	if (!g_SysConfig.bLCAgent)
	{
		return;
	}

	if (LC_MITSUBISHI_MODE == g_SysConfig.LC_ComMode)
	{
		if (CmdBufferEmpty() && g_LCAProtocol.nStatus == LCA_STATUS_IDLE)
		{
			LCASendStateCheck();
		}
	}	

	LCAHeartBeat();

	LCAWaitingAckPro();

	while (MxGetMsg(&msgRecev))
	{
#ifdef LCA_DEBUG
		printf("%s lcmode=%d.get message = %x,g_LCAProtocol.nStatus=%d...\n", __FUNCTION__,g_SysConfig.LC_ComMode, msgRecev.dwMsg,g_LCAProtocol.nStatus);
#endif	
		switch(msgRecev.dwMsg)
		{
			
			case FC_LF_STOP_UP:
			case FC_LF_STOP_DOWN:
			case FC_LF_STOP:
			case FC_LF_A_B:
			case FC_LF_UNLOCK:
			case FC_LF_STATE:
			case FC_LF_STATE_NOTIFY:
				AddMsg2Buffer(&msgRecev);
				break;
			case FC_LF_HEARTBEAT:
				HCLFHeartBeat(&msgRecev);
				break;
			case FC_ACK_LF_STATE:
				HCGetLFStats(&msgRecev);
				break;
			case FC_ACK_LF_A_B:
				
			case FC_ACK_LF_STOP:
			case FC_ACK_LF_STOP_UP:
			case FC_ACK_LF_STOP_DOWN:
			case FC_ACK_LF_UNLOCK:
			//printf("TX:FC_ACK_LF_UNLOCK\n");
			case FC_ACK_LF_STATE_NOTIFY:
		
				HCLFCmdAckPro(&msgRecev);
				break;
			default:
				break;
		}
	
		DoClearResource(&msgRecev);
	}

	if (LCA_STATUS_IDLE == g_LCAProtocol.nStatus)
	{
		cmdCur = GetCmdFromBuff();

		if (NULL != cmdCur)
		{
			msgCmd.dwMsg	= cmdCur->usCMD;
			// pass cmd ID
			msgCmd.dwParam=cmdCur->dwIP;
			msgCmd.pParam	= (BYTE*)malloc(cmdCur->nDataLen);
			memcpy(msgCmd.pParam, cmdCur->pData, cmdCur->nDataLen);
			
			memcpy(msgCmd.szSrcDev,cmdCur->szSrcDev,MAX_LEN_DEV_CODE + 1);

			switch(msgCmd.dwMsg)
			{
				case FC_LF_A_B:
				 	HCLF_A_BPro(&msgCmd);
				 	break;
				case FC_LF_STOP:
				case FC_LF_STOP_UP:
				case FC_LF_STOP_DOWN:
				 	HCLFStopPro(&msgCmd);
				 	break;
				case FC_LF_UNLOCK:
				 	HCLF_UnlockPro(&msgCmd);
				 	break;
				case FC_LF_STATE:
				 	HCLFStateRsq(&msgCmd);
				 	break;
				case FC_LF_STATE_NOTIFY:
				 	break;
 				default:
 					break;
	 		}

			/*if (LC_HITACHI_MODE == g_SysConfig.LC_ComMode)
			{
				EGMLCACmdAckPro(&msgCmd);
			}*/
			msgCmd.dwParam	= cmdCur->nCmdId;
			RMCMDByMsg(&msgCmd);
			

			DoClearResource(&msgCmd);
		}
	}

	LCAProtocolPro();
}

VOID
LoadLCInfo(VOID)
{
    if(g_SysConfig.bLCAgent)
    {
        FILE * fd = NULL;
	    struct stat pStat;
	
	    if ((fd = fopen(ALT_FILE, "r+")) != NULL)
	    {   
		    stat(ALT_FILE, &pStat);
		    fseek(fd,   0,  SEEK_SET);
		    fread(&g_LCInfo.versionnum,     sizeof(int),                  (size_t)1,  fd);
		    fseek(fd,   LC_INFO_VERSION_OFFSET,  SEEK_SET);
		    fread(&g_LCInfo.nCount,         sizeof(int),                  (size_t)1,  fd);	
		    fseek(fd,   LC_INFO_VERSION_OFFSET+LC_INFO_COUNT_OFFSET,  SEEK_SET);
		    fread(&g_LCInfo.nCardInfoLen,   sizeof(int),                  (size_t)1,  fd);
		    fseek(fd,   LC_INFO_VERSION_OFFSET+LC_INFO_COUNT_OFFSET+LC_INFO_CARD_LENGTH_OFFSET, SEEK_SET);                                      
            fread(g_LCInfo.pLFCard,         pStat.st_size - (LC_INFO_VERSION_OFFSET+LC_INFO_COUNT_OFFSET+LC_INFO_CARD_LENGTH_OFFSET), (size_t)1,  fd);
		    fclose(fd);
	    }
	    else
	    {
	    	printf("ALT file open error\n");
	    	return;
	    }
	
	    if (g_LCInfo.nCount > MAX_LF_CARD_CNT)
	    {
	    	printf("Lift card count error\n");
	    	return;		
	    }
    }
	
}

static
VOID
LoadCodeLift(VOID)
{    
	FILE * fd = NULL;
	struct stat pStat;
	int i = 0;
	
	if ((fd = fopen(CLT_FILE, "r+")) != NULL)
	{
		stat(CLT_FILE, &pStat);
        
		fseek(fd, 0, SEEK_SET);
		fread(&g_CodeLift.versionnum, sizeof(int), (size_t)1, fd);
		fseek(fd, 0, SEEK_CUR);
		fread(&g_CodeLift.nCount, sizeof(int), (size_t)1, fd);
        fseek(fd, 0, SEEK_CUR);
		fread(&g_CodeLift.nCodeInfoLen, sizeof(int), (size_t)1, fd);
        fseek(fd, 0, SEEK_CUR);
		for(i=0;i<g_CodeLift.nCount;i++)
		{
			if(1==g_CodeLift.versionnum)
			{
				fread(&g_CodeLift.pCodeInfo[i], sizeof(CodeInfo), (size_t)1, fd);
				fseek(fd, 0, SEEK_CUR);
			}
			else
			{
				fread(&g_CodeLift.pCodeInfo[i], sizeof(CodeInfo_V0), (size_t)1, fd);
				fseek(fd, 0, SEEK_CUR);
				g_CodeLift.pCodeInfo[i].byMethod=CALL_LIFT_METHOD_DEF;
			}
		}
		//fread(g_CodeLift.pCodeInfo, pStat.st_size - sizeof(int) - sizeof(int) - sizeof(int), (size_t)1, fd);
		
		fclose(fd);
	}
	else
	{
		printf("CLT file open error\n");
		return;
	}
	
	if (g_CodeLift.nCount > MAX_LF_RD_CNT)
	{
		printf("Resident code count error\n");
		return;		
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	HCLF_A_BPro()
**	AUTHOR:			Harry Qian
**	DATE:			28 - Oct - 2010
**
**	DESCRIPTION:	
**				Home control system process the FC_LF_A_B command
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
HCLF_A_BPro(MXMSG *pmsg)
{
	switch(g_SysConfig.LC_ComMode)
	{
		case LC_MOX_MODE_V1:
		{
			switch(g_SysConfig.nLCFuncs)
			{
				case LC_RUN_MODE_CALL:
					Mox_OnlyCallStop(pmsg);
					break;
				case LC_RUN_MODE_CALL_UNLOCK:
					Mox_OnlyCallStop(pmsg);
					Mox_OnlyUnlockPro(pmsg);
					break;
				case LC_RUN_MODE_AUTO:
					Mox_CallLiftA2B(pmsg);
					break;
				default:
					break;
			}
			break;
		}
		case LC_MOX_MODE_V2:
			Mox2_CallLiftA2B(pmsg);
			break;
		case LC_HITACHI_MODE:
			Hitachi_CallLiftA2B(pmsg);
			break;
		case LC_MITSUBISHI_MODE:
		{
			Mit_CallLiftA2B(pmsg);
			break;
		}	
		case LC_BY_DO_MODULE: 
			DOM_CallLiftA2B(pmsg);		
		default:
			break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	HCLFStopPro()
**	AUTHOR:			Harry Qian
**	DATE:			28 - Oct - 2010
**
**	DESCRIPTION:	
**				Home control system process the FC_LF_STOP command
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void 
HCLFStopPro(MXMSG *pmsg)
{
	switch(g_SysConfig.LC_ComMode)
	{
	case LC_MOX_MODE_V1:
		Mox_CallLiftStop(pmsg);
		break;
	case LC_MOX_MODE_V2:
		MOX2_CallLiftStop(pmsg);
		break;
	case LC_HITACHI_MODE:
		Hitachi_CallLiftStop(pmsg);
		break;
	case LC_MITSUBISHI_MODE:
		Mit_CallLiftStop(pmsg);
		break;
	case LC_BY_DO_MODULE: //  “ƒ⁄ª˙∫ÙÃ›
		DOM_CallLiftStop(pmsg);
	default:
		break;
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	HCLF_UnlockPro()
**	AUTHOR:			Harry Qian
**	DATE:			28 - Oct - 2010
**
**	DESCRIPTION:	
**				Home control system process the FC_LF_UNLOCK command
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/

static void
HCLF_UnlockPro(MXMSG *pmsg)
{
	switch(g_SysConfig.LC_ComMode)
	{
	case LC_MOX_MODE_V1:
		Mox_CallLiftUnlock(pmsg);
		break;
	case LC_MOX_MODE_V2:
		MOX2_CallLiftUnlock(pmsg);
		break;
	case LC_HITACHI_MODE:
		Hitachi_CallLiftUnlock(pmsg);
		break;
	case LC_MITSUBISHI_MODE:
		Mit_CallLiftUnlock(pmsg);
		break;
	case LC_BY_DO_MODULE:
		DOM_CallLiftUnlock(pmsg);
	default:
		break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	HCLFStateRsq()
**	AUTHOR:			Harry Qian
**	DATE:			28 - Oct - 2010
**
**	DESCRIPTION:	
**				Home control system process the FC_LF_STATE command
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/

static void
HCLFStateRsq(MXMSG *pmsg)
{
	switch(g_SysConfig.LC_ComMode)
	{
	case LC_MOX_MODE_V1:
		Mox_LFStateRsq(pmsg);
		break;
	case LC_MOX_MODE_V2:
		break;
	case LC_HITACHI_MODE:
		break;

	case LC_MITSUBISHI_MODE:
		Mit_LFStateRsq(pmsg);
		break;
	default:
		break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	HCLFHeartBeat()
**	AUTHOR:			Wayde Zeng
**	DATE:			14 - Jan - 2011
**
**	DESCRIPTION:	
**				Heart Beat
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
void
HCLFHeartBeat(MXMSG *pmsg)
{
	switch(g_SysConfig.LC_ComMode)
	{
		case LC_MOX_MODE_V1:
			break;		
		case LC_MOX_MODE_V2:
			MOX2_LFHeartBeat(pmsg);
			break;
		case LC_HITACHI_MODE:
			break;			
		case LC_MITSUBISHI_MODE:
//			Mit_LFHeartBeat(pmsg);
			break;
		default:
			break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	HCGetLFStats()
**	AUTHOR:			Harry Qian
**	DATE:			28 - Oct - 2010
**
**	DESCRIPTION:	
**				Home control system process the FC_ACK_LF_STATE command
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/

static void
HCGetLFStats(MXMSG *pmsg)
{
	switch(g_SysConfig.LC_ComMode)
	{
	case LC_MOX_MODE_V1:
		GetLFStats_Mox(pmsg);
		break;
	case LC_MOX_MODE_V2:
		break;
	case LC_HITACHI_MODE:
		break;

	case LC_MITSUBISHI_MODE:
		GetLFStats_Mit(pmsg);
		break;

	default:
		break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	HCLFCmdAckPro()
**	AUTHOR:			Harry Qian
**	DATE:			28 - Oct - 2010
**
**	DESCRIPTION:	
**				Home control system process the FC_ACK_LF_STATE command
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/

static void
HCLFCmdAckPro(MXMSG *pmsg)
{
	switch(g_SysConfig.LC_ComMode)
	{
	case LC_MOX_MODE_V1:
		EGMLCACmdAckPro(pmsg);
		break;
	case LC_MOX_MODE_V2:
		EGMLCACmdAckPro(pmsg);
		break;
	case LC_HITACHI_MODE:
		break;

	case LC_MITSUBISHI_MODE:
		EGMLCACmdAckPro(pmsg);
		break;
	case LC_BY_DO_MODULE:
		EGMLCACmdAckPro(pmsg);			
	default:	
		break;
	}
}


static void
Mox_CallLiftA2B(MXMSG* pmsg)
{
	MXMSG	msgSend;
	int		offset = 0;
	DWORD	dwDA = 0;
//	int		i = 0;
//	int		nFindPos = 0;
//	BOOL	bFind = FALSE;
	BYTE	CardCSN[CSN_LEN] = { 0 };
	LFCard * pLFCard=NULL;
	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
	unsigned char ucDataFormat = *(pmsg->pParam + 1);
	memset(&msgSend, 0, sizeof(MXMSG));

	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V1;

	msgSend.dwMsg	= pmsg->dwMsg;
	msgSend.pParam	= malloc(11);// 1 + 3 + 3 +  3: nDataLen + ucdataFormatA + ucdataFormatB + DA 


	if (msgSend.pParam == NULL)
	{
		printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
		return ;
	}
	memset(msgSend.pParam, 0, 11);
	*msgSend.pParam = 10;	
	msgSend.dwParam = pmsg->dwParam;

	switch(ucDataFormat)
	{
	case LEVEL_NUMBER_FORMAT:
		dwDA = GetLiftIDbyLayer(*(pmsg->pParam + 2));
		*(msgSend.pParam + 1) = ucDataFormat;
		*(msgSend.pParam + 2) = *(pmsg->pParam + 2);
		*(msgSend.pParam + 3) = 0xff;
		offset = 3;
		break;
	case LEVEL_CODE_FORMAT:
		dwDA = GetLiftIDbyCode((const char * )pmsg->pParam + 2);
		*(msgSend.pParam + 1) = 0;
		*(msgSend.pParam + 2) = GetLayerbyCode((const char * )pmsg->pParam + 2);
		*(msgSend.pParam + 3) = 0xff;
		
		offset = 2 + RD_CODE_LEN;

		break;
	default:
		break;

	}

	ucDataFormat = *(pmsg->pParam + offset);
	offset += 1;
	switch(ucDataFormat)
	{
	case LEVEL_NUMBER_FORMAT:
		*(msgSend.pParam + 4) = ucDataFormat;
		*(msgSend.pParam + 5) = *(pmsg->pParam + offset);
		*(msgSend.pParam + 6) = 0xff;
		break;
	case LEVEL_CODE_FORMAT:
		*(msgSend.pParam + 4) = 0;
		*(msgSend.pParam + 5) = GetLayerbyCode((const char * )pmsg->pParam + offset);	
		*(msgSend.pParam + 6) = 0xff;
		break;
	case LEVEL_CSN_FORMAT:
		{
			memset(CardCSN, 0, CSN_LEN);
			memcpy(CardCSN, pmsg->pParam + offset, CSN_LEN);
			pLFCard=FindLFCardByCSN(CardCSN);
			if(pLFCard)
			{
				*(msgSend.pParam + 4) = 0;
				*(msgSend.pParam + 5) = pLFCard->byDestFloor;	
				*(msgSend.pParam + 6) = 0xff;
			}
			else
			{
				LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
				if(msgSend.pParam)
				{
					free(msgSend.pParam);
				}
				return;
			}
			break;
		}
	default:
		break;
	}
	memcpy(msgSend.pParam + 7, &dwDA, 4);
	MxPutMsg(&msgSend);
#ifdef MOX_DEBUG
	printf("LCA send the data:\n");
	for (offset = 0; offset < 11; offset++)
	{
		printf("  %x ", *(msgSend.pParam + offset));
	}
	printf("\n");
#endif

}

LFCard* FindLFCardByCSN(BYTE* szCSN)
{
	int i;
	for (i = 0; i < g_LCInfo.nCount ; i++)
	{
		if (CSNCompare(szCSN, g_LCInfo.pLFCard[i].CSN))
		{
			return &g_LCInfo.pLFCard[i];
		}
	}
	return NULL;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Mox_OnlyUnlockPro()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
Mox_OnlyUnlockPro(MXMSG* pmsg)
{
	MXMSG	msgSend;
	int		offset = 0;
	DWORD	dwDA = 0;
	unsigned short nTime = 0;
	int		i = 0;
	int		nFindPos = 0;
	BOOL	bFind = FALSE;
	BYTE	CardCSN[CSN_LEN] = { 0 };

	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
	unsigned char ucDataFormat = *(pmsg->pParam + 1);
	memset(&msgSend, 0, sizeof(MXMSG));

	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V1;

	msgSend.dwMsg	= FC_LF_UNLOCK;
	msgSend.pParam	= malloc(11);// 1 + 3 + 3 +  3: nDataLen + ucdataFormatA + ucdataFormatB + DA 


	if (msgSend.pParam == NULL)
	{
		printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
		return ;
	}
	memset(msgSend.pParam, 0, 11);
	*msgSend.pParam = 10;	
	msgSend.dwParam = pmsg->dwParam;
	*(msgSend.pParam + 1) = 1; //nLevelCnt = 1;
	switch(ucDataFormat)
	{
	case 0:
		offset = 3;
		break;
	case 1:
		offset = 6;
		break;
	case 2:
		offset = 2 + RD_CODE_LEN;

		break;
	default:
		break;
	}

	ucDataFormat = *(pmsg->pParam + offset);
	offset += 1;
	switch(ucDataFormat)
	{
	case 0:
		*(msgSend.pParam + 2) = ucDataFormat;
		*(msgSend.pParam + 3) = *(pmsg->pParam + offset);
		*(msgSend.pParam + 4) = 0xff;
		break;
	case 2:
		*(msgSend.pParam + 2) = 0;
		*(msgSend.pParam + 3) = GetLayerbyCode((const char * )pmsg->pParam + offset);	
		*(msgSend.pParam + 4) = 0xff;
		break;
	case 3:
		{
			memset(CardCSN, 0, CSN_LEN);
			memcpy(CardCSN, pmsg->pParam + offset, CSN_LEN);
			
			for (i = 0; i < g_LCInfo.nCount; i++)
			{
				if (CSNCompare(CardCSN, g_LCInfo.pLFCard[i].CSN))
				{
					bFind = TRUE;
					nFindPos = i;
					break;
				}
			}
			
			if (bFind)
			{
				*(msgSend.pParam + 2) = 0;
				*(msgSend.pParam + 3) = g_LCInfo.pLFCard[nFindPos].byDestFloor;	
				*(msgSend.pParam + 4) = 0xff;
			}
			else
			{
				LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
				if(msgSend.pParam)
				{
					free(msgSend.pParam);
				}
				return;
			}
			
			break;
		}
	default:
		break;
	}

	memcpy(msgSend.pParam + 5, &nTime, 2);
	memcpy(msgSend.pParam + 7, &dwDA, 4);
	MxPutMsg(&msgSend);
#ifdef MOX_DEBUG
	printf("LC only Unlock the level.\nLCA send the data:\n");
	for (offset = 0; offset < 11; offset++)
	{
		printf("  %x ", *(msgSend.pParam + offset));
	}
	printf("\n");
#endif
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Mox_OnlyCallStop()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
Mox_OnlyCallStop(MXMSG* pmsg)
{
	MXMSG	msgSend;
	int		offset = 0;
	DWORD	dwDA = 0;
	int		nLevelA	= 0;
	int		nLevelB = 0;
	int		i = 0;
	int		nFindPos = 0;
	BOOL	bFind = FALSE;
	BYTE	CardCSN[CSN_LEN] = { 0 };

	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
	unsigned char ucDataFormat = *(pmsg->pParam + 1);
	memset(&msgSend, 0, sizeof(MXMSG));

	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V1;

	msgSend.dwMsg	= FC_LF_STOP;

	msgSend.pParam	= malloc(8);// 
	if (msgSend.pParam == NULL)
	{
		printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
		return ;
	}
	*msgSend.pParam = 7;
	msgSend.dwParam	= pmsg->dwParam;	
	
	switch(ucDataFormat)
	{
	case 0:
		dwDA = GetLiftIDbyLayer(*(pmsg->pParam + 2));
		*(msgSend.pParam + 1) = ucDataFormat;
		*(msgSend.pParam + 2) = *(pmsg->pParam + 2);
		*(msgSend.pParam + 3) = 0xff;
		nLevelA = *(pmsg->pParam + 2);

		offset = 3;
		break;
	case 2:
		dwDA = GetLiftIDbyCode((const char * )pmsg->pParam + 2);
		*(msgSend.pParam + 1) = 0;
		*(msgSend.pParam + 2) = GetLayerbyCode((const char * )pmsg->pParam + 2);
		*(msgSend.pParam + 3) = 0xff;
		nLevelA = GetLayerbyCode((const char * )pmsg->pParam + 2);
		
		offset = 2 + RD_CODE_LEN;

		break;
	default:
		break;

	}

	ucDataFormat = *(pmsg->pParam + offset);
	offset += 1;
	switch(ucDataFormat)
	{
	case 0:
		nLevelB = *(pmsg->pParam + offset);
		break;
	case 2:
		nLevelB = GetLayerbyCode((const char * )pmsg->pParam + offset);	
		break;
	case 3:
		{
			memset(CardCSN, 0, CSN_LEN);
			memcpy(CardCSN, pmsg->pParam + offset, CSN_LEN);
			
			for (i = 0; i < g_LCInfo.nCount; i++)
			{
				if (CSNCompare(CardCSN, g_LCInfo.pLFCard[i].CSN))
				{
					bFind = TRUE;
					nFindPos = i;
					break;
				}
			}
			
			if (bFind)
			{
				nLevelB = g_LCInfo.pLFCard[nFindPos].byDestFloor;
			}
			else
			{
				LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
				if(msgSend.pParam)
				{
					free(msgSend.pParam);
				}
				return;
			}
			
			break;
		}
	default:
		break;
	}

	if (nLevelA < nLevelB)
	{
		msgSend.dwMsg	= FC_LF_STOP_UP;
	}
	else if (nLevelA > nLevelB)
	{
		msgSend.dwMsg	= FC_LF_STOP_DOWN;
	}

	memcpy(msgSend.pParam + 4, &dwDA, 4);
	MxPutMsg(&msgSend);
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Mox_CallLiftStop()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				Change ip address string to a dword value
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
Mox_CallLiftStop(MXMSG* pmsg)
{
	MXMSG	msgSend;
	DWORD	dwDA = 0;
	unsigned char ucDataFormat = *(pmsg->pParam + 1);

	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V1;

	msgSend.dwMsg	= pmsg->dwMsg;
	msgSend.pParam	= malloc(8);// 
	if (msgSend.pParam == NULL)
	{
		printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
		return ;
	}
	*msgSend.pParam = 7;
	msgSend.dwParam	= pmsg->dwParam;
	
	switch(ucDataFormat)
	{
		case 0:
			dwDA = GetLiftIDbyLayer(*(pmsg->pParam + 2));
			*(msgSend.pParam + 1) = ucDataFormat;
			*(msgSend.pParam + 2) = *(pmsg->pParam + 2);
			*(msgSend.pParam + 3) = 0xff;
			break;
		case 2:
			dwDA = GetLiftIDbyCode((const char * )pmsg->pParam + 2);
			*(msgSend.pParam + 1) = 0;
			*(msgSend.pParam + 2) = GetLayerbyCode((const char * )pmsg->pParam + 2);
			*(msgSend.pParam + 3) = 0xff;
			
			break;
		default:
			break;
	}

	memcpy(msgSend.pParam + 4, &dwDA, 4);
	MxPutMsg(&msgSend);
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Mox_CallLiftUnlock()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				Change ip address string to a dword value
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
Mox_CallLiftUnlock(MXMSG* pmsg)
{
//	EthResolvInfo 	ResolvInfo;
	MXMSG	msgSend;
	int	offset = 0;
	DWORD	dwDA = 0;
	int	 nLevelCnt = 0;
	unsigned char ucDataFormat = 0;
	int i = 0;
	if (pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
	nLevelCnt = *(pmsg->pParam);
	ucDataFormat = *(pmsg->pParam + 1);

	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V1;

	msgSend.dwMsg	= pmsg->dwMsg;
	msgSend.pParam	= malloc(8 + 3 * nLevelCnt);// 

	if (msgSend.pParam == NULL)
	{
		printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
		return ;
	}

	*msgSend.pParam = 7 + 3 * nLevelCnt;
	msgSend.dwParam	= pmsg->dwParam;
	*(msgSend.pParam + 1) = nLevelCnt;
	switch(ucDataFormat)
	{
	case 0:
		for (i = 0; i < nLevelCnt; i++)
		{
			dwDA = GetLiftIDbyLayer(*(pmsg->pParam + 2 + i * 2));

			*(msgSend.pParam + 2 + i * 3) = ucDataFormat;
			*(msgSend.pParam + 3 + i * 3) = *(pmsg->pParam + 2 + i * 2);
			*(msgSend.pParam + 4 + i * 3) = 0xff;
			offset = 3 + i * 2;
		}

		break;
	case 2:
		offset = 1;
		for (i = 0; i < nLevelCnt; i++)
		{
			offset += 1;
			dwDA = GetLiftIDbyCode((const char * )pmsg->pParam + offset );

			*(msgSend.pParam + 2 + i * 3) = 0;
			*(msgSend.pParam + 3 + i * 3) = GetLayerbyCode((const char * )pmsg->pParam + offset);
			*(msgSend.pParam + 4 + i * 3) = 0xff;			
			offset += strlen((char *)pmsg->pParam + offset);		
		}

		break;
	default:
		break;

	}
	
	memcpy(msgSend.pParam + 2 + 3 * nLevelCnt, pmsg->pParam + offset, 2);
	memcpy(msgSend.pParam + 4 + 3 * nLevelCnt, &dwDA, 4);
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Mox_LFStateRsq()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				request the lift state
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
Mox_LFStateRsq(MXMSG* pmsg)
{
	//EthResolvInfo 	ResolvInfo;
	MXMSG	msgSend;
	//int	offset = 0;

	if (pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
	//unsigned char ucDataFormat = *(pmsg->pParam);
	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V1;

	msgSend.dwMsg	= pmsg->dwMsg;
	msgSend.dwParam	= pmsg->dwParam;
	msgSend.pParam	= NULL;

	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetLFStats_Mox()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				lift control agent get the lift running state
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
GetLFStats_Mox(MXMSG* pmsg)
{
	LCCMD * pCmd = CheckCmdExist(pmsg->dwParam);
	if (pmsg->pParam != NULL && pCmd != NULL)
	{
		LCASendLiftRunningInfo(pCmd, pmsg);	
		RmCmdFromBuffer(pCmd);
	}
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Hitachi_CallLiftA2B()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				convert the Call lift from A to B to hitachi protocol
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
Hitachi_CallLiftA2B(MXMSG* pmsg)
{
	MXMSG	msgSend;
//	int		i = 0;
	int				offset = 0;
	BYTE	CardCSN[CSN_LEN] = { 0 };
	LFCard * pLFCard=NULL;
#ifdef LCA_DEBUG
	printf("%s: get the function command=%d.\n", __FUNCTION__, pmsg->dwMsg);
#endif	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_HITACHI;
	msgSend.pParam		= malloc(2);
	msgSend.dwParam		= pmsg->dwParam;	
	strcpy(msgSend.szSrcDev, pmsg->szSrcDev);
	strcpy(msgSend.szDestDev, g_TalkInfo.szTalkDevCode);
#ifdef LCA_DEBUG
	printf("the pParam[0]=%d, 1=%d.\n", pmsg->pParam[0], pmsg->pParam[1]);
#endif	

	if (LEVEL_CODE_FORMAT == pmsg->pParam[1]) 
	{
		offset+=2;
		memcpy((char *)(msgSend.szSrcDev), (char*)(pmsg->pParam + offset), RD_CODE_LEN);
		offset+=RD_CODE_LEN;
		switch(pmsg->pParam[offset])
		{
		case LEVEL_CODE_FORMAT:
			offset+=1;
			memcpy((char *)(msgSend.szDestDev), (char*)(pmsg->pParam + RD_CODE_LEN + 3), RD_CODE_LEN);
#ifdef LCA_DEBUG
			printf("get the dest device code:%s.\n", msgSend.szDestDev);
#endif
			break;
		case LEVEL_CSN_FORMAT:
			offset+=1;
			memset(CardCSN, 0, CSN_LEN);
			memcpy(CardCSN, pmsg->pParam + offset, CSN_LEN);
			pLFCard=FindLFCardByCSN(CardCSN);
			if(pLFCard)
			{
				memcpy((char *)(msgSend.szDestDev), pLFCard->Code, RD_CODE_LEN);
			}
			else
			{
				printf("Can not find LFCard by CSN \n");
				LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
				if(msgSend.pParam)
				{
					free(msgSend.pParam);
				}
				return;
			}
			break;
		default:
			break;
		}
	}
	else
	{
		printf("%s: the src level info is error: the infor kind=%d.\n", __FUNCTION__, pmsg->pParam[1]);
		return;
	}
	msgSend.dwMsg		= pmsg->dwMsg;

	//((unsigned char *)(&(msgSend.dwParam)))[0] = pmsg->pParam[0];
	//((unsigned char *)(&(msgSend.dwParam)))[1] = g_SysConfig.VldCodeDigits;
	msgSend.pParam[0] = pmsg->pParam[0];
	msgSend.pParam[1] = g_SysConfig.VldCodeDigits;
	
	printf("%s the src=%s, dest=%s.\n", __FUNCTION__, msgSend.szSrcDev, msgSend.szDestDev);
	MxPutMsg(&msgSend);	

}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Hitachi_CallLiftStop()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				convert the stop up command to Hitachi protocol
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
Hitachi_CallLiftStop(MXMSG* pmsg)
{
		MXMSG  msgSend;
	EthResolvInfo 	ResolvInfo;
	//CodeInfo * pCode=NULL;
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_HITACHI;

	msgSend.dwMsg		= FC_LF_STOP;
	//msgSend.pParam		= NULL;
	msgSend.dwParam		= pmsg->dwParam;		
	msgSend.pParam		= malloc(2);
	
	printf("pmsg->szSrcDev=%s\n",pmsg->szSrcDev);
	if(strlen(pmsg->szSrcDev))
	{
		msgSend.pParam[0]=HO_QUERY_METHOD_EHV;
		//pCode=GetCodeInfoByCode(msgSend.szSrcDev);
		//memcpy((char *)(msgSend.szDestDev), pCode->Code, RD_CODE_LEN);
		memcpy((char *)(msgSend.szDestDev), pmsg->szSrcDev, RD_CODE_LEN);
	}
	else
	{
		memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
		ResolvInfo.Level = 0xffff;
		ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;
		ResolvInfo.nIP = ChangeIPFormat(pmsg->dwParam);
		FdFromAMTResolv(&ResolvInfo);
		if (0xffff == ResolvInfo.Level)
		{
		#ifdef LCA_DEBUG
			printf("*************** No Target Callcode in AMT!\n");
		#endif
			return;
		}
		else 
		{
			memcpy((char *)(msgSend.szDestDev), (char*)(ResolvInfo.szDevCode), RD_CODE_LEN);
		}
		printf("ResolvInfo.nType1=%d\n",ResolvInfo.nType);
		
		if (ATM_TYPE_EHV == ResolvInfo.nType) 
		{
			//((unsigned char *)(&(msgSend.dwParam)))[0] = HO_QUERY_METHOD_EHV;
			msgSend.pParam[0]=HO_QUERY_METHOD_EHV;
		}
		else
		{
			//((unsigned char *)(&(msgSend.dwParam)))[0] = HO_QUERY_METHOD_OUTSIDE;		
			msgSend.pParam[0]=HO_QUERY_METHOD_OUTSIDE;
		}
	}

	memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
	ResolvInfo.Level = 0xffff;
	ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;
	ResolvInfo.nIP = ChangeIPFormat(GetSelfIP());
	FdFromAMTResolv(&ResolvInfo);
	if (0xffff == ResolvInfo.Level)
	{
#ifdef LCA_DEBUG
		printf("*************** No Self Callcode in AMT!\n");
#endif
		return;
	}
 	else memcpy((char *)(msgSend.szSrcDev), (char*)(ResolvInfo.szDevCode), RD_CODE_LEN);

	// Get the number of room digits
	//((unsigned char *)(&(msgSend.dwParam)))[1] = g_SysConfig.VldCodeDigits;
	msgSend.pParam[1]=g_SysConfig.VldCodeDigits;
	MxPutMsg(&msgSend);
}

static CodeInfo * GetCodeInfoByCode(char * szCode)
{
	int i;
	for (i = 0; i < g_CodeLift.nCount; i++)
	{
		if (0 == strcmp(szCode, g_CodeLift.pCodeInfo[i].Code))
		{
			return &g_CodeLift.pCodeInfo[i];
		}
	}
	return NULL;
}

static void
DOM_CallLiftStop(MXMSG* pmsg)
{
	EthResolvInfo ResolvInfo;
	DWORD dwSrcIP = 0;
	char call_codes[1+2*RD_CODE_LEN] = {0}; 
	
	memcpy(&dwSrcIP,pmsg->pParam,4);
	if(strlen(pmsg->szSrcDev))
	{
		memcpy(call_codes, pmsg->szSrcDev, RD_CODE_LEN);
	}
	else
	{
		memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
		ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;
		ResolvInfo.nIP = ChangeIPFormat(dwSrcIP);
		FdFromAMTResolv(&ResolvInfo);
		memcpy(call_codes, ResolvInfo.szDevCode, RD_CODE_LEN);	
		printf("dwMsg=%x\n", pmsg->dwMsg);
		switch (pmsg->dwMsg)
		{
		case FC_LF_STOP:
		case FC_LF_STOP_DOWN:
			{
				call_codes[21] = PRESS_KEY_DOWN;
			}
			break;
		case FC_LF_STOP_UP:	
			{
				call_codes[21] = PRESS_KEY_UP;				
			}
			break;		
		default:
			break;
		}
	}
	printf("call_codes=%s\n", call_codes);
	printf("call_codes[21]=%x\n", call_codes[21]);
	ctrl_lift(MXMDID_LCA, HV_CALL_LIFT, 0, call_codes);
}

static void
Mit_CallLiftStop(MXMSG* pmsg)
{
	MXMSG  msgSend;
	EthResolvInfo 	ResolvInfo;
	BOOL bRet=FALSE;
	DWORD dwSrcIP;
	CodeInfo * pCode=NULL;
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MITSUBISHI;
	memcpy(&dwSrcIP,pmsg->pParam,4);
    printf("Mit_CallLiftStop dwSrcIP=0x%08lx\n",dwSrcIP);
	msgSend.dwMsg		= FC_LF_A_B;
	msgSend.dwParam = pmsg->dwParam;
	msgSend.pParam = malloc(7/*bank+node+length+position+src floor+admin+dst floor*/);

	memset(msgSend.pParam, 0, 7);

	*(msgSend.pParam + 2) = 4;//LEN
	*(msgSend.pParam + 3) = 0x80;
	if(strlen(pmsg->szSrcDev))
	{
		pCode=GetCodeInfoByCode(pmsg->szSrcDev);
	}
	else
	{
		memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
		ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;
		ResolvInfo.nIP = ChangeIPFormat(dwSrcIP);
		FdFromAMTResolv(&ResolvInfo);
	    printf("ResolvInfo.szDevCode1=%s\n",ResolvInfo.szDevCode);	
		pCode=GetCodeInfoByCode(ResolvInfo.szDevCode);
	}
	
	if(pCode!=NULL)
	{
		*(msgSend.pParam + 4) = pCode->byFloor;
		*(msgSend.pParam) = pCode->bank;
		*(msgSend.pParam + 1) = pCode->node;
		*(msgSend.pParam + 5) = 0;
		*(msgSend.pParam + 3) |=pCode->byFBDoor;
	
		memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
		ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;
		ResolvInfo.nIP = ChangeIPFormat(GetSelfIP());
		FdFromAMTResolv(&ResolvInfo);
		pCode=GetCodeInfoByCode(ResolvInfo.szDevCode);
		printf("ResolvInfo.szDevCode2=%s\n",ResolvInfo.szDevCode);	
		if(pCode!=NULL)
		{
			*(msgSend.pParam + 6) = pCode->byFloor;
			bRet=TRUE;
			MxPutMsg(&msgSend);
		}
	}
	if(bRet==FALSE)
	{
		printf("Mit_CallLiftStop Error\n");
		free(msgSend.pParam);
	}
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Hitachi_CallLiftStopUp()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**			 convert the stop up command to Hitachi protocol
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
/*static void
Hitachi_CallLiftStopUp(MXMSG* pmsg)
{
}*/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Hitachi_CallLiftStopDown()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				Change ip address string to a dword value
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
/*static void
Hitachi_CallLiftStopDown(MXMSG* pmsg)
{

}*/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EGMLCAMdInit()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				Change ip address string to a dword value
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
Hitachi_CallLiftUnlock(MXMSG* pmsg)
{
;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CmdBufferEmpty()
**	AUTHOR:			Wayde Zeng
**	DATE:			14 - Jan - 2011
**
**	DESCRIPTION:	
**				Judge whether command buffer is empty
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
BOOL
CmdBufferEmpty(VOID)
{
	int i = 0;
	
	for (i = 0; i < MAX_CMD_NUM; i++)
	{
		if (g_CMDBuff[i].bValid) 
		{
			return FALSE;
		}
	}
	
	return TRUE;
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddCmd2Buffer()
**	AUTHOR:			Harry Qian
**	DATE:			28 - Sep - 2010
**
**	DESCRIPTION:	
**				add a command to command buffer
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
/*static void
AddCmd2Buffer(int nCmdId, DWORD dwIP, unsigned short usCmd)
{
	LCCMD	* pCmd = NULL;

	pCmd = GetBlankCmdBuff();
	if (pCmd == NULL)
	{
		printf("%s: Error, there is no blank command buffer.\n", __FUNCTION__);
		return;
	}
	pCmd->bValid	= TRUE;
	pCmd->nCmdId	= nCmdId;
	pCmd->dwIP		= dwIP;
	pCmd->usCMD		= usCmd;
	pCmd->dwTimeStart = GetTickCount();
#ifdef LCA_DEBUG
	printf("%s: nCommandID=%d, dwIP=%x, usCommand=%x\n", __FUNCTION__, nCmdId, dwIP, usCmd);
#endif
}*/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddMsg2Buffer()
**	AUTHOR:			Wayde Zeng
**	DATE:			20 - Jan - 2011
**
**	DESCRIPTION:	
**				add a message to command buffer
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
AddMsg2Buffer(MXMSG* pMsg)
{
	LCCMD*	pCmd = NULL;
	int		nDataLen = 0;	
	
	pCmd = GetBlankCmdBuff();
	if (pCmd == NULL)
	{
		printf("%s: Error, there is no blank command buffer.\n", __FUNCTION__);
		return;
	}
	pCmd->bValid	= TRUE;
	pCmd->nCmdId	= g_CmdIndex++;
	pCmd->dwIP		= pMsg->dwParam;
	pCmd->usCMD		= pMsg->dwMsg;
	memcpy(pCmd->szSrcDev,pMsg->szSrcDev,MAX_LEN_DEV_CODE + 1);
	memcpy(pCmd->szDestDev,pMsg->szDestDev,MAX_LEN_DEV_CODE + 1);
	memcpy(&nDataLen, pMsg->pParam, sizeof(int));

	pCmd->nDataLen	= nDataLen;
	pCmd->pData		= (BYTE*)malloc(pCmd->nDataLen);
	memcpy(pCmd->pData, pMsg->pParam + sizeof(int), pCmd->nDataLen);
	pCmd->dwTimeStart = GetTickCount();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetBlankCmdBuff()
**	AUTHOR:			Harry Qian
**	DATE:			28 - Sep - 2010
**
**	DESCRIPTION:	
**				get a black command buffer
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static LCCMD *
GetBlankCmdBuff(void)
{
	int i = 0;

	for (i = 0; i < MAX_CMD_NUM; i++)
	{
		if (!g_CMDBuff[i].bValid) 
		{
			return &g_CMDBuff[i];
		}
	}

	return NULL;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddMsg2Buffer()
**	AUTHOR:			Wayde Zeng
**	DATE:			20 - Jan - 2011
**
**	DESCRIPTION:	
**				get a message from command buffer
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static LCCMD *
GetCmdFromBuff(void)
{
	int i = 0;
	
	for (i = 0; i < MAX_CMD_NUM; i++)
	{
		if (g_CMDBuff[i].bValid) 
		{
			return &g_CMDBuff[i];
		}
	}
	
	return NULL;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CheckCmdExist()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				To check if the command exist in buffer
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nCmdId		[IN]		int		the command id
**	RETURNED VALUE:	
**		a pointer of the command	
**	NOTES:
**		
*/
static LCCMD *
CheckCmdExist(int nCmdId)
{
	int i = 0;

	for (i = 0; i < MAX_CMD_NUM; i++)
	{
		if (g_CMDBuff[i].nCmdId == nCmdId) 
		{
			return &g_CMDBuff[i];
		}
	}
	return NULL;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RmCmdFromBuffer()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				remove the command from buffer
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**			
**	NOTES:
**		
*/

 static BOOL RMCMDByMsg(MXMSG * pmsg)
{
	LCCMD * pCmd = CheckCmdExist(pmsg->dwParam);	
 	if ( pCmd != NULL)
	{
		RmCmdFromBuffer(pCmd);
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}


static LCCMD *
RmCmdFromBuffer(LCCMD * pCmd)
{
#ifdef LCA_DEBUG
	printf("%s: nCommandID=%d, dwIP=%x, usCommand=%x\n", __FUNCTION__, pCmd->nCmdId, pCmd->dwIP, pCmd->usCMD);
#endif
	if (pCmd != NULL)
	{
		pCmd->bValid	= FALSE;
		pCmd->nCmdId	= 0;
		pCmd->dwIP		= 0;
		pCmd->usCMD		= 0;
		pCmd->dwTimeStart = GetTickCount();
		if(pCmd->pData)
		 {
		 	MXFree(pCmd->pData);
			pCmd->pData=NULL;
		 }
	}
	pCmd = NULL;

	return pCmd;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RmCmdFromBuffer()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**				remove the command from buffer
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**			
**	NOTES:
**		
*/
/*static void
CheckCmdTimeout(LCCMD * pCmd)
{
	if (pCmd != NULL)
	{
		pCmd->bValid	= FALSE;
		pCmd->nCmdId	= 0;
		pCmd->dwIP		= 0;
		pCmd->usCMD		= 0;
		pCmd->dwTimeStart = GetTickCount();
	}
	pCmd = NULL;
	return pCmd;
}*/


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	EGMLCACmdAckPro()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**			Lift control Agent send lift command ACK to Lift control module
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**			
**	NOTES:
**		
*/
static void
EGMLCACmdAckPro(MXMSG * pmsg)
{
LCASendErrorACK2LC(pmsg);
	/*LCCMD * pCmd = CheckCmdExist(pmsg->dwParam);
	if (pmsg->pParam != NULL && pCmd != NULL)
	{
		//if(*(pmsg->pParam))
		{
			LCASendErrorACK2LC(pCmd, *(pmsg->pParam));
		}
		RmCmdFromBuffer(pCmd);
	}
	return TRUE;*/
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCASendErrorACK2LC()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**			Lift control Agent send lift command ACK to Lift control module
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**			
**	NOTES:
**		
*/
static void
LCASendErrorACK2LC(MXMSG * pmsg)
{
	MXMSG  msgSend;

	if (pmsg == NULL)
	{
		return ;
	}

	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_ETH;

	//msgSend.dwMsg		= pCmd->usCMD + 0x8000;
	msgSend.dwMsg		= pmsg->dwMsg;
	msgSend.dwParam		= pmsg->dwParam;
	msgSend.pParam		= malloc(1);
	*msgSend.pParam		= pmsg->pParam[0];
	MxPutMsg(&msgSend);
//#ifdef LCA_DEBUG
	printf("Send the msg=%lx, error ACK=%d. to ip=%lx\n", msgSend.dwMsg, *msgSend.pParam, msgSend.dwParam);
//#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCASendLiftRunningInfo()
**	AUTHOR:			Harry Qian
**	DATE:			15 - Aug - 2010
**
**	DESCRIPTION:	
**			Lift control Agent send lift running information to Lift control module
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**			
**	NOTES:
**		
*/
static void
LCASendLiftRunningInfo(LCCMD * pCmd, MXMSG * pmsg)
{
	MXMSG  msgSend;
	unsigned short nLen = 7;

	if (pCmd == NULL || pmsg == NULL || pmsg->pParam == NULL)
	{
		return ;
	}

	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LC;

	msgSend.dwMsg		= pCmd->usCMD + 0x8000;
	msgSend.dwParam		= pCmd->dwIP;
	msgSend.pParam		= malloc(9);
	memcpy(msgSend.pParam, &nLen, 2);
	memcpy(msgSend.pParam + 2, pmsg->pParam, 7);

	MxPutMsg(&msgSend);
#ifdef LCA_DEBUG
	printf("Send the msg=%x,  to ip=%x\n", msgSend.dwMsg,  pCmd->dwIP);
#endif
}

static void
DOM_CallLiftA2B(MXMSG* pmsg)
{
	int i = 0;
	unsigned char pos = 0;
	unsigned char data_format = 0; 
	char call_codes[1+2*RD_CODE_LEN] = {0};	
	char csn_code[CSN_LEN] = {0};
	LFCard * pLFCard = NULL;

	pos = *(pmsg->pParam);
	switch (pos)
	{
	case HO_QUERY_METHOD_CALLCODE: // ∑√øÕø™À¯
		{
			data_format = *(pmsg->pParam + 1);			
			if (LEVEL_CODE_FORMAT == data_format)
			{
				memcpy(call_codes, pmsg->pParam + 2, RD_CODE_LEN);
				call_codes[20] = 0;
				data_format = *(pmsg->pParam + 22);
				if (LEVEL_CODE_FORMAT == data_format)
				{
					memcpy(call_codes+21, pmsg->pParam + 23, RD_CODE_LEN);
				}
			}
		}
		ctrl_lift(MXMDID_LCA, VISTOR_UNLOCK, 0, call_codes);
		break;
	case HO_QUERY_METHOD_CSN: // À¢ø®ø™À¯
		{
			data_format = *(pmsg->pParam + 1);			
			if (LEVEL_CODE_FORMAT == data_format)
			{
				memcpy(call_codes, pmsg->pParam + 2, RD_CODE_LEN);
				call_codes[20] = 0;
				data_format = *(pmsg->pParam + 22);
				if (LEVEL_CSN_FORMAT == data_format)
				{
					memcpy(csn_code, pmsg->pParam + 23, CSN_LEN);
					pLFCard = FindLFCardByCSN(csn_code);
					if (NULL != pLFCard)
					{
						memcpy(call_codes+21, pLFCard->Code, RD_CODE_LEN);
						ctrl_lift(MXMDID_LCA, SWIPER_CARD_UNLOCK, 0, call_codes);
					}
				}
			}
		}
		break;
	case HO_QUERY_METHOD_HOME: // ªßªß∂‘Ω≤
		{
			data_format = *(pmsg->pParam + 1);			
			if (LEVEL_CODE_FORMAT == data_format)
			{
				memcpy(call_codes, pmsg->pParam + 2, RD_CODE_LEN);
				call_codes[20] = 0;
				data_format = *(pmsg->pParam + 22);
				if (LEVEL_CODE_FORMAT == data_format)
				{
					memcpy(call_codes+21, pmsg->pParam + 23, RD_CODE_LEN);
				}
			}
		}
		ctrl_lift(MXMDID_LCA, HV_CALLING_HV_CALL_LIFT, 0, call_codes);
		break;
	default:
		{
			printf("%s:%s:%d pos=0x%x\n", __FILE__, __FUNCTION__, __LINE__, pos);
		}
		break;
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Mit_CallLiftA2B()
**	AUTHOR:			Harry Qian
**	DATE:			21 - Nov - 2010
**
**	DESCRIPTION:	for Opening Gate by Card ( + PWD) and Opening Gate by RD + PWD
**                  1.according to CLT file,get information. 
**                  2.if it is Opening Gate by Card ( + PWD),get information from ALT file.
**                    if it is Opening Gate by RD + PWD,get information from CLT file.
**				    we can know that whether this user is Admin with ALT file 
**			        we can know Calling way with CLT file
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**
**	NOTES:
**			
*/
static void
Mit_CallLiftA2B(MXMSG* pmsg)
{
	MXMSG			msgSend;
	int				offset = 0;
	unsigned char	pos = 0;
	unsigned char	ucDataFormat = 0;
	int				i = 0;
	int				nFindPos = 0;
	BOOL			bFind = FALSE;
	BYTE			CardCSN[CSN_LEN] = {0};
	char			szCode[RD_CODE_LEN] = {0};
	LFCard * pLFCard=NULL;
	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
	
	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MITSUBISHI;

	msgSend.dwMsg	= pmsg->dwMsg;
	msgSend.dwParam = pmsg->dwParam;
    
	msgSend.pParam = malloc(7/*bank+node+length+position+src floor+admin+dst floor*/);
	if (msgSend.pParam == NULL)
	{
		printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
		return ;
	}

	memset(msgSend.pParam, 0, 7);

	*(msgSend.pParam + 2) = 4;
    
	pos = *(pmsg->pParam);

	switch (pos)
	{
		case HO_QUERY_METHOD_CSN:
		case HO_QUERY_METHOD_RDCODE:
		{
			*(msgSend.pParam + 3) = 0x40;
			break;
		}
		case HO_QUERY_METHOD_CALLCODE:
		case HO_QUERY_METHOD_HOME:
		{
			*(msgSend.pParam + 3) = 0x80;
			break;
		}
		default:
		{
			*(msgSend.pParam + 3) = 0x00;
			break;
		}
	}

	ucDataFormat = *(pmsg->pParam + 1);
	printf("ucDataFormat1 = %d\n",ucDataFormat);
	switch(ucDataFormat)
	{
		case LEVEL_CODE_FORMAT:
		{
			memset(szCode, 0, RD_CODE_LEN);
			memcpy(szCode, pmsg->pParam + 2, RD_CODE_LEN);
            
			switch(pos)
			{
				case HO_QUERY_METHOD_HOME:
					break;
				default:
					break;
			}
			if(HO_QUERY_METHOD_HOME==pos)
			{
				for (i = 0; i < g_CodeLift.nCount; i++)
				{
				    //printf("g_CodeLift.pCodeInfo[%d].Code=%s",i,g_CodeLift.pCodeInfo[i].Code);
					if ((CALL_LIFT_METHOD_DOORDOOR==g_CodeLift.pCodeInfo[i].byMethod) && (0 == strcmp(szCode, g_CodeLift.pCodeInfo[i].Code)))
					{
						bFind = TRUE;
						nFindPos = i;
						//printf("DoorDoor  pCodeInfo[%d].Code=%s\n",i,g_CodeLift.pCodeInfo[i].Code);
						break;
					}
				}
			}
			if(FALSE==bFind)
			{
				for (i = 0; i < g_CodeLift.nCount; i++)
				{
				    //printf("g_CodeLift.pCodeInfo[%d].Code=%s \n",i,g_CodeLift.pCodeInfo[i].Code);
					if ((CALL_LIFT_METHOD_DEF==g_CodeLift.pCodeInfo[i].byMethod) && (0 == strcmp(szCode, g_CodeLift.pCodeInfo[i].Code)))
					{
						bFind = TRUE;
						nFindPos = i;
						break;
					}
				}
			}


			if (bFind)
			{
				*(msgSend.pParam + 4) = g_CodeLift.pCodeInfo[nFindPos].byFloor;
				*(msgSend.pParam + 3)|= g_CodeLift.pCodeInfo[nFindPos].byFBDoor;
				*(msgSend.pParam) = g_CodeLift.pCodeInfo[nFindPos].bank;
				*(msgSend.pParam + 1) = g_CodeLift.pCodeInfo[nFindPos].node;
			}
			else
			{
				printf("Error Can not find Code in g_CodeLift.pCodeInfo\n");
				LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
				if(msgSend.pParam)
				{
					free(msgSend.pParam);
				}
				return;
			}

			offset = 2 + RD_CODE_LEN;
			break;	
		}			
		default:
			break;		
	}

	*(msgSend.pParam + 5) = 0;
	bFind=FALSE;
	ucDataFormat = *(pmsg->pParam + offset);
	offset += 1;
	printf("ucDataFormat2 = %d\n",ucDataFormat);
	switch(ucDataFormat)
	{
		case LEVEL_CODE_FORMAT:
		{
			memset(szCode, 0, RD_CODE_LEN);
			memcpy(szCode, pmsg->pParam + offset, RD_CODE_LEN);
            
			if(HO_QUERY_METHOD_HOME==pos)
			{
				for (i = 0; i < g_CodeLift.nCount; i++)
				{
			    	//printf("g_CodeLift.pCodeInfo[%d].Code=%s \n",i,g_CodeLift.pCodeInfo[i].Code);
					if ((CALL_LIFT_METHOD_DOORDOOR==g_CodeLift.pCodeInfo[i].byMethod) && (0 == strcmp(szCode, g_CodeLift.pCodeInfo[i].Code)))
					{
						bFind = TRUE;
						nFindPos = i;
						//printf("DoorDoor  pCodeInfo[%d].Code=%s \n",i,g_CodeLift.pCodeInfo[i].Code);
						break;
					}
				}
			}
			if(FALSE==bFind)
			{
				for (i = 0; i < g_CodeLift.nCount; i++)
				{
				    //printf("g_CodeLift.pCodeInfo[%d].Code=%s \n",i,g_CodeLift.pCodeInfo[i].Code);
					if ((CALL_LIFT_METHOD_DEF==g_CodeLift.pCodeInfo[i].byMethod) && (0 == strcmp(szCode, g_CodeLift.pCodeInfo[i].Code)))
					{
						bFind = TRUE;
						nFindPos = i;
						break;
					}
				}
			}
			
			if (bFind)
			{
				*(msgSend.pParam + 6) = g_CodeLift.pCodeInfo[nFindPos].byFloor;
				*(msgSend.pParam + 3)|= (g_CodeLift.pCodeInfo[nFindPos].byFBDoor)<<4;
				if(IsMitSCPLFlag())
				{
				    printf("Set Bank Node 2\n");
					*(msgSend.pParam) = g_CodeLift.pCodeInfo[nFindPos].bank;
					*(msgSend.pParam + 1) = g_CodeLift.pCodeInfo[nFindPos].node;
				}
			}
			else
			{
				LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
				if(msgSend.pParam)
				{
					free(msgSend.pParam);
				}
				return;
			}
			
			break;	
		}
		case LEVEL_CSN_FORMAT:
		{
			memset(CardCSN, 0, CSN_LEN);
			memcpy(CardCSN, pmsg->pParam + offset, CSN_LEN);
			
			pLFCard = FindLFCardByCSN(CardCSN);
			if(pLFCard)
			{
				for (i = 0; i < g_CodeLift.nCount; i++)
				{
					if (0 == strcmp(pLFCard->Code, g_CodeLift.pCodeInfo[i].Code) && (CALL_LIFT_METHOD_DEF==g_CodeLift.pCodeInfo[i].byMethod))
					{
						bFind = TRUE;
						nFindPos = i;
						break;
					}
				}
				
				if (bFind)
				{
					if(IsMitSCPLFlag())
					{
					    printf("Set Bank Node1\n");
						*(msgSend.pParam) = g_CodeLift.pCodeInfo[nFindPos].bank;
						*(msgSend.pParam + 1) = g_CodeLift.pCodeInfo[nFindPos].node;
					}
				}
				*(msgSend.pParam + 6) = pLFCard->byDestFloor;
                *(msgSend.pParam + 5) |=  (pLFCard->bAdmin)<<4; //added by [MichaelMa] at 29-9-2012
			}
			else
			{
				LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
				if(msgSend.pParam)
				{
					free(msgSend.pParam);
				}
				return;
			}	
			break;
		}
		default:
			break;		
	}

	if( *(msgSend.pParam+3) & LIFT_FB_DOOR)//«∞∫Û√≈¥¶¿Ì–Ë«Û
	{
		MXMSG	tempMsg;
		BYTE DoorType = *(msgSend.pParam+3);
		memcpy(&tempMsg,&msgSend,sizeof(MXMSG));
		tempMsg.pParam = malloc(7);
		if (tempMsg.pParam == NULL)
		{
			printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
			return ;
		}
		memcpy(tempMsg.pParam, msgSend.pParam, 7);
		DoorType &= (~LIFT_FB_DOOR);
		*(msgSend.pParam + 3) = DoorType | LIFT_FRONT_DOOR;// «∞√≈
		MxPutMsg(&msgSend);
		*(tempMsg.pParam + 3) = DoorType | LIFT_BACK_DOOR;// ∫Û√≈
		MxPutMsg(&tempMsg);
	}
	else
	{
		MxPutMsg(&msgSend);
	}
	printf("LCA send the data: ");
	for (offset = 0 ; offset < 7 ; offset++)
	{
		printf("%02x ", *(msgSend.pParam + offset));
	}
	printf("\n");
}


static void
DOM_CallLiftUnlock(MXMSG* pmsg)
{
	BYTE CardCSN[CSN_LEN] = { 0 };
	BYTE bStationNum = 0;
	LFCard *pLFCard = NULL;
	char codes[1+2*RD_CODE_LEN] = {0};
	
	if (pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
	memcpy(CardCSN, pmsg->pParam, CSN_LEN);
	memcpy(&bStationNum, pmsg->pParam+CSN_LEN, 1);	
	pLFCard = FindLFCardByCSN(CardCSN);
	if (NULL != pLFCard)
	{
		if (pLFCard->bAdmin)
		{
			memcpy(codes, CODE_ADMIN, RD_CODE_LEN);
		}
		else
		{
			memcpy(codes, pLFCard->Code, RD_CODE_LEN);
		}	
		memcpy(codes+21, (BYTE *)&(pmsg->dwParam), 4);	
		ctrl_lift(MXMDID_LCA, SWIPER_CARD_IN_LIFT, bStationNum, codes);
	}
	else
	{
		memcpy(codes, CODE_INVALID, RD_CODE_LEN);
		memcpy(codes+21, (BYTE *)&(pmsg->dwParam), 4);	
		ctrl_lift(MXMDID_LCA, SWIPER_CARD_IN_LIFT, bStationNum, codes);		
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Mit_CallLiftUnlock()
**	AUTHOR:			Harry Qian
**	DATE:			21 - Nov - 2010
**
**	DESCRIPTION:	
**				Change ip address string to a dword value
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
Mit_CallLiftUnlock(MXMSG* pmsg)
{
	MXMSG msgSend;
	BYTE CardCSN[CSN_LEN] = { 0 };
	int i = 0;
	int	nFindPos = 0;
	BOOL bFind = FALSE;
	
	BOOL bFindCodeInfo = FALSE;
	BOOL bFindRealCodeInfo = FALSE;
	
	BYTE	bStationNum=0;
	LTCARDREADER *	pCardReader;
	if (pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MITSUBISHI;

	msgSend.dwMsg	= pmsg->dwMsg;
	msgSend.dwParam = pmsg->dwParam;

	memcpy(CardCSN, pmsg->pParam, CSN_LEN);
	
	memcpy(&bStationNum, pmsg->pParam+CSN_LEN, 1);
	
	for (i = 0; i < g_LCInfo.nCount; i++)
	{
		if (CSNCompare(CardCSN, g_LCInfo.pLFCard[i].CSN))
		{
			bFind = TRUE;
			nFindPos = i;
		
			break;
		}
	}

	pCardReader = GetLiftInfoByStationNum(bStationNum);
	
	if (bFind)
	{
		if (0 == g_LCInfo.pLFCard[nFindPos].byFrontDoorLen && 0 == g_LCInfo.pLFCard[nFindPos].byBackDoorLen)
		{
			msgSend.pParam = malloc(7);
			if (NULL == msgSend.pParam)
			{
				printf(" %s: ERROR, malloc memory failed.\n", __FUNCTION__);
				return;
			}	
			if(pCardReader)
			{
				for (i = 0; i < g_CodeLift.nCount; i++)
				{
				    //printf("g_CodeLift.pCodeInfo[%d].Code=%s\n",i,g_CodeLift.pCodeInfo[i].Code);
					if ((CALL_LIFT_METHOD_LIFTUNLOCK==g_CodeLift.pCodeInfo[i].byMethod) && (0 == strcmp(g_LCInfo.pLFCard[nFindPos].Code, g_CodeLift.pCodeInfo[i].Code)))//ÈÖçÁΩÆ‰∫ÜËΩøÂé¢Âà∑Âç°ÊñπÂºè
					{
				        //printf("bFindCodeInfo = TRUE,pCardReader->bBank=%d,g_CodeLift.pCodeInfo[i].bank=%d\n",pCardReader->bBank,g_CodeLift.pCodeInfo[i].bank);
				        //printf("pCardReader->bNode=%d,g_CodeLift.pCodeInfo[i].node=%d\n",pCardReader->bNode,g_CodeLift.pCodeInfo[i].node);
						bFindCodeInfo = TRUE;
						if((g_CodeLift.pCodeInfo[i].bank==pCardReader->bBank) && (g_CodeLift.pCodeInfo[i].node==pCardReader->bNode))
						{
							//printf("bFindRealCodeInfo=TRUE\n");
							bFindRealCodeInfo = TRUE;
						}
						//break;
					}
				}
				if(bFindCodeInfo==FALSE || bFindRealCodeInfo==TRUE)
				{
					*(msgSend.pParam) = pCardReader->bBank;
					*(msgSend.pParam + 1) = pCardReader->bNode;
				}
				else
				{
					LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
					if(msgSend.pParam)
					{
						free(msgSend.pParam);
					}
					return;
				}
			}
			else
			{
				*(msgSend.pParam) = g_LCInfo.pLFCard[nFindPos].bank;
				*(msgSend.pParam + 1) = g_LCInfo.pLFCard[nFindPos].node;
            }

			*(msgSend.pParam + 2) = 4;
			*(msgSend.pParam + 3) = 0;
			if (g_LCInfo.pLFCard[nFindPos].bAdmin)
			{
				*(msgSend.pParam + 4) = 0x10;
				*(msgSend.pParam + 5) = 0xff;
			}
			else
			{
				*(msgSend.pParam + 4) = 0;
				*(msgSend.pParam + 5) = g_LCInfo.pLFCard[nFindPos].byDestFloor;
			}
			
			
			*(msgSend.pParam + 6) = 0;
		}
		else
		{
			msgSend.pParam = malloc(6 + g_LCInfo.pLFCard[nFindPos].byFrontDoorLen + g_LCInfo.pLFCard[nFindPos].byBackDoorLen);
			if (NULL == msgSend.pParam)
			{
				printf(" %s: ERROR, malloc memory failed.\n", __FUNCTION__);
				return;
			}
			if(pCardReader)
			{
				for (i = 0; i < g_CodeLift.nCount; i++)
				{
					if ((CALL_LIFT_METHOD_LIFTUNLOCK==g_CodeLift.pCodeInfo[i].byMethod) && (0 == strcmp(g_LCInfo.pLFCard[nFindPos].Code, g_CodeLift.pCodeInfo[i].Code)))//ÈÖçÁΩÆ‰∫ÜËΩøÂé¢Âà∑Âç°ÊñπÂºè
					{
						bFindCodeInfo = TRUE;
						if((g_CodeLift.pCodeInfo[i].bank==pCardReader->bBank) && (g_CodeLift.pCodeInfo[i].node==pCardReader->bNode))
						{
							bFindRealCodeInfo=TRUE;
						}
					}
				}
				if(bFindCodeInfo==FALSE || bFindRealCodeInfo==TRUE)
				{
					*(msgSend.pParam) = pCardReader->bBank;
					*(msgSend.pParam + 1) = pCardReader->bNode;
				}
				else
				{
					LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
					if(msgSend.pParam)
					{
						free(msgSend.pParam);
					}
					return;
				}
			}
			else
			{
				*(msgSend.pParam) = g_LCInfo.pLFCard[nFindPos].bank;
				*(msgSend.pParam + 1) = g_LCInfo.pLFCard[nFindPos].node;
			}
			*(msgSend.pParam + 2) = 3 + g_LCInfo.pLFCard[nFindPos].byFrontDoorLen + g_LCInfo.pLFCard[nFindPos].byBackDoorLen;
			*(msgSend.pParam + 3) = 0;
			if (g_LCInfo.pLFCard[nFindPos].bAdmin)
			{
				*(msgSend.pParam + 4) = 0x10;
			}
			else
			{
				*(msgSend.pParam + 4) = 0;
			}
			*(msgSend.pParam + 5) = (g_LCInfo.pLFCard[nFindPos].byBackDoorLen << 4) + g_LCInfo.pLFCard[nFindPos].byFrontDoorLen;
			memcpy(msgSend.pParam + 6, &(g_LCInfo.pLFCard[nFindPos].dlFrontUnlockLevel), g_LCInfo.pLFCard[nFindPos].byFrontDoorLen);
			memcpy(msgSend.pParam + 6 + g_LCInfo.pLFCard[nFindPos].byFrontDoorLen, &(g_LCInfo.pLFCard[nFindPos].dlBackUnlockLevel), g_LCInfo.pLFCard[nFindPos].byBackDoorLen);
		}
	}
	else
	{
		LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
		if(msgSend.pParam)
		{
			free(msgSend.pParam);
		}
		return;
	}
    
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Mit_LFStateRsq()
**	AUTHOR:			Harry Qian
**	DATE:			21 - Nov - 2010
**
**	DESCRIPTION:	
**				request the lift state
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
Mit_LFStateRsq(MXMSG* pmsg)
{
	MXMSG	msgSend;

	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MITSUBISHI;

	msgSend.dwMsg	= pmsg->dwMsg;
	msgSend.dwParam = pmsg->dwParam;
	msgSend.pParam	= NULL;

	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GetLFStats_Mit()
**	AUTHOR:			Harry Qian
**	DATE:			21 - Nov - 2010
**
**	DESCRIPTION:	
**				lift control agent get the lift running state
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static void
GetLFStats_Mit(MXMSG* pmsg)
{
	LCCMD * pCmd = CheckCmdExist(pmsg->dwParam);
	if (pmsg->pParam != NULL && pCmd != NULL)
	{
		//LCASendLiftRunningInfo(pCmd, pmsg);	
		RmCmdFromBuffer(pCmd);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCASendStateCheck()
**	AUTHOR:			Wayde Zeng
**	DATE:			14 - Jan - 2011
**
**	DESCRIPTION:	
**				send check state
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
LCASendStateCheck(VOID)
{
	MXMSG msgSend;
	int	  nDataLen = 0;
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LCA;

	msgSend.dwMsg		= FC_LF_STATE;
	//msgSend.pParam		= NULL;
	msgSend.pParam		= (BYTE*)malloc(sizeof(int));
	memcpy(msgSend.pParam, &nDataLen, sizeof(int));
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAHeartBeat()
**	AUTHOR:			Wayde Zeng
**	DATE:			14 - Jan - 2011
**
**	DESCRIPTION:	
**				Heart Beat
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
LCAHeartBeat(VOID)
{
	MXMSG msgSend;

	if (GetTickCount() - g_dwHeartBeatTick > 2000)
	{
		g_dwHeartBeatTick = GetTickCount();
		
		memset(&msgSend, 0, sizeof(msgSend));
		msgSend.dwSrcMd		= MXMDID_LCA;
		msgSend.dwDestMd	= MXMDID_LCA;
	
		msgSend.dwMsg		= FC_LF_HEARTBEAT;		
		msgSend.pParam		= NULL;
		
		MxPutMsg(&msgSend);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Mit_LFHeartBeat()
**	AUTHOR:			Wayde Zeng
**	DATE:			14 - Jan - 2011
**
**	DESCRIPTION:	
**				Send Heart Beat to Mitsubishi
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
void
Mit_LFHeartBeat(MXMSG* pmsg)
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MITSUBISHI;
	
	msgSend.dwMsg	= pmsg->dwMsg;
	msgSend.pParam	= NULL;
	
	MxPutMsg(&msgSend);
}
static
void
MOX2_LFHeartBeat(MXMSG* pmsg)
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V2;
	
	msgSend.dwMsg	= pmsg->dwMsg;
	msgSend.pParam	= NULL;
	
	MxPutMsg(&msgSend);
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCASendErrorACK2Target()
**	AUTHOR:			Wayde Zeng
**	DATE:			20 - Jan - 2011
**
**	DESCRIPTION:	
**				Send error ACK to target
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
void
LCASendErrorACK2Target(DWORD dwMsg, DWORD dwCmdID, unsigned char ucRetCode)
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LCA;
	
	if (dwMsg < 0x8000)
	{
		msgSend.dwMsg		= dwMsg + 0x8000;
	}
	
	msgSend.dwParam		= dwCmdID;
	msgSend.pParam		= malloc(1);
	*msgSend.pParam		= ucRetCode;
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAWaitingAckPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			20 - Jan - 2011
**
**	DESCRIPTION:	
**				wait for ACK from PC
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
LCAWaitingAckPro(VOID)
{
	switch(g_SysConfig.LC_ComMode)
	{
		case LC_MOX_MODE_V1:
			LCAMoxWaitingAckPro();
			break;
		case LC_MOX_MODE_V2:
			LCAMox2WaitingAckPro();
			break;
		case LC_HITACHI_MODE:
			break;
		case LC_MITSUBISHI_MODE:
			LCAMitWaitingAckPro();
			break;
		default:
			break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LCAProtocolPro()
**	AUTHOR:			Wayde Zeng
**	DATE:			20 - Jan - 2011
**
**	DESCRIPTION:	
**				protocol process
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static
VOID
LCAProtocolPro(VOID)
{
	switch(g_SysConfig.LC_ComMode)
	{
		case LC_MOX_MODE_V1:
			LCAMoxThreadFun();
			break;
		case LC_MOX_MODE_V2:
			LCAMox2ThreadFun();
			break;
		case LC_HITACHI_MODE:
			LCConversionThreadFun();
			break;
		case LC_MITSUBISHI_MODE:
			LCAMitThreadFun();
			break;
		default:
			break;
	}
}


static void
Mox2_CallLiftA2B(MXMSG* pmsg)
{
//EthResolvInfo	ResolvInfo;
	MXMSG			msgSend;
	int				offset = 0;
//	DWORD			dwDA = 0;
	unsigned char	pos = 0;
	unsigned char	ucDataFormat = 0;
//	int				i = 0;
//	int				nFindPos = 0;
//	BOOL			bFind = FALSE;
	BYTE			CardCSN[CSN_LEN] = { 0 };
//	char			szCode[RD_CODE_LEN];
	MOXUSER	*	pUser;
	//MOXUSER	*	pUserSrc;
	char			szDoorCode[RD_CODE_LEN] = { 0 };
	char			szUserCode[RD_CODE_LEN] = { 0 };
	char			szSrcUserCode[RD_CODE_LEN] = { 0 };
	unsigned char*	pData;
	MOXLIFT	*	pLift;
	MOXDOOR	*	pDoor;
	unsigned short sIdentity=0;

	printf("############### %s\n", __FUNCTION__);
	LFCard * pLFCard=NULL;
	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
	
	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V2;

	msgSend.dwMsg	= pmsg->dwMsg;

	
	pos = *(pmsg->pParam);
	
	printf("pos=%d\n",pos);
	ucDataFormat = *(pmsg->pParam + 1);
	
	switch(ucDataFormat)
	{
		case LEVEL_CODE_FORMAT:
		{
			if(HO_QUERY_METHOD_HOME!=pos)
			{
				memcpy(szDoorCode, pmsg->pParam + 2, RD_CODE_LEN);
				if(0==strlen(szDoorCode))
				{
					printf("Error Can not find Code in g_CodeLift.pCodeInfo\n");
					LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
					return;
				}
			}
			else
			{
				memcpy(szSrcUserCode, pmsg->pParam + 2, RD_CODE_LEN);
				//pUserSrc=GetpUserByCode( szUserCode,"");
				//szUserCode[0]='\0';
				szDoorCode[0]='\0';
			}
			
			printf("szDoorCode=%s\n",szDoorCode);
			offset = 2 + RD_CODE_LEN;
			break;	
		}			
		default:
			break;		
	}


	ucDataFormat = *(pmsg->pParam + offset);
	offset += 1;
	switch(ucDataFormat)
	{
		case LEVEL_CODE_FORMAT:
		{
			memcpy(szUserCode, pmsg->pParam + offset, RD_CODE_LEN);
			if(HO_QUERY_METHOD_HOME==pos)
			{
				MOX2_SendHV2HVMsg(pmsg,szSrcUserCode,szUserCode);
				return;
			}
			break;	
		}
		case LEVEL_CSN_FORMAT:
		{
			memset(CardCSN, 0, CSN_LEN);
			memcpy(CardCSN, pmsg->pParam + offset, CSN_LEN);
			
			pLFCard=FindLFCardByCSN(CardCSN);
			if(pLFCard)
			{
				//*(msgSend.pParam + 6) = pLFCard->byDestFloor;
				printf("pLFCard->Code=%s\n",pLFCard->Code);
				memcpy(szUserCode, pLFCard->Code, RD_CODE_LEN);
				
				if(pLFCard->bAdmin)
				{
					sIdentity|=MOX2_IDENTITY_TYPE_ADMIN;
				}
				
			}
			else
			{
				LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
				return;
			}	
			break;
		}
		default:
			break;		
	}
	pUser=GetpUserByCode( szUserCode,szDoorCode);

	if(NULL==pUser)
	{
		LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
		return;
	}			
	pLift=GetpLiftByLiftID(pUser->cLiftID);
	if(NULL==pLift)
	{
		LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
		return;
	}
	pDoor=GetpDoorByCode(szDoorCode);	
	if(NULL==pDoor)
	{
		LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
		return;
	}
	msgSend.szDestDev[0]=pLift->cDA1;
	msgSend.szDestDev[1]=pLift->cDA2;
	msgSend.szDestDev[2]=pLift->cDA3;
	

	switch (pos)
	{

		case HO_QUERY_METHOD_CSN:
		case HO_QUERY_METHOD_RDCODE://Èó®Á¶Å
	
		{
			msgSend.dwMsg=FC_LF_A_B;
			break;
		}
		case HO_QUERY_METHOD_CALLCODE:
		{
			sIdentity|=MOX2_IDENTITY_TYPE_GUEST;
			msgSend.dwMsg=FC_LF_A_B;
			break;
		}
		default:
		{
			
			printf("%s Error Line=%d\n",__FUNCTION__,__LINE__);
			break;
		}
	}
	

	msgSend.wDataLen=13-4;
	msgSend.pParam = malloc(msgSend.wDataLen);
	
	if (msgSend.pParam == NULL)
	{
		printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
		return ;
	}
	pData=msgSend.pParam;
	*pData=MOX2_LOCATION_TYPE_GM;
	pData++;
	
	*pData=pLift->cGroup;
	pData++;
	*pData=MOX2_LIFT_INDEX_BROADCAST;
	pData++;
	*pData=sIdentity>>8;
	pData++;
	*pData=sIdentity & 0xff;
	pData++;
	*pData=pDoor->cLevel;
	pData++;
	*pData=pDoor->cDoorType;
	pData++;
	*pData=pUser->cLevel;
	pData++;
	*pData=pUser->cDoorType;
	pData++;
	MxPutMsg(&msgSend);
	
	/*printf("LCA send the data:\n");
	for (offset = 0; offset < msgSend.wDataLen; offset++)
	{
		printf("  %x ", *(msgSend.pParam + offset));
	}
	printf("\n");*/
	
}

static void MOX2_SendHV2HVMsg(MXMSG* pmsg,char * szSrcUserCode,char * szDestUsetCode)
{
	MXMSG  msgSend;
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V2;
	msgSend.dwMsg=MXMSG_MOX2LIFT_HV2HV;	
	msgSend.pParam = malloc(RD_CODE_LEN*2);	
	msgSend.dwParam = pmsg->dwParam;
	if (msgSend.pParam == NULL)
	{
		printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
		return ;
	}
	memcpy(msgSend.pParam, szSrcUserCode, RD_CODE_LEN);
	memcpy(msgSend.pParam+RD_CODE_LEN, szDestUsetCode, RD_CODE_LEN);
	MxPutMsg(&msgSend);
}

static void
MOX2_CallLiftStop(MXMSG* pmsg)
{
	MXMSG  msgSend;
	EthResolvInfo 	ResolvInfo;
	DWORD dwSrcIP;
	//CodeInfo * pCode=NULL;
	
	MOXUSER	*	pUser;
	unsigned char*	pData;
	MOXLIFT	*	pLift;
//	MOXDOOR	*	pDoor;
	unsigned short sIdentity=0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V2;
	memcpy(&dwSrcIP,pmsg->pParam,4);
	msgSend.dwMsg		= pmsg->dwMsg;
	msgSend.dwParam = pmsg->dwParam;

	sIdentity=0;
	if(strlen(pmsg->szSrcDev))
	{
		pUser=GetpUserByCode( pmsg->szSrcDev,"");
	}
	else
	{
		memset(&ResolvInfo, 0, sizeof(EthResolvInfo));
		ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;
		ResolvInfo.nIP = ChangeIPFormat(dwSrcIP);
		FdFromAMTResolv(&ResolvInfo);
		if(strlen(ResolvInfo.szDevCode)==0)
		{
			return;
		}
		pUser=GetpUserByCode(ResolvInfo.szDevCode,"");
		if(NULL==pUser)
		{
			return;
		}
	}
	pLift=GetpLiftByLiftID(pUser->cLiftID);
	
	msgSend.szDestDev[0]=pLift->cDA1;
	msgSend.szDestDev[1]=pLift->cDA2;
	msgSend.szDestDev[2]=pLift->cDA3;
	
	if(pUser!=NULL && pLift!=NULL)
	{
	
			msgSend.wDataLen=11-4;
			msgSend.pParam = malloc(msgSend.wDataLen);
			
			if (msgSend.pParam == NULL)
			{
				printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
				return ;
			}
			pData=msgSend.pParam;
			*pData=MOX2_LOCATION_TYPE_HV;
			pData++;
			
			*pData=pLift->cGroup;
			pData++;
			*pData=MOX2_LIFT_INDEX_BROADCAST;
			pData++;
			*pData=pUser->cDoorType;
			pData++;
			*pData=sIdentity>>8;
			pData++;
			*pData=sIdentity & 0xff;
			pData++;
			*pData=pUser->cLevel;
			pData++;
			MxPutMsg(&msgSend);
	
	}
}

static void
MOX2_CallLiftUnlock(MXMSG* pmsg)
{
//	EthResolvInfo 	ResolvInfo;
	MXMSG	msgSend;
//	int nDataLen = 0;
//	int offset = 0;
	BYTE CardCSN[CSN_LEN] = { 0 };
	int i = 0;
	int	nFindPos = 0;
	BOOL bFind = FALSE;
	unsigned char cStationNum=0;
	unsigned short sIdentity=0;
	unsigned short sTime=0;
	LFCard * pLFCard=NULL;
	MOXUSER	*	pUser;
	MOXLIFT	*	pLift;
	unsigned char cLiftIndex;
	int j;
	unsigned char*	pData;
	//unsigned char*	pDataLevelCnt;
	unsigned char cLevelCnt=0;
	
	if (pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}

	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_LCA;
	msgSend.dwDestMd	= MXMDID_LT_MOX_V2;

	msgSend.dwMsg	= pmsg->dwMsg;
	msgSend.dwParam = pmsg->dwParam;

	memcpy(CardCSN, pmsg->pParam, CSN_LEN);
	cStationNum=pmsg->pParam[CSN_LEN];
	
	printf("cStationNum=%d\n",cStationNum);
	
	for (i = 0; i < g_LCInfo.nCount; i++)
	{
		if (CSNCompare(CardCSN, g_LCInfo.pLFCard[i].CSN))
		{
			bFind = TRUE;
			nFindPos = i;
			break;
		}
	}
	if (bFind)
	{
	
		pLFCard=&g_LCInfo.pLFCard[nFindPos];
		if(pLFCard->bAdmin)
		{
			sIdentity|=MOX2_IDENTITY_TYPE_ADMIN;
		}
		pUser=GetpUserByCode(  pLFCard->Code,"");
		if(NULL==pUser)
		{
			LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
			return;
		}

		pLift=GetpLiftByCardReaderStationNum(cStationNum,&cLiftIndex);
		if(NULL==pLift)
		{
			LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
			return;
		}
	
		msgSend.szDestDev[0]=pLift->cDA1;
		msgSend.szDestDev[1]=pLift->cDA2;
		msgSend.szDestDev[2]=pLift->cDA3;
		
	
		if(pLFCard->byFrontDoorLen)
		{
			for (j = 0; j < 64; j++)
			{
				if(pLFCard->dlFrontUnlockLevel & ((unsigned long long)1 << j))
				{
					cLevelCnt++;
				}
			}
		}
		if(pLFCard->byBackDoorLen)
		{
			for (j = 0; j < 64; j++)
			{
				if(pLFCard->dlBackUnlockLevel & ((unsigned long long)1 << j))
				{
					cLevelCnt++;
				}
			}
		}
	
		msgSend.wDataLen=12-4+cLevelCnt*2;
		msgSend.pParam = malloc(msgSend.wDataLen);
		
		if (msgSend.pParam == NULL)
		{
			printf("%s: error, the pointer cannot be NULL.\n", __FUNCTION__);
			return ;
		}
		pData=msgSend.pParam;
		*pData=MOX2_LOCATION_TYPE_LIFT_CAR;
		pData++;
		
		*pData=pLift->cGroup;
		pData++;
		*pData=cLiftIndex;
		pData++;
		*pData=sIdentity>>8;
		pData++;
		*pData=sIdentity & 0xff;
		pData++;
		
		*pData=cLevelCnt;
		
		pData++;
		if(pLFCard->byFrontDoorLen)
		{
			for (j = 0; j < 64; j++)
			{
				if(pLFCard->dlFrontUnlockLevel & ((unsigned long long)1 << j))
				{
					*pData=j-8;
					pData++;
					*pData=MOX2_FRONT_DOOR_FLAG;
					pData++;
				}
			}
		}
		if(pLFCard->byBackDoorLen)
		{
			for (j = 0; j < 64; j++)
			{
				if(pLFCard->dlBackUnlockLevel & ((unsigned long long)1 << j))
				{
					*pData=j-8;
					pData++;
					*pData=MOX2_BACK_DOOR_FLAG;
					pData++;
				}
			}
		}
		*pData=sTime>>8;
		pData++;
		*pData=sTime & 0xff;
		pData++;	
	}
	else
	{
		LCASendErrorACK2Target(pmsg->dwMsg, pmsg->dwParam, 0xFF);
		return;
	}

	MxPutMsg(&msgSend);
}
