/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MainMenu.c
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
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <MoxFB.h>
#include <device.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MenuCommon.h"
#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"
#include "ModuleTalk.h"
#include "SysConMenu.h"
#include "AccessSystemWnd.h"
#include "SysRestoreWnd.h"
#include "SysInfoWnd.h"
#include "TalkLogReport.h"
#include "rtc.h"

/************** DEFINES **************************************************************/

#define MAINMENUDEPTH_AS		6//4
#define MAINMENUDISPCOUNT_AS	5

#define MAINMENUDEPTH_NOAS		5//3
#define MAINMENUDISPCOUNT_NOAS	5

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/


static VOID		MainMenuTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		MainMenuWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK MainMenuWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static void		SysConfigFun();
static void		TalkingSet();
static void		DoorSet();
//static void		FunConfig();
static void		SysInfo();
static void		AccessSystem();
static void		SysRestart();
static void		SysRestore();

static void MainMenuKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

//static void ConvertTime2String(char *pTimeShow);

static MENUINFO g_MainMenuInfo_As[MAINMENUDEPTH_AS] = 
{
	{TRUE,  HS_ID_SYSSETTINGS,"System Settings",	"系统配置",			SysConfigFun},
	{FALSE, HS_ID_TALKSETTINGS,"Talk Settings",	"通话设置",			TalkingSet},
	{FALSE, HS_ID_ENTRYSETTINGS,"Entry Mode",		"开门设置",			DoorSet},
	{FALSE, HS_ID_ACCESSSETTINGS,"Access Settings",	"门禁设置",			AccessSystem},
	{FALSE, HS_ID_SYSINFO,"System Info.",		"系统信息",			SysInfo},
	{FALSE, HS_ID_RESETSETTINGS,"Reset Settings", 	"出厂设置",			SysRestore}
/*
	{TRUE,  "Configuration",		"系统配置",			SysConfigFun},
	{FALSE, "Access Setting",	"门禁设置",			AccessSystem},
	{FALSE, "Information",			"系统信息",			SysInfo},
	{FALSE, "Restore", 		"系统还原",			SysRestore}
*/
};

static MENUINFO g_MainMenuInfo_NoAs[MAINMENUDEPTH_NOAS] = 
{
	{TRUE, HS_ID_SYSSETTINGS, "System Settings",	"系统配置",			SysConfigFun},
	{FALSE, HS_ID_TALKSETTINGS,"Talk Settings",	"通话设置",			TalkingSet},
	{FALSE, HS_ID_ENTRYSETTINGS,"Entry Mode",		"开门设置",			DoorSet},
	{FALSE, HS_ID_SYSINFO,"System Info.",		"系统信息",			SysInfo},
	{FALSE, HS_ID_RESETSETTINGS,"Reset Settings", 	"出厂设置",			SysRestore}
	/*
	{TRUE,  "Configuration",		"系统配置",			SysConfigFun},
	{FALSE, "Information",			"系统信息",			SysInfo},
	{FALSE, "Restore", 		"系统还原",			SysRestore}
*/
};

static	MENUINFO *g_MainMenuInfo = NULL;
static	int		 nMainMenuDepth = 0;
static  int		 nDispCount		= 0;

static MENUENTERSTEP MainMenuStep = MENU_INPUT_PASSWORD;

static BOOL  g_bFirstDrawMenu = FALSE;
static	int	 nLastDisMuneNum = 0;
static  int  nShowHead = 0;


/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MainMenuTimerProc
**	AUTHOR:			Jeff Wang
**	DATE:			21 - Aug - 2008
**
**	DESCRIPTION:	
**			Process Main Menu Process Timer
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/

static VOID
MainMenuTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (MENU_PASSWORD_ERROR == MainMenuStep) 
	{
		MainMenuStep = MENU_INPUT_PASSWORD;
		ResetTimer(hWnd, TIMER_MAIN_MENU, INTERFACE_SETTLE_TIME, NULL);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
	}
	else
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
	g_KeyInputBufferLen		=	0;			
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MainMenuWndPaint
**	AUTHOR:			Jeff Wang
**	DATE:			21 - Aug - 2008
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
MainMenuWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	int								  i	 = 0;
	char		TimeShow[255] = { 0 };
	char		DisBuffer[19] = { 0 };
	INT			xPos = 0;
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	

	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;
	
	printf("MainMenuWndPaint\n");
	if (MENU_INPUT_PASSWORD == MainMenuStep)
	{
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos(STR_SYSPWD_EN);
			MXDrawText_Left(Hdc, STR_ETR_EN, xPos, 0);
			MXDrawText_Left(Hdc, STR_SYSPWD_EN, xPos, 1);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos(STR_ETRSYSPWD_CN);
			MXDrawText_Left(Hdc, STR_ETRSYSPWD_CN, xPos, 0);
			MXDrawText_Left(Hdc, STR_PRESSENTER_CN, xPos, 1);
		}
		else if(SET_HEBREW == g_SysConfig.LangSel)
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ETRSYSPWD));
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_ETRSYSPWD), xPos, 1);	
		}
		memset(DisBuffer, 42, 19);
		MulLineDisProc(Hdc, DisBuffer, g_KeyInputBufferLen, 2);
	}
	else if (MENU_PASSWORD_ERROR == MainMenuStep) 
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
		else if(SET_HEBREW == g_SysConfig.LangSel)
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_PWDINCORRECT));
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PWDINCORRECT), xPos, 1);	
		}
	}
	else if (MENU_ENTER_MENU == MainMenuStep) 
	{
#if 1		
		for(i = 1; i < nDispCount; i++)
		{
#ifndef NEW_OLED_ENABLE 
            /*Using highlight mechnism to display selected item.*/
            if (g_MainMenuInfo[i-1+ nShowHead].bSelected)
            {			
                Hdc->textcolor = FONT_COLOR_SEL;
            }
            else
            {			
                Hdc->textcolor = FONT_COLOR;
            }
#endif 
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos("  Access Setting");
				memset(pDateBuf, ' ',2);				
				strcpy(&pDateBuf[2],g_MainMenuInfo[i-1+ nShowHead].MenuEnglishName);
				MXDrawText_Left(Hdc, pDateBuf, xPos, i-1);	
#ifdef NEW_OLED_ENABLE 
                if (g_MainMenuInfo[i-1+ nShowHead].bSelected)
				{
					DrawEnglishCur(Hdc,xPos,i-1);
				}
#endif 
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_SYSSETTINGS));
				memset(pDateBuf, ' ',2);	
				strcpy(&pDateBuf[2],GetHebrewStr(g_MainMenuInfo[i-1+ nShowHead].HebrewStrID));					
				MXDrawText_Right(Hdc, pDateBuf, xPos, i -1);
#ifdef NEW_OLED_ENABLE
                if (g_MainMenuInfo[i-1+ nShowHead].bSelected)
				{
					DrawHebrewCur(Hdc,xPos,i-1);
				}
#endif
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{	

				xPos = GetXPos("  系统配置");

				memset(pDateBuf, ' ',2);

				strcpy(&pDateBuf[2],g_MainMenuInfo[i-1+ nShowHead].MenuChineseName);
				MXDrawText_Left(Hdc, pDateBuf, xPos, i-1);	
#ifdef NEW_OLED_ENABLE
                if (g_MainMenuInfo[i-1+ nShowHead].bSelected)
				{
					DrawChineseCur(Hdc,xPos,i-1);
				}
#endif
			}
		}
		if (nShowHead > 0) 
		{
			DrawPageUp(Hdc);
		}
		if (nShowHead < nMainMenuDepth - 4) 
		{
			DrawPageDown(Hdc);
		}
		
#else
		ConvertTime2String(TimeShow);
		MXDrawText_Center(Hdc, TimeShow, 0);
		for(i = 1; i < nDispCount; i++)
		{
			if (g_MainMenuInfo[i-1].bSelected)
			{
				Hdc->textcolor = FONT_COLOR_SEL;				
			}
			else
			{
				Hdc->textcolor = FONT_COLOR;
			}
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos("Access Setting");
				MXDrawText_Left(Hdc, g_MainMenuInfo[i-1].MenuEnglishName, xPos, i);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{	
				xPos = GetXPos("系统配置");
				MXDrawText_Left(Hdc, g_MainMenuInfo[i-1].MenuChineseName, xPos, i);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_SYSSETTINGS));
				MXDrawText_Right(Hdc, GetHebrewStr(g_MainMenuInfo[i-1].HebrewStrID), xPos, i);
			}
		}

#endif
		ResetTimer(hWnd, TIMER_MAIN_MENU, INTERFACE_SETTLE_TIME, NULL);			
		
	}
	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MainMenuWndProc
**	AUTHOR:			Jeff Wang
**	DATE:			21 - Aug - 2008
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static LRESULT CALLBACK 
MainMenuWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		g_bFirstDrawMenu = TRUE;
		nShowHead = 0;
		printf("######WM_CREATE\n");
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);	

		ClearKeyBuffer();
		if ((g_DevFun.bCardControlUsed)  && (DEVICE_CORAL_DB != g_DevFun.DevType) )
		{
			nMainMenuDepth = MAINMENUDEPTH_AS;
			g_MainMenuInfo = (MENUINFO*)malloc(nMainMenuDepth * sizeof(MENUINFO));
			memcpy(g_MainMenuInfo, g_MainMenuInfo_As, nMainMenuDepth * sizeof(MENUINFO));
			nDispCount	=	MAINMENUDISPCOUNT_AS;
		}
		else
		{
			nMainMenuDepth = MAINMENUDEPTH_NOAS;
			g_MainMenuInfo = (MENUINFO*)malloc(nMainMenuDepth * sizeof(MENUINFO));
			memcpy(g_MainMenuInfo, g_MainMenuInfo_NoAs, nMainMenuDepth * sizeof(MENUINFO));
			nDispCount	=	MAINMENUDISPCOUNT_NOAS;
		}
		break;
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_MAIN_MENU, wParam, NULL);

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			MainMenuWndPaint(hWnd, Msg, wParam, lParam);
		}
		break;
		
	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		if (MENU_INPUT_PASSWORD == MainMenuStep
			|| MENU_ENTER_MENU == MainMenuStep
			) 
		{
			ResetTimer(hWnd, TIMER_MAIN_MENU, INTERFACE_SETTLE_TIME, NULL);			
			MainMenuKeyProcess(hWnd, Msg, wParam, lParam);
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
		
	case WM_TIMER:
		MainMenuTimerProc(hWnd, Msg, wParam, lParam);
		break;

	case WM_REFRESHPARENT_WND:
		if (MENU_INPUT_PASSWORD == MainMenuStep
			|| MENU_ENTER_MENU == MainMenuStep) 
		{
			ResetTimer(hWnd, TIMER_MAIN_MENU, INTERFACE_SETTLE_TIME, NULL);
		}
		else
		{
			ResetTimer(hWnd, TIMER_MAIN_MENU, PROMPT_SHOW_TIME, NULL);
		}
		SetFocus(hWnd);
		SendMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_MAIN_MENU);
		MainMenuStep = MENU_INPUT_PASSWORD;
		free(g_MainMenuInfo);
		g_MainMenuInfo = NULL;
//		PostMessage(GetParent(hWnd),  WM_REFRESHPARENT_WND, 0, 0);
//		SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		break;
		
	default:
//		if (WM_CLOSE == Msg) 
//		{
//			RemoveOneWnd(hWnd);
//		}
//		return DefWindowProc(hWnd, Msg, wParam, lParam);

		DefWindowProc(hWnd, Msg, wParam, lParam);
		
		if (WM_CLOSE == Msg) 
		{
			RemoveOneWnd(hWnd);
		}		
	}
	return 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CreateMainMenuWnd
**	AUTHOR:			Jeff Wang
**	DATE:			21 - Aug - 2008
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
CreateMainMenuWnd(HWND hwndParent)
{
	static char szAppName[] = "MainMenuWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;

	nShowHead = 0;	
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) MainMenuWndProc;
	WndClass.cbClsExtra		= 0;
	WndClass.cbWndExtra		= 0;
	WndClass.hInstance		= 0;
	WndClass.hIcon			= 0;
	WndClass.hCursor		= 0;
	WndClass.hbrBackground	= (HBRUSH)GetStockObject(BACKGROUND_COLOR);
	WndClass.lpszMenuName	= NULL;
	WndClass.lpszClassName	= szAppName;
	
	RegisterClass(&WndClass);
	
	hWnd = CreateWindowEx(
		0L,					// Extend style	(0)
		szAppName,				// Class name	(NULL)
		"MainMenuWnd",			// Window name	(Not NULL)
		WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,	// Style		(0)
		0,					// x			(0)
		0,					// y			(0)
		SCREEN_WIDTH,		// Width		
		SCREEN_HEIGHT,		// Height
		g_hMainWnd /*hwndParent*/,		// Parent		(MwGetFocus())
		NULL,				// Menu			(NULL)
		NULL,				// Instance		(NULL)
		NULL);				// Parameter	(NULL)
	
	AddOneWnd(hWnd,WND_MEUN_PRIORITY_1);
	if (hWnd == GetFocus())
	{
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
	}
	else
	{
		ShowWindow(hWnd, SW_HIDE);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysConfigFun
**	AUTHOR:		   Jeff Wang
**	DATE:		28 - May - 2008
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
SysConfigFun()
{
	CreateSysConfigWnd(GetFocus());
}


static void 
TalkingSet()
{
	CreateOtherParaSetWnd(GetFocus());
}

static void 
DoorSet()
{
	CreateGateOpenModeSetWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	FunConfig
**	AUTHOR:		   Jeff Wang
**	DATE:		9 - July - 2008
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
/*
static void FunConfig()
{
	g_FunConfig.EnAccessSystem	= TRUE;
	g_FunConfig.EnAlarming		= TRUE;
	g_FunConfig.EnLogReporting	= TRUE;
	g_FunConfig.EnMonitoring	= TRUE;
	g_FunConfig.EnTalking		= TRUE;
	g_FunConfig.EnUnlocking		= TRUE;
}
*/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysInfo
**	AUTHOR:		   Jeff Wang
**	DATE:		9 - July - 2008
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
SysInfo()
{
	CreateSysInfoWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AccessSystem
**	AUTHOR:		   Jeff Wang
**	DATE:		9 - July - 2008
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
AccessSystem()
{
//	CreateAccessSystemWnd(GetFocus());
	CreateResiMngWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysRestart
**	AUTHOR:		   Jeff Wang
**	DATE:		29 - Sep - 2008
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
SysRestart()
{
	CreateSysRestartWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysRestore
**	AUTHOR:		   Jeff Wang
**	DATE:		9 - July - 2008
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

static void SysRestore()
{
	CreateSysRestoreWnd(GetFocus());
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	MainMenuKeyProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		7 - July - 2008
**
**	DESCRIPTION:	
**			Process UP,DOWN,RETURM and ENTER	
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/

static void 
MainMenuKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int i = 0;

	wParam =  KeyBoardMap(wParam);
	if (MENU_INPUT_PASSWORD == MainMenuStep)
	{
		if (wParam == KEY_RETURN)
		{
			memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
			g_KeyInputBufferLen = 0;
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9)) 
		{	
			if (g_KeyInputBufferLen < KEY_BUF_LEN)
			{
				g_KeyInputBuffer[g_KeyInputBufferLen++] = (UCHAR)wParam;
			}
			else
			{
				g_KeyInputBufferLen	=0;
				g_KeyInputBuffer[g_KeyInputBufferLen++] = (UCHAR)wParam;
			}
		}
		else if (wParam == KEY_ENTER)
		{	
			if (g_KeyInputBufferLen < MIN_PWD_LEN || g_KeyInputBufferLen > MAX_PWD_LEN) 
			{
				g_KeyInputBufferLen = 0;
				ResetTimer(hWnd, TIMER_MAIN_MENU, PROMPT_SHOW_TIME, NULL);		
				MainMenuStep = MENU_PASSWORD_ERROR;
				StartPlayErrorANote();
			}
			else
			{
				if (CodeCompare(g_KeyInputBuffer, g_SysConfig.SysPwd, g_KeyInputBufferLen)) 
				{
					MainMenuStep = MENU_ENTER_MENU;
					memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
					g_KeyInputBufferLen = 0;
				}
				else
				{
					g_KeyInputBufferLen = 0;
					ResetTimer(hWnd, TIMER_MAIN_MENU, PROMPT_SHOW_TIME, NULL);		
					MainMenuStep = MENU_PASSWORD_ERROR;
					StartPlayErrorANote();
					PostMessage(hWnd,  WM_PAINT, 0, 0);
				}	
			}
				
		}
	}
	else if (MENU_ENTER_MENU == MainMenuStep) 
	{
		switch(wParam) 
		{
		case KEY_RETURN:
			{
				PostMessage(hWnd,  WM_CLOSE, 0, 0);
			}
			break;
		case KEY_ENTER:
			{
				for (i = 0; i < nMainMenuDepth; i++)
				{
					if (g_MainMenuInfo[i].bSelected) 
					{
						if(g_MainMenuInfo[i].Menufun)
						{
							g_MainMenuInfo[i].Menufun();					
						}
						break;
					}
				}
			}
			break;
		case KEY_UP:
			{
				for (i = 0; i < nMainMenuDepth; i++)
				{
					if ( g_MainMenuInfo[i].bSelected ) 
					{
						if (0 == i) 
						{
						} 
						else
						{
							g_MainMenuInfo[i].bSelected = FALSE;
							g_MainMenuInfo[i-1].bSelected = TRUE;
						}
						if((i <= nShowHead) && (nShowHead > 0))
						{
							nShowHead --;
						}						
						break;
					}
				}
			}
			
			break;
		case KEY_DOWN:
			{
				for (i = 0; i < nMainMenuDepth; i++)
				{
					if ( g_MainMenuInfo[i].bSelected ) 
					{
						if (nMainMenuDepth-1 == i) 
						{
						} 
						else
						{
							g_MainMenuInfo[i].bSelected = FALSE;
							g_MainMenuInfo[i+1].bSelected = TRUE;
						}
						if((nShowHead < (nMainMenuDepth - 4)) && (i >= nShowHead+3))
						{
							nShowHead ++;
						}
						break;
					}
				}
			}
			break;
		case KEY_LEFT:
			break;
		case KEY_RIGHT:
			break;		
		default:
			break;
		}
	}
}

#if 0 //not used for now
static void 
ConvertTime2String(char *pTimeShow)
{
	time_t	TIMEVAL=	0;
	UCHAR OutTimeData[6] = { 0 };
	UCHAR OutTimeStr[12] = { 0 };
	int   i              =   0;
	char *pMonth[12] = {"Jan.", "Feb.", "Mar.", "Apr.", "May.", "Jun.", "July.", "Aug.", "Sep.", "Oct.", "Nov.", "Dec."};
	char  *pCurTimeC = "2008年10月01日00:00";
	char  *pCurTimeE = "Oct.01,2008 00:00";
	char  CurTime[255] = { 0 };	

	if (!IsMCOnline()) 
	{
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			strcpy(pTimeShow, pCurTimeE);			
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			strcpy(pTimeShow, pCurTimeE);			
		}
		else if (SET_CHINESE == g_SysConfig.LangSel)
		{
			strcpy(pTimeShow, pCurTimeC);
		}		
		return;
	}
	
	TIMEVAL	=	GetRtcTime();
	ConvertFromTime_t(OutTimeData, &TIMEVAL);
	for(i = 0; i < 6; i++)
	{
		OutTimeStr[2*i]		= OutTimeData[i]/10 + 0x30;
		OutTimeStr[2*i+1]	= OutTimeData[i]%10 + 0x30;		
	}

	if (SET_ENGLISH == g_SysConfig.LangSel) 
	{
		strcpy(CurTime, pCurTimeE);
		memcpy(CurTime + 0,		pMonth[OutTimeData[1]-1], 4);
		memcpy(CurTime + 4,		OutTimeStr + 4, 2);
		memcpy(CurTime + 9,		OutTimeStr + 0, 2);
		memcpy(CurTime + 12, 	OutTimeStr + 6, 2);
		memcpy(CurTime + 15, 	OutTimeStr + 8, 2);
	}
	else if (SET_HEBREW == g_SysConfig.LangSel) 
	{
		strcpy(CurTime, pCurTimeE);
		memcpy(CurTime + 0,		pMonth[OutTimeData[1]-1], 4);
		memcpy(CurTime + 4,		OutTimeStr + 4, 2);
		memcpy(CurTime + 9,		OutTimeStr + 0, 2);
		memcpy(CurTime + 12, 	OutTimeStr + 6, 2);
		memcpy(CurTime + 15, 	OutTimeStr + 8, 2);
	}
	else if (SET_CHINESE == g_SysConfig.LangSel) 
	{
		strcpy(CurTime, pCurTimeC);
		memcpy(CurTime	+ 2,	OutTimeStr + 0, 2);
		memcpy(CurTime	+ 6,	OutTimeStr + 2, 2);
		memcpy(CurTime	+ 10,	OutTimeStr + 4, 2);
		memcpy(CurTime	+ 14,	OutTimeStr + 6, 2);	
		memcpy(CurTime	+ 17,	OutTimeStr + 8, 2);			
	}	
	memcpy(pTimeShow, CurTime, strlen(CurTime));
}
#endif 

void 
KillAllChildWnd(HWND hWnd)
{
	int i = 0;
	if (GetFocus() == hWnd) 
	{
//		while (g_hMainWnd != GetParent(hWnd)) 
//		{
//			PostMessage(hWnd,  WM_CLOSE, 0, 0);
//			hWnd = GetParent(hWnd);		
//		}
		for(i=0; i<g_WndMan.nChildWndNum; i++)
		{
			PostMessage(g_WndMan.pChildWndInfo[i].CurWndHdle,  WM_CLOSE, 0, 0);
		}
		
		PostMessage(g_hMainWnd,  WM_REFRESHPARENT_WND, 0, 0);
		g_WndMan.nChildWndNum = 0;
	}
	else
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	
	
}
