
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MwMain.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		20 - Aug - 2008
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <device.h>
#include <sys/mman.h>
#include <pthread.h>
#include <linux/sockios.h>
#include <errno.h>
#include <net/if.h>
#include <winman.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "Dispatch.h"
#include "MXCommon.h"
#include "SystemInfoAPI.h"

#include "TalkEventWnd.h"
#include "TalkLogReport.h"
#include "TalkPrjWnd.h"
#include "TalkLogReport.h"
#include "Multimedia.h"
#include "MenuParaProc.h"
#include "ManOpenSetWnd.h"
#include "MainMenu.h"
#include "HelpInfoWnd.h"
#include "GateOpenModeSetWnd.h"
#include "PwdModifyWnd.h"
#include "PwdUlk.h"
#include "PwdUlkWnd.h"
#include "PwdModifyWnd.h"
#include "Eth.h"
#include "ASPwdModWnd.h"
#include "ParaSetting.h"
#include "IOControl.h"
#include "DiagTelnetd.h"
#include "PioApi.h"
#include "AccessProc.h"
#include "CardProc.h"
#include "ASRdPwdModWnd.h"
#include "CheckNetStatus.h"
#include "rtc.h"
#include "RS485.h"
#include "AsAlarmWnd.h"
#include "SAAlarmWnd.h"
#include "SysLogProc.h"
#include "EGMLCAgent.h"
#include "LiftCtrlConversion.h"
#include "EGMLiftControl.h"
#include "YUV2Jpg.h"
#include "SecurityAlarm.h"
#include "ParaSetting.h"
#include "ACAgentProc.h"
#include "GPVideo.h"
#include "MenuParaProc.h"
#include "UnicodeString.h"
#include "LiftCtrlByDOModule.h"
#include "AccessLogReport.h"
#ifdef __SUPPORT_PROTOCOL_900_1A__
#include "DIDOClient.h"
#include "soapcommon.h"
#endif 


/************** DEFINES **************************************************************/
//#define ETHGM_MAIN_DEBUG
#define DEBUG_ETHGMMAIN
//#define DEBUG_MAINTIMER
//#define DEBUG_MAINPAINT


#ifdef NEW_OLED_ENABLE
#define  DISP_STEP		8
#else
#define  DISP_STEP		16
#endif


#define  MWMAINDISCOUNT 5

#define CALLDELAYTIME	 2000
#define PRMPTMSGDISPTIME 1000
#define FUNSELSTAYTIME	 2000
#define ERRORTIPDISTIME	 2000

/************** TYPEDEFS *************************************************************/

typedef enum _MAINWNDSTATUS
{
	STATUS_MSGHELP_WINDOW = 1,
	STATUS_FUN_SEL_WND,
	STATUS_CODE_WINDOW,
	STATUS_CODE_INPUT_ERROR,
	STATUS_PASSWORD_INPUT_ERROR,
	STATUS_PASSWORD_INPUT_OK,
	STATUS_PASSWORD_INPUT_DISABLE,
}MAINWNDSTATUS;


/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

WNDMAN    g_WndMan;

static VOID MainWndTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID MainWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID	MainWndKeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID SendRdPassword2ACC(VOID);


MAINWNDSTATUS g_MainWndStatus = STATUS_MSGHELP_WINDOW;

static HFONT hFont48;
static HFONT hFont32;
static HFONT hFont24;
static HFONT	GetHZKFont48(void);
static HFONT	GetHZKFont24(void);
static void HZKFontInit(void);

HWND g_hMainWnd = 0;

BOOL g_bEthLinkStatus						= TRUE;//offline

CHAR g_KeyInputBuffer[KEY_BUF_LEN] = { 0 };
INT  g_KeyInputBufferLen = 0;

CHAR g_KeyInputBkp[KEY_BUF_LEN] = { 0 };
INT  g_KeyInputBkpLen = 0;
HANDLE*   g_HandleHashCard = NULL;//卡片存储的句柄



static void	MulLineDisProc_48(HDC Hdc, char *pBuf, int nBufLen, UCHAR CountStart);

/*************************************************************************************/


void	
MwHookInit()
{

	LoadMenuParaFromMem();
	HZKFontInit();
	IOCtrlInit();	
	SaveMenuPara2Mem();
	SetNetWork();
	PublicFuncMdInit();
	InstallAudioModule();	//modify for shell GM
	TalkInit();	
	MultimediaInit();
	SecurityAlarmInit();
	YUV2JpgInit();
	ParaSettingInit();	
	EthInit();
	PwdUlkInit();
	AccessInit();
	TelnetInit();
	CheckNetStatusInit();
	RtcInit();
	Rs485Init();
	SysLogInit();
	//
	EGMLCAMdInit();
	EGMLCMdInit();
	
	InitGPVideo();
//	LCAMdInit();

	ACAgentInit();
	lift_ctrl_by_DO_module_init();	
#ifdef __SUPPORT_PROTOCOL_900_1A__
	if(MOX_PROTOCOL_SUPPORT(MOX_P_900_1A))
	{
		DIDOClient_Init();
	}
	MoxSoapInit();
#endif 
	usleep(3000 * 1000);

	SetCamera(0);

	g_bNeedRestart = FALSE;	

	WatchDogInit();
	InitUnicodeString();
	LoadHebrewString();

	EthGMLog(TYPE_REBOOT);

#ifdef ETHGM_MAIN_DEBUG
	printf("bCardControlUsed:%d,bNewPnUsed:%d,DevType:%d,SeriesType:%d\n",g_DevFun.bCardControlUsed,g_DevFun.bNewPnUsed,g_DevFun.DevType,g_DevFun.SeriesType);
#endif
}

void	
MwHookProcess()
{
	if (g_bNeedRestart)
	{
		while (1);
	}
	PublicFuncProc();
	TalkProcess();
	SecurityAlarmProcess();
	AccessProc();
	IOCtrlProc();
	CheckNetStatusProc();
	//TestRtc();
//	LCFuncProc();
	EGMLCMdProcess();
	EGMLCAFuncProc();
	ACAgentProc();
}

void	
MwHookExit()
{
	PublicFuncMdExit();
	EthExit();
	PIOExit();
	CheckNetStatusExit();
	RtcExit();
//	LCMdExit();
	EGMLCAMdExit();
	EGMLCMdExit();
	LCConversionMdExit();
	ACAgentExit();
#ifdef __SUPPORT_PROTOCOL_900_1A__
	if(MOX_PROTOCOL_SUPPORT(MOX_P_900_1A))
	{
		DIDOClient_Exit();
	}
	MoxSoapExit();
#endif 
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   PSTR szCmdLine, int iCmdShow)
{
	MSG		Msg;
	BOOL	bHaveMsg;
	WNDCLASS wndclass;
	static char szAppName[] = "EthGMMain";
	
	wndclass.style          = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc    = (WNDPROC)MainWndProc;
	wndclass.cbClsExtra     =0;
	wndclass.cbWndExtra     =0;
	wndclass.hInstance      =0;
	wndclass.hIcon          =0;
	wndclass.hCursor        =0;
	//wndclass.hbrBackground  =(HBRUSH)&g_bkBrush;
	 wndclass.hbrBackground  =(HBRUSH)GetStockObject(BACKGROUND_COLOR);
	wndclass.lpszMenuName   =NULL;
	wndclass.lpszClassName  = szAppName;
	
	RegisterClass(&wndclass);
	g_hMainWnd = CreateWindowEx(0L,
					szAppName,
					"Mox",
					WS_OVERLAPPED | WS_VISIBLE,
					0,
					0,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					NULL,
					NULL,
					NULL,
					NULL);

	MwHookInit();

	ShowWindow(g_hMainWnd,iCmdShow);
	UpdateWindow(g_hMainWnd);

	MainWndSwitch(MainWndType_UNKNOW, MainWndType_EHV);

	g_WndMan.MainWndHdle = g_hMainWnd;
	g_WndMan.nChildWndNum = 0;
	
	g_WndMan.pChildWndInfo = (WNDINFO*) malloc (MAX_WINDOWS_NUM * sizeof(WNDINFO));
    ResetTimer(g_WndMan.MainWndHdle, 101, 1000, NULL);//add by [MichaelMa] at 2013-2-22
	
	while (TRUE)
	{
		bHaveMsg = PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE);
		if (bHaveMsg)
		{
			if (Msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		MwHookProcess();
		
		WatchDog();

		usleep(0);
	}
	
	MwHookExit();
	return Msg.wParam;
}

static void
MainWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC				Hdc;
	PAINTSTRUCT		ps;
	RECT			Rect;
	static	RECT	RectPmt = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	char			pDateBuf[TITLE_BUF_LEN] = {0};

#ifdef XIU_ZHOU_POLICE_STATION
    RECT	RectPmt2 = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	char    pDateBuf2[TITLE_BUF_LEN] = {0};
#endif //XIU_ZHOU_POLICE_STATION
    
	LONG		xPos	=	0;
	
	static BOOL bShowNoLinkInfo = TRUE;
	static BOOL bXstatus		= FALSE;
	static BOOL bYstatus		= FALSE;

	if ((hWnd != GetFocus()) || (!IsWindowVisible(hWnd)))
	{
		return;
	}

	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;
	Hdc->textcolor = FONT_COLOR;
	
#ifdef DEBUG_MAINPAINT
	printf("************MainWndPaintd*****************\n");
	printf("g_MainWndStatus = %d\n", g_MainWndStatus);
#endif

	if (STATUS_MSGHELP_WINDOW == g_MainWndStatus) 
	{
		//SetRtcProc();
		Hdc->textcolor = PROMPT_FONT;
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			SelectObject( Hdc, GetHZKFont32());

			if (g_bEthLinkStatus)
			{
				strcpy(pDateBuf, STR_PRESSHELP_EN);
#ifdef XIU_ZHOU_POLICE_STATION
                strcpy(pDateBuf2, STR_PRESSHELP_EN2);
#endif
			}
			else
			{
				if (bShowNoLinkInfo)
				{
					strcpy(pDateBuf, STR_PRESSHELP_EN);
#ifdef XIU_ZHOU_POLICE_STATION
                strcpy(pDateBuf2, STR_PRESSHELP_EN2);
#endif                    
				}
				else
				{
					strcpy(pDateBuf, STR_NETFAIL_EN);
				}				
				bShowNoLinkInfo = !bShowNoLinkInfo;
			}
#ifndef XIU_ZHOU_POLICE_STATION
#ifdef NEW_OLED_ENABLE
			RectPmt.left = GetXPos(pDateBuf);
			RectPmt.right	= RectPmt.left + 6 * strlen(pDateBuf);
			RectPmt.bottom	= RectPmt.top + 16;
#else
			RectPmt.right	   = RectPmt.left + 16 * strlen(pDateBuf);
			RectPmt.bottom	= RectPmt.top + 40 - 1;
#endif
			
			RectPmt.top  += DISP_STEP;
			if (RectPmt.bottom  >= SCREEN_HEIGHT) 
			{
				RectPmt.top	= 0;
				RectPmt.bottom	= RectPmt.top + 16;
			}

			DrawText(Hdc, pDateBuf, -1, &RectPmt, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
#else
            RectPmt.right	= RectPmt.left + 6 * strlen(pDateBuf);
            RectPmt.left    = 4;
		    RectPmt.bottom	= RectPmt.top + 16 - 1;
            RectPmt.top     = 32;
            DrawText(Hdc, pDateBuf, -1, &RectPmt, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
            if(strcmp(pDateBuf,STR_NETFAIL_EN) != 0)
            {
                RectPmt2.top = RectPmt.top - 16;
                RectPmt2.bottom = RectPmt.bottom - 16;
                RectPmt2.left = RectPmt.left;
                RectPmt2.right = RectPmt.right;
                DrawText(Hdc, pDateBuf2, -1, &RectPmt2, DT_SINGLELINE|DT_LEFT|DT_VCENTER);
            }
#endif //XIU_ZHOU_POLICE_STATION
			
		}
		else if(SET_HEBREW == g_SysConfig.LangSel)
		{
			
			SelectObject( Hdc, GetHZKFont32());

			if (g_bEthLinkStatus)
			{
				//strcpy(pDateBuf, STR_PRESSHELP_EN);
				strcpy(pDateBuf, GetHebrewStr(HS_ID_PRESSHELP)); 
			}
			else
			{
				if (bShowNoLinkInfo)
				{
					strcpy(pDateBuf,  GetHebrewStr(HS_ID_PRESSHELP));
				}
				else
				{
					strcpy(pDateBuf,  GetHebrewStr(HS_ID_NETFAIL));
				}				
				bShowNoLinkInfo = !bShowNoLinkInfo;
			}
			RectPmt.top  += DISP_STEP;
#ifdef NEW_OLED_ENABLE
			RectPmt.left = 0;
			//RectPmt.right	= RectPmt.left + 6 * strlen(pDateBuf);
			RectPmt.right	= SCREEN_WIDTH;
			RectPmt.bottom	= RectPmt.top + HEBREW_STR_HEIGHT;
#else
			RectPmt.right	   = RectPmt.left + 16 * strlen(pDateBuf);
			RectPmt.bottom	= RectPmt.top + 40 - 1;
#endif
			
		
			if (RectPmt.bottom  >= SCREEN_HEIGHT) 
			{
				RectPmt.top	= 0;
				RectPmt.bottom	= RectPmt.top + HEBREW_STR_HEIGHT;
			}
			HebrewDrawText(Hdc, pDateBuf, -1, &RectPmt, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
#ifdef DEBUG_MAINPAINT
			printf(" RectPmt.left = %d RectPmt.top = %d RectPmt.right = %d RectPmt.bottom = %d\n", 
				RectPmt.left, 
				RectPmt.top, 
				RectPmt.right, 
				RectPmt.bottom);
#endif
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			
			SelectObject( Hdc, GetHZKFont32());
			if (g_bEthLinkStatus)
			{
				strcpy(pDateBuf, STR_PRESSHELP_CN);
#ifdef XIU_ZHOU_POLICE_STATION
                strcpy(pDateBuf2, STR_PRESSHELP_CN2);
#endif
			}
			else
			{
				if (bShowNoLinkInfo)
				{
					strcpy(pDateBuf, STR_PRESSHELP_CN);
#ifdef XIU_ZHOU_POLICE_STATION
                    strcpy(pDateBuf2, STR_PRESSHELP_CN2);
#endif
				}
				else
				{
					strcpy(pDateBuf, STR_NETFAIL_CN); 
				}
				bShowNoLinkInfo = !bShowNoLinkInfo;				
			}

#ifndef XIU_ZHOU_POLICE_STATION
			if (bYstatus)
			{
				RectPmt.top -= DISP_STEP;
			}
			else
			{
				RectPmt.top  += DISP_STEP;
			}
			if (bXstatus) 
			{
				RectPmt.left -= DISP_STEP;
			}
			else
			{
				RectPmt.left += DISP_STEP;
			}

#ifdef NEW_OLED_ENABLE
			RectPmt.right		= RectPmt.left + 6 * strlen(pDateBuf);
			RectPmt.bottom	= RectPmt.top + 16 - 1;
#else
			RectPmt.right		= RectPmt.left + 16 * strlen(pDateBuf);
			RectPmt.bottom	= RectPmt.top + 40 - 1;
#endif
			

			if (RectPmt.bottom > SCREEN_HEIGHT
				|| RectPmt.top < 4
				|| RectPmt.right > SCREEN_WIDTH
				|| RectPmt.left < 4)
			{
#ifdef NEW_OLED_ENABLE
				RectPmt.left = rand() % (SCREEN_WIDTH - 6 * strlen(pDateBuf));
				if (RectPmt.left > (SCREEN_WIDTH - 6 * strlen(pDateBuf)/2)) 
				{
					RectPmt.left = Rect.left + SCREEN_WIDTH - 6 * strlen(pDateBuf);
					bXstatus = TRUE;
				}
				else
				{
//					RectPmt.left = Rect.left;
					RectPmt.left = 4;
					bXstatus = FALSE;
				}
				RectPmt.top = rand() % (SCREEN_HEIGHT - 24);
				if (RectPmt.top > ((SCREEN_HEIGHT - 24)/2)) 
				{
					bYstatus = TRUE;
				}
				else
				{
					bYstatus = FALSE;
				}
				if (RectPmt.top < 4) 
				{
					RectPmt.top = 4;
				}
#else
				RectPmt.left = rand() % (SCREEN_WIDTH - 16 * strlen(pDateBuf));
				if (RectPmt.left > (SCREEN_WIDTH - 16 * strlen(pDateBuf)/2)) 
				{
					RectPmt.left = Rect.left + SCREEN_WIDTH - 16 * strlen(pDateBuf);
					bXstatus = TRUE;
				}
				else
				{
					RectPmt.left = Rect.left;
					bXstatus = FALSE;
				}
				RectPmt.top = rand() % (SCREEN_HEIGHT - 32);
				if (RectPmt.top > ((SCREEN_HEIGHT - 32)/2)) 
				{
					bYstatus = TRUE;
				}
				else
				{
					bYstatus = FALSE;
				}

				if (RectPmt.top < 4) 
				{
					RectPmt.top = 4;
				}
#endif
				
#ifdef NEW_OLED_ENABLE
				RectPmt.right	= RectPmt.left + 6 * strlen(pDateBuf);
				RectPmt.bottom	= RectPmt.top + 16 - 1;		
#else
				RectPmt.right	= RectPmt.left + 16 * strlen(pDateBuf);
				RectPmt.bottom	= RectPmt.top + 40 - 1;		
#endif
			}
			DrawText(Hdc, pDateBuf, -1, &RectPmt, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
#else
            RectPmt.right	= RectPmt.left + 6 * strlen(pDateBuf);
            RectPmt.left    = 15;
		    RectPmt.bottom	= RectPmt.top + 16 - 1;
            RectPmt.top     = 32;
            DrawText(Hdc, pDateBuf, -1, &RectPmt, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
            if(strcmp(pDateBuf,STR_NETFAIL_CN) != 0)
            {
                RectPmt2.top = RectPmt.top - 16;
                RectPmt2.bottom = RectPmt.bottom - 16;
                RectPmt2.left = RectPmt.left;
                RectPmt2.right = RectPmt.right;
                DrawText(Hdc, pDateBuf2, -1, &RectPmt2, DT_SINGLELINE|DT_LEFT|DT_VCENTER);
            }
#endif //XIU_ZHOU_POLICE_STATION
		}
	}
	else if (STATUS_FUN_SEL_WND == g_MainWndStatus) 
	{
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos(STR_ENTERFUNNUM_EN);
			MXDrawText_Left(Hdc, STR_ENTERFUNNUM_EN, xPos, 0);
			MXDrawText_Left(Hdc, STR_ENTERPASSWORD_EN, xPos, 1);
		}
		else if(SET_HEBREW == g_SysConfig.LangSel)
		{
			//此处没有希伯来语，暂时使用英文
			xPos = GetXPos(STR_ENTERFUNNUM_EN);
			MXDrawText_Left(Hdc, STR_ENTERFUNNUM_EN, xPos, 0);
			MXDrawText_Left(Hdc, STR_ENTERPASSWORD_EN, xPos, 1);
//			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ENTERFUNNUM));
//			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_ENTERFUNNUM), xPos, 0);
//			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PRESSENTER), xPos, 1);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos(STR_ENTERFUNNUM_CN);
			MXDrawText_Left(Hdc, STR_ENTERFUNNUM_CN, xPos, 0);
			MXDrawText_Left(Hdc, STR_ENTERPASSWORD_CN, xPos, 1);
		}
		

		memset(pDateBuf, 0, TITLE_BUF_LEN);
		pDateBuf[0] = '*';
		memcpy(pDateBuf+1, g_KeyInputBuffer, g_KeyInputBufferLen);
		MulLineDisProc_48(Hdc, pDateBuf, g_KeyInputBufferLen+1, 2);
	}
	else if (STATUS_CODE_WINDOW == g_MainWndStatus) 
	{
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{	
			xPos = GetXPos(STR_ENTERNUM_EN);
			MXDrawText_Left(Hdc, STR_ENTERRES_EN, xPos, 0);
			MXDrawText_Left(Hdc, STR_ENTERNUM_EN, xPos, 1);
		}
		else if(SET_HEBREW == g_SysConfig.LangSel)
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ENTERCALLOCODE));
			//MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_ENTERFUNNUM), xPos, 0);
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_ENTERCALLOCODE), xPos, 1);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{	
			MXDrawText_Center(Hdc,  STR_ENTERRESNUM_CN, 1);
		}		
		memset(pDateBuf, 0, TITLE_BUF_LEN);
		memcpy(pDateBuf, g_KeyInputBuffer, g_KeyInputBufferLen);
		MulLineDisProc_48(Hdc, pDateBuf, g_KeyInputBufferLen, 2);
	}
	else if(STATUS_PASSWORD_INPUT_ERROR == g_MainWndStatus)
	{
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos(STR_PWDIS_EN);
			MXDrawText_Left(Hdc, STR_PWDIS_EN, xPos, 1);
			MXDrawText_Left(Hdc, STR_INCORRECT_EN, xPos, 2);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{			
			MXDrawText_Center(Hdc, STR_PWDINCORRECT_CN, 1);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_PWDINCORRECT), 1);
		}
		
	}
	else if (STATUS_PASSWORD_INPUT_OK == g_MainWndStatus) 
	{
		if (SET_CHINESE == g_SysConfig.LangSel) 
		{	
			MXDrawText_Center(Hdc, STR_GATEUNLOCK_CN, 1);
		}
		else if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			MXDrawText_Center(Hdc, STR_GATEUNLOCK_EN, 1);	
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_GATEUNLOCK), 1);
		}
	}
	else if(STATUS_PASSWORD_INPUT_DISABLE == g_MainWndStatus)
	{
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos(STR_NOTENABLED_EN);
			MXDrawText_Left(Hdc, STR_FUNIS_EN, xPos, 1);
			MXDrawText_Left(Hdc, STR_NOTENABLED_EN, xPos, 2);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{			
			MXDrawText_Center(Hdc, STR_FUNISNOTEN_CN, 1);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_FUNISNOTEN), 1);
		}
		
	}
	EndPaint(hWnd, &ps);
}

static VOID
MainWndTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (STATUS_MSGHELP_WINDOW == g_MainWndStatus) 
	{
		PostMessage(hWnd, WM_PAINT, 0, 0);
	}	
	else if (STATUS_CODE_WINDOW == g_MainWndStatus) 
	{
		if (DEVICE_CORAL_DB == g_DevFun.DevType) 
		{
			if ( 1 == g_KeyInputBufferLen
				&& KEY_DB_CALL == g_KeyInputBuffer[0]				
				)
			{
				g_CallStatus = CALLING_DB_EHV;
			}
			else if ( 2 == g_KeyInputBufferLen 
					&& '0' == g_KeyInputBuffer[0]
					&& '0' == g_KeyInputBuffer[1]
					) 
			{
				g_CallStatus = CALLING_DB_MC;
			}
			else
			{
				g_CallStatus = CALLING_DB_ERROR_CODE;
			}
		}
		else if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType) 
		{
			
			g_CallStatus = CALLING_GM;
		}
		else if (DEVICE_CORAL_GM == g_DevFun.DevType) 
		{
			g_CallStatus = CALLING_GM;
		}
			
		if (ST_ORIGINAL == g_TalkInfo.talking.dwTalkState)
		{
			ShowTalkWnd();
//			ShowTalkUlkWnd();
		}

		g_MainWndStatus = STATUS_MSGHELP_WINDOW;
		ResetTimer(hWnd, TIMER_MAIN, PRMPTMSGDISPTIME, NULL);
	}
	else if (STATUS_CODE_INPUT_ERROR == g_MainWndStatus) 
	{
		g_MainWndStatus = STATUS_MSGHELP_WINDOW;
		ResetTimer(hWnd, TIMER_MAIN, PRMPTMSGDISPTIME, NULL);
		PostMessage(hWnd, WM_PAINT, 0, 0);
	}
	else if (STATUS_FUN_SEL_WND == g_MainWndStatus) 
	{
		g_MainWndStatus = STATUS_MSGHELP_WINDOW;
		ResetTimer(hWnd, TIMER_MAIN, PRMPTMSGDISPTIME, NULL);
		PostMessage(hWnd, WM_PAINT, 0, 0);
		ClearKeyBuffer();
	}
	else if (STATUS_PASSWORD_INPUT_ERROR == g_MainWndStatus || 
			STATUS_PASSWORD_INPUT_OK == g_MainWndStatus 	||
			STATUS_PASSWORD_INPUT_DISABLE == g_MainWndStatus)
	{
		g_MainWndStatus = STATUS_MSGHELP_WINDOW;
		ResetTimer(hWnd, TIMER_MAIN, PRMPTMSGDISPTIME, NULL);
		PostMessage(hWnd, WM_PAINT, 0, 0);
		ClearKeyBuffer();
	}
	
}


static LRESULT CALLBACK
MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{   
	BYTE byResult = 0;
	if(g_WndMan.MainWndHdle == GetFocus())
		SetRtcProc();
	switch (Msg)
	{
	case WM_CREATE:		
		SetTimer(hWnd, TIMER_MAIN, PRMPTMSGDISPTIME, NULL);
		break;

	case WM_PAINT:	
		MainWndPaint(hWnd, Msg, wParam, lParam);
		break;
	case WM_RDPASSWORD:
		byResult = (BYTE)lParam;

		switch(byResult)
		{
			case 0:
			{
				StartPlayRightANote();
				g_MainWndStatus = STATUS_PASSWORD_INPUT_OK;
				break;
			}
			default:
			{
				StartPlayErrorANote();
				g_MainWndStatus = STATUS_PASSWORD_INPUT_ERROR;
				break;
			}
		}
		
		ResetTimer(hWnd, TIMER_MAIN, ERRORTIPDISTIME, NULL);
		PostMessage(hWnd, WM_PAINT, 0, 0);
		break;
	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		if (g_HwndAsAlarm)
		{
			SendMessage(g_HwndAsAlarm,  WM_CHAR, wParam, 0l);
		}
		else if (g_hWNDTalk) 
		{
			SendMessage(g_hWNDTalk,  WM_CHAR, wParam, 0l);
		}
		else
		{
			MainWndKeyProc(hWnd, Msg, wParam, lParam);
			SendMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
	case WM_REFRESHPARENT_WND:
#ifdef DEBUG_MAINPAINT
		printf("Main : WM_REFRESHPARENT_WND\n");		
#endif
		ClearKeyBuffer();
		g_MainWndStatus = STATUS_MSGHELP_WINDOW;
		ResetTimer(hWnd, TIMER_MAIN, PRMPTMSGDISPTIME, NULL);
		SetFocus(hWnd);
		printf("~~~~~~~~~~~~~~~Set Main WND Focus hWnd=%x\n",hWnd);
		SendMessage(hWnd,  WM_PAINT, 0, 0);
		break;
	case WM_TIMER:
		if(CheckRecallKey() && (GetFocus()== hWnd))
		{
			ClearRecallKey();
			MainWndKeyProc(hWnd, Msg, GetRecallKey(), lParam);
		}
		else
		{
			MainWndTimerProc(hWnd, Msg, wParam, lParam);
		}
		
		
		break;
	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MainWndKeyProc
**	AUTHOR:		   Jeff Wang
**	DATE:		28 - Sep - 2008
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
static void
MainWndKeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{	
	wParam =  KeyBoardMap(wParam);

				
				
	if (wParam == KEY_RETURN)
	{
		if ((STATUS_CODE_WINDOW == g_MainWndStatus)
			||(STATUS_FUN_SEL_WND == g_MainWndStatus))
		{
			if(g_KeyInputBufferLen > 1 && STATUS_FUN_SEL_WND == g_MainWndStatus)
			{
				if(g_ASPara.ASOpenMode&MODE_ROOMCODE_PASSWORD_SET)
				{
                    /*
                        Only when the format is *resident pwd*,GM open the door
                    */
					SendRdPassword2ACC();
//					g_MainWndStatus = STATUS_MSGHELP_WINDOW;
//					ResetTimer(hWnd, TIMER_MAIN, PRMPTMSGDISPTIME, NULL);
				}
				else
				{
					StartPlayErrorANote();
					g_MainWndStatus = STATUS_PASSWORD_INPUT_DISABLE;
				}
				
			}
			else
			{
				g_MainWndStatus = STATUS_MSGHELP_WINDOW;
				ResetTimer(hWnd, TIMER_MAIN, PRMPTMSGDISPTIME, NULL);
			}
			
			
		}
		else if (STATUS_MSGHELP_WINDOW == g_MainWndStatus)
		{
			g_MainWndStatus	= STATUS_FUN_SEL_WND;
			ResetTimer(hWnd, TIMER_MAIN, FUNSELSTAYTIME, NULL);
		}
		ClearKeyBuffer();
	}
	else if (wParam >= KEY_NUM_0 && wParam <= KEY_NUM_9) 
	{
	
	
		if (STATUS_MSGHELP_WINDOW == g_MainWndStatus)
		{
			if( DEVICE_CORAL_DB == g_DevFun.DevType && (MODE_ROOMCODE_PASSWORD_SET & g_ASPara.ASOpenMode))
			{
				CreatePwdUnlockWnd(hWnd);
			//	CORAL_DBSendRdPassword2ACC((CHAR)wParam);
			}
			else
			{
				g_MainWndStatus = STATUS_CODE_WINDOW;
				ResetTimer(hWnd, TIMER_MAIN, CALLDELAYTIME, NULL);
			}
		}
		else if (STATUS_FUN_SEL_WND == g_MainWndStatus) 
		{
			ResetTimer(hWnd, TIMER_MAIN, FUNSELSTAYTIME, NULL);
		}
		else if (STATUS_CODE_WINDOW == g_MainWndStatus) 
		{
			ResetTimer(hWnd, TIMER_MAIN, CALLDELAYTIME, NULL);
		}
		CircleBufEnter(g_KeyInputBuffer, &g_KeyInputBufferLen, KEY_BUF_LEN-1, (CHAR)wParam);
	}
	else if (KEY_DB_CALL == wParam && DEVICE_CORAL_DB == g_DevFun.DevType) 
	{
		ClearKeyBuffer();
		g_MainWndStatus = STATUS_CODE_WINDOW;
		ResetTimer(hWnd, TIMER_MAIN, 10, NULL);
		
		if (g_KeyInputBufferLen < KEY_BUF_LEN) 
		{
			g_KeyInputBuffer[g_KeyInputBufferLen++] = (CHAR)wParam;
		}
		else
		{
			g_KeyInputBufferLen		=	0;	
			g_KeyInputBuffer[g_KeyInputBufferLen++] = (CHAR)wParam;
		}
	}
	else if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
	{
		if (wParam >= KEY_ROOM_1 && wParam <= KEY_ROOM_15) 
		{
			if (!IsAlarming()) 
			{
				ClearKeyBuffer();
				g_MainWndStatus = STATUS_CODE_WINDOW;
				ResetTimer(hWnd, TIMER_MAIN, 10, NULL);
				
				g_KeyInputBufferLen = strlen(g_SysConfig.CallCode[wParam - 1]);
				strcpy(g_KeyInputBuffer,g_SysConfig.CallCode[wParam - 1]);
				
				SetCalloutKey(g_KeyInputBuffer);
				printf("Call Code Len = %d; Call Code: %s\n",g_KeyInputBufferLen,g_KeyInputBuffer);
			}
			else
			{
				printf("Alarm Status!!!!!!\n");
			}
			
		}
	}
		
	else if (wParam == KEY_ENTER)
	{
		if (STATUS_FUN_SEL_WND == g_MainWndStatus)
		{
			if (1 == g_KeyInputBufferLen) 
			{
				if ('1' == g_KeyInputBuffer[0]) 
				{
					CreatePwdModifyWnd(hWnd);
				}
				else if ('2' == g_KeyInputBuffer[0]) 
				{
					CreatePwdUnlockWnd(hWnd);
				}
				else if ('3' == g_KeyInputBuffer[0]) 
				{
					if (g_DevFun.bCardControlUsed)
					{
						CreatePwdModWnd(hWnd);
						g_ASInfo.ASWorkStatus = STATUS_PWDMOD;
					}
				}
				else if ('4' == g_KeyInputBuffer[0]) 
				{
					if (g_DevFun.bCardControlUsed 
						&& (DEVICE_CORAL_DB == g_DevFun.DevType))
					{
						CreateGateOpenModeSetWnd(hWnd);//Gate Open Mode Change
					}
				}
				else if ('5' == g_KeyInputBuffer[0])
				{	
					if (g_DevFun.bCardControlUsed 
						&& (DEVICE_CORAL_DB == g_DevFun.DevType)) 
					{
						CreateManOpenSetWnd(hWnd);//Manually Switch Set
					}
				}
				else if ('6' == g_KeyInputBuffer[0])
				{
					CreateTalkPrjWnd(hWnd);
				}
/*
				else if ('7' == g_KeyInputBuffer[0])
				{
					ClearKeyBuffer();
					g_ASInfo.ASWorkStatus = STATUS_RDPWDMOD;
					CreateRdPwdModWnd(hWnd);
				}
*/
			}
			else if (4 == g_KeyInputBufferLen) 
			{
				if (   ('5' == g_KeyInputBuffer[0]) 
					&& ('6' == g_KeyInputBuffer[1]) 
					&& ('7' == g_KeyInputBuffer[2]) 
					&& ('8' == g_KeyInputBuffer[3]))
				{
					CreateMainMenuWnd(hWnd);
				}
				else if (   ('6' == g_KeyInputBuffer[0])
					&& ('7' == g_KeyInputBuffer[1])
					&& ('8' == g_KeyInputBuffer[2])
					&& ('9' == g_KeyInputBuffer[3]))
				{
					system("reboot");
					while (1);
				}			
			}
			g_MainWndStatus = STATUS_MSGHELP_WINDOW;
			ResetTimer(hWnd, TIMER_MAIN, PRMPTMSGDISPTIME, NULL);
			ClearKeyBuffer();
		}

		else if (STATUS_MSGHELP_WINDOW == g_MainWndStatus) 
		{
			if (0 == g_KeyInputBufferLen) 
			{
				if (DEVICE_CORAL_DB != g_DevFun.DevType && DEVICE_CORAL_DIRECT_GM != g_DevFun.DevType)
				{
					CreateHelpInfoWnd(hWnd);				
				}
				else
				{
					ClearKeyBuffer();
					g_MainWndStatus = STATUS_MSGHELP_WINDOW;
				}
				 
			}
			else
			{
				ClearKeyBuffer();
				g_MainWndStatus = STATUS_MSGHELP_WINDOW;
			}
		}
	/*	else if(STATUS_CODE_WINDOW == g_MainWndStatus && DEVICE_CORAL_DB == g_DevFun.DevType && (MODE_ROOMCODE_PASSWORD_SET & g_ASPara.ASOpenMode))
		{
			CORAL_DBSendRdPassword2ACC(g_KeyInputBuffer,g_KeyInputBufferLen);
		}*/
		else
		{
			ClearKeyBuffer();
			g_MainWndStatus = STATUS_MSGHELP_WINDOW;
		}
	}	
}


static void
HZKFontInit(void)
{
#ifdef NEW_OLED_ENABLE
	if(SET_HEBREW==g_SysConfig.LangSel)
	{
		hFont48 = CreateFont(	12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "HEBREWFONT");
		hFont32 = CreateFont(	12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "HEBREWFONT");
		hFont24 = CreateFont(	12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "HEBREWFONT");
	}
	else
	{
		hFont48 = CreateFont(	12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "HZKFONT");
		hFont32 = CreateFont(	12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "HZKFONT");
		hFont24 = CreateFont(	12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "HZKFONT");
	}


#else
	hFont48 = CreateFont(	HZ_HEIGHT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "HZKFONT");
	hFont32 = CreateFont(	HZ_HEIGHT_32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "HZKFONT");
	hFont24 = CreateFont(	HZ_HEIGHT_24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "HZKFONT");

#endif

#ifdef DEBUG_ETHGMMAIN
	printf("HZKFont Init \n");
#endif
}

static HFONT
GetHZKFont48(void)
{
	return hFont48;
}

HFONT
GetHZKFont32(void)
{
	return hFont32;
}

static HFONT
GetHZKFont24(void)
{
	return hFont24;
}

void 
MXDrawText_En(HDC Hdc, RECT* pRect, char *pBuf, UCHAR Count)
{
	RECT			RectInfo;
	SelectObject( Hdc, GetHZKFont32());

#ifdef NEW_OLED_ENABLE
	Hdc->textcolor = FONT_COLOR;
	RectInfo.left		 = pRect->left;
	RectInfo.top		= pRect->top + Count * 15 + 3;
	RectInfo.right		= pRect->left + SCREEN_WIDTH;
	RectInfo.bottom	= RectInfo.top + 15;
#else
	RectInfo.left		 = pRect->left;
	RectInfo.top		= pRect->top + Count * 40;
	RectInfo.right		= pRect->left + 320;
	RectInfo.bottom	= RectInfo.top + 40 - 1;
#endif

	
	DrawText(Hdc, pBuf, -1, &RectInfo, DT_SINGLELINE|DT_CENTER|DT_VCENTER);		
}
void 
MXDrawText_Hebrew(HDC Hdc, RECT* pRect, char *pBuf, UCHAR Count)
{
	RECT			RectInfo;
	SelectObject( Hdc, GetHZKFont32());

#ifdef NEW_OLED_ENABLE
	Hdc->textcolor = FONT_COLOR;
	RectInfo.left		 = pRect->left;
	RectInfo.top		= pRect->top + Count * 15 + 3;
	RectInfo.right		= pRect->left + SCREEN_WIDTH;
	RectInfo.bottom	= RectInfo.top + 15;
#else
	RectInfo.left		 = pRect->left;
	RectInfo.top		= pRect->top + Count * 40;
	RectInfo.right		= pRect->left + 320;
	RectInfo.bottom	= RectInfo.top + 40 - 1;
#endif

	
	MXDrawText(Hdc, pBuf, -1, &RectInfo, DT_SINGLELINE|DT_CENTER|DT_VCENTER);		
}
void 
MXDrawText_Cn(HDC Hdc, RECT* pRect, char *pBuf, UCHAR Count)
{
	RECT			RectInfo;
	SelectObject( Hdc, GetHZKFont32());

#ifdef  NEW_OLED_ENABLE
	Hdc->textcolor = FONT_COLOR;				
	RectInfo.left		 = pRect->left;
	RectInfo.top		= pRect->top + Count * 15 + 3;
	RectInfo.right		= pRect->left + SCREEN_WIDTH;
	RectInfo.bottom	= RectInfo.top + 15;
#else
	RectInfo.left		 = pRect->left;
	RectInfo.top		= pRect->top + Count * 40;
	RectInfo.right		= pRect->left + 320;
	RectInfo.bottom	= RectInfo.top + 40 - 1;
#endif
	
	DrawText(Hdc, pBuf, -1, &RectInfo, DT_SINGLELINE|DT_CENTER|DT_VCENTER);		
}

VOID 
MXDrawText_Left(HDC Hdc, CHAR *pBuf,  LONG xPos, UCHAR LineStart)
{
	RECT			RectInfo;
	SelectObject( Hdc, GetHZKFont32());
#ifdef NEW_OLED_ENABLE
//	printf("pBuf = %s\n",pBuf);
	Hdc->textcolor = FONT_COLOR;							
	RectInfo.left		 = xPos;
	RectInfo.top	   = LineStart * 15 +3;
	RectInfo.right		= SCREEN_WIDTH - 1;
	RectInfo.bottom	= RectInfo.top + 15;
#else
	RectInfo.left		 = xPos;
	RectInfo.top	   = LineStart * 40;
	RectInfo.right		= SCREEN_WIDTH - 1;
	RectInfo.bottom	= RectInfo.top + 40 - 1;
#endif
	DrawText(Hdc, pBuf, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);		
	
	
}
VOID 
MXDrawText_Right(HDC Hdc, CHAR *pBuf,  LONG xPos, UCHAR LineStart)
{
	RECT			RectInfo;
	SelectObject( Hdc, GetHZKFont32());
#ifdef NEW_OLED_ENABLE
//	printf("pBuf = %s\n",pBuf);
	Hdc->textcolor = FONT_COLOR;							
	RectInfo.left		 = 0;
	RectInfo.top	   = LineStart * 15 +3;
	RectInfo.right		= xPos-1;
	RectInfo.bottom	= RectInfo.top + 15;
#else
	RectInfo.left		 = xPos;
	RectInfo.top	   = LineStart * 40;
	RectInfo.right		= xPos-1;
	RectInfo.bottom	= RectInfo.top + 40 - 1;
#endif
		
	MXDrawText(Hdc, pBuf, -1, &RectInfo, DT_SINGLELINE|DT_RIGHT|DT_VCENTER);	
	
}
LONG 
GetXPos(CHAR *pBuf)
{
#ifdef NEW_OLED_ENABLE
	if(SCREEN_WIDTH>6 * strlen(pBuf))
	{
		return (SCREEN_WIDTH - 6 * strlen(pBuf)) / 2;
	}
	else
	{
		return 0;
	}
#else
	if(SCREEN_WIDTH>16* strlen(pBuf))
	{
		return (SCREEN_WIDTH - 16 * strlen(pBuf)) / 2;
	}
	else
	{
		return 0;
	}
	
#endif
}

VOID 
MXDrawText_Center(HDC Hdc, CHAR *pBuf,  UCHAR LineStart)
{
	RECT			RectInfo;
	SelectObject( Hdc, GetHZKFont32());
#ifdef NEW_OLED_ENABLE
	Hdc->textcolor = FONT_COLOR;				
	RectInfo.left		 = 0;
	RectInfo.top	   = LineStart * 15 + 3;
	RectInfo.right	   = SCREEN_WIDTH - 1;
	RectInfo.bottom	= RectInfo.top + 15;
#else
	RectInfo.left		 = 0;
	RectInfo.top	   = LineStart * 40;
	RectInfo.right	   = SCREEN_WIDTH - 1;
	RectInfo.bottom	= RectInfo.top + 40 - 1;
#endif
	
	
	MXDrawText(Hdc, pBuf, -1, &RectInfo, DT_SINGLELINE|DT_CENTER|DT_VCENTER);		
}

VOID 
MXDrawText_Center_24(HDC Hdc, CHAR *pBuf,  UCHAR LineStart)
{
	RECT			RectInfo;
	SelectObject( Hdc, GetHZKFont24());
#ifdef NEW_OLED_ENABLE
	Hdc->textcolor = FONT_COLOR;				
	RectInfo.left		 = 0;
	RectInfo.top	   = LineStart * 15 + 3;
	RectInfo.right	   = SCREEN_WIDTH - 1;;
	RectInfo.bottom	= RectInfo.top + 15;
#else
	RectInfo.left		 = 0;
	RectInfo.top	   = LineStart * 40;
	RectInfo.right	   = SCREEN_WIDTH - 1;;
	RectInfo.bottom	= RectInfo.top + 40 - 1;
#endif
	
	
	MXDrawText(Hdc, pBuf, -1, &RectInfo, DT_SINGLELINE|DT_CENTER|DT_VCENTER);		
}

VOID 
MXDrawText_Center_48(HDC Hdc, CHAR *pBuf,  UCHAR LineStart)
{
	RECT			RectInfo;
	SelectObject( Hdc, GetHZKFont48());
#ifdef NEW_OLED_ENABLE
	Hdc->textcolor = FONT_COLOR;				
	RectInfo.left		 = 0;
	RectInfo.top	   = LineStart * 15 + 3;
	RectInfo.right	   = SCREEN_WIDTH - 1;
	RectInfo.bottom	= RectInfo.top + 15;
#else
	RectInfo.left		 = 0;
	RectInfo.top	   = LineStart * 40;
	RectInfo.right	   = SCREEN_WIDTH - 1;
	RectInfo.bottom	= RectInfo.top + 40 - 1;
#endif
	
	
	MXDrawText(Hdc, pBuf, -1, &RectInfo, DT_SINGLELINE|DT_CENTER|DT_VCENTER);		
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MulLineDisProc
**	AUTHOR:		   Jeff Wang
**	DATE:		12 - Dec - 2008
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
void
MulLineDisProc(HDC Hdc, CHAR *pBuf, INT nBufLen, UCHAR CountStart)
{
	char pTemp[TITLE_BUF_LEN] = { 0 };
#ifdef NEW_OLED_ENABLE
	Hdc->textcolor = FONT_COLOR;				
	if (nBufLen <= 10) 
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf, nBufLen);
		MXDrawText_Center(Hdc, pTemp, CountStart);
	}
	else if ((nBufLen > 10) && (nBufLen < 20))
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf, 10);	
		MXDrawText_Center(Hdc, pTemp, CountStart);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf+10, nBufLen-10);
		MXDrawText_Center(Hdc, pTemp, CountStart+1);
	}
#else
	if (nBufLen <= 8) 
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf, nBufLen);
		MXDrawText_Center(Hdc, pTemp, CountStart);
	}
	else if ((nBufLen > 8) && (nBufLen <= 16))
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf, 8);	
		MXDrawText_Center(Hdc, pTemp, CountStart);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf+8, nBufLen-8);
		MXDrawText_Center(Hdc, pTemp, CountStart+1);
	}
	else
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf, 8);
		MXDrawText_Center(Hdc, pTemp, CountStart);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf+8, 8);
		MXDrawText_Center(Hdc, pTemp, CountStart+1);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf+16, nBufLen-16);
		MXDrawText_Center(Hdc, pTemp, CountStart+2);
	}
#endif
	
}

static void
MulLineDisProc_48(HDC Hdc, char *pBuf, int nBufLen, UCHAR CountStart)
{
	char pTemp[TITLE_BUF_LEN] = { 0 };
//printf("MulLineDisProc_48\n");

#ifdef NEW_OLED_ENABLE
	Hdc->textcolor = FONT_COLOR;				
	if (nBufLen <= 10) 
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf, nBufLen);
		MXDrawText_Center_48(Hdc, pTemp, CountStart);
	}
	else if ((nBufLen > 10) && (nBufLen < 20))
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		
		memcpy(pTemp, pBuf, 10);	
		MXDrawText_Center_48(Hdc, pTemp, CountStart);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf+10, nBufLen-10);
		MXDrawText_Center_48(Hdc, pTemp, CountStart+1);
	}
#else
	if (nBufLen <= 8) 
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		
		memcpy(pTemp, pBuf, nBufLen);
		MXDrawText_Center_48(Hdc, pTemp, CountStart);
	}
	else if ((nBufLen > 8) && (nBufLen <= 16))
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		
		memcpy(pTemp, pBuf, 8);	
		MXDrawText_Center_48(Hdc, pTemp, CountStart);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf+8, nBufLen-8);
		MXDrawText_Center_48(Hdc, pTemp, CountStart+1);
	}
	else
	{
		memset(pTemp , 0, TITLE_BUF_LEN);
		
		memcpy(pTemp, pBuf, 8);
		MXDrawText_Center_48(Hdc, pTemp, CountStart);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf+8, 8);
		MXDrawText_Center_48(Hdc, pTemp, CountStart+1);
		memset(pTemp , 0, TITLE_BUF_LEN);
		memcpy(pTemp, pBuf+16, nBufLen-16);
		MXDrawText_Center_48(Hdc, pTemp, CountStart+2);
	}
#endif

	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendKeySoundNote2MM
**	AUTHOR:		   Jeff Wang
**	DATE:		08 - Jan - 2009
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
SendKeySoundNote2MM()
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_NULL;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_PLY_KEY_A;
	msgSend.pParam		= NULL;	
	MxPutMsg(&msgSend);	
}

CHAR KeyBoardMap(WPARAM wParam)
{
	CHAR nRet = 0;

	printf("Get Key Raw Value: %d\n",wParam);

	if (DEVICE_CORAL_DIRECT_GM == g_DevFun.DevType)
	{
		switch(wParam) 
		{
		case 129:
			nRet = KEY_NUM_ON;//KEY_ROOM_CANCEL;
			break;
		case 130:
			nRet = KEY_ROOM_1;
			break;
		case 125:
			nRet = KEY_ROOM_2;
			break;
		case 126:
			nRet = KEY_ROOM_3;
			break;
		case 127:
			nRet = KEY_ROOM_4;
			break;
		case 128:
			nRet = KEY_ROOM_5;
			break;
		case 114:
			nRet = KEY_ROOM_6;
			break;
		case 113:
			nRet = KEY_ROOM_7;
			break;
		case 117:
			nRet = KEY_ROOM_8;
			break;
		case 116:
			nRet = KEY_ROOM_9;
			break;
		case 120:
			nRet = KEY_ROOM_10;
			break;
		case 119:
			nRet = KEY_ROOM_11;
			break;
		case 123:
			nRet = KEY_ROOM_12;
			break;
		case 122:
			nRet = KEY_ROOM_13;
			break;
		case 124://mini shell
			nRet = KEY_ROOM_2;
			break;
		default:
			nRet = wParam;		
			break;
		}
	}
	else
	{
		switch(wParam) 
		{
		case 112:
			nRet = KEY_NUM_1;
			break;
		case 113:
			nRet = KEY_NUM_2;
			break;
		case 114:
			nRet = KEY_NUM_3;
			break;
		case 115:
			nRet = KEY_NUM_4;
			break;
		case 116:
			nRet = KEY_NUM_5;
			break;
		case 117:
			nRet = KEY_NUM_6;
			break;
		case 118:
			nRet = KEY_NUM_7;
			break;
		case 119:
			nRet = KEY_NUM_8;
			break;
		case 120:
			nRet = KEY_NUM_9;
			break;
		case 121:
			nRet = KEY_NUM_ON;
			break;
		case 122:
			nRet = KEY_NUM_0;
			break;
		case 123:
			nRet = KEY_NUM_ENTER;
			break;
		case 124:
			nRet = KEY_DB_CALL;
			break;
		case 252:
			nRet = KEY_DB_CALL;
			break;
		default:
			nRet = wParam;		
			break;
		}	
	}

	
	if ((ST_ORIGINAL != g_TalkInfo.talking.dwTalkState && ((KEY_NUM_ON == nRet) || isDBCallKeyPressed(nRet)))
		||ST_ORIGINAL == g_TalkInfo.talking.dwTalkState
		)
	{
		SendKeySoundNote2MM();	
//		printf("-------------GetFocus = %d\n",GetFocus());

		if (g_WndMan.nChildWndNum) 
		{
			if (g_WndMan.pChildWndInfo[0].CurWndHdle != GetFocus()) 
			{
				SetFocus(g_WndMan.pChildWndInfo[0].CurWndHdle);
//				printf("XXXXXXXXXXXXxx-------------GetFocus = %d\n",GetFocus());
			}
		}
			

	}	
	return nRet;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DrawDataParaSet
**	AUTHOR:		   Jeff Wang
**	DATE:		19 - Jan - 2009
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
void
DrawDataParaSet(HDC Hdc, CHAR *pBuf, UCHAR Index)
{
	CHAR pTemp[TITLE_BUF_LEN] = { 0 };
	INT		 nStrLen									= 0;
	RECT  RectInfo;
    LONG top2;
	LONG button2;
	
	
	SelectObject( Hdc, GetHZKFont32());
	if (SET_HEBREW == g_SysConfig.LangSel) 
	{

#ifdef NEW_OLED_ENABLE	
	Hdc->textcolor = FONT_COLOR;						
	RectInfo.top		=  15 * 2;
	top2 = RectInfo.top;
	RectInfo.bottom	=  RectInfo.top  + 15;	
	button2 = RectInfo.bottom;
	
	//nStrLen = strlen(pBuf);	
	RectInfo.left=0;
	RectInfo.right=SCREEN_WIDTH;
	MXDrawText(Hdc, pBuf, -1, &RectInfo, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
	
	
	memset(pTemp, 0, TITLE_BUF_LEN);
	memcpy(pTemp, pBuf, Index);
	
	RectInfo.left =  0;
	//RectInfo.right = (SCREEN_WIDTH + HebrewGetTextWidth(Hdc,pBuf)) / 2-HebrewGetTextWidth(Hdc,pTemp);
	RectInfo.left = (SCREEN_WIDTH - HebrewGetTextWidth(Hdc,pBuf)) / 2+HebrewGetTextWidth(Hdc,pTemp);
	RectInfo.right=SCREEN_WIDTH;
	RectInfo.top =  RectInfo.bottom;
	RectInfo.bottom = RectInfo.top+15 ;
	memset(pTemp, 0, TITLE_BUF_LEN);
	memset(pTemp, '*', 1);	
	//MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_RIGHT|DT_VCENTER);
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);

	
#else	
	/*RectInfo.top		=  32 * 4;
	RectInfo.bottom	=  RectInfo.top  + 32;
	
	//nStrLen = strlen(pBuf);
	
	RectInfo.left = (SCREEN_WIDTH - HebrewGetTextWidth(Hdc,pBuf)) / 2;
	RectInfo.right = RectInfo.left + Index*16;
	Hdc->textcolor = FONT_COLOR;
	memset(pTemp, 0, TITLE_BUF_LEN);
	memcpy(pTemp, pBuf, Index+1);
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);		
	
	RectInfo.left = RectInfo.right;
	RectInfo.right = RectInfo.left + 16;
	Hdc->textcolor = FONT_COLOR_SEL;
	memset(pTemp, 0, TITLE_BUF_LEN);
	memcpy(pTemp, pBuf+Index, 1);	
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);		
	
	RectInfo.left = RectInfo.right;
	RectInfo.right = RectInfo.left + 16*(nStrLen-Index-1);
	Hdc->textcolor = FONT_COLOR;
	memset(pTemp, 0, TITLE_BUF_LEN);
	memcpy(pTemp, pBuf+Index+1, nStrLen-Index-1);		
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);
*/
#endif
	}
	else
	{
#ifdef NEW_OLED_ENABLE	
	Hdc->textcolor = FONT_COLOR;						
	RectInfo.top		=  12 * 3;
	top2 = RectInfo.top;
	RectInfo.bottom	=  RectInfo.top  + 12;	
	button2 = RectInfo.bottom;
	nStrLen = strlen(pBuf);	
	RectInfo.left = (SCREEN_WIDTH - 6 * nStrLen) / 2;
	RectInfo.right = RectInfo.left + Index*6;
	memset(pTemp, 0, TITLE_BUF_LEN);
	memcpy(pTemp, pBuf, Index+1);
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);

	RectInfo.left = RectInfo.right;
	RectInfo.right = RectInfo.left + 6;
	memset(pTemp, 0, TITLE_BUF_LEN);
	memcpy(pTemp, pBuf+Index, 1);	
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);


	RectInfo.top =  RectInfo.bottom -2;
	RectInfo.bottom = RectInfo.top + 6;
	memset(pTemp, 0, TITLE_BUF_LEN);
	memset(pTemp, '*', 1);	
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);
	
	
	RectInfo.top =  top2;
	RectInfo.bottom = button2;
	RectInfo.left = RectInfo.right;
	RectInfo.right = RectInfo.left + 6*(nStrLen-Index-1);
	memset(pTemp, 0, TITLE_BUF_LEN);
	memcpy(pTemp, pBuf+Index+1, nStrLen-Index-1);		
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);
	
	
	
	
#else	
	RectInfo.top		=  32 * 4;
	RectInfo.bottom	=  RectInfo.top  + 32;
	
	nStrLen = strlen(pBuf);
	
	RectInfo.left = (SCREEN_WIDTH - 16 * nStrLen) / 2;
	RectInfo.right = RectInfo.left + Index*16;
	Hdc->textcolor = FONT_COLOR;
	memset(pTemp, 0, TITLE_BUF_LEN);
	memcpy(pTemp, pBuf, Index+1);
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);		
	
	RectInfo.left = RectInfo.right;
	RectInfo.right = RectInfo.left + 16;
	Hdc->textcolor = FONT_COLOR_SEL;
	memset(pTemp, 0, TITLE_BUF_LEN);
	memcpy(pTemp, pBuf+Index, 1);	
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);		
	
	RectInfo.left = RectInfo.right;
	RectInfo.right = RectInfo.left + 16*(nStrLen-Index-1);
	Hdc->textcolor = FONT_COLOR;
	memset(pTemp, 0, TITLE_BUF_LEN);
	memcpy(pTemp, pBuf+Index+1, nStrLen-Index-1);		
	MXDrawText(Hdc, pTemp, -1, &RectInfo, DT_SINGLELINE|DT_LEFT|DT_VCENTER);

#endif
	}


}

VOID 
ClearKeyBuffer()
{
	memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
	g_KeyInputBufferLen = 0;
}

VOID
SaveKeyBuffer()
{
	memcpy(g_KeyInputBkp, g_KeyInputBuffer, KEY_BUF_LEN);
	g_KeyInputBkpLen = g_KeyInputBufferLen;
}

VOID
LoadKeyBuffer()
{
	memcpy(g_KeyInputBuffer, g_KeyInputBkp, KEY_BUF_LEN);
	g_KeyInputBufferLen = g_KeyInputBkpLen;
}

VOID 
ResetTimer (HWND hWnd , UINT nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
	KillTimer(hWnd, nIDEvent);
	SetTimer(hWnd, nIDEvent, uElapse, (TIMERPROC)lpTimerFunc);	
}

VOID
CircleBufEnter(CHAR* pKeyBuf, INT* pKeyBufLen, INT nMaxBufLen, CHAR KeyValue)
{
	if (*pKeyBufLen < nMaxBufLen)
	{
		pKeyBuf[(*pKeyBufLen)++] = (CHAR)KeyValue;
	}
	else
	{
		*pKeyBufLen		=	0;
		pKeyBuf[(*pKeyBufLen)++] = (CHAR)KeyValue;
	}
}


VOID
CloseChildWnd(HWND hWnd)
{
	KillChildWnd(hWnd);
	ResetTimer(hWnd, TIMER_MAIN, PRMPTMSGDISPTIME, NULL);
	g_MainWndStatus = STATUS_MSGHELP_WINDOW;
	g_WndMan.nChildWndNum = 0;
	ClearKeyBuffer();
}




VOID
DrawLine(HDC Hdc, int Fx1, int Fy1, int Fx2, int Fy2)
{
	HPEN    hOldPen = NULL;
	
//	printf("Fx1 = %d,Fy1 = %d,Fx2 = %d,Fy2 = %d\n",Fx1,Fy1,Fx2,Fy2);
	
	hOldPen = SelectObject( Hdc, CreatePen(FONT_COLOR, 1, FONT_COLOR));
	
	MoveToEx(Hdc, Fx1, Fy1, NULL);
	LineTo(Hdc, Fx2, Fy2);
	
	DeleteObject(SelectObject(Hdc, hOldPen));
}


VOID
DrawChineseCur(HDC Hdc, int xPos, int linePos)
{
#ifdef NEW_OLED_ENABLE	
	DrawLine(Hdc,xPos+1,linePos*15+8,xPos+3,linePos*15+6);
	DrawLine(Hdc,xPos+2,linePos*15+8,xPos+3,linePos*15+7);
	DrawLine(Hdc,xPos+2,linePos*15+9,xPos+4,linePos*15+7);
	DrawLine(Hdc,xPos+3,linePos*15+9,xPos+4,linePos*15+8);
	DrawLine(Hdc,xPos+3,linePos*15+10,xPos+5,linePos*15+8);
#else
	DrawLine(Hdc,xPos+1,linePos*40+20,xPos+3,linePos*40+18);
	DrawLine(Hdc,xPos+2,linePos*40+20,xPos+3,linePos*40+19);
	DrawLine(Hdc,xPos+2,linePos*40+21,xPos+4,linePos*40+19);
	DrawLine(Hdc,xPos+3,linePos*40+21,xPos+4,linePos*40+20);
	DrawLine(Hdc,xPos+3,linePos*40+22,xPos+5,linePos*40+20);
#endif	
//	DrawLine(Hdc,xPos,linePos*16+2,xPos+6,linePos*16+6);
//	DrawLine(Hdc,xPos,linePos*16+10,xPos+6,linePos*16+6);
//	DrawLine(Hdc,xPos,linePos*16+6,xPos+6,linePos*16+6);
}	

VOID
DrawEnglishCur(HDC Hdc, int xPos, int linePos)
{
#ifdef NEW_OLED_ENABLE	
	DrawLine(Hdc,xPos+1,linePos*15+8-2,xPos+3,linePos*15+6-2);
	DrawLine(Hdc,xPos+2,linePos*15+8-2,xPos+3,linePos*15+7-2);
	DrawLine(Hdc,xPos+2,linePos*15+9-2,xPos+4,linePos*15+7-2);
	DrawLine(Hdc,xPos+3,linePos*15+9-2,xPos+4,linePos*15+8-2);
	DrawLine(Hdc,xPos+3,linePos*15+10-2,xPos+5,linePos*15+8-2);
#else
	DrawLine(Hdc,xPos+1,linePos*40+20-2,xPos+3,linePos*40+18-2);
	DrawLine(Hdc,xPos+2,linePos*40+20-2,xPos+3,linePos*40+19-2);
	DrawLine(Hdc,xPos+2,linePos*40+21-2,xPos+4,linePos*40+19-2);
	DrawLine(Hdc,xPos+3,linePos*40+21-2,xPos+4,linePos*40+20-2);
	DrawLine(Hdc,xPos+3,linePos*40+22-2,xPos+5,linePos*49+20-2);
#endif	
//	DrawLine(Hdc,xPos,linePos*16,xPos+6,linePos*16+4);
//	DrawLine(Hdc,xPos,linePos*16+8,xPos+6,linePos*16+4);
//	DrawLine(Hdc,xPos,linePos*16+4,xPos+6,linePos*16+4);
}	
VOID
DrawHebrewCur(HDC Hdc, int xPos, int linePos)
{
#ifdef NEW_OLED_ENABLE	
	DrawLine(Hdc,xPos+1,linePos*15+8+3,xPos+3,linePos*15+6+3);
	DrawLine(Hdc,xPos+2,linePos*15+8+3,xPos+3,linePos*15+7+3);
	DrawLine(Hdc,xPos+2,linePos*15+9+3,xPos+4,linePos*15+7+3);
	DrawLine(Hdc,xPos+3,linePos*15+9+3,xPos+4,linePos*15+8+3);
	DrawLine(Hdc,xPos+3,linePos*15+10+3,xPos+5,linePos*15+8+3);
#else
	DrawLine(Hdc,xPos+1,linePos*40+20+2,xPos+3,linePos*40+18+2);
	DrawLine(Hdc,xPos+2,linePos*40+20+2,xPos+3,linePos*40+19+2);
	DrawLine(Hdc,xPos+2,linePos*40+21+2,xPos+4,linePos*40+19+2);
	DrawLine(Hdc,xPos+3,linePos*40+21+2,xPos+4,linePos*40+20+2);
	DrawLine(Hdc,xPos+3,linePos*40+22+2,xPos+5,linePos*49+20+2);
#endif	
//	DrawLine(Hdc,xPos,linePos*16,xPos+6,linePos*16+4);
//	DrawLine(Hdc,xPos,linePos*16+8,xPos+6,linePos*16+4);
//	DrawLine(Hdc,xPos,linePos*16+4,xPos+6,linePos*16+4);
}
VOID
DrawPageUp(HDC Hdc)
{
	if (SET_HEBREW == g_SysConfig.LangSel) 
	{
#ifdef NEW_OLED_ENABLE	
		DrawLine(Hdc,SCREEN_WIDTH-5,6,SCREEN_WIDTH-9,0);
		DrawLine(Hdc,SCREEN_WIDTH-9,0,SCREEN_WIDTH-13,6);
#else
		DrawLine(Hdc,12+20,15,22+20,0);
		DrawLine(Hdc,22+20,0,32+20,15);
#endif
	}
	else
	{
#ifdef NEW_OLED_ENABLE	
		DrawLine(Hdc,5,6,9,0);
		DrawLine(Hdc,9,0,13,6);
#else
		DrawLine(Hdc,12+20,15,22+20,0);
		DrawLine(Hdc,22+20,0,32+20,15);
#endif
	}
}

	
VOID
DrawPageDown(HDC Hdc)
{
	if (SET_HEBREW == g_SysConfig.LangSel) 
	{
#ifdef NEW_OLED_ENABLE	
		DrawLine(Hdc,SCREEN_WIDTH-5,54,SCREEN_WIDTH-9,60);
		DrawLine(Hdc,SCREEN_WIDTH-9,60,SCREEN_WIDTH-13,54);
#else
		DrawLine(Hdc,12+20,135,22+20,150);
		DrawLine(Hdc,22+20,150,32+20,135);
#endif	
	}
	else
	{
#ifdef NEW_OLED_ENABLE	
		DrawLine(Hdc,5,54,9,60);
		DrawLine(Hdc,9,60,13,54);
#else
	DrawLine(Hdc,12+20,135,22+20,150);
	DrawLine(Hdc,22+20,150,32+20,135);
#endif
	}
}


BOOL 
AddOneWnd(HWND hWnd,INT Priority)
{
	INT i = 0;
	INT j = 0;
	BOOL nRet = FALSE;
//	printf("----AddOneWnd----1 g_WndMan.nChildWndNum = %d\n",g_WndMan.nChildWndNum);
//	for(i=0; i<g_WndMan.nChildWndNum; i++)
//	{
//		printf("----AddOneWnd----1 g_WndMan.pChildWndInfo[%d].CurWndHdle = %d\n",i,g_WndMan.pChildWndInfo[i].CurWndHdle);
//	}
	if (g_WndMan.nChildWndNum) 
	{
		if (g_WndMan.nChildWndNum >= MAX_WINDOWS_NUM - 1)
		{
			return FALSE;
		}

		for(i=0; i<g_WndMan.nChildWndNum; i++)
		{
			if (Priority >= g_WndMan.pChildWndInfo[i].Priority) 
			{
				for(j=(g_WndMan.nChildWndNum - 1); j>=i;j--)
				{
					g_WndMan.pChildWndInfo[j+1].CurWndHdle = g_WndMan.pChildWndInfo[j].CurWndHdle;
					g_WndMan.pChildWndInfo[j+1].Priority = g_WndMan.pChildWndInfo[j].Priority;					
				}
				g_WndMan.pChildWndInfo[i].CurWndHdle = hWnd;
				g_WndMan.pChildWndInfo[i].Priority = Priority;
				g_WndMan.nChildWndNum ++;
				nRet = TRUE;
				break;
			}
		}
		if (!nRet) 
		{
			g_WndMan.pChildWndInfo[i].CurWndHdle = hWnd;
			g_WndMan.pChildWndInfo[i].Priority = Priority;
			g_WndMan.nChildWndNum ++;
			nRet = TRUE;			
		}

	}
	else
	{
		g_WndMan.pChildWndInfo[g_WndMan.nChildWndNum].CurWndHdle = hWnd;
		g_WndMan.pChildWndInfo[g_WndMan.nChildWndNum].Priority = Priority;
		g_WndMan.nChildWndNum = 1;
		nRet = TRUE;			
	}
//	for(i=0; i<g_WndMan.nChildWndNum; i++)
//	{
//		printf("----AddOneWnd----2 g_WndMan.pChildWndInfo[%d].CurWndHdle = %d\n",i,g_WndMan.pChildWndInfo[i].CurWndHdle);
//	}
	SetFocus(g_WndMan.pChildWndInfo[0].CurWndHdle);	
	SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);
    
	return nRet;
}

BOOL 
RemoveOneWnd(HWND hWnd)
{
	BOOL nRet = FALSE;
	INT  i = 0;
	INT  j = 0;
//	printf("----RemoveOneWnd----1 g_WndMan.nChildWndNum = %d\n",g_WndMan.nChildWndNum);
//	for(i=0; i<g_WndMan.nChildWndNum; i++)
//	{
//		printf("----RemoveOneWnd----1 g_WndMan.pChildWndInfo[%d].CurWndHdle = %d\n",i,g_WndMan.pChildWndInfo[i].CurWndHdle);
//	}
	if (g_WndMan.nChildWndNum) 
	{

		if (1 == g_WndMan.nChildWndNum) 
		{
			g_WndMan.nChildWndNum = 0;			
			nRet = TRUE;
		}
		else
		{
			for(i=0; i<g_WndMan.nChildWndNum; i++)			
			{
				if (hWnd == g_WndMan.pChildWndInfo[i].CurWndHdle) 
				{
					for(j = i; j< (g_WndMan.nChildWndNum - 1); j++)
					{
						g_WndMan.pChildWndInfo[j].CurWndHdle = g_WndMan.pChildWndInfo[j+1].CurWndHdle;
						g_WndMan.pChildWndInfo[j].Priority = g_WndMan.pChildWndInfo[j+1].Priority;											
					}
					g_WndMan.nChildWndNum --;
					nRet = TRUE;
					break;					
				}
			}

		}
	}
	else
	{
		nRet = FALSE;
	}
	if (g_WndMan.nChildWndNum) 
	{
		ShowWindow(g_WndMan.pChildWndInfo[0].CurWndHdle, SW_SHOW);
		SetFocus(g_WndMan.pChildWndInfo[0].CurWndHdle);		
	}
	else
	{
		SetFocus(g_WndMan.MainWndHdle);		
	}
//	for(i=0; i<g_WndMan.nChildWndNum; i++)
//	{
//		printf("----RemoveOneWnd----2 g_WndMan.pChildWndInfo[%d].CurWndHdle = %d\n",i,g_WndMan.pChildWndInfo[i].CurWndHdle);
//	}
//	printf("----RemoveOneWnd----2 g_WndMan.nChildWndNum = %d\n",g_WndMan.nChildWndNum);
//
//
//	printf("-------------g_hWNDTalk = %d; GetFocus = %d\n",g_hWNDTalk,GetFocus());
	SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);					
		
	return nRet;
}	

static
VOID
SendRdPassword2ACC()
{
	MXMSG		msgSend;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACC;
	msgSend.dwDestMd	= MXMDID_ACC;
	msgSend.dwMsg		= FC_AC_RSD_PWD;
	
	nDataLen = RD_CODE_LEN + 1/*Pwd len*/ + PWD_LEN + 1/*Gate Index*/;
	msgSend.pParam		= (unsigned char *) malloc(sizeof(unsigned short) + nDataLen);

	memcpy(msgSend.pParam, &nDataLen, sizeof(unsigned short));
	memcpy(msgSend.pParam + sizeof(unsigned short), "", RD_CODE_LEN);
	if (g_KeyInputBufferLen < PWD_LEN)
	{
		*(msgSend.pParam + sizeof(unsigned short) + RD_CODE_LEN) = g_KeyInputBufferLen;
		memcpy(msgSend.pParam + sizeof(unsigned short) + RD_CODE_LEN + 1, g_KeyInputBuffer, g_KeyInputBufferLen);
	}
	 
	*(msgSend.pParam + sizeof(unsigned short) + RD_CODE_LEN + 1 + PWD_LEN) = GATE_ID_DEFAULT;
	
	MxPutMsg(&msgSend);	
}

