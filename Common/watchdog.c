//Copy from Documentation\watchdog.txt  Example Watchdog Driver
//xtp-d501
//-----------------------

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

#include "MXTypes.h"
#include "MXCommon.h"

//#define DISABLE_WATCHDOG

#define      WATCHDOG_TIMEOVER  30

#ifndef DISABLE_WATCHDOG
	static INT fd_watchdog = 0;
#endif

VOID 
WatchDogInit()
{
#ifndef DISABLE_WATCHDOG
	ULONG setval = WATCHDOG_TIMEOVER;
	INT ret;
	
	fd_watchdog = open("/dev/misc/watchdog",O_WRONLY);
	if(fd_watchdog < 0)
	{
		printf("dev/misc/watchdog can't open! \n");
		return;
	}
	else
	{
		printf("/dev/misc/watchdog/: fd_watchdog = %d \n", fd_watchdog);
	}
	
	ret = ioctl(fd_watchdog,WDIOC_SETTIMEOUT,&setval);
	if(ret != 0)
	{
		printf("set timeout fail,default=60 sec\n");
	}
#endif
}

VOID
WatchDog()
{
#ifndef DISABLE_WATCHDOG
	ioctl(fd_watchdog,WDIOC_KEEPALIVE,0);
#endif
}




