/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	LiftCtrlByDOModule.c
**
**	AUTHOR:		Jason Wang
**
**	DATE:		2012-04-20
**
**	FILE DESCRIPTION:
**		Control lift through DO module 
**
**	FUNCTIONS:
**
**	NOTES:
**
*/
/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <sys/timeb.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>  
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <termios.h>
#include <errno.h>
#include <sys/select.h> 
#include <sys/mman.h> 

/************** USER INCLUDE FILES ***************************************************/
#include "LiftCtrlByDOModule.h"
#include "modnet.h" 
#include "modbus.h"
#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"
#include "BacpNetCtrl.h"
#include "MenuParaProc.h"

/************** DEFINES **************************************************************/
//#define DO_DBG 
#define DO_PORT_NUM 16
#define IN			0
#define OUT			1
#define WAY_COUNT	2
#define DELAY_TIME  1000 //4 针对连续发DO 信号丢失的问题
#define UCHAR2SHORT(a)	(a & 0x80? (a&(~0x80))-128:a)
/************** TYPEDEFS *************************************************************/
 
/************** STRUCTURES ***********************************************************/

typedef struct _RESIDENTS_HEAD
{
	DWORD count;
	RESIDENT_INFO *head;
}RESIDENTS_HEAD;

typedef struct _LIFTS_HEAD
{
	DWORD count;
	LIFT_PARA *head;	
}LIFTS_HEAD;

typedef struct _DO_MODULES_HEAD
{
	DWORD count;
	DO_MODULE *head;		
}DO_MODULES_HEAD;

typedef struct _DOOR_ACCESS_CONTROL_HEAD
{
	DWORD count;
	DAC_PARA *head;
}DAC_HEAD;

typedef struct _TIME_DO_MODULE
{
	DO_MODULE	DO_MODULE;
	BYTE port_id;
	BYTE cmd;
	unsigned short count;
	unsigned short value;
	DWORD tickStart;
	DWORD entryType;
}TIME_DO_MODULE_t; 
typedef struct _TIMER_BUF
{
	TIME_DO_MODULE_t tData;
	BOOL	TimerOnFlag;//default FALSE
}TIME_BUF_DO_MODULE_t;
#define DO_MAX_TIMER_COUNT	20
static TIME_BUF_DO_MODULE_t DO_TIMER_BUF[DO_MAX_TIMER_COUNT];

/************** EXTERNAL DECLARATIONS ************************************************/
extern SYSCONFIG g_SysConfig; 
//!!!  It is H/H++ file specific, nothing should be defined here
 
/************** ENTRY POINT DECLARATIONS *********************************************/
 
/************** LOCAL DECLARATIONS ***************************************************/
THREAD_SYNC thread_sync;
static BOOL lc_thread_state = FALSE;
static BOOL modbus_LT_para_init_state = FALSE;
static pthread_t lift_ctrl_pid = 0; 
static COMMON_PARA common_para;
static LIFTS_HEAD lifts;
static RESIDENTS_HEAD residents;
static DO_MODULES_HEAD DO_modules;
static DAC_HEAD DAC;

static void free_msg(MXMSG* pMsg);
static void* lift_ctrl_by_DO_module_fun(void* arg);
static BOOL send_cmd_to_DO(const DO_MODULE *DO_Module, const BYTE port_id, BYTE cmd, unsigned short count, unsigned short value,DWORD entryType);

static RESIDENT_INFO *get_resident_node(const char *code);
static RESIDENT_INFO *get_resident_node_type(const char *code, const DWORD type, const BYTE value);
static LIFT_PARA *get_lift_No(BYTE card_reader_addr);
static DO_MODULE *get_DO_module_and_port(FUNC_PARA spara, PORT_ID *port_id);
static BOOL comm_with_DO_by_eth(unsigned int rip, unsigned short rport, BYTE cmd, unsigned short start_addr, unsigned short count, unsigned short value); 
static BOOL call_lift(const BYTE lift_No, const short floor, const DWORD call_lift_way, const BYTE opt);
static BOOL my_memcpy(BYTE *dest, BYTE *start, int offset, int len, off_t size);
static void unlock_all_floors(const BYTE lift_No, const BYTE opt);
static void send_ack_to_card_reader(BYTE val, DWORD rip);
static void DoTimerProc(void);//定时处理
static BOOL TimeSetDo(TIME_DO_MODULE_t tData);//设置定时处理
static void TimerSendCmd2DO(TIME_DO_MODULE_t tData);
static BOOL DoTimerBufInit();
#ifdef DO_DBG
static void DoTimerBufShow();
#endif
static BOOL _visitor_call_lift(char* , char*);
static BOOL _HV_call_lift(char*, char);
static BOOL _HV_call_HV_call_lift(char*, char*);
static BOOL _swiper_card_call_lift(char*, char*);
static BOOL _liftcar_swiper_card(char*, BYTE, DWORD);
/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    my_memcpy
**   Owner: Jason Wang
**	DATE:		2012-04-28 
**	
**	DESCRIPTION:	
**		用mmap() 读取文件初始化参数时避免用memcpy造成段错误
**	ARGUMENTS:	
**		dest: 目标地址
**		start: 映射文件的起始地址
**		offset: 要拷贝的字符串源地址在映射文件中的偏移
**		len: 要拷贝的字符长度
**		size: 映射文件的总长度
**	RETURNED VALUE:	
**		TRUE: copy success; FALSE: copy failure
**	NOTES:
**	
*/
static BOOL my_memcpy(BYTE *dest, BYTE *start, int offset, int len, off_t size)
{
	if (offset+len <= size)
	{
		memcpy(dest, start+offset, len);		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    modbus_LT_para_init
**   Owner: Jason Wang
**	DATE:		2012-04-28 
**	
**	DESCRIPTION:	
**		从配置文件中读取该模块的参数
**	ARGUMENTS:	
**		NULL
**	RETURNED VALUE:	
**		TRUE:初始化参数成功；FALSE:初始化失败
**	NOTES:
**	
*/
BOOL modbus_LT_para_init()
{
	int fd = 0;
	int i = 0;
	int j = 0;
	DWORD port_count = 0; // 配置的port 个数
	BYTE port_num = 0; // 已经配置的port index	
	struct stat file_stat;
	BYTE *start = 0;
	off_t offset = 0;
	BOOL rst_flag = FALSE;
	
	memset(&file_stat, 0, sizeof(struct stat));	

	if( LC_BY_DO_MODULE != g_SysConfig.LC_ComMode)
	{
		return FALSE;
	}
	
	
	fd = open(DOLTFILE, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	if (-1 == fd)
	{
		printf("%s:%s:%dn error\n", __FILE__, __FUNCTION__, __LINE__);
		perror("123");
		goto error1;
	}
	if (-1 == stat(DOLTFILE, &file_stat))
	{
		printf("%s:%s:%dn error\n", __FILE__, __FUNCTION__, __LINE__);
		perror("123");
		goto error2;
	}
	printf("Jason--->file_stat.st_size=%ld\n", file_stat.st_size);
	start = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, offset);
	if (MAP_FAILED == start)
	{
		printf("%s:%s:%dn error\n", __FILE__, __FUNCTION__, __LINE__);
		perror("123");
		goto error2;
	}
	offset += 2 * sizeof(DWORD); // version + length = 2
	//电梯设置初始化
	rst_flag = my_memcpy((BYTE *)&(lifts.count), start, offset, sizeof(lifts.count), file_stat.st_size);
	if (rst_flag == FALSE)
	{
		goto error3;
	}
	offset += sizeof(lifts.count);	
	offset += 4; //4 lifts 数据长度
	if(lifts.head != NULL)
	{
		free(lifts.head);
		lifts.head = NULL;
	}
	lifts.head = (LIFT_PARA *)malloc(lifts.count * sizeof(LIFT_PARA));
	memset(lifts.head,0,lifts.count * sizeof(LIFT_PARA));
	if (NULL == lifts.head)
	{
		printf("%s:%s:%dn error\n", __FILE__, __FUNCTION__, __LINE__);
		goto error3;
	}	
	for (i=0; i<lifts.count; i++)
	{
		lifts.head[i].lift_No = start[offset];
		offset++;
		lifts.head[i].lift_type = start[offset];
		offset++;
		memcpy(&lifts.head[i].card_reader_addr, start+offset,sizeof(DWORD));
		offset+=4;
		memcpy(&lifts.head[i].reserved_1, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&lifts.head[i].reserved_2, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&lifts.head[i].reserved_3, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&lifts.head[i].reserved_4, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
#ifdef DO_DBG
	printf("lift[%d].lift_No=%d \t lifts.head[%d].lift_type=%d \t lift[%d].card_addr=%ld \t lifts[%d].reserved_1=%ld\n", 
	i,lifts.head[i].lift_No,
	i,lifts.head[i].lift_type,
	i, lifts.head[i].card_reader_addr,
	i,lifts.head[i].reserved_1);
#endif		
	}
	//住户信息初始化
	rst_flag = my_memcpy((BYTE *)&(residents.count), start, offset, sizeof(residents.count), file_stat.st_size);
	if (rst_flag == FALSE)
	{
		goto error3;
	}
	offset += sizeof(residents.count);	
	offset += 4; //4 data length
	if(residents.head != NULL)
	{
		free(residents.head);
		residents.head = NULL;
	}
	residents.head = (RESIDENT_INFO *)malloc(residents.count * sizeof(RESIDENT_INFO));
	memset(residents.head,0,residents.count * sizeof(RESIDENT_INFO));
	if (NULL == residents.head)
	{
		printf("%s:%s:%dn error\n", __FILE__, __FUNCTION__, __LINE__);
		goto error3;
	}	
	for (i=0; i<residents.count; i++)
	{
		rst_flag = my_memcpy((BYTE *)&(residents.head[i].resident_code), start, offset, IDENTITY_LEN, file_stat.st_size);
		if (rst_flag == FALSE)
		{
			goto error3;
		}	
		offset += IDENTITY_LEN;
		residents.head[i].floor = UCHAR2SHORT(start[offset]);
		offset++;
		residents.head[i].lift_No_1 = start[offset];
		offset++;
		residents.head[i].lift_No_1_door = start[offset];
		offset++;
		residents.head[i].lift_No_2 = start[offset];
		offset++;
		residents.head[i].lift_No_2_door = start[offset];
		offset++;
		residents.head[i].lift_No_3 = start[offset];
		offset++;
		residents.head[i].lift_No_3_door = start[offset];
		offset++;
		residents.head[i].lift_No_4 = start[offset];
		offset++;		
		residents.head[i].lift_No_4_door = start[offset];
		offset++;
		memcpy(&residents.head[i].reserved_1, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&residents.head[i].reserved_2, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&residents.head[i].reserved_3, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&residents.head[i].reserved_4, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
#ifdef DO_DBG
	printf("the i=%d \t floor=%d \t lift_No_1=%d \t lift_No_2=%d \t lift_No_3=%d \t lift_No_4=%d \t code=%s\n", i, residents.head[i].floor, residents.head[i].lift_No_1, residents.head[i].lift_No_2, residents.head[i].lift_No_3, residents.head[i].lift_No_4, residents.head[i].resident_code);
#endif			
	}
	//门口机或门禁控制器设置初始化
	rst_flag = my_memcpy((BYTE *)&(DAC.count), start, offset, sizeof(DAC.count), file_stat.st_size);	
	if (rst_flag == FALSE)
	{
		goto error3;
	}
	printf("DAC.count = %ld\n",DAC.count);
	offset += sizeof(DAC.count);
	offset += 4; //data length
	
	if(DAC.head != NULL)
	{
		free(DAC.head);
		DAC.head = NULL;
	}
	DAC.head = (DAC_PARA *)malloc(DAC.count * sizeof(DAC_PARA));
	memset(DAC.head,0,DAC.count * sizeof(DAC_PARA));
	if (NULL == DAC.head)
	{
		printf("%s:%s:%dn error\n", __FILE__, __FUNCTION__, __LINE__);
		goto error3;
	}	
	for (i=0; i<DAC.count; i++)
	{
		rst_flag = my_memcpy((BYTE *)&(DAC.head[i].door_code), start, offset, IDENTITY_LEN, file_stat.st_size);
		if (rst_flag == FALSE)
		{
			goto error3;
		}	
		offset += IDENTITY_LEN;
		DAC.head[i].floor = UCHAR2SHORT(start[offset]);
		offset++;
		DAC.head[i].call_lift_type = start[offset];
		offset++;
		DAC.head[i].lift_No = start[offset];
		offset++;
		DAC.head[i].door_type = start[offset];
		offset++;
		memcpy(&DAC.head[i].reserved_1, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&DAC.head[i].reserved_2, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&DAC.head[i].reserved_3, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&DAC.head[i].reserved_4, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
#ifdef DO_DBG
		printf("the i=%d \t door_code=%s \t floor=%d \t call_lift_type=%d \t lift_No=%d \t door_type=%d \n", i, 
		DAC.head[i].door_code,DAC.head[i].floor,DAC.head[i].call_lift_type,DAC.head[i].lift_No,DAC.head[i].door_type);
#endif			
	}
	//DO 模块设置初始化
	rst_flag = my_memcpy((BYTE *)&(DO_modules.count), start, offset, sizeof(DO_modules.count), file_stat.st_size);	
	if (rst_flag == FALSE)
	{
		goto error3;
	}
	offset += sizeof(DO_modules.count);
	offset += 4; //data length
	
	if(DO_modules.head != NULL)
	{
		free(DO_modules.head);
		DO_modules.head = NULL;
	}
	DO_modules.head = (DO_MODULE *)malloc(DO_modules.count * sizeof(DO_MODULE));
	memset(DO_modules.head,0,DO_modules.count * sizeof(DO_MODULE));
	if (NULL == DO_modules.head)
	{
		printf("%s:%s:%dn error\n", __FILE__, __FUNCTION__, __LINE__);
		goto error3;
	}	
	for (i=0; i<DO_modules.count; i++)
	{
		DO_modules.head[i].comm_way = start[offset];
		offset++;
		rst_flag = my_memcpy((BYTE *)&(DO_modules.head[i].ip), start, offset, sizeof(DWORD), file_stat.st_size);	
		if (rst_flag == FALSE)
		{
			goto error3;
		}	
		offset += sizeof(DWORD);
		rst_flag = my_memcpy((BYTE *)&(DO_modules.head[i].TCP_port), start, offset, sizeof(WORD), file_stat.st_size);	
		if (rst_flag == FALSE)
		{
			goto error3;
		}	
		offset += sizeof(WORD);	
		rst_flag = my_memcpy((BYTE *)&(DO_modules.head[i].UDP_port), start, offset, sizeof(WORD), file_stat.st_size);	
		if (rst_flag == FALSE)
		{
			goto error3;
		}	
		offset += sizeof(WORD);
		DO_modules.head[i].modbus_addr = start[offset];
		offset++;
		rst_flag = my_memcpy((BYTE *)&(DO_modules.head[i].baud_rate), start, offset, sizeof(DWORD), file_stat.st_size);	
		if (rst_flag == FALSE)
		{
			goto error3;
		}	
		offset += sizeof(DWORD);
		DO_modules.head[i].data_bits = start[offset];
		offset++;
		DO_modules.head[i].parity_flag = start[offset];
		offset++;	
		DO_modules.head[i].stop_bits= start[offset];
		offset++;
		memcpy(&DO_modules.head[i].reserved_1, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&DO_modules.head[i].reserved_2, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&DO_modules.head[i].reserved_3, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
		memcpy(&DO_modules.head[i].reserved_4, start+offset,sizeof(DWORD));//4	保留字段
		offset+=4;
#ifdef DO_DBG
		printf("index=%d \t comm_way=%d \t ip=%lx \t TCP_port=%d \t UDP_port=%d \t modbus_addr=%x \n", i, 
		DO_modules.head[i].comm_way, DO_modules.head[i].ip, DO_modules.head[i].TCP_port, 
		DO_modules.head[i].UDP_port,DO_modules.head[i].modbus_addr);
#endif 
		rst_flag = my_memcpy((BYTE *)&(port_count), start, offset, sizeof(DWORD), file_stat.st_size);	
		if (rst_flag == FALSE)
		{
			goto error3;
		}	
		offset += sizeof(DWORD);
		offset += sizeof(DWORD); // data length
		for (j=0; j<port_count; j++)
		{
			port_num = start[offset];
			offset++;
			switch (port_num - 1)
			{
				case DO_1:
					{
						DO_modules.head[i].DO1_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO1_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO1_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO1_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO1_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO1_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO1_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO1_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO1 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO1_func.floor,DO_modules.head[i].DO1_func.lift_No,  
						DO_modules.head[i].DO1_func.func,DO_modules.head[i].DO1_func.door_type);
#endif
					}
					break;
				case DO_2:
					{
						DO_modules.head[i].DO2_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO2_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO2_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO2_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO2_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO2_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO2_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO2_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO2 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO2_func.floor,DO_modules.head[i].DO2_func.lift_No,  
						DO_modules.head[i].DO2_func.func,DO_modules.head[i].DO2_func.door_type);
#endif
					}
					break;
				case DO_3:
					{
						DO_modules.head[i].DO3_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO3_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO3_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO3_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO3_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO3_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO3_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO3_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO3 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO3_func.floor,DO_modules.head[i].DO3_func.lift_No,  
						DO_modules.head[i].DO3_func.func,DO_modules.head[i].DO3_func.door_type);
#endif
					}
					break;
				case DO_4:
					{
						DO_modules.head[i].DO4_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO4_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO4_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO4_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO4_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO4_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO4_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO4_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO4 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO4_func.floor,DO_modules.head[i].DO4_func.lift_No,  
						DO_modules.head[i].DO4_func.func,DO_modules.head[i].DO4_func.door_type);
#endif
					}
					break;
				case DO_5:
					{
						DO_modules.head[i].DO5_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO5_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO5_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO5_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO5_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO5_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO5_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO5_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO5 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO5_func.floor,DO_modules.head[i].DO5_func.lift_No,  
						DO_modules.head[i].DO5_func.func,DO_modules.head[i].DO5_func.door_type);
#endif
					}
					break;
				case DO_6:
					{
						DO_modules.head[i].DO6_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO6_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO6_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO6_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO6_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO6_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO6_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO6_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO6 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO6_func.floor,DO_modules.head[i].DO6_func.lift_No,  
						DO_modules.head[i].DO6_func.func,DO_modules.head[i].DO6_func.door_type);
#endif
					}
					break;
				case DO_7:
					{
						DO_modules.head[i].DO7_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO7_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO7_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO7_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO7_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO7_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO7_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO7_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO7 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO7_func.floor,DO_modules.head[i].DO7_func.lift_No,  
						DO_modules.head[i].DO7_func.func,DO_modules.head[i].DO7_func.door_type);
#endif
					}
					break;
				case DO_8:
					{
						DO_modules.head[i].DO8_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO8_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO8_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO8_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO8_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO8_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO8_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO8_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO8 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO8_func.floor,DO_modules.head[i].DO8_func.lift_No,  
						DO_modules.head[i].DO8_func.func,DO_modules.head[i].DO8_func.door_type);
#endif
					}
					break;
				case DO_9:
					{
						DO_modules.head[i].DO9_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO9_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO9_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO9_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO9_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO9_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO9_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO9_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO9 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO9_func.floor,DO_modules.head[i].DO9_func.lift_No,  
						DO_modules.head[i].DO9_func.func,DO_modules.head[i].DO9_func.door_type);
#endif
					}
					break;
				case DO_10:
					{
						DO_modules.head[i].DO10_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO10_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO10_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO10_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO10_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO10_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO10_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO10_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO10 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO10_func.floor,DO_modules.head[i].DO10_func.lift_No,  
						DO_modules.head[i].DO10_func.func,DO_modules.head[i].DO10_func.door_type);
#endif
					}
					break;
				case DO_11:
					{

						DO_modules.head[i].DO11_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO11_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO11_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO11_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO11_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO11_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO11_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO11_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO11 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO11_func.floor,DO_modules.head[i].DO11_func.lift_No,  
						DO_modules.head[i].DO11_func.func,DO_modules.head[i].DO11_func.door_type);
#endif
					}
					break;
				case DO_12:
					{
						DO_modules.head[i].DO12_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO12_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO12_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO12_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO12_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO12_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO12_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO12_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO12 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO12_func.floor,DO_modules.head[i].DO12_func.lift_No,  
						DO_modules.head[i].DO12_func.func,DO_modules.head[i].DO12_func.door_type);
#endif
					}
					break;
				case DO_13:
					{
						DO_modules.head[i].DO13_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO13_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO13_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO13_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO13_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO13_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO13_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO13_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO13 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO13_func.floor,DO_modules.head[i].DO13_func.lift_No,  
						DO_modules.head[i].DO13_func.func,DO_modules.head[i].DO13_func.door_type);
#endif
					}
					break;
				case DO_14:
					{
						DO_modules.head[i].DO14_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO14_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO14_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO14_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO14_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO14_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO14_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO14_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO14 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO14_func.floor,DO_modules.head[i].DO14_func.lift_No,  
						DO_modules.head[i].DO14_func.func,DO_modules.head[i].DO14_func.door_type);
#endif
					}
					break;
				case DO_15:
					{
						DO_modules.head[i].DO15_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO15_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO15_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO15_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO15_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO15_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO15_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO15_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO15 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO15_func.floor,DO_modules.head[i].DO15_func.lift_No,  
						DO_modules.head[i].DO15_func.func,DO_modules.head[i].DO15_func.door_type);
#endif
					}
					break;
				case DO_16:
					{
						DO_modules.head[i].DO16_func.floor = UCHAR2SHORT(start[offset]);
						offset++;
						DO_modules.head[i].DO16_func.lift_No = start[offset];
						offset++;
						DO_modules.head[i].DO16_func.func = start[offset];
						offset++;
						DO_modules.head[i].DO16_func.door_type = start[offset];
						offset++;
						memcpy(&DO_modules.head[i].DO16_func.reserved_1, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO16_func.reserved_2, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO16_func.reserved_3, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
						memcpy(&DO_modules.head[i].DO16_func.reserved_4, start+offset,sizeof(DWORD));//4	保留字段
						offset+=4;
#ifdef DO_DBG
						printf("index=%d \t DO16 \t floor=%d \t lift_No=%d \t func=%d \t door_type=%d\n", i, 
						DO_modules.head[i].DO16_func.floor,DO_modules.head[i].DO16_func.lift_No,  
						DO_modules.head[i].DO16_func.func,DO_modules.head[i].DO16_func.door_type);
#endif
					}
					break;
				default:
					{
						offset +=3; // for bug 10963
					}
					break;
			}

		}
	}
	//梯控设置初始化
	memcpy(&common_para, start+offset, sizeof(common_para));
#ifdef DO_DBG
	printf("common_para.pulse_width1=%d \tcommon_para.pulse_width2=%d \tcommon_para.pulse_width3=%d \t call_lift_way_1=%d \t call_lift_way_2=%d \t call_lift_way_3=%d \n", 
	common_para.pulse_width_1,common_para.pulse_width_2,common_para.pulse_width_3, common_para.call_lift_way_1, common_para.call_lift_way_2, common_para.call_lift_way_3);
#endif
	offset += sizeof(common_para);

	munmap(start, file_stat.st_size);
	close(fd);
	modbus_LT_para_init_state = TRUE;
	return TRUE;

error1:
	return FALSE;
error2:
	close(fd);
	return FALSE;
error3:
	munmap(start, file_stat.st_size);
	close(fd);		
	return FALSE;

#if 0
	g_SysConfig.LC_ComMode = LC_BY_DO_MODULE;

	common_para.pulse_width = 4;
	common_para.call_lift_way_1 = CALL_LIFT_WAY_KEY_AND_UNLOCK;
	common_para.call_lift_way_2 = CALL_LIFT_WAY_KEY_AND_UNLOCK;
	common_para.call_lift_way_3 = CALL_LIFT_WAY_KEY_AND_UNLOCK;	

	residents.count = 5;
	residents.head = (RESIDENT_INFO *)malloc(4*sizeof(RESIDENT_INFO));
	residents.head[0].floor = 5;
	residents.head[0].lift_No_1 = 1;
	residents.head[0].lift_No_2 = 1;
	residents.head[0].lift_No_3 = 1;
	residents.head[0].lift_No_4 = 1;
	memcpy(residents.head[0].resident_code, "003", 20); 

	residents.head[1].floor = 5;
	residents.head[1].lift_No_1 = 1;
	residents.head[1].lift_No_2 = 1;
	residents.head[1].lift_No_3 = 1;
	residents.head[1].lift_No_4 = 1;
	memcpy(residents.head[1].resident_code, "103", 20); 	

	residents.head[2].floor = 1;
	residents.head[2].lift_No_1 = 1;
	residents.head[2].lift_No_2 = 1;
	residents.head[2].lift_No_3 = 1;
	residents.head[2].lift_No_4 = 1;
	memcpy(residents.head[2].resident_code, "105", 20); 

	residents.head[3].floor = 1;
	residents.head[3].lift_No_1 = 1;
	residents.head[3].lift_No_2 = 1;
	residents.head[3].lift_No_3 = 1;
	residents.head[3].lift_No_4 = 1;
	memcpy(residents.head[3].resident_code, "002", 20); 	

	residents.head[1].floor = 2;
	residents.head[1].lift_No_1 = 1;
	residents.head[1].lift_No_2 = 1;
	residents.head[1].lift_No_3 = 1;
	residents.head[1].lift_No_4 = 1;
	memcpy(residents.head[1].resident_code, "005", 20); 	


	lifts.count = 1;
	lifts.head = (LIFT_PARA *)malloc(sizeof(LIFT_PARA));
	lifts.head[0].card_reader_addr = 1;
	lifts.head[0].lift_No = 1;

	DO_modules.count = 1;
	DO_modules.head = (DO_MODULE *)malloc(sizeof(DO_MODULE));
	DO_modules.head[0].modbus_addr = 1;
	DO_modules.head[0].comm_way = COMM_WAY_ETH;
	DO_modules.head[0].ip = 0xac10000b; // 172,16.0.11
	DO_modules.head[0].port = 503;
	DO_modules.head[0].DO1_func.floor = 1;
	DO_modules.head[0].DO1_func.lift_No = 1;
	DO_modules.head[0].DO1_func.func = PRESS_KEY_UP;
	DO_modules.head[0].DO2_func.floor = 5;
	DO_modules.head[0].DO2_func.lift_No = 1;
	DO_modules.head[0].DO2_func.func = PRESS_KEY_UNLOCK;
	DO_modules.head[0].DO3_func.floor = 5;
	DO_modules.head[0].DO3_func.lift_No = 1;
	DO_modules.head[0].DO3_func.func = PRESS_KEY_DOWN;
	DO_modules.head[0].DO4_func.floor = 2;
	DO_modules.head[0].DO4_func.lift_No = 1;
	DO_modules.head[0].DO4_func.func = PRESS_KEY_UP;	

	return TRUE;
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    lift_ctrl_by_DO_module_init
**   Owner: Jason Wang
**	DATE:		2012-04-28 
**	
**	DESCRIPTION:	
**		init the module
**	ARGUMENTS:	
**		NULL
**	RETURNED VALUE:	
**		NULL
**	NOTES:
**	
*/
void lift_ctrl_by_DO_module_init()
{
	memset(&lifts, 0, sizeof(LIFTS_HEAD));
	memset(&residents, 0, sizeof(RESIDENTS_HEAD));
	memset(&DO_modules, 0, sizeof(DO_MODULES_HEAD));
	lc_thread_state = FALSE;
	modbus_LT_para_init_state = FALSE;
	
	if( LC_BY_DO_MODULE != g_SysConfig.LC_ComMode)
	{
		return;
	}

	if (!modbus_LT_para_init())
	{
		printf("1*****************************************\n");
		return;
	}
	if (!modnet_init())
	{
		printf("2*****************************************\n");
		return;
	}
	if (!modbus_init())
	{
		printf("3*****************************************\n");
		return;
	}	
	pthread_mutex_init(&(thread_sync.mutex), NULL);
	pthread_cond_init(&(thread_sync.cond), NULL);
	thread_sync.flag = FALSE;
	if ((pthread_create(&lift_ctrl_pid, NULL, lift_ctrl_by_DO_module_fun, NULL)) != 0)	
	{
		printf("%s: %d create thread fail\n", __FUNCTION__, __LINE__);
		return;
	}

	lc_thread_state = TRUE;

	DoTimerBufInit();
	
	DpcAddMd(MXMDID_LT_BY_DO_MODULE, NULL);	
}


/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    ctrl_lift
**   Owner: Jason Wang
**	DATE:		2012-04-24 
**	
**	DESCRIPTION:	
**		control lift by DO module interface, other module can use this function to control lift
**	ARGUMENTS:	
**		src: srcMd
**		type: 呼梯类型(室内机呼梯，访客开锁，刷卡开锁，户户对讲，轿厢内刷卡)
**		addr: 轿厢内刷卡时刷卡头地址，其它类型不用到
**		codes: 呼叫方和被呼叫方的住户号,可能只有一个住户号
**	RETURNED VALUE:	
**		NULL
**	NOTES:
**	
*/
void ctrl_lift(const DWORD src, DWORD type, DWORD addr, const unsigned char *codes)
{
	MXMSG	msgSend;

	memset(&msgSend, 0, sizeof(MXMSG));
	msgSend.dwSrcMd = src; // 发送该消息的模块
	msgSend.dwDestMd = MXMDID_LT_BY_DO_MODULE;
	msgSend.dwMsg = type; // 呼梯类型
	msgSend.dwParam = addr;
	msgSend.pParam = (unsigned char *)malloc(1+2*IDENTITY_LEN);
	if (NULL == msgSend.pParam) 
	{
		printf("NULL != msgSend.pParam\n");
		return;
	}
	memcpy(msgSend.pParam, codes, 1+2*IDENTITY_LEN);

	MxPutMsg(&msgSend);
#if 0	
	pthread_mutex_lock(&thread_sync.mutex);
	thread_sync.flag = TRUE;
	pthread_mutex_unlock(&thread_sync.mutex);
	pthread_cond_signal(&thread_sync.cond);
#endif	

}
/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    comm_with_DO_by_eth
**   Owner: Jason Wang
**	DATE:		2012-04-23 
**	
**	DESCRIPTION:	
**			最多发三次，
**	ARGUMENTS:	
**		xx
**	RETURNED VALUE:	
**		TRUE: 设置状态成功; FALSE: 设置状态失败
**	NOTES:
**	
*/
static BOOL comm_with_DO_by_eth(unsigned int rip, unsigned short rport, BYTE cmd, unsigned short start_addr, unsigned short count, unsigned short value) 
{
	unsigned int time_out = 1;
	BOOL ack_rst = FALSE;
	unsigned int ip = 0;
	unsigned short port = 0;
	unsigned int rst = 0;
	int i = 0;
	BYTE ack_buf[BUF_LEN] = {0};
	unsigned int stat_val = 0;
#ifdef DO_DBG
			printf("%s: \n", __FUNCTION__);
#endif

	for (i=0; i<RETRY_TIMES; i++)
	{
		send_cmd_to_DO_by_eth(rip, rport, cmd, start_addr, count, value);
		rst = wait_slave_ack_eth(ack_buf, sizeof(ack_buf), &ip, &port, time_out);
		if (ACK_TIME_OUT != rst) // 设置DO 成功
		{
			ack_rst = analysis_ack_eth(ack_buf, rst, ip, port, &stat_val);
			if (ack_rst) // 正确响应
			{
				return TRUE;
			}
		}
	}
	if (RETRY_TIMES == i) // 重试多次设置DO 不成功
	{
		return FALSE;
	}
	return FALSE;
}


static BOOL comm_with_DO_by_serial(BYTE stop_addr, BYTE cmd, unsigned short start_addr, unsigned short count, unsigned short value) 
{
	unsigned int time_out = 2;
	BOOL ack_rst = FALSE;
	unsigned int rst = 0;
	int i = 0;
	BYTE ack_buf[BUF_LEN] = {0};
	unsigned int stat_val = 0;

	for (i=0; i<RETRY_TIMES; i++)
	{
		send_cmd_to_DO_serial(stop_addr, cmd, start_addr, count, value);
		rst = wait_slave_ack_serial(ack_buf, sizeof(ack_buf), time_out);
		if (ACK_TIME_OUT != rst) // 设置DO 成功
		{
			ack_rst = analysis_ack_serial(ack_buf, rst, &stat_val);
			if (ack_rst) // 正确响应
			{
				return TRUE;
			}
		}
	}
	if (RETRY_TIMES == i) // 重试多次设置DO 不成功
	{
		return FALSE;
	}
	return FALSE;
}

/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    send_cmd_to_DO
**   Owner: Jason Wang
**	DATE:		2012-04-23 
**	
**	DESCRIPTION:	
**		设置DO 状态，发脉冲
**	ARGUMENTS:	
**		xx
**	RETURNED VALUE:	
**		xx
**	NOTES:
**	
*/
static BOOL send_cmd_to_DO(const DO_MODULE *DO_Module, const BYTE port_id, BYTE cmd, unsigned short count, unsigned short value, DWORD entryType)
{
	BOOL comm_rst = FALSE;
	TIME_DO_MODULE_t tData;

	tData.cmd = cmd;
	tData.count = count;
	tData.port_id = port_id;
	tData.value = value;
	memcpy(&(tData.DO_MODULE),DO_Module,sizeof(DO_MODULE));
	tData.entryType = entryType;
#ifdef DO_DBG
	printf("%s:port=%d \t cmd=0x%x \n", __FUNCTION__, port_id, cmd);
#endif

	if (COMM_WAY_ETH == DO_Module->comm_way) // 以太网控制DO
	{
		if (0x05 == cmd)
		{
			comm_rst = comm_with_DO_by_eth(DO_Module->ip,DO_Module->UDP_port,cmd,(unsigned short)port_id, 1, COIL_ON);
			if (comm_rst)
			{
				TimeSetDo(tData);
			}
		}
		else if (0x0f == cmd)
		{
			comm_rst = comm_with_DO_by_eth(DO_Module->ip,DO_Module->UDP_port,cmd,(unsigned short)port_id, count, value);
			if (comm_rst)
			{
				TimeSetDo(tData);
			}

		}
		else
		{
			return FALSE; // do nothing
		}
	}
	else // RS485 控制DO
	{
		set_serial_para(DO_Module->baud_rate, DO_Module->data_bits, DO_Module->parity_flag, DO_Module->stop_bits);	
		if (0x05 == cmd)
		{
			comm_rst = comm_with_DO_by_serial(DO_Module->modbus_addr, cmd,(unsigned short)port_id, 1, COIL_ON);
			if (comm_rst)
			{
				TimeSetDo(tData);
			}
		}
		else if (0x0f == cmd)
		{
			comm_rst = comm_with_DO_by_serial(DO_Module->modbus_addr, cmd, (unsigned short)port_id, count,value);
			if (comm_rst)
			{	
				TimeSetDo(tData);
			}

		}
		else
		{
			return FALSE; // do nothing
		}		
	}
	return comm_rst;
}

/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    call_lift
**   Owner: Jason Wang
**	DATE:		2012-04-23 
**	
**	DESCRIPTION:	
**		call lift through DO module
**	ARGUMENTS:	
**		msg: message from other app
**		lift_No: lift number which want to control
**		call_lift_way: use which way to control the lift
**		opt: press up or down or unlock
**	RETURNED VALUE:	
**		TRUE:sucess; FALSE: 
**	NOTES:
**	
*/
static BOOL call_lift(const BYTE lift_No, const SHORT floor, const DWORD call_lift_way, const BYTE opt)
{
	DO_MODULE *DO_Module = NULL;
	PORT_ID port_id = DO_INVALID;
	BOOL rst = FALSE;
	DWORD callLiftWay = 0, entry_type = call_lift_way;
	
#ifdef DO_DBG
	printf("%s:lift_No=%d \t floor %d \t call_way=%d \t opt=%d\n", __FUNCTION__, lift_No, floor, call_lift_way, opt);
#endif

	if((call_lift_way & CALL_LIFT_WAY_KEY) == CALL_LIFT_WAY_KEY)
		callLiftWay = CALL_LIFT_WAY_KEY;
	else if((call_lift_way & CALL_LIFT_WAY_UNLOCK) == CALL_LIFT_WAY_UNLOCK)
		callLiftWay = CALL_LIFT_WAY_KEY;
	else
		callLiftWay = call_lift_way;
	
	if (CALL_LIFT_WAY_KEY == callLiftWay || CALL_LIFT_WAY_UNLOCK == callLiftWay)
	{
		FUNC_PARA spara={0};
		spara.lift_No = lift_No;
		spara.floor = floor;
		spara.func = opt;
		if(entry_type & ENTRY_TYPE_BACK)
		{
			spara.door_type = VALUE_BACK_DOOR;
			DO_Module = get_DO_module_and_port(spara, &port_id);
			if (NULL != DO_Module && DO_INVALID != port_id)
			{
				rst = send_cmd_to_DO(DO_Module, port_id, 0x05, 0x1, 0, entry_type); 
				usleep(DELAY_TIME);
			}
		}
		
		if(entry_type & ENTRY_TYPE_FRONT)
		{
			spara.door_type = VALUE_FRONT_DOOR;
			DO_Module = get_DO_module_and_port(spara, &port_id);
			if (NULL != DO_Module && DO_INVALID != port_id)
			{ 
				rst = send_cmd_to_DO(DO_Module, port_id, 0x05, 0x1, 0, entry_type);
				usleep(DELAY_TIME);
			}
		}
	}
	else 
	{
		printf("%s:%s:%d\n error", __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}	
	return rst;
}

#define MAX_LIFT 15

void residentsShow(RESIDENT_INFO* Residents[])
{
	int i=0;

	for(i=0;i<MAX_LIFT;i++)
	{
		if(Residents[i]== NULL)
		{
			printf("Valid Residents is %d\n",i);
			break;
		}
		printf("Residents[%d]=%s\n",i,Residents[i]->resident_code);
		printf("Residents[%d]floor=%d\n",i,Residents[i]->floor);
		printf("Residents[%d]lift_No_4=%d\n",i,Residents[i]->lift_No_4);
	}
}

void GetResidents(RESIDENT_INFO* Residents[], const char *code)
{
	
	int i = 0,j=0;

	for (i=0; i<residents.count; i++)
	{
		if (!strcmp((char *)code, (char *)(residents.head[i].resident_code))) // 找到该住户的信息
		{
			Residents[j++] = &(residents.head[i]);
		}
	}
	residentsShow(Residents);
}

void DACSShow(DAC_PARA * DACS[])
{
	int i=0;

	for(i=0;i<MAX_LIFT;i++)
	{
		if(DACS[i]== NULL)
		{
			printf("Valid DACS is %d\n",i);
			break;
		}
		printf("DACS[%d]=%s\n",i,DACS[i]->door_code);
		printf("DACS[%d]floor=%d\n",i,DACS[i]->floor);
		printf("DACS[%d]call_lift_type=%d\n",i,DACS[i]->call_lift_type);
		printf("DACS[%d]lift_No=%d\n",i,DACS[i]->lift_No);
		printf("DACS[%d]door_type=%d\n",i,DACS[i]->door_type);
	}
}
void GetDACS(DAC_PARA * DACS[], const char *code)
{
	
	int i = 0,j=0;

	for (i=0; i<DAC.count; i++)
	{
		if (!strcmp((char *)code, (char *)(DAC.head[i].door_code))) // 找到该门控制的信息
		{
			DACS[j++] = &(DAC.head[i]);
        }
	}
	DACSShow(DACS);
}


#define GETPRESSKEY(a,b) (((a-b)>0)?PRESS_KEY_DOWN : PRESS_KEY_UP)
static void* lift_ctrl_by_DO_module_fun(void* arg)
{
	MXMSG msg;
	char call_code[IDENTITY_LEN] = {0};
	char called_code[IDENTITY_LEN] = {0};
	
	while (TRUE)
	{
#if 0	
		pthread_mutex_lock(&thread_sync.mutex);
		while (FALSE == thread_sync.flag)
		{
			pthread_cond_wait(&thread_sync.cond, &thread_sync.mutex);
		}
#endif		
		usleep(1000);
		DoTimerProc();
		memset(&msg, 0, sizeof(msg));
		msg.dwDestMd = MXMDID_LT_BY_DO_MODULE;
		msg.pParam = NULL; // 住户号
		if (MxGetMsg(&msg))
		{
			switch (msg.dwMsg)
			{
			case VISTOR_UNLOCK: // 访客开锁
				{
					memcpy(call_code, msg.pParam, IDENTITY_LEN);
					memcpy(called_code, msg.pParam+21, IDENTITY_LEN);
					_visitor_call_lift(call_code,called_code);
					break;
				}
				break;
			case HV_CALL_LIFT: // 室内机呼梯
				{	
					char up_or_down = 0;
					
					memcpy(call_code, msg.pParam, IDENTITY_LEN);
					up_or_down = msg.pParam[21];
					printf("up_or_down=%x\n", up_or_down);
					_HV_call_lift(call_code, up_or_down);
				}
				break;
			case HV_CALLING_HV_CALL_LIFT: // 户户对讲
				{
					memcpy(call_code, msg.pParam, IDENTITY_LEN);
					memcpy(called_code, msg.pParam+21, IDENTITY_LEN);
					_HV_call_HV_call_lift(call_code,called_code);
				}
				break;
			case SWIPER_CARD_UNLOCK: // 刷卡开锁
				{
					memcpy(call_code, msg.pParam, IDENTITY_LEN);
					memcpy(called_code, msg.pParam+21, IDENTITY_LEN); 
					_swiper_card_call_lift(call_code,called_code);
				}
				break;
			case SWIPER_CARD_IN_LIFT: // 轿厢内刷卡
				{
					BYTE addr = 0;
					DWORD rip = 0;

					memcpy(call_code, msg.pParam, IDENTITY_LEN);
					memcpy(&rip, msg.pParam+21, 4);					
					addr = (BYTE)(msg.dwParam); // 轿厢内刷卡头站号
					_liftcar_swiper_card(call_code, addr, rip);
				}
				break;
			default: 
				{

				}
				break;			
			}
		}
		free_msg(&msg);	
#if 0		
		thread_sync.flag = FALSE;
		pthread_mutex_unlock(&thread_sync.mutex);
#endif		
	}
	return NULL;	
}

/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    send_ack_to_card_reader
**   Owner: Jason Wang
**	DATE:		2012-06-11 
**	
**	DESCRIPTION:	
**		xx
**	ARGUMENTS:	
**		val: 0x0: unlock success; 0x1 unlock failure 
**	RETURNED VALUE:	
**		xx
**	NOTES:
**	
*/
static void send_ack_to_card_reader(BYTE val, DWORD rip)
{
	MXMSG  msgSend;
	
	memset(&msgSend, 0, sizeof(msgSend));
	msgSend.dwSrcMd		= MXMDID_LT_BY_DO_MODULE;
	msgSend.dwDestMd	= MXMDID_LCA;

	//msgSend.dwMsg		= pCmd->usCMD + 0x8000;
	msgSend.dwMsg		= FC_ACK_LF_UNLOCK;
	msgSend.dwParam		= rip;
	msgSend.pParam		= malloc(1);
	*msgSend.pParam		= val;
	MxPutMsg(&msgSend);
//#ifdef LCA_DEBUG
	printf("Send the msg=%lx, error ACK=%d. to ip=%lx\n", msgSend.dwMsg, *msgSend.pParam, msgSend.dwParam);
//#endif
}


static void unlock_all_floors(const BYTE lift_No, const BYTE opt)
{
	int i = 0;
	unsigned short value = 0;
	for (i=0; i<DO_modules.count; i++)
	{
		value = 0;
		if (DO_modules.head[i].DO1_func.lift_No == lift_No 
			&& DO_modules.head[i].DO1_func.func == opt)
		{
			value |= 0x1 << DO_1;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_1, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO2_func.lift_No == lift_No 
			&& DO_modules.head[i].DO2_func.func == opt)
		{
			value |= 0x1 << DO_2;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_2, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO3_func.lift_No == lift_No 
			&& DO_modules.head[i].DO3_func.func == opt)
		{
			value |= 0x1 << DO_3;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_3, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO4_func.lift_No == lift_No 
			&& DO_modules.head[i].DO4_func.func == opt)
		{
			value |= 0x1 << DO_4;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_4, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO5_func.lift_No == lift_No 
			&& DO_modules.head[i].DO5_func.func == opt)
		{
			value |= 0x1 << DO_5;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_5, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO6_func.lift_No == lift_No 
			&& DO_modules.head[i].DO6_func.func == opt)
		{
			value |= 0x1 << DO_6;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_6, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO7_func.lift_No == lift_No 
			&& DO_modules.head[i].DO7_func.func == opt)
		{
			value |= 0x1 << DO_7;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_7, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO8_func.lift_No == lift_No 
			&& DO_modules.head[i].DO8_func.func == opt)
		{
			value |= 0x1 << DO_8;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_8, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO9_func.lift_No == lift_No 
			&& DO_modules.head[i].DO9_func.func == opt)
		{
			value |= 0x1 << DO_9;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_9, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO10_func.lift_No == lift_No 
			&& DO_modules.head[i].DO10_func.func == opt)
		{
			value |= 0x1 << DO_10;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_10, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO11_func.lift_No == lift_No 
			&& DO_modules.head[i].DO11_func.func == opt)
		{
			value |= 0x1 << DO_11;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_11, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO12_func.lift_No == lift_No 
			&& DO_modules.head[i].DO12_func.func == opt)
		{
			value |= 0x1 << DO_12;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_12, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO13_func.lift_No == lift_No 
			&& DO_modules.head[i].DO13_func.func == opt)
		{
			value |= 0x1 << DO_13;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_13, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO14_func.lift_No == lift_No 
			&& DO_modules.head[i].DO14_func.func == opt)
		{
			value |= 0x1 << DO_14;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_14, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO15_func.lift_No == lift_No 
			&& DO_modules.head[i].DO15_func.func == opt)
		{
			value |= 0x1 << DO_15;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_15, 0x05, 0x1, 0);	
		}
		if (DO_modules.head[i].DO16_func.lift_No == lift_No 
			&& DO_modules.head[i].DO16_func.func == opt)
		{
			value |= 0x1 << DO_16;
			// send_cmd_to_DO(&(DO_modules.head[i]), DO_16, 0x05, 0x1, 0);	
		}
		if (0 != value)
		{
			send_cmd_to_DO(&(DO_modules.head[i]), DO_1, 0x0f, DO_PORT_NUM, value, ENTRY_TYPE_LIFTCAR | CALL_LIFT_WAY_UNLOCK);
		}
	}
}
/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    get_DO_module_and_port
**   Owner: Jason Wang
**	DATE:		2012-04-23 
**	
**	DESCRIPTION:	
**		get the DO module and the port id by lift No and floor and operation
**	ARGUMENTS:	
**		lift_No: which lift want to control
**		floor: the dest floor
**		opt: want to up or down or unlock the floor
**		port_id: return the port id which want to control
**	RETURNED VALUE:	
**		NULL: no DO module controlling the lift refered by the lift_No
**		Not NULL: the DO module structure which want to control 
**	NOTES:
**	
*/
static DO_MODULE *get_DO_module_and_port(FUNC_PARA Spara,PORT_ID *port_id)
{
	int i = 0;
	FUNC_PARA tmp_func = {0};
	int DOSIZE = sizeof(FUNC_PARA);
	memcpy(&tmp_func, &Spara, sizeof(FUNC_PARA));

	for (i=0; i<DO_modules.count; i++)
	{
		if (!memcmp((BYTE *)&(DO_modules.head[i].DO1_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_1;
			return &(DO_modules.head[i]);
		}
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO2_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_2;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO3_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_3;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO4_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_4;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO5_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_5;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO6_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_6;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO7_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_7;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO8_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_8;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO9_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_9;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO10_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_10;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO11_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_11;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO12_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_12;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO13_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_13;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO14_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_14;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO15_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_15;
			return &(DO_modules.head[i]);
		}	
		else if (!memcmp((BYTE *)&(DO_modules.head[i].DO16_func), (BYTE *)&tmp_func, DOSIZE))
		{
			*port_id = DO_16;
			return &(DO_modules.head[i]);
		}	
		else
		{
			; // do nothing
		}
	}
	return NULL; // 该住户号不存在	
}



/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    get_lift_No
**   Owner: Jason Wang
**	DATE:		2012-04-20 
**	
**	DESCRIPTION:	
**		get the lift No by card reader addr when swipe card in the lift
**	ARGUMENTS:	
**		card_reader_addr: card reader address
**	RETURNED VALUE:	
**		NULL: not found the info, otherwise found
**	NOTES:
**	
*/
static LIFT_PARA *get_lift_No(BYTE card_reader_addr)
{
	int i = 0;
	DWORD addrIn = 0;

	addrIn = 0x1<<(card_reader_addr-1);

	for (i=0; i<lifts.count; i++)
	{
		if (addrIn & lifts.head[i].card_reader_addr) // 找到该住户的信息
		{
			return &(lifts.head[i]);
		}
	}
	return NULL; // 该电梯没有配置	
}


/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    get_resident_node
**   Owner: Jason Wang
**	DATE:		2012-04-20 
**	
**	DESCRIPTION:	
**		find resident infomation by the code
**	ARGUMENTS:	
**		code: resident code from swipe card or HV msg
**	RETURNED VALUE:	
**		NULL: not found the info, otherwise found
**	NOTES:
**	
*/
static RESIDENT_INFO *get_resident_node(const char *code)
{
	int i = 0;

	for (i=0; i<residents.count; i++)
	{
		if (!strcmp((char *)code, (char *)(residents.head[i].resident_code))) // 找到该住户的信息
		{
			return &(residents.head[i]);
		}
	}
	return NULL; // 该住户号不存在
}

static RESIDENT_INFO *get_resident_node_type(const char *code, const DWORD type, const BYTE value)
{
	int i = 0;
	BOOL rFlag = FALSE;
	
	for (i=0; i<residents.count; i++)
	{
		if (!strcmp((char *)code, (char *)(residents.head[i].resident_code))) // 找到该住户的信息
		{
			switch(type)
			{
				case VISTOR_UNLOCK:
					if(residents.head[i].lift_No_1 == value)
						rFlag = TRUE;
					break;
				case HV_CALL_LIFT:
					if(residents.head[i].lift_No_2 == value)
						rFlag = TRUE;
					break;
				case HV_CALLING_HV_CALL_LIFT:
					if(residents.head[i].lift_No_3 == value)
						rFlag = TRUE;
					break;
				case SWIPER_CARD_UNLOCK:
					if(residents.head[i].lift_No_4 == value)
						rFlag = TRUE;
					break;
				default:
					break;
			}
			if(rFlag)
				return &(residents.head[i]);
		}
	}
	return NULL; // 该住户号不存在
}

static void free_msg(MXMSG* pMsg)
{
	if (pMsg->pParam != NULL)
	{
		MXFree(pMsg->pParam);
		pMsg->pParam = NULL;
	}
}

static BOOL DoTimerBufInit()
{
	memset(DO_TIMER_BUF, 0, sizeof(TIME_BUF_DO_MODULE_t)*DO_MAX_TIMER_COUNT);
	return TRUE;	
}

#ifdef DO_DBG
static void DoTimerBufShow()
{
	int i;
	for(i = 0; i< DO_MAX_TIMER_COUNT; i++)
	{
		printf("[%d]TimerOnFlag=%d\t",i,DO_TIMER_BUF[i].TimerOnFlag);
		printf("cmd=%d\t",DO_TIMER_BUF[i].tData.cmd);
		printf("count=%d\t",DO_TIMER_BUF[i].tData.count);
		printf("port_id=%d\t",DO_TIMER_BUF[i].tData.port_id);
		printf("tickStart=%ld\t",DO_TIMER_BUF[i].tData.tickStart);
		printf("value=%u\t",DO_TIMER_BUF[i].tData.value);
		printf("ip=0x%08lx\t",DO_TIMER_BUF[i].tData.DO_MODULE.ip);
		printf("port=%d\n",DO_TIMER_BUF[i].tData.DO_MODULE.UDP_port);
	}
}
#endif
static void DoTimerProc(void)
{
	int i;
	DWORD	ticket = GetTickCount();
	for(i = 0; i< DO_MAX_TIMER_COUNT; i++)
	{
		if((DO_TIMER_BUF[i].TimerOnFlag == TRUE) && (DO_TIMER_BUF[i].tData.tickStart <= ticket))
		{
			printf("[%d]now:%ld\t 2:%ld\n",i,ticket,DO_TIMER_BUF[i].tData.tickStart);
			TimerSendCmd2DO(DO_TIMER_BUF[i].tData);
			usleep(DELAY_TIME);
			DO_TIMER_BUF[i].TimerOnFlag = FALSE;
			memset(&DO_TIMER_BUF[i].tData,0,sizeof(DO_TIMER_BUF[i].tData));
		}
	}
}
static BOOL TimeSetDo(TIME_DO_MODULE_t tData)
{
	BOOL bRet = FALSE;
	int i;
	WORD pulseWidth = 0;
	DWORD entryType = 0;

	if(tData.entryType & ENTRY_TYPE_DOOR)
		entryType |= ENTRY_TYPE_DOOR;
	if(tData.entryType & ENTRY_TYPE_DEF)
		entryType |= ENTRY_TYPE_DEF;
	if(tData.entryType & ENTRY_TYPE_LIFTCAR)
		entryType |= ENTRY_TYPE_LIFTCAR;
	if(tData.entryType & CALL_LIFT_WAY_UNLOCK)
		entryType |= CALL_LIFT_WAY_UNLOCK;
	if(tData.entryType & CALL_LIFT_WAY_KEY)
		entryType |= CALL_LIFT_WAY_KEY;
		
	switch(entryType)
	{
		case (ENTRY_TYPE_DOOR | CALL_LIFT_WAY_UNLOCK):
		case (ENTRY_TYPE_DEF | CALL_LIFT_WAY_UNLOCK):
			pulseWidth = common_para.pulse_width_1;
			break;
		case (ENTRY_TYPE_LIFTCAR | CALL_LIFT_WAY_UNLOCK):
		case CALL_LIFT_WAY_UNLOCK:
			pulseWidth = common_para.pulse_width_2;
			break;
		case (ENTRY_TYPE_DOOR | CALL_LIFT_WAY_KEY):
		case (ENTRY_TYPE_LIFTCAR | CALL_LIFT_WAY_KEY):
		case (ENTRY_TYPE_DEF | CALL_LIFT_WAY_KEY):
		case CALL_LIFT_WAY_KEY:
			pulseWidth = common_para.pulse_width_3;
			break;
		default:
			pulseWidth = common_para.pulse_width_2;
			break;
	}
	tData.tickStart = GetTickCount() + pulseWidth*1000;

	for(i = 0; i< DO_MAX_TIMER_COUNT; i++)
	{
		if(	DO_TIMER_BUF[i].TimerOnFlag == FALSE 
		   ||
	 	    (  tData.port_id == DO_TIMER_BUF[i].tData.port_id
	 		&& tData.DO_MODULE.ip == DO_TIMER_BUF[i].tData.DO_MODULE.ip
	 		)
		  )
		{
			memcpy(&(DO_TIMER_BUF[i].tData), &tData, sizeof(tData));
			DO_TIMER_BUF[i].TimerOnFlag = TRUE;
			bRet = TRUE;
			break;
		}
	}
#ifdef DO_DBG
	DoTimerBufShow();
#endif
	return bRet;
}

void TimerSendCmd2DO(TIME_DO_MODULE_t tData)
{
	BYTE cmd = tData.cmd;
	BYTE port_id = tData.port_id;
//	unsigned short value =  tData.value;
	unsigned short port = tData.DO_MODULE.UDP_port;
	unsigned int ip = tData.DO_MODULE.ip;
	unsigned short count = tData.count;
	unsigned int baud_rate = tData.DO_MODULE.baud_rate;
	BYTE data_bits = tData.DO_MODULE.data_bits;
	BYTE parity_flag = tData.DO_MODULE.parity_flag;
	BYTE stop_bits = tData.DO_MODULE.stop_bits;
	BYTE modbus_addr = tData.DO_MODULE.modbus_addr;
	
	if (COMM_WAY_ETH == tData.DO_MODULE.comm_way) // 以太网控制DO
	{
		if (0x05 == tData.cmd)
		{
			comm_with_DO_by_eth(ip,port,cmd,(unsigned short)port_id, 1, COIL_OFF);
		}
		else if (0x0f == cmd)
		{
			comm_with_DO_by_eth(ip,port,cmd,(unsigned short)port_id, count, 0x0000);
		}
	}
	else // RS485 控制DO
	{
		set_serial_para(baud_rate, data_bits, parity_flag, stop_bits);	
		if (0x05 == cmd)
		{
			comm_with_DO_by_serial(modbus_addr, cmd,(unsigned short)port_id, 1, COIL_OFF);
		}
		else if (0x0f == cmd)
		{
			comm_with_DO_by_serial(modbus_addr, cmd, (unsigned short)port_id, count, 0x0000);
		}
	}
}

BOOL LC_thread_state()
{
	BOOL bRet = TRUE;

	if(!lc_thread_state && modbus_LT_para_init_state)
		bRet = FALSE;

	return bRet;
}

static BOOL _visitor_call_lift(char* call_code, char* called_code)
{
	DAC_PARA * DACS[MAX_LIFT] = {NULL};
	RESIDENT_INFO *GM = NULL, *Residents[MAX_LIFT] = {NULL};
	int D_index, R_index, DACS_count,i;

	GetDACS(DACS,call_code);
	if(NULL == DACS[0])
	{
		GM = get_resident_node(call_code);
		if(NULL == GM)
			return FALSE;
		else	// 兼容住户设置中配置门口机代码的方式
		{
			memcpy(DACS[0]->door_code,GM->resident_code,IDENTITY_LEN);
			DACS[0]->floor = GM->floor;
			DACS[0]->call_lift_type = CALL_LIFT_BY_RESIDENT;
			DACS[0]->lift_No = GM->lift_No_1;
			DACS[0]->door_type = GM->lift_No_1_door;
		}
	}
	GetResidents(Residents,called_code);
	if(NULL == Residents[0])
		return FALSE;

	switch(DACS[0]->call_lift_type)
	{
		case CALL_LIFT_BY_DOOR:
			DACS_count = MAX_LIFT;
			break;
		case CALL_LIFT_BY_RESIDENT:
			DACS_count = 1;
			break;
	}
	printf("DACS_count is %d\n",DACS_count);

	
	for(D_index=0; D_index<DACS_count; D_index++)
	{
		if(NULL == DACS[D_index])
			break;
		for(R_index=0; R_index<MAX_LIFT; R_index++)
		{
			DWORD call_lift_way[WAY_COUNT] = {0};
			BYTE door_type[WAY_COUNT] = {0}, opt = 0;
			
			if(NULL == Residents[R_index])
				break;
			if(DACS[0]->call_lift_type == CALL_LIFT_BY_DOOR)
			{
				if(Residents[R_index]->lift_No_1 != DACS[D_index]->lift_No)
					continue;
			}

			door_type[IN] = DACS[D_index]->door_type;
			door_type[OUT] = Residents[R_index]->lift_No_1_door;

			opt = GETPRESSKEY(DACS[D_index]->floor,Residents[R_index]->floor);

			for(i=0; i< WAY_COUNT; i++)
			{
				switch(door_type[i])
				{
					case 0:// 前门
						call_lift_way[i] = ENTRY_TYPE_FRONT | ENTRY_TYPE_DOOR;
						break;
					case 1:// 后门
						call_lift_way[i] = ENTRY_TYPE_BACK | ENTRY_TYPE_DOOR;
						break;
					case 2:// 前后门
						call_lift_way[i] = ENTRY_TYPE_BACK | ENTRY_TYPE_FRONT | ENTRY_TYPE_DOOR;
						break;
				}
				if(IN == i)
					call_lift_way[i] |= CALL_LIFT_WAY_KEY;
				else if(OUT == i)
					call_lift_way[i] |= CALL_LIFT_WAY_UNLOCK;
			}
			
			call_lift(Residents[R_index]->lift_No_1, DACS[D_index]->floor, call_lift_way[IN], opt); // 招梯到GM 所在楼?
			usleep(DELAY_TIME);
			if (CALL_LIFT_WAY_KEY_AND_UNLOCK == common_para.call_lift_way_1)
			{	
				call_lift(Residents[R_index]->lift_No_1, Residents[R_index]->floor, call_lift_way[OUT], PRESS_KEY_UNLOCK);
				usleep(DELAY_TIME);
			}
		}
	}
	return TRUE;
}

static BOOL _HV_call_HV_call_lift(char* call_code, char* called_code)
{
	RESIDENT_INFO *Inviter[MAX_LIFT]= {NULL}, *Visitor[MAX_LIFT] = {NULL};

	GetResidents(Inviter, called_code);
	GetResidents(Visitor, call_code);
	
	if (NULL != Inviter[0] && NULL != Visitor[0])
	{	
		int inviter_index = 0, visitor_index = 0, i;
		BYTE opt = 0;

		opt = GETPRESSKEY(Visitor[0]->floor,Inviter[0]->floor);
		for(inviter_index = 0; inviter_index < MAX_LIFT; inviter_index++)
		{
			if(NULL == Inviter[inviter_index])
				break;
			for(visitor_index = 0; visitor_index < MAX_LIFT; visitor_index++)
			{
				DWORD call_lift_way[WAY_COUNT] = {0};
				BYTE door_type[WAY_COUNT] = {0};
			
				if(NULL == Visitor[visitor_index])
					break;
				if(Inviter[inviter_index]->lift_No_3 != Visitor[visitor_index]->lift_No_3)
					continue;
				door_type[IN] = Visitor[visitor_index]->lift_No_3_door;
				door_type[OUT] = Inviter[inviter_index]->lift_No_3_door;
				for(i =0; i< WAY_COUNT; i++)
				{
					switch(door_type[i])
					{
					case 0:// 前门
						call_lift_way[i] = ENTRY_TYPE_FRONT | ENTRY_TYPE_DEF;
						break;
					case 1:// 后门
						call_lift_way[i] = ENTRY_TYPE_BACK | ENTRY_TYPE_DEF;
						break;
					case 2:// 前后门
						call_lift_way[i] = ENTRY_TYPE_BACK | ENTRY_TYPE_FRONT | ENTRY_TYPE_DEF;
						break;
					}
					if(IN == i)
						call_lift_way[IN] |= CALL_LIFT_WAY_KEY;
					else if(OUT == i)
						call_lift_way[OUT] |= CALL_LIFT_WAY_UNLOCK;
				}
				
				call_lift(Visitor[visitor_index]->lift_No_3, Visitor[visitor_index]->floor, call_lift_way[IN], opt); //4 招梯到visitor 所在楼层
				usleep(DELAY_TIME);
				if (CALL_LIFT_WAY_KEY_AND_UNLOCK == common_para.call_lift_way_2)
				{
					call_lift(Inviter[inviter_index]->lift_No_3, Inviter[inviter_index]->floor, call_lift_way[OUT], PRESS_KEY_UNLOCK);
					usleep(DELAY_TIME);
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

static BOOL _HV_call_lift(char* call_code, char direction)
{
	RESIDENT_INFO *Residents[MAX_LIFT] = {NULL};

	GetResidents(Residents,call_code);
	
	if (NULL != Residents[0])
	{
		int i = 0;
		for(i = 0; i < MAX_LIFT; i++)
		{
			DWORD call_lift_way = 0;
			BYTE door_type = 0;
			if(NULL == Residents[i])
				break;

			door_type = Residents[i]->lift_No_2_door;
			switch(door_type)
			{
				case 0:// 前门
					call_lift_way = ENTRY_TYPE_FRONT | ENTRY_TYPE_DEF | CALL_LIFT_WAY_KEY;
					break;
				case 1:// 后门
					call_lift_way = ENTRY_TYPE_BACK | ENTRY_TYPE_DEF | CALL_LIFT_WAY_KEY;
					break;
				case 2:// 前后门
					call_lift_way = ENTRY_TYPE_BACK | ENTRY_TYPE_FRONT | ENTRY_TYPE_DEF | CALL_LIFT_WAY_KEY;
					break;
			}
			
			call_lift(Residents[i]->lift_No_2, Residents[i]->floor, call_lift_way, direction);
			usleep(DELAY_TIME);
		}
	}	
	return TRUE;
}

static BOOL _swiper_card_call_lift(char* call_code, char* called_code)
{
	DAC_PARA * DACS[MAX_LIFT] = {NULL};
	RESIDENT_INFO *GM = NULL, *Residents[MAX_LIFT] = {NULL};
	int D_index, R_index, DACS_count,i;
    BOOL bGetFlag = FALSE;

	GetDACS(DACS,call_code);
	if(NULL == DACS[0])
	{
        printf("DACS[0] is NULL\n");
        bGetFlag = FALSE;
		GM = get_resident_node(call_code);
		if(NULL == GM)
		{
			return FALSE;
		}
        else
		{
            DACS[0] = (DAC_PARA*)malloc(sizeof(DAC_PARA));              //added by [MichaelMa] at 28-9-2012
            memset(DACS[0],0,sizeof(DAC_PARA));
			memcpy(DACS[0]->door_code,GM->resident_code,IDENTITY_LEN);
			DACS[0]->floor = GM->floor;
			DACS[0]->call_lift_type = CALL_LIFT_BY_RESIDENT;
			DACS[0]->lift_No = GM->lift_No_4;
			DACS[0]->door_type = GM->lift_No_4_door;
		}
	}
    else
    {
        bGetFlag = TRUE;
    }
	GetResidents(Residents,called_code);
	if(NULL == Residents[0])
	{
        printf("Residents[0] is NULL\n");
        if(DACS[0] != NULL && !bGetFlag)
        {
            free(DACS[0]);
            printf("free memory of DACS[0]\n");
        }
        DACS[0] = NULL;
        GM = NULL;
		return FALSE;
	}
	switch(DACS[0]->call_lift_type)
	{
		case CALL_LIFT_BY_DOOR:
			DACS_count = MAX_LIFT;
			break;
		case CALL_LIFT_BY_RESIDENT:
			DACS_count = 1;
			break;
	}
	printf("DACS_count is %d\n",DACS_count);

	
	for(D_index=0; D_index<DACS_count; D_index++)
	{
		if(NULL == DACS[D_index])
			break;
		for(R_index=0; R_index<MAX_LIFT; R_index++)
		{
			DWORD call_lift_way[WAY_COUNT] = {0};
			BYTE door_type[WAY_COUNT] = {0}, opt = 0;
			
			if(NULL == Residents[R_index])
				break;
			if(DACS[0]->call_lift_type == CALL_LIFT_BY_DOOR)
			{
				if(Residents[R_index]->lift_No_4 != DACS[D_index]->lift_No)
					continue;
			}

			door_type[IN] = DACS[D_index]->door_type;
			door_type[OUT] = Residents[R_index]->lift_No_4_door;

			opt = GETPRESSKEY(DACS[D_index]->floor,Residents[R_index]->floor);

			for(i=0; i< WAY_COUNT; i++)
			{
				switch(door_type[i])
				{
					case 0:// 前门
						call_lift_way[i] = ENTRY_TYPE_FRONT | ENTRY_TYPE_DOOR;
						break;
					case 1:// 后门
						call_lift_way[i] = ENTRY_TYPE_BACK | ENTRY_TYPE_DOOR;
						break;
					case 2:// 前后门
						call_lift_way[i] = ENTRY_TYPE_BACK | ENTRY_TYPE_FRONT | ENTRY_TYPE_DOOR;
						break;
				}
				if(IN == i)
					call_lift_way[i] |= CALL_LIFT_WAY_KEY;
				else if(OUT == i)
					call_lift_way[i] |= CALL_LIFT_WAY_UNLOCK;
			}
			
			call_lift(Residents[R_index]->lift_No_4, DACS[D_index]->floor, call_lift_way[IN], opt); // 招梯到GM 所在楼?
			usleep(DELAY_TIME);
			if (CALL_LIFT_WAY_KEY_AND_UNLOCK == common_para.call_lift_way_3)
			{	
				call_lift(Residents[R_index]->lift_No_4, Residents[R_index]->floor, call_lift_way[OUT], PRESS_KEY_UNLOCK);
				usleep(DELAY_TIME);
			}
		}
	}
    if(DACS[0] != NULL && !bGetFlag)
    {
        free(DACS[0]);
        printf("free memory of DACS[0]\n");
    }
    DACS[0] = NULL;
    GM = NULL;
	return TRUE;
}

static BOOL _liftcar_swiper_card(char* call_code, BYTE addr, DWORD rip)
{
	LIFT_PARA *lift = NULL; 
	RESIDENT_INFO *Residents[MAX_LIFT] = {NULL};
	
	lift = get_lift_No(addr);
	
	if (NULL == lift)
	{
		send_ack_to_card_reader(0x1, rip);
		return FALSE;
	}

	if (!strcmp(CODE_ADMIN, call_code)) // 管理员卡
	{
		printf("Jason--->admin lift card to unlock all floors\n");
		send_ack_to_card_reader(0x0, rip);
		unlock_all_floors(lift->lift_No, PRESS_KEY_UNLOCK);
	}
	else
	{
		int i = 0;
		BOOL flag = FALSE;
		GetResidents(Residents,call_code);
		for(i = 0; i < MAX_LIFT; i++)
		{
			DWORD call_lift_way = 0;
			BYTE door_type = 0;
			if(NULL == Residents[i])
				break;
			if(Residents[i]->lift_No_4 != lift->lift_No)
				continue;
			flag = TRUE;
			door_type = Residents[i]->lift_No_4_door;
			switch(door_type)
			{
				case 0:// 前门
					call_lift_way = ENTRY_TYPE_FRONT | ENTRY_TYPE_LIFTCAR | CALL_LIFT_WAY_UNLOCK;
					break;
				case 1:// 后门
					call_lift_way = ENTRY_TYPE_BACK | ENTRY_TYPE_LIFTCAR | CALL_LIFT_WAY_UNLOCK;
					break;
				case 2:// 前后门
					call_lift_way = ENTRY_TYPE_BACK | ENTRY_TYPE_FRONT | ENTRY_TYPE_LIFTCAR | CALL_LIFT_WAY_UNLOCK;
					break;
			}
			
			call_lift(lift->lift_No, Residents[i]->floor, call_lift_way, PRESS_KEY_UNLOCK);
			usleep(DELAY_TIME);
		}
		
		if (0 == strcmp(CODE_INVALID,call_code) || NULL == Residents[0] || FALSE == flag)
		{
			send_ack_to_card_reader(0x1, rip);
			return FALSE;
		}
		else
			send_ack_to_card_reader(0x0, rip);
	}			
	return TRUE;
}
