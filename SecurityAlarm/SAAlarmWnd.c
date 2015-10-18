/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SAAlarmWnd.c
**
**	AUTHOR:		Mike Zhang
**
**	DATE:		13 - Dec - 2010
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
#include "MXMem.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"

#include "MenuParaProc.h"
#include "SAAlarmWnd.h"
#include "AccessProc.h"

/************** DEFINES **************************************************************/

#define STR_INVALIDCARD_EN				"Invalid Card"
#define STR_SWIPEALARM_EN				"Swipe Alarm"
#define STR_INVALIDCARDSWIPEALARM_CN	"无效刷卡报警"


#define STR_TAMPERALARM_EN				"Tamper Alarm"
#define STR_TAMPERALARM_CN				"防拆报警"

#define STR_GATEOPENOVERTIME_EN			"Gate Open Overtime"
#define STR_GATEOVERTIMEALARM_CN		"开门超时报警"

#define STR_INFRAREDALARM_EN			"Infrared Alarm"
#define STR_INFRAREDALARM_CN			"红外报警"


#define STR_TAMPER_OPEN_ALARM_EN		"Tamper+Open Alarm"
#define STR_TAMPER_OPEN_ALARM_CN		"防拆+开门超时报警"

#define STR_TAMPER_INFRARED_ALARM_EN	"Tamper+Infrared Alarm"
#define STR_TAMPER_INFRARED_ALARM_CN	"防拆+红外报警"

#define STR_OPEN_INFRARED_ALARM_EN		"Open+Infrared Alarm"
#define STR_OPEN_INFRARED_ALARM_CN		"开门超时+红外报警"

#define STR_ALL_EN						"Tamper+Open+Infrared"
#define	STR_ALARM_ALL_CN				"防拆+开门+红外报警"


#define STR_ALARMCANCEL_EN				"Alarm Confirmed"
#define STR_ALARMCANCEL_CN				"报警已确认"

/************** TYPEDEFS *************************************************************/

typedef enum _ASALARMSTEP
{
	STEP_TAMPER_ALARM,
	STEP_GATEOPENOVERTIME_ALARM,
	STEP_INFRARED_ALARM,
	STEP_TAMPER_OPEN_ALARM,
	STEP_TAMPER_INFRARED_ALARM,
	STEP_OPEN_INFRARED_ALARM,
	STEP_ALL_ALARM,
	STEP_ALARM_CANCEL,
	STEP_IVDCARD_ALARM,
	STEP_PWD_ERROR
}ASALARMSTEP;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static TIMERPROC CALLBACK	AsAlarmTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		AsAlarmWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK AsAlarmWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		AsAlarmKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static void		SendAlarmConfirm2SA(void);

ASALARMSTEP AsAlarmStep = STEP_GATEOPENOVERTIME_ALARM;

HWND	g_HwndAsAlarm	=	NULL;

ASALARMSTEP AsAlarmStepBak = STEP_GATEOPENOVERTIME_ALARM;

static BOOL bPasswordStatus=FALSE;//for bug 14792


/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsAlarmTimerProc
**	AUTHOR:			Jeff Wang
**	DATE:			05 - May - 2009
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
static TIMERPROC CALLBACK
AsAlarmTimerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	KillTimer(hWnd, TIMER_ASALARM);
	
	if (STEP_PWD_ERROR == AsAlarmStep) 
	{
		AsAlarmStep = AsAlarmStepBak;
		PostMessage(hWnd, WM_PAINT, 0, 0);
	}
	else
	{
		PostMessage(hWnd, WM_CLOSE, 0, 0);
	}
	ClearKeyBuffer();
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsAlarmWndPaint
**	AUTHOR:			Jeff Wang
**	DATE:			05 - May - 2009
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
AsAlarmWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT					ps;
	RECT						Rect;
	CHAR						pDateBuf[TITLE_BUF_LEN] = {0};
	INT							xPos = 0;
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;		
	
	switch(AsAlarmStep)
	{
	case STEP_TAMPER_ALARM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TAMPERALARM_EN);
				MXDrawText_Left(Hdc, STR_TAMPERALARM_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRPWD_CN);
				MXDrawText_Left(Hdc, STR_TAMPERALARM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_CN, xPos, 1);			
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_TAMPERALARM));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_TAMPERALARM), xPos, 0);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ETRPWD), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);			
		}
		break;

	case STEP_GATEOPENOVERTIME_ALARM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_GATEOPENOVERTIME_EN);
				MXDrawText_Left(Hdc, STR_GATEOPENOVERTIME_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_GATEOVERTIMEALARM_CN);
				MXDrawText_Left(Hdc, STR_GATEOVERTIMEALARM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_CN, xPos, 1);			
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_GATEOVERTIMEALARM));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_GATEOVERTIMEALARM), xPos, 0);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ETRPWD), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);			
		}
		break;
	case STEP_IVDCARD_ALARM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRPWD_EN);
				MXDrawText_Left(Hdc, STR_INVALIDCARD_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_SWIPEALARM_EN, xPos, 1);
				MXDrawText_Left(Hdc, STR_ETRPWD_EN, xPos, 2);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_INVALIDCARDSWIPEALARM_CN);
				MXDrawText_Left(Hdc, STR_INVALIDCARDSWIPEALARM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_INVALIDCARDSWIPEALARM));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_INVALIDCARDSWIPEALARM), xPos, 0);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ETRPWD), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);			
		}
		break;

	case STEP_INFRARED_ALARM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRPWD_EN);
				MXDrawText_Left(Hdc, STR_INFRAREDALARM_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ETRPWD_CN);
				MXDrawText_Left(Hdc, STR_INFRAREDALARM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_INFRAREDALARM));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_INFRAREDALARM), xPos, 0);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ETRPWD), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);			
		}
		break;
	case STEP_TAMPER_OPEN_ALARM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TAMPER_OPEN_ALARM_EN);
				MXDrawText_Left(Hdc, STR_TAMPER_OPEN_ALARM_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TAMPER_OPEN_ALARM_CN);
				MXDrawText_Left(Hdc, STR_TAMPER_OPEN_ALARM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_TAMPER_OPEN_ALARM));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_TAMPER_OPEN_ALARM), xPos, 0);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ETRPWD), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);			
		}
		break;

	case STEP_TAMPER_INFRARED_ALARM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TAMPER_INFRARED_ALARM_EN);
				MXDrawText_Left(Hdc, STR_TAMPER_INFRARED_ALARM_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_TAMPER_INFRARED_ALARM_CN);
				MXDrawText_Left(Hdc, STR_TAMPER_INFRARED_ALARM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_TAMPER_INFRARED_ALARM));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_TAMPER_INFRARED_ALARM), xPos, 0);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ETRPWD), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);			
		}
		break;

	case STEP_OPEN_INFRARED_ALARM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_OPEN_INFRARED_ALARM_EN);
				MXDrawText_Left(Hdc, STR_OPEN_INFRARED_ALARM_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_OPEN_INFRARED_ALARM_CN);
				MXDrawText_Left(Hdc, STR_OPEN_INFRARED_ALARM_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_OPEN_INFRARED_ALARM));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_OPEN_INFRARED_ALARM), xPos, 0);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ETRPWD), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);			
		}
		break;
		
	case STEP_ALL_ALARM:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ALL_EN);
				MXDrawText_Left(Hdc, STR_ALL_EN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ALARM_ALL_CN);
				MXDrawText_Left(Hdc, STR_ALARM_ALL_CN, xPos, 0);
				MXDrawText_Left(Hdc, STR_ETRPWD_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				xPos = HebrewGetXPos(Hdc,GetHebrewStr(HS_ID_ALARM_ALL));
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ALARM_ALL), xPos, 0);
				MXDrawText_Right(Hdc,GetHebrewStr(HS_ID_ETRPWD), xPos, 1);
			}
			memset(pDateBuf, 42, 19);
			MulLineDisProc(Hdc, pDateBuf, g_KeyInputBufferLen, 2);			
		}
		break;

	case STEP_ALARM_CANCEL:
		{
			if (SET_ENGLISH == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ALARMCANCEL_EN);
				MXDrawText_Left(Hdc, STR_ALARMCANCEL_EN, xPos, 1);
			}
			else if (SET_CHINESE == g_SysConfig.LangSel) 
			{
				xPos = GetXPos(STR_ALARMCANCEL_CN);
				MXDrawText_Left(Hdc, STR_ALARMCANCEL_CN, xPos, 1);
			}
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_ALARMCONFIRM), 1);
			}
		}
		break;
	case STEP_PWD_ERROR:
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
			else if (SET_HEBREW == g_SysConfig.LangSel) 
			{
				MXDrawText_Center(Hdc, GetHebrewStr(HS_ID_PWDINCORRECT), 1);
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
**	FUNCTION NAME:	AsAlarmWndProc
**	AUTHOR:			Jeff Wang
**	DATE:			05 - May - 2009
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**
**	RETURNED VALUE:	
**	
**	NOTES:
**			
*/
static LRESULT CALLBACK 
AsAlarmWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		ClearKeyBuffer();
		break;
		
	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			AsAlarmWndPaint(hWnd, Msg, wParam, lParam);
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
		AsAlarmKeyProcess(hWnd, Msg, wParam, lParam);
		PostMessage(hWnd, WM_PAINT, 0, 0);
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_ASALARM);
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_ASALARM);
		g_ASInfo.ASWorkStatus = STATUS_OPEN;
//		SendMessage(GetParent(g_HwndAsAlarm),  WM_REFRESHPARENT_WND, 0, 0);		
//		SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		g_HwndAsAlarm	=	NULL;
		ClearKeyBuffer();
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
**	FUNCTION NAME:	CreateSysConMenuWnd
**	AUTHOR:			Jeff Wang
**	DATE:			05 - May - 2009
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**
**	RETURNED VALUE:	
**
**	NOTES:
**			
*/
void
CreateAsAlarmWnd(int nAlaType)
{
	static char szAppName[] = "AsAlarm";
	HWND		hWnd;
	WNDCLASS	WndClass;

	if (g_HwndAsAlarm)
	{
		return;
	}
	CloseChildWnd(g_hMainWnd);

	AsAlarmStep = nAlaType; 
	bPasswordStatus=FALSE;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) AsAlarmWndProc;
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
		"AsAlarmWnd",			// Window name	(Not NULL)
		WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,	// Style		(0)
		0,					// x			(0)
		0,					// y			(0)
		SCREEN_WIDTH,		// Width		
		SCREEN_HEIGHT,		// Height
		g_hMainWnd /*GetFocus()*/,		// Parent		(MwGetFocus())
		NULL,				// Menu			(NULL)
		NULL,				// Instance		(NULL)
		NULL);				// Parameter	(NULL)
	
	AddOneWnd(hWnd,WND_ALARM_PRIORITY_5);
	if (hWnd == GetFocus())
	{
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
	}
	else
	{
		ShowWindow(hWnd, SW_HIDE);
	}
	
	g_HwndAsAlarm	=	hWnd;
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	IPSetKeyProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		05 - May - 2009
**
**	DESCRIPTION:	
**			Process UP,DOWN,RETURM and ENTER	
**
**	ARGUMENTS:		
**
**	RETURNED VALUE:	
**
**	NOTES:
**
*/

static void 
AsAlarmKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	wParam =  KeyBoardMap(wParam);
	if(bPasswordStatus==TRUE)
		return;

	if (KEY_RETURN == wParam) 
	{
		//PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if (KEY_ENTER == wParam) 
	{
		if (CodeCompare(g_KeyInputBuffer, g_SysConfig.SysPwd, g_KeyInputBufferLen))
		{
			AsAlarmStep = STEP_ALARM_CANCEL;
			SetTimer(hWnd, TIMER_ASALARM, PROMPT_SHOW_TIME, (TIMERPROC)AsAlarmTimerProc);
//			StopPlayAlarmANote();
			SendAlarmConfirm2SA();
			bPasswordStatus=TRUE;		
		}
		else
		{
			AsAlarmStep = STEP_PWD_ERROR;
			SetTimer(hWnd, TIMER_ASALARM, PROMPT_SHOW_TIME, (TIMERPROC)AsAlarmTimerProc);			
		}
		ClearKeyBuffer();
	}
	else if (wParam >= KEY_NUM_0 && wParam <= KEY_NUM_9)
	{
		CircleBufEnter(g_KeyInputBuffer, &g_KeyInputBufferLen, KEY_BUF_LEN, (CHAR)wParam);
	}
}



void
CloseSAConfirmWnd()
{
	SendMessage(g_HwndAsAlarm,  WM_CLOSE, 0, 0);
}

void
UpdateAlarmConfirmWnd(int nType)
{
	AsAlarmStepBak = nType;
	
	if(ALA_WND_DESTROY == nType)
	{
		if(NULL != g_HwndAsAlarm)
		{
			SendMessage(g_HwndAsAlarm,  WM_CLOSE, 0, 0);
		}
	}
	else
	{
		if(NULL == g_HwndAsAlarm)
		{
			CreateAsAlarmWnd(nType);
		}
		else
		{
			AsAlarmStep  =  nType;
			memset(g_KeyInputBuffer, 0, KEY_BUF_LEN);
			g_KeyInputBufferLen = 0;
			SendMessage(g_HwndAsAlarm,  WM_PAINT, 0, 0);		
		}
	}
}



static void
SendAlarmConfirm2SA(void)
{
	MXMSG	MsgSend ;
	
	MsgSend.dwDestMd	= MXMDID_SA;
	MsgSend.dwSrcMd		= MXMDID_SA;
	MsgSend.dwMsg		= MXMSG_SECURITY_ALARM_CONFIRM;
	MsgSend.dwParam		= 0;
	MsgSend.pParam      = NULL;
	MxPutMsg(&MsgSend);
}



