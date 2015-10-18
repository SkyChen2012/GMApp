
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <sys/timeb.h>
#include <time.h>

#include "MXTypes.h"
#include "MXCommon.h"
#include "Dispatch.h"
#include "MXMdId.h"
#include "MXMsg.h"

static INT RtcFd = 0;
VOID SetRtcTime(BYTE* pInCurTime);
time_t GetRtcTime();
VOID
RtcInit()
{
	CHAR *dev  = "/dev/rtc0";

	RtcFd = open(dev, O_RDONLY);
	if(-1 == RtcFd){ 			
		printf("Can't Open Rtc Dev\n");
	}
	DpcAddMd(MXMDID_RTC,NULL);
}

static VOID RtcClearResource(MXMSG *pmsg)
{
	if (NULL != pmsg->pParam)
	{
		free(pmsg->pParam);
		pmsg->pParam = NULL;
	}
}

VOID SetRtcProc()
{
	MXMSG	msgRecev;
	time_t	tmCurtime = 0;
	time_t  tmtime = 0;
	time_t   t;
	struct tm *ptmTime = 0;	
	struct rtc_time rtc_tm;
	struct   tm   strTmt;
	
	memset(&msgRecev, 0, sizeof(msgRecev));
	msgRecev.dwDestMd	= MXMDID_RTC;
	msgRecev.pParam		= NULL;	
	while(MxGetMsg(&msgRecev));
	//if (MxGetMsg(&msgRecev))
	//{
		switch(msgRecev.dwMsg)
		{
			case FC_DTM_ADJUST:
				tmCurtime = GetRtcTime();
				tmtime=msgRecev.pParam[6]|msgRecev.pParam[7]<<8|msgRecev.pParam[8]<<16|msgRecev.pParam[9]<<24;                              

				rtc_tm.tm_year=msgRecev.pParam[0]+2000-1900;
				rtc_tm.tm_mon=msgRecev.pParam[1]-1;
				rtc_tm.tm_mday=msgRecev.pParam[2];
				rtc_tm.tm_hour=msgRecev.pParam[3];
				rtc_tm.tm_min=msgRecev.pParam[4];
				rtc_tm.tm_sec=msgRecev.pParam[5];
			
				printf("Date/time %d-%d-%d,%02d:%02d:%02d:",
				rtc_tm.tm_mday, rtc_tm.tm_mon +1,  rtc_tm.tm_year +1900,
				rtc_tm.tm_hour,  rtc_tm.tm_min, rtc_tm.tm_sec);
				
				printf("SetRtcProc:%ld  tmtime %ld\r\n",tmCurtime,tmtime);
				tmtime=tmCurtime-tmtime;
                printf("SetRtcProc:%ld  tmtime %ld\r\n",tmCurtime,tmtime);
				tmCurtime = mktime(& rtc_tm);
				
				tmCurtime=tmCurtime+tmtime;

				ptmTime=localtime(&tmCurtime);

				printf("Date/time %d-%d-%d,%02d:%02d:%02d:",
				ptmTime->tm_mday, ptmTime->tm_mon +1, ptmTime->tm_year +1900,
				ptmTime->tm_hour, ptmTime->tm_min, ptmTime->tm_sec);
				

				rtc_tm.tm_year=ptmTime->tm_year;
				rtc_tm.tm_mon=ptmTime->tm_mon;
				rtc_tm.tm_mday=ptmTime->tm_mday;
				rtc_tm.tm_hour=ptmTime->tm_hour;
				rtc_tm.tm_min=ptmTime->tm_min;
				rtc_tm.tm_sec=ptmTime->tm_sec;
				
				if ( ioctl(RtcFd, RTC_SET_TIME, &rtc_tm) < 0)
				{
					printf("*******SetRtcTime error*******\n");
				//	return FALSE;
				}

				strTmt.tm_year=rtc_tm.tm_year;
				strTmt.tm_mon=rtc_tm.tm_mon;
				strTmt.tm_mday=rtc_tm.tm_mday;
				strTmt.tm_hour=rtc_tm.tm_hour;
				strTmt.tm_min=rtc_tm.tm_min;
				strTmt.tm_sec=rtc_tm.tm_sec;

				t=   mktime(&strTmt); 
				stime(&t);

				fprintf(stderr, "Set Current RTC date/time to %d-%d-%d, %02d:%02d:%02d.\n",
				rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
				rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
			
				if(GetFocus() == g_WndMan.MainWndHdle)
				{
				  ResetTimer(g_WndMan.MainWndHdle, 101, 1000, NULL);//for bug 11634
				}
			
				default:
			
					break;
			
			}
		
		RtcClearResource(&msgRecev);	


	//}
}
	
	
time_t
GetRtcTime()
{
	time_t	tmCurtime = 0;
	struct rtc_time rtc_tm;
	struct tm	tmTime = { 0 };	
	
	if ( ioctl(RtcFd, RTC_RD_TIME, &rtc_tm) < 0)
	{
		printf("*******GetRtcTime error*******\n");
		return 0;
	}
	tmTime.tm_year = rtc_tm.tm_year;
	tmTime.tm_mon = rtc_tm.tm_mon;
	tmTime.tm_mday = rtc_tm.tm_mday;
	tmTime.tm_hour = rtc_tm.tm_hour;
	tmTime.tm_min = rtc_tm.tm_min;
	tmTime.tm_sec = rtc_tm.tm_sec;
	printf("NOW date/time to %d-%d-%d, %02d:%02d:%02d.\n",    rtc_tm.tm_mday,rtc_tm.tm_mon,rtc_tm.tm_year, rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec);
		
	tmCurtime = mktime(&tmTime);

	return tmCurtime;
}

void
ReadRtcTime(DWORD *RTCTimeBuf)
{
	time_t	tmCurtime = 0;
	struct rtc_time rtc_tm;
	struct tm	tmTime = { 0 };	
	
	if ( ioctl(RtcFd, RTC_RD_TIME, &rtc_tm) < 0)
	{
		printf("*******GetRtcTime error*******\n");
		return 0;
	}
	RTCTimeBuf[0] = rtc_tm.tm_year;
	RTCTimeBuf[1] = rtc_tm.tm_mon;
	RTCTimeBuf[2] = rtc_tm.tm_mday;
	RTCTimeBuf[3] = rtc_tm.tm_hour;
	RTCTimeBuf[4] = rtc_tm.tm_min;
	RTCTimeBuf[5] = rtc_tm.tm_sec;		

	
	
}


time_t
GetSysTime()
{
	time_t	tmCurtime = 0;
	
	struct timeb tmCurDate = {0};
	
	if (0 == ftime(&tmCurDate)) 
	{
		tmCurtime = tmCurDate.time; 
	}
	
	return tmCurtime;
}


VOID
SetRtcTime(BYTE* pInCurTime)
{
	struct rtc_time rtc_tm;
	struct   tm   strTmt;
	time_t   t;
	rtc_tm.tm_year  = 2000 + pInCurTime[0] - 1900;
	rtc_tm.tm_mon   = pInCurTime[1] - 1;
	rtc_tm.tm_mday  = pInCurTime[2];
	rtc_tm.tm_hour  = pInCurTime[3];
	rtc_tm.tm_min   = pInCurTime[4];
	rtc_tm.tm_sec   = pInCurTime[5];	

	if ( ioctl(RtcFd, RTC_SET_TIME, &rtc_tm) < 0)
	{
		printf("*******SetRtcTime error*******\n");
	//	return FALSE;
	}

	strTmt.tm_year=rtc_tm.tm_year;
	strTmt.tm_mon=rtc_tm.tm_mon;
	strTmt.tm_mday=rtc_tm.tm_mday;
	strTmt.tm_hour=rtc_tm.tm_hour;
	strTmt.tm_min=rtc_tm.tm_min;
	strTmt.tm_sec=rtc_tm.tm_sec;
		
	 t   =   mktime(&strTmt); 
	stime(&t);
	
	fprintf(stderr, "Set Current RTC date/time to %d-%d-%d, %02d:%02d:%02d.\n",
		rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
		rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
		
		

}


VOID
RtcExit()
{
	close(RtcFd);
}


VOID
GetStrRtc(CHAR* pOut)
{
	time_t TimeTicks = 0;
	struct rtc_time *prtc_tm = 0;
	
	TimeTicks = GetRtcTime();
		
	prtc_tm = localtime(&TimeTicks);
		
	sprintf(pOut, "Date/time %d-%d-%d,%02d:%02d:%02d:",
		prtc_tm->tm_mday, prtc_tm->tm_mon + 1, prtc_tm->tm_year + 1900,
		prtc_tm->tm_hour, prtc_tm->tm_min, prtc_tm->tm_sec);
}


VOID
TestRtc()
{
	time_t TimeTicks = 0;
	static DWORD preticks = 0;
	struct rtc_time *prtc_tm = 0;	

	if (GetTickCount() - preticks > 1000) 
	{
		preticks = GetTickCount();
		TimeTicks = GetRtcTime();

		prtc_tm = localtime(&TimeTicks);
		
		fprintf(stderr, "Read Current RTC date/time to %d-%d-%d, %02d:%02d:%02d.\n",
			prtc_tm->tm_mday, prtc_tm->tm_mon + 1, prtc_tm->tm_year + 1900,
			prtc_tm->tm_hour, prtc_tm->tm_min, prtc_tm->tm_sec);
	}
}