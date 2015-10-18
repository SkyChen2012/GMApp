/*hdr
**
**	Copyright Mox Group
**
**	FILE NAME:	hash.c
**
**	AUTHOR:		Denny Du
**
**	DATE:		26 - 12 - 2014
**
**	FILE DESCRIPTION:
**				hash�㷨���ڿ�Ƭ��Ϣ����
**
**	FUNCTIONS:
**
**	NOTES:
**		
**	
*/

#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hash.h"

//#define HASH_DEBUG	//debug printf switch
#ifdef HASH_DEBUG
  #define DEBUG_PRINTF(...)  printf( __VA_ARGS__)
#else
	#define DEBUG_PRINTF(...)  {} 
#endif


#define FLASH_BASE_FUNC			0								//0x00000000
#define FLASH_BASE_LIST			FLASH_BASE_FUNC+0x40000			//0x00040000
#define FLASH_BASE_EMPTY_LIST	FLASH_BASE_LIST+0x64000			//0x000A4000
#define FLASH_BASE_DATA			FLASH_BASE_EMPTY_LIST+0x19000	//0x000BD000
#define CARD_DATA_EDGE			FLASH_BASE_DATA+0x400000		//0x004BD000
#define COMPRESS_OFFSET			CARD_DATA_EDGE+0x3000			//0x004C0000
												//+0x40000		//0x00500000

#define CARD_DATA_SUM		51000								//��Ƭ����
#define CARD_DATA_LEN		80									//��Ƭ��Ϣ������
#define CARD_DATA_SUM_PER_SECTOR	51							//ÿ�������ɴ濨Ƭ����(4096/80)
#define HASH_FUNC_SUM_PER_PACKAGE	256							//1024����4

#define HASH_FUNCTION_SUM	0x10000								//hashֵ��0��0xFFFF
#define HASH_EMPTY_LIST_SECTOR_SUM	25							//51000�ſ�Ƭ��Ҫ25������������ǿ��б�

#define USED_FLAG_TRUE		0								
#define USED_FLAG_FALSE		0xFF
		
#define ADDRESS2SECTOR(addr) (addr&0xFFFFF000)					//��flash��ַת��Ϊ������������ַ


 
typedef enum list_status
{
	LIST_STATUS_ONLY_ONE = 0,
	LIST_STATUS_FIRST,
	LIST_STATUS_MIDDLE,
	LIST_STATUS_LAST
}LIST_STATUS;

typedef struct
{
	U16 Func2List;
	U8  UsedFlag;
	U8  Reserved;
}HASH_Func;


typedef struct
{
	U16 HashList_Next;
	U16 HashDataAddr;
	U8	HashUsedFlag;
	U8	CardType;
	U8	Reserved[2];
}HASH_List;

typedef struct
{
	U16	card_sum;
	U16 access_sum;
	U16 lift_sum;
	U16 patrol_sum;
	U32 Version;
}HASH_FLAG;

typedef struct
{
	U16	Offset;
	U16 Func2List;
}HASH_Comp;


#ifdef HASH_SUPPORT_GM_FILE_SYSTEM

/**
 * \brief   ���ļ�ϵͳ��ͬ����RAM��
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��.
 *
 * \return  �Ƿ�ͬ���ɹ�
 **/
BOOL SyncHashFile2RAM(HANDLE * pCardHandle)
{
	FILE* fd;
	U8 *SyncDataBuf = (U8 *)pCardHandle;
	
	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);

	if ((fd = fopen(AHASHFILE, "r+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fread(SyncDataBuf, SYNCDATABUFSIZE, (size_t)1, fd);
		
		fclose(fd);
	}
	else
	{
		DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
		return FALSE;
	}

	return TRUE;
}
#endif

/**
 * \brief   ���ļ�ϵͳ�ж�ȡָ������
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   scr	����Դƫ�Ƶ�ַ
 * \param	length	���ݳ���
 * \param	dst	�������ݿռ�
 *
 * \return  none
 **/
static void  read_hash_data(HANDLE * pCardHandle,U32 scr,U32 length,U8* dst)
{
	U8 *SyncDataBuf = (U8 *)pCardHandle;
	memcpy((void *)dst,&SyncDataBuf[scr],length);
}

/**
 * \brief   ������д���ļ�ϵͳָ��λ��
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   scr	����Դƫ�Ƶ�ַ
 * \param	length	���ݳ���
 * \param	dst	�������ݿռ�
 *
 * \return  none
 **/
static void write_hash_data(HANDLE * pCardHandle,U32 dst,U8* scr,U32 length)
{
	FILE* fd;
	U8 *SyncDataBuf = (U8 *)pCardHandle;

	DEBUG_PRINTF("%s,%d,dst:0x%x,len:%d\r\n",__func__,__LINE__,dst,length);
	memcpy(&SyncDataBuf[dst],(void *)scr,length);

#ifdef HASH_SUPPORT_GM_FILE_SYSTEM	
	//�򿪿ɶ�д���ļ������ļ��������
	if ((fd = fopen(AHASHFILE, "r+")) != NULL)
	{
		fseek(fd, dst, SEEK_SET);
		fwrite(&SyncDataBuf[dst], length, (size_t)1, fd);
		
		fclose(fd);
	}
	else
	{
		DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
	}
#endif
	
}


/**
 * \brief   ��hash func�л�ȡlist��ַ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   hash_value hashֵ
 *
 * \return  list��ַƫ��
 **/
static U16 get_list_addr(HANDLE * pCardHandle,U16 hash_value)
{
	HASH_Func data;
	DEBUG_PRINTF("%s,%d,hash_value:0x%x\r\n",__func__,__LINE__,hash_value);

	read_hash_data(pCardHandle,FLASH_BASE_FUNC+sizeof(HASH_Func)*hash_value,sizeof(HASH_Func),(U8 *)&data);

	DEBUG_PRINTF("%s,%d,data.UsedFlag:0x%x,data.Func2List:0x%x\r\n",__func__,__LINE__,data.UsedFlag,data.Func2List);
	
	if(USED_FLAG_TRUE == data.UsedFlag)
		return data.Func2List;
	else
		return 0xFFFF;

}

/**
 * \brief   ��list��ַд���Ӧ��hashֵӳ��λ��
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   hash_value 	hashֵ
 * \param   data 		list��ַ
 *
 * \return  none
 **/
static void set_list_addr(HANDLE * pCardHandle,U16 hash_value,U16 data)
{
	HASH_Func tmp;

	read_hash_data(pCardHandle,hash_value*sizeof(HASH_Func)+FLASH_BASE_FUNC,sizeof(HASH_Func),(U8 *)&tmp);

	if(0xFFFF == data)
	{
		memset(&tmp,0xFF,sizeof(HASH_Func));
	}
	else
	{
		tmp.Func2List = data;
		tmp.UsedFlag  = 0;
	}
	
	write_hash_data(pCardHandle,hash_value*sizeof(HASH_Func)+FLASH_BASE_FUNC,(U8 *)&tmp,sizeof(HASH_Func));
}

/**
 * \brief   ����ͻlist��ӵ�func�б���
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   hash_value		hashֵ
 * \param   list_addr		hashֵ��Ӧ��list��ַƫ��
 *
 * \return  none
 **/
static void add_list_2_func(HANDLE * pCardHandle,U16 hash_value,U16 list_addr)
{
	HASH_Func tmp;
	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);

	tmp.Func2List = list_addr;
	tmp.UsedFlag  = 0;
	write_hash_data(pCardHandle,hash_value*sizeof(HASH_Func),(U8 *)&tmp,sizeof(HASH_Func));
}


/**
 * \brief   ��list�л�ȡ��Ƭ��Ϣ��ַƫ��
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   list_addr  	list��ַ
 * \param	*data		���ڱ����ȡ�Ŀ�Ƭ��Ϣ
 *
 * \return  none
 **/
static void get_card_info_addr(HANDLE * pCardHandle,U16 list_addr,HASH_List *data)
{
	read_hash_data(pCardHandle,FLASH_BASE_LIST+sizeof(HASH_List)*list_addr,sizeof(HASH_List),(U8 *)data);

	DEBUG_PRINTF("%s,%d,list_addr:0x%x,N:0x%x,A:0x%x,F:0x%x\r\n",__func__,__LINE__,list_addr,
			data->HashList_Next,data->HashDataAddr,data->HashUsedFlag);
}




/**
 * \brief   �����Ż����hashֵ
 *
 * \param   *CSN	����
 *
 * \return  hashֵ
 **/
 static U16 hash_function(BYTE *CSN)
{
	U16 value;	

	value = (CSN[0]+(CSN[1]*2)+(CSN[2]*3)+(CSN[3]*4)+(CSN[4]*5))<<8;
	value += (CSN[0]*9)+(CSN[1]*8)+(CSN[2]*7)+(CSN[3]*6)+(CSN[4]*5);

	DEBUG_PRINTF("%s,%d,CSN:%x-%x-%x-%x-%x,hash:0x%x\r\n",__func__,__LINE__,
			CSN[0],CSN[1],CSN[2],CSN[3],CSN[4],value);
	
	return (U16)value;
}

/**
 * \brief   ��ȡ��Ƭ��Ϣ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   address		��Ƭƫ�Ƶ�ַ
 * \param   *Card		���ڱ��濨Ƭ��Ϣ
 *
 * \return  none
 **/
static void get_card_id(HANDLE * pCardHandle,U16 address,CDINFO *Card)
{
	U32 sec_num = address/CARD_DATA_SUM_PER_SECTOR;
	U32 sector_n = ADDRESS2SECTOR(FLASH_BASE_DATA+0x1000*sec_num);
	
	read_hash_data(pCardHandle,(sector_n+((address%CARD_DATA_SUM_PER_SECTOR)*CARD_DATA_LEN)),sizeof(CDINFO),(U8 *)Card);
#ifdef HASH_SUPPORT_GM_FILE_SYSTEM
	DEBUG_PRINTF("%s,%d,address:0x%x,data:%x-%x-%x-%x-%x\r\n",__func__,__LINE__,address,
			(U8 *)Card->CSN[0],(U8 *)Card->CSN[1],(U8 *)Card->CSN[2],(U8 *)Card->CSN[3],(U8 *)Card->CSN[4]);
#endif
}




/**
 * \brief   ����Ƭ��Ϣд��ָ����ַ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   *Card		��Ƭ��Ϣ
 * \param   address		���ݿռ�ƫ�Ƶ�ַ
 *
 * \return  none
 **/
static void write_data_2_mem(HANDLE * pCardHandle,CDINFO *Card,U16 address)
{
	
	U32 sec_num = address/CARD_DATA_SUM_PER_SECTOR;
	U32 sector_n = ADDRESS2SECTOR(FLASH_BASE_DATA+0x1000*sec_num);
	DEBUG_PRINTF("%s,%d,address;0x%x,sec_num:0x%x,sector_n:0x%x\r\n",__func__,__LINE__,address,sec_num,sector_n);

	write_hash_data(pCardHandle,sector_n+((address%CARD_DATA_SUM_PER_SECTOR)*CARD_DATA_LEN),(U8 *)Card,sizeof(CDINFO));

}


/**
 * \brief   ������list����empty�б�
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   address		����listƫ�Ƶ�ַ
 *
 * \return  �Ƿ���ӳɹ�
 **/
static int add_list_2_empty_list(HANDLE * pCardHandle,U16 address)
{
	U16 data;
	U32 i;
	
	DEBUG_PRINTF("%s,%d,address:0x%x\r\n",__func__,__LINE__,address);

	for(i=0;i<(2*CARD_DATA_SUM);i+=2)
	{
		read_hash_data(pCardHandle,FLASH_BASE_EMPTY_LIST+i,2,(void *)&data);
		if(0xFFFF == data)
		{
			DEBUG_PRINTF("%s,%d,empty offset:0x%x\r\n",__func__,__LINE__,i);
			write_hash_data(pCardHandle,FLASH_BASE_EMPTY_LIST+i,(U8 *)&address,2);
			return 0;
		}
	}

	return -1;
}


/**
 * \brief   ��ȡ���õ�empty list
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   *list		�����ȡ��list��Ϣ
 *
 * \return  listƫ�Ƶ�ַ
 **/
static int get_empty_list(HANDLE * pCardHandle,HASH_List *list,U8 CardType)
{
	U16 data,temp;
	int i;
	HASH_FLAG data_edge;
	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);

	for(i=0;i<(2*CARD_DATA_SUM);i+=2)
	{
		read_hash_data(pCardHandle,FLASH_BASE_EMPTY_LIST+i,2,(void *)&data);
		if(0xFFFF != data)
		{
			DEBUG_PRINTF("%s,%d,i:%d,data:0x%x\r\n",__func__,__LINE__,i,data);

			read_hash_data(pCardHandle,FLASH_BASE_LIST+data*sizeof(HASH_List),sizeof(HASH_List),(U8 *)list);
			
			memset((void *)&temp,0xFF,2);

			write_hash_data(pCardHandle,FLASH_BASE_EMPTY_LIST+i,(U8 *)&temp,2);

			return data;
		}			
	}

	//empty list û�п�����Դ,ֱ����data��		
	read_hash_data(pCardHandle,CARD_DATA_EDGE,sizeof(HASH_FLAG),(U8 *)&data_edge);
	
	data_edge.card_sum++;
	
	if(TYPE_NORMAL_CARD == CardType)
	{
		data_edge.access_sum++;
	}
	else if(TYPE_PATROL_CARD == CardType)
	{
		data_edge.patrol_sum++;
	}
	else if(TYPE_LIFT_CARD == CardType)
	{
		data_edge.lift_sum++;
	}
	
	write_hash_data(pCardHandle,CARD_DATA_EDGE,(U8 *)&data_edge,sizeof(HASH_FLAG));
	list->HashDataAddr = data_edge.card_sum;
	DEBUG_PRINTF("%s,%d,card_sum:0x%x\r\n",__func__,__LINE__,data_edge.card_sum);

	return data_edge.card_sum;
}

/**
 * \brief   ��ȡlist��ͻ�б������һ����Աƫ�Ƶ�ַ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   first_address		��ͻ�б��е�һ����Աƫ�Ƶ�ַ
 *
 * \return  ���һ����Աƫ�Ƶ�ַ
 **/
static U16 get_last_list_addr(HANDLE * pCardHandle,U16 first_address)
{
	HASH_List list;
	U16 list_addr = first_address;
	DEBUG_PRINTF("%s,%d,first:0x%x\r\n",__func__,__LINE__,first_address);

	do{
		get_card_info_addr(pCardHandle,list_addr,&list);

		if(0xFFFF != list.HashList_Next)//��ǰԪ�ز�����β
		{
			list_addr = list.HashList_Next;//�ȴ����²���
		}
		else
		{
			return list_addr;
		}
		
	}while(0==list.HashUsedFlag);

	return 0xFFFF;
}


/**
 * \brief   ���µ�list���ӵ�������
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param   current		��ǰ��Ա
 * \param   next		��һ����Ա
 * \param   data_address ��Ƭ���ݵ�ַ
 *
 * \return  none
 **/
static void link_empty_list(HANDLE * pCardHandle,U16 current,U16 next,U16 data_address,U8 CardType)
{
	HASH_List tmp;
	DEBUG_PRINTF("%s,%d,c:0x%x,n:0x%x\r\n",__func__,__LINE__,current,next);

	memset(&tmp,0xFF,sizeof(HASH_List));

	tmp.CardType = CardType;

	if(0xFFFF == current)
	{
		tmp.HashUsedFlag = 0;
		tmp.HashDataAddr = data_address;
		write_hash_data(pCardHandle,FLASH_BASE_LIST+sizeof(HASH_List)*next,(U8 *)&tmp,sizeof(HASH_List));
	}
	else
	{
		write_hash_data(pCardHandle,FLASH_BASE_LIST+sizeof(HASH_List)*current,(U8 *)&next,2);
		
		tmp.HashUsedFlag = 0;
		tmp.HashDataAddr = data_address;
		write_hash_data(pCardHandle,FLASH_BASE_LIST+sizeof(HASH_List)*next,(U8 *)&tmp,sizeof(HASH_List));
	}
}



/**
 * \brief  	ɾ����ǰ�б�
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ�� 
 * \param   list_addr		��ǰ�б��ַ
 *
 * \return  none
 **/
static void delete_current_list(HANDLE * pCardHandle,U16 list_addr)
{
	HASH_List list;

	DEBUG_PRINTF("%s,%d,list_addr:0x%x\r\n",__func__,__LINE__,list_addr);
	read_hash_data(pCardHandle,FLASH_BASE_LIST+list_addr*sizeof(HASH_List),sizeof(HASH_List),(U8 *)&list);
	list.HashList_Next = 0xFFFF;
	list.HashUsedFlag  = 0xFF;
	list.CardType	   = 0xFF;
	write_hash_data(pCardHandle,FLASH_BASE_LIST+list_addr*sizeof(HASH_List),(U8 *)&list,sizeof(HASH_List));

}

/**
 * \brief   �޸�ǰһ���б�
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ�� 
 * \param   list_prev	ǰһ���б��ַ
 * \param   list_n		��һ���б��Ա��ַ
 *
 * \return  none
 **/
static void modify_previous_list(HANDLE * pCardHandle,U16 list_prev,U16 list_n)
{
	HASH_List tmp;
	DEBUG_PRINTF("%s,%d,list_prev:0x%x,list_n:0x%x\r\n",__func__,__LINE__,list_prev,list_n);

	read_hash_data(pCardHandle,FLASH_BASE_LIST+list_prev*sizeof(HASH_List),sizeof(HASH_List),(U8 *)&tmp);

	tmp.HashList_Next = list_n;

	write_hash_data(pCardHandle,FLASH_BASE_LIST+list_prev*sizeof(HASH_List),(U8 *)&tmp,sizeof(HASH_List));
}

/**
 * \brief   ɾ����Ƭ����
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ�� 
 * \param   HashDataAddr	��Ƭ���ڵ�ַ
 *
 * \return  none
 **/
static void delete_card_data(HANDLE * pCardHandle,U16 HashDataAddr)
{
	U8 tmp[CARD_DATA_LEN];
	U32 sec_num = HashDataAddr/CARD_DATA_SUM_PER_SECTOR;
	U32 sector_n = ADDRESS2SECTOR(FLASH_BASE_DATA+0x1000*sec_num);
	DEBUG_PRINTF("%s,%d,address;0x%x,sec_num:0x%x,sector_n:0x%x\r\n",__func__,__LINE__,HashDataAddr,sec_num,sector_n);

	memset((void *)tmp,0xFF,CARD_DATA_LEN);
	write_hash_data(pCardHandle,sector_n+((HashDataAddr%CARD_DATA_SUM_PER_SECTOR)*CARD_DATA_LEN),tmp,CARD_DATA_LEN);

}


/**
 * \brief   ���ҿ�Ƭ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ�� 
 * \param   *CSN		����
 * \param   CardType	���ڱ�ǿ�Ƭ����
 * \param   *Card		���ڱ��濨Ƭ��Ϣ
 * \param   *list_c		��Ӧ��������list��Ϣ
 * \param   *list_prev	ǰһ����������list��Ϣ
 * \param   *list_status	�б�״̬
 *
 * \return  ��Ӧlistƫ�Ƶ�ַ
 **/
static U16 SearchCard(HANDLE * pCardHandle,BYTE *CSN,U8 CardType,CDINFO *Card,HASH_List *list_c,U16 *list_prev,LIST_STATUS *list_status)
{	
	U16 list_addr;	
	U16 hash_value = hash_function(CSN);
	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);

	list_addr = get_list_addr(pCardHandle,hash_value);
	
	if(0xFFFF == list_addr)
	{
		DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
		return 0xFFFF;
	}

	*list_status = LIST_STATUS_ONLY_ONE;

	do{
		get_card_info_addr(pCardHandle,list_addr,list_c);

		switch(*list_status)
		{
			case LIST_STATUS_ONLY_ONE:
			{
				if(0xFFFF == list_c->HashList_Next)
				{
					*list_status = LIST_STATUS_ONLY_ONE;
				}
				else
				{
					*list_status = LIST_STATUS_FIRST;
				}
				break;
			}
			case LIST_STATUS_MIDDLE:
			{
				if(0xFFFF == list_c->HashList_Next)
				{
					*list_status = LIST_STATUS_LAST;
				}
				else
				{
					*list_status = LIST_STATUS_MIDDLE;
				}
				break;

			}
			default:
				break;
		}
				

		if(USED_FLAG_TRUE == list_c->HashUsedFlag)//��ǰԪ���Ѿ�����
		{
			get_card_id(pCardHandle,list_c->HashDataAddr,Card);
			
			DEBUG_PRINTF("CSN :%x-%x-%x-%x-%x,type:%d\r\n",CSN[0],CSN[1],CSN[2],CSN[3],CSN[4],CardType);
			DEBUG_PRINTF("Card:%x-%x-%x-%x-%x,type:%d\r\n",Card->CSN[0],Card->CSN[1],Card->CSN[2],Card->CSN[3],Card->CSN[4],list_c->CardType);
			
			if( (0==memcmp((void *)CSN,(void *)Card->CSN,CSN_LEN)) &&
				(CardType == list_c->CardType) )
			{				
				DEBUG_PRINTF("%s,%d,Card found\r\n",__func__,__LINE__);
				return list_addr;
			}
			else
			{
				*list_status 	= LIST_STATUS_MIDDLE;			
				*list_prev 		= list_addr;
				
				if(list_addr == list_c->HashList_Next)
				{
					DEBUG_PRINTF("%s,%d,search finish\r\n",__func__,__LINE__);
					return 0xFFFF;
				}
				else
				{
					list_addr = list_c->HashList_Next;
					DEBUG_PRINTF("%s,%d,CSN or Type different\r\n",__func__,__LINE__);
				}
			}
		}
		else if(USED_FLAG_FALSE == list_c->HashUsedFlag)//��ǰԪ��Ϊ��
		{
			DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
			return 0xFFFF;
		}

	}while(1);
	
}



/**
 * \brief   ����
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ�� 
 * \param   *CSN		����
 * \param   *Card		���ڱ��濨Ƭ��Ϣ
 * \param   CardType	���ڱ�ǿ�Ƭ����
 *
 * \return  �Ƿ��ҵ��˿�
 **/
BOOL ReadCard(HANDLE * pCardHandle,BYTE *CSN,U8 CardType,CDINFO *Card)
{
	U16 list_p;
	LIST_STATUS status;
	HASH_List list;
	if(NULL == pCardHandle)
	{
		return FALSE;
	}
	DEBUG_PRINTF("\r\n%s,%d\r\n",__func__,__LINE__);
	if(0xFFFF == SearchCard(pCardHandle,CSN,CardType,Card,&list,&list_p,&status))
	{
		memset((void *)Card,0,sizeof(CDINFO));
		DEBUG_PRINTF("%s,%d,Card non-existed\r\n",__func__,__LINE__);
		return FALSE;
	}
	else
		return TRUE;
}


#ifdef HASH_SUPPORT_GM_FILE_SYSTEM

/**
 * \brief   ɾ����Ƭ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ�� 
 * \param   *Card		��Ƭ��Ϣ
 * \param   CardType	���ڱ�ǿ�Ƭ����
 *
 * \return  �Ƿ�ɾ���ɹ�
 **/
BOOL DeleteCard(HANDLE * pCardHandle,BYTE *CSN,U8 CardType)
{
	HASH_List list;
	CDINFO card_tmp;
	LIST_STATUS status;
	U16 list_addr,list_p;
	U16 hash_value;
	if(NULL == pCardHandle)
	{
		return FALSE;
	}
	DEBUG_PRINTF("\r\n%s,%d\r\n",__func__,__LINE__);

	hash_value = hash_function(CSN);

	list_addr = get_list_addr(pCardHandle,hash_value);
	if(0xFFFF == list_addr)
	{		
		DEBUG_PRINTF("%s,%d,func non-existed\r\n",__func__,__LINE__);
		return FALSE;
	}

	list_addr = SearchCard(pCardHandle,CSN,CardType,&card_tmp,&list,&list_p,&status);

	if(0xFFFF == list_addr)
	{
		DEBUG_PRINTF("%s,%d,Card non-existed\r\n",__func__,__LINE__);
		return FALSE;
	}

	switch(status)
	{
		case LIST_STATUS_ONLY_ONE:
		{
			//ɾlist����empty list��ɾdata����func
			//��func
			DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
			set_list_addr(pCardHandle,hash_value,0xFFFF);
			
			//��ǰһ��
			delete_current_list(pCardHandle,list_addr);
			add_list_2_empty_list(pCardHandle,list_addr);		
			
			//data
			delete_card_data(pCardHandle,list.HashDataAddr);
			break;
		}
		case LIST_STATUS_FIRST:
		{
			//��list��ɾdata����func
			//��func
			DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
			set_list_addr(pCardHandle,hash_value,list.HashList_Next);
			
			//��ǰһ��
			delete_current_list(pCardHandle,list_addr);
			add_list_2_empty_list(pCardHandle,list_addr);
					
			//data
			delete_card_data(pCardHandle,list.HashDataAddr);
			break;
		}
		case LIST_STATUS_MIDDLE:
		{
			//��list��ɾdata
			//��ǰһ��
			DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
			delete_current_list(pCardHandle,list_addr);
			add_list_2_empty_list(pCardHandle,list_addr);
			
			//ǰһ��
			modify_previous_list(pCardHandle,list_p,list.HashList_Next);			
				
			//data
			delete_card_data(pCardHandle,list.HashDataAddr);
			break;
		}
		case LIST_STATUS_LAST:
		{
			//��list��ɾdata
			//���һ��
			DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
			
			delete_current_list(pCardHandle,list_addr);
			add_list_2_empty_list(pCardHandle,list_addr);
			
			//ǰһ��
			modify_previous_list(pCardHandle,list_p,0xFFFF);	
			//data
			delete_card_data(pCardHandle,list.HashDataAddr);
			break;
		}

		default:
			break;

	}
	
	return TRUE;
}
#endif

/**
 * \brief   д�뿨Ƭ��Ϣ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ�� 
 * \param   *Card		��Ƭ��Ϣ
 * \param   CardType	���ڱ�ǿ�Ƭ����
 *
 * \return  �Ƿ�д��ɹ�(�����Ƭ�Ѿ����ڣ�д��ʧ��)
 **/
BOOL WriteCardInfo(HANDLE * pCardHandle,CDINFO *Card,U8 CardType)
{
	HASH_List list,tmp;
	CDINFO card_tmp;
	LIST_STATUS status;
	U16 list_addr,list_p,last_addr,empty_addr;
	BYTE data[5];
	U16 hash_value;
	if(NULL == pCardHandle)
	{
		return FALSE;
	}

	DEBUG_PRINTF("\r\n%s,%d\r\n",__func__,__LINE__);

	memcpy((void *)data,(void *)Card->CSN,CSN_LEN);
	
	hash_value = hash_function(data);

	list_addr = get_list_addr(pCardHandle,hash_value);

	if(0xFFFF != SearchCard(pCardHandle,data,CardType,&card_tmp,&list,&list_p,&status))
	{
		DEBUG_PRINTF("%s,%d,Card exist\r\n",__func__,__LINE__);
		return FALSE;

	}

	empty_addr = get_empty_list(pCardHandle,&tmp,CardType);

	DEBUG_PRINTF("%s,%d,hash_value:0x%x,list_addr:0x%x,empty_addr:0x%x\r\n",__func__,__LINE__,hash_value,list_addr,empty_addr);

	if(0xFFFF == list_addr)//func���޴˳�Ա
	{
		DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
		add_list_2_func(pCardHandle,hash_value,empty_addr);
		link_empty_list(pCardHandle,0xFFFF,empty_addr,tmp.HashDataAddr,CardType);
		write_data_2_mem(pCardHandle,Card,tmp.HashDataAddr);
	}
	else
	{
		DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
		last_addr = get_last_list_addr(pCardHandle,list_addr);
		link_empty_list(pCardHandle,last_addr,empty_addr,tmp.HashDataAddr,CardType);
		write_data_2_mem(pCardHandle,Card,tmp.HashDataAddr);
	}
	return TRUE;
}

#ifdef HASH_SUPPORT_GM_FILE_SYSTEM
/**
 * \brief   ������������Լ��б�
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ�� 
 *
 * \return  none
 **/
void ClearAllCardData(HANDLE * pCardHandle)
{	
	FILE* fd;
	U8 *SyncDataBuf = (U8 *)pCardHandle;
	if(NULL == pCardHandle)
	{
		return FALSE;
	}
	
	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
	
	memset(SyncDataBuf,0xFF,SYNCDATABUFSIZE);

	//�򿪿ɶ�д�ļ������ļ��������ļ�������Ϊ�㣬�����ļ����ݻ���ʧ�����ļ��������������ļ�
	if ((fd = fopen(AHASHFILE, "w+")) != NULL)
	{
		fseek(fd, 0, SEEK_SET);
		fwrite(SyncDataBuf, SYNCDATABUFSIZE, (size_t)1, fd);
		
		fclose(fd);
	}
	else
	{
		DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
		return;
	}

}
#endif


/**
 * \brief   ��ȡ���ݿ�汾��Ϣ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ�� 
 * \param	none
 *
 * \return  ���ݿ�汾
 **/
static int GetHashVersionInfo(HANDLE * pCardHandle)
{
	HASH_FLAG data;
	read_hash_data(pCardHandle,CARD_DATA_EDGE,sizeof(HASH_FLAG),(U8 *)&data);

	return data.Version;
}

/**
 * \brief   ��ȡ��Ƭ����
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 *
 * \return  ��Ƭ����
 **/
U16 GetHashCardSum(HANDLE * pCardHandle)
{
	HASH_FLAG data;
	if(NULL == pCardHandle)
	{
		return FALSE;
	}
	read_hash_data(pCardHandle,CARD_DATA_EDGE,sizeof(HASH_FLAG),(U8 *)&data);

	return data.card_sum+1;
}

/**
 * \brief   ��ȡѹ����Ͱ���ݳ���
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 *
 * \return  ѹ�������ݳ���
 **/
static U32 GetComPFuncLength(HANDLE * pCardHandle)
{
	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
	return 4*GetHashCardSum(pCardHandle);
}

/**
 * \brief   ��ȡѹ�����б���
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 *
 * \return  ѹ�����б����ݳ���
 **/
static U32 GetComPListLength(HANDLE * pCardHandle)
{
	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
	return 8*GetHashCardSum(pCardHandle);
}

/**
 * \brief   ��ȡ��Ƭ���ݳ���
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 *
 * \return  ��Ƭ���ݳ���
 **/
static U32 GetCardDataLength(HANDLE * pCardHandle)
{
	U32 sec_num = GetHashCardSum(pCardHandle)/CARD_DATA_SUM_PER_SECTOR;
	U32	offset = GetHashCardSum(pCardHandle)%CARD_DATA_SUM_PER_SECTOR;
	
	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
	if(0 == offset)
	{
		return 0x1000*sec_num;
	}
	else
	{
		return 0x1000*(sec_num+1);
	}
}

/**
 * \brief   ��hashͰ����ѹ��
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 *
 * \return  none
 **/
static void compress_func(HANDLE * pCardHandle)
{
	U32 i;
	U32 j=0;
	HASH_Func data;
	HASH_Comp index;

	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
	
	for(i=0;i<=0xFFFF;i++)
	{		
		read_hash_data(pCardHandle,FLASH_BASE_FUNC+sizeof(HASH_Func)*i,sizeof(HASH_Func),(U8 *)&data);
		if(0xFF != data.UsedFlag)
		{
			index.Offset = i;
			index.Func2List = data.Func2List;
			write_hash_data(pCardHandle,COMPRESS_OFFSET+sizeof(HASH_Comp)*j,(U8 *)&index,sizeof(HASH_Comp));
			j++;
		}
	}
}

/**
 * \brief   ��hashͰ���ݽ��н�ѹ��
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param	pCardBuff	ѹ������
 * \param	pCardBuffSize ѹ�����ݳ���
 *
 * \return  none
 **/
static BOOL decompress_func(HANDLE * pCardHandle,BYTE *pCardBuff,int pCardBuffSize)
{
	U16 Offset;
	U32 i,sum;
	HASH_Func data;	
	HASH_Comp value;
	
	sum = pCardBuffSize/sizeof(HASH_Comp);
	
	if((0 != pCardBuffSize%sizeof(HASH_Comp))&&(sum > HASH_FUNC_SUM_PER_PACKAGE))
	{
		DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
		return FALSE;
	}
	
	for(i=0;i<sum;i++)
	{	
		memcpy(&value,&pCardBuff[sizeof(HASH_Comp)*i],sizeof(HASH_Comp)); 
		Offset= value.Offset;
		data.Func2List = value.Func2List;
		data.UsedFlag = 0;
		DEBUG_PRINTF("%s,%d,offset;0x%x,list:0x%X\r\n",__func__,__LINE__,Offset,data.Func2List);
		if(0xFFFF != data.Func2List)
		{	
			write_hash_data(pCardHandle,FLASH_BASE_FUNC+sizeof(HASH_Func)*Offset,(U8 *)&data,sizeof(HASH_Func));
		}
	}

	return TRUE;
}

/**
 * \brief   ���ӿ�Ƭ��Ϣ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param	nCardType	��Ƭ����
 * \param	pCardInfo 	��Ƭ������Ϣ
 * \param	nCardNumber	��Ƭ����
 *
 * \return  �ӿ��Ƿ�ɹ�
 **/
BOOL AddCard(HANDLE * pCardHandle,int nCardType,void * pCardInfo,int nCardNumber)
{
	int i;
	U8 *SyncDataBuf = (U8 *)pCardHandle;
	if(NULL == pCardHandle)
	{
		DEBUG_PRINTF("\r\n%s,%d\r\n",__func__,__LINE__);
		return FALSE;
	}
	
	for(i=0;i<nCardNumber;i++)
	{
		WriteCardInfo(pCardHandle,(CDINFO *)(pCardInfo+(sizeof(CDINFO)*i)),(U8)nCardType);
	}
	
	return TRUE;
}


#ifdef HASH_SUPPORT_GM_FILE_SYSTEM

/**
 * \brief   �򿪿�Ƭ���ݿ�
 *
 * \param  	none
 *
 * \return  ��Ƭ����ָ�룬NULLʧ�ܣ���NULL�ɹ�
 **/
HANDLE *OpenCardDataBase(void)
{
	BYTE *buf =NULL;
		
	buf = (U8 *)malloc(SYNCDATABUFSIZE);
	
	if((access(AHASHFILE,F_OK)) == 0)
	{
		DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
		SyncHashFile2RAM((HANDLE *)buf);
	}
	else
	{
		DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
		ClearAllCardData((HANDLE *)buf);
	}
	
	return (HANDLE *)buf;
}
#else
/**
 * \brief   �򿪿�Ƭ���ݿ�
 *
 * \param   pCardBuff 	��Ƭ����Ӧ��ָ��
 * \param	pCardBuffSize	��Ƭ����Ӧ��ָ�����ݳ���
 * \param	DeviceType 	�豸����//1Ϊ�ſڻ���2Ϊ�Ž���3Ϊ�ݿ�
 *
 * \return  ��Ƭ����ָ�룬NULLʧ�ܣ���NULL�ɹ�
 **/
HANDLE *OpenCardDataBase(BYTE *pCardBuff,int pCardBuffSize,int DeviceType)
{
	BYTE *buf =NULL;
	
	buf = (U8 *)malloc(SYNCDATABUFSIZE);
	memset((void*)buf,0xFF,SYNCDATABUFSIZE);

	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
	
	if(NULL != pCardBuff)
	{
		memcpy((void*)(buf+FLASH_BASE_DATA),(void *)pCardBuff,pCardBuffSize); 
	}
	
	return (HANDLE *)buf;
}
#endif

/**
 * \brief   �رտ�Ƭ���ݿ�
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 *
 * \return  �Ƿ�رճɹ�
 **/
BOOL CloseCardDataBase(HANDLE * pCardHandle)
{
	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
	free(pCardHandle);
	pCardHandle = NULL;
	return TRUE;
}

/**
 * \brief   ��տ�Ƭ���ݿ�
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 *
 * \return   �Ƿ���ճɹ�
 **/
BOOL ClearCardDataBase(HANDLE * pCardHandle)
{
	U8 *SyncDataBuf = (U8 *)pCardHandle;
	if(NULL == pCardHandle)
	{
		return FALSE;
	}
	
	DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
	memset(SyncDataBuf,0xFF,SYNCDATABUFSIZE);
	return TRUE;
}

/**
 * \brief   �ڿ�Ƭ�ļ��е������������ڴ���Ϣ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param	nBufType	�������� ��0/hashͰ��1/��ͻ�б�2/��Ƭ���ݣ�3/������Ϣ
 * \param   pCardBuff 	�������������ڴ�ָ��
 * \param	pCardBuffSize	�������������ڴ�ָ�����ݳ���
 *
 * \return   �Ƿ��ȡ�ɹ�
 **/
BOOL GetCardBuffInfo(HANDLE * pCardHandle,int nBufType, BYTE **pCardBuff,int *pCardFileSize)
{
	U8 *SyncDataBuf = (U8 *)pCardHandle;

	if(NULL == pCardHandle)
	{
		return FALSE;
	}
	
	switch(nBufType)
	{
		case BUF_TYPE_HASH_FUNC:
		{
			compress_func(pCardHandle);
			*pCardFileSize = GetComPFuncLength(pCardHandle);
			*pCardBuff = (BYTE *)(SyncDataBuf+COMPRESS_OFFSET);
			break;
		}
		
		case BUF_TYPE_HASH_LIST:
		{
			*pCardFileSize = GetComPListLength(pCardHandle);
			*pCardBuff = (BYTE *)(SyncDataBuf+FLASH_BASE_LIST);
			break;
		}

		case BUF_TYPE_HASH_CARD:
		{
			*pCardFileSize = GetCardDataLength(pCardHandle);
			*pCardBuff = (BYTE *)(SyncDataBuf+FLASH_BASE_DATA);
			break;
		}

		case BUF_TYPE_HASH_INFO:
		{
			*pCardFileSize = sizeof(HASH_FLAG);
			*pCardBuff = (BYTE *)(SyncDataBuf+CARD_DATA_EDGE);
			break;
		}

		default:
			return FALSE;

	}
	DEBUG_PRINTF("%s,%d,size:0x%x,nBufType:%d\r\n",__func__,__LINE__,*pCardFileSize,nBufType);
	return TRUE;
}


/**
 * \brief   д�뵥�����������ڴ���Ϣ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param	nBufType	�������� ��0/hashͰ��1/��ͻ�б�2/��Ƭ���ݣ�3/������Ϣ
 * \param   pCardBuff 	�������������ڴ�ָ��
 * \param	pCardBuffSize	�������������ڴ�ָ�����ݳ���
 * \param	pCardFlieOffset	��Ƭ�����ڴ�ƫ��
 *
 * \return   �Ƿ����óɹ�
 **/
BOOL SetCardBuffInfo(HANDLE * pCardHandle,int nBufType, BYTE *pCardBuff,int pCardFileSize,int	pCardFlieOffset)
{
	if(NULL == pCardHandle)
	{
		return FALSE;
	}
	
	DEBUG_PRINTF("%s,%d,BufType:%d\r\n",__func__,__LINE__,nBufType);
	
	switch(nBufType)
	{
		case BUF_TYPE_HASH_FUNC:
		{
			decompress_func(pCardHandle,pCardBuff,pCardFileSize);
			break;
		}
		
		case BUF_TYPE_HASH_LIST:
		{
			write_hash_data(pCardHandle,FLASH_BASE_LIST+pCardFlieOffset,(U8 *)pCardBuff,pCardFileSize);
			break;
		}

		case BUF_TYPE_HASH_CARD:
		{
			write_hash_data(pCardHandle,FLASH_BASE_DATA+pCardFlieOffset,(U8 *)pCardBuff,pCardFileSize);
			break;
		}

		case BUF_TYPE_HASH_INFO:
		{
			write_hash_data(pCardHandle,CARD_DATA_EDGE+pCardFlieOffset,(U8 *)pCardBuff,pCardFileSize);
			break;
		}

		default:
			return FALSE;

	}
	return TRUE;
}

/**
 * \brief   ��ȡĳ�࿨Ƭ����
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param	nCardType:�������� //2Ϊ�Ž���, 3ΪѲ����, 4Ϊ�ݿؿ�
 *
 * \return  ��Ƭ����
 **/
int GetCardNumber(HANDLE * pCardHandle, int nCardType) 
{
	HASH_FLAG data;
	
	if(NULL == pCardHandle)
	{
		return FALSE;
	}
	
	read_hash_data(pCardHandle,CARD_DATA_EDGE,sizeof(HASH_FLAG),(U8 *)&data);

	if(TYPE_NORMAL_CARD == nCardType)
	{
		return ((data.access_sum+1)&0xFFFF);
	}
	else if(TYPE_PATROL_CARD == nCardType)
	{
		return ((data.patrol_sum+1)&0xFFFF);
	}
	else if(TYPE_LIFT_CARD == nCardType)
	{
		return ((data.lift_sum+1)&0xFFFF);
	}
	else
	{
		return 0;
	}
}

/**
 * \brief   �жϿ����Ƿ���Ч
 *
 * \param   Card	����
 *
 * \return  �����Ƿ���Ч
 **/
static BOOL IsCardIDValid(CDINFO *Card)
{
	if((0xFF==Card->CSN[0])&&
		(0xFF==Card->CSN[1])&&
		(0xFF==Card->CSN[2])&&
		(0xFF==Card->CSN[3])&&
		(0xFF==Card->CSN[4]) )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/**
 * \brief   д�뵥�����������ڴ���Ϣ
 *
 * \param   pCardHandle ��Ƭ�ļ�����ָ��
 * \param	nCardType �������� //2Ϊ�Ž���, 3ΪѲ����, 4Ϊ�ݿؿ�
 * \param   nCardNumber ��ȡ�Ŀ�Ƭ����
 * \param	pCardInfo ��ȡ���Ŀ�Ƭ�ṹ��ָ��, ��Ƭ�ṹ�����
 *
 * \return   �Ƿ��ȡ�ɹ�
 **/
BOOL GetCardInfo( HANDLE * pCardHandle, int nCardType,int *nCardNumber,void * pCardInfo) 
{
	CDINFO Card;
	int i=0;
	int index = 0;
	U8 *SyncDataBuf = (U8 *)pCardHandle;

	if(NULL == pCardHandle)
	{
		return FALSE;
	}
	
	while(i<CARD_DATA_SUM)
	{
		get_card_id(pCardHandle,i++,&Card);
		if(TRUE == IsCardIDValid(&Card))
		{
			if(Card.CardType == nCardType)
			{	
				DEBUG_PRINTF("%s,%d\r\n",__func__,__LINE__);
				memcpy(&pCardInfo[index*sizeof(CDINFO)],(void *)&Card,sizeof(CDINFO));
				index++;			
			}
		}
	}
	*nCardNumber = index;
	return TRUE;
}

