/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	LiftCtrlByDOModule.h
**
**	AUTHOR:		Jason Wang
**
**	DATE:		2012-04-20
**
**	FILE DESCRIPTION:
**		Control list by DO module
**
**	FUNCTIONS:
**
**	NOTES:
**
*/
#ifndef _LIFT_CTRL_BY_DO_MODULE_
#define _LIFT_CTRL_BY_DO_MODULE_
/************** SYSTEM INCLUDE FILES **************************************************/
 #include <pthread.h>
 #include "MXCommon.h"
/************** USER INCLUDE FILES ***************************************************/
 
/************** DEFINES **************************************************************/

// ���ֺ��ݷ�ʽ
#define VISTOR_UNLOCK			0x1 // �ÿͿ���
#define HV_CALL_LIFT			0x2 // ���ڻ�����
#define HV_CALLING_HV_CALL_LIFT 0x3 // �����Խ�����
#define SWIPER_CARD_UNLOCK		0x4 // ˢ������
#define SWIPER_CARD_IN_LIFT		0x5 // ������ˢ��

#define ENTRY_TYPE_DEF			0x0010	//  Ĭ��
#define ENTRY_TYPE_DOOR			0x0020	// �ſڻ�
#define ENTRY_TYPE_LIFTCAR		0x0040	// ����
#define ENTRY_TYPE_FRONT		0x0080	// ǰ
#define ENTRY_TYPE_BACK			0x0100	// ��
#define VALUE_FRONT_DOOR	0
#define VALUE_BACK_DOOR		1

#define CALL_LIFT_BY_DOOR		0x00
#define CALL_LIFT_BY_RESIDENT	0x01

#define TYPE_DEFAULT		0x0001
#define TYPE_LIFT_NO		0x0002
#define TYPE_VISITOR			0x0004
#define TYPE_HV_CALL_LIFT	0x0008
#define TYPE_HV_CALL_HV		0x0010
#define TYPE_SWIPER_CARD	0x0020

#define CALL_LIFT_WAY_KEY					0x00 // ���ݷ�ʽ: ����
#define CALL_LIFT_WAY_UNLOCK				0x02 // ���ݷ�ʽ: ����
#define CALL_LIFT_WAY_KEY_AND_UNLOCK		0x01 // ���ݷ�ʽ: ����+ ����

#define PRESS_KEY_UP 0x0
#define PRESS_KEY_DOWN 0x1
#define PRESS_KEY_UNLOCK 0x2

#define COMM_WAY_ETH	0x00
#define COMM_WAY_485	0x01

#define RETRY_TIMES		2

#define CODE_ADMIN	"9999999999999999999"
#define CODE_INVALID "8888888888888888888"

#define IDENTITY_LEN RD_CODE_LEN

typedef enum
{
	DO_1 = 0,
	DO_2,
	DO_3,
	DO_4,
	DO_5,
	DO_6,
	DO_7,
	DO_8,
	DO_9,
	DO_10,
	DO_11,
	DO_12,
	DO_13,
	DO_14,
	DO_15,
	DO_16,	
	DO_INVALID = 255,
}PORT_ID;

/************** TYPEDEFS *************************************************************/
 
/************** STRUCTURES ***********************************************************/
typedef struct _COMMON_PARA
{
	WORD pulse_width_1; // ���Ƶ��ݷÿͽ�����������
	WORD pulse_width_2; // ���Ƶ��ݽ��������������
	WORD pulse_width_3; // ���Ƶ��ݰ������ݵ�������
	BYTE call_lift_way_1; // �ÿͿ������ݷ�ʽ
	BYTE call_lift_way_2; // �����Խ����ݷ�ʽ
	BYTE call_lift_way_3; // ˢ���������ݷ�ʽ	
	DWORD reserved_1;
	DWORD reserved_2;
	DWORD reserved_3;
	DWORD reserved_4;
}COMMON_PARA;

typedef struct _RESIDENT_INFO
{
	BYTE resident_code[IDENTITY_LEN]; // ס����
	SHORT floor; // ס������¥��
	BYTE lift_No_1; // �ÿͿ������ݺ�
	BYTE lift_No_1_door; // �ÿͿ�������ǰ����
	BYTE lift_No_2; // ���ڻ����ݵ��ݺ�
	BYTE lift_No_2_door; // ���ڻ����ݵ���ǰ����
	BYTE lift_No_3; // �����Խ����ݺ�
	BYTE lift_No_3_door; // �����Խ�����ǰ����
	BYTE lift_No_4; // ˢ���������ݺ�
	BYTE lift_No_4_door; // ˢ����������ǰ����
	DWORD reserved_1;
	DWORD reserved_2;
	DWORD reserved_3;
	DWORD reserved_4;
}RESIDENT_INFO;

typedef struct _LIFT_PARA 
{
	BYTE lift_No;
	BYTE lift_type;
	DWORD card_reader_addr; // ��õ��ݽ����ڵĶ�������ַ
	DWORD reserved_1;
	DWORD reserved_2;
	DWORD reserved_3;
	DWORD reserved_4;
}LIFT_PARA;

typedef struct _FUNC_PARA
{
	SHORT floor; // ¥��
	BYTE lift_No; // ���ݺ�
	BYTE func; // ���ϣ����£�����
	BYTE door_type; // 0-ǰ��1-����2-ǰ����
	DWORD reserved_1;
	DWORD reserved_2;
	DWORD reserved_3;
	DWORD reserved_4;
}FUNC_PARA;

typedef struct _DO_MODULE
{
	BYTE comm_way; //4 ��DO ģ���ͨѶ��ʽ
	DWORD ip; //4 DO ģ���IP ��ַ
	WORD TCP_port; //4 TCP �˿ں�
	WORD UDP_port; //4 UDP �˿ں�
	BYTE modbus_addr; //4 DO ģ��վ��
	DWORD baud_rate; //4 ������
	BYTE data_bits; //4 ����λ
	BYTE parity_flag; //4 У��λ
	BYTE stop_bits; //4 ֹͣλ
	FUNC_PARA DO1_func;
	FUNC_PARA DO2_func;	
	FUNC_PARA DO3_func;
	FUNC_PARA DO4_func;	
	FUNC_PARA DO5_func;
	FUNC_PARA DO6_func;	
	FUNC_PARA DO7_func;
	FUNC_PARA DO8_func;	
	FUNC_PARA DO9_func;
	FUNC_PARA DO10_func;	
	FUNC_PARA DO11_func;
	FUNC_PARA DO12_func;	
	FUNC_PARA DO13_func;
	FUNC_PARA DO14_func;	
	FUNC_PARA DO15_func;
	FUNC_PARA DO16_func;
	DWORD reserved_1;
	DWORD reserved_2;
	DWORD reserved_3;
	DWORD reserved_4;
}DO_MODULE;

typedef struct _DOOR_ACCESS_CONTROL_PARA
{
	BYTE door_code[IDENTITY_LEN]; //4 �Ŵ��롾���һλΪ'\0'��
	SHORT floor; //4 ������¥�� ������Ϊ������
	BYTE call_lift_type; //4 ��������
	BYTE lift_No; //4 ���ݺ�
	BYTE door_type; //4 0-ǰ��1-����2-ǰ����
	DWORD reserved_1;
	DWORD reserved_2;
	DWORD reserved_3;
	DWORD reserved_4;
}DAC_PARA;

typedef struct _THREAD_SYNC
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	BOOL flag;
}THREAD_SYNC;

extern THREAD_SYNC thread_sync;
/************** EXTERNAL DECLARATIONS ************************************************/
extern void lift_ctrl_by_DO_module_init(void);
extern BOOL modbus_LT_para_init(void);
extern void ctrl_lift(const DWORD src, DWORD type, DWORD addr, const unsigned char *code);
extern BOOL LC_thread_state();
//!!!  It is H/H++ file specific, nothing should be defined here
 
/************** ENTRY POINT DECLARATIONS *********************************************/
 
/************** LOCAL DECLARATIONS ***************************************************/
 
/*************************************************************************************/

#endif 

