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
#include "Eth.h"
#include "TalkLogReport.h"
#include "PioApi.h"
#include "vpu_voip_app.h"
#include "JpegApp.h"
#include "Bmp.h"
#include "DiagTelnetd.h"
#include "VideoCapture.h"
#include "SysLogProc.h"
#include "FM1182.h"
#include "MenuParaProc.h"
#include "GPVideo.h"
#include "testmod_ssi.h"


/************** DEFINES **************************************************************/

#define CALL_NOTE_DEBUG
//#define	LEAVE_PHOTO_DEBUG

//#define	MM_VIDEO_DATA_SIZE				(120 * 1024)
//#define	MM_VIDEO_DATA_SIZE				(200 * 1024)

#define	MM_AUDIO_DATA_SIZE				(10 * 1024)

#define	MM_VIDEO__ARRAY_CNT			1
#define	MM_AUDIO__ARRAY_CNT			10

#define	AUDIO_PLAY_PER_LEN				1000
#define	AUDIO_PLAY_PER_PKT				4
#define	AUDIO_PLAY_INTERVAL				25 * (AUDIO_PLAY_PER_PKT) 	// ms

#define	AV_PLAY_STATUS_IDLE				0
#define	AV_PLAY_STATUS_RUN				1
#define	AV_PLAY_STATUS_PAUSE			2
#define AV_PLAY_STATUS_CAPTURE			3
#define AV_PLAY_STATUS_PLAY				4
#define AV_PLAY_STATUS_LEAVEPHTO		5
#define AV_STATUS_EMSG_SEND				6

#define AUDIO_VALUE_MAX					100

#define AV_DATA_TYPE_AUDIO			1
#define AV_DATA_TYPE_VIDEO			2

#define LEAVEPHOTOTIMEOUT			3000

#define SSI_IOCTL             				0x54

#define IOCTL_SSI_GET_PLAYBACK_CALLBACKS				0x0
#define IOCTL_SSI_GET_STATUS			0x1
#define IOCTL_SSI_CONFIGURE_AUDMUX			0x2
#define IOCTL_SSI_CONFIGURE_MC13783			0x3
#define IOCTL_SSI_CONFIGURE_MC13783_MIX			0x4
#define IOCTL_SSI_DMA_TRIGGER			0x5
#define IOCTL_SSI_DMA_STOP 0x6
#define IOCTL_SSI_GET_CAPTURE_CALLBACKS				0xb
#define IOCTL_SSI_RESET				0xc

#define DEVICE_NAME_SSI1				"/dev/mxc_ssi1"
#define DEVICE_NAME_SSI2				"/dev/mxc_ssi2"

#define MOX_GET_KEY_STATUS	0xf

//#define SAVE_RAW_VIDEODATA

//#define MM_WR_SND_V_DEBUG

struct key_run_status{
	unsigned long read_times;
	unsigned long key_times;
};


#define STEREO_DAC					0x0
#define CODEC						0x1

#define MAX_CHUNK_SIZE				1000

#define MM_TIMER					8
#define NO_TIME_LIMIT				-1

#define FILE_CALL_RING				   "/mox/Sounds/Ring.pcm"
#define FILE_ARREARS_RING				  "/mox/Sounds/Arrears.pcm"
#define FILE_KEY_SOUND				 "/mox/Sounds/Prompt.pcm"
#define FILE_ERROR_SOUND		"/mox/Sounds/Error.pcm"
#define FILE_RIGHT_SOUND		  "/mox/Sounds/Right.pcm"
#define FILE_ALARM_SOUND		 "/mox/Sounds/Alarm.pcm"

#define FILE_DI1_SOUND		 "/mox/rdwr/DI1.pcm"
#define FILE_DI2_SOUND		 "/mox/rdwr/DI2.pcm"

#define CCD_PWR_ON		 1
#define CCD_PWR_OFF		0


#define BEEP_CONTROL_ON		 0
#define BEEP_CONTROL_OFF	 1

#define AMP_PWR_ON		 1
#define AMP_PWR_OFF		0


#define BEEP_SHORT_TIME    200
#define BEEP_ERROR_TIME    60
#define BEEP_RIGHT_TIME    200

#define AUDIO_MASK_TIME 500

#define VOLUME_MODE_LOCAL	1	//本地音频（铃声，报警音...）
#define VOLUME_MODE_NET	2	//通话音量

/************** TYPEDEFS *************************************************************/
typedef	enum 
{
	DATA_TYPE_VIDEO = 0,
	DATA_TYPE_AUDIO,
	DATA_TYPE_PHOTO_BMP,
	DATA_TYPE_PHOTO_JPG
} DATATYPE;

typedef	struct _AVCapPlayFlag
{
	BOOL						bCapA2Eth;
	DWORD					nACapStartTick;
	DWORD					nACapBytesCnt;
	BOOL						bCapV2Eth;
	DWORD					nVCapStartTick;
	BOOL						bPlayEthA;
	DWORD					nAPlayStartCnt;
	BOOL						bPlayEthV;
	BOOL						bPhoto2EHV;
	BOOL						bPhoto2MC;
} AVCapPlayFlag;

typedef	struct _AudioPlayMemManStr
{
	unsigned char*				pBuf;
	int							nBufLen;
	int							nCurPlayPos;
	DWORD						dwNextPlayTick;
	int							nLenPerPlay;
	DWORD						dwPlayInterval;
	
	BOOL						bPlayOnce;
	BOOL						bLoopEnd;
} AudioPlayMemManStr;

typedef	struct _AVFrmHead
{
	UINT				nFrmIdx			PACKED;
	UCHAR			nPckIdx			PACKED;
	UCHAR			nTotalPckCnt	PACKED;
	UCHAR			nType	  		PACKED;
	UCHAR			nCapture		PACKED;
	UCHAR			nFormat			PACKED;
	UCHAR			Reserved		PACKED;
	USHORT		   nLen			PACKED;
} AVFrmHead ;

typedef	struct _MmRcvAVData
{
	BOOL						bUsed;
	AVFrmHead					LastAVHead;
	int							nCurLen;
	unsigned char*				pBuf;
	int							nBufLen;
} MmRcvAVData;

typedef	struct _MmSendAVData
{
	AVFrmHead					LastAVHead;
	
	int							nMaxLenPerPack;
	
	int							nCurLen;
	unsigned char*				pBuf;
	int							nBufLen;
} MmSendAVData;

typedef	struct _MmManStruct
{
	pthread_t					thWork;
	BOOL						bthWorkQuit;

	unsigned int				nAVDestIP;
	
	AVCapPlayFlag				StackAVFlag;					
	
	MmRcvAVData					RcvAudio;
	MmRcvAVData					RcvVideo;
	
	MmSendAVData				SendAudio;
	MmSendAVData				SendVideo;
	
	unsigned char*				pBuf;
	int							nBufLen;
	
	int							nPlayPromptStatus;
	int							nPlayCallAStatus;
	int							nPlayAlarmAStatus;
	int							nRdirEthMmAVStatus;

	AudioPlayMemManStr			AudioPlayMemMan;
	
	BOOL						bBeepShort;
	DWORD						dwBeepShortTick;
	BOOL						bBeepRight;
	DWORD						dwBeepRightTick;
	BOOL						bBeepError;
	int                         nBeepErrorCount;

	
} MmManStruct;

typedef	struct _MmAVBuf
{
	MXList						List;
	BOOL						bUsed;
	int							nCurLen;
	unsigned char*				pBuf;
	int							nBufLen;
} MmAVBuf;

typedef	struct _MmAVBufStruct
{
	MXListHead					Head;
	
	pthread_mutex_t				Mutex;
	int							nValidAVBufCnt;
	
	MmAVBuf*					pAVBuf;
	int							nAVBufCnt;
	
	unsigned char*				pBuf;
	int							nBufLen;
	
} MmAVBufStruct;

typedef struct AVF_HDRStruct
{
	DWORD 	dwVer;	//fixed 0x01000000
	BYTE 	bType;	//1 - audio, 2 - video
	BYTE	bFmt;	//Audio: 1 - wav, 2 - mp3; Video: 1 - BMP, 2 - JPEG, 3 - MPEG4
	BYTE	bExFmt;	//Audio: 1 - 8K, 2 - 11.25K, >= 3 reserved
}	AVF_HDR;

/************** EXTERNAL DECLARATIONS ************************************************/
extern	pthread_mutex_t		MutexFrm;

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/
DWORD dwMultiTick = 0;

/*For controling MC's calling GM*/
unsigned char g_uPlayingRing = 0;

#ifdef MM_VIDEO_FRM_INDEX_DEBUG
static unsigned int		nLastVideoFrmIdx	= 0;
#endif
#ifdef MM_AUDIO_FRM_INDEX_DEBUG
static unsigned int		nLastAudioFrmIdx	= 0;
#endif

static int		mox_kb_led_fd = -1;
static unsigned long kb_led_times = 0;

static DWORD	dwPlaybackLastTick	= 0;
static DWORD	dwCaptureLastTick	= 0;
	
static int playback_reset = 0;
static int capture_reset = 0;
	
static unsigned long playback_callback_times	= 0;
static unsigned long capture_callback_times	= 0;
static unsigned long playback_callback_last_times	= 0;
static unsigned long capture_callback_last_times	= 0;

//static unsigned int playback_error_times	= 0;
//static unsigned int capture_error_times	= 0;
//static DWORD	capture_tickcount_start		= 0;
//static DWORD	capture_tickcount_end		= 0;

static		UINT	nVideoLastFrmID = 0;
static		UINT	nAudioLastFrmID = 0;

static MmAVBufStruct	MmAudioBuf;
static MmAVBufStruct	MmVideoBuf;
static MmManStruct		MmMan;
static UINT				nTalkVolume			= SC_DEFAULT_TALK_VOLUME;


void				   MmWorkThreadFun(void* arg);
static BOOL		MmGetMsg(MXMSG* pMsg);
static void			MmFreeMXMSG(MXMSG* pMsg);
static void			MmWrite(DATATYPE nDataType, unsigned char* pBuf, int nBufLen);

static void			ProcessEth2MmAV();

static void			MmAVDataInit();
static void			MmAVDataExit();
static void			MmSendAV2Eth(DATATYPE nType, MmSendAVData* pSendData);

#ifdef	AV_TO_MM_DEBUG
static void			AVFrmPrintHead(AVFrmHead* pHead);
#endif

static void			MmAVBufInit(MmAVBufStruct* pAV, int nCnt, int nBufSize);
static void			MmAVBufExit(MmAVBufStruct* pAV);
static MmAVBuf*		AllocAVBuf(MmAVBufStruct* pAV);
static void			MmAVBufPut(MmAVBufStruct* pAV, unsigned char* pBuf, int nBufLen);
static void			MmAVBufFree(MmAVBufStruct* pAV, MmAVBuf* pAVBuf);
static void			MmAVBufReset(MmAVBufStruct* pAV);

static void			MmAVFrmIdxDebug(DATATYPE nDataType, AVFrmHead* pHead);

static void			AudioPlayMemInit(AudioPlayMemManStr* pAPlayMem);
static void			LoadAudio2Mem(unsigned char* pFileName, AudioPlayMemManStr* pAPlayMem, DWORD dwPlayInterval, int nLenPerPlay, BOOL bPlayOnce);
static void			AudioPlayMemExit(AudioPlayMemManStr* pAPlayMem);
static BOOL			PlayMemAudio(AudioPlayMemManStr* pAPlayMem);

static void			StopPlayCallRing();
static void			PausePlayCallA();

static void			StartRdirEthMmAV(BOOL bCapA2Eth, BOOL bCapV2Eth, BOOL bPlayEthA, BOOL bPlayEthV);
static void			PauseRdirEthMmAV();
static void			StopRdirEthMmAV();

static int			ReadAVData(DATATYPE nDataType, unsigned char* pBuf, int nLen);
static int			WriteAVData(DATATYPE nDataType, unsigned char* pBuf, int nLen);


static void	MMPhotoSaveNote();

static void	MmProcessDiag();

static void	MmSendPhoto2EHV(unsigned char *pBuf, int nBufLen,int nPicType);
static void	MmSendPhoto2MC(unsigned char *pBuf, int nBufLen);

DWORD g_LogReference = 0;

static void StartPlayErrorSound();
static void MMSendLeavePhotoEnd2Talk();
static void MMSendPhotoNote2Eth(int nType, unsigned char* pBuf, int nBufLen);

BYTE	ucVideoFrameHeader[50] = {0};
SHORT	nVideoHeaderLen = 0;
BOOL	DecInitFlag = FALSE;
static void	MmVCodeInit(void);

static void AudioInit(void);
static void AudioStart(void);
static void AudioEnd(void);
static int	AudioPlay(char *pBuf, int nLen);
static int	NoteAudioPlay(char *pBuf, int nLen);
static int	AudioCapture(char* pBuf);

static int mxc_ssi_fd = -1;
static int mxc_ssi2_fd = -1;

UINT	nTimer_id = 0;

static void ProcessPlayAudio();

static void StopPlayCallA();
static BOOL StartPlayMusic(const char * pFileName, BYTE bPlayMode, DWORD dwPlayTime, UINT nCycleCnt);
static void			StartPlayOnceSound(const char *pszRingName);

static TIMERPROC CALLBACK lpTimerProcess();

/*
    HV发给GM的命令GC_AV_REQUEST_VIDEO_IVOP置此变量值为1用来请求生成I帧
*/
int bRequestIFrame		=	0;

#ifdef MM_WR_RCV_AU_DEBUG
static UCHAR WrBuf[1024 * 1024] = { 0 };
static INT		   nWrLen = 0;
static FILE*     FdWrA = NULL;
static BOOL		bWrStored = FALSE;
#endif

#ifdef MM_WR_SND_AU_DEBUG
static UCHAR RdBuf[1024 * 1024] = { 0 };
static INT		   nRdLen = 0;
static FILE*     FdRdA = NULL;
static BOOL		bRdStored = FALSE;
#endif

#ifdef MM_WR_SND_V_DEBUG
static BYTE		VBuf[15 * 1024 * 1024];
static DWORD	nVLen = 0;
static FILE*    FdV = NULL;

#endif

static VOID	StartPlayAlarmA();
static VOID	StopPlayAlarmA();

static VOID	MMSendLogPicEnd2Talk();

static VOID	CountByteRate(INT DataBytes);


static void		StartPlayKeySound(void);
static void		StartPlayErrorSound(void);
static void		StartPlayRightSound(void);


static BOOL	bVideoInit=FALSE;
BYTERATE g_ByteRate;



static MmAVBufStruct	MmVideoSendBuf;
static pthread_t		thAVSend2EthWork;
static BOOL 			g_PlayMusicState = TYPE_PLAY_ONCE;
static DWORD 			g_audio_play_ticks = 0;

#define ETH_TEST_SEND2ETH_DEBUG
#define	MM_VIDEO_SEND__ARRAY_CNT			20
#define	MM_VIDEO_SEND_DATA_SIZE				(150 * 1024)
static void MmSendAV2EthWorkThreadFun(void* arg);
static int MmAVSendBufPut( void);



/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MultimediaInit
**	AUTHOR:			Jerry Huang
**	DATE:			30 - Oct - 2006
**
**	DESCRIPTION:	
**			Multi-Media module init
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void 
MultimediaInit()
{
	if ((mox_kb_led_fd = open("/dev/mox_kb_led", O_RDWR)) == -1)
	{
		printf("open mox keyboard fail\n");
	}	

	kb_led_times = 0;
	//video
#ifndef VIDEO_FLOW_DEBUG
//	vpu_SystemInit();
#endif

#ifndef AUDIO_FLOW_DEBUG
	AudioInit();
#endif	
	DpcAddMd(MXMDID_MM, NULL);
	MmAVDataInit();
	MmAVBufInit(&MmAudioBuf, MM_AUDIO__ARRAY_CNT, MM_AUDIO_DATA_SIZE);
	MmAVBufInit(&MmVideoBuf, MM_VIDEO__ARRAY_CNT, MAX_VIDEOCAP_DATA_SIZE);

#ifdef ETH_TEST_SEND2ETH_DEBUG
	MmAVBufInit(&MmVideoSendBuf, MM_VIDEO_SEND__ARRAY_CNT, MM_VIDEO_SEND_DATA_SIZE);
#endif
	//MmAVBufTest(&MmAudioBuf);

	memset(&MmMan.SendVideo.LastAVHead, 0, sizeof (AVFrmHead));
	memset(&MmMan.SendAudio.LastAVHead, 0, sizeof (AVFrmHead));

	MmMan.SendVideo.LastAVHead.nFrmIdx		= 0;
	MmMan.SendVideo.LastAVHead.nType		= MM_VIDEO_TYPE;
	MmMan.SendVideo.LastAVHead.nCapture		= MM_VIDEO_CAPTURE_DEFAULT;
	MmMan.SendVideo.LastAVHead.nFormat		= MM_VIDEO_FORMAT_DEFAULT;

	MmMan.SendAudio.LastAVHead.nFrmIdx		= 0;
	MmMan.SendAudio.LastAVHead.nType		= MM_AUDIO_TYPE;
	MmMan.SendAudio.LastAVHead.nCapture		= MM_AUDIO_CAPTURE_DEFAULT;
	MmMan.SendAudio.LastAVHead.nFormat		= MM_AUDIO_FORMAT_DEFAULT;

	AudioPlayMemInit(&MmMan.AudioPlayMemMan);

	MmMan.StackAVFlag.bCapA2Eth	= FALSE;
	MmMan.StackAVFlag.bCapV2Eth = FALSE;
	MmMan.StackAVFlag.bPlayEthA = FALSE;
	MmMan.StackAVFlag.bPlayEthV = FALSE;
	MmMan.StackAVFlag.bPhoto2EHV= FALSE;
	MmMan.StackAVFlag.bPhoto2MC= FALSE;
	MmMan.StackAVFlag.nVCapStartTick = GetTickCount();
	MmMan.StackAVFlag.nACapStartTick = GetTickCount();
	MmMan.StackAVFlag.nAPlayStartCnt = 0;

	MmMan.nPlayPromptStatus = AV_PLAY_STATUS_IDLE;
	MmMan.nPlayCallAStatus = AV_PLAY_STATUS_IDLE;
	MmMan.nPlayAlarmAStatus = AV_PLAY_STATUS_IDLE;
	MmMan.nRdirEthMmAVStatus = AV_PLAY_STATUS_IDLE;
#ifndef VIDEO_FLOW_DEBUG
	//PIOWrite(PIO_TYPE_CCD_POWER, CCD_PWR_OFF);
	MmMan.SendVideo.LastAVHead.nFrmIdx = 1;
/*	MmVCodeInit();
	
	PIOWrite(PIO_TYPE_CCD_POWER, CCD_PWR_OFF);
	Encode_Reset();	
	
	Decode_Reset();
	
	usleep(300 * 1000);
	Dec_Exit();
	Enc_Exit();*/
#endif

	if ((pthread_create(&MmMan.thWork, NULL, MmWorkThreadFun, NULL)) != 0)	
	{
		printf("Mm: create thread fail\n");
	}

#ifdef ETH_TEST_SEND2ETH_DEBUG	
	if ((pthread_create(&thAVSend2EthWork, NULL, MmSendAV2EthWorkThreadFun, NULL)) != 0)	
	{
		printf("Mm: create thAVSend2EthWork fail\n");
	}
#endif	

	
	MmMan.bthWorkQuit = FALSE;


	MmVideoInit();


//	SetCamera();
	
	
#ifdef MM_INIT_EXIT_DEBUG
	printf("Mm: Initialize ...\n");
#endif
}

static void
StartRdirEthMmAV(BOOL bCapA2Eth, BOOL bCapV2Eth, BOOL bPlayEthA, BOOL bPlayEthV)
{

#ifndef VIDEO_FLOW_DEBUG
    if(!bVideoInit)
    {
    	if ((bCapV2Eth || bPlayEthV) )
    	{
    		MmMan.SendVideo.LastAVHead.nFormat		= GetTalkLocStreamFormat();
    		int iret = 0;
    		if (bCapV2Eth)
    		{
    			//PIOWrite(PIO_TYPE_CCD_POWER, CCD_PWR_ON);
    			SetCamera(1);
    		}
    		iret = vpu_SystemInit();
            //usleep(1000*1000);	//at least delay 1000ms
			MultiTimeProcess();
			usleep(200*1000);	
			MultiTimeProcess();
			usleep(200*1000);	
			MultiTimeProcess();
			usleep(200*1000);	
			MultiTimeProcess();
			usleep(200*1000);	
			MultiTimeProcess();
			usleep(200*1000);	
			MultiTimeProcess();
    		if (!iret)
    		{
    			usleep(100 * 1000);
    			MmVCodeInit();
    			usleep(50 * 1000);
    			
    		}
    	}
    }

	if (bCapV2Eth)
	{
		//usleep(650 * 1000);
		Encode_Reset();
		MmMan.StackAVFlag.nVCapStartTick = GetTickCount();
		MmMan.StackAVFlag.bCapV2Eth = bCapV2Eth;		
	}

	if (bPlayEthV)
	{
		Decode_Reset();
		MmMan.StackAVFlag.bPlayEthV = bPlayEthV;		
	}

    if(!bVideoInit && (bCapV2Eth || bPlayEthV))
    {
    	bVideoInit=TRUE;
    	MmVideoStart();
    	EthGMLog_videologopen();
    }
#endif


#ifndef AUDIO_FLOW_DEBUG
	if (bPlayEthA || bCapA2Eth)
	{
#ifdef MM_WR_RCV_AU_DEBUG
		nWrLen = 0;
		bWrStored = FALSE;
#endif

#ifdef MM_WR_SND_AU_DEBUG
		nRdLen = 0;
		bRdStored = FALSE;
#endif
		
		AudioStart();
		MmMan.StackAVFlag.nAPlayStartCnt = 0;
		MmMan.StackAVFlag.bCapA2Eth = bCapA2Eth;		
		MmMan.StackAVFlag.bPlayEthA = bPlayEthA;
		
		MmMan.StackAVFlag.nACapStartTick = GetTickCount();
		MmMan.StackAVFlag.nACapBytesCnt	 = 0;
	}
#endif

	
	MmAVBufReset(&MmAudioBuf);
	MmAVBufReset(&MmVideoBuf);
	
#ifdef ETH_TEST_SEND2ETH_DEBUG
	MmAVBufReset(&MmVideoSendBuf);
#endif	

	nVideoLastFrmID = 0;
	nAudioLastFrmID = 0;
	
	printf("Start rdirEthMM: the BOOL: %d,%d, %d,%d.\n", 
		MmMan.StackAVFlag.bCapA2Eth, 
		MmMan.StackAVFlag.bCapV2Eth, 
		MmMan.StackAVFlag.bPlayEthA, 
		MmMan.StackAVFlag.bPlayEthV);
}

static void
PauseRdirEthMmAV()
{
	StopRdirEthMmAV();
}

static void
StopRdirEthMmAV()
{
	BOOL	bCapA2Eth = MmMan.StackAVFlag.bCapA2Eth;
	BOOL	bCapV2Eth = MmMan.StackAVFlag.bCapV2Eth;
	BOOL	bPlayEthA = MmMan.StackAVFlag.bPlayEthA;
	BOOL	bPlayEthV = MmMan.StackAVFlag.bPlayEthV;
	if(bVideoInit)
	{
#ifdef VIDEO_CAPTURE_LOG_DEBUG		
		EthGMLog_videolog();
		EthGMLog_videologclose();
#endif			
		MmVideoStop();
		usleep(100 * 1000);
		MultiTimeProcess();
	}
#ifndef VIDEO_FLOW_DEBUG
	if (bCapV2Eth)
	{
		Enc_Exit();
		//PIOWrite(PIO_TYPE_CCD_POWER, CCD_PWR_OFF);
		SetCamera(0);
		MmMan.SendVideo.LastAVHead.nFrmIdx = 1;		
	}
	
	if (bPlayEthV)
	{
		Dec_Exit();
	}
	
	if(bVideoInit)
	{
		bVideoInit=FALSE;

		if ((bCapV2Eth || bPlayEthV)) 
		{
			//bVPUOpen = FALSE;
			usleep(100 * 1000);
			MultiTimeProcess();
			vpu_SystemShutdown();
			usleep(50 * 1000);
			MultiTimeProcess();
		}
	}

#endif
#ifndef AUDIO_FLOW_DEBUG
	if (bPlayEthA || bCapA2Eth)
	{
		AudioEnd();
		MmMan.StackAVFlag.nAPlayStartCnt = 0;
	}
#endif

//	printf("Stop rdirEthMM: the BOOL: %d,%d, %d,%d.\n", MmMan.bCapA2Eth, MmMan.bCapV2Eth, MmMan.bPlayEthA, MmMan.bPlayEthV);
	
	MmMan.StackAVFlag.bCapA2Eth = FALSE;
	MmMan.StackAVFlag.bCapV2Eth = FALSE;
	MmMan.StackAVFlag.bPlayEthA = FALSE;
	MmMan.StackAVFlag.bPlayEthV = FALSE;
	//MmWrite(_DATA_TYPE_CMD, (unsigned char*) &nCmd, sizeof (nCmd));
	MmMan.RcvAudio.nCurLen	= 0;
	MmMan.RcvAudio.bUsed	= FALSE;
	MmMan.RcvVideo.nCurLen	= 0;
	MmMan.RcvVideo.bUsed	= FALSE;
	MmAVBufReset(&MmAudioBuf);
	MmAVBufReset(&MmVideoBuf);

#ifdef ETH_TEST_SEND2ETH_DEBUG
	MmAVBufReset(&MmVideoSendBuf);
#endif
    g_uAreahead = 1;
	usleep(100 * 1000);
	MultiTimeProcess();
}

void PutCapData2Buf(void)
{
	int nLen;
    /*
        在程序收到消息需要开始视频的时候此条件成立
    */
	if (MmMan.nRdirEthMmAVStatus == AV_PLAY_STATUS_RUN)
	{
		/*
            如果通讯过程中需要视频，则置此变量为真
        */
		if (MmMan.StackAVFlag.bCapV2Eth)
		{
            /*
                在程序收到消息需要开始视频的时候开始计时
            */
			//if (GetTickCount() - MmMan.StackAVFlag.nVCapStartTick < 500) 
			//{
			//	return;
			//}
			nLen = MmAVSendBufPut();
			if (nLen > 0)
			{
				dwDiag[DIAG_V_SENT] += nLen;
				dwDiag[DIAG_V_FRM_SENT]++;

				CountByteRate(nLen);
			}
		}
	}
}

static void
ProcessMm2EthAV()
{	
	static	DWORD  nPreTkReadV = 0;
	int		nRepeat		=		0;
	int nLen;
	
	if (MmMan.nRdirEthMmAVStatus == AV_PLAY_STATUS_RUN)
	{
		//Read and Send Audio
		if (MmMan.StackAVFlag.bCapA2Eth)
		{
			for(nRepeat = 0; nRepeat < 1; nRepeat++)
			{
				MmMan.SendAudio.nCurLen = ReadAVData(DATA_TYPE_AUDIO, 
					MmMan.SendAudio.pBuf, MmMan.SendAudio.nBufLen);
				
				MmMan.StackAVFlag.nACapBytesCnt += MmMan.SendAudio.nCurLen;
/*
				if (0 == MmMan.StackAVFlag.nACapBytesCnt
					&& GetTickCount() - MmMan.StackAVFlag.nACapStartTick > 1000 * 3
					) 
				{
					EthGMLog(TYPE_NO_A_CAP);
					system("reboot");
				}
*/				
				if (MmMan.SendAudio.nCurLen > 0)
				{
					dwDiag[DIAG_A_SENT] += MmMan.SendAudio.nCurLen;
				}					
				MmSendAV2Eth(DATA_TYPE_AUDIO, &MmMan.SendAudio);
			}
		}	

	}
	else if (MmMan.nRdirEthMmAVStatus == AV_PLAY_STATUS_LEAVEPHTO)
	{
#ifdef LEAVE_PHOTO_DEBUG
		printf("Send Photo\n");
#endif

		if (MmMan.StackAVFlag.bPhoto2EHV) 
		{	
			//Send picture to EHV
			usleep(500 * 1000);
//			MmSendPhoto2EHV(JpgPicBuf, nJpgBufLen);		
			MmSendPhoto2EHV(JpgPicBuf, nJpgBufLen,2);				
			MmMan.StackAVFlag.bPhoto2EHV= FALSE;
			MMSendLeavePhotoEnd2Talk();
#ifdef LEAVE_PHOTO_DEBUG
			printf("Leaving Photo 2 EHV end\n");			
#endif			
		}
		if (MmMan.StackAVFlag.bPhoto2MC) 
		{
			MmSendPhoto2MC(g_TLInfo.PicBuf, g_TLInfo.nPicLen);
			MmMan.StackAVFlag.bPhoto2MC			=	FALSE;
			MMSendLogPicEnd2Talk();
#ifdef LEAVE_PHOTO_DEBUG
			printf("Leaving Photo 2 MC end, nPicLen=%d\n", g_TLInfo.nPicLen);			
#endif			
		}
		MmMan.nRdirEthMmAVStatus = AV_PLAY_STATUS_IDLE;		
	}
	else
	{
		MmMan.SendAudio.nCurLen = 0;
		MmMan.SendVideo.nCurLen = 0;
	}
}


static void
ProcessEth2MmAV()
{	
	MmAVBuf*		pAVBuf;
	int				nRet;		
	
	if (MmMan.nRdirEthMmAVStatus == AV_PLAY_STATUS_RUN)
	{
		if (MmMan.StackAVFlag.bPlayEthA)
		{
			if ((MmAudioBuf.nValidAVBufCnt > 0) && (MmAudioBuf.Head.pHead != NULL))
			{
				pAVBuf = (MmAVBuf*) MmAudioBuf.Head.pHead;
				nRet = WriteAVData(DATA_TYPE_AUDIO, pAVBuf->pBuf, pAVBuf->nCurLen);
				if (nRet > 0)
				{
					MmAVBufFree(&MmAudioBuf, pAVBuf);
				}
			}
		}
		
		if (MmMan.StackAVFlag.bPlayEthV)
		{
			if ((MmVideoBuf.nValidAVBufCnt > 0) && (MmVideoBuf.Head.pHead != NULL))
			{
				pAVBuf = (MmAVBuf*) MmVideoBuf.Head.pHead;
				nRet = WriteAVData(DATA_TYPE_VIDEO, pAVBuf->pBuf, pAVBuf->nCurLen);
				if (nRet > 0)
				{
					MmAVBufFree(&MmVideoBuf, pAVBuf);
				}
			}
		}
	}	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MultimediaProcess
**	AUTHOR:			Jerry Huang
**	DATE:			30 - Oct - 2006
**
**	DESCRIPTION:	
**			Multi-Media module process
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
MultimediaProcess()
{
	MXMSG			MsgGet;
	int		i,nRepeat		=		0;

	BOOL	bCapA2Eth = FALSE;
	BOOL	bCapV2Eth = FALSE;
	BOOL	bPlayEthA = FALSE;
	BOOL	bPlayEthV = FALSE;


	struct key_run_status key_status;

	kb_led_times++;
	if (100 == kb_led_times)
	{
		kb_led_times = 0;
		ioctl(mox_kb_led_fd, MOX_GET_KEY_STATUS, &key_status);
		dwDiag[DIAG_ETH_SENT] = key_status.read_times;
		dwDiag[DIAG_ETH_SENT_ERR] = key_status.key_times;
	}

	if (MmGetMsg(&MsgGet))
	{
#ifdef AUDIO_WR_DEBUG	
	printf("MsgGet.dwMsg=0x%08x\n",MsgGet.dwMsg);
#endif
		switch (MsgGet.dwMsg)
		{
		case MXMSG_PLY_DI1_A:
			{
				if (MmMan.nPlayAlarmAStatus != AV_PLAY_STATUS_IDLE)
				{
					
				}
				else
				{
                    StartPlayOnceSound(FILE_DI1_SOUND);
                    MmMan.nPlayPromptStatus = AV_PLAY_STATUS_RUN;
				}
				break;
			}
		case MXMSG_PLY_DI2_A:
			{
				if (MmMan.nPlayAlarmAStatus != AV_PLAY_STATUS_IDLE)
				{
					
				}
				else
				{
                    StartPlayOnceSound(FILE_DI2_SOUND);
                    MmMan.nPlayPromptStatus = AV_PLAY_STATUS_RUN;
				}
				break;
			}
		case MXMSG_PLY_KEY_A:
			{
//				if (MmMan.nPlayAlarmAStatus != AV_PLAY_STATUS_IDLE)
//				{
//					
//				}
//				else
//				{
					if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType) 
					{
						StartPlayKeySound();
					}
					else
					{
						if (g_DevFun.bNewPnUsed)
						{
							StartPlayKeySound();
						} 							
						else if (strcmp(g_SysInfo.HardareVersion,"V2.30.00") >=0) 
						{
							printf("g_SysInfo.HardareVersion is more than V2.30.00 \n");
							StartPlayKeySound();
						}
						else
						{
							printf("g_SysInfo.HardareVersion is less than V2.30.00 \n");
							if (MmMan.nPlayAlarmAStatus == AV_PLAY_STATUS_IDLE)
							{								
								StartPlayOnceSound(FILE_KEY_SOUND);
								MmMan.nPlayPromptStatus = AV_PLAY_STATUS_RUN;
							}
							
						}
					}
						
//				}
				break;
			}
		case MXMSG_PLY_ERROR_A:
			{
				if (MmMan.nPlayAlarmAStatus != AV_PLAY_STATUS_IDLE)
				{
					
				}
				else
				{
					if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType) 
					{
						StartPlayErrorSound();
					}
					else
					{
						if (g_DevFun.bNewPnUsed)
						{
							StartPlayErrorSound();
						}
						else if (strcmp(g_SysInfo.HardareVersion,"V2.30.00") >=0) 
						{
							StartPlayErrorSound();
						}
						else
						{
							StartPlayOnceSound(FILE_ERROR_SOUND);
							MmMan.nPlayPromptStatus = AV_PLAY_STATUS_RUN;
						}						
					}						
				}
				break;
			}
		case MXMSG_PLY_RIGHT_A:
			{
				if (MmMan.nPlayAlarmAStatus != AV_PLAY_STATUS_IDLE)
				{
					
				}
				else
				{
					if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType) 
					{
//						StartPlayKeySound();
						StartPlayRightSound();
					}
					else
					{
						if (g_DevFun.bNewPnUsed)
						{
							StartPlayRightSound();
						}							
						else if (strcmp(g_SysInfo.HardareVersion,"V2.30.00") >=0) 
						{
							StartPlayRightSound();
						}
						else
						{
							StartPlayOnceSound(FILE_RIGHT_SOUND);
							MmMan.nPlayPromptStatus = AV_PLAY_STATUS_RUN;					
						}						
					}
				}
				break;
			}

		case MXMSG_PLY_CALL_A:
			{
				if (MmMan.nPlayAlarmAStatus != AV_PLAY_STATUS_IDLE)
				{
					
				}
				else
				{
					AudioStart();
                    g_uPlayingRing = 1;
					g_PlayMusicState = StartPlayMusic(FILE_CALL_RING, TYPE_PLAY_RECYCLE, NO_TIME_LIMIT, 0);
					MmMan.nPlayCallAStatus = AV_PLAY_STATUS_RUN;
				}
				break;
			}
		case MXMSG_STP_CALL_A:
			{
                g_uPlayingRing = 0;
				if (MmMan.nPlayCallAStatus == AV_PLAY_STATUS_RUN)
				{
					AudioEnd();
					StopPlayCallRing();
				}
				MmMan.nPlayCallAStatus = AV_PLAY_STATUS_IDLE;
				
				break;
			}
		case MXMSG_PLY_ARREARS_A:
			{
				if (MmMan.nPlayAlarmAStatus != AV_PLAY_STATUS_IDLE)
				{
					
				}
				else
				{

					AudioStart();
					g_PlayMusicState = StartPlayMusic(FILE_ARREARS_RING, TYPE_PLAY_ONCE, NO_TIME_LIMIT, 0);
					MmMan.nPlayCallAStatus = AV_PLAY_STATUS_RUN;
				}
				break;
			}
		case MXMSG_STP_ARREARS_A:
			{
				if (MmMan.nPlayCallAStatus == AV_PLAY_STATUS_RUN)
				{
					AudioEnd();
					StopPlayCallRing();
				}
				MmMan.nPlayCallAStatus = AV_PLAY_STATUS_IDLE;
				
				break;
			}
		case MXMSG_PLAY_TALK_TEST:
			{
				if (MmMan.nPlayAlarmAStatus != AV_PLAY_STATUS_IDLE)
				{
				//	MmMan.nPlayCallAStatus = AV_PLAY_STATUS_PAUSE;
				}
				else
				{

//					StartPlaySystemNoteA();
//					SetEHVTalkVolume();
					MmMan.nPlayCallAStatus = AV_PLAY_STATUS_RUN;
				}
				
				break;
			}

		case MXMSG_STA_RDIR_ETH_MM_A:
		case MXMSG_STA_MM2ETH_V:
		case MXMSG_STA_ETH2MM_A_MM2ETH_AV:
			MmMan.nAVDestIP = MsgGet.dwParam;

//			nJpgBufLen	=	0;
//			memset(JpgPicBuf,0,PHOTO_WIDTH * PHOTO_HEIGHT);	
			
			if (MmMan.nPlayAlarmAStatus != AV_PLAY_STATUS_IDLE)
			{
				MmMan.nRdirEthMmAVStatus = AV_PLAY_STATUS_PAUSE;
			}
			else
			{                
				if (MsgGet.dwMsg == MXMSG_STA_ETH2MM_A_MM2ETH_AV)//Talking with MC and HV
				{
					bCapA2Eth = TRUE;
					bCapV2Eth = TRUE;
					bPlayEthA = TRUE;
					bPlayEthV = FALSE;
				}
				else if (MsgGet.dwMsg == MXMSG_STA_MM2ETH_V)//monitored or when calling MC,HV
				{
					bCapA2Eth = FALSE;
					bCapV2Eth = TRUE;
					bPlayEthA = FALSE;
					bPlayEthV = FALSE;
				}
				else if (MsgGet.dwMsg == MXMSG_STA_RDIR_ETH_MM_A)//Talking with GM
				{
					bCapA2Eth = TRUE;
					bCapV2Eth = FALSE;
					bPlayEthA = TRUE;
					bPlayEthV = FALSE;
				}
				else if (MsgGet.dwMsg == MXMSG_STA_CALLRING)
				{
					bCapA2Eth = FALSE;
					bCapV2Eth = FALSE;
					bPlayEthA = FALSE;
					bPlayEthV = FALSE;
				}
				MmMan.nRdirEthMmAVStatus = AV_PLAY_STATUS_RUN;
                /* turn off the sound when you are in debug , the sound is disgusting */
                //bCapA2Eth = bPlayEthA = FALSE;//[MichaelMa]
                StartRdirEthMmAV(bCapA2Eth,bCapV2Eth,bPlayEthA,bPlayEthV);
				SetGMTalkVolume();
				if(MXMSG_STA_MM2ETH_V==MsgGet.dwMsg)
				{
					StartGPVideo(MsgGet.dwParam);
				}
			}
			break;

		case MXMSG_STP_RDIR_ETH_MM_A:
		case MXMSG_STP_MM2ETH_V:
		case MXMSG_STP_ETH2MM_A_MM2ETH_AV:
			if (MmMan.nRdirEthMmAVStatus == AV_PLAY_STATUS_RUN)
			{
				StopRdirEthMmAV();
			}
			MmMan.nRdirEthMmAVStatus = AV_PLAY_STATUS_IDLE;
#ifdef LEAVE_PHOTO_DEBUG			
			printf("%s msg:%x\n",__FUNCTION__,MsgGet.dwMsg);
#endif
			StopGPVideo();
			break;
	
		case MXMSG_PLY_ALM_A:
			StartAlarmOutPut();
			if (MmMan.nPlayCallAStatus != AV_PLAY_STATUS_IDLE)
			{
				PausePlayCallA();
				MmMan.nPlayCallAStatus = AV_PLAY_STATUS_PAUSE;
			}

			if (MmMan.nRdirEthMmAVStatus != AV_PLAY_STATUS_IDLE)
			{
				PauseRdirEthMmAV();
				MmMan.nRdirEthMmAVStatus = AV_PLAY_STATUS_PAUSE;
			}

			StartPlayAlarmA();
			MmMan.nPlayAlarmAStatus = AV_PLAY_STATUS_RUN;

			break;

		case MXMSG_STP_ALM_A:
			StopAlarmOutPut();
			StopPlayAlarmA();
			MmMan.nPlayAlarmAStatus = AV_PLAY_STATUS_IDLE;

			break;

		case MXMSG_SEND_PHOTO_BMP:
			{
#ifdef LEAVE_PHOTO_DEBUG
				printf("Reiceive Send bmp Photo Command\n");
#endif				
				MmMan.nRdirEthMmAVStatus = AV_PLAY_STATUS_LEAVEPHTO;
				MmMan.StackAVFlag.bPhoto2EHV = TRUE;
			}
			break;
		case MXMSG_SEND_PHOTO_JPG:
			{
#ifdef LEAVE_PHOTO_DEBUG
				printf("Reiceive Send jpg Photo Command\n");
#endif
				MmMan.nRdirEthMmAVStatus = AV_PLAY_STATUS_LEAVEPHTO;
				MmMan.StackAVFlag.bPhoto2MC = TRUE;
			}
			break;

		case MXMSG_SAVE_PHOTO:
			{
				MMPhotoSaveNote();
			}
			break;

		case FC_AV_REQUEST_VIDEO_IVOP:
			{
				pthread_mutex_lock(&MutexFrm);		
				bRequestIFrame = 1;
				pthread_mutex_unlock(&MutexFrm);
			}
			break;

		default:
			break;
		}
		MmFreeMXMSG(&MsgGet);
	}

	for(nRepeat = 0; nRepeat < 10; nRepeat++)
	{
		ProcessEth2MmAV();
	}

//	if (GetTickCount() - dwMultiTick> 150) 
//	{
//		printf("-----------Mutil Overtime: %d \n",GetTickCount() - dwMultiTick);
//	}
//	dwMultiTick = GetTickCount();


	ProcessPlayAudio();
	ProcessMm2EthAV();
	MmProcessDiag();
	MultiTimeProcess();
}

static void
MmProcessDiag()
{
	static DWORD	dwVideoLastTick	= 0;
	static DWORD	nVideoFrame		= 0;
	static DWORD	nVideoSent		= 0;
	static DWORD	nAudioSent		= 0;
	static DWORD	nAudioRcv		= 0;
	struct run_status ssi_status;

	if (GetTickCount() - dwVideoLastTick > 1000)
	{
		dwDiag[DIAG_V_SENT_FRM_RATE] = dwDiag[DIAG_V_FRM_SENT] - nVideoFrame;
		nVideoFrame = dwDiag[DIAG_V_FRM_SENT];

		dwDiag[DIAG_A_SENT_RATE] = (dwDiag[DIAG_A_SENT]  - nAudioSent) / 1000;
		nAudioSent = dwDiag[DIAG_A_SENT];

		dwDiag[DIAG_A_RCV_RATE] = (dwDiag[DIAG_A_RCV]  - nAudioRcv) / 1000;
		nAudioRcv = dwDiag[DIAG_A_RCV];
		
		dwDiag[DIAG_V_SENT_RATE]	=	dwDiag[DIAG_V_SENT]  - nVideoSent;
		nVideoSent = dwDiag[DIAG_V_SENT];

		dwVideoLastTick = GetTickCount();
	}

		if ( (GetTickCount() - dwPlaybackLastTick > 2000) )
		{
			ioctl(mxc_ssi_fd, IOCTL_SSI_GET_PLAYBACK_CALLBACKS, &playback_callback_times);
			if ( playback_callback_last_times == playback_callback_times)
			{
			//	printf("GetTickCount()=%d,dwPlaybackLastTick=%d,playback_callback_times=%d\n",GetTickCount(),dwPlaybackLastTick,playback_callback_times);
				ioctl(mxc_ssi_fd, IOCTL_SSI_GET_STATUS, (void *)(&ssi_status));
				EthGMLog_ssi_playback(ssi_status);
					
				system("reboot");
				while (1);
			}
			playback_callback_last_times = playback_callback_times;
			dwPlaybackLastTick = GetTickCount();
		}
		if ( (GetTickCount() - dwCaptureLastTick > 2000) )
		{	
			ioctl(mxc_ssi_fd, IOCTL_SSI_GET_CAPTURE_CALLBACKS, &capture_callback_times);
			if ( capture_callback_last_times == capture_callback_times)
			{
			//	printf("GetTickCount()=%d,dwCaptureLastTick=%d,capture_callback_times=%d\n",GetTickCount(),dwCaptureLastTick,capture_callback_times);
				ioctl(mxc_ssi_fd, IOCTL_SSI_GET_STATUS, (void *)(&ssi_status));
				EthGMLog_ssi_playback(ssi_status);
					
				system("reboot");
				while (1);
			}	
			capture_callback_last_times = capture_callback_times;
			dwCaptureLastTick = GetTickCount();
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
MultimediaExit()
{
	MmMan.bthWorkQuit = TRUE;

	MmAVBufExit(&MmAudioBuf);
	MmAVBufExit(&MmVideoBuf);
#ifdef ETH_TEST_SEND2ETH_DEBUG
	MmAVBufExit(&MmVideoSendBuf);
#endif
	
	MmAVDataExit();

	DpcRmMd(MXMDID_MM);

#ifdef MM_INIT_EXIT_DEBUG
	printf("Mm: Exit ...\n");
#endif
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmWorkThreadFun
**	AUTHOR:			Jerry Huang
**	DATE:			30 - Oct - 2006
**
**	DESCRIPTION:	
**			Multi-Media module work thread function
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				arg			[IN]		void*
**	RETURNED VALUE:	
**				0
**	NOTES:
**			
*/
void
MmWorkThreadFun(void* arg)
{
	ioctl(mxc_ssi_fd, IOCTL_SSI_DMA_TRIGGER, NULL);
//	ioctl(mxc_ssi2_fd, IOCTL_SSI_DMA_TRIGGER, NULL);
	dwPlaybackLastTick = GetTickCount();
	dwCaptureLastTick = GetTickCount();
	
	while (!MmMan.bthWorkQuit)
	{
		MultimediaProcess();

		dwDiag[DIAG_THREAD_COUNT]++;
		
		usleep(1000);
	}
	pthread_exit(0);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmGetMsg
**	AUTHOR:			Jerry Huang
**	DATE:			30 - Oct - 2006
**
**	DESCRIPTION:	
**			Get Multi-Media module message
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMsg		[OUT]		MXMSG*
**	RETURNED VALUE:	
**				True if get
**	NOTES:
**			
*/
static BOOL
MmGetMsg(MXMSG* pMsg)
{
	pMsg->dwDestMd = MXMDID_MM;
	return MxGetMsg(pMsg);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmFreeMXMSG
**	AUTHOR:			Jerry Huang
**	DATE:			30 - Oct - 2006
**
**	DESCRIPTION:	
**			Multi-Media free the memory of MXMSG -> pParam
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MmFreeMXMSG(MXMSG* pMsg)
{
	if (pMsg->pParam != NULL)
	{
		MXFree(pMsg->pParam);
		pMsg->pParam = NULL;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmWrite
**	DATE:			31 - Oct - 2006
**
**	DESCRIPTION:	
**			Multi-Media write data to 
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nDataType	[IN]		DataType
**				pBuf		[IN]		unsigned char*
**				nBufLen		[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void			
MmWrite(DATATYPE nDataType, unsigned char* pBuf, int nBufLen)
{
	int		nRet = -1;

	// If the buffer is not free, the previous AV have not put to , so put it in buffer first
	if ((DATA_TYPE_VIDEO == nDataType) && (MmVideoBuf.nValidAVBufCnt > 0))
	{
	//	printf("Put video, delay 1  the buflen=%d.\n\n", nBufLen);
		MmAVBufPut(&MmVideoBuf, pBuf, nBufLen);
		return;
	}
	else if ((DATA_TYPE_AUDIO == nDataType) && (MmAudioBuf.nValidAVBufCnt > 0))
	{
		//printf("nBufLen = %d\n", nBufLen);
#ifdef MM_WR_RCV_AU_DEBUG
		if (nWrLen < 1024 * 1024)
		{
			memcpy(&WrBuf[nWrLen], pBuf, nBufLen);
			nWrLen += nBufLen;
		}
		else if(!bWrStored)
		{
			bWrStored = TRUE;
			FdWrA = fopen("/mox/rdwr/rcvau.pcm", "wr");
			fseek(FdWrA, 0, SEEK_SET);

			fwrite(WrBuf, nWrLen-nBufLen, 1, FdWrA);
			fclose(FdWrA);
			printf("Audio receive file saved\n");
		}
#endif
		MmAVBufPut(&MmAudioBuf, pBuf, nBufLen);
		MmMan.StackAVFlag.nAPlayStartCnt++;		
		return;
	}

	if ((DATA_TYPE_AUDIO == nDataType) || (DATA_TYPE_VIDEO == nDataType))
	{
		nRet = -1;
	}
	else
	{
		//nRet = Write(nDataType, pBuf, nBufLen);
	}

	if (nRet <= 0)
	{
		if (DATA_TYPE_VIDEO == nDataType)
		{
//			printf("Put video, delay 2\n");
			MmAVBufPut(&MmVideoBuf, pBuf, nBufLen);
		}
		else if (DATA_TYPE_AUDIO == nDataType)
		{
			//printf("nBufLen = %d\n", nBufLen);
#ifdef MM_WR_RCV_AU_DEBUG
			if (nWrLen < 1024 * 1024)
			{
				memcpy(&WrBuf[nWrLen], pBuf, nBufLen);
				nWrLen += nBufLen;
			}
			else if(!bWrStored)
			{
				bWrStored = TRUE;
				FdWrA = fopen("/mox/rdwr/rcvau.pcm", "wr");
				fseek(FdWrA, 0, SEEK_SET);
				
				fwrite(WrBuf, nWrLen-nBufLen, 1, FdWrA);
				fclose(FdWrA);
				printf("Audio receive file saved\n");
			}
#endif

			MmAVBufPut(&MmAudioBuf, pBuf, nBufLen);
			MmMan.StackAVFlag.nAPlayStartCnt++;		
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MxSendAV2Mm
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Oct - 2006
**
**	DESCRIPTION:	
**			Send auido/video to Multi-Media
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nType		[IN]		int
**				pBuf		[IN]		unsigned char*
**				nBufLen		[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void				
MxSendAV2Mm(int nType, unsigned char* pBuf, int nBufLen)
{
	AVFrmHead		head;
	MmRcvAVData*	pAV;
	DATATYPE		nDataType;

	memcpy(&head, pBuf, sizeof (AVFrmHead));

#ifdef	AV_TO_MM_DEBUG
//	AVFrmPrintHead(&head);
#endif
	if (AV2MM_AUDIO == nType)
	{
		pAV = &MmMan.RcvAudio;
		nDataType = DATA_TYPE_AUDIO;
	}
	else if (AV2MM_VIDEO == nType)
	{
		pAV = &MmMan.RcvVideo;
		nDataType = DATA_TYPE_VIDEO;
	}
	else
	{
		return;
	}

	// There is only one frame, just send to 
	if ( (1 == head.nTotalPckCnt)				&& 
		 (head.nPckIdx == head.nTotalPckCnt)	&&
		 (nBufLen >= head.nLen + sizeof (AVFrmHead)))
	{
		// Maybe we need store frame paramters (current omit)

		if ( (MmMan.StackAVFlag.bPlayEthA && (DATA_TYPE_AUDIO == nDataType)) ||
			 (MmMan.StackAVFlag.bPlayEthV && (DATA_TYPE_VIDEO == nDataType) && 
			 (head.nFormat == GetTalkLocStreamFormat())))
		{
			MmWrite(nDataType, pBuf + sizeof (AVFrmHead), head.nLen);

			MmAVFrmIdxDebug(nDataType, &head);
			if (DATA_TYPE_VIDEO == nDataType)
			{
				if ((head.nFrmIdx > 1 + nVideoLastFrmID) && nVideoLastFrmID)
				{
					printf("MMVideo:  Lost %d frmae, the current frame=%X. last=%X\n", head.nFrmIdx - nVideoLastFrmID - 1, head.nFrmIdx, nVideoLastFrmID);
				}
				nVideoLastFrmID = head.nFrmIdx;
			//	printf("Mm Video:the bLeaveMode=%d the current frame=%d\n",bLeaveMode, nVideoLastFrmID);
			}
			if (DATA_TYPE_AUDIO == nDataType)
			{
				if ((head.nFrmIdx > 1 + nAudioLastFrmID) && nAudioLastFrmID)
				{
					printf("MM: audio Lost %d frmae, last %x the current frame=%x.\n", head.nFrmIdx - nAudioLastFrmID - 1, nAudioLastFrmID, head.nFrmIdx);
				}
				nAudioLastFrmID = head.nFrmIdx;
			}			
		}

		pAV->nCurLen = 0;
		pAV->bUsed = FALSE;

#ifdef	AV_TO_MM_DEBUG
		printf("Mm: AV data one frame to \n");
#endif
		return;
	}

	if ( (pAV->bUsed)											&& 
		 (head.nFrmIdx == pAV->LastAVHead.nFrmIdx)				&&
		 (head.nPckIdx == pAV->LastAVHead.nPckIdx + 1)			&&
		 (head.nTotalPckCnt == pAV->LastAVHead.nTotalPckCnt)	&&
		 (head.nType == pAV->LastAVHead.nType)				&&
		 (head.nCapture == pAV->LastAVHead.nCapture)			&&
		 (head.nFormat == pAV->LastAVHead.nFormat))
	{
		pAV->LastAVHead.nPckIdx = head.nPckIdx;

		if ((pAV->nBufLen - pAV->nCurLen) >= nBufLen)
		{
			memcpy(pAV->pBuf + pAV->nCurLen, pBuf + sizeof (AVFrmHead), head.nLen);
			pAV->nCurLen += head.nLen;
		}
		else
		{
			memcpy(pAV->pBuf + pAV->nCurLen, pBuf + sizeof (AVFrmHead), pAV->nBufLen - pAV->nCurLen);
			pAV->nCurLen = pAV->nBufLen;

			printf("Mm: AV buffer overflow\n");
		}
			
#ifdef	AV_TO_MM_DEBUG
		printf("Mm: AV data store packet %d, CNT=%d\n", head.nPckIdx, head.nTotalPckCnt);
#endif

		if (head.nPckIdx == head.nTotalPckCnt)
		{
			if ( (MmMan.StackAVFlag.bPlayEthA && (DATA_TYPE_AUDIO == nDataType)) ||
				(MmMan.StackAVFlag.bPlayEthV && (DATA_TYPE_VIDEO == nDataType) && 
				(head.nFormat == GetTalkLocStreamFormat())))
			{
				MmWrite(nDataType, pAV->pBuf, pAV->nCurLen);

				MmAVFrmIdxDebug(nDataType, &head);
				if (DATA_TYPE_VIDEO == nDataType)
				{
					if ((head.nFrmIdx > 1 + nVideoLastFrmID) && nVideoLastFrmID)
					{
						printf("MMVideo:  Lost %d frmae, the current frame=%d. last=%d\n", head.nFrmIdx - nVideoLastFrmID - 1, head.nFrmIdx, nVideoLastFrmID);
					}
					nVideoLastFrmID = head.nFrmIdx;
				//	printf("Mm Video: the current frame=%d\n", nVideoLastFrmID);
				}
				if (DATA_TYPE_AUDIO == nDataType)
				{
					if ((head.nFrmIdx > 1 + nAudioLastFrmID) && nAudioLastFrmID)
					{
						dwDiag[DIAG_A_RCV_ERR] += (head.nFrmIdx - nAudioLastFrmID - 1);
						printf("MM: audio Lost %d frmae, the current frame=%d.\n", head.nFrmIdx - nAudioLastFrmID - 1, head.nFrmIdx);
					}
					nAudioLastFrmID = head.nFrmIdx;
				}
#ifdef	AV_TO_MM_DEBUG
				printf("Send I frame....************************************************\n");
#endif

			}
			
			pAV->nCurLen = 0;
			pAV->bUsed = FALSE;

#ifdef	AV_TO_MM_DEBUG
		printf("Mm: AV data write \n");
#endif
		}
	}
	else if ( (1 == head.nPckIdx)			&& 
			  (head.nLen <= pAV->nBufLen))
	{
		pAV->nCurLen = 0;
		pAV->bUsed = TRUE;

		memcpy(pAV->pBuf + pAV->nCurLen, pBuf + sizeof (AVFrmHead), head.nLen);
		pAV->nCurLen += head.nLen;

		memcpy(&pAV->LastAVHead, &head, sizeof (AVFrmHead));

#ifdef	AV_TO_MM_DEBUG
		printf("Mm: AV data store packet %d\n", head.nPckIdx);
#endif
	}
	else
	{
#ifdef	AV_TO_MM_DEBUG
		printf("Mm: AV head not match, ");
		AVFrmPrintHead(&pAV->LastAVHead);
#endif
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmAVDataInit
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Oct - 2006
**
**	DESCRIPTION:	
**			Auido/video buffer initialize
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MmAVDataInit()
{
	MmRcvAVData*		pRcvVideo	= &MmMan.RcvVideo;
	MmRcvAVData*		pRcvAudio	= &MmMan.RcvAudio;
	MmSendAVData*		pSendVideo	= &MmMan.SendVideo;
	MmSendAVData*		pSendAudio	= &MmMan.SendAudio;

	pRcvVideo->pBuf		= NULL;
	pRcvVideo->nBufLen	= 0;
	pRcvVideo->bUsed	= FALSE;
	pRcvVideo->nCurLen	= 0;
	pRcvVideo->bUsed	= FALSE;

	pRcvAudio->pBuf		= NULL;
	pRcvAudio->nBufLen	= 0;
	pRcvAudio->bUsed	= FALSE;
	pRcvAudio->nCurLen	= 0;
	pRcvAudio->bUsed	= FALSE;

	pSendVideo->pBuf	= NULL;
	pSendVideo->nBufLen	= 0;
	pSendVideo->nCurLen	= 0;
	pSendVideo->nMaxLenPerPack	= VIDEO_LEN_PER_PACK;

	pSendAudio->pBuf	= NULL;
	pSendAudio->nBufLen	= 0;
	pSendAudio->nCurLen	= 0;
	pSendAudio->nMaxLenPerPack	= AUDIO_LEN_PER_PACK;

	MmMan.nBufLen = MAX_VIDEOCAP_DATA_SIZE * 2 + MM_AUDIO_DATA_SIZE * 2;
	MmMan.pBuf = malloc(MmMan.nBufLen);

	if (NULL == MmMan.pBuf)
	{
		return;
	}

#ifdef	MM_MEM_DEBUG
		printf("Mm: malloc %08X\n", (unsigned int) MmMan.pBuf);
#endif

	memset(MmMan.pBuf, 0, MmMan.nBufLen);
	
	pRcvVideo->pBuf		= MmMan.pBuf;
	pRcvVideo->nBufLen	= MAX_VIDEOCAP_DATA_SIZE;
	pRcvAudio->pBuf		= pRcvVideo->pBuf + pRcvVideo->nBufLen;
	pRcvAudio->nBufLen	= MM_AUDIO_DATA_SIZE;
	pSendVideo->pBuf	= pRcvAudio->pBuf + pRcvAudio->nBufLen;
	pSendVideo->nBufLen	= MAX_VIDEOCAP_DATA_SIZE;
	pSendAudio->pBuf	= pSendVideo->pBuf + pSendVideo->nBufLen;
	pSendAudio->nBufLen	= MM_AUDIO_DATA_SIZE;
#if 0
	printf(	"Rcv Video %d, length %d\n"
		"Rcv Audio %d, length %d\n"
		"Send Video %d, length %d\n"
		"Send Audio %d, length %d\n",
		pRcvVideo->pBuf,
		pRcvVideo->nBufLen,
		pRcvAudio->pBuf,
		pRcvAudio->nBufLen,
		pSendVideo->pBuf,
		pSendVideo->nBufLen,
		pSendAudio->pBuf,
		pSendAudio->nBufLen);
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmAVDataExit
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Oct - 2006
**
**	DESCRIPTION:	
**			Auido/video buffer free
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MmAVDataExit()
{
	MmRcvAVData*		pRcvVideo	= &MmMan.RcvVideo;
	MmRcvAVData*		pRcvAudio	= &MmMan.RcvAudio;
	MmSendAVData*		pSendVideo	= &MmMan.SendVideo;
	MmSendAVData*		pSendAudio	= &MmMan.SendAudio;

	if (MmMan.pBuf != NULL)
	{
#ifdef	MM_MEM_DEBUG
		printf("Mm: free %08X\n", (unsigned int) MmMan.pBuf);
#endif
		free(MmMan.pBuf);
		MmMan.pBuf = NULL;
		MmMan.nBufLen = 0;
	}

	pRcvVideo->pBuf		= NULL;
	pRcvVideo->nBufLen	= 0;
	pRcvVideo->bUsed	= FALSE;
	pRcvVideo->nCurLen	= 0;
	pRcvVideo->bUsed	= FALSE;

	pRcvAudio->pBuf		= NULL;
	pRcvAudio->nBufLen	= 0;
	pRcvAudio->bUsed	= FALSE;
	pRcvAudio->nCurLen	= 0;
	pRcvAudio->bUsed	= FALSE;

	pSendVideo->pBuf	= NULL;
	pSendVideo->nBufLen	= 0;
	pSendVideo->nCurLen	= 0;

	pSendAudio->pBuf	= NULL;
	pSendAudio->nBufLen	= 0;
	pSendAudio->nCurLen	= 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmVCodeInit
**	AUTHOR:			Jeff Wang
**	DATE:			26 - Dec - 2008
**
**	DESCRIPTION:	
**			Initial video encode and decode
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
#ifndef VIDEO_FLOW_DEBUG
static void
MmVCodeInit(void)
{
	INT	    nBufLen = 0;
    UCHAR   pBuf[50] = { 0 };

	DecInitFlag = FALSE;
    
	if((nBufLen = Encode_Init(pBuf,sizeof(pBuf))) <= 0)
	{
		printf("ERROR: can not get encoding header info..\n");
		return;
	}

	Decode_Init(2,pBuf,sizeof(pBuf));
	memcpy(ucVideoFrameHeader, pBuf, nBufLen);
	nVideoHeaderLen = nBufLen;
	MTSendDecargsAckCMD();
    
	DecInitFlag = TRUE;
	
#ifdef LEAVE_AV_DEBUG
    printf(" \n******************** \n");
    //printf("the header info:    nLen=%d nBufLen=%d *******\n", nVideoHeaderLen, pSendData->nCurLen);
	for (i = 0; i < nVideoHeaderLen; i++)
	{
		printf("  %x  ", ucVideoFrameHeader[i]);
	}
	printf(" \n******************** \n");
#endif
}
#endif

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmSendAV2Eth
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Oct - 2006
**
**	DESCRIPTION:	
**			Multi-media send audio/video data to ethernet module
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nType		[IN]		DataType
**				pSendData	[IN/OUT]	MmSendAVData*	
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MmSendAV2Eth(DATATYPE nType, MmSendAVData* pSendData)
{
	AVFrmHead*			pHead		= &pSendData->LastAVHead;
	unsigned char		Data[MAX_APP_FRM_LEN];
	int					nDataLen	= 0;
	int					i;
	int					nAVType		= 0;

    FILE*    FdVSave = NULL;
	
	if (pSendData->nCurLen <= 0)
	{
		return;
	}

	if (DATA_TYPE_AUDIO == nType)
	{
		nAVType = 1;
	}
	else if (DATA_TYPE_VIDEO == nType)
	{
		nAVType = 2;
	}

	pHead->nTotalPckCnt	= 
		(pSendData->nCurLen + pSendData->nMaxLenPerPack - 1) / pSendData->nMaxLenPerPack;

	for (i = 0; i < pHead->nTotalPckCnt; i++)
	{
		pHead->nPckIdx = (unsigned char) (i + 1);
		if (pHead->nPckIdx * pSendData->nMaxLenPerPack > pSendData->nCurLen)
		{
			pHead->nLen = pSendData->nCurLen % pSendData->nMaxLenPerPack;
		}
		else
		{
			pHead->nLen = pSendData->nMaxLenPerPack;
		}
		
		memcpy(Data, pHead, sizeof (AVFrmHead));
		nDataLen = sizeof (AVFrmHead);

		memcpy(	(Data + nDataLen), 
			(pSendData->pBuf + (pHead->nPckIdx - 1) * pSendData->nMaxLenPerPack), 
			pHead->nLen);
		nDataLen += pHead->nLen;
#ifdef MM_WR_SND_V_DEBUG
		if (nAVType == 2 && pHead->nFrmIdx  <=1 && pHead->nPckIdx == 1)
		{
			printf("****************Addncode Head*******************\n");
			memcpy(VBuf, ucVideoFrameHeader, nVideoHeaderLen);
			nVLen = nVideoHeaderLen;
			system("rm -fr /mox/rdwr/sndv.h264");
			printf("nVideoHeaderLen = %d\n",nVideoHeaderLen);
			for (i = 0; i < nVideoHeaderLen; i++)
			{
				printf("  %x  ", ucVideoFrameHeader[i]);
			}
			printf(" \n******************** \n");
		}
		else if (nAVType == 2)
		{			
			memcpy(VBuf+nVLen, 
				(pSendData->pBuf + (pHead->nPckIdx - 1) * pSendData->nMaxLenPerPack), 
				pHead->nLen);
			nVLen += pHead->nLen;
		}
		
		
		if (nVLen > 10 * 1024 * 1024) 
		{
			FdV = fopen("/mox/rdwr/sndv.h264", "wr");
			fseek(FdV, 0, SEEK_SET);
			
			fwrite(VBuf, nVLen, 1, FdV);
			fclose(FdV);
			nVLen = 0;
		}
#endif
		if(GetTalkLocStreamFormat()==MM_VIDEO_FORMAT_MPEG4_PAL)//由于Lite带宽受限
		{
			if (2 == nAVType 
				&& 0 == i % 10
				&& i > 0)
			{
				usleep(20*1000);
			}
		}
		else
		{
			if (2 == nAVType 
				&& 0 == i % 30
				&& i > 0)
			{
				usleep(20*1000);
			}
		}

	
		MxSendAV2Eth(nAVType, Data, nDataLen);

#ifdef SAVE_RAW_VIDEODATA
		if (NULL != (FdVSave  = fopen("/mox/rdwr/CaptureV.pcm", "a+"))) 
		{
			fwrite(Data, nDataLen, (size_t)1, FdVSave);			
			fclose(FdVSave);				
		}
		else
		{
			printf("Open CaptureV.pcm File Fail \n");
		}
#endif
	}	
	pHead->nFrmIdx++;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmSendPhoto2EHV
**	AUTHOR:			Jerry Huang
**	DATE:			30 - Aug - 2008
**
**	DESCRIPTION:	
**			Multi-media send photo data to ethernet module
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nType		[IN]		DataType
**				pSendData	[IN/OUT]	MmSendAVData*	
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID
MmSendPhoto2EHV(unsigned char *pBuf, int nBufLen,int nPicType)
{
	unsigned char		Data[MAX_APP_FRM_LEN];
	int					nDataLen	= 0;
	int					i;
	int					nAVType		= 0;
	DWORD				FileLen     = 0;
	WORD				FrameIdx	= 0;
	WORD              FrameCount	= 0;
	UCHAR				PicType     = 0;
	WORD				DataPackLen = 0;

	if (nBufLen <= 0)
	{
		return;
	}
	
	nAVType = 3;

	FrameCount	= 
		(nBufLen + PHOTO_LEN_PER_PACK - 1) / PHOTO_LEN_PER_PACK;

	FileLen = nBufLen;
	PicType = nPicType;//1:bmp 2:Jpeg Map

	for (i = 0; i < FrameCount; i++)
	{
		if (0 == ((i+1) % 25)) usleep(20 * 1000);
		
		FrameIdx			= (WORD) (i + 1);

		if (FrameIdx * PHOTO_LEN_PER_PACK > nBufLen)
		{
			DataPackLen = nBufLen % PHOTO_LEN_PER_PACK;
		}
		else
		{
			DataPackLen = PHOTO_LEN_PER_PACK;
		}

		memcpy(Data + 0,	&FileLen,		sizeof(DWORD));			
		memcpy(Data + 4,	&FrameIdx,		sizeof(WORD));
		memcpy(Data + 6,	&FrameCount,	sizeof(WORD));
		memcpy(Data + 8,	&PicType,		sizeof(UCHAR));	
		
		nDataLen	= 9;
		
		memcpy(	(Data + nDataLen), 
			(pBuf + (FrameIdx - 1) * PHOTO_LEN_PER_PACK), 
			DataPackLen);
		nDataLen += DataPackLen;

		MxSendAV2Eth(nAVType, Data, nDataLen);

		//printf("send the av date=%d to Eth.. the len=%d.\n", nAVType, nDataLen);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmSendPhoto2MC
**	AUTHOR:			Jerry Huang
**	DATE:			30 - Aug - 2008
**
**	DESCRIPTION:	
**			Multi-media send photo data to ethernet module
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nType		[IN]		DataType
**				pSendData	[IN/OUT]	MmSendAVData*	
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID
MmSendPhoto2MC(unsigned char *pBuf, int nBufLen)
{
	unsigned char		Data[MAX_APP_FRM_LEN];
	int					nDataLen	= 0;
	int					i;
	int					nAVType		= 0;

	UCHAR				PicType     = 0;
	DWORD				PicLen		= 0;
	WORD				FrameIdx	= 0;
	WORD              FrameCount	= 0;
	WORD				PicDataLen	= 0;

	if (nBufLen <= 0)
	{
		return;
	}

	nAVType = 4;

	FrameCount	= 
		(nBufLen + PHOTO_LEN_PER_PACK - 1) / PHOTO_LEN_PER_PACK;
	PicLen	=	nBufLen;
	PicType = 2;//Jpeg

	for (i = 0; i < FrameCount; i++)
	{
		if (0 == ((i+1) % 25)) usleep(20 * 1000);
		
		FrameIdx			= (WORD) (i + 1);
		
		if (FrameIdx * PHOTO_LEN_PER_PACK > nBufLen)
		{
			PicDataLen = nBufLen % PHOTO_LEN_PER_PACK;
		}
		else
		{
			PicDataLen = PHOTO_LEN_PER_PACK;
		}

		memcpy(Data + 0,  &g_LogReference,		sizeof(DWORD));
		memcpy(Data + 4,  &PicType,		sizeof(UCHAR));
		memcpy(Data + 5,  &PicLen,		sizeof(DWORD));
		memcpy(Data + 9,  &FrameIdx,	sizeof(WORD));
		memcpy(Data + 11, &FrameCount,	sizeof(WORD));
		memcpy(Data + 13, &PicDataLen,	sizeof(WORD));

		nDataLen	= 15;

		memcpy(	(Data + nDataLen), 
			(pBuf + (FrameIdx - 1) * PHOTO_LEN_PER_PACK), 
			PicDataLen);
		nDataLen += PicDataLen;
		MxSendAV2Eth(nAVType, Data, nDataLen);
		//printf("send the av date=%d to Eth.. the len=%d.\n", nAVType, nDataLen);
	}
}

#ifdef	AV_TO_MM_DEBUG
static void
AVFrmPrintHead(AVFrmHead* pHead)
{
	printf("Mm: AV Frame Head :: "
		"Frm Idx %08X, "
		"Pck Idx %02X, "
		"Pck Tot %02X, "
		"Type %02X, "
		"Cap %02X, "
		"Fmt %02X, "
		"Res %02X, "
		"Len %d\n", 
		pHead->nFrmIdx,
		pHead->nPckIdx,
		pHead->nTotalPckCnt,
		pHead->nType,
		pHead->nCapture,
		pHead->nFormat,
		pHead->Reserved, 
		pHead->nLen);
}
#endif

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmAVBufInit
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Nov - 2006
**
**	DESCRIPTION:	
**			AV  buffer initialize
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pAV			[IN/OUT]	MmAVBufStruct*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MmAVBufInit(MmAVBufStruct* pAV, int nCnt, int nBufSize)
{
	int		i;

	pAV->Head.pHead = NULL;
	pAV->Head.pTail = NULL;
	pAV->nValidAVBufCnt= 0;
	pAV->nAVBufCnt = 0;
	pthread_mutex_init(&pAV->Mutex, NULL);

	pAV->nBufLen = (sizeof (MmAVBuf) + nBufSize) * nCnt;
	pAV->pBuf = (unsigned char*) malloc(pAV->nBufLen);
	if (NULL == pAV->pBuf)
	{
		pAV->nBufLen = 0;
		return;
	}

#ifdef MM_MEM_DEBUG
	printf("Mm: malloc AV %08X\n", (unsigned int) pAV->pBuf);
#endif

#ifdef MM_AV__BUF_DEBUG
	printf("Mm: AV  Buf %u, length %d\n", (unsigned int) pAV->pBuf, pAV->nBufLen);
#endif

	memset(pAV->pBuf, 0, pAV->nBufLen);
	pAV->pAVBuf = (MmAVBuf*) pAV->pBuf;
	pAV->nAVBufCnt = nCnt;
    /*
        目录结构
               |-------------------------------|
          |-----------------------------|      |
          |    |                        |      |
        |_A_||_A_|...|_A_||_A_|(20个)|__B__||__B__|...|__B__||__B__|(20个)
    */
	for (i = 0; i < nCnt; i++)
	{
		pAV->pAVBuf[i].pBuf = pAV->pBuf + sizeof (MmAVBuf) * nCnt + nBufSize * i;
		pAV->pAVBuf[i].nBufLen = nBufSize;
		pAV->pAVBuf[i].nCurLen = 0;
		pAV->pAVBuf[i].bUsed = FALSE;
#ifdef MM_AV__BUF_DEBUG
		printf("Mm: AV Buf %u, sizeof (MmAVBuf) %d, pBuf %u, length %d\n", 
			(unsigned int) (pAV->pAVBuf + i),
			sizeof (MmAVBuf),
			(unsigned int) pAV->pAVBuf[i].pBuf,
			nBufSize);
#endif
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmAVBufExit
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Nov - 2006
**
**	DESCRIPTION:	
**			AV  buffer exit
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pAV			[IN/OUT]	MmAVBufStruct*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MmAVBufExit(MmAVBufStruct* pAV)
{
	pAV->nValidAVBufCnt= 0;
	pthread_mutex_destroy(&pAV->Mutex);
	pAV->Head.pHead = NULL;
	pAV->Head.pTail = NULL;
	
	if (pAV->pBuf != NULL)
	{
#ifdef MM_MEM_DEBUG
	printf("Mm: free AV %08X\n", (unsigned int) pAV->pBuf);
#endif
		free(pAV->pBuf);
		pAV->pBuf = NULL;
		pAV->nBufLen = 0;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AllocAVBuf
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Nov - 2006
**
**	DESCRIPTION:	
**			Get unused AV  buffer
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pAV			[IN/OUT]	MmAVBufStruct*
**	RETURNED VALUE:	
**				Buf pointer
**	NOTES:
**			
*/
static MmAVBuf*
AllocAVBuf(MmAVBufStruct* pAV)
{
	MmAVBuf*	pAVBuf	= NULL;
	int			i;

	for (i = 0; i < pAV->nAVBufCnt; i++)
	{
		if (!pAV->pAVBuf[i].bUsed)
		{
			pAVBuf = pAV->pAVBuf + i;
			break;
		}
	}

	return pAVBuf;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmAVBufPut
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Nov - 2006
**
**	DESCRIPTION:	
**			AV  buffer put
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pAV			[IN/OUT]	MmAVBufStruct*
**				pBuf		[IN]		unsigned char*
**				nBufLen		[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MmAVBufPut(MmAVBufStruct* pAV, unsigned char* pBuf, int nBufLen)
{
	MmAVBuf*	pAVBuf;	

 	pthread_mutex_lock(&pAV->Mutex);

	pAVBuf = AllocAVBuf(pAV);
	if (pAVBuf != NULL)
	{
		memcpy(pAVBuf->pBuf, pBuf, nBufLen);
		pAVBuf->nCurLen = nBufLen;
		pAVBuf->bUsed = TRUE;
		MXListAdd(&pAV->Head, (MXList*) pAVBuf);
		pAV->nValidAVBufCnt++;
	}

	pthread_mutex_unlock(&pAV->Mutex);

	if (NULL == pAVBuf)
	{
		printf("Mm: MmAVBufPut fail\n");
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmAVBufFree
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Nov - 2006
**
**	DESCRIPTION:	
**			AV  buffer free
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pAV			[IN/OUT]	MmAVBufStruct*
**				pAVBuf		[IN]		MmAVBuf*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MmAVBufFree(MmAVBufStruct* pAV, MmAVBuf* pAVBuf)
{
	pthread_mutex_lock(&pAV->Mutex);
	
	MXListRm(&pAV->Head, (MXList*) pAVBuf);
	pAV->nValidAVBufCnt--;
	pAVBuf->bUsed = FALSE;

	pthread_mutex_unlock(&pAV->Mutex);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmAVBufReset
**	AUTHOR:			Jerry Huang
**	DATE:			18 - Nov - 2006
**
**	DESCRIPTION:	
**			AV  buffer reset
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pAV			[IN/OUT]	MmAVBufStruct*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MmAVBufReset(MmAVBufStruct* pAV)
{
	int		i;

	pthread_mutex_lock(&pAV->Mutex);
	
	pAV->Head.pHead = NULL;
	pAV->Head.pTail = NULL;
	pAV->nValidAVBufCnt = 0;
	
	for (i = 0; i < pAV->nAVBufCnt; i++)
	{
		pAV->pAVBuf[i].bUsed = FALSE;
	}

	pthread_mutex_unlock(&pAV->Mutex);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MmAVFrmIdxDebug
**	AUTHOR:			Jerry Huang
**	DATE:			21 - Nov - 2006
**
**	DESCRIPTION:	
**			AV frame index debug
**
**	ARGUMENTS:	ARGNAME			DRIECTION	TYPE	DESCRIPTION
**				nDataType	[IN]		DataType
**				pHead			[IN]		AVFrmHead*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MmAVFrmIdxDebug(DATATYPE nDataType, AVFrmHead* pHead)
{
#ifdef MM_VIDEO_FRM_INDEX_DEBUG
	if (DATA_TYPE_VIDEO == nDataType)
	{
		if ((nLastVideoFrmIdx + 1) != pHead->nFrmIdx)
		{
			printf("Mm: Video gap, Last Frame Index %u, Current Frame Index %u\n",
				nLastVideoFrmIdx, pHead->nFrmIdx);
		}
		nLastVideoFrmIdx = pHead->nFrmIdx;
	}
#endif
#ifdef MM_AUDIO_FRM_INDEX_DEBUG
	if (DATA_TYPE_AUDIO == nDataType)
	{
		if ((nLastAudioFrmIdx + 1) != pHead->nFrmIdx)
		{
			printf("Mm: Audio gap, Last Frame Index %u, Current Frame Index %u\n",
				nLastAudioFrmIdx, pHead->nFrmIdx);
		}
		nLastAudioFrmIdx = pHead->nFrmIdx;
	}
#endif
}

static void
AudioPlayMemInit(AudioPlayMemManStr* pAPlayMem)
{
	pAPlayMem->nBufLen = 0;
	pAPlayMem->pBuf = NULL;
	pAPlayMem->dwPlayInterval = 0;
	pAPlayMem->nCurPlayPos = 0;
	pAPlayMem->nLenPerPlay = 0;
	pAPlayMem->bLoopEnd = FALSE;
	pAPlayMem->bPlayOnce = FALSE;
}

static void
LoadAudio2Mem(unsigned char* pFileName, AudioPlayMemManStr* pAPlayMem, DWORD dwPlayInterval, int nLenPerPlay, BOOL bPlayOnce)
{
	FILE*			f;
	struct stat		StatBuf;
    
	if (stat(pFileName, &StatBuf) != 0)
	{
		return;
	}

	pAPlayMem->nBufLen = StatBuf.st_size;

	if (NULL != pAPlayMem->pBuf)
	{
		free(pAPlayMem->pBuf);
	}

	if ( ((f = fopen(pFileName, "rb")) == NULL)	||
		 ((pAPlayMem->pBuf = (unsigned char*) malloc(pAPlayMem->nBufLen)) == NULL))
	{
		if (f != NULL)
		{
			fclose(f);
			f = NULL;
		}
		pAPlayMem->nBufLen = 0;
		return;
	}

	fread(pAPlayMem->pBuf, 1, pAPlayMem->nBufLen, f);

	pAPlayMem->dwPlayInterval = dwPlayInterval;
	pAPlayMem->nCurPlayPos = 0;
	pAPlayMem->nLenPerPlay = nLenPerPlay;
	pAPlayMem->dwNextPlayTick = GetTickCount();
	
	pAPlayMem->bLoopEnd = FALSE;
	pAPlayMem->bPlayOnce = bPlayOnce;
	fclose(f);
#ifdef CALL_NOTE_DEBUG
	printf("CO Ring: ##### start to malloc a memory. file:%s....\n", pFileName);
#endif
	
}

static void
AudioPlayMemExit(AudioPlayMemManStr* pAPlayMem)
{
	if (pAPlayMem->pBuf != NULL)
	{	
		free(pAPlayMem->pBuf);
		pAPlayMem->pBuf = NULL;
#ifdef CALL_NOTE_DEBUG
		printf("CO Ring: ******* start to free a memory .\n");
#endif
		
	}

	pAPlayMem->nBufLen = 0;
}

static BOOL
PlayMemAudio(AudioPlayMemManStr* pAPlayMem)
{
	DWORD			dwCurTick;
	int				nRealLen;
	unsigned char*	pStart		=	NULL;
	int				nPlayLen	=	0;
	int				i			=	0;

	if (pAPlayMem->nBufLen <= 0)
	{
		return FALSE;
	}

	dwCurTick = GetTickCount();

	if (dwCurTick >= pAPlayMem->dwNextPlayTick)
	{
//		if (dwCurTick - pAPlayMem->dwNextPlayTick > 100) 
//		{
//			printf("-------PlayMemAudio Tick Long: %d\n",dwCurTick - pAPlayMem->dwNextPlayTick);
//		}
		for(i = 0; i < AUDIO_PLAY_PER_PKT; i++)
		{
			if (pAPlayMem->bLoopEnd && pAPlayMem->bPlayOnce)
			{
				return TRUE;
			}
			
			pStart = pAPlayMem->pBuf + pAPlayMem->nCurPlayPos;
			
			if (pAPlayMem->nCurPlayPos + pAPlayMem->nLenPerPlay >= pAPlayMem->nBufLen)
			{
				nRealLen = pAPlayMem->nBufLen - pAPlayMem->nCurPlayPos;
			}
			else
			{
				nRealLen = pAPlayMem->nLenPerPlay;
			}

			nPlayLen = NoteAudioPlay(pStart, nRealLen);			

			if (nPlayLen < 0)
			{
#ifdef AUDIO_WR_DEBUG			
				printf("Audio Play buffer ful\n");
#endif
				break;
			}

			if (pAPlayMem->nCurPlayPos + pAPlayMem->nLenPerPlay >= pAPlayMem->nBufLen)
			{
				pAPlayMem->nCurPlayPos = 0;
				pAPlayMem->bLoopEnd = TRUE;
			}
			else
			{
				pAPlayMem->nCurPlayPos += nPlayLen;			
			}			
		}

		pAPlayMem->dwNextPlayTick = dwCurTick + pAPlayMem->dwPlayInterval;
		
		//printf("Mm: Play memory audio, tick %lu, length %d, Pos %d\n", dwCurTick, nRealLen, pAPlayMem->nCurPlayPos);
	}
	return FALSE;
}

static void
PausePlayCallA()
{
	AudioEnd();
	StopPlayCallRing();
	printf("Pause Play Call Audio\n");
}


static void
StopPlayCallRing()
{
	if(!g_PlayMusicState)
	{
		AudioPlayMemExit(&MmMan.AudioPlayMemMan);
		printf("Stop Play Call Audio\n");
	}
}

static void
StartPlayOnceSound(const char *pszRingName)
{
	//if (MmMan.nRdirEthMmAVStatus == AV_PLAY_STATUS_IDLE) 
	{
		AudioStart();
	}
	 g_PlayMusicState = StartPlayMusic(pszRingName, TYPE_PLAY_ONCE, 0, 0);	
}
BOOL IsMMRunning(void)
{
	if(MmMan.nRdirEthMmAVStatus == AV_PLAY_STATUS_RUN)
	{
		return TRUE;
	}
	return FALSE;
}
int 
ReadAVData(DATATYPE nDataType, unsigned char* pBuf, int nLen)
{
	int	nReadLen		= 0;

	switch(nDataType)
	{
	case DATA_TYPE_VIDEO:
		{	
#ifndef VIDEO_FLOW_DEBUG
			nReadLen = GetVCapData(pBuf);
#endif
		}
		break;
	case DATA_TYPE_AUDIO:
		{
#ifndef AUDIO_FLOW_DEBUG
			nReadLen = AudioCapture(pBuf);
#endif

#ifdef AUDIO_WR_DEBUG
			if (nReadLen > 0)
			{
				printf("Read Audio Data, the data len = %d\n", nReadLen);
			}
#endif

#ifdef MM_WR_SND_AU_DEBUG
			if (nRdLen < 1024 * 1024)
			{
				memcpy(&RdBuf[nRdLen], pBuf, nReadLen);
				nRdLen += nReadLen;
			}
			else if(!bRdStored)
			{
				bRdStored = TRUE;
				FdRdA = fopen("/mox/rdwr/sndau.pcm", "wr");
				fseek(FdRdA, 0, SEEK_SET);
				
				fwrite(RdBuf, nRdLen-nReadLen, 1, FdRdA);
				fclose(FdRdA);
				printf("Audio Send file saved\n");
			}
#endif
		}
		break;
	default:
		break;
	}
	
	return nReadLen;	
}

int 
WriteAVData(DATATYPE nDataType, unsigned char* pBuf, int nLen)
{	
	int nRetLen = 0;
	switch(nDataType)
	{
	case DATA_TYPE_VIDEO:
		{
#ifndef VIDEO_FLOW_DEBUG

#endif	
		}
		break;
	case DATA_TYPE_AUDIO:
		{
#ifndef AUDIO_FLOW_DEBUG
			if (MmMan.StackAVFlag.nAPlayStartCnt < 6) 
			{
				//printf("MmMan.StackAVFlag.nAPlayStartCnt = %d\n", MmMan.StackAVFlag.nAPlayStartCnt);
				return 0;
			}
			nRetLen = AudioPlay(pBuf, nLen);
#endif

#ifdef AUDIO_WR_DEBUG
			printf("Audio play, nLen = %d\n", nLen);				
#endif
		}
		break;
	default:
		break;
	}
	return nRetLen;	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MMPhotoSaveNote
**	AUTHOR:			Jeff Wang
**	DATE:			29 - July - 2008
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**			
**				
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MMPhotoSaveNote()
{
	printf("MMPhotoSaveNote old status=%d\n",FlagEnCapYuv);
	FlagEnCapYuv = 1;
	
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MMSendLeavePhotoEnd2Talk
**	AUTHOR:		   Jeff Wang
**	DATE:		06 - Oct - 2008
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
static void 
MMSendLeavePhotoEnd2Talk()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_MM;
	msgSend.dwDestMd	= MXMDID_TALKING;
	msgSend.dwMsg		= COMM_TAKEPHOTO_END;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MMSendPhotoNote2Eth
**	AUTHOR:		   Jeff Wang
**	DATE:		05 - Nov - 2008
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
static void 
MMSendPhotoNote2Eth(int nType, unsigned char* pBuf, int nBufLen)
{
	MXMSG  msgSend;
	unsigned short nDatalen = 0;

	nDatalen = (unsigned short)nBufLen;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_MM;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_REPORT_PIC;
	msgSend.pParam		= (unsigned char*)malloc(nBufLen + sizeof(unsigned short));
	memcpy(msgSend.pParam, &nDatalen, sizeof(unsigned short));
	memcpy(msgSend.pParam + sizeof(unsigned short), pBuf, nBufLen);
	msgSend.dwParam		= g_TalkInfo.dwMCIP;
	MxPutMsg(&msgSend);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AudioInit
**	AUTHOR:		   Jeff Wang
**	DATE:		31 - Dec - 2008
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
static void 
AudioInit(void)
{
	int ret = 0;
	FILE *f;
	unsigned long count = 0;

	if (DEVICE_CORAL_DIRECT_GM != g_DevFun.DevType) 
	{
		if (!g_DevFun.bNewPnUsed)
		{
			ret = SendtoFM1182(NULL, 0);
		}
	}
					

	if (-1 == ret)
	{
		printf("******************* FM1182 ERROR!!!!!!!!!!!!!! \n");
		usleep(1000 * 1000);	
		system("reboot");
		while (1);
	}

	
	printf("Audio Init...\n");
	
	mxc_ssi_fd = open(DEVICE_NAME_SSI1, O_RDWR);
	mxc_ssi2_fd = open(DEVICE_NAME_SSI2, O_RDWR);

	
	OpenIniFile(SC);
	nTalkVolume= (UINT)ReadInt(SC_SEC_GLOBAL, SC_TALK_VOLUME, SC_DEFAULT_TALK_VOLUME);
	CloseIniFile();
	SetGMTalkVolume();
    ioctl(mxc_ssi_fd, IOCTL_SSI_DMA_STOP, NULL);
//	printf("******************* AudioInit:current volume:%d*******************\n",nTalkVolume);

	PIOWrite(PIO_TYPE_AMP_PWR, AMP_PWR_OFF);
//	capture_tickcount_start = GetTickCount();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AudioStart
**	AUTHOR:		   Jeff Wang
**	DATE:		31 - Dec - 2008
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
static void 
AudioStart(void)
{
	ioctl(mxc_ssi_fd, IOCTL_SSI_DMA_STOP, NULL);
//	ioctl(mxc_ssi2_fd, IOCTL_SSI_DMA_STOP, NULL);
	ioctl(mxc_ssi_fd, IOCTL_SSI_DMA_TRIGGER, NULL);
//	ioctl(mxc_ssi2_fd, IOCTL_SSI_DMA_TRIGGER, NULL);
	PIOWrite(PIO_TYPE_AMP_PWR, AMP_PWR_ON);
	TelLogStr("Audio Capture Start");
    if(SERIES_SHELL == g_DevFun.SeriesType || SERIES_MINI_SHELL == g_DevFun.SeriesType) 
    {
        SetGMTalkVolume();
    }    
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AudioEnd
**	AUTHOR:		   Jeff Wang
**	DATE:		31 - Dec - 2008
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
static void 
AudioEnd(void)
{
	if(!g_PlayMusicState)
	{
		ioctl(mxc_ssi_fd, IOCTL_SSI_DMA_STOP, NULL);
		ioctl(mxc_ssi2_fd, IOCTL_SSI_DMA_STOP, NULL);
		PIOWrite(PIO_TYPE_AMP_PWR, AMP_PWR_OFF);
		TelLogStr("Audio Capture End\n");
	}
}


static int 
NoteAudioPlay(char *pBuf, int nLen)
{
	int bytes_read = 0;
	int bytes_written = 0;
	char* p = NULL;
	int		 i  = 0;
	SHORT nTemp = 0;
	char* pUse = NULL;
//	struct run_status ssi_status;

	g_audio_play_ticks = GetTickCount();

	pUse = (char*)malloc(nLen);

	memcpy(pUse, pBuf, nLen);

	p = pUse;
	
	bytes_read  = nLen;

	for (i = 0; i < bytes_read /2; i++)		
	{		
		memcpy(&nTemp, &pUse[i * 2], 2);		
		nTemp /= 4;		
		memcpy(&pUse[i * 2], &nTemp, 2);		
	}	

	//bytes_written = write(mxc_ssi2_fd, pUse, bytes_read);
	bytes_written = write(mxc_ssi_fd, pUse, bytes_read);
	if (-1 == bytes_read  && errno != EINTR)
	{
		return -1;
	}
	
	if (bytes_written > 0) 
	{
//		playback_error_times = 0;
		dwDiag[DIAG_A_RCV] += bytes_written;
	}

#ifdef AUDIO_WR_DEBUG
	printf("%d bytes write finished\n", bytes_written);
#endif
	
	if(bytes_written == -1)
	{
/*		playback_error_times++;
		if (playback_error_times > 60)
		{
			playback_error_times = 0;
			ioctl(mxc_ssi_fd, IOCTL_SSI_GET_STATUS, (void *)(&ssi_status));
			EthGMLog_ssi_playback(ssi_status);
		}
*/		//printf("error\n");
	}
	free(pUse);
	pUse = NULL;
	return bytes_written;
}

static int 
AudioPlay(char *pBuf, int nLen)
{
	int bytes_read = 0;
	int bytes_written = 0;
	char* p = NULL;
	int		 i  = 0;
	SHORT nTemp = 0;
	char* pUse = NULL;
	DWORD time = GetTickCount();
//	struct run_status ssi_status;

	if(	(time >= g_audio_play_ticks)&&
		(AUDIO_MASK_TIME >= (time - g_audio_play_ticks)) )
		return nLen;
	
	pUse = (char*)malloc(nLen);

	memcpy(pUse, pBuf, nLen);

	p = pUse;
	
	bytes_read  = nLen;

	for (i = 0; i < bytes_read /2; i++)		
	{		
		memcpy(&nTemp, &pUse[i * 2], 2);		
		nTemp /= 4;		
		memcpy(&pUse[i * 2], &nTemp, 2);		
	}	

	bytes_written = write(mxc_ssi_fd, pUse, bytes_read);
	
	if (-1 == bytes_read  && errno != EINTR)
	{
		return -1;
	}
	
	if (bytes_written > 0) 
	{
//		playback_error_times = 0;
		dwDiag[DIAG_A_RCV] += bytes_written;
	}

#ifdef AUDIO_WR_DEBUG
	printf("%d bytes write finished\n", bytes_written);
#endif
	
	if(bytes_written == -1)
	{
/*		playback_error_times++;
		if (playback_error_times > 60)
		{
			playback_error_times = 0;
			ioctl(mxc_ssi_fd, IOCTL_SSI_GET_STATUS, (void *)(&ssi_status));
			EthGMLog_ssi_playback(ssi_status);
		}
*/		//printf("error\n");
	}
	free(pUse);
	pUse = NULL;
	return bytes_written;
}

//static int temp = 0;

static int 
AudioCapture(char* pBuf)
{
	int bytes_read		 =   0;
	int bytes_written	 =   0;
	char* p					   =   NULL;
	int		i					    =	0;
	SHORT nTemp		=	0;
//	struct run_status ssi_status;

	p = pBuf;	
/*
	if (temp == 0)
	{
			printf("**************** OK11111111111\n");
			ioctl(mxc_ssi_fd, IOCTL_SSI_GET_STATUS, (void *)(&ssi_status));
			printf("**************** OK22222222222\n");
			//printf("*************** ssi_status.reg[0]: 0x%lx. \n", ssi_status.reg[0]);

			EthGMLog_ssi_playback(ssi_status);
			printf("****************************************** OKKKKKKKKKKKKKKKKKKKKKK!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			temp = 1;
	}
*/
	bytes_read = read(mxc_ssi_fd, p, MAX_CHUNK_SIZE);
	if((bytes_read == -1) && (errno != EINTR)){
/*		capture_error_times++;
		capture_tickcount_end = GetTickCount();
		if ( (capture_error_times > 50) && (capture_tickcount_end - capture_tickcount_start) > 2000 )
		{
			capture_error_times = 0;
			capture_tickcount_start = GetTickCount();
			capture_tickcount_end = GetTickCount();	
			ioctl(mxc_ssi_fd, IOCTL_SSI_GET_STATUS, (void *)(&ssi_status));
			EthGMLog_ssi_playback(ssi_status);
		}
*/
		//printf("error\n");
		return -1;
	}
	
	for (i = 0; i< bytes_read; i+= 4)		
	{		
		memcpy(&p[i], &p[i+2], 2);		
	}
	
	if (bytes_read > 0)		
	{		
//		capture_error_times = 0;
//		capture_tickcount_start = GetTickCount();
//		capture_tickcount_end = GetTickCount();

		for (i =0; i< bytes_read / 2; i++)			
		{			
			memcpy(&nTemp, &p[2 * i], 2);

			if (nTemp > 8191)				
			{
				nTemp = 32767;				
			}
			else if (nTemp < -8191)				
			{				
					nTemp = -32768;		
			}
			else 
			{
				nTemp *= 4;	
			}
//			nTemp = bufTemp++;
			memcpy(&p[2 * i], &nTemp, 2);
		}		
		//printf("bufTemp= %d\n", bufTemp);
	}
#ifdef AUDIO_WR_DEBUG
	if (bytes_read > 0)
	{
		printf("%d bytes record finished\n", bytes_read);
	}
#endif
	return bytes_read;
}

static void
ProcessPlayAudio()
{
	if (MmMan.nPlayPromptStatus == AV_PLAY_STATUS_RUN) 
	{
		if(PlayMemAudio(&MmMan.AudioPlayMemMan))
		{
            AudioEnd();         //stop loudspeaker
            StopPlayCallRing(); //added by [MichaelMa] for avoiding memory leak
			MmMan.nPlayPromptStatus = AV_PLAY_STATUS_IDLE;
#ifdef ONCEPLAY_DEBUG
			printf("the prompt once play end.  stop play\n");
#endif            
            if(1 == g_uPlayingRing)
            {
                /*Excuted when DI finished playing sound(DI1.pcm or DI2.pcm) during talking*/
                GMStartCallRing();
            }
            if(
                ST_CO_TALKING == g_TalkInfo.talking.dwTalkState || 
                ST_CI_TALKING == g_TalkInfo.talking.dwTalkState
            )
            {
                GMStartTalking();
            }
		}
	}

	if (MmMan.nPlayCallAStatus == AV_PLAY_STATUS_RUN)
	{
		if(PlayMemAudio(&MmMan.AudioPlayMemMan))
		{
#ifdef ONCEPLAY_DEBUG
			printf("the once play end.  stop play\n");
#endif
    		AudioEnd();
            StopPlayCallRing(); //added by [MichaelMa] for avoiding memory leak
    		MmMan.nPlayCallAStatus = AV_PLAY_STATUS_IDLE;
		}
	}
	if (MmMan.nPlayAlarmAStatus == AV_PLAY_STATUS_RUN)
	{
		if (PlayMemAudio(&MmMan.AudioPlayMemMan)) 
		{
		    AudioEnd();
            StopPlayCallRing(); //added by [MichaelMa] for avoiding memory leak
		    MmMan.nPlayAlarmAStatus = AV_PLAY_STATUS_IDLE;
        }
	}
}

/*hdr
**	Copyright MOX Products, Australia
**
**	FUNCTION NAME: StartPlayMusic
**	AUTHOR:			Harry	Qian
**	DATE:			4 - Sep - 2008
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pFileName   [IN]     const char*	filename of the music file
**				bPlayMode	[IN]	BYTE		the play mode, once ,cycle
**				dwPlayTime	[IN]	DWORD		the time want to play
**				nCycleCnt	[IN]	UINT		count want to cycle.
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static BOOL
StartPlayMusic(const char * pFileName, BYTE bPlayMode, DWORD dwPlayTime, UINT nCycleCnt)
{
	LoadAudio2Mem(pFileName, &MmMan.AudioPlayMemMan, AUDIO_PLAY_INTERVAL, AUDIO_PLAY_PER_LEN, bPlayMode);
	
	if (bPlayMode == TYPE_PLAY_RECYCLE && dwPlayTime != NO_TIME_LIMIT) 
	{
		nTimer_id = SetTimer(NULL, MM_TIMER, dwPlayTime, (TIMERPROC)lpTimerProcess); 
	}
#ifdef CALL_NOTE_DEBUG
	printf("Start playing call note\n");
#endif	
	return bPlayMode;
}

static VOID
StartPlayAlarmA()
{
	AudioStart();
	g_PlayMusicState = StartPlayMusic(FILE_ALARM_SOUND, TYPE_PLAY_RECYCLE, NO_TIME_LIMIT, 0);	
	MmMan.nPlayAlarmAStatus = AV_PLAY_STATUS_RUN;
}

static VOID
StopPlayAlarmA()
{
	if (MmMan.nPlayAlarmAStatus == AV_PLAY_STATUS_RUN)
	{
		AudioEnd();
		StopPlayCallRing();
	}
	MmMan.nPlayAlarmAStatus = AV_PLAY_STATUS_IDLE;
}


/*hdr
**	Copyright MOX Products, Australia
**
**	FUNCTION NAME: lpTimerProcess
**	AUTHOR:  Harry	Qian
**	DATE:    4 - Sep - 2008
**
**	DESCRIPTION:	
**			
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static TIMERPROC CALLBACK 
lpTimerProcess()
{
	KillTimer(NULL, nTimer_id);
	nTimer_id = 0;
	StopPlayCallA();
	return 0;
}

static void
StopPlayCallA()
{
	AudioPlayMemExit(&MmMan.AudioPlayMemMan);
    ioctl(mxc_ssi_fd, IOCTL_SSI_DMA_STOP, NULL);//[MichaelMa] 2014-4-29
}

VOID
StartPlayDI1ANote()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= NULL;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_PLY_DI1_A;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}
VOID
StartPlayDI2ANote()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= NULL;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_PLY_DI2_A;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}

VOID
StartPlayRightANote()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= NULL;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_PLY_RIGHT_A;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}


VOID
StartPlayArrearsNote()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= NULL;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_PLY_ARREARS_A;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}


VOID
StartPlayErrorANote()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= NULL;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_PLY_ERROR_A;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}

VOID
StartPlayAlarmANote()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= NULL;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_PLY_ALM_A;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}
VOID
StopPlayAlarmANote()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= NULL;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_STP_ALM_A;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}


static VOID
MMSendLogPicEnd2Talk()
{
	MXMSG  MsgSend;
	
	memset(&MsgSend, 0, sizeof(MXMSG));
	strcpy(MsgSend.szSrcDev, "");
	strcpy(MsgSend.szDestDev, "");
	
	MsgSend.dwDestMd	= MXMDID_TALKING;
	MsgSend.dwSrcMd		= MXMDID_MM;
	MsgSend.dwMsg		= FC_ACK_REPORT_LOGDATA;	
	MsgSend.pParam		= NULL;
	
	MxPutMsg(&MsgSend);
}


VOID
MMYUV2JPGNote()
{
	MXMSG  MsgSend;
	
	memset(&MsgSend, 0, sizeof(MXMSG));
	strcpy(MsgSend.szSrcDev, "");
	strcpy(MsgSend.szDestDev, "");
	
//	MsgSend.dwDestMd	= MXMDID_PARASET;
	MsgSend.dwDestMd	= MXMDID_YUV2JPG;
	MsgSend.dwSrcMd		= MXMDID_MM;
	MsgSend.dwMsg		= COMM_YUV2JPG;	
	MsgSend.pParam		= NULL;
	
	MxPutMsg(&MsgSend);
}


static VOID
CountByteRate(INT DataBytes)
{
	int i = 0;
	
	if (g_ByteRate.nCurFrmCnt < MAX_FRM_CNT) 
	{
		g_ByteRate.nByteOneFrm[g_ByteRate.nCurFrmCnt++] = DataBytes;
	}
	else if (MAX_FRM_CNT == g_ByteRate.nCurFrmCnt) 
	{
		for(i=0;i<(MAX_FRM_CNT-1);i++)
		{
			g_ByteRate.nByteOneFrm[i] = g_ByteRate.nByteOneFrm[i+1];
		}
		g_ByteRate.nByteOneFrm[MAX_FRM_CNT-1] = DataBytes;
	}
	
	g_ByteRate.nTotalByte = 0;
	for(i=0;i<MAX_FRM_CNT;i++)
	{
		g_ByteRate.nTotalByte += g_ByteRate.nByteOneFrm[i];
	}
}






static void
StartPlayKeySound()
{
	/*printf("StartPlayKeySound................\n\n");
	PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_ON);
//	MmMan.bBeepShort = TRUE;				
//	MmMan.bBeepError = FALSE;
//	MmMan.bBeepRight = FALSE;
//	MmMan.dwBeepShortTick = GetTickCount();
	usleep(BEEP_SHORT_TIME*1000);
	PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_OFF);
	*/
	StartPlayRightSound();
}

static void
StartPlayErrorSound()
{
	printf("StartPlayErrorSound................\n\n");
	PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_ON);
	MmMan.bBeepError = TRUE;				
	MmMan.bBeepShort = FALSE;
	MmMan.bBeepRight = FALSE;
	MmMan.dwBeepShortTick = GetTickCount();
	MmMan.nBeepErrorCount = 5;
	
}


static void
StartPlayRightSound()
{
	printf("StartPlayRightSound................\n\n");
	PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_ON);
	MmMan.bBeepRight = TRUE;				
	MmMan.bBeepShort = FALSE;
	MmMan.bBeepError = FALSE;
	MmMan.dwBeepRightTick = GetTickCount();
	MmMan.nBeepErrorCount = 0;
}


void 		
MultiTimeProcess(void)
{
	if(MmMan.bBeepShort && (GetTickCount() - MmMan.dwBeepShortTick) > BEEP_SHORT_TIME)
	{
		MmMan.bBeepShort = FALSE;
		MmMan.dwBeepShortTick = GetTickCount();				
		PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_OFF);
	}
	else if(MmMan.bBeepRight && (GetTickCount() - MmMan.dwBeepRightTick) > BEEP_RIGHT_TIME)
	{
		MmMan.bBeepRight = FALSE;
		MmMan.dwBeepRightTick = GetTickCount();				
		PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_OFF);
	}
	else if(MmMan.bBeepError && (GetTickCount() - MmMan.dwBeepShortTick) > BEEP_ERROR_TIME)
	{
		if(5 == MmMan.nBeepErrorCount)
		{
			MmMan.nBeepErrorCount --;
			MmMan.dwBeepShortTick = GetTickCount();
			PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_OFF);
		}
		else if(4 == MmMan.nBeepErrorCount)
		{
			MmMan.nBeepErrorCount --;
			MmMan.dwBeepShortTick = GetTickCount();
			PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_ON);
		}
		else if(3 == MmMan.nBeepErrorCount)
		{
			MmMan.nBeepErrorCount --;
			MmMan.dwBeepShortTick = GetTickCount();
			PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_OFF);
		}
		else if(2 == MmMan.nBeepErrorCount)
		{
			MmMan.nBeepErrorCount --;
			MmMan.dwBeepShortTick = GetTickCount();
			PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_ON);
		}
		else if(1 == MmMan.nBeepErrorCount)
		{
			MmMan.nBeepErrorCount --;
			MmMan.dwBeepShortTick = GetTickCount();
			PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_OFF);
			MmMan.bBeepShort = FALSE;
		}
		else
		{
			PIOWrite(PIO_TYPE_BEEP_CONTROL, BEEP_CONTROL_OFF);
			MmMan.bBeepShort = FALSE;
		}
	}	
}





static void
MmSendAV2EthWorkThreadFun(void* arg)
{
	MmAVBuf*		pAVBuf;	
	while (1)
	{      
		while ((MmVideoSendBuf.nValidAVBufCnt > 0) && (MmVideoSendBuf.Head.pHead != NULL))
		{
			pAVBuf = (MmAVBuf*) MmVideoSendBuf.Head.pHead;
            /*
                拷贝这一份缓冲区中的数据到发送缓冲区
            */
			memcpy(MmMan.SendVideo.pBuf,pAVBuf->pBuf, pAVBuf->nCurLen);
			MmMan.SendVideo.nCurLen=pAVBuf->nCurLen;
            
            MmSendAV2Eth(DATA_TYPE_VIDEO, &MmMan.SendVideo);
            /*
                将这个标记从队列中移除，并初始化这个缓冲区以便接下来的视频数据使用
            */
            MmAVBufFree(&MmVideoSendBuf, pAVBuf);
		}
		usleep(1000);
	}
	pthread_exit(0);
}
static int
MmAVSendBufPut( void)
{
	MmAVBuf*	pAVBuf;	
	MmAVBufStruct* pAV;
	
	pAV=&MmVideoSendBuf;
	pthread_mutex_lock(&pAV->Mutex);
    /*
        从20个缓冲区中取出一个没有使用的缓冲区 
    */
	pAVBuf = AllocAVBuf(pAV);
	if (pAVBuf != NULL)
	{
        /*
            将视频捕获缓冲区中的数据全部取出来放到目标缓冲区，然后清空
            视频捕获缓冲区以便接收接下来的视频数据
        */
		pAVBuf->nCurLen = ReadAVData(DATA_TYPE_VIDEO, pAVBuf->pBuf, MmMan.SendVideo.nBufLen);
		if(pAVBuf->nCurLen)
		{
            /*
                标记本缓冲区已经被使用了
            */
			pAVBuf->bUsed = TRUE;
            /*
                将刚刚的数据加入队列等待处理
            */
			MXListAdd(&pAV->Head, (MXList*) pAVBuf);
			pAV->nValidAVBufCnt++;
		}
	}
	
	pthread_mutex_unlock(&pAV->Mutex);
	
	if (NULL == pAVBuf)
	{
		printf("Mm: MmAVSendBufPut fail\n");
		return 0;
	}
	
	return pAVBuf->nCurLen;
}

BOOL IfCX20707(void)
{
	if(AUDIO_MODULE_CX20707 == g_SysConfig.AudioMode)
	{
		return TRUE;
	}
	return FALSE;
}




static void
SetGMVolume(int nVolume, int nMode)
{

	int					nParaLen = 0;
	BYTE				nVolumeDefault[20]={0x02, 0x05, 0x08, 0x0B, 0x0F, 
											0x14, 0x18, 0x1B, 0x1D, 0x1F, 
											0x22, 0x24, 0x26, 0x28, 0x2B, 
											0x2D, 0x2F, 0x31, 0x33, 0x35};
	BYTE				nVolume20707_Loc[20]={-74,-20,-18,-16,-15,-14,-13,-12,-11,
											-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0};

	BYTE             nVolume20707_Net[20]={-74,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,
											-10,-9,-8,-7,-6,-5,-4,-3};

	BYTE	*			pPara = NULL;
	BYTE				ucPara[100];
	
	if(IfCX20707())
	{
		if(VOLUME_MODE_LOCAL==nMode)
		{
			pPara=nVolume20707_Loc;
			nParaLen = sizeof (nVolume20707_Loc);
		}
		else if(VOLUME_MODE_NET==nMode)
		{
			pPara=nVolume20707_Net;
			nParaLen = sizeof (nVolume20707_Net);
		}
		else
		{
			printf("%s Error nMode=%d\n",__FUNCTION__,nMode);
		}

	}
	else
	{
		pPara=nVolumeDefault;
		nParaLen = sizeof (nVolumeDefault);
	}
		
	if (nVolume < 0)
	{
		nVolume=0;
	}
	if (nVolume > 19)
	{
		nVolume = 19;
	}
	printf("audio set the volume = %x.----index =%d.\n", pPara[nVolume], nVolume);
	ioctl(mxc_ssi_fd, IOCTL_SSI_CX20707, pPara[nVolume]);
}

void
SetGMTalkVolume(void)
{
	SetGMVolume(nTalkVolume, VOLUME_MODE_NET);	
}

void
TalkVolumeChangedNotify2MM(UINT nTalkVol)
{
	nTalkVolume = nTalkVol;
}

void
GMSetVolumeAPI(int nVolume)
{
	SetGMVolume(nVolume, VOLUME_MODE_NET);
    ioctl(mxc_ssi_fd, IOCTL_SSI_DMA_STOP, NULL);
}

	
