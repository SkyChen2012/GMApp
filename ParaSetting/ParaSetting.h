/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	tskParaSetting.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		16 - Oct - 2007
**
**	FILE DESCRIPTION:
**			Mox Modnet definition files
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef TSKPARASETTING_H
#define TSKPARASETTING_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/
#include "MXTypes.h"
/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/
#define   CNF_QUERY_AMT_VERSION	 0x02

#define   FILE_FIRST_PACKET      0x01
#define   FILE_MIDDLE_PACKET     0x02
#define   FILE_END_PACKET        0x03
//////////////////////
#define   Set_Type_Error         0x03  //distinguish UM or GP
#define   Edition_Error          0x02
#define	  ATM_VERSION_ERROR		 0x07
#define	  SETTING_BUSY_ERROR	 0x05
#define	  ATM_LEN_OVERFLOW		 0x06
#define   FLASH_ERR		         0x04   

#define   FLASH_START        0x90000000
#define   FLASH_KEY1         0xaa
#define   FLASH_KEY2         0x55
#define   FLASH_KEY3         0xa0
#define   FLASH_KEY4         0x80
#define   FLASH_KEY5         0x10
#define   FLASH_KEY6         0x30

//command

#define SET_HC_ADDR						0x02
#define RE_SET_HC_ADDR					0x82
#define CNF_WRITE_TMPERIOD				0x03
#define RE_CNF_WRITE_TMPERIOD			0x83
#define CNF_READ_TMPERIOD				0x04
#define RE_CNF_READ_TMPERIOD			0x84
#define CNF_DWNLD_PRG					0x05
#define RE_CNF_DWNLD_PRG				0x85
#define CFG_CHK_PRG						0x06
#define RE_CFG_CHK_PRG					0x86   
#define CNF_DWNLD_AMT					0x07
#define RE_CNF_DWNLD_AMT				0x87
#define CNF_UPLD_AMT					0x08
#define RE_CNF_UPLD_AMT					0x88 
#define CNF_FLASH_BURN_PER				0x09
#define	CNF_QUERY_PARAMETER				0x0a
#define	RE_CNF_QUERY_PARAMETER			0x8a

#define COMM_READ_FACTORY_PARA			0x0B 
#define COMM_ACK_READ_FACTORY_PARA		0x8B 

#define COMM_WRITE_FACTORY_PARA 		0x20
#define COMM_ACK_WRITE_FACTORY_PARA		0xA0


#define COMM_WRITE_SYS_PARA				0x0C 
#define COMM_ACK_WRITE_SYS_PARA			0x8C 
#define COMM_READ_SYS_PARA				0x0D 
#define COMM_ACK_READ_FSYS_PARA			0x8D 

#define COMM_WRITE_AS_PARA				0x0E 
#define COMM_ACK_WRITE_AS_PARA			0x8E 
#define COMM_READ_AS_PARA				0x0F 
#define COMM_ACK_READ_AS_PARA			0x8F 

#define COMM_REQ_CARD					0x12
#define COMM_ACK_REQ_CARD				0x92

#define COMM_DOWNLOAD_CARD				0x13
#define COMM_ACK_DOWNLOAD_CARD			0x93

#define COMM_UPLOAD_CARD				0x14
#define COMM_ACK_UPLOAD_CARD			0x94

#define CNF_DWNLD_HELPINFO				0x15
#define RE_CNF_DWNLD_HELPINFO			0x95

#define CNF_CLEAR_HELPINFO				0x16
#define RE_CNF_CLEAR_HELPINFO			0x96

#define COMM_DOWNLOAD_CARD				0x13
#define COMM_ACK_DOWNLOAD_CARD			0x93

#define CNF_WRITE_ROOMCODE				0x22
#define RE_CNF_WRITE_ROOMCODE			0xA2
#define CNF_READ_ROOMCODE				0x23
#define RE_CNF_READ_ROOMCODE			0xA3


#define CNF_WRITE_LIFTCTRL				0x24	
#define RE_CNF_WRITE_LIFTCTRL			0xA4	
#define CNF_READ_LIFTCTRL				0x25	
#define RE_CNF_READ_LIFTCTRL			0xA5	

#define COMM_WRITE_SYS_PARA_V2          0x26
#define COMM_ACK_WRITE_SYS_PARA_V2      0xA6
#define COMM_READ_SYS_PARA_V2           0x27
#define COMM_ACK_READ_FSYS_PARA_V2      0xA7

//Factory parameter
#define FACTORY_MODULETYPE_OFFSET		0x00
#define FACTORY_MODULETYPE_LEN			0x20

#define FACTORY_MODULETYPE_WRITE_OFFSET 0x04

#define FACTORY_PARALEN_OFFSET			0x20
#define FACTORY_PARALEN_LEN				0x02

#define FACTORY_PARAVER_OFFSET			0x22
#define FACTORY_PARAVER_LEN				0x04

#define FACTORY_HARDVER_OFFSET			0x26
#define FACTORY_HARDVER_LEN				0x04

#define FACTORY_HARDVER_WRITE_OFFSET	0x24

#define FACTORY_MACADDR_OFFSET			0x2A
#define FACTORY_MACADDR_LEN				0x06

#define FACTORY_SOFTVER_OFFSET			0x30
#define FACTORY_SOFTVER_LEN				0x13

#define FACTORY_DATA_LEN				0x43

//System parameter

#define SYS_LANGUAGESET_OFFSET		0x00
#define SYS_LANGUAGESET_LEN			0x01

#define SYS_IPADDR_OFFSET			0x01
#define SYS_IPADDR_LEN				0x04

#define SYS_RINGSEL_OFFSET			0x05
#define SYS_RINGSEL_LEN				0x01

#define SYS_SYSPWD_OFFSET			0x06
#define SYS_SYSPWD_LEN				0x13

#define SYS_RESICODEVLD_OFFSET		0x1F
#define SYS_RESICODEVLD_LEN			0x01

#define SYS_TAMPERALARM_OFFSET		0x20
#define SYS_TAMPERALARM_LEN			0x01

#define SYS_CAMERASEL_OFFSET		0x21
#define SYS_CAMERASEL_LEN			0x01

#define SYS_FUNDI1_OFFSET			0x22
#define SYS_FUNDI1_LEN				0x01

#define SYS_FUNDI2_OFFSET			0x23
#define SYS_FUNDI2_LEN				0x01

#define SYS_FUNDO1_OFFSET			0x24
#define SYS_FUNDO1_LEN				0x01

#define SYS_FUNDO2_OFFSET			0x25
#define SYS_FUNDO2_LEN				0x01

#define SYS_FUNTALKTIME_OFFSET			0x26
#define SYS_FUNTALKTIME_LEN			0x04

#define SYS_FUNRINGTIME_OFFSET			0x2a
#define SYS_FUNRINGTIME_LEN			0x04

//#define SYS_LCENABLE_OFFSET			0x2e
//#define SYS_LCENABLE_LEN			0x01
//
//#define SYS_LCMODE_OFFSET			0x2f
//#define SYS_LCMODE_LEN				0x01
//
//#define SYS_LC_COM_MODE_OFFSET			0x30
//#define SYS_LC_COM_MODE_LEN				0x01

#define SYS_FUNDO1_PULSETIME_OFFSET		0x2e//	0x31
#define SYS_FUNDO1_PULSETIME_LEN			0x01

#define SYS_FUNDO2_PULSETIME_OFFSET		0x2f//	0x32
#define SYS_FUNDO2_PULSETIME_LEN		0x01

#define SYS_MCIP_SERVER_OFFSET			0x30
#define SYS_MCIP_SERVER_LEN				0x04

#define SYS_MCIP_CLIENT_OFFSET			0x34
#define SYS_MCIP_CLIENT_LEN				0x04

#define SYS_TALK_VOLUME_OFFSET			0x38
#define SYS_TALK_VOLUME_LEN				0x04

#define SYS_UNLOCK_TIME_OFFSET			0x3C
#define SYS_UNLOCK_TIME_LEN				0x04


#define SYS_DATA_LEN			0x40

#define ROOM_CODE_DATA_LEN			0xF7
//Access Control System

#define PU_FUNVLD_OFFSET			0x00
#define PU_FUNVLD_LEN				0x01

#define PU_MNGPWD_OFFSET			0x01
#define PU_MNGPWD_LEN				0x13

#define PU_RESIPWD_OFFSET			0x14
#define PU_RESIPWD_LEN				0x13

#define AS_WGCODE_OFFSET			0x27
#define AS_WGCODE_LEN				0x01

#define AS_GATEOPENDLYT_OFFSET		0x28
#define AS_GATEOPENDLYT_LEN			0x01

#define AS_GOOTA_OFFSET				0x29
#define AS_GOOTA_LEN				0x01

#define AS_IVLDCARDT_OFFSET			0x2A
#define AS_IVLDCARDT_LEN			0x01

#define AS_IVLDCARDA_OFFSET			0x2B
#define AS_IVLDCARDA_LEN			0x01

#define AS_DOORMANOPEN_OFFSET		0x2C
#define AS_DOORMANOPEN_LEN			0x01

#define AS_GATEOPENMODE_OFFSET		0x2D
#define AS_GATEOPENMODE_LEN			0x01

#define AS_GATERELAYPULSE_OFFSET	0x2E
#define AS_GATERELAYPULSE_LEN		0x02

#define AS_MCUNLOCKENABLE_OFFSET	0x30
#define AS_MCUNLOCKENABLE_LEN		0x01

#define AS_GATECONTACTOR_OFFSET		0x31
#define AS_GATECONTACTOR_LEN		0x01

#define AS_ENABLEAS_OFFSET			0x32
#define AS_ENABLEAS_LEN				0x01

#define AS_ACCSERVERIP_OFFSET		0x33
#define AS_ACCSERVERIP_LEN			0x04

#define AS_ACCDEVCODE_OFFSET			0x37
#define AS_ACCDEVCODE_LEN				0x13

#ifdef __SUPPORT_WIEGAND_CARD_READER__
#define AS_ACCWIEGANDBITNUM_OFFSET	0x57
#define AS_ACCWIEGANDBITNUM_LEN		0x01

#define AS_ACCWIEGANDREVERSE_OFFSET	0x58
#define AS_ACCWIEGANDREVERSE_LEN	0x01

#define AS_DATA_LEN					0x59
#else
#define AS_DATA_LEN					0x4A
#endif 

#define AS_FORCEULKREPORT_OFFSET    0x59


#define CARD_REQ_INFO_LEN			0x02
#define CARD_REQ_LENTH_LEN			0x04

#define CARD_DLD_INFO_LEN			0x02
#define CARD_DLD_RDCODE_LEN			0x13
#define CARD_DLD_LENTH_LEN			0x04
#define CARD_DLD_PWD_LEN			0x13
#define CARD_DLD_CDTYPE_LEN			0x01


// lift control system parameter
#define LC_ENABLE_OFFSET		0X00
#define LC_ENABLE_LEN			0X01

#define LC_MODE_OFFSET		0X01
#define LC_MODE_LEN			0X01

#define LC_FUNCTION_OFFSET	0X02
#define LC_FUNCTION_LEN		0X01

#define LC_LCAIP_OFFSET	0X03
#define LC_LCAIP_LEN		0X04

#define LCA_ENABLE_OFFSET	0X07
#define LCA_EBABLE_LEN	0X01

#define LCA_COMM_MODE_OFFSET	0X08
#define LCA_COMM_MODE_LEN	0X01


#define LC_DATA_LEN		9

//CONF_TYPE

#define CONF_TYPE_WIEGAND 		1
#define CONF_TYPE_ACCESSCARD 	2
#define CONF_TYPE_MAINTKERNEL 	3
#define CONF_TYPE_SORTINGALGORITHM 	4
#define CONF_TYPE_ONECARD		5


#ifdef __SUPPORT_PROTOCOL_900_1A__
typedef enum
{
	MOX_P_900_1 = 1,
	MOX_P_900_1A,
}MOX_PROTOCOL_TYPE;
#endif 
/************** TYPEDEFS **************************************************************************************/

typedef struct _CARDULDINFO
{
	char CSN[32];
	int  nCardLen;
	BOOL bCardUpLoad;	
}CARDULDINFO;

typedef struct  {
    DWORD     START;
    UCHAR      CMD;
    WORD     DATALEN;
    DWORD     FRAMEIDX;
    DWORD     ERR;
    DWORD     RESERVED;
    DWORD     END;
}E2PROM_SETTING;

typedef struct
{
    CHAR uIsChanged;
    CHAR BackupPN[32];
    CHAR BackupHardVer[19];
}VERSION_CONTROL;

/************** STRUCTURES ************************************************************************************/

/************** GLOBAL VARIABLE DEFINITIONS *******************************************************************/

//!!!  It is C/C++ file specific, nothing should be defined here

/************** EXTERNAL DECLARATIONS *************************************************************************/
extern VERSION_CONTROL g_VerContrl;
extern UCHAR  DownloadDataBuf[0x800000];
extern UCHAR  FLAG_AMT_BURN;

extern UCHAR	Linkflag;
extern UCHAR	Pack_Flag;

extern CARDULDINFO g_CardUldInfo;

extern BOOL g_bNeedRestart;

extern void ParaSettingInit();
#ifdef __SUPPORT_PROTOCOL_900_1A__
extern BOOL MOX_PROTOCOL_SUPPORT(MOX_PROTOCOL_TYPE nType);
#endif
/**************************************************************************************************************/
#endif // TSKPARASETTING_H
