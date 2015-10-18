
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SysLogProc.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		23 - Jun - 2009
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				
**				
**	NOTES:
** 
*/
/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<errno.h>
#include <sys/ioctl.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXCommon.h"
#include "MXTypes.h"
#include "SysLogProc.h"
#include "rtc.h"

/************** DEFINES **************************************************************/
#define ETHGM_VIDEOLOG_FILE			"/mox/rdwr/Video.log"
#define ETHGM_VIDEODATA_FILE		"/mox/rdwr/Video.data"
#define ETHGM_LOG_FILE			"/mox/rdwr/EthGM.log"
#define ETHGM_LOG_BAK_FILE		"/mox/rdwr/EthGMBak.log"
#define ETHGM_LOG_MAX_SIZE		(512 * 1024)

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//extern void	DecodePhoto(unsigned char* pDisBuf);

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static pthread_mutex_t		LogMutex;

static int					nBootFlag = 0;

static char *	pVideoLogData = NULL;

/*************************************************************************************/

VOID
SysLogInit()
{
	pthread_mutex_init(&LogMutex, NULL);
}

VOID 
EthGMLog(SYSLOGTYPE LogType)
{
	FILE*			f;
	CHAR			Data[256];
	struct stat		buf;	

	
	pthread_mutex_lock(&LogMutex);
	
	GetStrRtc(Data);

	if (stat(ETHGM_LOG_FILE, &buf) == -1)
	{
		if (ENOENT != errno)
		{
			pthread_mutex_unlock(&LogMutex);
			return;
		}
	}
	else
	{
		if (buf.st_size >= ETHGM_LOG_MAX_SIZE)
		{
			rename(ETHGM_LOG_FILE, ETHGM_LOG_BAK_FILE);
		}
	}
	
	if ((f = fopen(ETHGM_LOG_FILE, "a+")) == NULL)
	{
		pthread_mutex_unlock(&LogMutex);
		return;
	}	

	if (TYPE_REBOOT == LogType) 
	{
		strcat(Data, "Reboot The system\n");
		nBootFlag = 1;
	}
	else if (TYPE_VIDEO_READ_ERROR == LogType)
	{
		strcat(Data, "VPU data error\n");
	}
	else if (TYPE_NO_A_CAP == LogType)
	{
		strcat(Data, "No Audio Capture and reboot\n");
	}
	else if(TYPE_VIDEO_SEND_ERROR==LogType)
	{
		strcat(Data, "Video Send Error and reboot\n");
	}
	
	fwrite(Data, strlen(Data), 1, f);

	fclose(f);
	
	pthread_mutex_unlock(&LogMutex);
	
	sync();
}


VOID 
EthGMLog_videologopen()
{
	printf("EthGMLog_videologopen\n");
	pVideoLogData = malloc(ETHGM_LOG_MAX_SIZE);
}

VOID 
EthGMLog_videologclose()
{
	printf("EthGMLog_videologclose\n");
	if(pVideoLogData!=NULL)
	{
		free(pVideoLogData);
		pVideoLogData = NULL;
	}
}

VOID 
EthGMLog_videologadd(char *str)
{
	CHAR			Data[256];
	if(pVideoLogData!=NULL)
	{
		GetStrRtc(Data);
		strcat(pVideoLogData, Data);
		strcat(pVideoLogData, str);
	}
}



VOID 
EthGMLog_videolog()
{
	FILE*			f;

	struct stat		buf;	

	if(pVideoLogData==NULL || strlen(pVideoLogData) <= 0)
	{
		return ;
	}

	
	pthread_mutex_lock(&LogMutex);
	
	
	if ((f = fopen(ETHGM_VIDEOLOG_FILE, "a+")) == NULL)
	{
		pthread_mutex_unlock(&LogMutex);
		return;
	}	
	
	fwrite(pVideoLogData, strlen(pVideoLogData), 1, f);

	fclose(f);
	pVideoLogData[0] = 0;
	pthread_mutex_unlock(&LogMutex);
	
	sync();
}

VOID 
EthGMLog_videodata(char *str,int len)
{
	FILE*			f;

	struct stat		buf;	
	if(str == NULL)
		return;
	if(nBootFlag)
	{
		printf("log:WriteVideoData\n");
		pthread_mutex_lock(&LogMutex);
		if ((f = fopen(ETHGM_VIDEODATA_FILE, "w")) == NULL)
		{
			pthread_mutex_unlock(&LogMutex);
			return;
		}	
		fwrite(str, len, 1, f);
		fclose(f);
		pthread_mutex_unlock(&LogMutex);
		sync();
		nBootFlag = 0;
	}
	
	
}



VOID 
EthGMLog_ssi_playback(struct run_status ssi_status)
{
	FILE*			f;
	CHAR			Data[1024];
	struct stat		buf;	

	CHAR			pOut[1024]={0};
	
	pthread_mutex_lock(&LogMutex);
	
	GetStrRtc(Data);

	if (stat(ETHGM_LOG_FILE, &buf) == -1)
	{
		if (ENOENT != errno)
		{
			pthread_mutex_unlock(&LogMutex);
			return;
		}
	}
	else
	{
		if (buf.st_size >= ETHGM_LOG_MAX_SIZE)
		{
			rename(ETHGM_LOG_FILE, ETHGM_LOG_BAK_FILE);
		}
	}
	
	if ((f = fopen(ETHGM_LOG_FILE, "a+")) == NULL)
	{
		pthread_mutex_unlock(&LogMutex);
		return;
	}	

	//printf("******************* fprintf start!\n");

	sprintf(pOut, "reg: 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx. playback_total_size: %d. playback_dmaDone: %d\n",
		ssi_status.reg[0], ssi_status.reg[1], ssi_status.reg[2], ssi_status.reg[3], ssi_status.reg[4], 
		ssi_status.reg[5], ssi_status.reg[6], ssi_status.reg[7], ssi_status.reg[8], ssi_status.reg[9], 
		ssi_status.reg[10], ssi_status.reg[11], ssi_status.reg[12],
		ssi_status.playback_total_size, ssi_status.playback_dmaDone);

	//printf("******************* fprintf end!\n");

	strcat(Data, pOut);

	fwrite(Data, strlen(Data), 1, f);

	fclose(f);
	
	pthread_mutex_unlock(&LogMutex);
	
	sync();
}




