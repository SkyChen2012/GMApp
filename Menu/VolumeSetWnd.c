/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	VolumeSetWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		03 - Jun - 2009
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

#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"
#include "Dispatch.h"
#include "MXMdId.h"
#include "MXMsg.h"
#include "VolumeSetWnd.h"
#include "Multimedia.h"
#include "IniFile.h"

extern void KillAllChildWnd(HWND hWnd);


/************** DEFINES **************************************************************/

#define MAX_TALK_VOLUME 19
#define MIN_TALK_VOLUME 0
#define VOLUME_BITS		2

typedef enum _VOLUMESETRESULT
{
	VOLUME_SET_FAIL = 1,
	VOLUME_SET_OK,
}VOLUMESETRESULT;


/************** TYPEDEFS *************************************************************/
typedef	struct tagSCVOLUME
{
	BOOL				bShowPromptInfo;
	VOLUMESETRESULT		eResult;
	INT					iStoreVolume;
	INT					iVolumeIndex;
} SCVOLUMEINFO;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

void CreateVolumeSetWnd(HWND hwndParent);
BOOL VolumeSetIsEnable();

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		ShowVolumeSetInput(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK VolumeSetWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static VOID VolumeSetKeyProcess(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static VOID	ShowVolumeSetPromptInfo(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static void SaveVolumeToFile(void) ;
//static void PlayTone(void);
static VOID VolumeSetCreate(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static VOID VolumeSetSetTimer(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static VOID VolumeSetPaint(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static VOID VolumeSetRefreshParentWnd(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static VOID VolumeSetKey(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static VOID VolumeSetTimer(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static VOID VolumeSetDestroy(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static BOOL IsValidVolume(int volume);
static int	GetNewNumber(int old,int position,int val);





static SCVOLUMEINFO		gAdjustVolume;


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenDelaySetWndPaint
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
ShowVolumeSetInput(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT					ps;
	RECT						Rect;	
	
	CHAR	pBuf[TITLE_BUF_LEN] = { 0 };

	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);	
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;		

	if (SET_CHINESE == g_SysConfig.LangSel) 
	{
		MXDrawText_Center(Hdc, STR_VOLUME_TIP_CN, 0);
	}
	else if (SET_ENGLISH == g_SysConfig.LangSel) 
	{
		MXDrawText_Center(Hdc, STR_VOLUME_TIP_EN, 0);
	}

	sprintf(pBuf,"%d",gAdjustVolume.iStoreVolume);
//	MXDrawText_Center(Hdc, pBuf, 2);
	DrawDataParaSet(Hdc, pBuf, gAdjustVolume.iVolumeIndex);
	
	EndPaint(hWnd, &ps);
}

static VOID VolumeSetCreate(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	gAdjustVolume.iVolumeIndex = 0;
	gAdjustVolume.eResult = VOLUME_SET_FAIL;
	gAdjustVolume.iStoreVolume = g_SysConfig.TalkVolume;
	gAdjustVolume.bShowPromptInfo = FALSE;
	PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
}

static VOID VolumeSetSetTimer(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	SetTimer(hWnd, TIMER_TALK_VOLUME, wParam, NULL);
}

static VOID VolumeSetPaint(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	if (GetFocus() == hWnd)
	{
		if (gAdjustVolume.bShowPromptInfo)
		{
			ShowVolumeSetPromptInfo(hWnd, iMsg, wParam, lParam);
		}
		else
		{
			ShowVolumeSetInput(hWnd, iMsg, wParam, lParam);
		}
	}
}

static VOID VolumeSetRefreshParentWnd(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	SetFocus(hWnd);
	PostMessage(hWnd,  WM_PAINT, 0, 0);
}

static VOID VolumeSetKey(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	if (GetFocus() != hWnd) 
	{
		SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
		return;			
	}
	if (!gAdjustVolume.bShowPromptInfo)
	{
		ResetTimer(hWnd, TIMER_TALK_VOLUME, INTERFACE_SETTLE_TIME, NULL);
		VolumeSetKeyProcess(hWnd, iMsg, wParam, lParam);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
	}
}

static VOID VolumeSetTimer(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	KillTimer(hWnd, TIMER_TALK_VOLUME);
	if (gAdjustVolume.bShowPromptInfo) 
	{
		gAdjustVolume.bShowPromptInfo = FALSE;		
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else
	{
		KillAllChildWnd(hWnd);		
	}
}

static VOID VolumeSetDestroy(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	KillTimer(hWnd, TIMER_TALK_VOLUME);
	gAdjustVolume.iStoreVolume	=	0;
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
VolumeSetWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{     
	switch (iMsg)
	{
	case WM_CREATE:
		VolumeSetCreate(hWnd, iMsg, wParam, lParam);
		break;		
		
	case WM_SETTIMER:
		VolumeSetSetTimer(hWnd, iMsg, wParam, lParam);
		break;

	case WM_PAINT:
		VolumeSetPaint(hWnd, iMsg, wParam, lParam);
		break;		
	case WM_REFRESHPARENT_WND:
		VolumeSetRefreshParentWnd(hWnd, iMsg, wParam, lParam);
		break;
	case WM_CHAR:
		VolumeSetKey(hWnd, iMsg, wParam, lParam);
		break;
		
	case WM_TIMER:
		VolumeSetTimer(hWnd, iMsg, wParam, lParam);		
		break;
		
	case WM_DESTROY:
		VolumeSetDestroy(hWnd, iMsg, wParam, lParam);		
		break;
		
	default:
		
		DefWindowProc(hWnd, iMsg, wParam, lParam);
		
		if (WM_CLOSE == iMsg) 
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
CreateVolumeSetWnd(HWND hwndParent)
{
	static char szAppName[] = "VolumeSet";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) VolumeSetWndProc;
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
		"VolumeSet",			// Window name	(Not NULL)
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
VolumeSetKeyProcess(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	UINT nKey =  KeyBoardMap(wParam);

	if (KEY_RETURN == nKey) 
	{	
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if (KEY_ENTER == nKey) 
	{
		gAdjustVolume.bShowPromptInfo = TRUE;
		if(IsValidVolume(gAdjustVolume.iStoreVolume))
		{
			gAdjustVolume.eResult = VOLUME_SET_OK;
			g_SysConfig.TalkVolume = gAdjustVolume.iStoreVolume;
			SaveVolumeToFile();
			GMSetVolumeAPI(gAdjustVolume.iStoreVolume);
		}
		else
		{
			gAdjustVolume.eResult = VOLUME_SET_FAIL;
		}

	}
	else if ((nKey >= KEY_NUM_0) && (nKey <= KEY_NUM_9))
	{
		gAdjustVolume.iVolumeIndex++;
		if(gAdjustVolume.iVolumeIndex >= VOLUME_BITS)
		{
			gAdjustVolume.iVolumeIndex = 0;
		}
		gAdjustVolume.iStoreVolume=GetNewNumber(gAdjustVolume.iStoreVolume,gAdjustVolume.iVolumeIndex,((BYTE)nKey - 0x30));
//		gAdjustVolume.iStoreVolume=gAdjustVolume.iStoreVolume*10+((BYTE)nKey - 0x30);
		
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
ShowVolumeSetPromptInfo(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);	
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;	
	if(gAdjustVolume.eResult == VOLUME_SET_FAIL)
	{
		if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			MXDrawText_Center(Hdc, STR_VOLUME_MODIFIED_TIP_FAIL_CN, 1);
		}
		else if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			MXDrawText_Center(Hdc, STR_VOLUME_MODIFIED_TIP_FAIL_EN, 1);
		}
	}
	else if(gAdjustVolume.eResult == VOLUME_SET_OK)
	{
		if (SET_CHINESE == g_SysConfig.LangSel) 
		{
			MXDrawText_Center(Hdc, STR_VOLUME_MODIFIED_TIP_OK_CN, 1);
		}
		else if (SET_ENGLISH == g_SysConfig.LangSel) 
		{
			MXDrawText_Center(Hdc, STR_VOLUME_MODIFIED_TIP_OK_EN, 1);
		}
	}
	
	
	ResetTimer(hWnd, TIMER_TALK_VOLUME, PROMPT_SHOW_TIME, NULL);

	EndPaint(hWnd, &ps);
}

//add for shell GM
static BOOL
IsValidVolume(int volume)
{
	if((volume > MAX_TALK_VOLUME) || (volume < MIN_TALK_VOLUME))
	{
		return FALSE;
	}
	return TRUE;
}

static void
SaveVolumeToFile(void) 
{
//	g_VolMd.gAdjustVolume.nRing = VOLUME_START + g_VolMd.Cnt.nRingCnt * PER_VOLUME;
//	g_VolMd.gAdjustVolume.nVolume = VOLUME_START + g_VolMd.Cnt.nVolumeCnt * PER_VOLUME;

	OpenIniFile(SC);
	WriteInt(SC_SEC_GLOBAL, SC_TALK_VOLUME, gAdjustVolume.iStoreVolume);
	CloseIniFile();



	// Notify multi-media module volume changed
	TalkVolumeChangedNotify2MM(gAdjustVolume.iStoreVolume);

//	UpdateAudioVolume(g_VolMd.gAdjustVolume.nVolume);
}

/*
static void
PlayTone(void)
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_MM;
	msgSend.dwDestMd	= MXMDID_MM;
	msgSend.dwMsg		= MXMSG_PLAY_TALK_TEST;
	msgSend.dwParam		= 0;
	msgSend.pParam		= NULL;
	
	MxPutMsg(&msgSend);
}
*/

BOOL VolumeSetIsEnable()
{
	return IfCX20707();
}

static int GetNewNumber(int old,int position,int val)
{
	int iNum = old;
	int i = 0;
	int iRet = 0;
	BYTE tmp;
	BYTE abyNum[VOLUME_BITS];
	for(i = 0;i < VOLUME_BITS;i++)
	{
		tmp = iNum/10;
		abyNum[i] = iNum-tmp*10;
		iNum = tmp;
	}
	abyNum[position]=val;
	iNum=1;
	for(i = 0;i < VOLUME_BITS;i++)
	{
		iRet+=(abyNum[i]*iNum);
		iNum *= 10;
	}
	return iRet;
}


