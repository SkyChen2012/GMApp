/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MenuParaProc.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		21 - Aug - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
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

#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "Dispatch.h"
#include "IniFile.h"
#include "MXCommon.h"
#include "AccessCommon.h"
#include "ParaSetting.h"
#include "MenuParaProc.h"
#include "AMT.h"
#include "PioApi.h"
#include "Multimedia.h"

/************** DEFINES **************************************************************/
//#define MENU_PARA_PROC_DEBUG

#define FLASH_SYS_PARA_BASE					0x903F0000
#define SYS_PARA_SET_FLAG_OFFSET			0x0000

#define SYS_PARA_FACTORY_OFFSET				0x0001
#define SYS_PARA_SYSCONFIG_OFFSET			0x0100
#define SYS_PARA_FUNCONFIG_OFFSET			0x0400
#define SYS_PARA_AS_OFFSET								0x0500

/************** TYPEDEFS *************************************************************/

#define TVP5150_IOC_CHANNEL_A		13
#define TVP5150_IOC_CHANNEL_B		14
#define TVP5150_IOC_CAM_OPEN		20
#define TVP5150_IOC_CAM_CLOSE		21
/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static void SetDeivceFun();
static void ParaVldDetect();
static void SetGMTypeByRule(int nValue2,int nValue3,int nValue4);
static BOOL CheckPNByValue(int nValue1,int nValue2,int nValue3,int nValue4);
static int GetValueLen(int nValue);
static BYTE GetValueByte(int nValue,int nByteIndex);
static void SetGMMoudleByRule(void);


static VOID		DWORD2IP_Str(DWORD dwIPAddr, CHAR *pIPStr);

VOID	SetCamera(bool Flag );

static BOOL PNCompare (CHAR *DestSting, CHAR *SoueSting);
static void ChangeGMPN(void);
#ifdef NEW_PN_SERIAL
static void setFunBasedOnPN(PN_STRUCT_t* pPN_Struct);
static BOOL judgmentAboutNewPNSerial(void);
#endif
#ifdef __SUPPORT_PROTOCOL_900_1A__
static void ReadNewSCPara();
static void WriteNewSCPara();
static void WriteNewASPara();
static void ShowNewSysCofig();
static char byPNLanguageWW = SC_VALUE_LANGSET;
SYSINFO_NEW_t g_NewSysConfig;
#endif 
FACTORYSET	 g_FactorySet;
SYSCONFIG		g_SysConfig;
ASPARA				g_ASPara;
PUPARA				g_PUPara;
#ifdef FORCE_ULK_FALG
DoorUlkBehaviour_t  g_DoorUlkBehav;
#endif //FORCE_ULK_FALG
DEVICEFUN g_DevFun = 
{
	DEVICE_CORAL_GM,
	TRUE
};

FUNCONFIG g_FunConfig = 
{
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	TRUE
};

SYSINFO g_SysInfo = 
{
	"V1.00.04.000",
	"v1.03",
	""
};

#ifdef NEW_PN_SERIAL
#define PN_INFO_LEN 37
const char* arrProductInfo[] = {
                        /*product type*/
                        "ANALOG_GM",
                        "DIGITAL_GM",
                        /*product serial*/
                        "REVERSE",                        
                        "AMBER_II",
                        "SHELL",
                        "MINI_SHELL",
                        "CORAL",
                        "CORAL_ROCK",
                        "CORAL_II",
                        "AMBER_GUARD",
                        /*keys layout*/
                        "DIGITAL",
                        "SINGLE_HOME",
                        "DIRECT_2HOME",
                        "DIRECT_4HOME",
                        "DIRECT_8HOME",
                        "DIRECT_10HOME",
                        "DIRECT_12HOME",
                        "ONE_HOME_PWD",
                        /*support access system or not*/
                        "NO_AS",
                        "ID_READER",
                        "IC_READER",
                        "WIGEND",
                        "EMPTY_SLOT",
                        /*support camera or not*/
                        "NO_CAMERA",
                        "D1_CAMERA",
                        /*color*/
                        "Aluminum silver sand",
                        "Aluminum silver wire",
                        "Aluminum ochre sand",
                        "Aluminum golden sand",
                        "Aluminum Coff-golden sand",
                        "Stainless steel wire",
                        /*languae*/
                        "CH+EN",
                        "EN+CH",
                        "HE+EN",
                        "BO+EN",
                        "TI+EN",
                        "UNKNOWN"
                        };
#endif

#ifdef __SUPPORT_PROTOCOL_900_1A__
static void ReadNewSCPara()
{
	const char *pStr = NULL;	
	printf("[%s]\n",__FUNCTION__);

	g_NewSysConfig.Version = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, SC_KEY_VERSION, SC_VALUE_KEY_VERSION);;

	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_IPADDR, SC_VALUE_IPADDR);
	g_NewSysConfig.IPAddr = IP_Str2DWORD(pStr);

	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_NETMASK, SC_VALUE_NETMASK);
	g_NewSysConfig.Mask = IP_Str2DWORD(pStr);

	pStr = ReadString(SC_SEC_GLOBAL_900_1A, SC_KEY_SERVER_IP, SC_VALUE_KEY_SERVER_IP);
	g_NewSysConfig.ServerIP = IP_Str2DWORD(pStr);
	
	g_NewSysConfig.Language = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_LANGSET, SC_VALUE_LANGSET);
	g_NewSysConfig.CameraMode = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_ACTIVECAMERA, SC_VALUE_ACTIVECAMERA);
	g_NewSysConfig.EnTamperAlarm = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_TAMPERALARM, SC_VALUE_TAMPERALARM);
	g_NewSysConfig.TalkTime = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_TALKTIME, SC_VALUE_TALKTIME);
	g_NewSysConfig.EnWiegand = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, SC_KEY_ENWIEGAND, SC_VALUE_KEY_ENWIEGAND);
	g_NewSysConfig.WiegandBit = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, AS_KEY_WIEGANDBITNUM, AS_VALUE_WIEGANDBITNUM);
	g_NewSysConfig.WiegandRev = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, AS_KEY_WIEGANDREVERSE, AS_VALUE_WIEGANDREVERSE);
	g_NewSysConfig.DI1Mode = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI1MODE, SC_VALUE_KEY_DI1MODE);
	g_NewSysConfig.DI1Conf = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI1CONF, SC_VALUE_KEY_DI1CONF);
	g_NewSysConfig.DI1Fun = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI1FUN, SC_VALUE_KEY_DI1FUN);
	g_NewSysConfig.DI2Mode = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI2MODE, SC_VALUE_KEY_DI2MODE);
	g_NewSysConfig.DI2Conf = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI2CONF, SC_VALUE_KEY_DI2CONF);
	g_NewSysConfig.DI2Fun = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI2FUN, SC_VALUE_KEY_DI2FUN);
	g_NewSysConfig.DO1Mode = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DO1MODE, SC_VALUE_KEY_DO1MODE);
	g_NewSysConfig.DO2Mode = (UCHAR)ReadInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DO2MODE, SC_VALUE_KEY_DO2MODE);
	g_NewSysConfig.TalkVol = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_TALK_VOLUME, SC_DEFAULT_TALK_VOLUME);
	g_NewSysConfig.RingTime = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_RINGTIME, SC_VALUE_RINGTIME);

	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_SYSPWD, SC_VALUE_SYSPWD);
	if (pStr)
	{
		strcpy(g_NewSysConfig.SysPwd, pStr);
	}
	g_NewSysConfig.VldCodeDigits = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_VLDCODEDIGITS, SC_VALUE_VLDCODEDIGITS);

}

static void WriteNewSCPara()
{
	CHAR pStr[20] = { 0 };
	printf("[%s]\n",__FUNCTION__);
	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_VERSION, (INT)g_NewSysConfig.Version);

	if(g_NewSysConfig.Version == 0)
		return;
	
	g_SysConfig.IPAddr = g_NewSysConfig.IPAddr;

	g_SysConfig.NetMask = g_NewSysConfig.Mask;

	g_SysConfig.LangSel = g_NewSysConfig.Language;

	g_SysConfig.ActiveCamera = g_NewSysConfig.CameraMode;

	g_SysConfig.EnTamperAlarm = g_NewSysConfig.EnTamperAlarm;

	g_SysConfig.TalkTime = g_NewSysConfig.TalkTime;

	g_SysConfig.TalkVolume = g_NewSysConfig.TalkVol;

	g_SysConfig.RingTime = g_NewSysConfig.RingTime;

	g_SysConfig.VldCodeDigits = g_NewSysConfig.VldCodeDigits;

	strcpy(g_SysConfig.SysPwd, g_NewSysConfig.SysPwd);
	
	DWORD2IP_Str(g_NewSysConfig.IPAddr, pStr);
	WriteString(SC_SEC_GLOBAL, SC_KEY_IPADDR, pStr);

	DWORD2IP_Str(g_NewSysConfig.Mask, pStr);
	WriteString(SC_SEC_GLOBAL, SC_KEY_NETMASK, pStr);

	WriteInt(SC_SEC_GLOBAL, SC_KEY_LANGSET, (INT)g_NewSysConfig.Language);
	
	WriteInt(SC_SEC_GLOBAL, SC_KEY_ACTIVECAMERA, (INT)g_NewSysConfig.CameraMode);

	WriteInt(SC_SEC_GLOBAL, SC_KEY_TAMPERALARM, (INT)g_NewSysConfig.EnTamperAlarm);
	
	WriteInt(SC_SEC_GLOBAL, SC_KEY_TALKTIME, (INT)g_NewSysConfig.TalkTime);
	
	WriteString(SC_SEC_GLOBAL, SC_KEY_SYSPWD, g_NewSysConfig.SysPwd);

	WriteInt(SC_SEC_GLOBAL, SC_TALK_VOLUME, (INT)g_NewSysConfig.TalkVol);

	WriteInt(SC_SEC_GLOBAL, SC_KEY_RINGTIME, (INT)g_NewSysConfig.RingTime);

	WriteInt(SC_SEC_GLOBAL, SC_KEY_VLDCODEDIGITS, (INT)g_SysConfig.VldCodeDigits);

	DWORD2IP_Str(g_NewSysConfig.ServerIP, pStr);
	WriteString(SC_SEC_GLOBAL_900_1A, SC_KEY_SERVER_IP, pStr);

	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_ENWIEGAND, (INT)g_NewSysConfig.EnWiegand);

	WriteInt(SC_SEC_GLOBAL_900_1A, AS_KEY_WIEGANDBITNUM, (INT)g_NewSysConfig.WiegandBit);

	WriteInt(SC_SEC_GLOBAL_900_1A, AS_KEY_WIEGANDREVERSE, (INT)g_NewSysConfig.WiegandRev);

	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI1MODE, (INT)g_NewSysConfig.DI1Mode);

	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI1CONF, (INT)g_NewSysConfig.DI1Conf);

	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI1FUN, (INT)g_NewSysConfig.DI1Fun);

	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI2MODE, (INT)g_NewSysConfig.DI2Mode);

	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI2CONF, (INT)g_NewSysConfig.DI2Conf);

	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DI2FUN, (INT)g_NewSysConfig.DI2Fun);

	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DO1MODE, (INT)g_NewSysConfig.DO1Mode);

	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_DO2MODE, (INT)g_NewSysConfig.DO2Mode);

	//======close liftcontrol function=====start========
	g_SysConfig.LCEnable = 0;
	g_SysConfig.LCMode = 0;
	g_SysConfig.LC_ComMode = 0;
	g_SysConfig.bLCAgent = 0;
	g_SysConfig.nLCFuncs = 0;
	WriteInt(SC_SEC_GLOBAL, SC_KEY_LCENABLE, (INT)g_SysConfig.LCEnable);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_LCMODE, (INT)g_SysConfig.LCMode);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_LC_COM_MODE, (INT)g_SysConfig.LC_ComMode);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_LCAENABLE, (INT)g_SysConfig.bLCAgent);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_LCFUNCTION, (INT)g_SysConfig.nLCFuncs);
	ShowNewSysCofig();
	//======close liftcontrol function=====end========
}

static void WriteNewASPara()
{
	CHAR pStr[20] = { 0 };
		printf("[%s]\n",__FUNCTION__);

	if(g_NewSysConfig.Version != 0)
	{
		DWORD2IP_Str(g_NewSysConfig.ServerIP, pStr);
		WriteString(SC_SEC_GLOBAL, AS_KEY_ACCSERVERIP, pStr);
		g_ASPara.dwAccServerIP = g_NewSysConfig.ServerIP;
	}
}

#endif 
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadHwSwVer
**	AUTHOR:		   Jeff Wang
**	DATE:		27 - Sep - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Anything need to be noticed.
*/
static void 
LoadHwSwVer()
{
	strcpy(g_SysInfo.SoftwareVersion, g_FactorySet.Softver);
	strcpy(g_SysInfo.HardareVersion, g_FactorySet.Hardver);
	strcpy(g_SysInfo.PN, g_FactorySet.ModuleType);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadFSPara
**	AUTHOR:		   Jeff Wang
**	DATE:		24 - Dec - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Read the factory set parameters from file
*/
static VOID
ReadFSPara()
{
	const char *pStr = NULL;
	
	OpenIniFile(HW);
	
	pStr = ReadString(HW_SEC_GLOBAL, HW_KEY_MODULETYPE, HW_VALUE_MODULETYPE);
	if (pStr)
	{
		strcpy(g_FactorySet.ModuleType, pStr);
	}
	pStr	= ReadString(HW_SEC_GLOBAL, HW_KEY_HARVER, HW_VALUE_HARVER);
	if (pStr)
	{
		strcpy(g_FactorySet.Hardver, pStr);
	}
	CloseIniFile();
	
	OpenIniFile(SW);
	pStr	= ReadString(SW_SEC_GLOBAL, SW_KEY_SWVER, SW_VALUE_SWVER);
	if (pStr)
	{
		strcpy(g_FactorySet.Softver, pStr);
	}
	CloseIniFile();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadSCPara
**	AUTHOR:		   Jeff Wang
**	DATE:		24 - Dec - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Read the system configure parameters from file
*/
static VOID
ReadSCPara()
{
	const char *pStr = NULL;
	const char *pIP = NULL;
	int         i = 0;
	
	OpenIniFile(SC);

	g_SysConfig.LangSel = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_LANGSET, byPNLanguageWW);
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_IPADDR, SC_VALUE_IPADDR);
	g_SysConfig.IPAddr = IP_Str2DWORD(pStr);

	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_MCSERVER_IPADDR, SC_VALUE_MCSERVER_IPADDR);
	g_SysConfig.MCIPSERVER = IP_Str2DWORD(pStr);
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_MCCLIENT_IPADDR, SC_VALUE_MCCLIENT_IPADDR);
	g_SysConfig.MCIPCLIENT = IP_Str2DWORD(pStr);
		
	pIP = pStr;
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_LCAIP, pIP);
	g_SysConfig.LCAIP = IP_Str2DWORD(pStr);

	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_NETMASK, SC_VALUE_NETMASK);
	g_SysConfig.NetMask = IP_Str2DWORD(pStr);

	g_SysConfig.RingNum = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_RINGNUM, SC_VALUE_RINGNUM);
	
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_SYSPWD, SC_VALUE_SYSPWD);
	if (pStr)
	{
		strcpy(g_SysConfig.SysPwd, pStr);
	}
	g_SysConfig.EnTamperAlarm = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_TAMPERALARM, SC_VALUE_TAMPERALARM);
	g_SysConfig.VldCodeDigits = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_VLDCODEDIGITS, SC_VALUE_VLDCODEDIGITS);
	g_SysConfig.bHelpInfoDld  = ReadBool(SC_SEC_GLOBAL, SC_KEY_HELPINFODLD, SC_VALUE_HELPINFODLD);
	g_SysConfig.ActiveCamera  = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_ACTIVECAMERA, SC_VALUE_ACTIVECAMERA);
	g_SysConfig.VideoFormat	  = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_VIDEOFORMAT, SC_VALUE_VIDEOFORMAT);
	g_SysConfig.DI1  = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_DI1, SC_VALUE_DI1);
	g_SysConfig.DI2  = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_DI2, SC_VALUE_DI2);
	g_SysConfig.DO1  = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_DO1, SC_VALUE_DO1);
	g_SysConfig.DO2  = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_DO2, SC_VALUE_DO2);
	g_SysConfig.TalkTime  = ReadInt(SC_SEC_GLOBAL, SC_KEY_TALKTIME, SC_VALUE_TALKTIME);
	g_SysConfig.RingTime  = ReadInt(SC_SEC_GLOBAL, SC_KEY_RINGTIME, SC_VALUE_RINGTIME);
	
	g_SysConfig.bLCAgent  = ReadInt(SC_SEC_GLOBAL, SC_KEY_LCAENABLE, SC_VALUE_LCAENABLE);
	g_SysConfig.LCEnable  = ReadInt(SC_SEC_GLOBAL, SC_KEY_LCENABLE, SC_VALUE_LCENABLE);
	g_SysConfig.LCMode  = ReadInt(SC_SEC_GLOBAL, SC_KEY_LCMODE, SC_VALUE_LCMODE);
	g_SysConfig.LC_ComMode  = ReadInt(SC_SEC_GLOBAL, SC_KEY_LC_COM_MODE, SC_VALUE_LC_COM_MODE);
	g_SysConfig.nLCFuncs  = ReadInt(SC_SEC_GLOBAL, SC_KEY_LCFUNCTION, SC_VALUE_LCFUNCTION);


	g_SysConfig.DO1_Pulse_Time = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_DO1_PULSETIME, SC_VALUE_DO1_PULSETIME);
	g_SysConfig.DO2_Pulse_Time = (UCHAR)ReadInt(SC_SEC_GLOBAL, SC_KEY_DO2_PULSETIME, SC_VALUE_DO2_PULSETIME);

	g_SysConfig.TalkVolume= (UINT)ReadInt(SC_SEC_GLOBAL, SC_TALK_VOLUME, SC_DEFAULT_TALK_VOLUME);
	g_SysConfig.UnlockTime= (UINT)ReadInt(SC_SEC_GLOBAL, SC_UNLOCK_TIME, SC_DEFAULT_UNLOCK_TIME);

	for(i = 0; i < MAX_ROOM; i++)
	{
		strcpy(g_SysConfig.CallCode[0], "");
	}




	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM1, SC_VALUE_KEY_ROOM1);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[0], pStr);
	}	
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM2, SC_VALUE_KEY_ROOM2);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[1], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM3, SC_VALUE_KEY_ROOM3);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[2], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM4, SC_VALUE_KEY_ROOM4);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[3], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM5, SC_VALUE_KEY_ROOM5);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[4], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM6, SC_VALUE_KEY_ROOM6);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[5], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM7, SC_VALUE_KEY_ROOM7);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[6], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM8, SC_VALUE_KEY_ROOM8);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[7], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM9, SC_VALUE_KEY_ROOM9);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[8], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM10, SC_VALUE_KEY_ROOM10);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[9], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM11, SC_VALUE_KEY_ROOM11);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[10], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM12, SC_VALUE_KEY_ROOM12);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[11], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM13, SC_VALUE_KEY_ROOM13);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[12], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM14, SC_VALUE_KEY_ROOM14);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[13], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM15, SC_VALUE_KEY_ROOM15);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[14], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM16, SC_VALUE_KEY_ROOM16);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[15], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM17, SC_VALUE_KEY_ROOM17);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[16], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM18, SC_VALUE_KEY_ROOM18);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[17], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM19, SC_VALUE_KEY_ROOM19);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[18], pStr);
	}
	pStr = ReadString(SC_SEC_GLOBAL, SC_KEY_ROOM20, SC_VALUE_KEY_ROOM20);
	if (pStr)
	{
		strcpy(g_SysConfig.CallCode[19], pStr);
	}
#ifdef __SUPPORT_PROTOCOL_900_1A__
	ReadNewSCPara();
#endif 
	CloseIniFile();
}

int
GetLCMode(void)
{
	return g_SysConfig.LC_ComMode;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	WriteSCPara
**	AUTHOR:		   Jeff Wang
**	DATE:		24 - Dec - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Write the system configure parameters to file
*/
static VOID
WriteSCPara()
{
	CHAR pStr[20] = { 0 };
	CHAR Str[128];

	int  index = 0;

	OpenIniFile(SC);
	
	WriteInt(SC_SEC_GLOBAL, SC_KEY_LANGSET, (INT)g_SysConfig.LangSel);
	DWORD2IP_Str(g_SysConfig.IPAddr, pStr);
	WriteString(SC_SEC_GLOBAL, SC_KEY_IPADDR, pStr);

	DWORD2IP_Str(g_SysConfig.LCAIP, pStr);
	WriteString(SC_SEC_GLOBAL, SC_KEY_LCAIP, pStr);


	DWORD2IP_Str(g_SysConfig.NetMask, pStr);
	WriteString(SC_SEC_GLOBAL, SC_KEY_NETMASK, pStr);

	DWORD2IP_Str(g_SysConfig.MCIPSERVER, pStr);
	WriteString(SC_SEC_GLOBAL, SC_KEY_MCSERVER_IPADDR, pStr);
	
	DWORD2IP_Str(g_SysConfig.MCIPCLIENT, pStr);
	WriteString(SC_SEC_GLOBAL, SC_KEY_MCCLIENT_IPADDR, pStr);
	


	WriteInt(SC_SEC_GLOBAL, SC_KEY_RINGNUM, (INT)g_SysConfig.RingNum);
	WriteString(SC_SEC_GLOBAL, SC_KEY_SYSPWD, g_SysConfig.SysPwd);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_TAMPERALARM, (INT)g_SysConfig.EnTamperAlarm);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_VLDCODEDIGITS, (INT)g_SysConfig.VldCodeDigits);
	WriteBool(SC_SEC_GLOBAL, SC_KEY_HELPINFODLD, (INT)g_SysConfig.bHelpInfoDld);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_ACTIVECAMERA, (INT)g_SysConfig.ActiveCamera);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_VIDEOFORMAT, (INT)g_SysConfig.VideoFormat);	

	WriteInt(SC_SEC_GLOBAL, SC_KEY_DI1, (INT)g_SysConfig.DI1);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_DI2, (INT)g_SysConfig.DI2);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_DO1, (INT)g_SysConfig.DO1);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_DO2, (INT)g_SysConfig.DO2);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_TALKTIME, (INT)g_SysConfig.TalkTime);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_RINGTIME, (INT)g_SysConfig.RingTime);

	WriteInt(SC_SEC_GLOBAL, SC_KEY_LCENABLE, (INT)g_SysConfig.LCEnable);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_LCMODE, (INT)g_SysConfig.LCMode);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_LC_COM_MODE, (INT)g_SysConfig.LC_ComMode);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_LCAENABLE, (INT)g_SysConfig.bLCAgent);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_LCFUNCTION, (INT)g_SysConfig.nLCFuncs);



	WriteInt(SC_SEC_GLOBAL, SC_KEY_DO1_PULSETIME, (INT)g_SysConfig.DO1_Pulse_Time);
	WriteInt(SC_SEC_GLOBAL, SC_KEY_DO2_PULSETIME, (INT)g_SysConfig.DO2_Pulse_Time);

	WriteInt(SC_SEC_GLOBAL, SC_TALK_VOLUME, (INT)g_SysConfig.TalkVolume);
	WriteInt(SC_SEC_GLOBAL, SC_UNLOCK_TIME, (INT)g_SysConfig.UnlockTime);

	for(index=0; index < 20; index++)
	{
		sprintf(Str,"ROOM CODE %d",index+1);
		WriteString(SC_SEC_GLOBAL, Str, g_SysConfig.CallCode[index]);				
	}
#ifdef __SUPPORT_PROTOCOL_900_1A__
	
	g_NewSysConfig.IPAddr = g_SysConfig.IPAddr;

	g_NewSysConfig.Mask = g_SysConfig.NetMask;

	g_NewSysConfig.Language = g_SysConfig.LangSel;

	g_NewSysConfig.CameraMode = g_SysConfig.ActiveCamera;

	g_NewSysConfig.EnTamperAlarm = g_SysConfig.EnTamperAlarm;

	g_NewSysConfig.TalkTime = g_SysConfig.TalkTime;

	g_NewSysConfig.TalkVol = g_SysConfig.TalkVolume;

	g_NewSysConfig.RingTime = g_SysConfig.RingTime;

	g_NewSysConfig.VldCodeDigits = g_SysConfig.VldCodeDigits;

	strcpy(g_SysConfig.SysPwd, g_NewSysConfig.SysPwd);
	if(g_NewSysConfig.Version != 0)
	{	
		WriteNewSCPara();
	}
#endif 
	WriteIniFile(SC);
	CloseIniFile();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadASPara
**	AUTHOR:		   Jeff Wang
**	DATE:		24 - Dec - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Read the access system parameters from file
*/
static VOID
ReadASPara()
{
	const char *pStr = NULL;
	
	OpenIniFile(AS);
	
	pStr = ReadString(AS_SEC_GLOBAL, AS_KEY_CDPWD, AS_VALUE_CDPWD);
	if (pStr)
	{
		strcpy(g_ASPara.CdPwdDefault, pStr);
	}

	g_ASPara.VagCodeSel = (UCHAR)ReadInt(AS_SEC_GLOBAL, AS_KEY_VAGCODESET, AS_VALUE_VAGCODESET);
	g_ASPara.GateOpenOverTime = (UCHAR)ReadInt(AS_SEC_GLOBAL, AS_KEY_GATEOVERTIMETIME, AS_VALUE_GATEOVERTIMETIME);
	g_ASPara.GateOpenAlmFlag  = ReadBool(AS_SEC_GLOBAL, AS_KEY_GATEOVERTIMEALARM, AS_VALUE_GATEOVERTIMEALARM);
	g_ASPara.GateOpenContactor = ReadBool(AS_SEC_GLOBAL, AS_KEY_GATEOPENCONTACTOR, AS_VALUE_GATEOPENCONTACTOR);

	g_ASPara.InvldCardSwipeRptSet = (UCHAR)ReadInt(AS_SEC_GLOBAL, AS_KEY_INVLDRPTTIME, AS_VALUE_INVLDRPTTIME);
	g_ASPara.InvldCardSwipeAlmFlag  = ReadBool(AS_SEC_GLOBAL, AS_KEY_INVLDRPTALARM, AS_VALUE_INVLDRPTALARM);
	
	g_ASPara.GateOpenManFlag  = ReadBool(AS_SEC_GLOBAL, AS_KEY_OPENMANUAL, AS_VALUE_OPENMANUAL);
	
	g_ASPara.ASOpenMode = (UCHAR)ReadInt(AS_SEC_GLOBAL, AS_KEY_GATEOPENMODE, AS_VALUE_GATEOPENMODE);
	g_ASPara.RelayPulseWith = (DWORD)ReadInt(AS_SEC_GLOBAL, AS_KEY_RELAYPULSEWIDTH, AS_VALUE_RELAYPULSEWIDTH);
	
	g_ASPara.InfraredContactor = ReadBool(AS_SEC_GLOBAL, AS_KEY_INFRAREDCONTACTOR, AS_VALUE_INFRAREDCONTACTOR);
	g_ASPara.InfraredOverTime = (UCHAR)ReadInt(AS_SEC_GLOBAL, AS_KEY_INFRAREDTIME, AS_VALUE_INFRAREDTIME);
	g_ASPara.InfraredAlmFlag = ReadBool(AS_SEC_GLOBAL, AS_KEY_INFRAREDALARM, AS_VALUE_INFRAREDALARM);
	
	if (DEVICE_CORAL_DB == g_DevFun.DevType) g_ASPara.MCUnlockEnable = ReadBool(AS_SEC_GLOBAL, AS_KEY_MCUNLOCKENABLE, AS_VALUE_MCUNLOCKENABLE_OFF);
	else g_ASPara.MCUnlockEnable = ReadBool(AS_SEC_GLOBAL, AS_KEY_MCUNLOCKENABLE, AS_VALUE_MCUNLOCKENABLE_ON);

	g_ASPara.bEnableAS = ReadBool(AS_SEC_GLOBAL, AS_KEY_ENABLEAS, !AS_VALUE_ENABLEAS);
    printf("enable access system ? : ===========================================================%d\n",g_ASPara.bEnableAS);        
	pStr = ReadString(SC_SEC_GLOBAL, AS_KEY_ACCSERVERIP, SC_VALUE_ACCSERVERIP);
	g_ASPara.dwAccServerIP = IP_Str2DWORD(pStr);
	
	pStr=ReadString(SC_SEC_GLOBAL, AS_KEY_ACCDEVCODE, AS_VALUE_ACCDEVCODE);
	strcpy(g_ASPara.szACCDevCode,pStr);
	
	g_ASPara.nPatrolNum=ReadInt(AS_SEC_GLOBAL, AS_KEY_PATROLNUM, AS_VALUE_PATROLNUM);
#ifdef __SUPPORT_WIEGAND_CARD_READER__
	g_ASPara.WiegandBitNum = (UCHAR)ReadInt(AS_SEC_GLOBAL, AS_KEY_WIEGANDBITNUM, AS_VALUE_WIEGANDBITNUM);
	g_ASPara.WiegandReverse = (UCHAR)ReadInt(AS_SEC_GLOBAL, AS_KEY_WIEGANDREVERSE, AS_VALUE_WIEGANDREVERSE);
#endif
	//printf("************* g_ASPara.MCUnlockEnable: %d\n", g_ASPara.MCUnlockEnable);
#ifdef FORCE_ULK_FALG
    g_ASPara.EnableForceUlkReport = ReadInt(AS_SEC_GLOBAL, AS_KEY_FORCE_ULK_REPORT,AS_VALUE_FORCE_ULK_REPORT);
#endif //FORCE_ULK_FALG


	CloseIniFile();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	WriteASPara
**	AUTHOR:		   Jeff Wang
**	DATE:		24 - Dec - 2008
**
**	DESCRIPTION:	p
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Write the access system parameters to file
*/
static VOID
WriteASPara()
{
	CHAR pStr[20] = { 0 };

	OpenIniFile(AS);
	
	WriteString(AS_SEC_GLOBAL, AS_KEY_CDPWD, g_ASPara.CdPwdDefault);
	WriteInt(AS_SEC_GLOBAL, AS_KEY_VAGCODESET, (INT)g_ASPara.VagCodeSel);
	WriteInt(AS_SEC_GLOBAL, AS_KEY_GATEOVERTIMETIME, (INT)g_ASPara.GateOpenOverTime);
	WriteBool(AS_SEC_GLOBAL, AS_KEY_GATEOVERTIMEALARM, g_ASPara.GateOpenAlmFlag);
	WriteInt(AS_SEC_GLOBAL, AS_KEY_INVLDRPTTIME, (INT)g_ASPara.InvldCardSwipeRptSet);
	WriteBool(AS_SEC_GLOBAL, AS_KEY_INVLDRPTALARM, g_ASPara.InvldCardSwipeAlmFlag);
	WriteBool(AS_SEC_GLOBAL, AS_KEY_OPENMANUAL, g_ASPara.GateOpenManFlag);
	WriteInt(AS_SEC_GLOBAL, AS_KEY_GATEOPENMODE, (INT)g_ASPara.ASOpenMode);
	WriteInt(AS_SEC_GLOBAL, AS_KEY_RELAYPULSEWIDTH, (INT)g_ASPara.RelayPulseWith);
	
	WriteBool(AS_SEC_GLOBAL, AS_KEY_GATEOPENCONTACTOR, g_ASPara.GateOpenContactor);
	WriteBool(AS_SEC_GLOBAL, AS_KEY_INFRAREDCONTACTOR, g_ASPara.InfraredContactor);
	WriteInt(AS_SEC_GLOBAL, AS_KEY_INFRAREDTIME, (INT)g_ASPara.InfraredOverTime);
	WriteBool(AS_SEC_GLOBAL, AS_KEY_INFRAREDALARM, g_ASPara.InfraredAlmFlag);
	WriteBool(AS_SEC_GLOBAL, AS_KEY_MCUNLOCKENABLE, g_ASPara.MCUnlockEnable);

	WriteBool(AS_SEC_GLOBAL, AS_KEY_ENABLEAS, g_ASPara.bEnableAS);
	DWORD2IP_Str(g_ASPara.dwAccServerIP, pStr);
	WriteString(SC_SEC_GLOBAL, AS_KEY_ACCSERVERIP, pStr);
	WriteString(SC_SEC_GLOBAL, AS_KEY_ACCDEVCODE, g_ASPara.szACCDevCode);
#ifdef __SUPPORT_WIEGAND_CARD_READER__
	WriteInt(AS_SEC_GLOBAL, AS_KEY_WIEGANDBITNUM, g_ASPara.WiegandBitNum);
	WriteInt(AS_SEC_GLOBAL, AS_KEY_WIEGANDREVERSE, g_ASPara.WiegandReverse);
#endif

#ifdef FORCE_ULK_FALG    
    WriteInt(AS_SEC_GLOBAL, AS_KEY_FORCE_ULK_REPORT, g_ASPara.EnableForceUlkReport);
#endif //FORCE_ULK_FALG
#ifdef __SUPPORT_PROTOCOL_900_1A__
	WriteNewASPara();
#endif 

	WriteIniFile(AS);
	CloseIniFile();
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadPUPara
**	AUTHOR:		   Jeff Wang
**	DATE:		24 - Dec - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Read the access system parameters from file
*/
static VOID
ReadPUPara()
{
	const char *pStr = NULL;
	
	OpenIniFile(PU);
	
	pStr = ReadString(PU_SEC_GLOBAL, PU_KEY_RDPWD, PU_VALUE_RDPWD);
	if (pStr)
	{
		strcpy(g_PUPara.ResiPwdDefault, pStr);
	}
	g_PUPara.bFunEnabled  = ReadBool(PU_SEC_GLOBAL, PU_KEY_FUNENABLE, PU_VALUE_FUNENABLE);

	CloseIniFile();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	WritePUPara
**	AUTHOR:		   Jeff Wang
**	DATE:		24 - Dec - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Write the access system parameters to file
*/
static VOID
WritePUPara()
{
	OpenIniFile(PU);
	
	WriteString(PU_SEC_GLOBAL, PU_KEY_RDPWD, g_PUPara.ResiPwdDefault);
	WriteBool(PU_SEC_GLOBAL, PU_KEY_FUNENABLE, g_PUPara.bFunEnabled);

	WriteIniFile(PU);
	CloseIniFile();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadMenuParaFromMem
**	AUTHOR:		   Jeff Wang
**	DATE:		8 - July - 2008
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

void 
LoadMenuParaFromMem()
{
	int SysReset = 0;
    
	PIOInit();	
	
	SysReset = PIORead(RESET_SWITCH_DI);

	if(1 == SysReset)
	{		
		system("mount -n -o remount rw /mox");
        system("mv /mox/rdwr/DI1.pcm /mox");
        system("mv /mox/rdwr/DI2.pcm /mox");
		system("rm -f /mox/rdwr/*");
        system("mv /mox/DI1.pcm /mox/rdwr");
        system("mv /mox/DI2.pcm /mox/rdwr");
        system("ls /mox/rdwr/");

		printf("Reset Setting Successfully!\n");
	}
		
	ReadFSPara();
	LoadHwSwVer();
    
    if(!judgmentAboutNewPNSerial())
    {
        ChangeGMPN();
    	SetDeivceFun();
    	SetGMMoudleByRule();
    }	
	ReadSCPara();
	ReadASPara();
	ReadPUPara();
	if(!g_ASPara.bEnableAS) //for bug 11541
		g_ASPara.InvldCardSwipeAlmFlag=0;

	ParaVldDetect();

	SetAudioModule();
	printf("Load parameters finished; SysReset = %d\n",SysReset);
}

#ifdef NEW_PN_SERIAL
static void setFunBasedOnPN(PN_STRUCT_t* pPN_Struct)
{
    unsigned char uResult = 0;
    unsigned char uIdx = PN_INFO_LEN - 1;
    switch(pPN_Struct->pnType)
    {
        /*模拟门口机*/
        case 5 : uIdx = 0 ; g_DevFun.bNewPnUsed = FALSE; break;
        /*数字门口机*/
        case 6 : uIdx = 1 ; g_DevFun.bNewPnUsed = TRUE; break;
        default : {uResult = 1;}
    }
    printf("<%s> ",arrProductInfo[uIdx]);
    uIdx = PN_INFO_LEN - 1;
    switch(pPN_Struct->pnSerial)
    {
        /*保留*/
        case 1 : uIdx = 2 ;  break;
        /*琥珀II*/
        case 2 : uIdx = 3 ; g_DevFun.SeriesType = SERIES_AMBER2; break;
        /*贝壳*/
        case 3 : uIdx = 4 ; g_DevFun.SeriesType = SERIES_SHELL; break;
        /*迷你贝壳*/
        case 4 : uIdx = 5 ; g_DevFun.SeriesType = SERIES_MINI_SHELL; break;
        /*珊瑚*/
        case 5 : uIdx = 6 ; g_DevFun.DevType = DEVICE_CORAL_DB; break;
        /*珊岩*/
        case 6 : uIdx = 7 ; break;
        /*珊瑚II*/
        case 7 : uIdx = 8 ; g_DevFun.SeriesType = SERIES_SHELL; break;
        /*琥珀立柱*/
        case 8 : uIdx = 9 ; g_DevFun.SeriesType = SERIES_SHELL; break;
        default : {uResult = 1;}
    }
    printf("<%s> ",arrProductInfo[uIdx]);
    uIdx = PN_INFO_LEN - 1;
    switch(pPN_Struct->pnKey)
    {
        /*数字式*/
        case 0 : uIdx = 10 ; g_DevFun.DevType = DEVICE_CORAL_GM; break;
        /*独户*/
        case 1 : uIdx = 11 ; g_DevFun.DevType = DEVICE_CORAL_DB; break;
        /*直按式2户*/
        case 2 : uIdx = 12 ; g_DevFun.DevType = DEVICE_CORAL_DIRECT_GM; break;
        /*直按式4户*/
        case 4 : uIdx = 13 ; g_DevFun.DevType = DEVICE_CORAL_DIRECT_GM; break;
        /*直按式8户*/
        case 8 : uIdx = 14 ; g_DevFun.DevType = DEVICE_CORAL_DIRECT_GM; break;
        /*直按式10户*/
        case 10 : uIdx = 15 ; g_DevFun.DevType = DEVICE_CORAL_DIRECT_GM; break;
        /*直按式12户*/
        case 12 : uIdx = 16 ; g_DevFun.DevType = DEVICE_CORAL_DIRECT_GM; break;
        /*独户带密码键盘*/
        case 99 : uIdx = 17 ; g_DevFun.DevType = DEVICE_CORAL_DB; break;
        default : {uResult = 1;}
    }
    printf("<%s> ",arrProductInfo[uIdx]);
    uIdx = PN_INFO_LEN - 1;
    switch(pPN_Struct->pnAS)
    {
        /*不带门禁*/
        case 0 : uIdx = 18 ; g_DevFun.bCardControlUsed = FALSE; break;
        /*ID读卡头*/
        case 1 : uIdx = 19 ; g_DevFun.bCardControlUsed = TRUE; break;
        /*IC读卡头*/
        case 2 : uIdx = 20 ; g_DevFun.bCardControlUsed = TRUE; break;
        /*韦根接口*/
        case 3 : uIdx = 21 ; g_DevFun.bCardControlUsed = TRUE; break;
        /*门禁空槽*/
        case 4 : uIdx = 22 ; g_DevFun.bCardControlUsed = FALSE; break;
        default : {uResult = 1;}
    }
    printf("<%s> ",arrProductInfo[uIdx]);
    uIdx = PN_INFO_LEN - 1;
    switch(pPN_Struct->pnCamera)
    {
        /*不带摄像头*/
        case 0 : uIdx = 23 ; break;
        /*D1宽动态摄像头*/
        case 1 : uIdx = 24 ; break;
        default : {uResult = 1;}
    }
    printf("<%s> ",arrProductInfo[uIdx]);
    uIdx = PN_INFO_LEN - 1;
    switch(pPN_Struct->pnColor)
    {
        /*铝银色喷砂*/
        case 1 : uIdx = 25 ; break;
        /*铝银色拉丝*/
        case 2 : uIdx = 26 ; break;
        /*铝赭石色喷砂*/
        case 3 : uIdx = 27 ; break;
        /*铝金色喷砂*/
        case 4 : uIdx = 28 ; break;
        /*铝咖啡金喷砂*/
        case 5 : uIdx = 29 ; break;
        /*不锈钢拉丝*/
        case 6 : uIdx = 30 ; break;
        default : {uResult = 1;}
    }
    printf("<%s> ",arrProductInfo[uIdx]);
    uIdx = PN_INFO_LEN - 1;
    switch(pPN_Struct->pnLanguage)
    {
        /*汉语+英语*/
        case 0 : 
			uIdx = 31 ;
			byPNLanguageWW = SET_CHINESE;
			break;
        /*英语+汉语*/
        case 1 : 
			uIdx = 32 ; 
			byPNLanguageWW = SET_ENGLISH;
			break;
        /*希伯来语+英语*/
        case 2 : 
			uIdx = 33 ;
			byPNLanguageWW = SET_HEBREW;
			break;
        /*波兰语+英语*/
        case 3 : uIdx = 34 ; break;
        /*泰语+英语*/
        case 4 : uIdx = 35 ; break;
        default : {uResult = 1;}
    }   
    printf("<%s> \n",arrProductInfo[uIdx]);
    /*one functionality is not found,so post exception!*/
    if(uResult)
    {
        
    }
}

static BOOL judgmentAboutNewPNSerial(void)
{ 
    BOOL bRet = FALSE;
    PN_STRUCT_t sPN_Struct;
    /*at least 21 for extension: length of new PN serial chars*/
    if(21 <= strlen(g_FactorySet.ModuleType))
    {
        sscanf(g_FactorySet.ModuleType,"604-40%1d%1d-%2d%2d-%1d%1d%2d-%2d",
                                            &(sPN_Struct.pnType),
                                            &(sPN_Struct.pnReverse),
                                            &(sPN_Struct.pnSerial),
                                            &(sPN_Struct.pnKey),
                                            &(sPN_Struct.pnAS),
                                            &(sPN_Struct.pnCamera),
                                            &(sPN_Struct.pnColor),
                                            &(sPN_Struct.pnLanguage)
                                            );
        bRet = TRUE;
        setFunBasedOnPN(&sPN_Struct);
    }    
    return bRet;
}
#endif

static void ChangeGMPN(void)
{
    /*VERSION_CONTROL
    1.       604-2209-03 v2.40以前版本，保持现有状态
    2.       604-2209-03 v2.40及以上版本，按现有604-2221-0501 v2.00处理
    3.       604-2209-07 v2.40以前版本，保持现有状态
    4.       604-2209-07 v2.40及以上版本，按现有604-2220-0101 v2.00处理
    */
	int nVer1 = 0;
	int nVer2 = 0;
	int nVer3 = 0;
	DWORD  dwVer = 0;
	
	sscanf(g_FactorySet.Hardver, "V%x.%x.%x", &nVer1, &nVer2, &nVer3);
	if((nVer1==0) && (nVer2==0) && (nVer3==0))
	{
		sscanf(g_FactorySet.Hardver, "%x.%x.%x", &nVer1, &nVer2, &nVer3);
	}
	dwVer = MAKEDWORD(0, nVer1, nVer2, nVer3);
    /*Backup factory inforamtion*/
    strcpy(g_VerContrl.BackupPN,g_FactorySet.ModuleType);
    strcpy(g_VerContrl.BackupHardVer,g_FactorySet.Hardver);
    g_VerContrl.uIsChanged = 1;
	if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM03)) && (dwVer>=0x24000))
	{
		strcpy(g_FactorySet.ModuleType,C_NEGM096);
		strcpy(g_FactorySet.Hardver,"V2.00");
		printf("[Change PN] g_FactorySet.ModuleType=%s,g_FactorySet.Hardver=%s\n",g_FactorySet.ModuleType,g_FactorySet.Hardver);
    }
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM07)) && (dwVer>=0x24000))
	{
		strcpy(g_FactorySet.ModuleType,C_NEGM001);
		strcpy(g_FactorySet.Hardver,"V2.00");
		printf("[Change PN] g_FactorySet.ModuleType=%s,g_FactorySet.Hardver=%s\n",g_FactorySet.ModuleType,g_FactorySet.Hardver);
    }
    else
    {
        g_VerContrl.uIsChanged = 0;
        memset(g_VerContrl.BackupHardVer,0,VER_INFO_LEN);
        memset(g_VerContrl.BackupPN,0,32);
    }
}

static void setFunForStand_ColumnGM(void)
{
    strcpy(g_VerContrl.BackupPN,g_FactorySet.ModuleType);
    strcpy(g_VerContrl.BackupHardVer,g_FactorySet.Hardver);
    g_VerContrl.uIsChanged = 1;
    printf("[OUT]PN %s,Hardver : %s\n",g_FactorySet.ModuleType,g_FactorySet.Hardver);    
    if(0 == PNCompare(g_FactorySet.ModuleType,C_EGM61))
    {
        strcpy(g_FactorySet.ModuleType,"604-4031-300-1204");
		strcpy(g_FactorySet.Hardver,"V1.03");
    }
    else if(0 == PNCompare(g_FactorySet.ModuleType,C_EGM62))
    {
        strcpy(g_FactorySet.ModuleType,"604-4032-301-0104");
		strcpy(g_FactorySet.Hardver,"V1.03");
    }
    else
    {
        g_VerContrl.uIsChanged = 0;
        memset(g_VerContrl.BackupHardVer,0,VER_INFO_LEN);
        memset(g_VerContrl.BackupPN,0,32);
    }
    printf("[IN]PN %s,Hardver : %s\n",g_FactorySet.ModuleType,g_FactorySet.Hardver);    
}

static void SetGMMoudleByRule(void)
{
	int nValue1=0,nValue2=0,nValue3=0,nValue4=0;
	char * szPN;
    setFunForStand_ColumnGM();
	szPN=g_FactorySet.ModuleType;
	/*for example : 604-4032-301-0104*/
	sscanf(szPN, "%x-%x-%x-%x", &nValue1, &nValue2, &nValue3,&nValue4);
	printf("######nValue1=%x,nValue2=%x,nValue3=%x,nValue4=%x\n",nValue1,nValue2,nValue3,nValue4);
	if(CheckPNByValue(nValue1,nValue2,nValue3,nValue4))
	{
		SetGMTypeByRule(nValue2,nValue3,nValue4);

	}	
}
static void SetGMTypeByRule(int nValue2,int nValue3,int nValue4)
{
	BYTE byData;
	BYTE byHomeNum;
	byData=GetValueByte(nValue2,4);
	g_DevFun.SeriesType=SERIES_CORAL;
	if(5==byData || 4==byData)// En ,Cn
	{
		if(0==GetValueByte(nValue2,3) && 3==GetValueByte(nValue2,2))//900.1
		{
			byData=GetValueByte(nValue2,1);
			switch(byData)
			{
				case 1://鏁板瓧
					printf("GM Type is Digital\n");
					g_DevFun.DevType=DEVICE_CORAL_GM;
					break;
				case 2://鐩存寜
					byHomeNum=GetValueByte(nValue3,1)+GetValueByte(nValue3,2)*10;
					if(1==byHomeNum)
					{
						printf("GM Type is Single Home\n");
						g_DevFun.DevType=DEVICE_CORAL_DB;
					}
					else if(byHomeNum>1)
					{
						printf("GM Type is Direct\n");
						g_DevFun.DevType=DEVICE_CORAL_DIRECT_GM;
					}
					else
					{
						printf("SetSerialTypeByRule Error byHomeNum=%d\n",byHomeNum);
					}	
					break;
				default:
					printf("SetSerialTypeByRule Error byData=%d\n",byData);
					break;
			}
			switch(GetValueByte(nValue3,3))
			{
				case 1:
					g_DevFun.SeriesType=SERIES_AMBER;
					g_DevFun.bNewPnUsed=TRUE;
					printf("GM Series is Amber \n");
					break;
				case 2:
					g_DevFun.SeriesType=SERIES_AMBER2;
					g_DevFun.bNewPnUsed=TRUE;
					printf("GM Series is Amber2 \n");
					break;
				case 3:
					g_DevFun.SeriesType=SERIES_SHELL;
					g_DevFun.bNewPnUsed=TRUE;
					printf("GM Series is Shell \n");
					break;
				case 4:
					g_DevFun.SeriesType=SERIES_MINI_SHELL;
					g_DevFun.bNewPnUsed=TRUE;
					printf("GM Series is Mini Shell \n");
					break;
				default:
					printf("SetSerialTypeByRule Error\n");
					break;
			}
			
			switch(GetValueByte(nValue4,4))
			{
				case 1:
				case 2:
				case 3:
					g_DevFun.bCardControlUsed=TRUE;
					printf("Use CardControl\n");			
					break;
				default:
					g_DevFun.bCardControlUsed=FALSE;
#ifdef MENU_PARA_PROC_DEBUG					
					printf("No use CardControl\n");	
#endif
					break;
			}
		}
	}
	

}

static BOOL CheckPNByValue(int nValue1,int nValue2,int nValue3,int nValue4)
{
	if((3==GetValueLen(nValue1)) && 
	(4==GetValueLen(nValue2)) && 
	(3==GetValueLen(nValue3)) && 
	((4==GetValueLen(nValue4))||(3==GetValueLen(nValue4)||(1==GetValueLen(nValue4)))) )
	{
		if((0x604==nValue1))
		{
			return TRUE;
		}
	}
	return FALSE;
}
static int GetValueLen(int nValue)
{
	int nTempData,nLen=0;
	nTempData=nValue;
	while(nTempData>0)
	{
		nLen+=1;
		nTempData=nTempData>>4;
	}
	return nLen;
}
static BYTE GetValueByte(int nValue,int nByteIndex)
{
	if(nByteIndex>0)
	{
		return (nValue >>(4*(nByteIndex-1))) & 0xf;
	}
	return 0;
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SaveMenuPara2Mem
**	AUTHOR:		   Jeff Wang
**	DATE:		8 - July - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Anything need to be noticed.
*/

void 
SaveMenuPara2Mem()
{
	WriteSCPara();
	WriteASPara();
	WritePUPara();
	LoadAMT();
	printf("Save parameters finished\n");
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DWORD2IP_Str
**	AUTHOR:		   Jeff Wang
**	DATE:		8 - July - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Anything need to be noticed.
*/
VOID
SetNetWork()
{
	char pTempIP[20] = { 0 };
	char pTempMAC[20] = { 0 };
	char pTempCmd[50] = { 0 };

	const char *pStr = NULL;

	OpenIniFile(MAC);	
	
	pStr = ReadString(MAC_SEC_GLOBAL, MAC_KEY_ADDRESS, MAC_VALUE_ADDRESS);
	strcpy(pTempMAC, pStr);
	CloseIniFile();

	system("ifconfig eth0 down");

	sprintf(pTempCmd, "ifconfig eth0 hw ether %s", pStr);
	system(pTempCmd);
		
	DWORD2IP_Str(g_SysConfig.IPAddr, pTempIP);
	sprintf(pTempCmd, "ifconfig eth0 %s", pTempIP);
	system(pTempCmd);

	system("ifconfig eth0 up");

	printf("IPAddr:%s\n", pTempIP);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SetCamera
**	AUTHOR:		   Jeff Wang
**	DATE:		8 - July - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**	
*/

VOID 
SetCamera(bool Flag)
{
	INT fd,nRet;
	CHAR *dev  = "/dev/tvp5150";

	fd = open(dev, O_RDWR);
	if(-1 == fd)
	{ 			
		printf("Can't Open device\n");
	}

	if(Flag)
		{
			 ioctl(fd, TVP5150_IOC_CAM_OPEN, NULL);
		}
	else
		{
			 ioctl(fd, TVP5150_IOC_CAM_CLOSE, NULL);
		}

	if (1 == g_SysConfig.ActiveCamera) 
	{
		nRet = ioctl(fd, TVP5150_IOC_CHANNEL_A, NULL);
		if (0 == nRet)
		{
			printf("******Internal Camera Set******\n");
		}
	}
	else if (2 == g_SysConfig.ActiveCamera) 
	{
		nRet = ioctl(fd, TVP5150_IOC_CHANNEL_B, NULL);
		if (0 == nRet)
		{
			printf("******External Camera Set******\n");
		}
	}
	close(fd);  
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IP_Str2DWORD()
**	AUTHOR:			Harry Qian
**	DATE:			25 - Sep - 2006
**
**	DESCRIPTION:	
**			If the Talk Module in idle state. then talk can update.
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
DWORD
IP_Str2DWORD(char * pIP)
{
	int nIP[4] = {0};
	if (NULL == pIP )
	{
		return 0;
	}

	sscanf(pIP, "%d.%d.%d.%d", &nIP[0], &nIP[1], &nIP[2], &nIP[3]);
	return MAKEDWORD(nIP[0], nIP[1], nIP[2], nIP[3]);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DWORD2IP_Str
**	AUTHOR:		   Jeff Wang
**	DATE:		8 - July - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Anything need to be noticed.
*/
static VOID 
DWORD2IP_Str(DWORD dwIPAddr, char *pIPStr)
{

	sprintf(pIPStr, "%d.%d.%d.%d",
							AHIBYTE(dwIPAddr),
							ALOBYTE(dwIPAddr),
							BHIBYTE(dwIPAddr),
							BLOBYTE(dwIPAddr)
							);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ParaVldDetect
**	AUTHOR:		   Jeff Wang
**	DATE:		31 - Oct - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Anything need to be noticed.
*/
static void 
ParaVldDetect()
{
	if (g_ASPara.RelayPulseWith >= 30000) 
	{
		g_ASPara.RelayPulseWith = 30000;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IsKeyInputComplete
**	AUTHOR:		   Jeff Wang
**	DATE:		11 - July - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Anything need to be noticed.
*/
BOOL 
IsKeyInputComplete(CHAR *pInputBuffer, INT *pInputBufferLen, UCHAR *pKeyDataValue)
{	
	if ( (*pKeyDataValue >= KEY_NUM_0) && ( *pKeyDataValue <= KEY_NUM_9) )
	{		
		pInputBuffer[*pInputBufferLen] = *pKeyDataValue;
		(*pInputBufferLen)++;		
	}
	else if (KEY_NUM_ENTER == *pKeyDataValue) 
	{
		return TRUE;
	}
	else if (*pInputBufferLen > 19)
	{
		printf("Input too more data\n");
	}
	return FALSE;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SetDeivceFun
**	AUTHOR:		   Jeff Wang
**	DATE:		29 - Oct - 2008
**
**	DESCRIPTION:	
**				
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Anything need to be noticed.
*/
static void 
SetDeivceFun()
{
/***************************************************   new     **********************************/
////Digital

	 if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM001))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM002))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM003))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM004))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM005))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM001))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM002))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM003))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM004))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM005)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM006))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM007))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM008))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM009))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM010))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM006))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM007))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM008))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM009))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM010)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM011))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM012))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM013))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM014))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM015))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM011))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM012))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM013))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM014))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM015)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM016))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM017))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM018))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM019))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM020))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM016))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM017))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM018))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM019))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM020)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM021))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM022))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM023))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM024))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM025))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM021))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM022))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM023))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM024))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM025)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM026))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM027))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM028))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM029))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM030))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM026))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM027))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM028))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM029))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM030))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, C_AEGM001))//Amber GM
		||(0 == PNCompare(g_FactorySet.ModuleType, C_AEGM002))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_AEGM003))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_AEGM001))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_AEGM002))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_AEGM003))
		
		)
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM031))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM032))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM033))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM034))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM035))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM031))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM032))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM033))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM034))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM035)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM036))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM037))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM038))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM039))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM040))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM036))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM037))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM038))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM039))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM040)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM041))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM042))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM043))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM044))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM045))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM041))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM042))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM043))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM044))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM045)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM046))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM047))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM048))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM049))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM050))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM046))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM047))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM048))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM049))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM050))
		
		)
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM051))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM052))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM053))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM054))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM055))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM051))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM052))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM053))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM054))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM055)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM056))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM057))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM058))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM059))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM060))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM056))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM057))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM058))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM059))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM060)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM061))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM062))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM063))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM064))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM065))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM061))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM062))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM063))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM064))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM065)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM066))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM067))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM068))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM069))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM070))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM066))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM067))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM068))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM069))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM070)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM071))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM072))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM073))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM074))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM075))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM071))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM072))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM073))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM074))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM075)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	

//Single Home with Digital Keypad
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM076))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM077))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM078))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM079))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM080))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM076))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM077))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM078))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM079))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM080)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM081))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM082))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM083))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM084))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM085))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM081))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM082))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM083))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM084))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM085)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM086))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM087))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM088))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM089))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM090))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM086))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM087))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM088))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM089))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM090)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM091))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM092))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM093))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM094))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM095))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM091))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM092))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM093))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM094))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM095)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM096))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM097))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM098))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM099))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM100))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM096))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM097))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM098))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM099))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM100)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}

	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM101))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM102))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM103))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM104))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM105))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM101))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM102))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM103))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM104))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM105)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM106))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM107))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM108))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM109))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM110))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM106))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM107))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM108))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM109))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM110)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM111))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM112))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM113))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM114))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM115))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM111))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM112))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM113))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM114))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM115)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM116))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM117))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM118))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM119))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM120))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM116))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM117))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM118))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM119))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM120)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM121))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM122))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM123))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM124))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM125))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM121))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM122))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM123))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM124))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM125)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM126))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM127))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM128))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM129))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM130))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM126))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM127))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM128))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM129))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM130)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM131))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM132))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM133))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM134))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM135))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM131))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM132))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM133))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM134))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM135)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM136))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM137))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM138))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM139))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM140))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM136))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM137))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM138))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM139))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM140)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM141))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM142))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM143))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM144))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM145))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM141))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM142))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM143))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM144))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM145)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM146))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM147))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM148))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM149))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM150))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM146))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM147))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM148))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM149))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM150))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM161))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM162))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM163))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM164))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM165))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM161))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM162))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM163))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM164))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM165))
		
		/////
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM676))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM677))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM678))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM679))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM680))

		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM696))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM697))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM698))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM699))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM700))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM716))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM717))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM718))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM719))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM720))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM676))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM677))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM678))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM679))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM680))

		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM696))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM697))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM698))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM699))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM700))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM716))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM717))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM718))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM719))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM720))
		)
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM151))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM152))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM153))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM154))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM155))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM151))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM152))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM153))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM154))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM155)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM156))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM157))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM158))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM159))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM160))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM156))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM157))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM158))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM159))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM160))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM166))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM167))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM168))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM169))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM170))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM171))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM172))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM173))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM174))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM175))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM176))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM177))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM178))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM179))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM180))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM181))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM182))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM183))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM184))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM185))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM166))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM167))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM168))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM169))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM170))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM171))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM172))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM173))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM174))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM175))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM176))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM177))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM178))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM179))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM180))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM181))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM182))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM183))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM184))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM185))
		
		
		//Single Home
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM681))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM682))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM683))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM684))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM685))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM686))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM687))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM688))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM689))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM690))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM691))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM692))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM693))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM694))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM695))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM701))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM702))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM703))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM704))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM705))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM706))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM707))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM708))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM709))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM710))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM711))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM712))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM713))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM714))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM715))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM723))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM724))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM725))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM726))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM727))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM728))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM729))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM730))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM731))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM732))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM733))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM734))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM735))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM736))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM737))

		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM681))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM682))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM683))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM684))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM685))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM686))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM687))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM688))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM689))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM690))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM691))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM692))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM693))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM694))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM695))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM701))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM702))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM703))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM704))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM705))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM706))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM707))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM708))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM709))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM710))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM711))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM712))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM713))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM714))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM715))
		
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM723))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM724))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM725))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM726))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM727))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM728))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM729))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM730))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM731))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM732))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM733))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM734))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM735))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM736))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM737))
		)
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}

////Direct GM(14 Home)

	 else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM201))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM202))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM203))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM204))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM205))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM201))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM202))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM203))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM204))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM205)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM206))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM207))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM208))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM209))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM210))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM206))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM207))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM208))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM209))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM210)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM211))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM212))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM213))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM214))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM215))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM211))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM212))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM213))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM214))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM215)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM216))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM217))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM218))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM219))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM220))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM216))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM217))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM218))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM219))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM220)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM221))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM222))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM223))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM224))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM225))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM221))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM222))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM223))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM224))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM225)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM226))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM227))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM228))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM229))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM230))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM226))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM227))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM228))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM229))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM230)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM231))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM232))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM233))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM234))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM235))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM231))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM232))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM233))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM234))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM235)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM236))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM237))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM238))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM239))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM240))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM236))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM237))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM238))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM239))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM240)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM241))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM242))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM243))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM244))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM245))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM241))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM242))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM243))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM244))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM245)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM246))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM247))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM248))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM249))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM250))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM246))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM247))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM248))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM249))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM250)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM251))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM252))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM253))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM254))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM255))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM251))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM252))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM253))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM254))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM255)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM256))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM257))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM258))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM259))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM260))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM256))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM257))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM258))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM259))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM260)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM261))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM262))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM263))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM264))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM265))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM261))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM262))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM263))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM264))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM265)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM266))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM267))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM268))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM269))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM270))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM266))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM267))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM268))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM269))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM270)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM271))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM272))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM273))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM274))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM275))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM271))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM272))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM273))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM274))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM275)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}	

////Direct GM(12 Home)
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM301))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM302))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM303))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM304))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM305))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM301))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM302))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM303))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM304))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM305)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM306))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM307))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM308))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM309))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM310))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM306))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM307))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM308))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM309))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM310)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM311))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM312))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM313))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM314))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM315))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM311))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM312))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM313))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM314))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM315)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM316))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM317))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM318))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM319))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM320))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM316))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM317))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM318))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM319))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM320)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM321))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM322))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM323))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM324))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM325))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM321))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM322))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM323))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM324))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM325)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM326))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM327))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM328))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM329))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM330))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM326))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM327))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM328))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM329))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM330)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM331))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM332))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM333))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM334))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM335))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM331))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM332))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM333))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM334))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM335)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM336))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM337))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM338))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM339))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM340))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM336))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM337))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM338))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM339))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM340)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM341))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM342))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM343))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM344))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM345))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM341))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM342))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM343))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM344))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM345)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM346))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM347))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM348))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM349))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM350))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM346))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM347))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM348))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM349))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM350)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM351))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM352))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM353))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM354))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM355))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM351))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM352))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM353))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM354))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM355)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM356))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM357))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM358))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM359))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM360))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM356))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM357))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM358))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM359))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM360)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM361))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM362))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM363))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM364))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM365))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM361))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM362))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM363))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM364))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM365)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM366))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM367))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM368))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM369))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM370))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM366))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM367))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM368))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM369))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM370)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM371))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM372))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM373))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM374))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM375))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM371))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM372))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM373))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM374))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM375)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	
////Direct GM(10 Home)	

	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM401))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM402))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM403))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM404))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM405))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM401))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM402))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM403))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM404))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM405)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM406))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM407))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM408))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM409))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM410))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM406))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM407))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM408))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM409))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM410)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM411))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM412))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM413))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM414))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM415))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM411))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM412))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM413))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM414))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM415)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM416))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM417))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM418))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM419))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM420))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM416))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM417))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM418))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM419))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM420)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM421))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM422))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM423))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM424))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM425))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM421))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM422))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM423))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM424))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM425)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM426))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM427))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM428))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM429))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM430))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM426))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM427))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM428))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM429))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM430)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM431))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM432))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM433))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM434))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM435))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM431))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM432))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM433))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM434))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM435)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM436))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM437))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM438))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM439))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM440))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM436))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM437))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM438))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM439))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM440)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM441))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM442))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM443))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM444))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM445))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM441))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM442))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM443))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM444))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM445)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM446))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM447))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM448))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM449))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM450))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM446))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM447))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM448))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM449))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM450)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM451))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM452))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM453))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM454))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM455))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM451))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM452))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM453))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM454))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM455)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM456))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM457))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM458))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM459))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM460))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM456))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM457))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM458))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM459))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM460)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM461))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM462))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM463))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM464))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM465))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM461))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM462))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM463))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM464))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM465)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM466))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM467))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM468))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM469))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM470))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM466))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM467))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM468))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM469))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM470)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM471))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM472))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM473))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM474))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM475))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM471))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM472))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM473))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM474))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM475)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	

////Direct GM(4 Home)
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM501))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM502))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM503))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM504))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM505))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM501))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM502))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM503))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM504))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM505)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM506))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM507))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM508))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM509))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM510))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM506))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM507))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM508))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM509))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM510)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM511))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM512))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM513))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM514))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM515))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM511))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM512))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM513))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM514))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM515)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM516))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM517))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM518))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM519))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM520))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM516))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM517))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM518))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM519))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM520)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM521))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM522))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM523))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM524))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM525))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM521))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM522))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM523))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM524))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM525)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM526))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM527))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM528))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM529))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM530))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM526))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM527))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM528))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM529))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM530)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM531))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM532))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM533))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM534))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM535))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM531))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM532))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM533))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM534))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM535)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM536))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM537))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM538))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM539))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM540))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM536))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM537))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM538))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM539))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM540)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM541))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM542))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM543))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM544))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM545))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM541))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM542))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM543))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM544))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM545)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM546))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM547))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM548))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM549))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM550))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM546))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM547))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM548))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM549))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM550)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM551))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM552))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM553))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM554))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM555))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM551))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM552))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM553))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM554))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM555)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM556))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM557))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM558))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM559))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM560))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM556))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM557))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM558))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM559))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM560)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM561))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM562))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM563))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM564))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM565))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM561))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM562))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM563))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM564))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM565)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM566))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM567))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM568))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM569))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM570))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM566))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM567))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM568))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM569))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM570)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM571))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM572))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM573))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM574))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM575))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM571))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM572))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM573))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM574))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM575)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}

////Direct GM(8 Home)

	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM601))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM602))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM603))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM604))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM605))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM601))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM602))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM603))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM604))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM605)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM606))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM607))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM608))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM609))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM610))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM606))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM607))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM608))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM609))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM610)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM611))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM612))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM613))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM614))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM615))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM611))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM612))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM613))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM614))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM615)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM616))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM617))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM618))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM619))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM620))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM616))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM617))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM618))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM619))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM620)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM621))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM622))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM623))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM624))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM625))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM621))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM622))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM623))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM624))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM625)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM626))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM627))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM628))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM629))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM630))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM626))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM627))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM628))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM629))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM630)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM631))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM632))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM633))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM634))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM635))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM631))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM632))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM633))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM634))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM635)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM636))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM637))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM638))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM639))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM640))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM636))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM637))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM638))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM639))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM640)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM641))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM642))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM643))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM644))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM645))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM641))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM642))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM643))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM644))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM645)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM646))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM647))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM648))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM649))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM650))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM646))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM647))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM648))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM649))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM650)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM651))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM652))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM653))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM654))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM655))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM651))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM652))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM653))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM654))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM655)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM656))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM657))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM658))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM659))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM660))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM656))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM657))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM658))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM659))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM660)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM661))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM662))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM663))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM664))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM665))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM661))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM662))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM663))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM664))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM665)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM666))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM667))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM668))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM669))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM670))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM666))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM667))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM668))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM669))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM670)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM671))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM672))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM673))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM674))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM675))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM671))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM672))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM673))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM674))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM675)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}

	
////Direct GM(2 Home)
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM801))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM802))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM803))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM804))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM805))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM801))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM802))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM803))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM804))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM805)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM806))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM807))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM808))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM809))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM810))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM806))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM807))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM808))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM809))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM810)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM811))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM812))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM813))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM814))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM815))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM811))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM812))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM813))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM814))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM815)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM816))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM817))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM818))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM819))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM820))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM816))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM817))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM818))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM819))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM820)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM821))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM822))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM823))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM824))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM825))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM821))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM822))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM823))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM824))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM825)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM826))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM827))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM828))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM829))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM830))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM826))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM827))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM828))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM829))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM830)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM831))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM832))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM833))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM834))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM835))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM831))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM832))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM833))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM834))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM835)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM836))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM837))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM838))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM839))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM840))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM836))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM837))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM838))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM839))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM840)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM841))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM842))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM843))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM844))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM845))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM841))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM842))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM843))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM844))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM845)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM846))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM847))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM848))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM849))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM850))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM846))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM847))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM848))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM849))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM850)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM851))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM852))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM853))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM854))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM855))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM851))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM852))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM853))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM854))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM855)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM856))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM857))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM858))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM859))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM860))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM856))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM857))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM858))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM859))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM860)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM861))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM862))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM863))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM864))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM865))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM861))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM862))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM863))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM864))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM865)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM866))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM867))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM868))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM869))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM870))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM866))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM867))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM868))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM869))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM870)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_NEGM871))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM872))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM873))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM874))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_NEGM875))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM871))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM872))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM873))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM874))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_NEGM875)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = TRUE;
	}
	
/**************************************** old *************************************/
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM01))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM01)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM02))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM02)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM03))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM03)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM04))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM04)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM05))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM05)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM06))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM06)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM07))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM07)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM08))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM08)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM09))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM09)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM10))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM10)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}


	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM11))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM11)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM12))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM12)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM13))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM13)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM14))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM14)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM15))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM15)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM16))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM16)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM17))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM17)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM18))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM18)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM19))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM19)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM20))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM20)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM21))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM21)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM22))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM22)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM23))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM23)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM24))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM24)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM25))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM25)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM26))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM26)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM27))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM27)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM28))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM28)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM29))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM29)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM30))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM30)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM31))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM31)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM32))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM32)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM33))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM33)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM34))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM34)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM35))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM35)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM36))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM36)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM37))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM37)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM38))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM38)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM39))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM39)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM40))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM40)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM41))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM41)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM42))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM42)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}

	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM43))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM43)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM44))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM44)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM45))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM45)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}

	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM46))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM46)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM47))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM47)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM48))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM48)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM49))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM49)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM50))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM50)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM51))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM51)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}

	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM52))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM52)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM53))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM53)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM54))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM54)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}

	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM55))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM55)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM56))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM56)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM57))
		||(0 == PNCompare(g_FactorySet.ModuleType, E_EGM57)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DB;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM80))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM81))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM82)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM83))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM84))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM85)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM86))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM87))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM88)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM89))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM90))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM91)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM92))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM93))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM94)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}

	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM95))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM96))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM97)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM98))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM99))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM100)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM101))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM102))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM103)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}

	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM104))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM105))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM106)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM107))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM108))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM109)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	

	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM110))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM111))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM112)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM113))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM114))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM115)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM116))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM117))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM118)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM119))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM120))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM121)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM122))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM123))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM124)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}





//////////10 keys
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM125))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM126))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM127)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM128))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM129))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM130)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM131))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM132))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM133)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM134))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM135))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM136)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM137))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM138))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM139)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}	
	
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM140))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM141))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM142)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM143))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM144))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM145)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM146))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM147))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM148)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM149))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM150))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM151)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM152))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM153))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM154)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}	
	
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM155))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM156))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM157)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM158))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM159))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM160)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM161))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM162))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM163)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM164))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM165))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM166)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM167))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM168))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM169)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}	

//////////////////////////////////////////////////////





//////////4 keys
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM170))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM171))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM172)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM173))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM174))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM175)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM176))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM177))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM178)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM179))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM180))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM181)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM182))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM183))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM184)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}	
	
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM185))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM186))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM187)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM188))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM189))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM190)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM191))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM192))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM193)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM194))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM195))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM196)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM197))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM198))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM199)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}	
	
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM200))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM201))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM202)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM203))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM204))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM205)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM206))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM207))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM208)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM209))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM210))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM211)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM212))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM213))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM214)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}	

//////////////////////////////////////////////////////
	


//////////8 keys
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM215))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM216))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM217)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM218))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM219))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM230)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM231))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM232))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM233)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM234))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM235))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM236)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM237))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM238))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM239)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}	
	
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM240))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM241))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM242)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM243))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM244))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM245)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM246))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM247))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM248)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM249))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM250))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM251)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM252))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM253))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM254)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}	
	
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM255))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM256))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM257)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM258))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM259))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM260)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM261))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM262))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM263)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	TRUE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM264))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM265))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM266)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}
	else if ((0 == PNCompare(g_FactorySet.ModuleType, C_EGM267))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM268))
		||(0 == PNCompare(g_FactorySet.ModuleType, C_EGM269)))
	{
		g_DevFun.DevType	=	DEVICE_CORAL_DIRECT_GM;
		g_DevFun.bCardControlUsed	=	FALSE;
		g_DevFun.bNewPnUsed = FALSE;
	}	

//////////////////////////////////////////////////////
	


	
	
	if (DEVICE_CORAL_GM == g_DevFun.DevType) 
	{
		printf("*****Device Coral GM************\n");			
	}
	else if (DEVICE_CORAL_DB == g_DevFun.DevType) 
	{
		printf("*****Device Coral DB************\n");
	}
	else if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType) 
	{
		printf("*****Device Coral Direct GM************\n");
	}
	
	if (g_DevFun.bCardControlUsed)
	{
		printf("CardControl Used\n");
	}
}


static BOOL 
PNCompare (CHAR *DestSting, CHAR *SoueSting)
{
	CHAR TempStr1[32] = {0};
	CHAR TempStr2[32] = {0};
	int	 i = 0;
	int	 j = 0;

	for(i = 0, j = 0; i < strlen(DestSting); i++)
	{
		if (DestSting[i] >= '0' && DestSting[i] <= '9') 
		{
				TempStr1[j] = DestSting[i];
				j++;
		}
	}
	for(i = 0, j = 0; i < strlen(SoueSting); i++)
	{
		if (SoueSting[i] >= '0' && SoueSting[i] <= '9') 
		{
			TempStr2[j] = SoueSting[i];
			j++;
		}
	}
	if (0 == strcmp(TempStr1,TempStr2))
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

VOID SetAudioModule()
{
	if (SERIES_SHELL == g_DevFun.SeriesType || SERIES_MINI_SHELL == g_DevFun.SeriesType) 
	{
		g_SysConfig.AudioMode = AUDIO_MODULE_CX20707;
		SetGMTalkVolume();
	}
	else
	{
		g_SysConfig.AudioMode = AUDIO_MODULE_AIC3104;
	}
}

VOID InstallAudioModule()
{
	if(AUDIO_MODULE_CX20707 == g_SysConfig.AudioMode)
	{
#ifdef MENU_PARA_PROC_DEBUG
		printf("insmod /mox/ssi-CX20707.ko\n");
#endif
		system("insmod /mox/ssi-CX20707.ko");
	}
	else if(AUDIO_MODULE_AIC3104 == g_SysConfig.AudioMode)
	{
#ifdef MENU_PARA_PROC_DEBUG
		printf("insmod /mox/ssi-normal.ko\n");
#endif		
		system("insmod /mox/ssi-normal.ko");
	}
	else
	{
		printf("Warning:AudioMode not set ,use default\n");
		system("insmod /mox/ssi-normal.ko");
	}
}

#ifdef __SUPPORT_PROTOCOL_900_1A__
int GetDOMode(int id)
{
	switch(id)
	{
		case DOPORT_1:
			return g_NewSysConfig.DO1Mode;
		case DOPORT_2:
			return g_NewSysConfig.DO2Mode;
		default:
			break;
	}
	return SC_VALUE_KEY_DO1MODE;
}
int GetDIMode(int id)
{
	switch(id)
	{
		case DIPORT_1:
			return g_NewSysConfig.DI1Mode;
		case DIPORT_2:
			return g_NewSysConfig.DI2Mode;
		default:
			break;
	}
	return SC_VALUE_KEY_DI1MODE;
}

int GetDIFunction(int id)
{
	switch(id)
	{
		case DIPORT_1:
			return g_NewSysConfig.DI1Fun;
		case DIPORT_2:
			return g_NewSysConfig.DI2Fun;
		default:
			break;
	}
	return SC_VALUE_KEY_DI1FUN;
}

int SetNewSysConfig(SYSINFO_NEW_t nConf)
{
	printf("[%s]\n",__FUNCTION__);

	//protocol remove Language EnWigand DI1Conf DI2Conf VldCodeDigits
	nConf.Language = g_SysConfig.LangSel;
	nConf.EnWiegand = nConf.WiegandBit > 0 ? TRUE : FALSE;
	nConf.DI1Conf = 0;
	nConf.DI2Conf = 0;
	nConf.VldCodeDigits = g_SysConfig.VldCodeDigits;
	
	memcpy(&g_NewSysConfig, &nConf, sizeof(SYSINFO_NEW_t));
	
	OpenIniFile(SC);
	WriteNewSCPara ();
	WriteIniFile(SC);
	CloseIniFile();
	OpenIniFile(AS);
	WriteNewASPara();
	WriteIniFile(AS);
	CloseIniFile();
	ShowNewSysCofig();

	usleep(20*1000);
	system("reboot");	
//	while (1);

	return TRUE;
}

int GetNewSysConfig(SYSINFO_NEW_t *pConf)
{
	printf("[%s]\n",__FUNCTION__);
	memcpy(pConf, &g_NewSysConfig, sizeof(SYSINFO_NEW_t));
	ShowNewSysCofig();

	return TRUE;
}

void SetNewSysConfigVersion(int ver)
{
	OpenIniFile(SC);
	WriteInt(SC_SEC_GLOBAL_900_1A, SC_KEY_VERSION, ver);
	WriteIniFile(SC);
	CloseIniFile();
}

static void ShowNewSysCofig()
{
	#if 1
	printf("[Version]\t%ld\n", g_NewSysConfig.Version);
	printf("[IPAddr]\t0x%lX\n", g_NewSysConfig.IPAddr);
	printf("[Mask]\t\t0x%lX\n", g_NewSysConfig.Mask);
	printf("[ServerIP]\t0x%lX\n", g_NewSysConfig.ServerIP);
	printf("[Language]\t%d\n", g_NewSysConfig.Language);
	printf("[CameraMode]\t%d\n", g_NewSysConfig.CameraMode);
	printf("[EnTamperAlarm]\t%d\n", g_NewSysConfig.EnTamperAlarm);
	printf("[TalkTime]\t%d\n", g_NewSysConfig.TalkTime);	
	printf("[EnWiegand]\t%d\n", g_NewSysConfig.EnWiegand);
	printf("[WiegandBit]\t%d\n", g_NewSysConfig.WiegandBit);
	printf("[WiegandRev]\t%d\n", g_NewSysConfig.WiegandRev);
	printf("[DI1Mode]\t%d\n", g_NewSysConfig.DI1Mode	);
	printf("[DI1Conf]\t%d\n", g_NewSysConfig.DI1Conf	);
	printf("[DI1Fun]\t%d\n", g_NewSysConfig.DI1Fun);	
	printf("[DI2Mode]\t%d\n", g_NewSysConfig.DI2Mode);	
	printf("[DI2Conf]\t%d\n", g_NewSysConfig.DI2Conf);
	printf("[DI2Fun]\t%d\n", g_NewSysConfig.DI2Fun);	
	printf("[DO1Mode]\t%d\n", g_NewSysConfig.DO1Mode);	
	printf("[DO2Mode]\t%d\n", g_NewSysConfig.DO2Mode);	
	printf("[TalkVol]\t%d\n", g_NewSysConfig.TalkVol);	
	printf("[RingTime]\t%d\n", g_NewSysConfig.RingTime);
	printf("[SysPwd]\t%s\n", g_NewSysConfig.SysPwd);
	printf("[VldCodeDigits]\t%d\n", g_NewSysConfig.VldCodeDigits);
	#endif /* aaron */
}
#endif 

