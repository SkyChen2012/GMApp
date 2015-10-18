/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	ModuleTalk.h
**
**	AUTHOR:		Harry Qian
**
**	DATE:		25 - Sep - 2006
**
**	FILE DESCRIPTION:
**				the headfile of ModuleTalk.c
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/
#ifndef MODULETALK_H_
#define MODULETALK_H_
/************** SYSTEM INCLUDE FILES **************************************************************************/

//Nothing should be included here

/************** USER INCLUDE FILES ****************************************************************************/
#include "MXMem.h"
#include "MXList.h"
#include "MXTypes.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "Dispatch.h"
#include "BacpNetCtrl.h"
//Nothing should be included here

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/
//Debug optical

//#define HV_TALKMD_DEBUG
//#define HV_TALKMD_TIMER_DEBUG
//#define TALK_TIMETICK_DEBUG


#define		WM_TL_ADD		(WM_USER + 66)

//DEVICE TYPE
#define TYPE_NORMAL					1
#define TYPE_HDB					2

#define PHOTO_HEADER_LEN			9

#define NAME_SIZE					256

//Timeout setting
#define IRIS_ACK_LOG_TIMEOUT		30000
#define IRIS_ACK_TIMEOUT			2500	//2s = 2000ms
#define IRIS_RING_TIMEOUT			45000	//45s
#define IRIS_TALKING_TIMEOUT		120000	//120s
#define IRIS_MONITOR_TIMEOUT		15000	//15s
#define IRIS_MONITOR_TIMEOUT_9001A  3600000	//3600s

#define TEST_PICKUP_TIME			3000//3s
#define IRIS_SHOW_INFO				2000
#define IRIS_SHOW_INFO2				2000
#define IRIS_TAKE_PHOTO				2000
#define IRIS_TALKING_COUNT_TIME		1000	//1s
#define IRIS_LEAVE_PHOTO_TIMEOUT    10000
#define IRIS_LEAVE_PHOTO_END_DELAY  5000

#define IRIS_TAKE_PHOTO_TIME		5	//the fifth second

//the message used by GUI and talking 
#define MXMSG_USER					0x000F0000

#define MXMSG_USER_PICKUP			(MXMSG_USER + 2)
#define MXMSG_USER_HANGUP			(MXMSG_USER + 3)

#define MXMSG_USER_CALL_IN			(MXMSG_USER + 4)	//when be called, the MHV send it to SHV that someone calling in
#define MXMSG_USER_STOP				(MXMSG_USER + 5)		//MHV send it to SHV that recover the initial state.

//the message from sending to Eth sub-module
#define MXMSG_USER_CALLOUT			(MXMSG_USER + 6)	//Talking -> Eth -> Talk control

//monitor message
#define MXMSG_USER_MONITOR			(MXMSG_USER + 8)	//from GUI that want to monnitor the GM.
#define MXMSG_USER_CANCEL_MOITOR	(MXMSG_USER + 9)	//cancel the monitor command,from the GUI
#define MXMSG_USER_UGM_MON			(MXMSG_USER + 19)
#define MXMSG_USER_HDB_MON			(MXMSG_USER + 18)
//UNLOCK 
#define MXMSG_USER_UNLOCK			(MXMSG_USER + 10)	//unlock the GM.

#define MXMSG_USER_HDB_CALL			(MXMSG_USER + 11)	// 
#define MXMSG_USER_HDB_STOP			(MXMSG_USER + 12)

#define MXMSG_USER_FORE_GM			(MXMSG_USER + 13)
#define MXMSG_USER_NEXT_GM			(MXMSG_USER + 14)


#define MXMSG_USER_HDB_QUERY		(MXMSG_USER + 15)

#define MXMSG_USER_CALL				(MXMSG_USER + 16)

#define MXMSG_USER_GM_CODECALLOUT	(MXMSG_USER + 17)

#define MXMSG_USER_GM_PRJCALLOUT	(MXMSG_USER + 18)

#define MSMSG_USER_FORWARD			(MXMSG_USER + 20)
//the response ACK command from original command
#define ACK_OFFSET		0x8000


//the statue of talk control sub module
#define ST_ORIGINAL		0x00000000


//When out module call in, ig: GM call HV. the state of talk control
#define ST_CI_WAIT_PICKUP	0x00000001
#define ST_CI_WAIT_PICKUP_ACK	0x00000002
#define ST_CI_TALKING			0x00000003
#define ST_CI_WAIT_HANGUP_ACK	0x00000004

#define ST_ASK_PHOTO			0x00000020
#define ST_GET_PHOTO			0x00000021
//When HV call out, ig: HV call MC. the state of talk control
#define ST_CO_WAIT_CALLOUT_ACK	0x00000005
#define ST_CO_WAIT_PICKUP		0x00000006
#define ST_CO_TALKING			0x00000007
#define ST_CO_WAIT_HANGUP_ACK	0x00000008
#define ST_CO_NO_ANSWER			0x00000009
#define ST_CO_CALL_FAIL			0x0000000A
#define ST_CO_CALL_CODE_INEXIST	0x0000000B
#define ST_TALK_END				0x0000000C
#define ST_TALK_LEAVEPHOTO		0x0000000D
#define ST_TALK_LEAVEPHOTO_END	0x0000000E
#define ST_TALK_WAIT_LEAVEPHOTO	0x0000000F


//When HV monitor GM. the state of talk control
#define ST_MT_MONITOR			0x00000010
#define ST_MT_INT_ACK			0x00000011
//when HV unlock GM, the state of talk control
#define ST_UK_DELAY				0x00000012




#define UK_DELAY_TIME			1500


#define		MM_TYPE_NORMAL		0
#define		MM_TYPE_TALK_GM		1
#define		MM_TYPE_TALK_MC		2
#define		MM_TYPE_TALK_EHV	3
//only for passing the compily




#define DEVICE_CODE_LEN		20	

#define FIRST_GM			1
#define HAVE_FIND_GM		0


//talk log status change
#define TYPE_IF_PICK_UP		1
#define TYPE_HAVE_PIC		2

//talk log picture directory


//talk event report
#define TLE_IDLE		0
#define TLE_BUSY		1



#define HDB_STATUS_IDLE			0
#define HDB_WAIT_QUERY_RESULT	1

#define MOM_TYPE_NODEVICE		0
#define MON_TYPE_MC				2
#define MON_TYPE_EHV			3


#define TAKE_PHOTO_RETRY_NUM	1


#define	DEVICE_TYPE_SHV			1
#define	DEVICE_TYPE_CGM			2
#define	DEVICE_TYPE_UGM			3
#define	DEVICE_TYPE_EHV			4
#define	DEVICE_TYPE_MC			5
#define DEVICE_TYPE_HDB			6
#define DEVICE_TYPE_LIST		7

#define TALK_FORWARD_IDLE		0
#define TALK_FORWARD_LIST		1
#define TALK_FORWARD_FORWARD	2

#define FORWARD_RESULT_ACCEPT	0
#define FORWARD_RESULT_FAIL	    1

/*the duration of a coral GM or a amber GM.it is a fixed value decided by the hardware rather than the IRIS Editor*/
#define CORAL_AMBER_GM_ULK_DURATION     40*1000

/************** STRUCTURES *******************************************************/

typedef	struct _TalkingInfo
{
	BYTE		bMMType;	//0, mean normal, 1 mean play A/V,send A, 2 mean play A, send A/V;
	DWORD		dwTalkState;			// 
	DWORD		dwDestIP;
	char		szTalkDestDevCode[DEVICE_CODE_LEN];	//
	char		szName[NAME_SIZE];
	BYTE		bNeedPhotoSaved;	//
	BYTE		bHavePhotoSaved;	//
} TALKINGINFO;

typedef	struct _TimerTickCount
{
	DWORD		dwTalkTimer;				// 
	DWORD		dwMonitorTimer;				//
	DWORD		dwUnlockTimer;
	DWORD		dwUnlockShowTimer;
	DWORD		dwCurrentTick;
	DWORD		dwPhotoSavedTimer;
} TIMERCONUT;

typedef	struct _MonitInfo
{
	BYTE		bType;		//the device type
	DWORD		dwMonState;		// TC state when in monitor process
	DWORD		dwDestIP;
	char		szMonSrcDevCode[DEVICE_CODE_LEN];	//
	char		szName[NAME_SIZE];
	UINT		nGMID;
	BOOL		bNextValid;
} MONITINFO;

typedef	struct _UnlockInfo
{
	DWORD		dwUnLockState;
	DWORD		dwUnLockShowState;
	UINT		nType;
	DWORD		dwDestIP;
	char		szULDestDevCode[DEVICE_CODE_LEN];	//
} UNLOCKINFO;

typedef	struct _GMDEVICECODE
{
	char			szGMCode[DEVICE_CODE_LEN];
	PBYTE			 pNext;
} GMCODELIST;

typedef	struct _MXTalkInfo
{
	UINT				nGMid;			//self hv id
	UINT				nRefnum;
	BOOL				bAutoPick;		// when leavemode, the ethhv automatic picking up.
	BOOL				bUnLock;		// 1 eable unlock, 0 disable bUnlock.
	BOOL				bSCR;			// the state of screen
	DWORD				dwSelfGMIP;	// the ip address of the MasterHV, using by talking under a family
	DWORD				dwMCIP;
	BOOL				bMCStatus;
	TALKINGINFO			talking;		// talking run information
	MONITINFO			monitor;		// monitor run information
	UNLOCKINFO			unlock;			// unlock
	TIMERCONUT			Timer;			// tiem tick count struct

	//	PROCTOL				PriCtl;
	char				szTalkDevCode[DEVICE_CODE_LEN];	//device code of EGM self
} GMTALKINFO;

typedef	struct _MXPRIPORControl
{
	DWORD	dwMonState;
	DWORD	dwTalkState;

} PROCTOL;

/************** functions       *******************************************************/
//extern function



extern void TalkInit();
extern void TalkProcess();
extern void TalkExit();

extern	DWORD	GetTalkState(void);
extern	DWORD	GetMonitorState(void);
extern	BOOL	IsTalkNoTalking(void);

extern DWORD	GetMHVIp(void);
extern void		* GetSelfHVCode(void);
extern BOOL		IsMasterHV(void);
extern UINT		GetHVSelfID(void);
extern void		HVQueryAllGM(void);
extern void		SHVGetTKLReportACK();
extern BOOL		SetCallDestIPCode(const char * pCode, UINT nIP);
extern UINT		GetHDBID(void);
extern BOOL		IfMulHVSystem(void);
extern void		HVSpecialAlarmPro(void);
extern void		UpdateLeaveMsgMode(BOOL bLeaveMsgMode);
extern void		AddTalkLogInfo(UINT nFileNameRefNum);
extern void		TalkSaveHDBPic(void);
extern void		SaveImageFile(const char * pFile, PBYTE pFileBuf, UINT nLen);
extern void		GetMdTalkInfo(PBYTE pBuf);
extern BOOL		TalkFuncAudioVideo(BOOL * pbCapA2Eth, BOOL * pbCapV2Eth, BOOL * pbPlayEthA, BOOL * pbPlayEthV);

extern GMTALKINFO g_TalkInfo;

 void	ReadConfig(void);
 BOOL	PriorCtrl(MXMSG *);
 void	DoMaster(MXMSG * ); 

 
 void	TalkHVPickup(void);


 BOOL	DoTKTalkingOut(MXMSG *);
 BOOL	DoTKTalkingIn(MXMSG *);


 void	DoFault(MXMSG *pmsg);
 void	DoClearResource(MXMSG *pmsg);
 BYTE	GetDestHVCode(void);
 UINT	GetTalkDeviceCode(void);
 
 BOOL	 IsLeaveWordMode(void);
 void	 TimeOutCtrl(void);
 void	 TalkTimeOutCtrl(void);
 void	 MTTimeOutCtrl(void); 
 
 void	SaveHVcodePickup(void);
 void	TalkStartLeaveWord(void);
 void	TCStopLeaveWord(void);


 void	 OpenScreen(void);
 void	 CloseScreen(void);
 void	 ResetDestiDevice(void);
/////////######################################################################//////


 void	 TalkSendGMcallGMAck(MXMSG *pmsg);
 void	 TalkSendMCcallGMAck(MXMSG *pmsg);

 void	 MHVNotifySHVCallIn(MXMSG *pmsg);
 void	 SHVNotifyMHVPickedUp(MXMSG * pmsg);
 void	 SHVNotifyMHVHangup(MXMSG * pmsg);
 void	 GMSendGMPickupToNet(void);

 void	 TalkStopRing(void);
 
 void	 GMStartTalking(void);
 void	 GMStopTalking(void);
	 
 //play gm video when gm calling hv
 void	 HVStopPlayGMVideo(void);
 void	 HVPlyGMVideo(MXMSG *pmsg);
 //////////////

 void	 GMStopCallingVideo(void);
 
 BOOL	 DoGMTalkingIn(MXMSG *pmsg);
 BOOL	 DoGMTalkingOut(MXMSG *pmsg);
 BOOL	 DoGMMonitor(MXMSG *pmsg);
 BOOL	 DoGMUnlock(MXMSG *pmsg);

 BOOL	 GMTalkInOriginalPro(MXMSG * pmsg);
 BOOL	 GMTalkInWaitGMPickupPro(MXMSG *pmsg);
 BOOL	 TalkInWaitforPickupAck(MXMSG * pmsg);
 BOOL	 TalkInTalkingPro(MXMSG *pmsg);
 BOOL	 WaitForHangupAckPro(MXMSG *pmsg );

 BOOL	 TalkOutOriginalPro(MXMSG * pmsg);
 BOOL	 WaitCalloutAckPro(MXMSG *pmsg);
 BOOL	 TalkOutWaitForPickupPro(MXMSG * pmsg);
 BOOL	 TalkOutTalkingPro(MXMSG *pmsg);
 void	 TalkSendHVcallMC(MXMSG *pmsg);
 void	 TalkSendMCPickupACK(MXMSG * pmsg);
 void	 TalkSendGMPickupACK(MXMSG * pmsg);
 BOOL	 LeavePhotoPro(MXMSG *pmsg)	 ;


 void	 TalkSendGMHangupAck(MXMSG *pmsg);
 void	 TalkSendMCHangupAck(MXMSG *pmsg);
 void	 TalkSendHVHangupAck(MXMSG *pmsg);
 
 BOOL	 SHVTalkInWaitHVPickupPro(MXMSG *pmsg);
 BOOL	 SHVTalkInOriginalPro(MXMSG * pmsg);
 

 //the monitor function.
 void	 MTSendMonStartAckCMD(MXMSG *pmsg);
 void	 MTSendMonCancelAckCMD(void);
 
 BOOL	 MTWaitMonitorAckPro(MXMSG * pmsg);
 BOOL	 MTMonitorPro(MXMSG * pmsg);
 BOOL	 MTOriginalPro(MXMSG *pmsg);
 void	 MTDisplyVideo(void);
 void	 MTSendCancelCMD(void);
 void	 MTStopPlayVideo(void);
 void	 SendMNTInterruptCMD(MXMSG *pmsg);


 //The unlock 
 BOOL	 HVCanUnLock(void);
 BOOL	 UKOriginalPro(MXMSG *pmsg);
 BOOL	 UKWaitAckPro(MXMSG *pmsg);
 void	 UKSendUnlockAck(DWORD dwUlkIP);
 void	 EnableUnlock(void);
 void	 DisableUnlock(void);
 void	 MakeUnLockIP(void);

//about the HDB function
 void	GMStartCallRing();
 void	GMStopCallRing(void);
 
 BOOL	 HVGetGMCode(MXMSG * pmsg);
 void	 HVTalkPhoto(void);
 BOOL	 HVGetPhoto(MXMSG * pmsg);
 void	 HVSavePhoto(void);
 void	 HVStopPhoto(void);
 BOOL	 HVAddNewTalkLog(void);
 UINT	 SavePicToIndexFile(const char * szPicName);


 void	 TalkEventReport(void);



extern DWORD ChangeIPFormat(DWORD OldIPAddr);


extern BYTE GetTalkDestStreamFormat(void);
extern BYTE GetTalkLocStreamFormat(void);
extern void MTSendDecargsAckCMD(void);
extern DWORD setMonitoringTime(void);

#endif


