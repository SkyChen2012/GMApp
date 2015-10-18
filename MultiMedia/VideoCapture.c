/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Multimedia.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		18 - Oct - 2006
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

/************** TEM INCLUDE FILES **************************************************/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMsg.h"
#include "MXTypes.h"
#include "MXCommon.h"
#include "Multimedia.h"
#include "Eth.h"
#include "PioApi.h"
#include "vpu_voip_app.h"
#include "JpegApp.h"
#include "DiagTelnetd.h"
#include "VideoCapture.h"
#include "SysLogProc.h"
#include "ModuleTalk.h"
/************** DEFINES **************************************************************/
#define CHECK_VIDEO_SEND

#define CHECK_VIDEO_SEND_TIMEOUT	(5*1000)

#define RESTARTFIFOFILE	 	"/mox/rdwr/RestartFifo"
/************** TYPEDEFS *************************************************************/



/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static pthread_t	MmVthWork;
static BOOL			bMmVVthQuit = FALSE;

//static BOOL         bEnCapVideo	=	TRUE;
pthread_mutex_t		MutexCap;
pthread_mutex_t		MutexFrm;

void	VideoWorkThreadFun(void* arg);

static BYTE VCapBuf[MAX_VIDEOCAP_DATA_SIZE];
static INT  VCapBufLen;
static BOOL	bVideoWork=FALSE;

static DWORD	dwCheckVideoTick=0;
static DWORD	dwLastVideoSendFrm=0;

static void CheckVideoSendProc(void);

static BOOL bGMReboot=FALSE;
static void CheckGMReboot(void);
/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmVideoInit
**	AUTHOR:			Jeff Wang
**	DATE:			17 - Jun - 2009
**
**	DESCRIPTION:	
**			Multi-Media Video init
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void 
MmVideoInit()
{
	if ((pthread_create(&MmVthWork, NULL, VideoWorkThreadFun, NULL)) != 0)	
	{
		printf("MmV: create thread fail\n");
	}
	bMmVVthQuit = FALSE;

	pthread_mutex_init(&MutexCap, NULL);
	pthread_mutex_init(&MutexFrm, NULL);

	printf("MmV: Initialize ...\n");
}
void 
MmVideoStart(void)
{
	bVideoWork=TRUE;
	dwCheckVideoTick=GetTickCount();
	dwLastVideoSendFrm=dwDiag[DIAG_V_FRM_SENT];
}
void 
MmVideoStop(void)
{
	bVideoWork=FALSE;
}

static void CheckVideoSendProc(void)
{
	if(bVideoWork)
	{
		if(dwLastVideoSendFrm!=dwDiag[DIAG_V_FRM_SENT])
		{
			dwCheckVideoTick=GetTickCount();
			dwLastVideoSendFrm=dwDiag[DIAG_V_FRM_SENT];
		}
		if((GetTickCount()-dwCheckVideoTick)>CHECK_VIDEO_SEND_TIMEOUT)
		{            
            if(!bGMReboot)
            {
                /*only excuted one time when timeout happened*/
			    EthGMLog(TYPE_VIDEO_SEND_ERROR);
            }
			//system("reboot");
			bGMReboot=TRUE;
		}
	}
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MultimediaExit
**	AUTHOR:			Jerry Huang
**	DATE:			30 - Oct - 2006
**
**	DESCRIPTION:	
**			Multi-Media module exit
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void 
MmVideoExit()
{
	pthread_mutex_destroy(&MutexCap);
	pthread_mutex_destroy(&MutexFrm);
	bMmVVthQuit = TRUE;
	pthread_exit(0);	
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	VideoWorkThreadFun
**	AUTHOR:			Jeff Wang
**	DATE:			17 - Jun - 2009
**
**	DESCRIPTION:	
**			Video Capture work thread function
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				arg			[IN]		void*
**	RETURNED VALUE:	
**				0
**	NOTES:
**			
*/
void
VideoWorkThreadFun(void* arg)
{
    /*
        当视频工作线程退出的时候，bMmVVthQuit才会被置为真
    */
	while (!bMmVVthQuit)
	{
        /*
            当通讯不再需要视频的时候，bVideoWork被置为假
        */
		if(bVideoWork)
		{
            /*
                MAX_VIDEOCAP_DATA_SIZE限定了VCapBuf的容量，防止内存越界
                VCapBufLen为接收到的视频数据的长度
                VCapBuf盛放接收的视频数据
            */
			VCapBufLen = ReadPicRawData(VCapBuf, MAX_VIDEOCAP_DATA_SIZE);
			if(VCapBufLen)
			{
				PutCapData2Buf();
			}
#ifdef CHECK_VIDEO_SEND
			CheckVideoSendProc();
#endif
		}
        /*
            休眠1ms
        */
		usleep(1000);
		CheckGMReboot();
	}
	MmVideoExit();
}
static void CheckGMReboot(void)
{
    
	if((ST_ORIGINAL==GetMonitorState()) && (ST_ORIGINAL==GetTalkState()) && (TRUE==bGMReboot))
	{
		//mkfifo( RESTARTFIFOFILE, 0777 );
#ifdef VIDEO_CAPTURE_LOG_DEBUG				
		EthGMLog_videolog();
#endif
		system("reboot");
	}
	
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DisableVCap
**	AUTHOR:			Jeff Wang
**	DATE:			17 - Jun - 2009
**
**	DESCRIPTION:	
**			Disable Video Capture
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**	
**	RETURNED VALUE:	
**				0
**	NOTES:
**			
*/
INT
GetVCapData(BYTE* pOutData)
{
	INT nRetLen = 0;

	pthread_mutex_lock(&MutexCap);

	if (VCapBufLen > 0) 
	{
		memcpy(pOutData, VCapBuf, VCapBufLen);
		nRetLen		=	VCapBufLen;
		memset(VCapBuf, 0, VCapBufLen);
		VCapBufLen	=	0;
	}
	pthread_mutex_unlock(&MutexCap);

	return nRetLen;
}
