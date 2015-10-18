/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SystemInfoAPI.c
**
**	AUTHOR:		Harry	Qian
**
**	DATE:		16 - Jun - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				
**				MXListAdd	
**				MXListRm
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
#include <MoxFB.h>
#include <device.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMdId.h"
#include "MXMsg.h"
#include "MXTypes.h"
#include "MXList.h"
#include "MXMem.h"
#include "Dispatch.h"

#include "SystemInfoAPI.h"
#include "ModuleTalk.h"
#ifdef __SUPPORT_PROTOCOL_900_1A__
#include "MenuParaProc.h"
#include "ParaSetting.h"
#endif
/************** DEFINES **************************************************************/
#define SYSINFO_DEBUG
//#define FREE_MEM_DEBUG

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

typedef	struct _SYSINFOFUNC
{
	DWORD							dwDestMd;
	DWORD							dwMsg;

} SysInfoFunc;
/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

SysInfoFunc		g_SysFunc;

static void	AnswerFuncInfoQuery(MXMSG * pmsg);
static void	AnswerSystemInfoQuery(MXMSG *  pmsg);
static void	GetModuleSysInfo(MXMSG *pmsg);
static void	GetModuleFuncInfo(MXMSG * pmsg);
static void AnswerSystemInfoQueryEx(MXMSG *  pmsg); // for 900.1a FCA_CG_READ_GM_PARA
static void AnswerSystemInfoSetEx(MXMSG *  pmsg); // for 900.1a FCA_CG_WRITE_GM_PARA
static BYTE DOReversal(BYTE arg);


/*************************************************************************************/

/**
 * 上位机 DO 1: 常闭 0: 常开
 * 门口机 DO 1: 常开 0: 常闭
 * 需要进行翻转	
 *
 * @param arg DO 状态
 *
 * @return  翻转后的DO状态
 */
BYTE DOReversal(BYTE arg)//bug 14824
{
	if(1 == arg)
		{
		return 0;
	}
	if(0 == arg)
		{
		return 1;
	}
	return arg;
}


void
PublicFuncMdInit(void)
{
	DpcAddMd(MXMDID_PUBLIC, NULL);
}


void
PublicFuncMdExit(void)
{
	DpcRmMd(MXMDID_PUBLIC);
}

void
PublicFuncProc(void)
{
	MXMSG	msgRecev;
	
	memset(&msgRecev, 0, sizeof(msgRecev));
	msgRecev.dwDestMd	= MXMDID_PUBLIC;
	msgRecev.pParam		= NULL;
	
	if (MxGetMsg(&msgRecev))
	{
		switch(msgRecev.dwMsg)
		{
			
		case FC_GET_SYSINFO:
#ifdef SYSINFO_DEBUG
			printf("Get FC_GET_SYSINFO command form ip=%x\n", msgRecev.dwParam);
#endif
			AnswerSystemInfoQuery(&msgRecev);
			break;
		case FC_ACK_GET_SYSINFO:
#ifdef SYSINFO_DEBUG
			printf("Get FC_ACK_GET_SYSINFO command form ip=%x\n", msgRecev.dwParam);
#endif
			GetModuleSysInfo(&msgRecev);
			break;
		case FC_GET_FUNCINFO:
#ifdef SYSINFO_DEBUG
			printf("Get FC_GET_FUNCINFO command form ip=%x\n", msgRecev.dwParam);
#endif
			AnswerFuncInfoQuery(&msgRecev);
			break;

			
		case FC_ACK_GET_FUNCINFO:
#ifdef SYSINFO_DEBUG
			printf("Get FC_ACK_GET_FUNCINFO command form ip=%x\n", msgRecev.dwParam);
#endif
			GetModuleFuncInfo(&msgRecev);
			break;
#ifdef __SUPPORT_PROTOCOL_900_1A__
		case FCA_CG_READ_GM_PARA:
#ifdef SYSINFO_DEBUG
			printf("Get FCA_CG_READ_GM_PARA command form ip=%x\n", msgRecev.dwParam);
#endif
			AnswerSystemInfoQueryEx(&msgRecev);
			break;

			
		case FCA_CG_WRITE_GM_PARA:
#ifdef SYSINFO_DEBUG			
			printf("Get FCA_CG_WRITE_GM_PARA command form ip=%x\n", msgRecev.dwParam);
#endif
			AnswerSystemInfoSetEx(&msgRecev);
			break;
#endif
		default:
			break;
		}

		DoClearResource(&msgRecev);
	}

}


void
SendFuncInfoQuery(DWORD dwIP, DWORD dwFunID)
{
	MXMSG	msgSend;

	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_PUBLIC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_GET_FUNCINFO;
	msgSend.dwParam		= dwIP;
	msgSend.pParam		= malloc(6);

	*(unsigned short *)msgSend.pParam = 4;

	memcpy(msgSend.pParam + 2, &dwFunID, 4);
#ifdef SYSINFO_DEBUG
	printf("query ip=%x the funcid=%d.\n", dwIP, dwFunID);
#endif
	MxPutMsg(&msgSend);

}



static void
AnswerFuncInfoQuery(MXMSG * pmsg)
{
	MXMSG	msgSend;
	DWORD	dwFuncID = *((DWORD *)&pmsg->pParam[0]);
	unsigned	short nLen = 0;

	memcpy(&dwFuncID, &pmsg->pParam[0], 4);

	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_PUBLIC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_GET_FUNCINFO;
	msgSend.dwParam		= pmsg->dwParam;


	if (dwFuncID == MDFUNC_CALL)
	{
		msgSend.pParam		= malloc(FUNCQUERY_TALK_LEN);
		*(unsigned short *)msgSend.pParam = 19;
		
		msgSend.pParam[2] = 0;

		memcpy(msgSend.pParam + 3, &dwFuncID, 4);
		nLen = 12;
		memcpy(msgSend.pParam + 7, &nLen, 2);

		msgSend.pParam[9] = 1;// version
		msgSend.pParam[10] = 3; //both be called in or launch call
		msgSend.pParam[11] = 3; //both capture and play audio
		msgSend.pParam[12] = AV_TYPE_AUDIO;
		msgSend.pParam[13] = AUDIO_SPEED_8;
		msgSend.pParam[14] = AUDIO_TYPE_WAV;
		msgSend.pParam[15] = 0;

		msgSend.pParam[16] = 1; // Just capture video
		msgSend.pParam[17] = AV_TYPE_VIDEO;
		msgSend.pParam[18] = VIDEO_FRAME_30;
		msgSend.pParam[19] = VIDEO_MODE_MPEG4_PAL;
		msgSend.pParam[20] = 0;

	}
	else if (dwFuncID == MDFUNC_MONITOER)
	{
		msgSend.pParam		= malloc(FUNCQUERY_MON_lEN);
		*(unsigned short *)msgSend.pParam = 19;
		
		msgSend.pParam[2] = 0;
		
		memcpy(msgSend.pParam + 3, &dwFuncID, 4);
		nLen = 12;
		memcpy(msgSend.pParam + 7, &nLen, 2);
		
		msgSend.pParam[9] = 1;// version
		msgSend.pParam[10] = 1;// can launch monitor
		msgSend.pParam[11] = 3; //both capture and play audio
		msgSend.pParam[12] = AV_TYPE_AUDIO;
		msgSend.pParam[13] = AUDIO_SPEED_8;
		msgSend.pParam[14] = AUDIO_TYPE_WAV;
		msgSend.pParam[15] = 0;

		msgSend.pParam[16] = 1;
		msgSend.pParam[17] = AV_TYPE_VIDEO;
		msgSend.pParam[18] = VIDEO_FRAME_30;
		msgSend.pParam[19] = VIDEO_MODE_MPEG4_PAL;
		msgSend.pParam[20] = 0;
	}
	else if (MDFUNC_IRISMSG == dwFuncID)
	{
		msgSend.pParam		= malloc(FUNCQUERY_IRISMSG_LEN);
		*(unsigned short *)msgSend.pParam = 19;
		
		msgSend.pParam[2] = 0;
		
		memcpy(msgSend.pParam + 3, &dwFuncID, 4);
		nLen = 12;
		memcpy(msgSend.pParam + 7, &nLen, 2);
		
		msgSend.pParam[9] = 1;// version	
		msgSend.pParam[10] = 1;	
	}

	MxPutMsg(&msgSend);
}




/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MXAlloc
**	AUTHOR:			Harry Qian
**	DATE:			13 - Sep - 2007
**
**	DESCRIPTION:	
**			alloc memory
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nSize		[IN]		unsigned int
**	RETURNED VALUE:	
**				memory pointer that alloced
**	NOTES:
**			
*/
void
SendSystemInfoQuery(DWORD dwIP)
{
	MXMSG	msgSend;
	
	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_PUBLIC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_GET_SYSINFO;
	msgSend.dwParam		= dwIP;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
}

static void
AnswerSystemInfoQuery(MXMSG *  pmsg)
{
/*	MXMSG	msgSend;
	const char*	pSoftVer;
	char*	pResult = NULL;	
	BYTE	bLen = 0;
	BYTE	bHW_PNLen = 0;
	BYTE	bSW_PNLen = 0;
	DWORD	dwFunc = 0;
	OpenIniFile(TARGET_CONFIG);
	
	pSoftVer = ReadString(TARGET_CONFIG_SEC_GLOBAL, TARGET_CONFIG_KEY_VERSION, TARGET_CONFIG_VALUE_VERSION);

	CloseIniFile();

	if (!OpenIniFile(HARDWARE_CONFIG))
	{
		printf("Read hardware.ini error\n");
		return;
	}
	
	pResult = ReadString(HARDWARE_CONFIG_SEC_GLOBAL, HARDWARE_CONFIG_KEY_PN, KEY_PN_VALUE_DEFAULT);

	CloseIniFile();	

	bLen = 17 + sizeof(pSoftVer) + sizeof(pResult);
	

	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd		= MXMDID_PUBLIC;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_GET_SYSINFO;
	msgSend.dwParam		= pmsg->dwParam;
	msgSend.pParam		= malloc(bLen);

	memset(msgSend.pParam, 0, bLen);

	*(unsigned short *)msgSend.pParam = bLen - 2;
	msgSend.pParam[2] = 0;
	bHW_PNLen = sizeof(pResult);
	msgSend.pParam[3] = bHW_PNLen;
	memcpy(msgSend.pParam + 4, pResult, bHW_PNLen);

	bSW_PNLen = sizeof(pSoftVer);
	msgSend.pParam[4 + bHW_PNLen] = bSW_PNLen;
	memcpy(msgSend.pParam + 5 + bHW_PNLen, pSoftVer, bSW_PNLen);

	dwFunc = MDFUNC_IRISMSG | MDFUNC_ALARM | MDFUNC_CALL | MDFUNC_MONITOER;

	memcpy(msgSend.pParam + 13 + bHW_PNLen + bSW_PNLen, &dwFunc, sizeof(DWORD));
	
	MxPutMsg(&msgSend);*/
	
}

static void
GetModuleSysInfo(MXMSG *pmsg)
{
;

}

static void
GetModuleFuncInfo(MXMSG * pmsg)
{
	//unsigned short nLen = 0;
	DWORD	dwFuncID = 0;
	BYTE	bResult =0 ;
	PBYTE	pData = pmsg->pParam;
	//nLen = *(unsigned short *)pData;
	
	pData += 2;
	bResult = *pData;
	if (bResult)
	{
		return;
	}

	pData += 1;
	memcpy(&dwFuncID, pData, sizeof(DWORD));
	pData += 4;
	switch(dwFuncID)
	{
	case MDFUNC_IRISMSG:
	//	GetMdMsgInfo(pData);
		break;
	case MDFUNC_CALL:
		GetMdTalkInfo(pData);
		break;
	case MDFUNC_MONITOER:
		//GetMdMoniterInfo(pData);
		break;
	default:
		break;
	}
}

#ifdef __SUPPORT_PROTOCOL_900_1A__
#pragma pack(1)
typedef struct _tag_protocol_900_1A_gm_parameter
{
	WORD	DataLen;
	BYTE 	Version;
	DWORD 	IPAddr;
	DWORD 	Mask;
	DWORD 	ServerIP;
	BYTE	CameraMode;
	BYTE	EnTamperAlarm;
	BYTE	TalkTime;
	BYTE	WiegandBit;
	BYTE	WiegandRev;
	BYTE	DI1Mode;
	BYTE	DI1Fun;
	BYTE	DI2Mode;
	BYTE	DI2Fun;
	BYTE	DO1Mode;
	BYTE	DO2Mode;
	BYTE	TalkVol;
	BYTE	RingTime;
	CHAR	SysPwd[SYS_PWD_LEN];
}GM_PTL_PARA_t;
#pragma pack()

static void
AnswerSystemInfoQueryEx(MXMSG *  pmsg)
{
	MXMSG	msgSend;
	SYSINFO_NEW_t local;
	GM_PTL_PARA_t out;

	printf("%s,%d pmsg->wDataLen:%d\n",__FUNCTION__,__LINE__,pmsg->wDataLen);
	
	memset(&out, 0, sizeof(GM_PTL_PARA_t));
	GetNewSysConfig (&local);
	out.DataLen 		= sizeof(GM_PTL_PARA_t) - sizeof(out.DataLen);
	out.Version 		= (BYTE)local.Version;
	out.IPAddr 			= local.IPAddr;
	out.Mask 			= local.Mask;
	out.ServerIP 		= local.ServerIP;
	out.CameraMode		= local.CameraMode;
	out.EnTamperAlarm 	= local.EnTamperAlarm;
	out.TalkTime 		= local.TalkTime;
	out.WiegandBit 		= local.EnWiegand == TRUE ? local.WiegandBit : 0;
	out.WiegandRev 		= local.WiegandRev;
	out.DI1Mode 		= local.DI1Mode;
	out.DI1Fun 			= local.DI1Fun;
	out.DI2Mode 		= local.DI2Mode;
	out.DI2Fun 			= local.DI2Fun;
	out.DO1Mode 		= DOReversal(local.DO1Mode);
	out.DO2Mode 		= DOReversal(local.DO2Mode);
	out.TalkVol 		= local.TalkVol;
	out.RingTime 		= local.RingTime;
	strcpy(out.SysPwd, local.SysPwd);

	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd     = MXMDID_PUBLIC;
    msgSend.dwDestMd    = MXMDID_ETH;
    msgSend.dwParam     = pmsg->dwParam;
    msgSend.dwMsg       = FCA_ACK_CG_READ_GM_PARA;
	msgSend.wDataLen 	= sizeof(GM_PTL_PARA_t);
	msgSend.pParam 		= (UCHAR*)malloc(msgSend.wDataLen);
	memcpy(msgSend.pParam, &out, msgSend.wDataLen);

	MxPutMsg(&msgSend);	
}

static void 
AnswerSystemInfoSetEx(MXMSG *  pmsg)
{
	MXMSG	msgSend;
	GM_PTL_PARA_t in;
	SYSINFO_NEW_t local;
	UCHAR	ret = 0; // 0 Success 1 Failure

	printf("%s,%d pmsg->wDataLen:%d\n",__FUNCTION__,__LINE__,pmsg->wDataLen);

	if(pmsg->wDataLen == sizeof(GM_PTL_PARA_t))
	{
		memset(&local, 0, sizeof(SYSINFO_NEW_t));
		memset(&in, 0, sizeof(GM_PTL_PARA_t));
		memcpy(&in, pmsg->pParam, pmsg->wDataLen);
		local.Version 		= (BYTE)in.Version;
		local.IPAddr 		= in.IPAddr;
		local.Mask 			= in.Mask;
		local.ServerIP 		= in.ServerIP;
		local.CameraMode 	= in.CameraMode;
		local.EnTamperAlarm = in.EnTamperAlarm;
		local.TalkTime 		= in.TalkTime;
		local.WiegandBit 	= in.WiegandBit;
		local.WiegandRev 	= in.WiegandRev;
		local.DI1Mode 		= in.DI1Mode;
		local.DI1Fun 		= in.DI1Fun;
		local.DI2Mode 		= in.DI2Mode;
		local.DI2Fun 		= in.DI2Fun;
		local.DO1Mode 		= DOReversal(in.DO1Mode);
		local.DO2Mode 		= DOReversal(in.DO2Mode);
		local.TalkVol 		= in.TalkVol;
		local.RingTime 		= in.RingTime;
		strcpy(local.SysPwd, in.SysPwd);
		SetNewSysConfig (local);
	}
	else
	{
		ret = 1;
	}

	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd     = MXMDID_PUBLIC;
    msgSend.dwDestMd    = MXMDID_ETH;
    msgSend.dwParam     = pmsg->dwParam;
    msgSend.dwMsg       = FCA_ACK_CG_WRITE_GM_PARA;
	msgSend.wDataLen 	= 1;
	msgSend.pParam 		= (UCHAR*)malloc(msgSend.wDataLen);
	msgSend.pParam[0] 	= ret;
	
	MxPutMsg(&msgSend);

	if(MOX_PROTOCOL_SUPPORT(MOX_P_900_1A))//bug 14815
	{		
		g_ASPara.bEnableAS = FALSE;
		SaveMenuPara2Mem();
	}
}

#endif


