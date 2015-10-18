/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	MXMsg.h
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		13 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef MXMSG_H
#define MXMSG_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/
#define GENE_ICON
#define ENGLISH_VERSION

// (Talking Control <->  Ethernet <-> Talking)			
// GM pickup 
#define	MXMSG_PICKUP_GM				0x0602	


// (Talking <-> Ethernet, Talking <-> Mulit-Media)		
// Start redirect video/audio from Multi-Media to Ethernet and from Ethernet to Multi-Media
#define	MXMSG_STA_RDIR_ETH_MM_AV	0xF003	

// (Talking <-> Ethernet, Talking <-> Mulit-Media)		
// Stop redirect video/audio from Multi-Media to Ethernet and from Ethernet to Multi-Media
#define	MXMSG_STP_RDIR_ETH_MM_AV	0xF004	

// (Talking <-> Ethernet <-> Talking Control)								
// Talking notify Talking Control GM hangup
#define	MXMSG_HANGUP_GM				0x0606

// Ethernet Module send data ok or fail send notify to the source Module
#define	MXMSG_ETH_SENDDATA_NTY		0xF0000001

#define	MXMSG_PLY_ALM_A				0xF0000002
#define	MXMSG_STP_ALM_A				0xF0000003

#define	MXMSG_STA_MM2ETH_V			0xF007	
#define	MXMSG_STP_MM2ETH_V			0xF008	

#define	MXMSG_STA_ETH2MM_A_MM2ETH_AV	0xF009	
#define	MXMSG_STP_ETH2MM_A_MM2ETH_AV	0xF00A	

//Talking -> multimedia
#define MXMSG_PLY_CALL_A				0xF00B
#define MXMSG_STP_CALL_A				0xF00C

//Error sound Note
#define MXMSG_PLY_ERROR_A				0xF01D
#define MXMSG_STP_ERROR_A				0xF01E

//Key sound Note
#define MXMSG_PLY_KEY_A					0xF020
#define MXMSG_STP_KEY_A					0xF021

//Right sound Note
#define MXMSG_PLY_RIGHT_A				0xF022
#define MXMSG_STP_RIGHT_A				0xF023

//Arrears sound Note
#define MXMSG_PLY_ARREARS_A				0xF030
#define MXMSG_STP_ARREARS_A				0xF031


//DI1 DI2 sound note
#define MXMSG_PLY_DI1_A				0xF024
#define MXMSG_PLY_DI2_A				0xF025

//Ring when a new message received
#define MXMSG_PLY_MSG_NOTE				0xF00D
#define MXMSG_PLAY_TALK_TEST			0xF00E
#define MXMSG_PLAY_RING_TEST			0xF00F

#define MXMSG_PLY_HDB_AUDIO				0xF010
#define MXMSG_STP_HDB_AUDIO				0xF011

#define MXMSG_PLY_GM_V					0xF012
#define MXMSG_STP_GM_V					0xF013

#define MXMSG_STA_LEAVE_AUDIO			0xF014
#define MXMSG_STP_LEAVE_AUDIO			0XF015
#define MXMSG_PLY_A_V					0XF016
#define MXMSG_STP_A_V					0XF017
#define MXMSG_STA_SEND_EXMSG			0XF018
#define MXMSG_STP_SEND_EXMSG			0XF019

#define MXMSG_STA_LEAVE_MSG_AV			0XF01A
#define MXMSG_STP_LEAVE_MSG_AV			0XF01B

#define MXMSG_STA_RECV_EXMSG			0XF01C
#define MXMSG_STP_RECV_EXMSG			0XF01D


#define MXMSG_STA_HDB_V_CAPPLY		0xF0000B
#define MXMSG_STP_HDB_V_CAPPLY		0xF0000C

//About Alarm Clock
#define MXMSG_ALARMCLOCK_AUDIO			0xF0000D
#define MXMSG_ALARMCLOCK_TALKING		0xF0000E
#define MXMSG_ALARMCLOCK_SCURITYALARM	0xF0000F


#define MXMSG_START_HDB_PIC				0XF020
#define MXMSG_STOP_HDB_PIC				0XF021

#define MXMSG_PLAY_MUSIC_TEST			0XF022
#define MXMSG_STP_MUSIC_TEST			0XF023

#define MXMSG_START_TAKE_PHOTO			0XF024
#define MXMSG_STOP_TAKE_PHOTO			0XF025

//EGM <-> EGM
#define	MXMSG_STA_RDIR_ETH_MM_A         0xF026
#define	MXMSG_STP_RDIR_ETH_MM_A			0xF027

#define	MXMSG_STA_CALLRING         		0xF028
#define	MXMSG_STP_CALLRING				0xF029	

//Access system
#define	MXMSG_AS_CARDNUM         		0xF040
#define	MXMSG_AS_KEYVAL					0xF041	

//OLED
#define	MXMSG_OLED_DIS_INFO			0xF050

//Take photo
#define MXMSG_SEND_PHOTO_BMP		0xF060
#define MXMSG_SAVE_PHOTO			0xF061
#define MXMSG_SEND_PHOTO_JPG		0xF062

////////////////////////////////////////////////////////////////
#define	FC_IMSG_SEND				0x0001
#define	FC_IMSG_BRADCAST			0x0002
#define	FC_IMSG_GETCOUNT			0x0003
#define	FC_IMSG_QUERY_MSG			0x0004
#define	FC_IMSG_SETSTATE			0x0005
#define	FC_IMSG_DELETE				0x0006
#define FC_IMSG_QUERY_LIST			0x0007
#define FC_IMSG_NOTIFY				0x0008
#define FC_IMSG_SEND_END			0x000A
#define FC_IMSG_SEND_CANCEL			0x000B

#define	FC_MNT_START				0x0401
#define	FC_MNT_CANCEL				0x0402
#define	FC_MNT_INTERRUPT			0x0403
#define	FC_CALL_HV_MC				0x0404
#define	FC_CALL_HV_HV				0x0405
#define FC_CALL_HV_GM				0x0412
#define	FC_CALL_GM_HV				0x0406
#define	FC_CALL_GM_MC				0x0407
#define	FC_CALL_GM_GM				0x0408
#define	FC_CALL_MC_HV				0x0409
#define	FC_CALL_MC_GM				0x040A
#define	FC_PICKUP_HV				0x040B
#define	FC_PICKUP_GM				0x040C
#define	FC_PICKUP_MC				0x040D
#define	FC_HANGUP_HV				0x040E
#define	FC_HANGUP_GM				0x040F
#define	FC_HANGUP_MC				0x0410
#define	FC_UNLK_GATE				0x0411
#define FC_CALL_HV_GM				0x0412
#define FC_CALL_BROADCAST			0x0413
#define FC_CALL_STP_BROADCAST		0x0414
#define FC_PICKUP_NTY				0x0415
#define FC_HANGUP_NTY				0x0416
#define FC_QUERY_CHILDREN			0x0417
#define FC_REDIRECT_MC				0x0418
#define FC_QUERY_HDB				0x0419

#define FC_CALL_FORWARD				0x041A
#define	FC_UNLOCK_NOTIFY_GM			0x0421


//#define	FC_ALM_REPORT				0x0801
#define	FC_ALM_REPORT				0x0802
#define FC_ALM_REPORT_MHV			0x0803

//#define FC_ALM_NEW_NOTIFY			0x0802	//MHV -> SHV
//#define FC_ALM_NEW_REPORT			0x0803	//SHV -> MHV
//Security Alarm Log
#define FC_SAL_QUERY_LIST			0x0805
#define FC_SAL_QUERY_CNT			0x0806
#define FC_SAL_QUERY_LOG			0x0807
#define FC_SAL_QUERY_DEL			0x0808
#define FC_SAL_STATUS_NOTIFY		0x0809



#define FC_TL_QUERY_LIST			0x0811
#define FC_TL_QUERY_CNT				0x0812
#define FC_TL_QUERY_LOG				0x0813
#define FC_TL_QUERY_DEL				0x0814

#define FC_TL_STATUS_NOTIFY			0x0815
#define FC_TL_REPORT_TO_MHV			0x0816

#define FC_TL_QUERY_PIC				0x0817
#define FC_TL_SEND_PIC				0x0818


#define FC_SA_SET_NOTIFY			0x0821
#define FC_SA_SET_REPORT			0x0822

#define FC_SA_ZONE_QUERY			0x0823
#define FC_SA_ZONE_SET				0x0824
#define FC_SA_ALARM_NOTIFY			0x0825
#define FC_SA_ALARM_CONFIRM_NOTIFY	0X0826

#define	FC_DTM_ADJUST				0x0C01
#define	FC_LINE_LOCK				0x0C02
#define	FC_LINE_FREE				0x0C03
#define	FC_HO_QUERY					0x0C04
#define FC_STATE_DIAGNOSE			0x0C05
#define	FC_DTM_GET					0x0C06
#define FC_HO_GET_COUNT				0x0C08
#define FC_HO_GET					0x0C09
#define FC_PSW_CHANGE				0x0c0a
#define FC_HO_GETCHILDREN_COUNT		0x0C0B
#define FC_HO_GETCHILDREN			0x0C0C

#define FC_GET_SYSINFO				0x0C10
#define FC_GET_FUNCINFO				0x0C11

#define	FC_AV_SENDDATA				0x1001
#define	FC_AV_REQUEST_VIDEO_IVOP	0x1002
#define FC_AV_TAKEPHOTO				0x1003
#define FC_AV_REQUEST_DECARGS		0x1006
#define FC_ACK_AV_REQUEST_DECARGS	0x9006

#define FC_AV_STREAM_ANNOUNCEMENT		0x1007
#define FC_ACK_AV_STREAM_ANNOUNCEMENT	0x9007
#define FC_AV_STREAM_REQUEST			0x1008
#define FC_ACK_AV_STREAM_REQUEST		0x9008
#define FC_AV_START						0x1004
#define FC_ACK_AV_START					0x9004
#define FC_AV_STOP						0x1005
#define FC_ACK_AV_STOP					0x9005

#define	FC_CNF_DWNLD_PRG			0x1401
#define	FC_CNF_CHK_PRG				0x1402
#define	FC_CNF_FALSH_BURN_PER		0x1403

#define FC_REPORT_LOGDATA			0x0C0D
#define FC_REPORT_PIC				0x0C0E

#define FC_AC_REPORT_EVENT			0x1A01
#define FC_ACK_AC_REPORT_EVENT		0x9A01

#define FC_AC_OPEN_GATE				0x1A02
#define FC_ACK_AC_OPEN_GATE			0x9A02

#define FC_AC_POLL					0x1A03
#define FC_ACK_AC_POLL				0x9A03

#define FC_AC_REPORT_CSN			0x1A04
#define FC_AC_REPORT_CSN_ACK		0x9A04

#define FC_AC_SWIPE_CARD			0x1A0C
#define FC_ACK_AC_SWIPE_CARD		0x9A0C

#define FC_AC_PWD_CHECK				0x1A0D
#define FC_ACK_AC_PWD_CHECK			0x9A0D

#define FC_AC_PWD_MODIFY			0x1A0E
#define FC_ACK_AC_PWD_MODIFY		0x9A0E

#define FC_AC_RSD_PWD				0x1A0F
#define FC_ACK_AC_RSD_PWD			0x9A0F

#define FC_AC_CARD_PASSWORD_MOD     0x1A11
#define FC_ACK_AC_CARD_PASSWORD_MOD 0x9A11

#define FC_CNF_DWNLD_FILE			0x1404
#define FC_ACK_CNF_DWNLD_FILE		0x9404

#define FC_CNF_UPLD_FILE			0x1405
#define FC_ACK_CNF_UPLD_FILE		0x9405

#define FC_CNF_REMOVE_FILE			0x1406
#define FC_ACK_CNF_REMOVE_FILE		0x9406

//#ifdef __SUPPORT_ADD_AND_SUBTRACT_CARDS__
#define	FC_AC_EVENT_CNT					0x1A05
#define	FC_ACK_AC_EVENT_CNT				0x9A05

#define	FC_AC_EVENT_READ				0x1A06
#define	FC_ACK_AC_EVENT_READ			0x9A06

#define	FC_AC_LAST_SWIPE_V1				0x1A10
#define	FC_ACK_AC_LAST_SWIPE_V1			0x9A10

#define	FC_AC_ADD_CARD					0x1A08
#define	FC_ACK_AC_ADD_CARD				0x9A08

#define	FC_AC_DEL_CARD					0x1A09
#define	FC_ACK_AC_DEL_CARD				0x9A09

#define	FC_AC_EDIT_CARD					0x1A0A
#define	FC_ACK_AC_EDIT_CARD				0x9A0A

#define	FC_AC_GET_CARD					0x1A0B
#define	FC_ACK_AC_GET_CARD				0x9A0B

#define FC_AC_GET_CARD_CNT				0x1A20
#define FC_ACK_AC_GET_CARD_CNT			0x9A20

#define FC_AC_GET_CARD_V2               0x1A21
#define FC_ACK_AC_GET_CARD_V2           0x9A21

//Access and one card 
/*~~~~~~~~~~~~~~~~~~~~~~~~*/
#define FC_AC_MC_ADD_ONE_CARD			0x1A23
#define FC_ACK_AC_MC_ADD_ONE_CARD		0x9A23

#define FC_AC_MC_DEL_ONE_CARD			0x1A24
#define FC_ACK_AC_MC_DEL_ONE_CARD		0x9A24

#define FC_AC_MC_EDIT_ONE_CARD			0x1A25
#define FC_ACK_AC_MC_EDIT_ONE_CARD		0x9A25

#define FC_AC_MC_QUERY_CARD				0x1A26
#define FC_ACK_AC_MC_QUERY_CARD			0x9A26
/*~~~~~~~~~~~~~~~~~~~~~~~~*/

#define FC_PC_MC_ADD_ONE_CARD			0x2401
#define FC_ACK_PC_MC_ADD_ONE_CARD		0xA401

#define FC_PC_MC_DEL_ONE_CARD			0x2402
#define FC_ACK_PC_MC_DEL_ONE_CARD		0xA402

#define FC_PC_MC_EDIT_ONE_CARD			0x2403
#define FC_ACK_PC_MC_EDIT_ONE_CARD		0xA403

#define FC_PC_MC_QUERY_CARD				0x2404
#define FC_ACK_PC_MC_QUERY_CARD			0x2404

/*~~~~~~~~~~~~~~~~~~~~~~~~*/






//#endif

//#ifdef __SUPPORT_PROTOCOL_900_1A__
#define MSG_STATUS_MASK					0x4000
#define MSG_ACK_MASK					0x8000

#define	CATEGORY_HC_PUBLIC				0x0100
#define	CATEGORY_HC_TALKING				0x0200
#define	CATEGORY_HC_SA					0x0300
#define	CATEGORY_HC_MULTIMEDIA			0x0400
#define	CATEGORY_HC_CONFIG				0x0500
#define	CATEGORY_HC_AUTHORIZATION		0x0600
#define	CATEGORY_HC_ACCESS				0x0700
#define	CATEGORY_HC_VISION				0x0800
///////////////////////////////////////////////////////////////
//900.1a
//////////////////////////////////////////////////////////////

//PUBLIC FUNCTION
#define	FCA_PB_VER_NOTIFY			(CATEGORY_HC_PUBLIC|0x01)
#define	FCA_PB_S2C_HEART_BEAT		(CATEGORY_HC_PUBLIC|0x02)
#define	FCA_PB_C2S_HEART_BEAT		(CATEGORY_HC_PUBLIC|0x03)
#define	FCA_PB_DATA_SYNC			(CATEGORY_HC_PUBLIC|0x04)
#define	FCA_PB_RECORD_UPDATE		(CATEGORY_HC_PUBLIC|0x05)
#define	FCA_PB_DEV_ONLINE			(CATEGORY_HC_PUBLIC|0x06)
#define	FCA_PB_FILE_SYNC			(CATEGORY_HC_PUBLIC|0x07)


//PUBLIC FUNCTION ACK
#define	FCA_ACK_PB_VER_NOTIFY		(FCA_PB_VER_NOTIFY|MSG_ACK_MASK)
#define	FCA_ACK_PB_DATA_SYNC		(FCA_PB_DATA_SYNC|MSG_ACK_MASK)
#define	FCA_ACK_PB_RECORD_UPDATE	(FCA_PB_RECORD_UPDATE|MSG_ACK_MASK)
#define	FCA_ACK_PB_DEV_ONLINE		(FCA_PB_DEV_ONLINE|MSG_ACK_MASK)
#define	FCA_ACK_PB_FILE_SYNC		(FCA_PB_FILE_SYNC|MSG_ACK_MASK)

//TALKING
#define FCA_IC_CALL					(CATEGORY_HC_TALKING|0x01)
#define FCA_IC_PICKUP				(CATEGORY_HC_TALKING|0x02)
#define FCA_IC_HANGUP				(CATEGORY_HC_TALKING|0x03)
#define FCA_IC_UNLK					(CATEGORY_HC_TALKING|0x04)
#define FCA_IC_RECORD				(CATEGORY_HC_TALKING|0x05)
#define FCA_IC_NTY_RING				(CATEGORY_HC_TALKING|0x06)
#define FCA_IC_NTY_CANCEL			(CATEGORY_HC_TALKING|0x07)
#define FCA_IC_NTY_UPDATE			(CATEGORY_HC_TALKING|0x08)
//TALKING ACK
#define FCA_ACK_IC_CALL					(FCA_IC_CALL|MSG_ACK_MASK)
#define FCA_ACK_IC_PICKUP				(FCA_IC_PICKUP|MSG_ACK_MASK)
#define FCA_ACK_IC_HANGUP				(FCA_IC_HANGUP|MSG_ACK_MASK)
#define FCA_ACK_IC_UNLK					(FCA_IC_UNLK|MSG_ACK_MASK)
#define FCA_ACK_IC_RECORD				(FCA_IC_RECORD|MSG_ACK_MASK)
#define FCA_ACK_IC_NTY_RING				(FCA_IC_NTY_RING|MSG_ACK_MASK)
#define FCA_ACK_IC_NTY_CANCEL			(FCA_IC_NTY_CANCEL|MSG_ACK_MASK)
#define FCA_ACK_IC_NTY_UPDATE			(FCA_IC_NTY_UPDATE|MSG_ACK_MASK)
//ACCESS
#define FCA_AC_REQUEST_ISSUE_CARD		(CATEGORY_HC_ACCESS|0x01)
#define FCA_AC_SWIPE_CARD				(CATEGORY_HC_ACCESS|0x02)
//ACCESS ACK
#define FCA_ACK_AC_REQUEST_ISSUE_CARD	(FCA_AC_REQUEST_ISSUE_CARD|MSG_ACK_MASK)

//SA FUNCITON
#define	FCA_SA_CONFIRM_ALARM			(CATEGORY_HC_SA|0x01)
#define	FCA_SA_ARM_DISARM				(CATEGORY_HC_SA|0x02)
#define	FCA_SA_NTY_EMERGENCY_ALARM		(CATEGORY_HC_SA|0x03)
#define	FCA_SA_SETTING					(CATEGORY_HC_SA|0x04)

//SA FUNCITON ACK
#define	FCA_ACK_SA_CONFIRM_ALARM		(FCA_SA_CONFIRM_ALARM|MSG_ACK_MASK)
#define	FCA_ACK_SA_ARM_DISARM			(FCA_SA_ARM_DISARM|MSG_ACK_MASK)
#define	FCA_ACK_SA_NTY_EMERGENCY_ALARM	(FCA_SA_NTY_EMERGENCY_ALARM|MSG_ACK_MASK)

//MULTI MEDIA
#define FCA_MM_AV_STREAM_REQUEST		(CATEGORY_HC_MULTIMEDIA|0x01)
//MULTI MEDIA ACK
#define FCA_ACK_MM_AV_STREAM_REQUEST	(FCA_MM_AV_STREAM_REQUEST|MSG_ACK_MASK)

//CONFIGURATION FUNCTION
#define	FCA_CG_READ_PARA				(CATEGORY_HC_CONFIG|0x01)
#define	FCA_CG_WRITE_PARA				(CATEGORY_HC_CONFIG|0x02)
#define	FCA_CG_UPLD_END					(CATEGORY_HC_CONFIG|0x03)
#define	FCA_CG_UPDATE_END				(CATEGORY_HC_CONFIG|0x04)
#define FCA_CG_READ_GM_PARA				(CATEGORY_HC_CONFIG|0x05)
#define FCA_CG_WRITE_GM_PARA			(CATEGORY_HC_CONFIG|0x06)


//CONFIGURATION FUNCTION ACK
#define	FCA_ACK_CG_READ_PARA			(FCA_CG_READ_PARA|MSG_ACK_MASK)
#define	FCA_ACK_CG_WRITE_PARA			(FCA_CG_WRITE_PARA|MSG_ACK_MASK)
#define	FCA_ACK_CG_UPLD_END				(FCA_CG_UPLD_END|MSG_ACK_MASK)
#define	FCA_ACK_CG_UPDATE_END			(FCA_CG_UPDATE_END|MSG_ACK_MASK)
#define FCA_ACK_CG_READ_GM_PARA			(FCA_CG_READ_GM_PARA|MSG_ACK_MASK)
#define FCA_ACK_CG_WRITE_GM_PARA		(FCA_CG_WRITE_GM_PARA|MSG_ACK_MASK)

//#define	FCA_ACK_HC_READ_PARA			(FCA_HC_READ_PARA|MSG_ACK_MASK)
//#define	FCA_ACK_HC_WRITE_PARA			(FCA_HC_WRITE_PARA|MSG_ACK_MASK)
//#define	FCA_ACK_HC_CNF_UPLD_END			(FCA_HC_CNF_UPLD_END|MSG_ACK_MASK)
//#define	FCA_ACK_HC_UPDATE_END			(FCA_HC_UPDATE_END|MSG_ACK_MASK)

//AUTHORIZATION
#define	FCA_PB_LOGIN					(CATEGORY_HC_AUTHORIZATION|0x01)
////AUTHORIZATION ACK
#define	FCA_ACK_PB_LOGIN				(FCA_PB_LOGIN|MSG_ACK_MASK)

//VISION
#define	FCA_VISION_DEV_REGISTER			(CATEGORY_HC_VISION|0x01)
#define	FCA_VISION_TAG_NOTIFY			(CATEGORY_HC_VISION|0x02)
#define	FCA_VISION_TAG_SET				(CATEGORY_HC_VISION|0x03)
#define	FCA_VISION_TAG_ADD				(CATEGORY_HC_VISION|0x04)
#define FCA_ACK_VISION_DEV_REGISTER		(FCA_VISION_DEV_REGISTER|MSG_ACK_MASK)
#define FCA_ACK_VISION_TAG_NOTIFY		(FCA_VISION_TAG_NOTIFY|MSG_ACK_MASK)
#define FCA_ACK_VISION_TAG_SET			(FCA_VISION_TAG_SET|MSG_ACK_MASK)
#define FCA_ACK_VISION_TAG_ADD			(FCA_VISION_TAG_ADD|MSG_ACK_MASK)

//#endif  __SUPPORT_PROTOCOL_900_1A__

////////////////////////////////////////////////////////////////

#define	FC_ACK_IMSG_SEND			0x8001

#define	FC_ACK_IMSG_GETCOUNT		0x8003
#define	FC_ACK_IMSG_QUERY_MSG		0x8004
#define	FC_ACK_IMSG_SETSTATE		0x8005
#define	FC_ACK_IMSG_DELETE			0x8006
#define FC_ACK_IMSG_QUERY_LIST		0x8007
#define FC_ACK_IMSG_NOTIFY			0x8008
#define FC_ACK_IMSG_SEND_END		0x800A
#define FC_ACK_IMSG_SEND_CANCEL		0x800B



#define	FC_ACK_MNT_START			0x8401
#define	FC_ACK_MNT_CANCEL			0x8402
#define	FC_ACK_MNT_INTERRUPT		0x8403
#define	FC_ACK_CALL_HV_MC			0x8404
#define	FC_ACK_CALL_HV_HV			0x8405
#define FC_ACK_CALL_HV_GM			0x8412
#define	FC_ACK_CALL_GM_HV			0x8406
#define	FC_ACK_CALL_GM_MC			0x8407
#define	FC_ACK_CALL_GM_GM			0x8408
#define	FC_ACK_CALL_MC_HV			0x8409
#define	FC_ACK_CALL_MC_GM			0x840A
#define	FC_ACK_PICKUP_HV			0x840B
#define	FC_ACK_PICKUP_GM			0x840C
#define	FC_ACK_PICKUP_MC			0x840D
#define	FC_ACK_HANGUP_HV			0x840E
#define	FC_ACK_HANGUP_GM			0x840F
#define	FC_ACK_HANGUP_MC			0x8410
#define	FC_ACK_UNLK_GATE			0x8411
#define FC_ACK_QUERY_CHILDREN		0x8417

#define FC_ACK_QUERY_HDB			0x8419

#define FC_ACK_CALL_FORWARD			0x841A

#define	FC_ACK_ALM_REPORT			0x8802
#define FC_ACK_ALM_NEW_REPORT		0x8803


//Security Alarm Log
#define FC_ACK_SAL_QUERY_LIST		0x8805
#define FC_ACK_SAL_QUERY_CNT		0x8806
#define FC_ACK_SAL_QUERY_LOG		0x8807
#define FC_ACK_SAL_QUERY_DEL		0x8808

#define FC_ACK_TL_QUERY_LIST		0x8811
#define FC_ACK_TL_QUERY_CNT			0x8812
#define FC_ACK_TL_QUERY_LOG			0x8813
#define FC_ACK_TL_QUERY_DEL			0x8814

#define FC_ACK_TL_QUERY_PIC			0x8817
#define FC_ACK_TL_SEND_PIC			0x8818
#define FC_ACK_TL_REPORT_MHV		0x8819

#define FC_ACK_SA_ZONE_QUERY		0x8823
#define FC_ACK_SA_ZONE_SET			0x8824

#define	FC_ACK_HO_QUERY				0x8C04
#define FC_ACK_STATE_DIAGNOSE		0x8C05
#define	FC_ACK_DTM_GET				0x8C06
#define FC_ACK_HO_GET_COUNT			0x8C08
#define FC_ACK_HO_GET				0x8C09
#define FC_ACK_PSW_CHANGE			0x8c0a
#define FC_ACK_HO_GETCHILDRENCOUNT	0x8C0B
#define FC_ACK_HO_GETCHILDREN		0x8C0C

#define FC_ACK_GET_SYSINFO			0x8C10
#define FC_ACK_GET_FUNCINFO			0x8C11
#ifdef __SUPPORT_WIEGAND_CARD_READER__
#define FC_ACK_GET_CONFINFO         0x8C12
#define FC_GET_CONFINFO				0x0C12
#endif



#define FC_ACK_AV_TAKEPHOTO			0x9003

#define FC_ACK_REPORT_LOGDATA		0x8C0D

// lift control
#define FC_LF_STOP				0x1B01
#define FC_LF_STOP_UP			0x1B02
#define FC_LF_STOP_DOWN			0x1B03
#define FC_LF_A_B				0x1B04
#define FC_LF_UNLOCK			0x1B05
#define FC_LF_STATE				0x1B06
#define FC_LF_STATE_NOTIFY		0x1B07
#define FC_LF_HEARTBEAT			0x1B08


#define FC_ACK_LF_STOP				0x9B01
#define FC_ACK_LF_STOP_UP			0x9B02
#define FC_ACK_LF_STOP_DOWN			0x9B03
#define FC_ACK_LF_A_B				0x9B04
#define FC_ACK_LF_UNLOCK			0x9B05
#define FC_ACK_LF_STATE				0x9B06
#define FC_ACK_LF_STATE_NOTIFY		0x9B07

		

///////////////////////////////////////////////////////////////

#define MXMSG_CLOSE_WINDOW          0xFFFE //added by [MichaelMa] at 30.8.2012

#define	MXMSG_SECURITY_ALARM		0xF0000004
#define MXMSG_SEND_KEYVALUE			0xF0000005
#define MXMSG_STATUS_CHANGE			0xF0000006
#define MXMSG_VIDEO_CAP_ON			0xF0000007
#define MXMSG_VIDEO_CAP_OFF			0xF0000008
#define MXMSG_AUDIO_CAP_ON			0xF0000009
#define MXMSG_AUDIO_CAP_OFF			0xF000000A
#define MXMSG_AUDIO_PLY_ON			0xF000000B
#define MXMSG_AUDIO_PLY_OFF			0xF000000C
#define COMM_SAVE_PHOTO				0xF000000F

#define COMM_TAKEPHOTO_END			0xF0000012

#define COMM_UNLOCK_LINKAGE_ON			 0xF0000013
#define COMM_UNLOCK_LINKAGE_OFF			0xF0000014

#define COMM_YUV2JPG				0xF0000015

#define COMM_CCDLED_ON				0xF0000016
#define COMM_CCDLED_OFF			0xF0000017

#define COMM_UNLOCK_DO1			0xF0000018
#define COMM_UNLOCK_DO2			0xF0000019

#define MXMSG_SECURITY_ALARM_CONFIRM       		0xF0000030

#define COMM_OVERTIME_ALARM_ON			0xF0000031
#define COMM_OVERTIME_ALARM_OFF			0xF0000032

#define COMM_GP_VIDEO_ON			 0xF0000033
#define COMM_GP_VIDEO_OFF			0xF0000034

#define MXMSG_MOX2LIFT_HV2HV				0xF0000035

//#define KEY_MONITOR				VK_F1
//#define	KEY_HANDFREE			VK_F2
//#define KEY_ZOOM				VK_F3
//#define KEY_ESC					VK_F4
//#define	KEY_UP					VK_F5
//#define KEY_DOWN				VK_F6
//#define KEY_ENTER				VK_F7
//#define KEY_CALL				VK_F8
//#define KEY_UNLOCK				VK_F9
//#define KEY_MODE_SWITCH			VK_F10


#define ID_ICON_RETURN		0
#define ID_ICON_UP			1
#define ID_ICON_DOWN		2
#define ID_ICON_ENTER		3

#define GENE_ICON_NUM		4



#define HZ_HEIGHT		48

#define HZ_HEIGHT_32	32
#define HZ_HEIGHT_24	24




#define MDFUNC_IRISMSG		0X00000001
#define MDFUNC_RMETER		0X00000002
#define MDFUNC_ALARM		0X00000004
#define MDFUNC_CALL			0X00000008
#define MDFUNC_MONITOER		0X00000010
#define MDFUNC_GSMMSG		0X00000020

#define FLAG_SELF_GM        0xE0000001

//Lift control
#define MXMSG_LC_CALL_LIFT	0XF100
#define MXMSG_GM_CALL_LIFT	0XF101
// Lift contorl driver
#define MXMSG_LF_STOP		0XF102
#define MXMSG_LF_A_B		0XF103

/************** TYPEDEFS **************************************************************************************/

typedef	struct _GENEICONMenu
{
	int						nId;
	char					nImageID[256];
	short					nX1Pos;
	short					nY1Pos;
	short					nX2Pos;
	short					nY2Pos;
} GENEICONMENU;


typedef	struct _ICONBUTTON
{
	int						nId;
	char					szName[28];
	char					szImage[256];
	char					szImageS[256];
	short					nxPos;
	short					nyPos;
	short					nX1Pos;
	short					nY1Pos;
	short					nX2Pos;
	short					nY2Pos;
} ICONBUTTONMENU;
/************** STRUCTURES ************************************************************************************/

/************** EXTERNAL DECLARATIONS *************************************************************************/

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif // MXMSG_H

