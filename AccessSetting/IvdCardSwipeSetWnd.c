/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	IvdCardSwipeSetWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		28 - April - 2009
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

/************** USER INCLUDE FILES ***************************************************/

#include "MenuCommon.h"
#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"

#include "IvdCardSwipeSetWnd.h"
#include "GataParaSetWnd.h"

/******************* DEFINES **************************************************************/

#define MAX_IVDSWIPE_TIME 100
#define MIN_IVDSWIPE_TIME 1

#define IVDCARDSWIPESETDEPTH	2
#define IVDCARDSWIPETIMELENTH	3

#define STR_INVALIDSWIPE_EN		"Invalid Swipe"
#define STR_TIMMODIFIED_EN			"Time Modified"
#define STR_INVALIDSWIPETIMEMODIED_CN		"无效刷卡次数已修改"

#define STR_INVALIDSWIPE_EN		"Invalid Swipe"
#define STR_ALARMISENABLED_EN			"Alarm Is Enabled"
#define STR_INVALIDSWIPEALARMISENABLED_CN		"无效刷卡报警已启动"
#define STR_ALARMISDISABLED_EN			"Alarm Is Disabled"
#define STR_INVALIDSWIPEALARMISDISABLED_CN		"无效刷卡报警已关闭"


#define MENU_ALARMON_EN		"Alarm ON"
#define MENU_ALARMON_CN		"报警开启"
#define MENU_ALARMOFF_EN		"Alarm OFF"
#define MENU_ALARMOFF_CN		"报警关闭"


/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		IvdCardSwipeSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK IvdCardSwipeSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID		IvdCardSwipeSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID		IvdCardSwipeTimeSet();
static VOID		IvdCardSwipeAlarmSet();

static	UCHAR DataSetIdx				=	0;
static	CHAR  TempIvdCardSwipeTimeset		=	0;

static TIMEALARMSTEP IvdCardSwipeTimeAlarmStep = STEP_MAIN_WND;

static MENUINFO g_IvdCardSwipeAlarmSetInfo[IVDCARDSWIPESETDEPTH] = 
{
	{TRUE,	 0,"",	"",				IvdCardSwipeTimeSet},
	{FALSE,  HS_ID_ALARMON,MENU_ALARMON_EN,	MENU_ALARMON_CN,			IvdCardSwipeAlarmSet}
};
static CHAR HebrewIvdCardSwipTimeName[MENU_LEN]={0};
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AddCardTimerProc
**	AUTHOR:			Jeff Wang
**	DATE:			24 - Aug - 2008
**
**	DESCRIPTION:	
**			Process SysCon Menu Process Timer
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/

static VOID	 
IvdCardSwipeSetTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (STEP_TIME_SET_PROMPT == IvdCardSwipeTimeAlarmStep)
	{
		IvdCardSwipeTimeAlarmStep = STEP_MAIN_WND;
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		ResetTimer(hWnd, TIMER_IVDCARDSWIPE_SET, INTERFACE_SETTLE_TIME, NULL);
	}
	else
	{
		if (STEP_ALARM_SET_PROMPT == IvdCardSwipeTimeAlarmStep)
		{
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		else
		{
			KillAllChildWnd(hWnd);				
		}
		
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IvdCardSwipeSetWndPaint
**	AUTHOR:			Jeff Wang
**	DATE:			28 - April - 2009
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
static VOID
IvdCardSwipeSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;
	INT		i		=	0;
	INT		xPos	=	0;
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);		
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;
	
	switch(IvdCardSwipeTimeAlarmStep) 
	{
	case STEP_MAIN_WND:
		{
#ifdef NEW_OLED_ENABLE	
			for(i = 0; i < IVDCARDSWIPESETDEPTH; i++)
			{	
/*
				if (g_IvdCardSwipeAlarmSetInfo[i].bSelected)
				{
					memset(pDateBuf, '*',1);
				}
				else
				{
					memset(pDateBuf, ' ',1);
				}
*/
				memset(pDateBuf, ' ',2);			
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{	
					strcpy(&pDateBuf[2],g_IvdCardSwipeAlarmSetInfo[i].MenuEnglishName);								
					MXDrawText_Center(Hdc,pDateBuf , i);
					if (g_IvdCardSwipeAlarmSetInfo[i].bSelected)
					{
						DrawEnglishCur(Hdc,20,i);
					}
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
				
					if(0==i)
					{
						memset(pDateBuf, ' ',2);			
						strcpy(&pDateBuf[2],HebrewIvdCardSwipTimeName);	
						MXDrawText_Center(Hdc, pDateBuf, i);
					}
					else
					{
						memset(pDateBuf, ' ',2);			
						strcpy(&pDateBuf[2],GetHebrewStr(g_IvdCardSwipeAlarmSetInfo[i].HebrewStrID));								
						MXDrawText_Center(Hdc, pDateBuf, i);
						if (g_IvdCardSwipeAlarmSetInfo[i].bSelected)
						{
							DrawHebrewCur(Hdc,108,i);
						}
					}
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{	
					strcpy(&pDateBuf[2],g_IvdCardSwipeAlarmSetInfo[i].MenuChineseName);								
					MXDrawText_Center(Hdc, pDateBuf, i);
					if (g_IvdCardSwipeAlarmSetInfo[i].bSelected)
					{
						DrawChineseCur(Hdc,20,i);
					}
				}
			}	
#else
			for(i = 0; i < IVDCARDSWIPESETDEPTH; i++)
			{	
				if (g_IvdCardSwipeAlarmSetInfo[i].bSelected)
				{
					Hdc->textcolor = FONT_COLOR_SEL;			
				}
				else
				{
					Hdc->textcolor = FONT_COLOR;				
				}
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{	
					MXDrawText_Center(Hdc, g_IvdCardSwipeAlarmSetInfo[i].MenuEnglishName, i);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{	
					MXDrawText_Center(Hdc, g_IvdCardSwipeAlarmSetInfo[i].MenuChineseName, i);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{	
					if(0==i)
					{
						MXDrawText_Center(Hdc, HebrewIvdCardSwipTimeName, i);
					}
					else
					{
						MXDrawText_Center(Hdc, GetHebrewStr(g_IvdCardSwipeAlarmSetInfo[i].HebrewStrID), i);
					}	
				}
			}	
#endif
		}		
		break;
			
	case STEP_TIME_SET:
		{
			ClearKeyBuffer();
			g_KeyInputBuffer[0] = (CHAR)(TempIvdCardSwipeTimeset / 100 + 0x30);
			g_KeyInputBuffer[1] = (CHAR)((TempIvdCardSwipeTimeset % 100)/10 + 0x30);
			g_KeyInputBuffer[2] = (CHAR)((TempIvdCardSwipeTimeset %100) % 10 + 0x30);
			
			DrawDataParaSet(Hdc, (CHAR*)g_KeyInputBuffer, DataSetIdx);
		}
		break;

	case STEP_TIME_SET_PROMPT:
		{
			if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_INVALIDSWIPETIMEMODIED_CN, 1);
			}
			else if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_INVALIDSWIPE_EN);
				MXDrawText_Left(Hdc, STR_INVALIDSWIPE_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_TIMMODIFIED_EN, xPos, 2);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr( HS_ID_INVALIDSWIPETIMEMODIED), 1);	
			}
		}
		break;


	case STEP_ALARM_SET_PROMPT:
		{
			if (g_ASPara.InvldCardSwipeAlmFlag)
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_INVALIDSWIPEALARMISENABLED_CN, 1);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_ALARMISENABLED_EN);
					MXDrawText_Left(Hdc, STR_INVALIDSWIPE_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_ALARMISENABLED_EN, xPos, 2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr( HS_ID_INVALIDSWIPEALARMISENABLED), 1);	
				}
			}
			else
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_INVALIDSWIPEALARMISDISABLED_CN, 1);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_ALARMISDISABLED_EN);
					MXDrawText_Left(Hdc, STR_INVALIDSWIPE_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_ALARMISDISABLED_EN, xPos, 2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr( HS_ID_INVALIDSWIPEALARMISDISABLED), 1);	
				}
			}
		}
		break;

	default:
		break;
	}
	
	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	InfraredSetWndProc
**	AUTHOR:			Jeff Wang
**	DATE:			28 - April - 2009
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
IvdCardSwipeSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		DataSetIdx = 0;
		TempIvdCardSwipeTimeset		=	g_ASPara.InvldCardSwipeRptSet;
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_IVDCARDSWIPE_SET, wParam, NULL);
		break;
		
	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			g_IvdCardSwipeAlarmSetInfo[0].MenuChineseName[0] = (UCHAR)(TempIvdCardSwipeTimeset / 100) + 0x30;
			g_IvdCardSwipeAlarmSetInfo[0].MenuChineseName[1] = (UCHAR)((TempIvdCardSwipeTimeset % 100) / 10) + 0x30;
			g_IvdCardSwipeAlarmSetInfo[0].MenuChineseName[2] = (UCHAR)((TempIvdCardSwipeTimeset % 100) % 10) + 0x30;
			strcpy(&g_IvdCardSwipeAlarmSetInfo[0].MenuChineseName[3], "次");
			
			g_IvdCardSwipeAlarmSetInfo[0].MenuEnglishName[0] = (UCHAR)(TempIvdCardSwipeTimeset / 100) + 0x30;
			g_IvdCardSwipeAlarmSetInfo[0].MenuEnglishName[1] = (UCHAR)((TempIvdCardSwipeTimeset % 100) / 10) + 0x30;
			g_IvdCardSwipeAlarmSetInfo[0].MenuEnglishName[2] = (UCHAR)((TempIvdCardSwipeTimeset % 100) % 10) + 0x30;
			strcpy(&g_IvdCardSwipeAlarmSetInfo[0].MenuEnglishName[3], "Times");
			g_IvdCardSwipeAlarmSetInfo[0].MenuEnglishName[0] = (UCHAR)(TempIvdCardSwipeTimeset / 100) + 0x30;
			
			HebrewIvdCardSwipTimeName[0] = (UCHAR)(TempIvdCardSwipeTimeset / 100) + 0x30;
			HebrewIvdCardSwipTimeName[1] = (UCHAR)((TempIvdCardSwipeTimeset % 100) / 10) + 0x30;
			HebrewIvdCardSwipTimeName[2] = (UCHAR)((TempIvdCardSwipeTimeset % 100) % 10) + 0x30;
			HebrewIvdCardSwipTimeName[3] ='\0';
			
			
			if (g_ASPara.InvldCardSwipeAlmFlag) 
			{
				strcpy(g_IvdCardSwipeAlarmSetInfo[1].MenuChineseName, MENU_ALARMON_CN);
				strcpy(g_IvdCardSwipeAlarmSetInfo[1].MenuEnglishName, MENU_ALARMON_EN);
				g_IvdCardSwipeAlarmSetInfo[1].HebrewStrID=HS_ID_ALARMON;
			}
			else
			{
				strcpy(g_IvdCardSwipeAlarmSetInfo[1].MenuChineseName, MENU_ALARMOFF_CN);
				strcpy(g_IvdCardSwipeAlarmSetInfo[1].MenuEnglishName, MENU_ALARMOFF_EN);
				g_IvdCardSwipeAlarmSetInfo[1].HebrewStrID=HS_ID_ALARMOFF;
			}
			
			IvdCardSwipeSetWndPaint(hWnd, Msg, wParam, lParam);
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
			return;			
		}
		ResetTimer(hWnd, TIMER_IVDCARDSWIPE_SET, INTERFACE_SETTLE_TIME, NULL);		
		IvdCardSwipeSetKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		IvdCardSwipeSetTimerProc(hWnd, Msg, wParam, lParam);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_IVDCARDSWIPE_SET);
		DataSetIdx				=	0;
		TempIvdCardSwipeTimeset		=	0;	
		IvdCardSwipeTimeAlarmStep = STEP_MAIN_WND;
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
**	FUNCTION NAME:	CreateIvdCardSwipeSetWnd
**	AUTHOR:			Jeff Wang
**	DATE:			28 - April - 2009
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
VOID
CreateIvdCardSwipeSetWnd(HWND hwndParent)
{
	static char szAppName[] = "IvdCardSwipeSetWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) IvdCardSwipeSetWndProc;
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
		"IvdCardSwipeSetWnd",		// Window name	(Not NULL)
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
**	FUNCTION NAME:	IvdCardSwipeSetKeyProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		28 - April - 2009
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

static VOID 
IvdCardSwipeSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	INT		i			=	0;
	
	wParam =  KeyBoardMap(wParam);

	if (KEY_RETURN == wParam)
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}

	if (STEP_MAIN_WND == IvdCardSwipeTimeAlarmStep) 
	{
		switch(wParam) 
		{
		case KEY_ENTER:
			{
				for (i = 0; i < IVDCARDSWIPESETDEPTH; i++)
				{
					if (g_IvdCardSwipeAlarmSetInfo[i].bSelected) 
					{
						if(g_IvdCardSwipeAlarmSetInfo[i].Menufun)
						{
							g_IvdCardSwipeAlarmSetInfo[i].Menufun();					
						}
						break;
					}
				}
			}
			break;
		case KEY_UP:
			{
				for (i = 0; i < IVDCARDSWIPESETDEPTH; i++)
				{
					if ( g_IvdCardSwipeAlarmSetInfo[i].bSelected ) 
					{
						if (0 == i) 
						{
							g_IvdCardSwipeAlarmSetInfo[i].bSelected = FALSE;
							g_IvdCardSwipeAlarmSetInfo[IVDCARDSWIPESETDEPTH-1].bSelected = TRUE;
						} 
						else
						{
							g_IvdCardSwipeAlarmSetInfo[i].bSelected = FALSE;
							g_IvdCardSwipeAlarmSetInfo[i-1].bSelected = TRUE;
						}
						break;
					}
				}
			}
			
			break;
		case KEY_DOWN:
			{
				for (i = 0; i < IVDCARDSWIPESETDEPTH; i++)
				{
					if ( g_IvdCardSwipeAlarmSetInfo[i].bSelected ) 
					{
						if (IVDCARDSWIPESETDEPTH-1 == i) 
						{
							g_IvdCardSwipeAlarmSetInfo[i].bSelected = FALSE;
							g_IvdCardSwipeAlarmSetInfo[0].bSelected = TRUE;
						} 
						else
						{
							g_IvdCardSwipeAlarmSetInfo[i].bSelected = FALSE;
							g_IvdCardSwipeAlarmSetInfo[i+1].bSelected = TRUE;
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

	else if(STEP_TIME_SET == IvdCardSwipeTimeAlarmStep)
	{
		if (KEY_ENTER == wParam)
		{
			g_ASPara.InvldCardSwipeRptSet = TempIvdCardSwipeTimeset;
			SaveMenuPara2Mem();
			IvdCardSwipeTimeAlarmStep = STEP_TIME_SET_PROMPT;
			ResetTimer(GetFocus(), TIMER_IVDCARDSWIPE_SET, PROMPT_SHOW_TIME, NULL);
		}
		else if ((wParam >=	KEY_NUM_0)&&(wParam <=KEY_NUM_9))
		{
			g_KeyInputBuffer[0] = (CHAR)(TempIvdCardSwipeTimeset / 100 + 0x30);
			g_KeyInputBuffer[1] = (CHAR)((TempIvdCardSwipeTimeset % 100)/10 + 0x30);
			g_KeyInputBuffer[2] = (CHAR)((TempIvdCardSwipeTimeset %100) % 10 + 0x30);
			
			CircleKeyEnter(g_KeyInputBuffer, &DataSetIdx, IVDCARDSWIPETIMELENTH, (CHAR)wParam);		
			
			TempIvdCardSwipeTimeset = 100 * (g_KeyInputBuffer[0] - 0x30)
				+10 * (g_KeyInputBuffer[1] - 0x30)
				+ (g_KeyInputBuffer[2] - 0x30);
			
			if (TempIvdCardSwipeTimeset > MAX_IVDSWIPE_TIME) 
			{
				TempIvdCardSwipeTimeset = MAX_IVDSWIPE_TIME;
			}
			else if (TempIvdCardSwipeTimeset < MIN_IVDSWIPE_TIME) 
			{
				TempIvdCardSwipeTimeset = MIN_IVDSWIPE_TIME;
			}	
		}
	}
}


static VOID
IvdCardSwipeTimeSet()
{
	IvdCardSwipeTimeAlarmStep	=	STEP_TIME_SET;
}

static VOID
IvdCardSwipeAlarmSet()
{
	g_ASPara.InvldCardSwipeAlmFlag	= !g_ASPara.InvldCardSwipeAlmFlag;
	SaveMenuPara2Mem();
	IvdCardSwipeTimeAlarmStep		= STEP_ALARM_SET_PROMPT;
	ResetTimer(GetFocus(), TIMER_IVDCARDSWIPE_SET, PROMPT_SHOW_TIME, NULL);
}





