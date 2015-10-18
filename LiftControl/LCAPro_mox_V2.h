/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	LCAPro_mox_V2.h
**
**	AUTHOR:		Harry Qian
**
**	DATE:		28 - Sep - 2010
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef LCAPRO_MOX_V2_H
#define LCAPRO_MOX_V2_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/
#define M2CCMT_FILE	"/mox/rdwr/m2ccmt.ini"

#define M2CCMT_GLOBAL_SECTION			"Global"
#define M2CCMT_LIFT_SECTION				"Lift%d"
#define M2CCMT_CARDREADER_SECTION		"CardReader%d"
#define M2CCMT_USER_SECTION				"User%d"
#define M2CCMT_DOOR_SECTION				"Door%d"

#define M2CCMT_KEY_LIFT_CNT				"LiftCount"
#define M2CCMT_KEY_USER_CNT				"UserCount"
#define M2CCMT_KEY_CARDREADER_CNT		"CardReaderCount"
#define M2CCMT_KEY_DOOR_CNT				"DoorCount"

#define M2CCMT_KEY_LIFT_DA1				"DA1"
#define M2CCMT_KEY_LIFT_DA2				"DA2"
#define M2CCMT_KEY_LIFT_DA3				"DA3"
#define M2CCMT_KEY_LIFT_GROUP			"Group"

#define M2CCMT_KEY_CARDREADER_LIFTID			"LiftID"
#define M2CCMT_KEY_CARDREADER_STATIONNUM		"StationNum"
#define M2CCMT_KEY_CARDREADER_LIFTINDEX			"LiftIndex"

#define M2CCMT_KEY_USER_AMTCODE				"AMTcode"
#define M2CCMT_KEY_USER_DNSCODE				"DNScode"
#define M2CCMT_KEY_USER_DOORCODE			"DoorCode"
#define M2CCMT_KEY_USER_LEVEL				"Level"
#define M2CCMT_KEY_USER_TYPE				"DoorType"
#define M2CCMT_KEY_USER_LIFTID				"LiftID"

#define M2CCMT_KEY_DOOR_DOORCODE			"DoorCode"
#define M2CCMT_KEY_DOOR_LEVEL				"Level"
#define M2CCMT_KEY_DOOR_TYPE				"DoorType"



#define MOX2_LOCATION_TYPE_NULL				0x00
#define MOX2_LOCATION_TYPE_GM				0x01//门口机
#define MOX2_LOCATION_TYPE_HV				0x02//室内机
#define MOX2_LOCATION_TYPE_LIFT_CAR			0x03//轿厢
#define MOX2_LOCATION_TYPE_HALL				0x04//大堂
#define MOX2_LOCATION_TYPE_LIFT_CONTROLLER	0xFE//梯控控制器

#define MOX2_IDENTITY_TYPE_ADMIN	(1<<0)//管理员
#define MOX2_IDENTITY_TYPE_GRIL		(1<<1)//女人
#define MOX2_IDENTITY_TYPE_KID		(1<<2)//小孩
#define MOX2_IDENTITY_TYPE_OLD		(1<<3)//老人
#define MOX2_IDENTITY_TYPE_VIP		(1<<4)//VIP
#define MOX2_IDENTITY_TYPE_GUEST	(1<<5)//访客
#define MOX2_IDENTITY_TYPE_NOSTOP	(1<<14)//直驶

#define MOX2_LIFT_INDEX_BROADCAST	0xff

#define MOX2_FRONT_DOOR_FLAG	0x00
#define MOX2_BACK_DOOR_FLAG		0x01

/************** STRUCTURES ************************************************************************************/

typedef struct _MOXLIFT
{
	unsigned char cDA1;
	unsigned char cDA2;
	unsigned char cDA3;
	unsigned char cGroup;
}MOXLIFT;

typedef struct _MOXDOOR
{
	char szDoorCode[20];
	char			cLevel;
	char		cDoorType;
}MOXDOOR;

typedef struct _MOXCARDREADER
{
	unsigned char cLiftID;
	unsigned char	 cStationNum;
	unsigned char cLiftIndex;
}MOXCARDREADER;


typedef struct _MOXUSER
{
	char szDNSCode[20];
	char szAMTCode[20];
	char szDoorCode[20];
	unsigned char cLiftID;
	char			cLevel;
	char			cDoorType;
}MOXUSER;
/************** EXTERNAL DECLARATIONS *************************************************************************/





extern void		LCAMOX2ProtoInit(void);
extern void		LCAMox2ThreadFun(void);
extern void		LCAMOX2ProtoExit(void);

extern void		LCAMox2WaitingAckPro(void);

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here
//extern void Mox2_CallLiftA2B(MXMSG* pmsg);


extern MOXUSER * GetpUserByCode(char * szUserCode,char * szDoorCode);
extern MOXLIFT	* GetpLiftByLiftID(unsigned char cLiftID);
extern MOXDOOR * GetpDoorByCode(char * szDoorCode);
extern MOXLIFT	* GetpLiftByCardReaderStationNum(unsigned char cStationNum,unsigned char * pcLiftIndex);
/**************************************************************************************************************/
#endif //






















