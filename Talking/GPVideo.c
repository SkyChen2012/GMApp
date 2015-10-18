/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	TalkLogReport.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		01 - Sep - 2008
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
#include "TalkLogReport.h"
#include "ModuleTalk.h"
#include "GPVideo.h"
#include "IniFile.h"
#include "MenuParaProc.h"


/************** DEFINES **************************************************************/
#define GP_VIDEO_DEBUG

#define GP_VIDEO_STATUS_IDLE		0
#define GP_VIDEO_STATUS_BUSY		1


/************** TYPEDEFS *************************************************************/

typedef	struct _GPConfig
{
	DWORD dwIP;
	int nPort;
	int nIPCnt;
	DWORD dwIPList[MAX_GPFILE_IP_CNT];
} GPConfig;

/************** STRUCTURES ***********************************************************/
typedef	struct _GPVIDEOInfo
{
	int nStatus;
	GPConfig GPData;
} GPVIDEOINFO;

/************** EXTERNAL DECLARATIONS ************************************************/


//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/
static void SendDOON2IOCtrl(void);
static void SendDOOFF2IOCtrl(void);
static void GMSendStartVideo2GP(void);
static void GMSendStopVideo2GP(void);
static void LoadGPConfig(void);
static BOOL CheckIPInGPConfig(DWORD dwIP);

static GPVIDEOINFO gGPVideo;
/*************************************************************************************/



	
	
void InitGPVideo(void)
{
#ifdef	GP_VIDEO_DEBUG
	printf("### GPVideo ### %s \n",__FUNCTION__ );
#endif
	memset(&gGPVideo,0,sizeof(GPVIDEOINFO));
	LoadGPConfig();
}



static void LoadGPConfig(void)
{

	const char *pStr = NULL;
	int i;
	char szSec[20] = {0};


	OpenIniFile(GP_CONFIG_FILE);
	pStr = ReadString(GP_CONFIG_SECTION, KEY_GPFILE_GP_IP, VALUE_GPFILE_GP_IP);
	gGPVideo.GPData.dwIP= IP_Str2DWORD((CHAR *)pStr);
	gGPVideo.GPData.nPort= ReadInt(GP_CONFIG_SECTION, KEY_GPFILE_GP_PORT, VALUE_GPFILE_GP_PORT);
	gGPVideo.GPData.nIPCnt= ReadInt(GP_CONFIG_SECTION, KEY_GPFILE_IP_Cnt, VALUE_GPFILE_IP_Cnt);
	
	

	if(gGPVideo.GPData.nIPCnt>MAX_GPFILE_IP_CNT)
	{
			printf("%s error gGPVideo.GPData.nIPCnt=%d\n",__FUNCTION__,gGPVideo.GPData.nIPCnt);
			return;
	}
	for(i=0;i<gGPVideo.GPData.nIPCnt;i++)
	{
		sprintf(szSec,"IP%d", i + 1);
		pStr = ReadString(GP_CONFIG_SECTION, szSec, "");
		gGPVideo.GPData.dwIPList[i]=IP_Str2DWORD((CHAR *)pStr);	
	}
	CloseIniFile();
}
static BOOL CheckIPInGPConfig(DWORD dwIP)
{
	int i;
	for(i=0;i<gGPVideo.GPData.nIPCnt;i++)
	{
		if(dwIP==gGPVideo.GPData.dwIPList[i])
		{
			return TRUE;
		}
	}
	return FALSE;
}


void StartGPVideo(unsigned long nSrcIPAddr)
{
#ifdef	GP_VIDEO_DEBUG
	printf("### GPVideo ### %s \n",__FUNCTION__ );
#endif
if(GP_VIDEO_STATUS_IDLE==gGPVideo.nStatus && CheckIPInGPConfig(nSrcIPAddr))
	{
		printf("SendDOON2IOCtrl\n");
		SendDOON2IOCtrl();
		gGPVideo.nStatus=GP_VIDEO_STATUS_BUSY;
		GMSendStartVideo2GP();
	}
}


void StopGPVideo(void)
{
#ifdef	GP_VIDEO_DEBUG
	printf("### GPVideo ### %s \n",__FUNCTION__ );
#endif
	if(GP_VIDEO_STATUS_BUSY==gGPVideo.nStatus)
	{
		SendDOOFF2IOCtrl();
		gGPVideo.nStatus=GP_VIDEO_STATUS_IDLE;
		GMSendStopVideo2GP();
	}
}

int CheckGPVideoCMD(unsigned short nFunCode,unsigned long nSrcIPAddr)
{	
	if(GP_VIDEO_STATUS_BUSY==gGPVideo.nStatus)
	{
		if(nSrcIPAddr==GetGPIP())
		{
			if(FC_AV_SENDDATA==nFunCode ||
			FC_ACK_AV_TAKEPHOTO==nFunCode ||
			FC_ACK_AV_REQUEST_DECARGS==nFunCode)
			{
				return CHECK_GP_RESULT_GP2DEST;
			}
		
		}
		else
		{
			if(
			FC_AV_REQUEST_DECARGS==nFunCode ||
			FC_AV_REQUEST_VIDEO_IVOP==nFunCode ||
			FC_AV_TAKEPHOTO==nFunCode ||
			FC_AV_STREAM_ANNOUNCEMENT==nFunCode ||
			FC_AV_STREAM_REQUEST==nFunCode)
			{
				return CHECK_GP_RESULT_DEST2GP;
			}
		}
	}
	else
	{
		if(nSrcIPAddr==GetGPIP())
		{
			GMSendStopVideo2GP();
		}	
		return CHECK_GP_RESULT_ERROR;
	}
}
BOOL IsUseGPVideo(void)
{
	if(GP_VIDEO_STATUS_BUSY==gGPVideo.nStatus)
	{
		return TRUE;
	}
	return FALSE;
}

static void 
SendDOON2IOCtrl(void)
{
	MXMSG  msgSend;
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_IOCTRL;
	msgSend.dwMsg		= COMM_GP_VIDEO_ON;
	msgSend.dwParam		= 0;
	msgSend.pParam		= NULL;
	MxPutMsg(&msgSend);
}

static void 
SendDOOFF2IOCtrl(void)
{
	MXMSG  msgSend;
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_IOCTRL;
	msgSend.dwMsg		= COMM_GP_VIDEO_OFF;
	msgSend.dwParam		= 0;
	msgSend.pParam		= NULL;
	MxPutMsg(&msgSend);
}





static void
GMSendStartVideo2GP(void)
{
	MXMSG	msgSend ;
	unsigned char *	pData =NULL;

	short	nLen = 8;

	DWORD	LocalHVIP=ntohl(GetSelfIP());
	
	memset(&msgSend, 0, sizeof(MXMSG));

	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_AV_START;
	msgSend.dwParam		= GetGPIP();//GP IP

	msgSend.pParam		=  malloc(nLen+2);
	pData=msgSend.pParam;
	memcpy(pData,&nLen,2);
	pData+=2;
	*pData=0x01;//Type
	pData++;
	*pData=0x01;//Action
	pData++;
	*pData=gGPVideo.GPData.nPort;//Video Port
	pData++;
	memcpy(pData,&LocalHVIP,sizeof(DWORD));//IP
	
	pData+=4;
	*pData=5;//DevType
	pData++;
	MxPutMsg(&msgSend);	

}
static void
GMSendStopVideo2GP(void)
{
	MXMSG	msgSend ;
	short	nLen = 2;
	memset(&msgSend, 0, sizeof(MXMSG));
	//strcpy(msgSend.szSrcDev, g_TalkInfo.szTalkDevCode);
	//strcpy(msgSend.szDestDev, g_TalkInfo.monitor.szMonDestDevCode);
	msgSend.dwSrcMd		= MXMDID_TALKING;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_AV_STOP;
	msgSend.dwParam		= GetGPIP();

	msgSend.pParam		=  malloc(nLen+2);
	memcpy(msgSend.pParam,&nLen,2);
	msgSend.pParam[2] = 0x01;
	msgSend.pParam[3] = 0x01;

	MxPutMsg(&msgSend);	

}

DWORD GetGPIP(void)
{
	//return 0xAC106506;
	return gGPVideo.GPData.dwIP;
}

