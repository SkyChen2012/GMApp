/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	LanguageSet.c
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
#include "LanguageSet.h"

/************** DEFINES **************************************************************/
#ifdef NEW_OLED_ENABLE	
	#define LANGUAGESETDEPTH  3
#else
	#define LANGUAGESETDEPTH  2
#endif
/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static void		LanguageSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		ShowLangSetPromptInfo(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK LanguageSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		LanguageSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static void		LanguageSelChinese();
static void		LanguageSelEnlish();
static void		LanguageSelHebrew();

static MENUINFO g_LangSettingInfo[LANGUAGESETDEPTH] = 
{
	{TRUE, HS_ID_LANG_CHINESE, "Chinese",	"Chinese",	       LanguageSelChinese},
	{FALSE, HS_ID_LANG_ENGLISH,"English", 	"English",		LanguageSelEnlish}
#ifdef NEW_OLED_ENABLE	
	,	
	{FALSE, HS_ID_LANG_HEBREW,"Hebrew", 	"Hebrew",		LanguageSelHebrew}
#endif
};
static	BOOL bShowPromptInfo = FALSE; 
static	BYTE byOldLangSel = 0; 
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
LanguageSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	int								  i	 = 0;
	INT								xPos = 0;
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	
	for(i = 0; i < LANGUAGESETDEPTH; i++)
	{

#ifdef NEW_OLED_ENABLE		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  Chinese");
/*
			if (g_LangSettingInfo[i].bSelected)
			{
				memset(pDateBuf, '*',1);
			}
			else
			{	
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);							
			strcpy(&pDateBuf[2],g_LangSettingInfo[i].MenuEnglishName);			
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_LangSettingInfo[i].bSelected)
			{
				DrawEnglishCur(Hdc,xPos,i);
			}
			
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_LANG_CHINESE));

			memset(pDateBuf, ' ',2);	
			strcpy(&pDateBuf[2],GetHebrewStr(g_LangSettingInfo[i].HebrewStrID)	);								
			MXDrawText_Right(Hdc, pDateBuf, xPos, i +1);
			if (g_LangSettingInfo[i].bSelected)
			{
				DrawHebrewCur(Hdc,xPos,i+1);
			}
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  Chinese");
/*
			if (g_LangSettingInfo[i].bSelected)
			{
				memset(pDateBuf, '*',1);
			}
			else
			{	
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_LangSettingInfo[i].MenuChineseName);			
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			if (g_LangSettingInfo[i].bSelected)
			{
				DrawChineseCur(Hdc,xPos,i);
			}
			
		}
#else
		if (g_LangSettingInfo[i].bSelected)
		{
			Hdc->textcolor = FONT_COLOR_SEL;			
		}
		else
		{	
			Hdc->textcolor = FONT_COLOR;			
		}
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("Chinese");
			MXDrawText_Left(Hdc, g_LangSettingInfo[i].MenuEnglishName, xPos, i);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("中文");
			MXDrawText_Left(Hdc, g_LangSettingInfo[i].MenuChineseName, xPos, i);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc, GetHebrewStr(HS_ID_LANG_CHINESE));
			MXDrawText_Right(Hdc, GetHebrewStr(g_LangSettingInfo[i].HebrewStrID), xPos, i);
		}
#endif
	}
	
	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ShowLangSetPromptInfo
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
ShowLangSetPromptInfo(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;
	INT			xPos = 0;
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);		
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;	
	if(SET_HEBREW==byOldLangSel)
	{
		if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_HEBREW_SELED));
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_HEBREW_SELED), xPos, 1);
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PLEASERESTART), xPos, 2);
		}
		else if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ENGLISH_SELED));
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_ENGLISH_SELED), xPos, 1);
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PLEASERESTART), xPos, 2);
		}
		else if(SET_CHINESE == g_SysConfig.LangSel) 
		{
			g_SysConfig.LangSel=SET_HEBREW;
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_LANG_CHINESE_SELED));
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_LANG_CHINESE_SELED), xPos, 1);
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_LANG_RESTART_CN), xPos, 2);
		}
		g_SysConfig.LangSel=SET_HEBREW;

	}
	else if(SET_HEBREW==g_SysConfig.LangSel)
	{
		if (SET_HEBREW == byOldLangSel) 
		{
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_HEBREW_SELED));
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_HEBREW_SELED), xPos, 1);
			MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PLEASERESTART), xPos, 2);
		}
		else if (SET_ENGLISH == byOldLangSel) 
		{
			MXDrawText_En(Hdc, &Rect, "Hebrew selected", 1);
			MXDrawText_En(Hdc, &Rect, "Please Restart", 2);
		}
		else if(SET_CHINESE==byOldLangSel)
		{
			MXDrawText_Cn(Hdc, &Rect, "Hebrew selected", 1);
			MXDrawText_Cn(Hdc, &Rect, "Please Restart", 2);
		}
		g_SysConfig.LangSel=byOldLangSel;
	}
	else
	{
		if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			MXDrawText_Cn(Hdc, &Rect, "中文已选择", 1);
		}
		else if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			MXDrawText_En(Hdc, &Rect, "English selected", 1);
		}
	}
	
	
	ResetTimer(hWnd, TIMER_LANGSET_PMT, PROMPT_SHOW_TIME, NULL);
	
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
LanguageSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);		
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_LANGSET_PMT, wParam, NULL);
		break;
		
	case WM_REFRESHPARENT_WND:
		SetFocus(hWnd);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;

	
	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			if (bShowPromptInfo) 
			{
				ShowLangSetPromptInfo(hWnd, Msg, wParam, lParam);
			}
			else
			{
				LanguageSetWndPaint(hWnd, Msg, wParam, lParam);
			}
		}
		
		break;
		
	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		if (!bShowPromptInfo)
		{
			ResetTimer(hWnd, TIMER_LANGSET_PMT, INTERFACE_SETTLE_TIME, NULL);
			LanguageSetKeyProcess(hWnd, Msg, wParam, lParam);
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_LANGSET_PMT);
		if (bShowPromptInfo) 
		{
			bShowPromptInfo = FALSE;		
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		else
		{
			KillAllChildWnd(hWnd);		
		}
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_LANGSET_PMT);
//		SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		break;
		
	default:
		DefWindowProc(hWnd, Msg, wParam, lParam);
		
		if (WM_CLOSE == Msg) 
		{
			RemoveOneWnd(hWnd);
		}
		
		//		return DefWindowProc(hWnd, Msg, wParam, lParam);
		/*
		if (WM_CLOSE == Msg) 
		{
			RemoveOneWnd(hWnd);
		}
		return DefWindowProc(hWnd, Msg, wParam, lParam);
*/
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
CreateLanguageSetWnd(HWND hwndParent)
{
	static char szAppName[] = "LanguageSetWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) LanguageSetWndProc;
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
		"LanguageSetWnd",			// Window name	(Not NULL)
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
**	FUNCTION NAME:	LanguageSetKeyProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		21 - Aug - 2008
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
LanguageSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int i = 0;

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
			for (i = 0; i < LANGUAGESETDEPTH; i++)
			{
				if (g_LangSettingInfo[i].bSelected) 
				{
					if(g_LangSettingInfo[i].Menufun)
					{
						g_LangSettingInfo[i].Menufun();					
					}
					break;
				}
			}
		}
		break;
	case KEY_UP:
		{
			for (i = 0; i < LANGUAGESETDEPTH; i++)
			{
				if ( g_LangSettingInfo[i].bSelected ) 
				{
					if (0 == i) 
					{
						g_LangSettingInfo[i].bSelected = FALSE;
						g_LangSettingInfo[LANGUAGESETDEPTH-1].bSelected = TRUE;
					} 
					else
					{
						g_LangSettingInfo[i].bSelected = FALSE;
						g_LangSettingInfo[i-1].bSelected = TRUE;
					}
					break;
				}
			}
		}
		
		break;
	case KEY_DOWN:
		{
			for (i = 0; i < LANGUAGESETDEPTH; i++)
			{
				if ( g_LangSettingInfo[i].bSelected ) 
				{
					if (LANGUAGESETDEPTH-1 == i) 
					{
						g_LangSettingInfo[i].bSelected = FALSE;
						g_LangSettingInfo[0].bSelected = TRUE;
					} 
					else
					{
						g_LangSettingInfo[i].bSelected = FALSE;
						g_LangSettingInfo[i+1].bSelected = TRUE;
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

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LanguageSelChinese
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
LanguageSelChinese()
{
	byOldLangSel=g_SysConfig.LangSel;
	g_SysConfig.LangSel = SET_CHINESE;
	bShowPromptInfo = TRUE;
	SaveMenuPara2Mem();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LanguageSelEnlish
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
LanguageSelEnlish()
{
	byOldLangSel=g_SysConfig.LangSel;
	g_SysConfig.LangSel = SET_ENGLISH;
	bShowPromptInfo = TRUE;
	SaveMenuPara2Mem();
}
static void
LanguageSelHebrew()
{
	byOldLangSel=g_SysConfig.LangSel;
	g_SysConfig.LangSel = SET_HEBREW;
	bShowPromptInfo = TRUE;
	SaveMenuPara2Mem();
}
