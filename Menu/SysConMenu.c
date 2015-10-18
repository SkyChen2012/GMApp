/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SysConMenu.c
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
#include "AccessSystemWnd.h"
#include "ModuleTalk.h"
#include "SysConMenu.h"
#include "SysPwdModifyWnd.h"
#include "NetworkSetWnd.h"
#include "SysInfoWnd.h"
#include "MainMenu.h"
#include "SysOtherParaSetWnd.h"
#include "LanguageSet.h"
#include "SysOtherParaSetWnd.h"
#include "VolumeSetWnd.h"
/************** DEFINES **************************************************************/

//#define SYSPARA_DEBUG
#define SYSCONFIGDISPLAY_MAX  4
#define SYSCONFIGCOUNT  5

/************** TYPEDEFS *************************************************************/
typedef	struct tagSysConMenuMan
{
	int		 		nDataIndex;
	int		 		nDataTotalCount;
	int		 		nDispIndex;
	int		 		nDispCount;
	MENUINFOEX		*pMenuView[SYSCONFIGDISPLAY_MAX];
	MENUINFOEX		*pMenuData;
	BOOL			bTop;	//menu scroll to top
	BOOL			bBottom;//menu scroll to bottom
} SysConMenuMan;

typedef enum _KEY_CMD
{
	CMD_KEY_UP = 1,
	CMD_KEY_DOWN,
	CMD_KEY_PAGEUP,
	CMD_KEY_PAGEDOWN,
}KEY_CMD;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/
extern VOID		DrawPageUp(HDC Hdc);
extern VOID		DrawPageDown(HDC Hdc);

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/
static void		SysConfigWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK SysConfigWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		LangSetFun();
static void		NetworkSetting();
//static void RingSelection();
static void		SysPwdModify();
//static void		OtherParaSet();
static void		TamperAlarmSel();
static void		VolumeSetting();
static void		InitMenu();
static void		MenuDataFresh(BOOL bSelectFirst);
static void		UpdateMenuView(KEY_CMD cmd);
static void		KeyUpDown(BOOL bDown);
static BOOL		isValidMenuDataItem(int position);
static BOOL		isMentuTop();
static BOOL		isMenutBottom();
static void		SysConfigKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

//user id
#define MENU_SYSCON_LANGUAGE_ID			100
#define MENU_SYSCON_NETWORK_ID			101
#define MENU_SYSCON_PASSWORD_ID			102
#define MENU_SYSCON_TAMPER_ALRAM_ID		103
#define MENU_SYSCON_VOLUME_SETTINGS_ID	104


static MENUINFOEX g_SysConfigInfo[SYSCONFIGCOUNT] = 
{
	{MENU_SYSCON_LANGUAGE_ID,		MENU_STATE_DEFAULT,	HS_ID_LANGUAGE,		"Language",			"ÓïÑÔ",		LangSetFun},
	{MENU_SYSCON_NETWORK_ID,		MENU_STATE_DEFAULT,	HS_ID_NETWORK,		"NetWork",			"ÍøÂç",		NetworkSetting},
	{MENU_SYSCON_PASSWORD_ID,		MENU_STATE_DEFAULT,	HS_ID_MODIFYPSW,		"Password",			"ÐÞ¸ÄÃÜÂë",	SysPwdModify},
	{MENU_SYSCON_TAMPER_ALRAM_ID,	MENU_STATE_DEFAULT,	HS_ID_TAMPERALARM,		"Tamper Alarm",		"·À²ð±¨¾¯",	TamperAlarmSel},
	{MENU_SYSCON_VOLUME_SETTINGS_ID,MENU_STATE_DEFAULT,	0,		"Volume Settings",	"ÒôÁ¿ÉèÖÃ",	VolumeSetting},
};

static SysConMenuMan g_SysConMenuMan;


/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysConMenuWndPaint
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
SysConfigWndPaint (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	int								  i	 = 0;
	INT							  xPos = 0;
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	

#ifdef NEW_OLED_ENABLE	
	Hdc->textcolor = FONT_COLOR;			
	for(i = 0; i < g_SysConMenuMan.nDispCount; i++)
	{				
		Hdc->textcolor = FONT_COLOR;			
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  Tamper Alarm");
			memset(pDateBuf, ' ',2);							
			strcpy(&pDateBuf[2],(g_SysConMenuMan.pMenuView[i])->MenuEnglishName);
			
			if((MENU_SYSCON_TAMPER_ALRAM_ID == (g_SysConMenuMan.pMenuView[i])->iID) && g_SysConfig.EnTamperAlarm) 
			{
				strcat(pDateBuf, "*");
			}		
			
			
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_SysConMenuMan.nDispIndex == i)
			{
				DrawEnglishCur(Hdc,xPos,i);
			}
		}
		if (SET_HEBREW == g_SysConfig.LangSel) 
		{	
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_TAMPERALARM));
			memset(pDateBuf, ' ',2);							
			strcpy(&pDateBuf[2],GetHebrewStr((g_SysConMenuMan.pMenuView[i])->HebrewStrID));
			
			if((MENU_SYSCON_TAMPER_ALRAM_ID == (g_SysConMenuMan.pMenuView[i])->iID) && g_SysConfig.EnTamperAlarm) 
			{
				strcat(pDateBuf, "*");
			}		
			
			MXDrawText_Right(Hdc, pDateBuf, xPos, i);
			if (g_SysConMenuMan.nDispIndex == i)
			{
				DrawHebrewCur(Hdc,xPos,i);
			}
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  ÓïÑÔÉèÖÃ");
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],(g_SysConMenuMan.pMenuView[i])->MenuChineseName);
			if((MENU_SYSCON_TAMPER_ALRAM_ID == (g_SysConMenuMan.pMenuView[i])->iID) && g_SysConfig.EnTamperAlarm) 
			{
				strcat(pDateBuf, "*");
			}		
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_SysConMenuMan.nDispIndex == i)
			{
				DrawChineseCur(Hdc,xPos,i);
			}
		}	
	}	
#else
	for(i = 0; i < g_SysConMenuMan.nDispCount; i++)
	{				
		if (g_SysConMenuMan.nDispIndex == i)
		{
			Hdc->textcolor = FONT_COLOR_SEL;			
		}
		else
		{	
			Hdc->textcolor = FONT_COLOR;			
		}
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("Other Parameters");
			strcpy(pDateBuf,(g_SysConMenuMan.pMenuView[i])->MenuEnglishName);
			
			if((MENU_SYSCON_TAMPER_ALRAM_ID == (g_SysConMenuMan.pMenuView[i])->iID) && g_SysConfig.EnTamperAlarm) 
			{
				strcat(pDateBuf, "*");
			}	
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
		}
		if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_TAMPERALARM));
			strcpy(pDateBuf,GetHebrewStr((g_SysConMenuMan.pMenuView[i])->HebrewStrID));
			
			if((MENU_SYSCON_TAMPER_ALRAM_ID == (g_SysConMenuMan.pMenuView[i])->iID) && g_SysConfig.EnTamperAlarm) 
			{
				strcat(pDateBuf, "*");
			}	
			MXDrawText_Right(Hdc, pDateBuf, xPos, i);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("ÓïÑÔÉèÖÃ");
			strcpy(pDateBuf,(g_SysConMenuMan.pMenuView[i])->MenuChineseName);
			if((MENU_SYSCON_TAMPER_ALRAM_ID == (g_SysConMenuMan.pMenuView[i])->iID) && g_SysConfig.EnTamperAlarm) 
			{
				strcat(pDateBuf, "*");
			}	
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
		}	
	}	
#endif
	if (!isMentuTop()) 
	{
		DrawPageUp(Hdc);
	}
	if (!isMenutBottom()) 
	{
		DrawPageDown(Hdc);
	}
	ResetTimer(hWnd, TIMER_SYSCONFIG_WND, INTERFACE_SETTLE_TIME, NULL);		
	
	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysConMenuWndProc
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
SysConfigWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		InitMenu();
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_SYSCONFIG_WND, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			SysConfigWndPaint(hWnd, Msg, wParam, lParam);
		}
		break;
		
	case WM_REFRESHPARENT_WND:
		SetFocus(hWnd);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;

	
	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return 0;			
		}
		ResetTimer(hWnd, TIMER_SYSCONFIG_WND, INTERFACE_SETTLE_TIME, NULL);		
		SysConfigKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_SYSCONFIG_WND);
		KillAllChildWnd(hWnd);		
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_SYSCONFIG_WND);
		break;
		
	default:
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
**	FUNCTION NAME:	CreateSysConMenuWnd
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
CreateSysConfigWnd(HWND hwndParent)
{
	static char szAppName[] = "SysConfig";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) SysConfigWndProc;
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
		"SysConfigWnd",			// Window name	(Not NULL)
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
**	FUNCTION NAME:	LangSetFun
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
LangSetFun()
{
	CreateLanguageSetWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	NetworkSetting
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
NetworkSetting()
{
	CreateNetworkSetWnd(GetFocus());
}

static void 
VolumeSetting()
{
	CreateVolumeSetWnd(GetFocus());
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	RingSelection
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
static void 
RingSelection()
{
	UCHAR RingNum = 1;
	g_SysConfig.RingNum = RingNum;
	printf("RingSelection\n");
}
*/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysPwdModify
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
SysPwdModify()
{
	CreateSysPwdModifyWnd(GetFocus());
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysPwdModify
**	AUTHOR:		   Jeff Wang
**	DATE:		27 - Oct - 2008
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
static void 
OtherParaSet()
{
	CreateOtherParaSetWnd(GetFocus());
}
*/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysConfigKeyProcess
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
SysConfigKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	wParam =  KeyBoardMap(wParam);
	switch(wParam) 
	{
	case KEY_RETURN:
		{
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		break;
	case KEY_ENTER:
		{
			(g_SysConMenuMan.pMenuView[g_SysConMenuMan.nDispIndex])->Menufun();
		}
		break;
	case KEY_UP:
		{
			KeyUpDown(FALSE);			
		}
		break;
	case KEY_DOWN:
		{
			KeyUpDown(TRUE);
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


static void 
TamperAlarmSel()
{
	g_SysConfig.EnTamperAlarm = !g_SysConfig.EnTamperAlarm;
	SaveMenuPara2Mem();	
}


static BOOL isValidMenuDataItem(int position)
{
	if(position >= SYSCONFIGCOUNT)
		return FALSE;
	if((g_SysConMenuMan.pMenuData[position].iState & MENU_STATE_VISIBLE) == 0 )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*
static INT getPreMenuItemIndex()
{
	int i = 0;
	if(g_SysConMenuMan.nDataIndex == 0)
		return 0;
	for(i = 0;i < g_SysConMenuMan.nDataIndex;i++)
	{
		if(isValidMenuDataItem(i))
			return i;
	}
	return -1;
}

static INT getNextMenuItemIndex()
{
	int i = 0;
	if((g_SysConMenuMan.nDataIndex+g_SysConMenuMan.nDispCount) >= g_SysConMenuMan.nDataTotalCount)
		return g_SysConMenuMan.nDataIndex;
	for(i = g_SysConMenuMan.nDataIndex+g_SysConMenuMan.nDispCount;i < g_SysConMenuMan.nDataTotalCount;i++)
	{
		if(isValidMenuDataItem(i))
			return i;
	}
	return -1;
}
*/

static BOOL isMentuTop()
{
	int i = 0;
	if(g_SysConMenuMan.nDataIndex == 0)
		return TRUE;
	for(i = 0;i < g_SysConMenuMan.nDataIndex;i++)
	{
		if(isValidMenuDataItem(i))
			return FALSE;
	}
	return TRUE;
}



static BOOL isMenutBottom()
{
	int i = 0;
	if((g_SysConMenuMan.nDataIndex+g_SysConMenuMan.nDispCount) >= g_SysConMenuMan.nDataTotalCount)
		return TRUE;
	for(i = g_SysConMenuMan.nDataIndex+g_SysConMenuMan.nDispCount;i < g_SysConMenuMan.nDataTotalCount;i++)
	{
		if(isValidMenuDataItem(i))
			return FALSE;
	}
	return TRUE;
}


static void InitMenu()
{
	int i = 0;
	g_SysConMenuMan.pMenuData = g_SysConfigInfo;
	for(i = 0 ;i < SYSCONFIGCOUNT;i++)
	{
		if(g_SysConMenuMan.pMenuData[i].iID == MENU_SYSCON_VOLUME_SETTINGS_ID)
		{
			if(VolumeSetIsEnable())
			{
				g_SysConMenuMan.pMenuData[i].iState |= MENU_STATE_VISIBLE;
			}
			else
			{
				g_SysConMenuMan.pMenuData[i].iState &= (~MENU_STATE_VISIBLE);
			}
#ifdef SYSPARA_DEBUG
			g_SysConMenuMan.pMenuData[0].iState &= (~MENU_STATE_VISIBLE);
			g_SysConMenuMan.pMenuData[3].iState &= (~MENU_STATE_VISIBLE);

#endif
		}
	}
	//set menu top or bottom must be after init menu state(enable or disable)
	for(i = 0 ;i < SYSCONFIGCOUNT;i++)
	{
		if(isValidMenuDataItem(i))
			break;
	}
	if(i >= SYSCONFIGCOUNT)
	{
		printf("all system menu is invalid\n");
	}
	g_SysConMenuMan.nDispCount = 0;
	g_SysConMenuMan.nDispIndex = 0;
	g_SysConMenuMan.nDataIndex = i;
	g_SysConMenuMan.nDataTotalCount = SYSCONFIGCOUNT;	
	g_SysConMenuMan.bTop = isMentuTop();
	g_SysConMenuMan.bBottom = isMenutBottom();
	MenuDataFresh(TRUE);
}

static void MenuDataFresh(BOOL bSelectFirst)
{
	int count = 0;
	int i = 0;
	for(i = 0 ;i < SYSCONFIGCOUNT;i++)
	{
		if(((g_SysConMenuMan.nDataIndex+count)>=g_SysConMenuMan.nDataTotalCount) || (count >= SYSCONFIGDISPLAY_MAX))
			break;
		if(!isValidMenuDataItem(g_SysConMenuMan.nDataIndex+i))
			continue;
		g_SysConMenuMan.pMenuView[count]=&g_SysConMenuMan.pMenuData[g_SysConMenuMan.nDataIndex+i];
		count++;
	}
	g_SysConMenuMan.nDispCount = count;
	if(g_SysConMenuMan.nDispCount == 0)
	{
		printf("system menu item is zero!!!\n");
	}
	if(bSelectFirst == TRUE)
	{
		g_SysConMenuMan.nDispIndex = 0;
	}
	else
	{
		g_SysConMenuMan.nDispIndex = (g_SysConMenuMan.nDispCount-1);
	}
	
}


static void UpdateMenuView(KEY_CMD cmd)
{
	if(cmd == CMD_KEY_UP)
	{
		if(!isMentuTop())
		{
			g_SysConMenuMan.nDataIndex--;
			MenuDataFresh(TRUE);
		}
	}
	else if(cmd == CMD_KEY_DOWN)
	{
		if(!isMenutBottom())
		{
			g_SysConMenuMan.nDataIndex++;
			MenuDataFresh(FALSE);
		}
	}
	
	
}



static void	KeyUpDown(BOOL bDown)
{
	if(bDown)
	{
		if(g_SysConMenuMan.nDispIndex >= (g_SysConMenuMan.nDispCount-1))
		{
			UpdateMenuView(CMD_KEY_DOWN);
		}
		else
		{
			g_SysConMenuMan.nDispIndex++;
		}
	}
	else
	{
		if(g_SysConMenuMan.nDispIndex <= 0)
		{
			UpdateMenuView(CMD_KEY_UP);
		}
		else
		{
			g_SysConMenuMan.nDispIndex--;
		}
	}
}


