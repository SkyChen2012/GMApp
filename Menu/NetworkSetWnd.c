/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	NetworkSetWnd.c
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
#include "NetworkSetWnd.h"

/************** DEFINES **************************************************************/

#define NETWORKSETDEPTH 2

#define MAX_NETSTR_COUNT	15
#define MAX_NETSTR_LEN		16

#define MENU_IPSET_EN		"IP"
#define MENU_IPSET_CN		"IP地址"
#define MENU_NETMASKSET_EN	"NetMask"
#define MENU_NETMASKSET_CN	"子网掩码"

/************** TYPEDEFS *************************************************************/

typedef enum _NETWORKSETSTEP
{
	STEP_MAIN_WND = 1,
	STEP_IP_SET,
	STEP_IP_SET_PROMPT,
	STEP_NETMASK_SET,
	STEP_NETMASK_SET_PROMPT,
	STEP_IP_SET_INVALID,
}NETWORKSETSTEP;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/
static BOOL CheckValidIP(const char *str);
static void		NetworkSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK NetworkSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		NetworkSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static BYTE IPIndex = 0;
static BYTE NetMaskIndex = 0;
static BYTE TempIPaddr[MAX_NETSTR_LEN] = { 0 };
static BYTE TempNetMask[MAX_NETSTR_LEN] = { 0 };

static VOID IPSet();
static VOID NetMaskSet();

static DWORD	Net_Str_2_DWORD(BYTE* pStr);
static VOID		Net_DWORD_2_Str(BYTE* pStr, DWORD dwInData);

static MENUINFO g_NeworkSetInfo[NETWORKSETDEPTH] = 
{
	{TRUE,  HS_ID_IP,MENU_IPSET_EN,	MENU_IPSET_CN,			IPSet},
	{FALSE,  HS_ID_NETMASK,MENU_NETMASKSET_EN,	MENU_NETMASKSET_CN,			NetMaskSet}
};

static NETWORKSETSTEP NetworkSetStep = STEP_MAIN_WND;

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IPSetWndPaint
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
NetworkSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	BYTE IPNum[12] = { 0 };	
	BYTE nTemp			=	0;
	INT		i =	0;
	INT			xPos = 0;
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);	
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;
	
	switch(NetworkSetStep)
	{
	case STEP_MAIN_WND:
		{
#ifdef NEW_OLED_ENABLE		
			for(i = 0; i < NETWORKSETDEPTH; i++)
			{	
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{	
					xPos = GetXPos(MENU_NETMASKSET_EN);
/*
					if (g_NeworkSetInfo[i].bSelected)
					{
						memset(pDateBuf, '*',1);
					}
					else
					{
						memset(pDateBuf, ' ',1);
					}
*/
					memset(pDateBuf, ' ',2);							
					strcpy(&pDateBuf[2],g_NeworkSetInfo[i].MenuEnglishName);					
					MXDrawText_Left(Hdc, pDateBuf, xPos, i+1);
					if (g_NeworkSetInfo[i].bSelected)
					{
						DrawEnglishCur(Hdc,xPos,i+1);
					}
					
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_NETMASK));

					memset(pDateBuf, ' ',2);	
					strcpy(&pDateBuf[2],GetHebrewStr(g_NeworkSetInfo[i].HebrewStrID)	);								
					MXDrawText_Right(Hdc, pDateBuf, xPos, i +1);
					if (g_NeworkSetInfo[i].bSelected)
					{
						DrawHebrewCur(Hdc,xPos,i+1);
					}
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{	
					xPos = GetXPos(MENU_NETMASKSET_CN);
/*
					if (g_NeworkSetInfo[i].bSelected)
					{
						memset(pDateBuf, '*',1);
					}
					else
					{
						memset(pDateBuf, ' ',1);
					}
*/
					memset(pDateBuf, ' ',2);			
					strcpy(&pDateBuf[2],g_NeworkSetInfo[i].MenuChineseName);					
					MXDrawText_Left(Hdc, pDateBuf, xPos, i+1);
					if (g_NeworkSetInfo[i].bSelected)
					{
						DrawChineseCur(Hdc,xPos,i+1);
					}
				}
			}	
#else
			for(i = 0; i < NETWORKSETDEPTH; i++)
			{	
				if (g_NeworkSetInfo[i].bSelected)
				{
					Hdc->textcolor = FONT_COLOR_SEL;			
				}
				else
				{
					Hdc->textcolor = FONT_COLOR;				
				}
				if (SET_ENGLISH == g_SysConfig.LangSel) 
				{	
					xPos = GetXPos(MENU_NETMASKSET_EN);
					MXDrawText_Left(Hdc, g_NeworkSetInfo[i].MenuEnglishName, xPos, i+2);
				}
				else if (SET_CHINESE == g_SysConfig.LangSel) 
				{	
					xPos = GetXPos(MENU_NETMASKSET_CN);
					MXDrawText_Left(Hdc, g_NeworkSetInfo[i].MenuChineseName, xPos, i+2);
				}
				else if (SET_HEBREW == g_SysConfig.LangSel) 
				{
					xPos = HebrewGetXPos(Hdc, GetHebrewStr(HS_ID_NETMASK));
					MXDrawText_Right(Hdc, GetHebrewStr(g_NeworkSetInfo[i].HebrewStrID), xPos, i+2);
				}
			}	
#endif
		}
		break;
		
	case STEP_IP_SET:
		{	
			DrawDataParaSet(Hdc, TempIPaddr, IPIndex);	
		}
		break;
		
	case STEP_IP_SET_PROMPT:
		{
			if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos("IP已修改");
				MXDrawText_Left(Hdc, "IP已修改", xPos, 1);
				MXDrawText_Left(Hdc, "请重启", xPos, 2);
			}
			else if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos("IP Modified");
				MXDrawText_Left(Hdc, "IP Modified", xPos, 1);
				MXDrawText_Left(Hdc, "Please Restart", xPos, 2);			
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_IPMODIFYED));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_IPMODIFYED), xPos, 1);
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PLEASERESTART), xPos, 2);			
			}
		}
		break;
		
	case STEP_NETMASK_SET:
		{
			DrawDataParaSet(Hdc, TempNetMask, NetMaskIndex);	
		}
		break;
	case STEP_IP_SET_INVALID:
        {
            if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos("  无效IP");
				MXDrawText_Left(Hdc, "  无效IP", xPos, 1);
			}
			else if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos("  Invalid IP");
				MXDrawText_Left(Hdc, "  Invalid IP", xPos, 1);			
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
                xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_INVALID_IP));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_INVALID_IP), xPos, 1);	
			}
            break;
	    }
	case STEP_NETMASK_SET_PROMPT:
		{
			if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos("子网掩码已修改");
				MXDrawText_Left(Hdc, "子网掩码已修改", xPos, 1);
				MXDrawText_Left(Hdc, "请重启", xPos, 2);
			}
			else if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos("NetMask Modified");
				MXDrawText_Left(Hdc, "NetMask Modified", xPos, 1);
				MXDrawText_Left(Hdc, "Please Restart", xPos, 2);			
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_NETMASKMODIFYED));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_NETMASKMODIFYED), xPos, 1);
				MXDrawText_Right(Hdc, GetHebrewStr(HS_ID_PLEASERESTART), xPos, 2);			
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
**	FUNCTION NAME:	NetworkSetWndProc
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
NetworkSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		IPIndex = 0;
		NetMaskIndex = 0;
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_NETWORKSET_WND, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			NetworkSetWndPaint(hWnd, Msg, wParam, lParam);		
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
		if (STEP_MAIN_WND	 == NetworkSetStep
			|| STEP_IP_SET		== NetworkSetStep
			|| STEP_NETMASK_SET == NetworkSetStep) 
		{
			NetworkSetKeyProcess(hWnd, Msg, wParam, lParam);
//			ResetTimer(hWnd, TIMER_NETWORKSET_WND, INTERFACE_SETTLE_TIME, NULL);		
//			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_NETWORKSET_WND);
		if (STEP_MAIN_WND	 == NetworkSetStep
			|| STEP_IP_SET		== NetworkSetStep
			|| STEP_NETMASK_SET == NetworkSetStep) 
		{
			KillAllChildWnd(hWnd);		
		}
        if(STEP_IP_SET_INVALID == NetworkSetStep)
        {
            NetworkSetStep = STEP_IP_SET;
            PostMessage(hWnd,  WM_PAINT, 0, 0);
        }
		else
		{
			NetworkSetStep = STEP_MAIN_WND;
			ResetTimer(hWnd, TIMER_NETWORKSET_WND, INTERFACE_SETTLE_TIME, NULL);		
			PostMessage(hWnd,  WM_PAINT, 0, 0);
//			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_NETWORKSET_WND);
		memset(TempIPaddr, 0, MAX_NETSTR_LEN);
		memset(TempNetMask, 0, MAX_NETSTR_LEN);
		NetworkSetStep = STEP_MAIN_WND;		
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
		
		//		return DefWindowProc(hWnd, Msg, wParam, lParam);
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
CreateNetworkSetWnd(HWND hwndParent)
{
	static char szAppName[] = "NetworkSet";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) NetworkSetWndProc;
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
		"NetworkSetWnd",			// Window name	(Not NULL)
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
**	FUNCTION NAME:	IPSetKeyProcess
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
NetworkSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BYTE	TempNum[3]	= { 0 };
	WORD	Temp		=	0; 
	INT		i	=	0;

	wParam =  KeyBoardMap(wParam);

	if (wParam == KEY_RETURN) 
	{
		if(STEP_MAIN_WND == NetworkSetStep)
		{
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		else
		{
			NetworkSetStep = STEP_MAIN_WND;
			ResetTimer(hWnd, TIMER_NETWORKSET_WND, INTERFACE_SETTLE_TIME, NULL);		
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}

		return;
			
	}

	if (STEP_MAIN_WND == NetworkSetStep)
	{
		switch(wParam) 
		{
		case KEY_ENTER:
			{
				for (i = 0; i < NETWORKSETDEPTH; i++)
				{
					if (g_NeworkSetInfo[i].bSelected) 
					{
						if(g_NeworkSetInfo[i].Menufun)
						{
							g_NeworkSetInfo[i].Menufun();					
						}
						break;
					}
				}
			}
			break;
		case KEY_UP:
			{
				for (i = 0; i < NETWORKSETDEPTH; i++)
				{
					if ( g_NeworkSetInfo[i].bSelected ) 
					{
						if (0 == i) 
						{
							g_NeworkSetInfo[i].bSelected = FALSE;
							g_NeworkSetInfo[NETWORKSETDEPTH-1].bSelected = TRUE;
						} 
						else
						{
							g_NeworkSetInfo[i].bSelected = FALSE;
							g_NeworkSetInfo[i-1].bSelected = TRUE;
						}
						break;
					}
				}
			}
			
			break;
		case KEY_DOWN:
			{
				for (i = 0; i < NETWORKSETDEPTH; i++)
				{
					if ( g_NeworkSetInfo[i].bSelected ) 
					{
						if (NETWORKSETDEPTH-1 == i) 
						{
							g_NeworkSetInfo[i].bSelected = FALSE;
							g_NeworkSetInfo[0].bSelected = TRUE;
						} 
						else
						{
							g_NeworkSetInfo[i].bSelected = FALSE;
							g_NeworkSetInfo[i+1].bSelected = TRUE;
						}
						break;
					}
				}
			}
			break;
		default:
			break;
		}

		ResetTimer(hWnd, TIMER_NETWORKSET_WND, INTERFACE_SETTLE_TIME, NULL);		
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		
	}
	else if (STEP_IP_SET == NetworkSetStep) 
	{
		if (KEY_ENTER == wParam)
		{
            if(CheckValidIP(TempIPaddr))
            {
                printf("IP is valid!\n");
                g_SysConfig.IPAddr = Net_Str_2_DWORD(TempIPaddr);
                SaveMenuPara2Mem();
			    NetworkSetStep = STEP_IP_SET_PROMPT;
            }
            else
            {
                printf("IP is invalid!\n");
                NetworkSetStep = STEP_IP_SET_INVALID;
            }
		    ResetTimer(hWnd, TIMER_NETWORKSET_WND, PROMPT_SHOW_TIME, NULL);	
			PostMessage(hWnd,  WM_PAINT, 0, 0);			
		}
		else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9))	
		{
			if (IPIndex >= 0 && IPIndex <= 2)
			{
				memcpy(&TempNum[0], &TempIPaddr[0], 3);
				TempNum[IPIndex] = (BYTE)wParam;
				Temp = 100 * (TempNum[0]-0x30) + 10 * (TempNum[1] - 0x30) + (TempNum[2] - 0x30);
				if (Temp <= 254) 
				{
					TempIPaddr[IPIndex] = (BYTE)wParam;
				}
				else
				{
					TempIPaddr[0] = 0x32;
					TempIPaddr[1] = 0x35;
					TempIPaddr[2] = 0x34;
				}
			}
			else if (IPIndex >=4 && IPIndex <= 6)
			{
				memcpy(&TempNum[0], &TempIPaddr[4], 3);
				TempNum[IPIndex - 4] = (BYTE)wParam;
				Temp = 100 * (TempNum[0]-0x30) + 10 * (TempNum[1] - 0x30) + (TempNum[2] - 0x30);
				if (Temp <= 254) 
				{
					TempIPaddr[IPIndex] = (BYTE)wParam;
				}
				else
				{
					TempIPaddr[4] = 0x32;
					TempIPaddr[5] = 0x35;
					TempIPaddr[6] = 0x34;
				}
			}
			else if (IPIndex >=8 && IPIndex <= 10)
			{
				memcpy(&TempNum[0], &TempIPaddr[8], 3);
				TempNum[IPIndex - 8] = (BYTE)wParam;
				Temp = 100 * (TempNum[0]-0x30) + 10 * (TempNum[1] - 0x30) + (TempNum[2] - 0x30);
				if (Temp <= 254) 
				{
					TempIPaddr[IPIndex] = (BYTE)wParam;
				}
				else
				{
					TempIPaddr[8] = 0x32;
					TempIPaddr[9] = 0x35;
					TempIPaddr[10] = 0x34;
				}
			}
			else if (IPIndex >=12 && IPIndex <= 14)
			{
				memcpy(&TempNum[0], &TempIPaddr[12], 3);
				TempNum[IPIndex - 12] = (BYTE)wParam;
				Temp = 100 * (TempNum[0]-0x30) + 10 * (TempNum[1] - 0x30) + (TempNum[2] - 0x30);
				if (Temp <= 254) 
				{
					TempIPaddr[IPIndex] = (BYTE)wParam;
				}
				else
				{
					TempIPaddr[12] = 0x32;
					TempIPaddr[13] = 0x35;
					TempIPaddr[14] = 0x34;
				}
			}
			IPIndex++;
			if (MAX_NETSTR_COUNT == IPIndex)
			{
				IPIndex = 0;
			}
			else if (3 == IPIndex 
					 || 7  == IPIndex
					 || 11 == IPIndex) 
			{
				IPIndex++;
			}
			ResetTimer(hWnd, TIMER_NETWORKSET_WND, INTERFACE_SETTLE_TIME, NULL);		
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		
	}
	else if (STEP_NETMASK_SET == NetworkSetStep) 
	{
		if (KEY_ENTER == wParam)
		{
//			g_SysConfig.NetMask = TempNetMask;
			g_SysConfig.NetMask = Net_Str_2_DWORD(TempNetMask);
			
			SaveMenuPara2Mem();
			NetworkSetStep = STEP_NETMASK_SET_PROMPT;
			ResetTimer(hWnd, TIMER_NETWORKSET_WND, PROMPT_SHOW_TIME, NULL);			
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		else if (wParam >= KEY_NUM_0 && wParam <= KEY_NUM_9)	
		{	
			if (NetMaskIndex >= 0 && NetMaskIndex <= 2)
			{
				memcpy(&TempNum[0], &TempNetMask[0], 3);
				TempNum[NetMaskIndex] = (BYTE)wParam;
				Temp = 100 * (TempNum[0]-0x30) + 10 * (TempNum[1] - 0x30) + (TempNum[2] - 0x30);
				if (Temp <= 255) 
				{
					TempNetMask[NetMaskIndex] = (BYTE)wParam;
				}
				else
				{
					TempNetMask[0] = 0x32;
					TempNetMask[1] = 0x35;
					TempNetMask[2] = 0x35;
				}
			}
			else if (NetMaskIndex >=4 && NetMaskIndex <= 6)
			{
				memcpy(&TempNum[0], &TempNetMask[4], 3);
				TempNum[NetMaskIndex - 4] = (BYTE)wParam;
				Temp = 100 * (TempNum[0]-0x30) + 10 * (TempNum[1] - 0x30) + (TempNum[2] - 0x30);
				if (Temp <= 255) 
				{
					TempNetMask[NetMaskIndex] = (BYTE)wParam;
				}
				else
				{
					TempNetMask[4] = 0x32;
					TempNetMask[5] = 0x35;
					TempNetMask[6] = 0x35;
				}
			}
			else if (NetMaskIndex >=8 && NetMaskIndex <= 10)
			{
				memcpy(&TempNum[0], &TempNetMask[8], 3);
				TempNum[NetMaskIndex - 8] = (BYTE)wParam;
				Temp = 100 * (TempNum[0]-0x30) + 10 * (TempNum[1] - 0x30) + (TempNum[2] - 0x30);
				if (Temp <= 255) 
				{
					TempNetMask[NetMaskIndex] = (BYTE)wParam;
				}
				else
				{
					TempNetMask[8] = 0x32;
					TempNetMask[9] = 0x35;
					TempNetMask[10] = 0x35;
				}
			}
			else if (NetMaskIndex >=12 && NetMaskIndex <= 14)
			{
				memcpy(&TempNum[0], &TempNetMask[12], 3);
				TempNum[NetMaskIndex - 12] = (BYTE)wParam;
				Temp = 100 * (TempNum[0]-0x30) + 10 * (TempNum[1] - 0x30) + (TempNum[2] - 0x30);

				if (Temp <= 255) 
				{
					TempNetMask[NetMaskIndex] = (BYTE)wParam;
				}
				else
				{
					TempNetMask[12] = 0x32;
					TempNetMask[13] = 0x35;
					TempNetMask[14] = 0x35;
				}
			}

			NetMaskIndex++;
			if (MAX_NETSTR_COUNT == NetMaskIndex)
			{
				NetMaskIndex = 0;
			}
			else if (3 == NetMaskIndex
				|| 7  == NetMaskIndex
				|| 11 == NetMaskIndex)
			{
				NetMaskIndex++;
			}
			ResetTimer(hWnd, TIMER_NETWORKSET_WND, INTERFACE_SETTLE_TIME, NULL);		
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
	}
}


static VOID 
IPSet()
{
	NetworkSetStep	= STEP_IP_SET;
	Net_DWORD_2_Str(TempIPaddr, g_SysConfig.IPAddr);
}


static VOID 
NetMaskSet()
{
	NetworkSetStep	= STEP_NETMASK_SET;
	Net_DWORD_2_Str(TempNetMask, g_SysConfig.NetMask);
}

static VOID
Net_DWORD_2_Str(BYTE* pStr, DWORD dwInData)
{
	BYTE NetNum[4]	=	{ 0 };
	INT	 OutNum[12]	=	{ 0 };

	memcpy(NetNum, &dwInData, sizeof(DWORD));

	OutNum[0] = (INT)(NetNum[3] / 100);
	OutNum[1] = (INT)((NetNum[3] % 100) / 10);
	OutNum[2] = (INT)((NetNum[3] % 100) % 10);
	OutNum[3] = (INT)(NetNum[2] / 100);
	OutNum[4] = (INT)((NetNum[2] % 100) / 10);
	OutNum[5] = (INT)((NetNum[2] % 100) % 10);
	OutNum[6] = (INT)(NetNum[1] / 100);
	OutNum[7] = (INT)((NetNum[1] % 100) / 10);
	OutNum[8] = (INT)((NetNum[1] % 100) % 10);
	OutNum[9] =	(INT)(NetNum[0] / 100);
	OutNum[10] = (INT)((NetNum[0] % 100) / 10);
	OutNum[11] = (INT)((NetNum[0] % 100) % 10);
	
	sprintf(pStr, "%d%d%d.%d%d%d.%d%d%d.%d%d%d",  
		OutNum[0] ,OutNum[1] ,OutNum[2] ,OutNum[3] ,OutNum[4] ,OutNum[5],
		OutNum[6] ,OutNum[7] ,OutNum[8] ,OutNum[9] ,OutNum[10] ,OutNum[11]);
}


static DWORD
Net_Str_2_DWORD(BYTE* pStr)
{
	INT		NetNum[4]	=	{ 0 };
		
	sscanf(pStr, "%d.%d.%d.%d",  
		&NetNum[0],&NetNum[1],&NetNum[2],&NetNum[3]);
	return MAKEDWORD(NetNum[0], NetNum[1], NetNum[2], NetNum[3]);	
}

static BOOL CheckValidIP(const char *str) 
{ 
    BOOL    bRet    = TRUE;
	int	    nIterator = 0;
	int 	bIP[4] = {0};
	unsigned long   nTemp = 0;
    unsigned long   nInAddr = 0;
	nTemp = Net_Str_2_DWORD(str);
	if (nTemp == -1)
	{
		bRet = FALSE;
		return bRet;
	}			
	
	nInAddr = ntohl(nTemp);
	bIP[0] = (unsigned short) ((nInAddr >> 24) & 0xFF);
	bIP[1] = (unsigned short) ((nInAddr >> 16) & 0xFF);
	bIP[2] = (unsigned short) ((nInAddr >> 8) & 0xFF);
	bIP[3] = (unsigned short) (nInAddr & 0xFF);
    printf(" the ip =  %d.%d.%d.%d.\n", bIP[0], bIP[1], bIP[2], bIP[3]);	
	for (nIterator = 0; nIterator < 3; nIterator++)
	{
		if (bIP[nIterator] > 255)
		{
			bRet = FALSE;
			break;
		}
	}
	if (!bIP[3] || !bIP[0])
	{
		bRet = FALSE;
	}
	return bRet;
}


