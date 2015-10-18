/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	ManOpenSetWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		23 - Aug - 2008
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

/************** USER INCLUDE FILES ***************************************************/
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

#include "ManOpenSetWnd.h"

/************** DEFINES **************************************************************/

#define MANOPENSETDEPTH  1

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		ManOpenSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK ManOpenSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID 	ShowManOpenSetPromptInfo(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID 	ManOpenSetProc();

static VOID		GMManOpenSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		DBManOpenSetKeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		DBPasswordInputProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static BOOL bShowPromptInfo = FALSE;

static MENUINFO g_ManOpenSetInfo[MANOPENSETDEPTH] = 
{
	{TRUE,HS_ID_ON,  "ON",	"开启",			ManOpenSetProc}
};

static MENUENTERSTEP GateManOpenStep = MENU_INPUT_PASSWORD;


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenAlarmWndPaint
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
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
ManOpenSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	int								  i	 = 0;
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	

	
#ifdef NEW_OLED_ENABLE	
	for(i = 0; i < MANOPENSETDEPTH; i++)
	{	
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
/*
			if (g_ManOpenSetInfo[i].bSelected)
			{
				memset(pDateBuf, '*',1);
			}
			else
			{
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_ManOpenSetInfo[i].MenuEnglishName);								
			MXDrawText_En(Hdc, &Rect, pDateBuf, i);
			if (g_ManOpenSetInfo[i].bSelected)
			{
				DrawEnglishCur(Hdc,20,i);
			}
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			memset(pDateBuf, ' ',2);	
			strcpy(&pDateBuf[2],GetHebrewStr(g_ManOpenSetInfo[i].HebrewStrID));					
			MXDrawText_Hebrew(Hdc, &Rect, pDateBuf, i);
			if (g_ManOpenSetInfo[i].bSelected)
			{
				DrawHebrewCur(Hdc,108,i);
			}
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
/*
			if (g_ManOpenSetInfo[i].bSelected)
			{
				memset(pDateBuf, '*',1);
			}
			else
			{
				memset(pDateBuf, ' ',1);
			}
*/
			memset(pDateBuf, ' ',2);			
			strcpy(&pDateBuf[2],g_ManOpenSetInfo[i].MenuChineseName);								
			MXDrawText_Cn(Hdc, &Rect, pDateBuf, i);
			if (g_ManOpenSetInfo[i].bSelected)
			{
					DrawChineseCur(Hdc,20,i);
			}
		}
	}
#else
	for(i = 0; i < MANOPENSETDEPTH; i++)
	{	
		if (g_ManOpenSetInfo[i].bSelected)
		{
			Hdc->textcolor = FONT_COLOR_SEL;
		}
		else
		{
			Hdc->textcolor = FONT_COLOR;
		}
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			MXDrawText_En(Hdc, &Rect, g_ManOpenSetInfo[i].MenuEnglishName, i);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			MXDrawText_Cn(Hdc, &Rect, g_ManOpenSetInfo[i].MenuChineseName, i);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			MXDrawText_Hebrew(Hdc, &Rect, GetHebrewStr(g_ManOpenSetInfo[i].HebrewStrID), i);
		}
	}
#endif
	
	EndPaint(hWnd, &ps);	
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenDelaySetWndProc
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
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
ManOpenSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);	
		break;
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_MAN_OPEN_SET, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			if (g_ASPara.GateOpenManFlag) 
			{
				strcpy(g_ManOpenSetInfo[0].MenuChineseName, "开启");
				strcpy(g_ManOpenSetInfo[0].MenuEnglishName, "ON");
			}
			else
			{
				strcpy(g_ManOpenSetInfo[0].MenuChineseName, "关闭");
				strcpy(g_ManOpenSetInfo[0].MenuEnglishName, "OFF");
			}
			
			if (bShowPromptInfo) 
			{
				ShowManOpenSetPromptInfo(hWnd, Msg, wParam, lParam);
			}
			else
			{
				ManOpenSetWndPaint(hWnd, Msg, wParam, lParam);
			}
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
		ResetTimer(hWnd, TIMER_MAN_OPEN_SET, INTERFACE_SETTLE_TIME, NULL);

		wParam =  KeyBoardMap(wParam);
		
		if (DEVICE_CORAL_DB == g_DevFun.DevType) 
		{
			if (MENU_INPUT_PASSWORD == GateManOpenStep) 
			{
				DBPasswordInputProc(hWnd, Msg, wParam, lParam);
			}
			else if (MENU_ENTER_MENU == GateManOpenStep)
			{
				DBManOpenSetKeyProc(hWnd, Msg, wParam, lParam);
			}
		}
		else
		{
			GMManOpenSetKeyProcess(hWnd, Msg, wParam, lParam);
		}

		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_MAN_OPEN_SET);

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
		GateManOpenStep	=	MENU_INPUT_PASSWORD;
		KillTimer(hWnd, TIMER_MAN_OPEN_SET);
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
**	FUNCTION NAME:	CreateGateOpenDelaySetWnd
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
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
CreateManOpenSetWnd(HWND hwndParent)
{
	static char szAppName[] = "ManOpenSetWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) ManOpenSetWndProc;
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
		"ManOpenSetWnd",			// Window name	(Not NULL)
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
**	FUNCTION NAME:	GateOpenDelaySetKeyProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		22 - Aug - 2008
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
GMManOpenSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int i = 0;
	
	switch(wParam) 
	{
	case KEY_RETURN:
		{
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		break;
	case KEY_ENTER:
		{
			for (i = 0; i < MANOPENSETDEPTH; i++)
			{
				if (g_ManOpenSetInfo[i].bSelected) 
				{
					if(g_ManOpenSetInfo[i].Menufun)
					{
						g_ManOpenSetInfo[i].Menufun();					
					}
					break;
				}
			}
		}
		break;
	case KEY_UP:
		{
			for (i = 0; i < MANOPENSETDEPTH; i++)
			{
				if ( g_ManOpenSetInfo[i].bSelected ) 
				{
					if (0 == i) 
					{
						g_ManOpenSetInfo[i].bSelected = FALSE;
						g_ManOpenSetInfo[MANOPENSETDEPTH-1].bSelected = TRUE;
					} 
					else
					{
						g_ManOpenSetInfo[i].bSelected = FALSE;
						g_ManOpenSetInfo[i-1].bSelected = TRUE;
					}
					break;
				}
			}
		}
		
		break;
	case KEY_DOWN:
		{
			for (i = 0; i < MANOPENSETDEPTH; i++)
			{
				if ( g_ManOpenSetInfo[i].bSelected ) 
				{
					if (MANOPENSETDEPTH-1 == i) 
					{
						g_ManOpenSetInfo[i].bSelected = FALSE;
						g_ManOpenSetInfo[0].bSelected = TRUE;
					} 
					else
					{
						g_ManOpenSetInfo[i].bSelected = FALSE;
						g_ManOpenSetInfo[i+1].bSelected = TRUE;
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
**	FUNCTION NAME:	ShowGateOpenDelaySetPromptInfo
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
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
ShowManOpenSetPromptInfo(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	

	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;		
	
	if (SET_CHINESE == g_SysConfig.LangSel) 
	{
		if (g_ASPara.GateOpenManFlag) 
		{
			MXDrawText_Cn(Hdc, &Rect, "手动开关已启用", 1);
		}
		else
		{
			MXDrawText_Cn(Hdc, &Rect, "手动开关已关闭", 1);
		}		
	}
	else if (SET_HEBREW == g_SysConfig.LangSel) 
	{
		
		if (g_ASPara.GateOpenManFlag) 
		{
			MXDrawText_Center(Hdc, GetHebrewStr( HS_ID_MANOPENENABLED), 1);	
		}
		else
		{
			MXDrawText_Center(Hdc, GetHebrewStr( HS_ID_MANOPENDISABLED), 1);	
		}	
	}	
	else if (SET_ENGLISH == g_SysConfig.LangSel) 
	{		
		if (g_ASPara.GateOpenManFlag) 
		{
			MXDrawText_En(Hdc, &Rect, "Manual Switch", 1);
			MXDrawText_En(Hdc, &Rect, "is enabled", 2);
		}
		else
		{
			MXDrawText_En(Hdc, &Rect, "Manual Switch", 1);
			MXDrawText_En(Hdc, &Rect, "is disabled", 2);
		}	
	}
	

	ResetTimer(hWnd, TIMER_MAN_OPEN_SET, PROMPT_SHOW_TIME, NULL);

	EndPaint(hWnd, &ps);
}

static VOID 
ManOpenSetProc()
{
	g_ASPara.GateOpenManFlag = !g_ASPara.GateOpenManFlag;
	bShowPromptInfo = TRUE;
	SaveMenuPara2Mem();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DBManOpenSetKeyProc
**	AUTHOR:		   Jeff Wang
**	DATE:		07 - Nov - 2008
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
DBManOpenSetKeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{	
	switch(wParam) 
	{
	case KEY_NUM_1:
		{			
			g_ASPara.GateOpenManFlag = TRUE;
			SaveMenuPara2Mem();
			StartPlayRightANote();
		}		
		break;
	case KEY_NUM_2:
		{
			g_ASPara.GateOpenManFlag = FALSE;
			SaveMenuPara2Mem();
			StartPlayRightANote();
		}
		break;

	default:		
		break;
	}
	PostMessage(hWnd,  WM_CLOSE, 0, 0);
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DBPasswordInputProc
**	AUTHOR:		   Jeff Wang
**	DATE:		07 - Nov - 2008
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
DBPasswordInputProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == KEY_RETURN)
	{
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
			g_KeyInputBufferLen		=	0;
			g_KeyInputBuffer[g_KeyInputBufferLen++] = (UCHAR)wParam;
		}
	}
	else if (wParam == KEY_ENTER)
	{	
		if (g_KeyInputBufferLen < 4 || g_KeyInputBufferLen > 12) 
		{
			StartPlayErrorANote();
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		else
		{
			if (CodeCompare(g_KeyInputBuffer, g_SysConfig.SysPwd, g_KeyInputBufferLen)) 
			{
				GateManOpenStep = MENU_ENTER_MENU;
				StartPlayRightANote();
			}
			else
			{
				StartPlayErrorANote();
				PostMessage(hWnd,  WM_CLOSE, 0, 0);
			}	
		}		
	}
}
