/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SysInfoWnd.c
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

#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"
#include "AccessSystemWnd.h"
#include "ModuleTalk.h"
#include "SysInfoWnd.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static void		SysInfoWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK SysInfoWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		SysInfoKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static void ConvertTime2String(char *pTimeShow);

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysInfoWndPaint
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
static void
SysInfoWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;
	char		szShow[255] = { 0 };
	char	    pDateBuf[TITLE_BUF_LEN] = {0};
	INT			xPos = 0;
	
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;
	
#ifdef NEW_OLED_ENABLE		
	if (SET_HEBREW == g_SysConfig.LangSel) 	
	{
		/*ConvertTime2String(szShow);
		MXDrawText_Center(Hdc, szShow, 0);
		xPos = HebrewGetXPos(Hdc,g_SysInfo.PN));
		sprintf(pDateBuf,"%s",g_SysInfo.PN);
		MXDrawText_Right(Hdc, pDateBuf, xPos, 1);
		
		strcpy(szShow,GetHebrewStr(HS_ID_HW))
		sprintf(pDateBuf,"%s",g_SysInfo.HardareVersion);
		strcat(szShow,pDateBuf)
		MXDrawText_Right(Hdc, szShow, xPos, 2);
		
		strcpy(szShow,GetHebrewStr(HS_ID_SW))
		sprintf(pDateBuf,"SW: %s",g_SysInfo.SoftwareVersion);
		MXDrawText_Left(Hdc, szShow, xPos, 3);*/
		ConvertTime2String(szShow);
		MXDrawText_Center(Hdc, szShow, 0);
		sprintf(pDateBuf,"%s",g_SysInfo.PN);
		xPos = HebrewGetXPos(Hdc,g_SysInfo.PN);
		MXDrawText_Right(Hdc, pDateBuf, xPos, 1);

		sprintf(pDateBuf,"HW: %s",g_SysInfo.HardareVersion);
		MXDrawText_Right(Hdc, pDateBuf, xPos, 2);
		sprintf(pDateBuf,"SW: %s",g_SysInfo.SoftwareVersion);
		MXDrawText_Right(Hdc, pDateBuf, xPos, 3);
	}
	else
	{
		ConvertTime2String(szShow);
		MXDrawText_Center(Hdc, szShow, 0);
		sprintf(pDateBuf,"%s",g_SysInfo.PN);
        if(21 == strlen(g_SysInfo.PN))
            MXDrawText_Left(Hdc, pDateBuf, 2, 1);
        else            
		    MXDrawText_Left(Hdc, pDateBuf, 10, 1);
		sprintf(pDateBuf,"HW: %s",g_SysInfo.HardareVersion);
		MXDrawText_Left(Hdc, pDateBuf, 10, 2);
		sprintf(pDateBuf,"SW: %s",g_SysInfo.SoftwareVersion);
		MXDrawText_Left(Hdc, pDateBuf, 10, 3);
	}

#else
	if (SET_CHINESE == g_SysConfig.LangSel) 
	{
		
		MXDrawText_Left(Hdc, "硬件版本号", 10, 0);
		MXDrawText_Left(Hdc, (char*)g_SysInfo.HardareVersion, 10, 1);
		MXDrawText_Left(Hdc, "软件版本号", 10, 2);
		MXDrawText_Left(Hdc, (char*)g_SysInfo.SoftwareVersion, 10, 3);
		MXDrawText_Left(Hdc, "产品编号", 10, 4);
		MXDrawText_Left(Hdc, g_SysInfo.PN, 10, 5);
	}
	else if (SET_ENGLISH == g_SysConfig.LangSel) 
	{
		MXDrawText_Left(Hdc, "Hardware", 10, 0);
		MXDrawText_Left(Hdc, (char*)g_SysInfo.HardareVersion, 10, 1);
		MXDrawText_Left(Hdc, "Software", 10, 2);
		MXDrawText_Left(Hdc,  (char*)g_SysInfo.SoftwareVersion, 10, 3);
		MXDrawText_Left(Hdc, "Product Number", 10, 4);
		MXDrawText_Left(Hdc,  g_SysInfo.PN, 10, 5);
	}
#endif
	
	

	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SysInfoWndProc
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
SysInfoWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_SYSINFO, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			SysInfoWndPaint(hWnd, Msg, wParam, lParam);
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
		ResetTimer(hWnd, TIMER_SYSINFO, INTERFACE_SETTLE_TIME, NULL);				
		SysInfoKeyProcess(hWnd, Msg, wParam, lParam);
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_SYSINFO);
		KillAllChildWnd(hWnd);		
		//		PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_SYSINFO);
//		SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		break;
		
	default:
/*
		if (WM_CLOSE == Msg) 
		{
			RemoveOneWnd(hWnd);
		}
		return DefWindowProc(hWnd, Msg, wParam, lParam);
*/
		
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
CreateSysInfoWnd(HWND hwndParent)
{
	static char szAppName[] = "SysInfoWnd";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) SysInfoWndProc;
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
		"SysInfoWnd",			// Window name	(Not NULL)
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
SysInfoKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	wParam =  KeyBoardMap(wParam);
	switch(wParam) 
	{
	case KEY_RETURN:
	case KEY_ENTER:
		{
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		break;	
	default:
		break;
	}
}


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
	char  *pCurTimeH = "2008-10-01 00:00";
	char  CurTime[255] = { 0 };	

#if 0
	if (!IsMCOnline()) 
	{
		if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			strcpy(pTimeShow, pCurTimeE);			
		}
		else if (SET_CHINESE == g_SysConfig.LangSel)
		{
			strcpy(pTimeShow, pCurTimeC);
		}	
		else if (SET_HEBREW == g_SysConfig.LangSel)
		{
			strcpy(pTimeShow, pCurTimeH);
		}			
		return;
	}
#endif

	TIMEVAL	=	GetSysTime();
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
		CurTime[17]='\0';
	}
	else if(SET_HEBREW == g_SysConfig.LangSel) 
	{
		strcpy(CurTime, pCurTimeH);
		memcpy(CurTime + 2,		OutTimeStr + 0, 2);//年
		memcpy(CurTime + 5,		OutTimeStr + 2, 2);//月
		memcpy(CurTime + 8,		OutTimeStr + 4, 2);//日
		memcpy(CurTime + 11,		OutTimeStr + 6, 2);//时
		memcpy(CurTime + 14,		OutTimeStr + 8, 2);//分
		CurTime[16]='\0';
	}
	else if (SET_CHINESE == g_SysConfig.LangSel) 
	{
		strcpy(CurTime, pCurTimeC);
		memcpy(CurTime	+ 2,	OutTimeStr + 0, 2);
		memcpy(CurTime	+ 6,	OutTimeStr + 2, 2);
		memcpy(CurTime	+ 10,	OutTimeStr + 4, 2);
		memcpy(CurTime	+ 14,	OutTimeStr + 6, 2);	
		memcpy(CurTime	+ 17,	OutTimeStr + 8, 2);			
		CurTime[19]='\0';
	}
    
    memcpy(pTimeShow, CurTime, strlen(CurTime));
}
