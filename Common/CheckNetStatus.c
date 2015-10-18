
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>
#include <unistd.h>

//#include <linux/ethtool.h>
#include <linux/sockios.h>

#include "MXTypes.h"
#include "MXCommon.h"

#include "ethtool-util.h"

#define DETECT_ETHSTATUS_INTERVAL	2500

extern BOOL g_bEthLinkStatus;

static 	INT skfd		=	-1;

static int 
detect_ethtool(int skfd, char *ifname)
{
	struct ifreq ifr;
	struct ethtool_value edata;
	
	memset(&ifr, 0, sizeof(ifr));
	edata.cmd = ETHTOOL_GLINK;
	
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);
	ifr.ifr_data = (char *) &edata;
	
	if (ioctl(skfd, SIOCETHTOOL, &ifr) == -1)
	{
		printf("ETHTOOL_GLINK failed: %s\n", strerror(errno));
		return 2;
	}
	
	return (edata.data ? 0 : 1);
}

VOID
CheckNetStatusInit()
{
	/* Open a socket. */
	if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
	{
		printf("socket error\n");
		exit(-1);
	}
}

VOID
CheckNetStatusProc()
{
	CHAR* ifname	=	"eth0";
	BYTE RetVal		=	0;
	static DWORD dwPreTicks = 0;

	if (GetTickCount() - dwPreTicks > DETECT_ETHSTATUS_INTERVAL) 
	{
		dwPreTicks = GetTickCount();
		RetVal = detect_ethtool(skfd, ifname);	
		
		switch(RetVal) 
		{
		case 0:
			{
				g_bEthLinkStatus	=	TRUE;
			}
			break;
		case 1:
			{
				g_bEthLinkStatus	=	FALSE;
			}
			break;
		default:
			{
				g_bEthLinkStatus	=	FALSE;
			}
			break;
		}
	}
}

VOID
CheckNetStatusExit()
{
	close(skfd);	
}