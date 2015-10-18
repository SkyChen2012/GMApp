/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MXCommon.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		01 - Dec - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef MXCOMMON_H
#define MXCOMMON_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/
#include <windows.h>
#include "MXTypes.h"
#include "UnicodeString.h"
/************** DEFINES ***************************************************************************************/
//#define NEW_OLED_ENABLE

//#define XIU_ZHOU_POLICE_STATION

#define SET_CHINESE			1
#define SET_ENGLISH			2
#define SET_HEBREW		 	3
//AMT file
#define AMTFILE																"/mox/rdwr/AMT"

//AST file
#define ASTFILE																"/mox/rdwr/AST"

//AST_HV file
#define AST_HVFILE                                                          "/mox/rdwr/AST_HV"

//APT file
#define APTFILE																"/mox/rdwr/APT"

//CPT file
#define CPTFILE																"/mox/rdwr/CPT"

//PUT file
#define PUTFILE																"/mox/rdwr/PUT"

//ALT file
#define ALTFILE																"/mox/rdwr/ALT"
//

#define DOLTFILE															"/mox/rdwr/DLT"

//AuthorizeCd file
#define AATFILE																"/mox/rdwr/AAT"

//serial port exchange
#define SPEFILE																"/mox/rdwr/SPE"

#define VAT_FILE				                                            "/mox/rdwr/VAT"//moved from LCAPro_mitsubishi.h to this by [MichaelMa] at 23.8.2012 
                                                                                           //to make self known by FileConfigure.c for bug 9848
//Access LOGFILE
#define ALOGFILE															"/mox/rdwr/ALOG"

//哈希算法文件存储
#define AHASHFILE															"/mox/rdwr/HASH"

//The interface string
#define STR_ENTERFUNNUM_CN				"输入功能码加#键"
#define STR_ENTERFUNNUM_EN				 "Function Number + #"

#define STR_ENTERPASSWORD_CN				"或输入密码加*键"
#define STR_ENTERPASSWORD_EN				 "Enter Password + *"


#define STR_PRESSENTER_CN				   "按#键确认"
#define STR_PRESSENTER_EN				    "Number+#"

#define STR_CONFDELETE_CN				   "确认删除吗"
#define STR_CONFDELETE_EN				 "Confirm Delete"
#define STR_AREYOUSURE_EN				 "Are You Sure"
#define STR_TODELETE_EN					"To Delete"


#ifdef XIU_ZHOU_POLICE_STATION
    #define STR_PRESSHELP_CN                     "或者直接按房间号"
    #define STR_PRESSHELP_CN2                     "请按0"
#else
    #define STR_PRESSHELP_CN                     "按#查看帮助"
#endif

#ifdef XIU_ZHOU_POLICE_STATION
    #define STR_PRESSHELP_EN                     "or press room number"
    #define STR_PRESSHELP_EN2                    "Press 0"
#else
    #define STR_PRESSHELP_EN                     "Press # for Help"
#endif


#define STR_NETFAIL_CN                     "网络故障"
#define STR_NETFAIL_EN                     "Network Failure"

#define STR_ENTERRES_EN                     "Enter"
#define STR_ENTERNUM_EN                     "Resident Number" //del +# by [MichaelMa] for bug 8846
#define STR_ENTERRESNUM_CN                     "请输入房间号"

#define STR_CALLOUT_EN                     "Calling Out..."
#define STR_CALLOUT_CN                     "正在呼叫"

#define STR_MC_EN                     "MC"
#define STR_MC_CN                     "管理中心"

#define STR_CONVERTIME_EN                     "Conversation Time"
#define STR_CONVERTIME_CN                     "通话中..."
#define STR_REMAIN_EN                     "Remaining..."
#define STR_SEC_EN                     " Seconds"
#define STR_SEC_CN                     " 秒"

#define STR_MCCALL_EN                     "MC Calling"
#define STR_CGMCALL_EN                  "CGM Calling"
#define STR_HMCALL_EN                     "HV Calling"
#define STR_CGM_CN                  "大门机"
#define STR_CGM_EN                  "CGM"

#define STR_PRESSANSWER_EN	"Press * to Answer"
#define STR_PRESSANSWER_CN	"按*接听"

#define STR_CALLIN_CN	"正在呼入"
#define STR_HVCALLIN_CN	"室内机正在呼入"

#define STR_LINEBUSY_EN	"Line Busy"
#define STR_LINEBUSY_CN	"线路忙"
#define STR_TRYAGAIN_EN	"Please Try Again"
#define STR_TRYAGAIN_CN	"请稍后重试"

#define STR_CALLEND_EN	"Call Cancelled"
#define STR_CALLEND_CN	"取消呼叫"
#define STR_CONVEREND_EN	"Conversation Over"
#define STR_CONVEREND_CN	"通话结束"

 #define STR_NOANSWER_EN "No Answer"
#define STR_NOANSWER_CN "无人应答"
#define STR_ANSWERLATER_CN "请稍后再呼"

 #define STR_IMGCAP_EN "Image Captured"
#define STR_IMGCAP_CN "正在留图"

 #define STR_TRYLATER_EN "Please Try Later"

#define STR_RESI_EN "Resident's"
#define STR_NUMISNOT_EN "Number is not"
#define STR_REGISTER_EN	"Registered"

#define STR_RESINUM_EN		"Resident Number"
#define STR_NOTREGISTER_EN	"Not Registered"

#define STR_ROOMNUMERROR_CN	"房间号未注册"

#define STR_GATEUNLOCK_EN "Gate Unlocked"
#define STR_GATEUNLOCK_CN "开锁成功"

#define STR_PATROLCARD_EN "Patrol Card"
#define STR_PATROLCARD_CN "巡更卡"

#define STR_IPADDR_EN	"IP Address:"
#define STR_IPADDR_CN	"IP地址:"

#define STR_PHYADDR_EN	"Physical Address:"
#define STR_PHYADDR_CN	"物理地址:"

#define STR_ENTERROR_EN	"Enter Error"
#define STR_ENTERROR_CN	"输入有误"

#define STR_FUNIS_EN			"The Function"
#define STR_NOTENABLED_EN		"Is Not Enabled"
#define STR_FUNISNOTEN_CN		"功能未开通"

#define STR_CARDISNOT_EN		"Card is Not"
#define STR_AUTHORIZED_EN		"Authorized"
#define STR_CARDISNOTAU_CN		"卡未授权"


#define STR_AUTHORIZEDCARD_EN		"Authorized Card"
#define STR_AUTHORIZEDCARD_CN		"授权卡"

#define STR_ENABLED_EN				"Enabled"
#define STR_CARDISNOTENABLED_CN		"卡被禁用"

#define STR_CARDEXPIRED_EN				"Card is Not Valid"//"Card Expired"
#define STR_CARDEXPIRED_CN		"卡不在有效期"//"卡已过期"

#define STR_ETRPWD_EN		"Enter Password+#"
#define STR_ETRPWD_CN		"请输入密码"

#define STR_ETRULKPWD_CN		"请输入开锁密码"
#define STR_ETRPATRULKPWD_CN		"巡更卡,请输入开锁密码"


#define STR_PWDIS_EN			"Password Is"
#define STR_INCORRECT_EN		"Incorrect"
#define STR_PWDINCORRECT_CN		"密码错误"

#define STR_SWIPECARD_EN		"Please Swipe Card"
#define STR_SWIPECARD_CN		"请刷卡"

#define STR_ETR_EN		"Enter"
//#define STR_OLDPWD_EN				"Old Password+#"

#define STR_ETROLD_EN		"Enter Old"
#define STR_SYSPWD_EN				"System Password+#"

#define STR_ETROLDPWD_CN		"请输入旧密码"

#define STR_OLDPWD_EN				"Old Password"
#define STR_OLDPWDJING_EN				"Old Password+#"
#define STR_OLDPWDINCORRECT_CN		"旧密码错误"

#define STR_NEWPWD_EN				"New Password+#"

#define STR_ETRNEWPWD_CN		"请输入新密码"

#define STR_DIGITSERROR_EN		"Digits Error"
#define STR_DIGITSERROR_CN		"密码位数错误"

#define STR_PWDAGAIN_EN		"Password Again+#"

#define STR_ENTERNEWPWD_EN		"Enter New Password+#"
#define STR_AGAIN_EN		"Again"
#define STR_ETRNEWPWDAGAIN_CN		"请再次输入新密码"

#define STR_NOTIDL_EN		"Not Identical"
#define STR_NOTIDL_CN		"密码不一致"

#define STR_ETRPWDAGAIN_CN		"请重新输入"

#define STR_PWDMODIFY_EN		"Password Modified"
#define STR_PWDMODIFY_CN		"密码修改成功"

#define STR_PRESSEXIT_EN				"Press * to Exit"
#define STR_PRESSEXIT_CN				"按*退出"

#define STR_CARDWAS_EN				"Card Was"
#define STR_CARDHASREG_CN				"卡已注册"

#define STR_CARDREGISTERED_EN				"Card Registered"
#define STR_CARDREGISTERED_CN				"卡注册成功"

#define STR_CARDREGISTER_FAIL_EN				"Card Register Fail"
#define STR_CARDREGISTER_FAIL_CN				"卡注册失败"


#define STR_ETRID_EN				"Enter ID"
#define STR_ETRIDJING_EN				"Please Enter ID+#"


#define STR_ETRID_CN				"请输入本地编号"

#define STR_IDNUM_EN					   "ID Number"
#define STR_ISREG_EN						"Is Registered"
#define STR_IDNUMREG_CN				 "ID号已注册"


//#define STR_RESI_EN "Resident's"
//#define STR_NUMISNOT_EN "Number is not"
//#define STR_REGISTER_EN	"Registered"
//#define STR_ROOMNUMERROR_CN	"房间号未注册"

#define STR_RDNUM_EN					   "Room Number"
#define STR_RDNUMNTEX_EN					"Is Not Exist"
#define STR_RDNUMNTEX_CN				  "房间号不存在"

#define STR_CARDDELETED_EN					"Card Deleted"
#define STR_CARDDELETED_CN					"卡删除成功"

#define STR_CARDHASDELETED_EN					"Card Has Deleted"
#define STR_CARDHASDELETED_CN					"卡已删除"

#define STR_ETR_LOCAL_EN						"Enter ID +#"
#define STR_ETR_LOCOALNUM_CN					"请输入本地编号"

#define STR_NOTREGISTERED_EN			  "Not Registered"

#define STR_IDNOTREGISTERED_EN			  "ID Not Registered"

#define STR_ID_NOTREGISTERED_CN		   "ID号未注册"

#define STR_ETRVLD_EN				"Enter"
#define STR_VLDSTATIME_EN				"Valid Start Date +#"
#define STR_VLDENDTIME_EN				"Valid End Date +#"
#define STR_ETRVLDSTATIME_CN				"请输入有效起始日期"
#define STR_ETRVLDENDTIME_CN				"请输入有效终止日期"

#define STR_ETRSYS_EN				"Enter System"
#define STR_ETRSYSPWD_CN	 "请输入系统密码"

#define STR_VOLUME_TIP_EN		"Please Enter Volume+#"
#define STR_VOLUME_TIP_CN		"请输入音量"

#define STR_VOLUME_MODIFIED_TIP_OK_EN		"Volume Modified"
#define STR_VOLUME_MODIFIED_TIP_OK_CN		"音量已修改"
#define STR_VOLUME_MODIFIED_TIP_FAIL_EN		"Enter Error"
#define STR_VOLUME_MODIFIED_TIP_FAIL_CN		"输入有误"



#define AV_REQUEST_I_FRAME	0x02
#define ETHERNET_PROTOCOL_STX	0x68468a86
#define ETHERNET_PROTOCOL_ETX	0x68a86486

#define CORAL_DB_ROOM_CODE	"000"

#define   NTSC				1
#define   PAL				0

#define   JPEG_NTSC			1
#define   JPEG_PAL			0 
#define	  MPEG4_NTSC		2
#define	  MPEG4_PAL			3	 

#define		MPEG4_PAL_MASK	0x30102
#define		MPEG4_NTSC_MASK	0x40102

#define ATM_HEAD_LEN_V3		0x0A
#define ATM_TYPE1_LEN		0x10
#define ATM_TYPE1_V3_LEN	0x3A
#define ATM_OLD_UINT_LEN	0x10
#define ATM_TYPE2_LEN       0x64

#define	ATM_VERSION_OLD		0x10000
#define ATM_VERSION_2		0x10200
#define ATM_VERSION_3		0x10300

#define ATM_VERSION_4		0x10400
#define ATM_HEAD_LEN_V4		0x0B
#define ATM_TYPE1_V4_LEN	0x3A
#define ATM_TYPE1_V4_MCLEN	0x38



#define ATM_TYPE_NODEVICE   0x00
#define ATM_TYPE_GM			0x01
#define ATM_TYPE_HV			0x02
#define ATM_TYPE_MC			0x03
#define ATM_TYPE_EHV		0x05
#define ATM_TYPE_EGM		0x06
#define ATM_FLASH_LEN		0x200000

#define MAX_PHOTO_JPEG_LEN	0x40000


#define LINE_SZ_PAL    704
#define NUM_LINES_PAL  576
#define LINE_SZ_NTSC   704
#define DISPLY_LINE_SZ_PAL  720
#define DISPLY_LINE_SZ_NTSC 720
#define NUM_LINES_NTSC 480
#define BIT16FILELEN	0xC6042

#define PMT_INPUT_ROOMCODE						"请输入房间号"
#define PMT_ROOMCODE_NOT_EXIST				"房间号未注册"
#define PMT_PWD_ERROR									 "密码错误"
#define PMT_OLD_PWD_ERROR							"旧密码错误"
#define PMT_NEW_PWD_ERROR							"新密码错误"
#define PMT_CARD_OPER										"请刷卡 按#保存，*退出"
#define PMT_ROOMCODE_NO_CARD_CN				"房间号未注册卡"
#define PMT_ROOMCODE_NO_CARD_EN				"No Card"


#ifdef NEW_OLED_ENABLE
#define SCREEN_HEIGHT		64
#define SCREEN_WIDTH		128
#define SCREEN_COLOR		1
#else
#define SCREEN_HEIGHT		240
#define SCREEN_WIDTH		320
#define SCREEN_COLOR		18
#endif



#define KEY_ROOM_CANCEL		0 
#define KEY_ROOM_1			1 
#define KEY_ROOM_2			2
#define KEY_ROOM_3			3
#define KEY_ROOM_4			4
#define KEY_ROOM_5			5
#define KEY_ROOM_6			6
#define KEY_ROOM_7			7
#define KEY_ROOM_8			8
#define KEY_ROOM_9			9
#define KEY_ROOM_10			10
#define KEY_ROOM_11			11
#define KEY_ROOM_12			12
#define KEY_ROOM_13			13
#define KEY_ROOM_14			14
#define KEY_ROOM_15			15





#define KEY_NUM_0		0x30
#define KEY_NUM_1		0x31 
#define KEY_NUM_2		0x32
#define KEY_NUM_3		0x33
#define KEY_NUM_4		0x34
#define KEY_NUM_5		0x35
#define KEY_NUM_6		0x36
#define KEY_NUM_7		0x37
#define KEY_NUM_8		0x38
#define KEY_NUM_9		0x39
#define KEY_NUM_ON		0x2A //*
#define KEY_NUM_ENTER	0x23 //#
#define KEY_DB_CALL		0x40

#define KEY_UP		KEY_NUM_2 
#define KEY_DOWN	KEY_NUM_8 
#define KEY_LEFT	KEY_NUM_4 
#define KEY_RIGHT	KEY_NUM_6 
#define KEY_RETURN  KEY_NUM_ON
#define KEY_ENTER	KEY_NUM_ENTER

#define KEY_BUF_LEN (19+1)

#define TIMER_MAIN				101
#define TIMER_MAIN_MENU			102
#define TIMER_SYSCONFIG_WND		103
#define TIMER_LANGSET_PMT		104
#define TIMER_SYSPWD_MODIFY		105
#define TIMER_SYSINFO			106
#define TIMER_MAN_OPEN_SET		107
#define TIMER_OPEN_MODE_SET		108
#define TIMER_RESI_MNG			109
#define TIMER_ADD_CARD			110
#define TIMER_DEL_CARD_MODE		111
#define TIMER_DEL_LOCALNUM_MODE	112
#define TIMER_PWDMODIFY			113
#define TIMER_PWDUNLCOK			114
#define TIMER_HELPMSG			115
#define TIMER_PRJ				116
#define TIMER_AS_PROC			117
#define TIMER_AS_PWDMOD			118
#define TIMER_SYS_RESTORE		119
#define TIMER_SYS_RESTART		120
#define TIMER_ADD_ACD			122
#define TIMER_AS_SET			123
#define TIMER_AS_MAIN			124
#define TIMER_NETWORKSET_WND	125
#define TIMER_DELMODE_WND		126
#define TIMER_GATE_PARA			127
#define TIMER_OTHER_PARA		128
#define TIMER_GATE_PULSE_WIDTH		129
#define TIMER_ROOM_CODE_DIGIT		 130
#define TIMER_DEL_RDNUM_MODE		131
#define TIMER_INFRARED_SET			132
#define TIMER_IVDCARDSWIPE_SET			133
#define TIMER_GATEOVERTIME_SET			134
#define TIMER_ASALARM					135
#define TIMER_TALK_TIME				136
#define TIMER_RING_TIME				137
#define TIMER_TALK_VOLUME				138




#define PROMPT_SHOW_TIME		2000
#define INTERFACE_SETTLE_TIME	30000

#define WM_CARDREAD							(WM_USER + 100)
#define WM_REFRESHPARENT_WND				(WM_USER + 101)
#define WM_SETTIMER							(WM_USER + 102)
#define WM_RDPASSWORD						(WM_USER + 103)

//#define BACKGROUND_COLOR	RGB(127, 152, 168)
#define BACKGROUND_COLOR	RGB(0, 0, 0)
#define FONT_COLOR						RGB(255, 0, 0)
#define FONT_COLOR_SEL  		  RGB(0, 255, 0)
#define PROMPT_FONT					  RGB(150,150,0)

#define NAME_LEN						50
#define RD_CODE_LEN						(19+1)
#define PWD_LEN							(12+1)
#define CSN_LEN							5
#define TIME_LEN						8
#define MAX_CDDATA_READ_LEN			 64
#define MAX_UART_DATA_LEN			 1024
#define MAX_PACKET_DATA_LEN			 64

#define MAX_PWD_LEN		12
#define MIN_PWD_LEN     4

#define TITLE_BUF_LEN				50

#define MAX_WINDOWS_NUM             100

#define WND_MEUN_PRIORITY_1			1
#define WND_CALL_PRIORITY_2			2
#define WND_CARD_PRIORITY_3			3
#define WND_UNLOCK_PRIORITY_4		4
#define WND_ALARM_PRIORITY_5		5

#ifndef __SUPPORT_ADD_AND_SUBTRACT_CARDS__ /*AaronWu, For support addition and subtraction card function*/
#define __SUPPORT_ADD_AND_SUBTRACT_CARDS__
#endif

/************** TYPEDEFS **************************************************************************************/


/************** STRUCTURES ************************************************************************************/
typedef struct  _WNDINFO
{
	HWND CurWndHdle;
	BYTE Priority;	
} WNDINFO;

typedef struct  _WNDMAN
{
	HWND MainWndHdle;
	INT  nChildWndNum;
	WNDINFO *pChildWndInfo;
} WNDMAN;


/************** EXTERNAL DECLARATIONS *************************************************************************/
extern WNDMAN    g_WndMan;

extern BOOL bTamperAla; 
extern BOOL bGateAla;
/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

extern CHAR g_KeyInputBuffer[KEY_BUF_LEN];
extern INT  g_KeyInputBufferLen;
extern CHAR g_KeyInputBkp[KEY_BUF_LEN];
extern INT  g_KeyInputBkpLen;
extern	  DWORD g_HwVer;

extern LONG GetXPos(CHAR *pBuf);
extern void MXDrawText_En(HDC Hdc, RECT* pRect, char *pBuf, UCHAR Count);
extern void MXDrawText_Cn(HDC Hdc, RECT* pRect, char *pBuf, UCHAR Count);
extern void MXDrawText_Hebrew(HDC Hdc, RECT* pRect, char *pBuf, UCHAR Count);
extern void	MulLineDisProc(HDC Hdc, CHAR *pBuf, INT nBufLen, UCHAR CountStart);

extern void MXDrawText_Left(HDC Hdc, CHAR *pBuf,  LONG xPos, UCHAR LineStart);
extern void MXDrawText_Right(HDC Hdc, CHAR *pBuf,  LONG xPos, UCHAR LineStart);
extern VOID MXDrawText_Center(HDC Hdc, CHAR *pBuf,  UCHAR LineStart);
extern VOID MXDrawText_Center_24(HDC Hdc, CHAR *pBuf,  UCHAR LineStart);
extern VOID MXDrawText_Center_48(HDC Hdc, CHAR *pBuf,  UCHAR LineStart);


extern VOID DrawLine(HDC Hdc, int Fx1, int Fy1, int Fx2, int Fy2);
extern VOID DrawEnglishCur(HDC Hdc, int xPos, int linePos);
extern VOID DrawChineseCur(HDC Hdc, int xPos, int linePos);
extern VOID DrawHebrewCur(HDC Hdc, int xPos, int linePos);

extern VOID DrawPageUp(HDC Hdc);
extern VOID DrawPageDown(HDC Hdc);




extern CHAR KeyBoardMap(WPARAM wParam);
extern void	DrawDataParaSet(HDC Hdc, CHAR *pBuf, UCHAR Index);

extern VOID WatchDogInit();
extern VOID WatchDog();

extern VOID StartPlayRightANote();
extern VOID StartPlayArrearsNote();
extern VOID	StartPlayErrorANote();
extern VOID	StartPlayAlarmANote();
extern VOID	StopPlayAlarmANote();
extern VOID 	StartPlayDI1ANote();
extern VOID 	StartPlayDI2ANote();

extern void UnlockGate();
extern void UnlockGateStart(BOOL LCenable, unsigned char mode, DWORD IP, CHAR RdNum[RD_CODE_LEN], BYTE CSN[CSN_LEN]);
extern void UnlockGateEnd();

extern BOOL CodeCompare(CHAR *pInCode, CHAR *pTableCode, INT InCodeLen);

extern VOID ClearKeyBuffer();
extern VOID SaveKeyBuffer();
extern VOID LoadKeyBuffer();

extern VOID ResetTimer (HWND hWnd , UINT nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc);
extern VOID	CircleBufEnter(CHAR* pKeyBuf, INT* pKeyBufLen, INT nMaxBufLen, CHAR KeyValue);
extern VOID	CircleKeyEnter(CHAR* pKeyInputBuffer, UCHAR *pDataIndex, INT MaxDataLen, CHAR KeyValue);

extern	BOOL	IsAlarming();
extern HWND g_hMainWnd;
extern VOID	CloseChildWnd(HWND hWnd);

extern void	MwHookProcess();


extern BOOL AddOneWnd(HWND hWnd,INT Priority);
extern BOOL RemoveOneWnd(HWND hWnd);
extern HFONT	GetHZKFont32(void);
extern char * GetHebrewStr(HEBREW_STR_ID StrID);
/**************************************************************************************************************/
#endif // MXCOMMON_H































































