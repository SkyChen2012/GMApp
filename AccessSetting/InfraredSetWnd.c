/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	InfraredSetWnd.c
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

#include "InfraredSetWnd.h"
#include "GataParaSetWnd.h"

/******************* DEFINES **************************************************************/

#define MAX_INFRARED_TIME 300
#define MIN_INFRARED_TIME 1

#define INFRAREDSETDEPTH	3
#define INFRAREDTIMELENTH	3

#define STR_INFRAREDDETECT_EN		"Infrared Detect"
#define STR_TIMMODIFIED_EN			"Time Modified"
#define STR_INFRAREDTIMEMODIED_CN		"红外检测时间已修改"

#define STR_INFRAREDALARM_EN		"Infrared Alarm"
#define STR_ISENABLED_EN			"Is Enabled"
#define STR_INFRAREDALARMISENABLED_CN		"红外报警已启动"
#define STR_ISDISABLED_EN			"Is Disabled"
#define STR_INFRAREDALARMISDISABLED_CN		"红外报警已关闭"

#define STR_INFRARED_EN					"Infrared"
#define STR_CONTACTOROPENED_EN			"Contactor Opened"
#define STR_INFRAREDCONTACTOROPENED_CN		"红外触点开"
#define STR_CONTACTORCLOSED_EN			"Contactor Closed"
#define STR_INFRAREDCONTACTORCLOSED_CN		"红外触点关"

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

static VOID		InfraredSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK InfraredSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID		InfraredSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID		InfraredTimeSet();
static VOID		InfraredAlarmSet();
static VOID		InfraredContactorSet();

static	UCHAR DataSetIdx				=	0;
static	INT  TempInfraredTimeset		=	0;

static TIMEALARMSTEP InfraredTimeAlarmStep = STEP_MAIN_WND;

static MENUINFO g_InfraredSetInfo[INFRAREDSETDEPTH] = 
{
	{TRUE,	0, "",	"",				InfraredTimeSet},
	{FALSE,  HS_ID_ALARMON,MENU_ALARMON_EN,	MENU_ALARMON_CN,			InfraredAlarmSet},
	{FALSE,  HS_ID_CONTACTORON,MENU_CONTACTORON_EN,	MENU_CONTACTORON_CN,			InfraredContactorSet}
};
static CHAR HebrewInfraredTimeName[MENU_LEN]={0};
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
InfraredSetTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (STEP_TIME_SET_PROMPT == InfraredTimeAlarmStep)
	{
		InfraredTimeAlarmStep = STEP_MAIN_WND;
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		ResetTimer(hWnd, TIMER_INFRARED_SET, INTERFACE_SETTLE_TIME, NULL);
	}
	else
	{
		if (STEP_ALARM_SET_PROMPT == InfraredTimeAlarmStep
			||STEP_CONTACTOR_SET_PROMPT == InfraredTimeAlarmStep) 
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
**	FUNCTION NAME:	InfraredSetWndPaint
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
InfraredSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
	
	switch(InfraredTimeAlarmStep) 
	{
	case STEP_MAIN_WND:
		{

#ifdef NEW_OLED_ENABLE	
			for(i = 0; i < INFRAREDSETDEPTH; i++)
			{	
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{	
/*
					if (g_InfraredSetInfo[i].bSelected)
					{
						memset(pDateBuf, '*',1);
					}
					else
					{
						memset(pDateBuf, ' ',1);
					}
*/
					memset(pDateBuf, ' ',2);			
					strcpy(&pDateBuf[2],g_InfraredSetInfo[i].MenuEnglishName);								
					MXDrawText_Center(Hdc, pDateBuf, i);
					if (g_InfraredSetInfo[i].bSelected)
					{
						DrawEnglishCur(Hdc,20,i);
					}
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
				
					if(0==i)
					{
						memset(pDateBuf, ' ',2);			
						strcpy(&pDateBuf[2],HebrewInfraredTimeName);	
						MXDrawText_Center(Hdc, pDateBuf, i);
					}
					else
					{
						memset(pDateBuf, ' ',2);			
						strcpy(&pDateBuf[2],GetHebrewStr(g_InfraredSetInfo[i].HebrewStrID));								
						MXDrawText_Center(Hdc, pDateBuf, i);
						if (g_InfraredSetInfo[i].bSelected)
						{
							DrawHebrewCur(Hdc,108,i);
						}
					}
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{	
/*
					if (g_InfraredSetInfo[i].bSelected)
					{
						memset(pDateBuf, '*',1);
					}
					else
					{
						memset(pDateBuf, ' ',1);
					}
*/
					memset(pDateBuf, ' ',2);			
					strcpy(&pDateBuf[2],g_InfraredSetInfo[i].MenuChineseName);								
					MXDrawText_Center(Hdc, pDateBuf, i);
					if (g_InfraredSetInfo[i].bSelected)
					{
						DrawChineseCur(Hdc,20,i);
					}
				}
			}	
#else
			for(i = 0; i < INFRAREDSETDEPTH; i++)
			{	
				if (g_InfraredSetInfo[i].bSelected)
				{
					Hdc->textcolor = FONT_COLOR_SEL;			
				}
				else
				{
					Hdc->textcolor = FONT_COLOR;				
				}
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{	
					MXDrawText_Center(Hdc, g_InfraredSetInfo[i].MenuEnglishName, i);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{	
					MXDrawText_Center(Hdc, g_InfraredSetInfo[i].MenuChineseName, i);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{	
					if(0==i)
					{
						MXDrawText_Center(Hdc, HebrewInfraredTimeName, i);
					}
					else
					{
						MXDrawText_Center(Hdc, GetHebrewStr(g_InfraredSetInfo[i].HebrewStrID), i);
					}	
				}
			}	
#endif
		}		
		break;
			
	case STEP_TIME_SET:
		{
			ClearKeyBuffer();
			g_KeyInputBuffer[0] = (CHAR)(TempInfraredTimeset / 100 + 0x30);
			g_KeyInputBuffer[1] = (CHAR)((TempInfraredTimeset % 100)/10 + 0x30);
			g_KeyInputBuffer[2] = (CHAR)((TempInfraredTimeset %100) % 10 + 0x30);
			
			DrawDataParaSet(Hdc, (CHAR*)g_KeyInputBuffer, DataSetIdx);
		}
		break;

	case STEP_TIME_SET_PROMPT:
		{
			if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, STR_INFRAREDTIMEMODIED_CN, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_NFRAREDTIMEMODIED), 1);
			}
			else if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_INFRAREDDETECT_EN);
				MXDrawText_Left(Hdc, STR_INFRAREDDETECT_EN, xPos, 2);
				MXDrawText_Left(Hdc, STR_TIMMODIFIED_EN, xPos, 3);
			}
		}
		break;


	case STEP_ALARM_SET_PROMPT:
		{
			if (g_ASPara.InfraredAlmFlag)
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_INFRAREDALARMISENABLED_CN, 1);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_INFRAREDALARM_EN);
					MXDrawText_Left(Hdc, STR_INFRAREDALARM_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_ISENABLED_EN, xPos, 2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_INFRAREDALARMISENABLED), 1);
				}
			}
			else
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_INFRAREDALARMISDISABLED_CN, 1);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_INFRAREDALARM_EN);
					MXDrawText_Left(Hdc, STR_INFRAREDALARM_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_ISDISABLED_EN, xPos, 2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_INFRAREDALARMISDISABLED), 1);
				}
			}
		}
		break;

		
	case STEP_CONTACTOR_SET_PROMPT:
		{
			if (g_ASPara.InfraredContactor)
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_INFRAREDCONTACTOROPENED_CN, 1);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_CONTACTOROPENED_EN);
					MXDrawText_Left(Hdc, STR_INFRARED_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_CONTACTOROPENED_EN, xPos, 2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_INFRAREDCONTACTOROPENED), 1);
				}
			}
			else
			{
				if (SET_CHINESE == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, STR_INFRAREDCONTACTORCLOSED_CN, 1);
				}
				else if (SET_ENGLISH == g_SysConfig.LangSel) 
				{
					xPos = GetXPos(STR_CONTACTORCLOSED_EN);
					MXDrawText_Left(Hdc, STR_INFRARED_EN, xPos, 1);
					MXDrawText_Left(Hdc, STR_CONTACTORCLOSED_EN, xPos, 2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_INFRAREDCONTACTORCLOSED), 1);
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
InfraredSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		DataSetIdx = 0;
		TempInfraredTimeset		=	g_ASPara.InfraredOverTime;
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_INFRARED_SET, wParam, NULL);
		break;
		
	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			g_InfraredSetInfo[0].MenuChineseName[0] = (UCHAR)(TempInfraredTimeset / 100) + 0x30;
			g_InfraredSetInfo[0].MenuChineseName[1] = (UCHAR)((TempInfraredTimeset % 100) / 10) + 0x30;
			g_InfraredSetInfo[0].MenuChineseName[2] = (UCHAR)((TempInfraredTimeset % 100) % 10) + 0x30;
			strcpy(&g_InfraredSetInfo[0].MenuChineseName[3], "秒");
			
			g_InfraredSetInfo[0].MenuEnglishName[0] = (UCHAR)(TempInfraredTimeset / 100) + 0x30;
			g_InfraredSetInfo[0].MenuEnglishName[1] = (UCHAR)((TempInfraredTimeset % 100) / 10) + 0x30;
			g_InfraredSetInfo[0].MenuEnglishName[2] = (UCHAR)((TempInfraredTimeset % 100) % 10) + 0x30;
			strcpy(&g_InfraredSetInfo[0].MenuEnglishName[3], "Sec");
			
			HebrewInfraredTimeName[0] = (UCHAR)(TempInfraredTimeset / 100) + 0x30;
			HebrewInfraredTimeName[1] = (UCHAR)((TempInfraredTimeset % 100) / 10) + 0x30;
			HebrewInfraredTimeName[2] = (UCHAR)((TempInfraredTimeset % 100) % 10) + 0x30;
			strcpy(&HebrewInfraredTimeName[3], GetHebrewStr(HS_ID_SECONDS));
			
			
			if (g_ASPara.InfraredAlmFlag) 
			{
				strcpy(g_InfraredSetInfo[1].MenuChineseName, MENU_ALARMON_CN);
				strcpy(g_InfraredSetInfo[1].MenuEnglishName, MENU_ALARMON_EN);
				g_InfraredSetInfo[1].HebrewStrID=HS_ID_ALARMON;
			}
			else
			{
				strcpy(g_InfraredSetInfo[1].MenuChineseName, MENU_ALARMOFF_CN);
				strcpy(g_InfraredSetInfo[1].MenuEnglishName, MENU_ALARMOFF_EN);
				g_InfraredSetInfo[1].HebrewStrID=HS_ID_ALARMOFF;
			}
			
			if (g_ASPara.InfraredContactor) 
			{
				strcpy(g_InfraredSetInfo[2].MenuChineseName, MENU_CONTACTORON_CN);
				strcpy(g_InfraredSetInfo[2].MenuEnglishName, MENU_CONTACTORON_EN);
				g_InfraredSetInfo[2].HebrewStrID=HS_ID_CONTACTORON;
			}
			else
			{
				strcpy(g_InfraredSetInfo[2].MenuChineseName, MENU_CONTACTOROFF_CN);
				strcpy(g_InfraredSetInfo[2].MenuEnglishName, MENU_CONTACTOROFF_EN);
				g_InfraredSetInfo[2].HebrewStrID=HS_ID_CONTACTOROFF;
			}
			
			InfraredSetWndPaint(hWnd, Msg, wParam, lParam);
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
		ResetTimer(hWnd, TIMER_INFRARED_SET, INTERFACE_SETTLE_TIME, NULL);		
		InfraredSetKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		InfraredSetTimerProc(hWnd, Msg, wParam, lParam);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_INFRARED_SET);
		DataSetIdx				=	0;
		TempInfraredTimeset		=	0;	
		InfraredTimeAlarmStep = STEP_MAIN_WND;
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
**	FUNCTION NAME:	CreateInfraredSetWnd
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
CreateInfraredSetWnd(HWND hwndParent)
{
	static char szAppName[] = "InfrareSetWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) InfraredSetWndProc;
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
		"InfrareSetWnd",		// Window name	(Not NULL)
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
**	FUNCTION NAME:	InfraredSetKeyProcess
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
InfraredSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	INT		i			=	0;
	
	wParam =  KeyBoardMap(wParam);

	if (KEY_RETURN == wParam)
	{
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}

	if (STEP_MAIN_WND == InfraredTimeAlarmStep) 
	{
		switch(wParam) 
		{
		case KEY_ENTER:
			{
				for (i = 0; i < INFRAREDSETDEPTH; i++)
				{
					if (g_InfraredSetInfo[i].bSelected) 
					{
						if(g_InfraredSetInfo[i].Menufun)
						{
							g_InfraredSetInfo[i].Menufun();					
						}
						break;
					}
				}
			}
			break;
		case KEY_UP:
			{
				for (i = 0; i < INFRAREDSETDEPTH; i++)
				{
					if ( g_InfraredSetInfo[i].bSelected ) 
					{
						if (0 == i) 
						{
							g_InfraredSetInfo[i].bSelected = FALSE;
							g_InfraredSetInfo[INFRAREDSETDEPTH-1].bSelected = TRUE;
						} 
						else
						{
							g_InfraredSetInfo[i].bSelected = FALSE;
							g_InfraredSetInfo[i-1].bSelected = TRUE;
						}
						break;
					}
				}
			}
			
			break;
		case KEY_DOWN:
			{
				for (i = 0; i < INFRAREDSETDEPTH; i++)
				{
					if ( g_InfraredSetInfo[i].bSelected ) 
					{
						if (INFRAREDSETDEPTH-1 == i) 
						{
							g_InfraredSetInfo[i].bSelected = FALSE;
							g_InfraredSetInfo[0].bSelected = TRUE;
						} 
						else
						{
							g_InfraredSetInfo[i].bSelected = FALSE;
							g_InfraredSetInfo[i+1].bSelected = TRUE;
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

	else if(STEP_TIME_SET == InfraredTimeAlarmStep)
	{
		if (KEY_ENTER == wParam)
		{
			g_ASPara.InfraredOverTime = TempInfraredTimeset;
			SaveMenuPara2Mem();
			InfraredTimeAlarmStep = STEP_TIME_SET_PROMPT;
			ResetTimer(GetFocus(), TIMER_INFRARED_SET, PROMPT_SHOW_TIME, NULL);
		}
		else if ((wParam >=	KEY_NUM_0)&&(wParam <=KEY_NUM_9))
		{
			g_KeyInputBuffer[0] = (CHAR)(TempInfraredTimeset / 100 + 0x30);
			g_KeyInputBuffer[1] = (CHAR)((TempInfraredTimeset % 100)/10 + 0x30);
			g_KeyInputBuffer[2] = (CHAR)((TempInfraredTimeset %100) % 10 + 0x30);
			
			CircleKeyEnter(g_KeyInputBuffer, &DataSetIdx, INFRAREDTIMELENTH, (CHAR)wParam);		
			
			TempInfraredTimeset = 100 * (g_KeyInputBuffer[0] - 0x30)
				+10 * (g_KeyInputBuffer[1] - 0x30)
				+ (g_KeyInputBuffer[2] - 0x30);
			
			if (TempInfraredTimeset > MAX_INFRARED_TIME) 
			{
				TempInfraredTimeset = MAX_INFRARED_TIME;
			}
			else if (TempInfraredTimeset < MIN_INFRARED_TIME) 
			{
				TempInfraredTimeset = MIN_INFRARED_TIME;
			}	
		}
	}
}


static VOID
InfraredTimeSet()
{
	InfraredTimeAlarmStep	=	STEP_TIME_SET;
}

static VOID
InfraredAlarmSet()
{
	g_ASPara.InfraredAlmFlag	= !g_ASPara.InfraredAlmFlag;
	SaveMenuPara2Mem();
	InfraredTimeAlarmStep		= STEP_ALARM_SET_PROMPT;
	ResetTimer(GetFocus(), TIMER_INFRARED_SET, PROMPT_SHOW_TIME, NULL);
}

static VOID		
InfraredContactorSet()
{
	g_ASPara.InfraredContactor	=	!g_ASPara.InfraredContactor;
	SaveMenuPara2Mem();
	InfraredTimeAlarmStep		=	STEP_CONTACTOR_SET_PROMPT;
	ResetTimer(GetFocus(), TIMER_INFRARED_SET, PROMPT_SHOW_TIME, NULL);
}



