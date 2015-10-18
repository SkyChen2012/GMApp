/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	GateOpenModeSetWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		24 - Aug - 2008
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

#include "GateOpenModeSetWnd.h"
#include "CardProc.h"
#include "AccessProc.h"

/************** DEFINES **************************************************************/

#define GATEOPENMODESETDEPTH  3

#define CANCEL_ALL_MODE			0x00
#define CDONLY_MODE				0x02
#define CDPWD_MODE				0x04
#define RDPWD_MODE				0x01
#define CDONLY_AND_RDPWD_MODE	0x03
#define RDPWD_AND_RDPWD_MODE	0x05

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		GateOpenModeSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK GateOpenModeSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		GMGateOpenModeSetKeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		DBGateOpenModeSetKeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID		DBPasswordInputProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static VOID RoomCodePassword();
static VOID CardPassword();
static VOID CardOnly();

static MENUINFO g_GateOpenModeSetInfo[GATEOPENMODESETDEPTH] = 
{
	{TRUE, HS_ID_RESCODEPSW, "Resident No. +PWD", "·¿¼äºÅ+ÃÜÂë",	RoomCodePassword},
	{FALSE, HS_ID_CARDPSW,"Card+PWD",	  "¿¨+ÃÜÂë",		CardPassword},
	{FALSE, HS_ID_CARD,"Card",			  "¿¨",			CardOnly}
};

static MENUENTERSTEP GateOpenModeStep = MENU_INPUT_PASSWORD;

static BOOL bKillALL = FALSE;

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
GateOpenModeSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	int								 i			= 0;
	char	     				   pDateBuf[TITLE_BUF_LEN] = {0};
	INT			xPos = 0;
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);	
	Hdc->bkcolor = BACKGROUND_COLOR;	

	
#ifdef NEW_OLED_ENABLE	
	for(i = 0; i < GATEOPENMODESETDEPTH; i++)
	{		
/*
		if (g_GateOpenModeSetInfo[i].bSelected)
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
			strcpy(&pDateBuf[2], g_GateOpenModeSetInfo[i].MenuEnglishName);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			strcpy(&pDateBuf[2],  GetHebrewStr(g_GateOpenModeSetInfo[i].HebrewStrID));
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			strcpy(&pDateBuf[2], g_GateOpenModeSetInfo[i].MenuChineseName);
		}
		
		if((MODE_ROOMCODE_PASSWORD_SET & g_ASPara.ASOpenMode)&&(0 == i))
		{
			strcat(pDateBuf, "*");
		}
		
		if ((MODE_CARD_PASSWORD_SET & g_ASPara.ASOpenMode)&&(1 == i)) 
		{
			strcat(pDateBuf, "*");		
		}
		
		if((MODE_CARD_ONLY_SET & g_ASPara.ASOpenMode)&&(2 == i))
		{
			strcat(pDateBuf, "*");
		}	
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  Resident No. +PWD");
			
//			MXDrawText_En(Hdc, &Rect, pDateBuf, i);
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			
			if (g_GateOpenModeSetInfo[i].bSelected)
			{
				DrawEnglishCur(Hdc,2,i);
			}
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
		
		printf("SET_HEBREW =%d\n",i);
			xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_RESCODEPSW));
			
//			MXDrawText_En(Hdc, &Rect, pDateBuf, i);
			MXDrawText_Right(Hdc, pDateBuf, xPos, i);
			
			if (g_GateOpenModeSetInfo[i].bSelected)
			{
				DrawHebrewCur(Hdc,xPos,i);
			}
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			xPos = GetXPos("  ·¿¼äºÅ+ÃÜÂë");

//			MXDrawText_Cn(Hdc, &Rect, pDateBuf, i);
			MXDrawText_Left(Hdc, pDateBuf, xPos, i);
			
			if (g_GateOpenModeSetInfo[i].bSelected)
			{
				DrawChineseCur(Hdc,20,i);
			}
		}
		memset(pDateBuf, 0, TITLE_BUF_LEN);
	}
#else
	for(i = 0; i < GATEOPENMODESETDEPTH; i++)
	{		
		if (g_GateOpenModeSetInfo[i].bSelected)
		{
			Hdc->textcolor = FONT_COLOR_SEL;			
		}
		else
		{
			Hdc->textcolor = FONT_COLOR;				
		}
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			strcpy(pDateBuf, g_GateOpenModeSetInfo[i].MenuEnglishName);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			strcpy(pDateBuf,  GetHebrewStr(g_GateOpenModeSetInfo[i].HebrewStrID));
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			strcpy(pDateBuf, g_GateOpenModeSetInfo[i].MenuChineseName);
		}
		
		if((MODE_ROOMCODE_PASSWORD_SET & g_ASPara.ASOpenMode)&&(0 == i))
		{
			strcat(pDateBuf, "*");
		}
		
		if ((MODE_CARD_PASSWORD_SET & g_ASPara.ASOpenMode)&&(1 == i)) 
		{
			strcat(pDateBuf, "*");		
		}
		
		if((MODE_CARD_ONLY_SET & g_ASPara.ASOpenMode)&&(2 == i))
		{
			strcat(pDateBuf, "*");
		}	
		
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			MXDrawText_En(Hdc, &Rect, pDateBuf, i);
		}
		else if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			MXDrawText_Cn(Hdc, &Rect, pDateBuf, i);
		}
		else if (SET_HEBREW == g_SysConfig.LangSel) 
		{
			MXDrawText_Hebrew(Hdc, &Rect, pDateBuf, i);
		}
		
		memset(pDateBuf, 0, TITLE_BUF_LEN);
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
GateOpenModeSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:	
		bKillALL = FALSE;
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);	
		break;
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_OPEN_MODE_SET, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			GateOpenModeSetWndPaint(hWnd, Msg, wParam, lParam);
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
		ResetTimer(hWnd, TIMER_OPEN_MODE_SET, INTERFACE_SETTLE_TIME, NULL);	

		wParam =  KeyBoardMap(wParam);

		if (DEVICE_CORAL_DB == g_DevFun.DevType) 
		{
			if (MENU_INPUT_PASSWORD == GateOpenModeStep) 
			{
				DBPasswordInputProc(hWnd, Msg, wParam, lParam);
			}
			else if (MENU_ENTER_MENU == GateOpenModeStep)
			{
				DBGateOpenModeSetKeyProc(hWnd, Msg, wParam, lParam);
			}
		}
		else
		{
			GMGateOpenModeSetKeyProc(hWnd, Msg, wParam, lParam);
		}
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		SaveMenuPara2Mem();
		KillTimer(hWnd, TIMER_OPEN_MODE_SET);
		KillAllChildWnd(hWnd);
		bKillALL = TRUE;		
//		PostMessage(hWnd,  WM_CLOSE, 0, 0);
		break;
		
	case WM_DESTROY:
		GateOpenModeStep = MENU_INPUT_PASSWORD;
		KillTimer(hWnd, TIMER_OPEN_MODE_SET);
		if (!bKillALL) 
		{
			ClearKeyBuffer();
//			PostMessage(GetParent(hWnd),  WM_REFRESHPARENT_WND, 0, 0);
//			SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		}
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
CreateGateOpenModeSetWnd(HWND hwndParent)
{
	static char szAppName[] = "GateOpenModeSetWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) GateOpenModeSetWndProc;
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
		"GateOpenModeSetWnd",			// Window name	(Not NULL)
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
**	FUNCTION NAME:	GMGateOpenModeSetKeyProc
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
GMGateOpenModeSetKeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int i = 0;
	
	switch(wParam) 
	{
	case KEY_RETURN:
		{
			SaveMenuPara2Mem();
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		break;
	case KEY_ENTER:
		{
			for (i = 0; i < GATEOPENMODESETDEPTH; i++)
			{
				if (g_GateOpenModeSetInfo[i].bSelected) 
				{
					if(g_GateOpenModeSetInfo[i].Menufun)
					{
						g_GateOpenModeSetInfo[i].Menufun();	
						PostMessage(hWnd,  WM_PAINT, 0, 0);
					}
					break;
				}
			}
		}
		break;
	case KEY_UP:
		{
			for (i = 0; i < GATEOPENMODESETDEPTH; i++)
			{
				if ( g_GateOpenModeSetInfo[i].bSelected ) 
				{
					if (0 == i) 
					{
						g_GateOpenModeSetInfo[i].bSelected = FALSE;
						g_GateOpenModeSetInfo[GATEOPENMODESETDEPTH-1].bSelected = TRUE;
					} 
					else
					{
						g_GateOpenModeSetInfo[i].bSelected = FALSE;
						g_GateOpenModeSetInfo[i-1].bSelected = TRUE;
					}
					break;
				}
			}
		}
		
		break;
	case KEY_DOWN:
		{
			for (i = 0; i < GATEOPENMODESETDEPTH; i++)
			{
				if ( g_GateOpenModeSetInfo[i].bSelected ) 
				{
					if (GATEOPENMODESETDEPTH-1 == i) 
					{
						g_GateOpenModeSetInfo[i].bSelected = FALSE;
						g_GateOpenModeSetInfo[0].bSelected = TRUE;
					} 
					else
					{
						g_GateOpenModeSetInfo[i].bSelected = FALSE;
						g_GateOpenModeSetInfo[i+1].bSelected = TRUE;
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
**	FUNCTION NAME:	RoomCodePassword
**	AUTHOR:		   Jeff Wang
**	DATE:		23 - Oct - 2008
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

static VOID 
RoomCodePassword()
{
	if (g_ASPara.ASOpenMode & MODE_ROOMCODE_PASSWORD_SET) 
	{
		g_ASPara.ASOpenMode &= MODE_ROOMCODE_PASSWORD_CLR;
	}
	else
	{
		g_ASPara.ASOpenMode |= MODE_ROOMCODE_PASSWORD_SET;		
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CardPassword
**	AUTHOR:		   Jeff Wang
**	DATE:		23 - Oct - 2008
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

static VOID 
CardPassword()
{
	if (g_ASPara.ASOpenMode & MODE_CARD_ONLY_SET) 
	{
		g_ASPara.ASOpenMode &= MODE_CARD_ONLY_CLR;
	}

	if (g_ASPara.ASOpenMode & MODE_CARD_PASSWORD_SET) 
	{
		g_ASPara.ASOpenMode &= MODE_CARD_PASSWORD_CLR;
	}
	else
	{
		g_ASPara.ASOpenMode |= MODE_CARD_PASSWORD_SET;		
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CardOnly
**	AUTHOR:		   Jeff Wang
**	DATE:		23 - Oct - 2008
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

static VOID 
CardOnly()
{
	if (g_ASPara.ASOpenMode & MODE_CARD_PASSWORD_SET) 
	{
		g_ASPara.ASOpenMode &= MODE_CARD_PASSWORD_CLR;
	}
	
	if (g_ASPara.ASOpenMode & MODE_CARD_ONLY_SET) 
	{
		g_ASPara.ASOpenMode &= MODE_CARD_ONLY_CLR;
	}
	else
	{
		g_ASPara.ASOpenMode |= MODE_CARD_ONLY_SET;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	DBGateOpenModeSetKeyProc
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
DBGateOpenModeSetKeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) 
	{
	case KEY_NUM_1:
		{
			g_ASPara.ASOpenMode = CDONLY_MODE;
			SaveMenuPara2Mem();
			StartPlayRightANote();
		}		
		break;
	case KEY_NUM_2:
		{
			g_ASPara.ASOpenMode = CDPWD_MODE;
			SaveMenuPara2Mem();
			StartPlayRightANote();
		}
		break;
	case KEY_NUM_3:
		{
			g_ASPara.ASOpenMode = RDPWD_MODE;
			SaveMenuPara2Mem();
			StartPlayRightANote();
		}
		break;
	case KEY_NUM_4:
		{
			g_ASPara.ASOpenMode = CDONLY_AND_RDPWD_MODE;
			SaveMenuPara2Mem();
			StartPlayRightANote();
		}
		break;
	case KEY_NUM_5:
		{
			g_ASPara.ASOpenMode = RDPWD_AND_RDPWD_MODE;
			SaveMenuPara2Mem();
			StartPlayRightANote();
		}
		break;
	case KEY_NUM_6:
		{
			g_ASPara.ASOpenMode = CANCEL_ALL_MODE;
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
**	FUNCTION NAME:	DBGateOpenModeSetKeyProc
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
				GateOpenModeStep = MENU_ENTER_MENU;
				StartPlayRightANote();
			}
			else
			{
				PostMessage(hWnd,  WM_CLOSE, 0, 0);
				StartPlayErrorANote();
			}	
		}		
	}
}

