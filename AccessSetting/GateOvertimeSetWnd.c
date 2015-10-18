/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	GateOvertimeSetWnd.c
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

#include "GateOvertimeSetWnd.h"
#include "GataParaSetWnd.h"

/******************* DEFINES **************************************************************/

#define MAX_GATEOVERTIME_TIME 100
#define MIN_GATEOVERTIME_TIME 1

#define GATEOVERTIMESETDEPTH	3
#define GATEOVERTIMETIMELENTH	3

#define STR_GATEOVERTIME_EN		"Gate Overtime"
#define STR_TIMMODIFIED_EN			"Time Modified"
#define STR_GATEOVERTIMETIMEMODIED_CN		"开门超时时间已修改"

#define STR_ALARMISENABLED_EN			"Alarm Is Enabled"
#define STR_GATEOVERTIMEALARMISENABLED_CN		"开门超时报警已启动"
#define STR_ALARMISDISABLED_EN			"Alarm Is Disabled"
#define STR_GATEOVERTIMEALARMISDISABLED_CN		"开门超时报警已关闭"

#define STR_CONTACTOROPENED_EN			"Contactor Opened"
#define STR_GATEOVERTIMECONTACTOROPENED_CN		"开门超时触点开"
#define STR_CONTACTORCLOSED_EN			"Contactor Closed"
#define STR_GATEOVERTIMECONTACTORCLOSED_CN		"开门超时触点关"

#define MENU_ALARMON_EN		"Alarm ON"
#define MENU_ALARMON_CN		"报警开启"
#define MENU_ALARMOFF_EN		"Alarm OFF"
#define MENU_ALARMOFF_CN		"报警关闭"

#define MENU_CONTACTORON_EN		"Contactor ON"
#define MENU_CONTACTORON_CN		"开触点"
#define MENU_CONTACTOROFF_EN		"Contactor OFF"
#define MENU_CONTACTOROFF_CN		"闭触点"


/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		GateOvertimeSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK GateOvertimeSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID		GateOvertimeSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID		GateOvertimeTimeSet();
static VOID		GateOvertimeAlarmSet();
static VOID		GateOvertimeContactorSet();

static	UCHAR DataSetIdx				=	0;
static	CHAR  TempGateOvertimeTimeset		=	0;

static TIMEALARMSTEP GateOvertimeTimeAlarmStep = STEP_MAIN_WND;

static MENUINFO g_GateOvertimeSetInfo[GATEOVERTIMESETDEPTH] = 
{
	{TRUE,	0, "",	"",				GateOvertimeTimeSet},
	{FALSE,  HS_ID_ALARMON,MENU_ALARMON_EN,	MENU_ALARMON_CN,			GateOvertimeAlarmSet},
	{FALSE,  HS_ID_CONTACTORON,MENU_CONTACTORON_EN,	MENU_CONTACTORON_CN,			GateOvertimeContactorSet}
};
static CHAR HebrewGateTimeName[MENU_LEN]={0};
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
GateOvertimeSetTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (STEP_TIME_SET_PROMPT == GateOvertimeTimeAlarmStep)
	{
		GateOvertimeTimeAlarmStep = STEP_MAIN_WND;
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		ResetTimer(hWnd, TIMER_GATEOVERTIME_SET, INTERFACE_SETTLE_TIME, NULL);
	}
	else
	{
		if (STEP_ALARM_SET_PROMPT == GateOvertimeTimeAlarmStep
			||STEP_CONTACTOR_SET_PROMPT == GateOvertimeTimeAlarmStep)
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
**	FUNCTION NAME:	GateOvertimeSetWndPaint
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
GateOvertimeSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
	
	switch(GateOvertimeTimeAlarmStep) 
	{
	case STEP_MAIN_WND:
		{

#ifdef NEW_OLED_ENABLE	
			for(i = 0; i < GATEOVERTIMESETDEPTH; i++)
			{	
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{	
/*
					if (g_GateOvertimeSetInfo[i].bSelected)
					{
						memset(pDateBuf, '*',1);
					}
					else
					{
						memset(pDateBuf, ' ',1);
					}
*/
					memset(pDateBuf, ' ',2);			
					strcpy(&pDateBuf[2],g_GateOvertimeSetInfo[i].MenuEnglishName);								
					MXDrawText_Center(Hdc, pDateBuf, i);
					if (g_GateOvertimeSetInfo[i].bSelected)
					{
						DrawEnglishCur(Hdc,20,i);
					}
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					if(0==i)
					{
						memset(pDateBuf, ' ',2);			
						strcpy(&pDateBuf[2],HebrewGateTimeName);	
						MXDrawText_Center(Hdc, pDateBuf, i);
					}
					else
					{
						memset(pDateBuf, ' ',2);			
						strcpy(&pDateBuf[2],GetHebrewStr(g_GateOvertimeSetInfo[i].HebrewStrID));								
						MXDrawText_Center(Hdc, pDateBuf, i);
						if (g_GateOvertimeSetInfo[i].bSelected)
						{
							DrawHebrewCur(Hdc,108,i);
						}
					}
					
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{	
/*
					if (g_GateOvertimeSetInfo[i].bSelected)
					{
						memset(pDateBuf, '*',1);
					}
					else
					{
						memset(pDateBuf, ' ',1);
					}
*/
					memset(pDateBuf, ' ',2);			
					strcpy(&pDateBuf[2],g_GateOvertimeSetInfo[i].MenuChineseName);								
					MXDrawText_Center(Hdc, pDateBuf, i);
					if (g_GateOvertimeSetInfo[i].bSelected)
					{
						DrawChineseCur(Hdc,20,i);
					}
				}
			}	
#else
			for(i = 0; i < GATEOVERTIMESETDEPTH; i++)
			{	
				if (g_GateOvertimeSetInfo[i].bSelected)
				{
					Hdc->textcolor = FONT_COLOR_SEL;			
				}
				else
				{
					Hdc->textcolor = FONT_COLOR;				
				}
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{	
					MXDrawText_Center(Hdc, g_GateOvertimeSetInfo[i].MenuEnglishName, i);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{	
					MXDrawText_Center(Hdc, g_GateOvertimeSetInfo[i].MenuChineseName, i);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{	
					if(0==i)
					{
						MXDrawText_Center(Hdc, HebrewGateTimeName, i);
					}
					else
					{
						MXDrawText_Center(Hdc, GetHebrewStr(g_GateOvertimeSetInfo[i].HebrewStrID), i);
					}	
				}
			}	
#endif
		}		
		break;
			
	case STEP_TIME_SET:
		{
			ClearKeyBuffer();
			g_KeyInputBuffer[0] = (CHAR)(TempGateOvertimeTimeset / 100 + 0x30);
			g_KeyInputBuffer[1] = (CHAR)((TempGateOvertimeTimeset % 100)/10 + 0x30);
			g_KeyInputBuffer[2] = (CHAR)((TempGateOvertimeTimeset %100) % 10 + 0x30);
			
			DrawDataParaSet(Hdc, (CHAR*)g_KeyInputBuffer, DataSetIdx);
		}
		break;

	case STEP_TIME_SET_PROMPT:
		{
			if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_GATEOVERTIMETIMEMODIED_CN, 1);
			}
			else if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_GATEOVERTIME_EN);
				MXDrawText_Left(Hdc, STR_GATEOVERTIME_EN, xPos, 2);
				MXDrawText_Left(Hdc, STR_TIMMODIFIED_EN, xPos, 3);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_GATEOVERTIMETIMEMODIED), 1);
			}
		}
		break;


	case STEP_ALARM_SET_PROMPT:
		{
			if (g_ASPara.GateOpenAlmFlag)
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_GATEOVERTIMEALARMISENABLED_CN, 1);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_GATEOVERTIME_EN);
					MXDrawText_Left(Hdc, STR_GATEOVERTIME_EN, xPos, 2);
					MXDrawText_Left(Hdc, STR_ALARMISENABLED_EN, xPos, 3);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_GATEOVERTIMEALARMISENABLED), 1);
				}
			}
			else
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_GATEOVERTIMEALARMISDISABLED_CN, 1);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_GATEOVERTIME_EN);
					MXDrawText_Left(Hdc, STR_GATEOVERTIME_EN, xPos, 2);
					MXDrawText_Left(Hdc, STR_ALARMISDISABLED_EN, xPos, 3);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_GATEOVERTIMEALARMISDISABLED), 1);
				}
			}
		}
		break;

		
	case STEP_CONTACTOR_SET_PROMPT:
		{
			if (g_ASPara.GateOpenContactor)
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_GATEOVERTIMECONTACTOROPENED_CN, 1);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_CONTACTOROPENED_EN);
					MXDrawText_Left(Hdc, STR_GATEOVERTIME_EN, xPos, 2);
					MXDrawText_Left(Hdc, STR_CONTACTOROPENED_EN, xPos, 3);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_GATEOVERTIMECONTACTOROPENED), 1);
				}
			}
			else
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_GATEOVERTIMECONTACTORCLOSED_CN, 1);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_CONTACTORCLOSED_EN);
					MXDrawText_Left(Hdc, STR_GATEOVERTIME_EN, xPos, 2);
					MXDrawText_Left(Hdc, STR_CONTACTORCLOSED_EN, xPos, 3);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_GATEOVERTIMECONTACTORCLOSED), 1);
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
**	FUNCTION NAME:	GateOvertimeSetWndProc
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
GateOvertimeSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		DataSetIdx = 0;
		TempGateOvertimeTimeset		=	g_ASPara.GateOpenOverTime;	
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);	
		break;
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_GATEOVERTIME_SET, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			g_GateOvertimeSetInfo[0].MenuChineseName[0] = (UCHAR)(TempGateOvertimeTimeset / 100) + 0x30;
			g_GateOvertimeSetInfo[0].MenuChineseName[1] = (UCHAR)((TempGateOvertimeTimeset % 100) / 10) + 0x30;
			g_GateOvertimeSetInfo[0].MenuChineseName[2] = (UCHAR)((TempGateOvertimeTimeset % 100) % 10) + 0x30;
			strcpy(&g_GateOvertimeSetInfo[0].MenuChineseName[3], "秒");
			
			g_GateOvertimeSetInfo[0].MenuEnglishName[0] = (UCHAR)(TempGateOvertimeTimeset / 100) + 0x30;
			g_GateOvertimeSetInfo[0].MenuEnglishName[1] = (UCHAR)((TempGateOvertimeTimeset % 100) / 10) + 0x30;
			g_GateOvertimeSetInfo[0].MenuEnglishName[2] = (UCHAR)((TempGateOvertimeTimeset % 100) % 10) + 0x30;
			strcpy(&g_GateOvertimeSetInfo[0].MenuEnglishName[3], "Sec");
			
			
			HebrewGateTimeName[0] = (UCHAR)(TempGateOvertimeTimeset / 100) + 0x30;
			HebrewGateTimeName[1] = (UCHAR)((TempGateOvertimeTimeset % 100) / 10) + 0x30;
			HebrewGateTimeName[2] = (UCHAR)((TempGateOvertimeTimeset % 100) % 10) + 0x30;
			strcpy(&HebrewGateTimeName[3], GetHebrewStr(HS_ID_SECONDS));
			
			if (g_ASPara.GateOpenAlmFlag) 
			{
				strcpy(g_GateOvertimeSetInfo[1].MenuChineseName, MENU_ALARMON_CN);
				strcpy(g_GateOvertimeSetInfo[1].MenuEnglishName, MENU_ALARMON_EN);
				g_GateOvertimeSetInfo[1].HebrewStrID=HS_ID_ALARMON;
			}
			else
			{
				strcpy(g_GateOvertimeSetInfo[1].MenuChineseName, MENU_ALARMOFF_CN);
				strcpy(g_GateOvertimeSetInfo[1].MenuEnglishName, MENU_ALARMOFF_EN);
				g_GateOvertimeSetInfo[1].HebrewStrID=HS_ID_ALARMOFF;
			}
			
			if (g_ASPara.GateOpenContactor) 
			{
				strcpy(g_GateOvertimeSetInfo[2].MenuChineseName, MENU_CONTACTORON_CN);
				strcpy(g_GateOvertimeSetInfo[2].MenuEnglishName, MENU_CONTACTORON_EN);
				g_GateOvertimeSetInfo[2].HebrewStrID=HS_ID_CONTACTORON;
			}
			else
			{
				strcpy(g_GateOvertimeSetInfo[2].MenuChineseName, MENU_CONTACTOROFF_CN);
				strcpy(g_GateOvertimeSetInfo[2].MenuEnglishName, MENU_CONTACTOROFF_EN);
				g_GateOvertimeSetInfo[2].HebrewStrID=HS_ID_CONTACTOROFF;
			}
			
			GateOvertimeSetWndPaint(hWnd, Msg, wParam, lParam);
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
		ResetTimer(hWnd, TIMER_GATEOVERTIME_SET, INTERFACE_SETTLE_TIME, NULL);		
		GateOvertimeSetKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		GateOvertimeSetTimerProc(hWnd, Msg, wParam, lParam);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_GATEOVERTIME_SET);
		DataSetIdx				=	0;
		TempGateOvertimeTimeset		=	0;	
		GateOvertimeTimeAlarmStep = STEP_MAIN_WND;
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
**	FUNCTION NAME:	CreateGateOvertimeSetWnd
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
CreateGateOvertimeSetWnd(HWND hwndParent)
{
	static char szAppName[] = "GateOvertimeSetWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) GateOvertimeSetWndProc;
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
		"GateOvertimeSetWnd",		// Window name	(Not NULL)
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
**	FUNCTION NAME:	GateOvertimeSetKeyProcess
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
GateOvertimeSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	INT		i			=	0;
	
	wParam =  KeyBoardMap(wParam);

	if (KEY_RETURN == wParam)
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}

	if (STEP_MAIN_WND == GateOvertimeTimeAlarmStep) 
	{
		switch(wParam) 
		{
		case KEY_ENTER:
			{
				for (i = 0; i < GATEOVERTIMESETDEPTH; i++)
				{
					if (g_GateOvertimeSetInfo[i].bSelected) 
					{
						if(g_GateOvertimeSetInfo[i].Menufun)
						{
							g_GateOvertimeSetInfo[i].Menufun();					
						}
						break;
					}
				}
			}
			break;
		case KEY_UP:
			{
				for (i = 0; i < GATEOVERTIMESETDEPTH; i++)
				{
					if ( g_GateOvertimeSetInfo[i].bSelected ) 
					{
						if (0 == i) 
						{
							g_GateOvertimeSetInfo[i].bSelected = FALSE;
							g_GateOvertimeSetInfo[GATEOVERTIMESETDEPTH-1].bSelected = TRUE;
						} 
						else
						{
							g_GateOvertimeSetInfo[i].bSelected = FALSE;
							g_GateOvertimeSetInfo[i-1].bSelected = TRUE;
						}
						break;
					}
				}
			}
			
			break;
		case KEY_DOWN:
			{
				for (i = 0; i < GATEOVERTIMESETDEPTH; i++)
				{
					if ( g_GateOvertimeSetInfo[i].bSelected ) 
					{
						if (GATEOVERTIMESETDEPTH-1 == i) 
						{
							g_GateOvertimeSetInfo[i].bSelected = FALSE;
							g_GateOvertimeSetInfo[0].bSelected = TRUE;
						} 
						else
						{
							g_GateOvertimeSetInfo[i].bSelected = FALSE;
							g_GateOvertimeSetInfo[i+1].bSelected = TRUE;
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

	else if(STEP_TIME_SET == GateOvertimeTimeAlarmStep)
	{
		if (KEY_ENTER == wParam)
		{
			g_ASPara.GateOpenOverTime = TempGateOvertimeTimeset;
			SaveMenuPara2Mem();
			GateOvertimeTimeAlarmStep = STEP_TIME_SET_PROMPT;
			ResetTimer(GetFocus(), TIMER_GATEOVERTIME_SET, PROMPT_SHOW_TIME, NULL);
		}
		else if ((wParam >=	KEY_NUM_0)&&(wParam <=KEY_NUM_9))
		{
			g_KeyInputBuffer[0] = (CHAR)(TempGateOvertimeTimeset / 100 + 0x30);
			g_KeyInputBuffer[1] = (CHAR)((TempGateOvertimeTimeset % 100)/10 + 0x30);
			g_KeyInputBuffer[2] = (CHAR)((TempGateOvertimeTimeset %100) % 10 + 0x30);
			
			CircleKeyEnter(g_KeyInputBuffer, &DataSetIdx, GATEOVERTIMETIMELENTH, (CHAR)wParam);		
			
			TempGateOvertimeTimeset = 100 * (g_KeyInputBuffer[0] - 0x30)
				+10 * (g_KeyInputBuffer[1] - 0x30)
				+ (g_KeyInputBuffer[2] - 0x30);
			
			if (TempGateOvertimeTimeset > MAX_GATEOVERTIME_TIME) 
			{
				TempGateOvertimeTimeset = MAX_GATEOVERTIME_TIME;
			}
			else if (TempGateOvertimeTimeset < MIN_GATEOVERTIME_TIME) 
			{
				TempGateOvertimeTimeset = MIN_GATEOVERTIME_TIME;
			}	
		}
	}
}


static VOID
GateOvertimeTimeSet()
{
	GateOvertimeTimeAlarmStep	=	STEP_TIME_SET;
}

static VOID
GateOvertimeAlarmSet()
{
	g_ASPara.GateOpenAlmFlag	= !g_ASPara.GateOpenAlmFlag;
	SaveMenuPara2Mem();
	GateOvertimeTimeAlarmStep		= STEP_ALARM_SET_PROMPT;
	ResetTimer(GetFocus(), TIMER_GATEOVERTIME_SET, PROMPT_SHOW_TIME, NULL);
}

static VOID		
GateOvertimeContactorSet()
{
	g_ASPara.GateOpenContactor	=	!g_ASPara.GateOpenContactor;
	SaveMenuPara2Mem();
	GateOvertimeTimeAlarmStep		=	STEP_CONTACTOR_SET_PROMPT;
	ResetTimer(GetFocus(), TIMER_GATEOVERTIME_SET, PROMPT_SHOW_TIME, NULL);
}





