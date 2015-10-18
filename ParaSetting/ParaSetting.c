/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	HPIdispose.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		17 - Oct - 2006
**
**	FILE DESCRIPTION:
**				catagory descripition of functions in this file
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

/************** USER INCLUDE FILES ***************************************************/

#include "MXMem.h"
#include "MXList.h"
#include "MXTypes.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "Dispatch.h"
#include "MXCommon.h"

#include "ParaSetting.h"
#include "MenuParaProc.h"
#include "AccessCommon.h"
#include "CardProc.h"
#include "ModuleTalk.h"
#include "AMT.h"
#include "PwdUlk.h"
#include "JpegApp.h"

/************** DEFINES **************************************************************/
#define PARA_SETTING_DEBUG
/************** TYPEDEFS *************************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/
extern DWORD	g_LocalNum;



int /*0 - false; 1 - OK*/
SndPacket(INT  * pSkt, PBYTE pSd, int nLen, struct sockaddr_in * pAddrTo);

/************** ENTRY POINT DECLARATIONS *********************************************/
VERSION_CONTROL g_VerContrl = {
    0,
    {0},
    {0}
};
UCHAR   DownloadDataBuf[0x800000];//8M

UCHAR  Flash_burn_Flag = 0;
UCHAR  FLAG_AMT_BURN  =  0;

UCHAR	Linkflag = 0;
UCHAR	Pack_Flag = 0;

/************** LOCAL DECLARATIONS ***************************************************/

static  UCHAR   BUF_PCSETTING[2 * (2560)];

static  WORD   FILE_FORMAT                      = 0;
static  long     FILE_LEN                         = 0;
static  DWORD   FILE_LEN_BAK                     = 0;
static  DWORD   FILE_AMT_VERSION                 = 0;
static  DWORD   FILE_COPY_REC                    = 0;
static  WORD   FILE_PACKET_NUM                  = 0;
static  WORD   FILE_PACKET_NUM_LST              = 0;
static  DWORD   FILE_PACKET_LEN                  = 0;
static  WORD   FILE_PACKET_AMOUNT               = 0;
static  DWORD   FLASH_AMT_VERSION                = 0;
static  DWORD   FLASH_AMT_LEN                    = 0;
static  WORD   FlashAMTCRC                      = 0;

static   WORD   check_sum_E2PROM = 0;
static   int     size_E2PROM = 0;

static struct sockaddr_in recv_addr_E2PROM;

static E2PROM_SETTING PC_SETTING_PARAMS; 

static INT  sock_E2PROM		= 0;
static VOID SendACK(E2PROM_SETTING PC_SETTING_PARAMS_PARA,UCHAR ERROR_ID,DWORD RESERVED_VALUE);

static void WriteSysPara(E2PROM_SETTING PCSettingPara);
static void ReadSysPara(E2PROM_SETTING PCSettingPara);

static void WriteSysPara_V2(E2PROM_SETTING PCSettingPara);
static void ReadSysPara_V2(E2PROM_SETTING PCSettingPara);

static void WriteASPara(E2PROM_SETTING PCSettingPara);
static void ReadASPara(E2PROM_SETTING PCSettingPara);

static void WriteRoomCode(E2PROM_SETTING PCSettingPara);
static void ReadRoomCode(E2PROM_SETTING PCSettingPara);


static void ReadFactoryPara(E2PROM_SETTING PCSettingPara);
static void WriteFactoryPara(E2PROM_SETTING PCSettingPara);


static void SendCardInfo(E2PROM_SETTING PCSettingPara);

static void CardInfoDownload(E2PROM_SETTING PCSettingPara);

static void ClearHelpInfo(E2PROM_SETTING PCSettingPara);


static void WriteLiftCtrl(E2PROM_SETTING PCSettingPara);
static void ReadLiftCtrl(E2PROM_SETTING PCSettingPara);

#ifdef __SUPPORT_PROTOCOL_900_1A__
static void Revert2OldVer();
#endif

unsigned short MoxCRC(unsigned char *pData, unsigned short nDataLen);

static	 pthread_t					ParathWork;	

BOOL g_bNeedRestart = FALSE;

void*	ParaWorkThreadFun(void* arg);

CARDULDINFO g_CardUldInfo = 
{
	"",
	0,
	FALSE
};

/*************************************************************************************/
 
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	void ParaSettingInit()
**	AUTHOR:		Jeff Wang
**	DATE:		10 - Sep - 2007
**
**	DESCRIPTION:	
**				Main task of command process
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/
void 
ParaSettingInit()
{
	int						nBlockFlag	= 1;
	struct sockaddr_in		LocalAddr;
	int						nLocalAddrLen	= sizeof (struct sockaddr_in);  
	
	g_bNeedRestart = FALSE;
	
    sock_E2PROM=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    LocalAddr.sin_family = AF_INET;
    LocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    LocalAddr.sin_port = htons(2028);
   
    bind(sock_E2PROM, (struct sockaddr*) &LocalAddr, nLocalAddrLen);
    ioctl(sock_E2PROM, FIONBIO, &nBlockFlag); 

	if ((pthread_create(&ParathWork, NULL, ParaWorkThreadFun, NULL)) != 0)	
	{
		printf("Para: create thread fail\n");
	}

	DpcAddMd(MXMDID_PARASET, NULL);

#ifdef DEBUG_PARA_SETTING
	printf("Para: Initialize success\n");
#endif
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	void ParaSettingProc()
**	AUTHOR:		Jeff Wang
**	DATE:		10 - Sep - 2007
**
**	DESCRIPTION:	
**				Main task of command process
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/
void* 
ParaWorkThreadFun(void* arg)
{			
	//int			     i = 0;     //by [MichaelMa]
    DWORD          Version =0;
    WORD		  nQuireType = 0;
    //unsigned char    type = 0;    //by [MichaelMa]
    //UCHAR           settype = 0;  //by [MichaelMa]
	int	   their_addr_len = sizeof (struct sockaddr_in);
	FILE* fd	=	NULL;	

	MXMSG	msgRecev;
	     
     while(TRUE)
     {
		 memset(&msgRecev, 0, sizeof(msgRecev));
		 msgRecev.dwDestMd	= MXMDID_PARASET;
		 msgRecev.pParam		= NULL;

		 if (MxGetMsg(&msgRecev))
		 {
/*
			 if (COMM_YUV2JPG == msgRecev.dwMsg)
			 {
				 ConvertYUV2Jpg();
				 printf("JPG Convert Success\n");
			 }

			 if (NULL != msgRecev.pParam)
			 {
				free(msgRecev.pParam);
				msgRecev.pParam = NULL;
			 }
*/
		 }
		 
      size_E2PROM = recvfrom(sock_E2PROM, BUF_PCSETTING, sizeof(BUF_PCSETTING),0,(struct sockaddr_in *)&recv_addr_E2PROM, &their_addr_len);	
		//printf("%s,%d,sin_addr = %s\r\n",__func__,__LINE__,inet_ntoa(recv_addr_E2PROM.sin_addr));
	  if( size_E2PROM > 0)
      {
//   		dwDiag[DIAG_ETH_RCV]++;
	  }
			
      if( size_E2PROM != -1)
      {
      	memcpy(&PC_SETTING_PARAMS, BUF_PCSETTING, sizeof(PC_SETTING_PARAMS));
        if(( ETHERNET_PROTOCOL_STX == PC_SETTING_PARAMS.START ) 
			&& ( ETHERNET_PROTOCOL_ETX == PC_SETTING_PARAMS.END ))
        {
			printf("PC_SETTING_PARAMS.CMD = 0x%x\n",PC_SETTING_PARAMS.CMD);
         switch( PC_SETTING_PARAMS.CMD )
        {			 
		 case COMM_WRITE_AS_PARA:
			 {
#ifdef __SUPPORT_PROTOCOL_900_1A__
				Revert2OldVer ();
#endif
				WriteASPara(PC_SETTING_PARAMS);
				break;
			 }
		 case COMM_READ_AS_PARA:
			 {
				 ReadASPara(PC_SETTING_PARAMS);
				 break;
			 }			 

		  case COMM_WRITE_SYS_PARA:
#ifdef __SUPPORT_PROTOCOL_900_1A__
			 Revert2OldVer ();
#endif
		  	 WriteSysPara(PC_SETTING_PARAMS);
		  	 break;

		  case COMM_READ_SYS_PARA:
			  ReadSysPara(PC_SETTING_PARAMS);
			  break;

		  case CNF_WRITE_ROOMCODE:
			  WriteRoomCode(PC_SETTING_PARAMS);
			  break;

		  case CNF_READ_ROOMCODE:
		  	 ReadRoomCode(PC_SETTING_PARAMS);
		  	 break;

		  case CNF_WRITE_LIFTCTRL:
#ifdef __SUPPORT_PROTOCOL_900_1A__
			  Revert2OldVer ();
#endif
			  WriteLiftCtrl(PC_SETTING_PARAMS);
			  break;

		  case CNF_READ_LIFTCTRL:
		  	 ReadLiftCtrl(PC_SETTING_PARAMS);
		  	 break;
            case COMM_WRITE_SYS_PARA_V2:
#ifdef __SUPPORT_PROTOCOL_900_1A__
				Revert2OldVer ();
#endif
                WriteSysPara_V2(PC_SETTING_PARAMS);
                break;
            case COMM_READ_SYS_PARA_V2:
                ReadSysPara_V2(PC_SETTING_PARAMS);
                break;
		  case COMM_READ_FACTORY_PARA:
			  ReadFactoryPara(PC_SETTING_PARAMS); 
			  break; 
		  case COMM_WRITE_FACTORY_PARA:
			  WriteFactoryPara(PC_SETTING_PARAMS); 
			  break; 
		  case COMM_REQ_CARD:
		  	  if (DEVICE_CORAL_DB == g_DevFun.DevType) 
	 	  	  {
	 	  	  	printf("%s,%d\r\n",__func__,__LINE__);
				break;
	 	  	  }
			  else
			  {
			  	SendCardInfo(PC_SETTING_PARAMS);
			  	break;
			  }
		  case COMM_DOWNLOAD_CARD:
		  	  if (DEVICE_CORAL_DB == g_DevFun.DevType) 
	 	  	  {
	 	  	  	printf("%s,%d\r\n",__func__,__LINE__);
				break;
	 	  	  }
			  else
			  {
			  	CardInfoDownload(PC_SETTING_PARAMS);
			 	break;
			  }
			  
		  case CNF_CLEAR_HELPINFO:
			  ClearHelpInfo(PC_SETTING_PARAMS);
			  break;

          case CNF_QUERY_PARAMETER:
               if( (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)) < size_E2PROM)
               {
                 check_sum_E2PROM = MoxCRC(&BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)], (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)-2));

                 if( check_sum_E2PROM == (((short)BUF_PCSETTING[PC_SETTING_PARAMS.DATALEN - 1])<<8)+((short)BUF_PCSETTING[PC_SETTING_PARAMS.DATALEN - 2]))
                 {
                  nQuireType=((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)]) + (((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+1]<<8));
#ifdef DEBUG_PARA_SETTING
				 printf("Enquire Type:%x\n",nQuireType);
#endif	
                 	switch(nQuireType)
                 	{
                 		case CNF_QUERY_AMT_VERSION:
                 			PC_SETTING_PARAMS.CMD      = RE_CNF_QUERY_PARAMETER;
                   			PC_SETTING_PARAMS.DATALEN  = sizeof(PC_SETTING_PARAMS)+6;
                   			PC_SETTING_PARAMS.ERR      = 0x00;
                   			
                   			memcpy(BUF_PCSETTING, &PC_SETTING_PARAMS, sizeof(PC_SETTING_PARAMS));
                   			Version = ATM_VERSION_4;
                   			memcpy(&BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)],&Version,4);
                   			BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+4] = MoxCRC(BUF_PCSETTING,(sizeof(PC_SETTING_PARAMS) + 6));
                   			
							SndPacket(&sock_E2PROM, BUF_PCSETTING, PC_SETTING_PARAMS.DATALEN, &recv_addr_E2PROM);
                 		break;
                 		default:
                 		break;
                 	}
                 }
                 else
                 {
                 	SendACK(PC_SETTING_PARAMS, 0xff, 0);
                 }
               }
               else
               {
                 SendACK(PC_SETTING_PARAMS, 0xff, 0);
               }          	   
          	   break;               
          case CNF_WRITE_TMPERIOD:
               break;
               
          case CNF_READ_TMPERIOD:
               break;
                       
			          case CNF_DWNLD_AMT:
               if( (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)) < size_E2PROM)
               {
                 check_sum_E2PROM = MoxCRC(&BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)], (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)-2));

                 if( check_sum_E2PROM == (((short)BUF_PCSETTING[PC_SETTING_PARAMS.DATALEN - 1])<<8)+((short)BUF_PCSETTING[PC_SETTING_PARAMS.DATALEN - 2]))
                 {
                  FILE_FORMAT=(((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)])<<8) + ((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+1]);
                  switch(FILE_FORMAT)
                  {
                   case FILE_FIRST_PACKET:
                   
                        FILE_PACKET_NUM     = (((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+2])<<8) + ((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+3]);
                        FILE_PACKET_NUM_LST =  FILE_PACKET_NUM;
                        if(FILE_PACKET_NUM == 0) 
                        {
                         FILE_LEN         =  (((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+4])<<24)
                                             +(((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+5])<<16)
                                             +(((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+6])<<8)
                                             +((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+7]);
                         
                         FILE_AMT_VERSION =  (((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+8])<<24)
                                             +(((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+9])<<16)
                                             +(((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+10])<<8)
                                             +((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+11]);
                         memset(DownloadDataBuf, 0, 8);
     					
						 FLAG_AMT_BURN = 1;
                         memcpy(&DownloadDataBuf[0],&FILE_AMT_VERSION,4);
#ifdef DEBUG_PARA_SETTING
						 printf("file len:%d,version:%x\n",FILE_LEN,FILE_AMT_VERSION);
#endif
						 
                         if(FILE_LEN < ATM_FLASH_LEN)
                         {
	                         if(FILE_AMT_VERSION < ATM_VERSION_4)
	                         {
		                          FILE_FORMAT                    = 0;
		                          FILE_LEN                       = 0;
		                          FILE_LEN_BAK                   = 0;
		                          FILE_COPY_REC                  = 0;
		                          FILE_PACKET_NUM                = 0;
		                          FILE_PACKET_NUM_LST            = 0;
		                          FILE_PACKET_LEN                = 0;
		                          SendACK(PC_SETTING_PARAMS, ATM_VERSION_ERROR, 0);
	                         }
	                         else
	                         {
		                         memcpy(&DownloadDataBuf[4],&FILE_LEN,4);
		                         FILE_COPY_REC = 8;
		                         FILE_LEN_BAK  = FILE_LEN + 8;
		                         //FILE_COPY_REC = 0x32;
		                         //FILE_LEN_BAK  = FILE_LEN + 0x32;
#ifdef DEBUG_PARA_SETTING 
								 printf("first pkt right\n");
#endif
								 
		                         SendACK(PC_SETTING_PARAMS, 0x00, 0);  
		                      }
		                   }
		                   else
		                   {
		                   		  FILE_FORMAT                    = 0;
		                          FILE_LEN                       = 0;
		                          FILE_LEN_BAK                   = 0;
		                          FILE_COPY_REC                  = 0;
		                          FILE_PACKET_NUM                = 0;
		                          FILE_PACKET_NUM_LST            = 0;
		                          FILE_PACKET_LEN                = 0;
		                          SendACK(PC_SETTING_PARAMS, ATM_LEN_OVERFLOW, 0);
		                   }
                        }
                        else
                        {
                          FILE_FORMAT                    = 0;
                          FILE_LEN                       = 0;
                          FILE_LEN_BAK                   = 0;
                          FILE_COPY_REC                  = 0;
                          FILE_PACKET_NUM                = 0;
                          FILE_PACKET_NUM_LST            = 0;
                          FILE_PACKET_LEN                = 0;
                          SendACK(PC_SETTING_PARAMS, 0xff, 0);
                        }
                        break;
                        
                   case FILE_MIDDLE_PACKET:
                        FILE_PACKET_NUM =(((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+2])<<8)+((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+3]);
                        if( FILE_PACKET_NUM_LST == (FILE_PACKET_NUM - 1))
                        {
                         FILE_PACKET_LEN     =  (((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+4])<<24)
                                               +(((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+5])<<16)
                                               +(((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+6])<<8)
                                               +((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+7]);
                         FILE_LEN            = FILE_LEN - FILE_PACKET_LEN;
#ifdef DEBUG_PARA_SETTING
                         printf("FILE_LEN = %x,FILE_PACKET_LEN = %x\n",FILE_LEN,FILE_PACKET_LEN);
#endif
						 
                         FILE_PACKET_NUM_LST = FILE_PACKET_NUM;
                         
                         memcpy(&DownloadDataBuf[FILE_COPY_REC],&BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+8],FILE_PACKET_LEN);
                         FILE_COPY_REC = FILE_COPY_REC + FILE_PACKET_LEN;
                       
                         SendACK(PC_SETTING_PARAMS, 0x00, 0);
                         if( FILE_LEN == 0)
                         {
							 WriteData2AMT(DownloadDataBuf, FILE_LEN_BAK);
                          FILE_FORMAT                    = 0;
                          FILE_LEN                       = 0;
                          FILE_COPY_REC                  = 0;
                          FILE_PACKET_NUM                = 0;
                          FILE_PACKET_NUM_LST            = 0;
                          FILE_PACKET_LEN                = 0;
                         
                           usleep(1000);

						   FLAG_AMT_BURN = 0;					   

						   UpdateRdfromAMT();
						   UpdateASTfromAMT();
                         }
                        }
                        else
                        {
                          FILE_FORMAT                    = 0;
                          FILE_LEN                       = 0;
                          FILE_LEN_BAK                   = 0;
                          FILE_COPY_REC                  = 0;
                          FILE_PACKET_NUM                = 0;
                          FILE_PACKET_NUM_LST            = 0;
                          FILE_PACKET_LEN                = 0;
                          SendACK(PC_SETTING_PARAMS, 0xff, 0);
                        }
                        break;
                        
                   default:
                          FILE_FORMAT                    = 0;
                          FILE_LEN                       = 0;
                          FILE_LEN_BAK                   = 0;
                          FILE_COPY_REC                  = 0;
                          FILE_PACKET_NUM                = 0;
                          FILE_PACKET_NUM_LST            = 0;
                          FILE_PACKET_LEN                = 0;
                          SendACK(PC_SETTING_PARAMS, 0xff, 0);
                        break;
                  }
                 }
               } 
               break;
               
          case CNF_UPLD_AMT:
              if( (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)) < size_E2PROM)
               {
				  FILE * fd			= NULL;
				  char szName[255]	= {0};
				  
                 check_sum_E2PROM = MoxCRC(&BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)], (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)-2));

                 if( check_sum_E2PROM == (((short)BUF_PCSETTING[PC_SETTING_PARAMS.DATALEN - 1])<<8)+((short)BUF_PCSETTING[PC_SETTING_PARAMS.DATALEN - 2]))
                 {
                  FILE_FORMAT=(((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)])<<8) + ((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+1]);
                  switch(FILE_FORMAT)
                  {
                   case FILE_FIRST_PACKET:
						FLAG_AMT_BURN = 1;

						strcpy(szName, AMTFILE);
						if ((fd = fopen(szName, "r+")) == NULL)
						{
							break;
						}

						fseek(fd, 0, SEEK_SET);
						fread(&FLASH_AMT_VERSION, 4, (size_t)1, fd);
						
						fseek(fd, 4, SEEK_SET);
						fread(&FLASH_AMT_LEN, 4, (size_t)1, fd);
#ifdef DEBUG_PARA_SETTING
						printf("Flash Len:%d\n",FLASH_AMT_LEN);
#endif
						
                        if( (FLASH_AMT_LEN > 0x00000000) && (FLASH_AMT_LEN < 0x200000) )
                        {
							fseek(fd, 8, SEEK_SET);
							fread(DownloadDataBuf, FLASH_AMT_LEN, (size_t)1, fd);
							fclose(fd);

							 PC_SETTING_PARAMS.CMD      = PC_SETTING_PARAMS.CMD + 0x80;
							 PC_SETTING_PARAMS.ERR      = 0;
							 PC_SETTING_PARAMS.DATALEN  = sizeof(PC_SETTING_PARAMS) + 14;
							 FILE_FORMAT                    = FILE_FIRST_PACKET;
							 FILE_LEN                       = FLASH_AMT_LEN;
							 FILE_LEN_BAK                   = FLASH_AMT_LEN;
							 FILE_COPY_REC                  = 0;
							 FILE_PACKET_NUM                = 0;
							 FILE_PACKET_LEN                = 800;
							 FILE_PACKET_AMOUNT             = FILE_LEN/FILE_PACKET_LEN;
							 if( FILE_LEN > FILE_PACKET_AMOUNT * FILE_PACKET_LEN)
							 {
							  FILE_PACKET_AMOUNT++;
							 }
							 memcpy(BUF_PCSETTING, &PC_SETTING_PARAMS, sizeof(PC_SETTING_PARAMS));
							 memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS), &FILE_FORMAT, 2);
							 memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS) +2, &FILE_PACKET_AMOUNT, 2);
							 memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS) +4, &FILE_LEN, 4);
							 memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS) +8, &FLASH_AMT_VERSION, 4);
							 FlashAMTCRC = MoxCRC(BUF_PCSETTING,(sizeof(PC_SETTING_PARAMS) + 12));
							 memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS)+12,&FlashAMTCRC, 2);

							 SndPacket(&sock_E2PROM, BUF_PCSETTING, sizeof(PC_SETTING_PARAMS)+14, &recv_addr_E2PROM);
						}
                   break;
                 
                 case FILE_MIDDLE_PACKET:
                      if( FILE_PACKET_NUM == (((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+2])<<8) + ((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+3])-1)
                      {
                       if((FILE_LEN > 800) && (FILE_PACKET_NUM < FILE_PACKET_AMOUNT) )
                       {
                        FILE_LEN                   = FILE_LEN - 800;
                        FILE_PACKET_LEN            = 800;
                        FILE_FORMAT                = FILE_MIDDLE_PACKET;
                        FILE_PACKET_NUM            = FILE_PACKET_NUM + 1;
                        PC_SETTING_PARAMS.CMD      = PC_SETTING_PARAMS.CMD + 0x80;
                        PC_SETTING_PARAMS.ERR      = 0;
                        PC_SETTING_PARAMS.DATALEN  = sizeof(PC_SETTING_PARAMS) + FILE_PACKET_LEN + 10;
                        memcpy(&BUF_PCSETTING[0], &PC_SETTING_PARAMS, sizeof(PC_SETTING_PARAMS));
                        memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS), &FILE_FORMAT, 2);
                        memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS) +2, &FILE_PACKET_NUM, 2);
                        memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS) +4, &FILE_PACKET_LEN, 4);
                        memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS) +8, &DownloadDataBuf[FILE_COPY_REC], FILE_PACKET_LEN);
                        FlashAMTCRC = MoxCRC(BUF_PCSETTING,(sizeof(PC_SETTING_PARAMS) + FILE_PACKET_LEN + 8));
                        memcpy(&BUF_PCSETTING[0] + (sizeof(PC_SETTING_PARAMS) + FILE_PACKET_LEN + 8),&FlashAMTCRC, 2);
                        SndPacket(&sock_E2PROM, BUF_PCSETTING, (sizeof(PC_SETTING_PARAMS) + FILE_PACKET_LEN + 10), &recv_addr_E2PROM);
                        
                        FILE_COPY_REC    = FILE_COPY_REC + FILE_PACKET_LEN;
                       }
                       else if( (FILE_LEN > 0) && (FILE_PACKET_NUM == (FILE_PACKET_AMOUNT-1)))
                       {
                        FILE_PACKET_LEN            = FILE_LEN;
                        FILE_LEN                   = 0;
                        FILE_FORMAT                = FILE_MIDDLE_PACKET;
                        FILE_PACKET_NUM            = FILE_PACKET_NUM + 1;
                        PC_SETTING_PARAMS.CMD      = PC_SETTING_PARAMS.CMD + 0x80;
                        PC_SETTING_PARAMS.ERR      = 0;
                        PC_SETTING_PARAMS.DATALEN  = sizeof(PC_SETTING_PARAMS) + FILE_PACKET_LEN + 10;
                        memcpy(&BUF_PCSETTING[0], &PC_SETTING_PARAMS, sizeof(PC_SETTING_PARAMS));
                        memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS), &FILE_FORMAT, 2);
                        memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS) +2, &FILE_PACKET_NUM, 2);
                        memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS) +4, &FILE_PACKET_LEN, 4);
                        memcpy(&BUF_PCSETTING[0] + sizeof(PC_SETTING_PARAMS) +8, &DownloadDataBuf[FILE_COPY_REC], FILE_PACKET_LEN);
                        FlashAMTCRC = MoxCRC(BUF_PCSETTING,(sizeof(PC_SETTING_PARAMS) + FILE_PACKET_LEN + 8));
                        memcpy(&BUF_PCSETTING[0] + (sizeof(PC_SETTING_PARAMS) + FILE_PACKET_LEN + 8),&FlashAMTCRC, 2);
                        SndPacket(&sock_E2PROM, BUF_PCSETTING, (sizeof(PC_SETTING_PARAMS) + FILE_PACKET_LEN + 10), &recv_addr_E2PROM);
						
						FLAG_AMT_BURN = 0;
                         FILE_COPY_REC = FILE_COPY_REC + FILE_PACKET_LEN;
                       }
                       else
                       {
                       } 
                      }  
                   break;
                   
                 default:
                 
                   break;
                   
                }
               }
              }
              break;

		case CNF_DWNLD_HELPINFO://Download Help Information
			if( (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)) < size_E2PROM)
            {
				check_sum_E2PROM = MoxCRC(&BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)], (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)-2));

                if( check_sum_E2PROM == (((short)BUF_PCSETTING[PC_SETTING_PARAMS.DATALEN - 1])<<8)+((short)BUF_PCSETTING[PC_SETTING_PARAMS.DATALEN - 2]))
                {
					FILE_FORMAT=(((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)])<<8) + ((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+1]);
					switch(FILE_FORMAT)
					{
					case FILE_FIRST_PACKET:
                   
						FILE_PACKET_NUM     = (((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+2])<<8) + ((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+3]);
                        FILE_PACKET_NUM_LST =  FILE_PACKET_NUM;
                        if(FILE_PACKET_NUM == 0) 
                        {
							FILE_LEN         =  (((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+4])<<24)
								                 +(((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+5])<<16)
									             +(((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+6])<<8)
										         +((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+7]);                         
#ifdef DEBUG_PARA_SETTING
							printf("Whole file len:%d\n",FILE_LEN);
#endif
							if(FILE_LEN < MAX_PHOTO_JPEG_LEN)
							{
								memcpy(&DownloadDataBuf[0], &FILE_LEN, 4);
								FILE_COPY_REC = 4;
								FILE_LEN_BAK  = FILE_LEN;
		                        SendACK(PC_SETTING_PARAMS, 0x00, 0);  
							}
							else
							{
								FILE_FORMAT                    = 0;
		                        FILE_LEN                       = 0;
		                        FILE_LEN_BAK                   = 0;
		                        FILE_COPY_REC                  = 0;
		                        FILE_PACKET_NUM                = 0;
		                        FILE_PACKET_NUM_LST            = 0;
		                        FILE_PACKET_LEN                = 0;
		                        SendACK(PC_SETTING_PARAMS, 0x01, 0);
							}
                        }
                        else
                        {
							FILE_FORMAT                    = 0;
							FILE_LEN                       = 0;
							FILE_LEN_BAK                   = 0;
							FILE_COPY_REC                  = 0;
							FILE_PACKET_NUM                = 0;
							FILE_PACKET_NUM_LST            = 0;
							FILE_PACKET_LEN                = 0;
							SendACK(PC_SETTING_PARAMS, 0x01, 0);
                        }
                        break;
                        
                   case FILE_MIDDLE_PACKET:
                        FILE_PACKET_NUM =(((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+2])<<8)+((WORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+3]);
                        if( FILE_PACKET_NUM_LST == (FILE_PACKET_NUM - 1))
                        {
							FILE_PACKET_LEN     =  (((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+4])<<24)
								                   +(((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+5])<<16)
									               +(((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+6])<<8)
										           +((DWORD)BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+7]);
							FILE_LEN            = FILE_LEN - FILE_PACKET_LEN;							

							FILE_PACKET_NUM_LST = FILE_PACKET_NUM;
                         
							memcpy(&DownloadDataBuf[FILE_COPY_REC],&BUF_PCSETTING[sizeof(PC_SETTING_PARAMS)+8],FILE_PACKET_LEN);
							FILE_COPY_REC = FILE_COPY_REC + FILE_PACKET_LEN;
                       
							SendACK(PC_SETTING_PARAMS, 0x00, 0);

							if( FILE_LEN == 0)
							{
								fd = fopen("/mox/rdwr/HelpInfo.bmp", "w+");
								if (NULL != fd) 
								{
									fwrite(DownloadDataBuf + 4,FILE_LEN_BAK, 1, fd);
								}
								fclose(fd);

								g_SysConfig.bHelpInfoDld	   = TRUE;

								FILE_FORMAT                    = 0;
								FILE_LEN                       = 0;
								FILE_COPY_REC                  = 0;
								FILE_PACKET_NUM                = 0;
								FILE_PACKET_NUM_LST            = 0;
								FILE_PACKET_LEN                = 0;
                         
								usleep(1000);                           

								SaveMenuPara2Mem();
							}							
                        }
                        else
                        {
							FILE_FORMAT                    = 0;
							FILE_LEN                       = 0;
							FILE_LEN_BAK                   = 0;
							FILE_COPY_REC                  = 0;
							FILE_PACKET_NUM                = 0;
							FILE_PACKET_NUM_LST            = 0;
							FILE_PACKET_LEN                = 0;
							SendACK(PC_SETTING_PARAMS, 0x01, 0);
                        }
                        break;
                        
                   default:
					{
						FILE_FORMAT                    = 0;
						FILE_LEN                       = 0;
						FILE_LEN_BAK                   = 0;
						FILE_COPY_REC                  = 0;
						FILE_PACKET_NUM                = 0;
						FILE_PACKET_NUM_LST            = 0;
						FILE_PACKET_LEN                = 0;
						SendACK(PC_SETTING_PARAMS, 0xff, 0);
					  }
                       break;
					}
                 }
               } 
               break;
                       
          default:
                          FILE_FORMAT                    = 0;
                          FILE_LEN                       = 0;
                          FILE_LEN_BAK                   = 0;
                          FILE_COPY_REC                  = 0;
                          FILE_PACKET_NUM                = 0;
                          FILE_PACKET_NUM_LST            = 0;
                          FILE_PACKET_LEN                = 0;
                          SendACK(PC_SETTING_PARAMS, 0xff, 0);
                 break;     
         }
        }
        
        else
        {
                FILE_FORMAT                    = 0;
                FILE_LEN                       = 0;
                FILE_LEN_BAK                   = 0;
                FILE_COPY_REC                  = 0;
                FILE_PACKET_NUM                = 0;
                FILE_PACKET_NUM_LST            = 0;
                FILE_PACKET_LEN                = 0;
                SendACK(PC_SETTING_PARAMS, 0xff, 0);
        }
      }
      else 
      {
       //Linkflag = PROGRAMDOWNLOADOFF;
      }
	usleep(1 * 1000);
	}	
	pthread_exit(0);
}

static VOID 
SendACK(E2PROM_SETTING PC_SETTING_PARAMS_PARA,UCHAR ERROR_ID,DWORD RESERVED_VALUE)
{
//	DWORD Send_Counter = 0;
	
	PC_SETTING_PARAMS_PARA.CMD			= PC_SETTING_PARAMS_PARA.CMD + 0x80;
    PC_SETTING_PARAMS_PARA.DATALEN		= sizeof(PC_SETTING_PARAMS_PARA);
    PC_SETTING_PARAMS_PARA.ERR          = ERROR_ID;
    PC_SETTING_PARAMS_PARA.RESERVED     = RESERVED_VALUE;
    memcpy(BUF_PCSETTING, &PC_SETTING_PARAMS_PARA, sizeof(PC_SETTING_PARAMS_PARA));

	SndPacket(&sock_E2PROM, BUF_PCSETTING, sizeof(PC_SETTING_PARAMS_PARA), &recv_addr_E2PROM);
}

static void 
WriteSysPara(E2PROM_SETTING PCSettingPara)
{
	DWORD   AddrForNet =  0;
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, sizeof(E2PROM_SETTING));
	check_sum_E2PROM = 0;

	g_bNeedRestart = TRUE;
	usleep(10*1000);
	
#ifdef PARA_SETTING_DEBUG
		int offset=0;
		int i=0;
		int len=PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING);
		BYTE *pData=(BYTE *)&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2];
		printf("WriteSysPara start...len:%d,size_E2PROM:%d,TalkVolume:%d,UnlockTime:%d\n",len,size_E2PROM,g_SysConfig.TalkVolume,g_SysConfig.UnlockTime);
		for(i=0;i<len;i++)
		{
			printf("%.2x  ",*(pData+i));
			offset++;
			if(offset==16)
			{
				offset=0;
				printf("\n");
			}
		}
		printf("\n");
#endif


	if( (PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING)) < size_E2PROM)
	{
		//memcpy(&g_SysConfig.LangSel,		&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_LANGUAGESET_OFFSET], SYS_LANGUAGESET_LEN);
		memcpy(&g_SysConfig.IPAddr,			&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_IPADDR_OFFSET], SYS_IPADDR_LEN);
        memcpy(&g_SysConfig.RingNum,		&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_RINGSEL_OFFSET], SYS_RINGSEL_LEN);
		
		memcpy(&g_SysConfig.SysPwd,			&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_SYSPWD_OFFSET], SYS_SYSPWD_LEN);
		memcpy(g_SysConfig.SysPwd, g_SysConfig.SysPwd, PU_MNGPWD_LEN);//系统密码和管理远密码一致
		
		memcpy(&g_SysConfig.VldCodeDigits,  &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_RESICODEVLD_OFFSET], SYS_RESICODEVLD_LEN);
		memcpy(&g_SysConfig.EnTamperAlarm,	&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_TAMPERALARM_OFFSET], SYS_TAMPERALARM_LEN);
		
		memcpy(&g_SysConfig.ActiveCamera,	&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_CAMERASEL_OFFSET],	SYS_CAMERASEL_LEN);
		memcpy(&g_SysConfig.DI1,			&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDI1_OFFSET],		SYS_FUNDI1_LEN);
		memcpy(&g_SysConfig.DI2,			&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDI2_OFFSET],		SYS_FUNDI2_LEN);
		memcpy(&g_SysConfig.DO1,			&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDO1_OFFSET],		SYS_FUNDO1_LEN);
		memcpy(&g_SysConfig.DO2,			&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDO2_OFFSET],		SYS_FUNDO2_LEN);
		memcpy(&g_SysConfig.TalkTime,			&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNTALKTIME_OFFSET],		SYS_FUNTALKTIME_LEN);
		memcpy(&g_SysConfig.RingTime,			&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNRINGTIME_OFFSET],		SYS_FUNRINGTIME_LEN);
		

		memcpy(&g_SysConfig.DO1_Pulse_Time,	&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDO1_PULSETIME_OFFSET],	SYS_FUNDO1_PULSETIME_LEN);
		memcpy(&g_SysConfig.DO2_Pulse_Time,	&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDO2_PULSETIME_OFFSET],	SYS_FUNDO2_PULSETIME_LEN);
        /*Only Shell and mini Shell serial GM can be set talking volume now [MichaelMa] at 2013-3-29*/
        if (SERIES_SHELL == g_DevFun.SeriesType || SERIES_MINI_SHELL == g_DevFun.SeriesType) 
		{
			memcpy(&g_SysConfig.TalkVolume,&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_TALK_VOLUME_OFFSET],		SYS_TALK_VOLUME_LEN);
			memcpy(&g_SysConfig.UnlockTime,&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_UNLOCK_TIME_OFFSET],		SYS_UNLOCK_TIME_LEN);
        }		

		memcpy(&AddrForNet,			&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_MCIP_SERVER_OFFSET], SYS_MCIP_SERVER_LEN);
		g_SysConfig.MCIPSERVER = ntohl(AddrForNet);
		memcpy(&AddrForNet,			&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_MCIP_CLIENT_OFFSET], SYS_MCIP_CLIENT_LEN);
		g_SysConfig.MCIPCLIENT = ntohl(AddrForNet);

		printf("------g_SysConfig.MCIPSERVER = %x; g_SysConfig.MCIPCLIENT = %x\n",g_SysConfig.MCIPSERVER,g_SysConfig.MCIPCLIENT);
		
		SendACK(PCSettingParaTemp, 0x00, 0);
		
		SaveMenuPara2Mem();
		
		usleep(20*1000);
		
		system("reboot");	
		while (1);
	}
	else
	{
		SendACK(PCSettingParaTemp, 0xff, 0);
	}
}

static void 
WriteRoomCode(E2PROM_SETTING PCSettingPara)
{
	int index = 0;
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, sizeof(E2PROM_SETTING));
	check_sum_E2PROM = 0;
	if( (PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING)) < size_E2PROM)
	{
		for(index = 0; index < 13; index++)
		{
			memcpy(g_SysConfig.CallCode[index],		&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+ 19*index], 19);
			printf("g_SysConfig.CallCode[%d] = %s\n",index,g_SysConfig.CallCode[index]);
		}
		
		SendACK(PCSettingParaTemp, 0x00, 0);
		
		SaveMenuPara2Mem();
		
		usleep(20*1000);

		system("reboot");	
		while (1);
	}
	else
	{
		SendACK(PCSettingParaTemp, 0xff, 0);
	}
}


static void 
WriteLiftCtrl(E2PROM_SETTING PCSettingPara)
{
	//int index = 0; //by [MichaelMa]
	DWORD dwIP = 0;
	BYTE bTemp = 0;
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, sizeof(E2PROM_SETTING));
	check_sum_E2PROM = 0;
	if( (PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING)) < size_E2PROM)
	{

		memcpy(&bTemp,	&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LC_ENABLE_OFFSET],	LC_ENABLE_LEN);
		g_SysConfig.LCEnable = !bTemp;
		memcpy(&g_SysConfig.LCMode,		&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LC_MODE_OFFSET],	LC_MODE_LEN);
		memcpy(&g_SysConfig.nLCFuncs,	&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LC_FUNCTION_OFFSET], LC_FUNCTION_LEN);
		
		memcpy(&dwIP,	&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LC_LCAIP_OFFSET],	LC_LCAIP_LEN);
		g_SysConfig.LCAIP = ChangeIPFormat(dwIP);

		memcpy(&bTemp,	&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LCA_ENABLE_OFFSET],		LCA_EBABLE_LEN);
		g_SysConfig.bLCAgent = !bTemp;
		memcpy(&g_SysConfig.LC_ComMode,	&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LCA_COMM_MODE_OFFSET],		LCA_COMM_MODE_LEN);
		
		SendACK(PCSettingParaTemp, 0x00, 0);
		
		SaveMenuPara2Mem();
		
		usleep(20*1000);

		system("reboot");	
		while (1);
	}
	else
	{
		SendACK(PCSettingParaTemp, 0xff, 0);
	}
}


static void 
ReadSysPara(E2PROM_SETTING PCSettingPara)
{
	WORD   ParaCRC        = 0;
	WORD   VldDataLen     = 0;
	DWORD   AddrForNet =  0;
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, sizeof(E2PROM_SETTING));
	check_sum_E2PROM = 0;
#ifdef PARA_SETTING_DEBUG
		int offset=0;
		int i=0;
		int len=PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING);
		BYTE *pData=(BYTE *)&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2];
		printf("ReadSysPara start...len:%d,TalkVolume:%d,UnlockTime:%d\n",len,g_SysConfig.TalkVolume,g_SysConfig.UnlockTime);
		for(i=0;i<len;i++)
		{
			printf("%.2x  ",*(pData+i));
			offset++;
			if(offset==16)
			{
				offset=0;
				printf("\n");
			}
		}
		printf("\n");
#endif
	if( (PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING)) < size_E2PROM)
	{
		//Head
		PCSettingParaTemp.CMD			= PCSettingParaTemp.CMD + 0x80;
		PCSettingParaTemp.DATALEN		= sizeof(E2PROM_SETTING) + 2 + SYS_DATA_LEN + 2;
		PCSettingParaTemp.ERR          = 0x00;
		memcpy(BUF_PCSETTING, &PCSettingParaTemp, sizeof(E2PROM_SETTING));
		//Data
		VldDataLen = SYS_DATA_LEN;
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)], &VldDataLen,	2);		
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_LANGUAGESET_OFFSET], &g_SysConfig.LangSel,	SYS_LANGUAGESET_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_IPADDR_OFFSET],		&g_SysConfig.IPAddr,	SYS_IPADDR_LEN);

		AddrForNet = htonl(g_SysConfig.MCIPSERVER);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_MCIP_SERVER_OFFSET],		&AddrForNet,	SYS_MCIP_SERVER_LEN);
		AddrForNet = htonl(g_SysConfig.MCIPCLIENT);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_MCIP_CLIENT_OFFSET],		&AddrForNet,	SYS_MCIP_CLIENT_LEN);		
		
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_RINGSEL_OFFSET],		&g_SysConfig.RingNum,	SYS_RINGSEL_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_SYSPWD_OFFSET],		&g_SysConfig.SysPwd,	SYS_SYSPWD_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_RESICODEVLD_OFFSET], &g_SysConfig.VldCodeDigits, SYS_RESICODEVLD_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_TAMPERALARM_OFFSET], &g_SysConfig.EnTamperAlarm, SYS_TAMPERALARM_LEN);

		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_CAMERASEL_OFFSET],	&g_SysConfig.ActiveCamera,	SYS_CAMERASEL_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDI1_OFFSET],		&g_SysConfig.DI1,			SYS_FUNDI1_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDI2_OFFSET],		&g_SysConfig.DI2,			SYS_FUNDI2_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDO1_OFFSET],		&g_SysConfig.DO1,			SYS_FUNDO1_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDO2_OFFSET],		&g_SysConfig.DO2,			SYS_FUNDO2_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNTALKTIME_OFFSET],		&g_SysConfig.TalkTime,			SYS_FUNTALKTIME_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNRINGTIME_OFFSET],		&g_SysConfig.RingTime,			SYS_FUNRINGTIME_LEN);


		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDO1_PULSETIME_OFFSET],	&g_SysConfig.DO1_Pulse_Time,	SYS_FUNDO1_PULSETIME_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_FUNDO2_PULSETIME_OFFSET],	&g_SysConfig.DO2_Pulse_Time,	SYS_FUNDO2_PULSETIME_LEN);
		if (SERIES_SHELL == g_DevFun.SeriesType || SERIES_MINI_SHELL == g_DevFun.SeriesType) 
		{
			memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_TALK_VOLUME_OFFSET],	&g_SysConfig.TalkVolume,	SYS_TALK_VOLUME_LEN);
			memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+SYS_UNLOCK_TIME_OFFSET],	&g_SysConfig.UnlockTime,	SYS_UNLOCK_TIME_LEN);
		}		
        ParaCRC = MoxCRC(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2], SYS_DATA_LEN);
	    memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING) +2 + SYS_DATA_LEN], &ParaCRC, 2);

		//Send data	
	    SndPacket(&sock_E2PROM, BUF_PCSETTING, PCSettingParaTemp.DATALEN, &recv_addr_E2PROM);
	
	}	
}
static void 
ReadLiftCtrl(E2PROM_SETTING PCSettingPara)
{
	WORD   ParaCRC        = 0;
	WORD   VldDataLen     = 0;
	DWORD dwIP = 0;
	BYTE	bTemp = 0;
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, sizeof(E2PROM_SETTING));
	check_sum_E2PROM = 0;
	if( (PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING)) < size_E2PROM)
	{
		//Head
		PCSettingParaTemp.CMD			= PCSettingParaTemp.CMD + 0x80;
		PCSettingParaTemp.DATALEN		= sizeof(E2PROM_SETTING) + 2 + LC_DATA_LEN + 2;
		PCSettingParaTemp.ERR          = 0x00;
		memcpy(BUF_PCSETTING, &PCSettingParaTemp, sizeof(E2PROM_SETTING));
		//Data
		VldDataLen = LC_DATA_LEN;
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)], &VldDataLen,	2);		
		bTemp = !g_SysConfig.LCEnable;
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LC_ENABLE_OFFSET], &bTemp,	LC_ENABLE_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LC_MODE_OFFSET],		&g_SysConfig.LCMode,	LC_MODE_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LC_FUNCTION_OFFSET],		&g_SysConfig.nLCFuncs,	LC_FUNCTION_LEN);

		dwIP = ChangeIPFormat(g_SysConfig.LCAIP);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LC_LCAIP_OFFSET],		&dwIP,	LC_LCAIP_LEN);

		bTemp = !g_SysConfig.bLCAgent;		
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LCA_ENABLE_OFFSET], &bTemp, LCA_EBABLE_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+LCA_COMM_MODE_OFFSET], &g_SysConfig.LC_ComMode,	LCA_COMM_MODE_LEN);

     	ParaCRC = MoxCRC(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2], LC_DATA_LEN);
	    	memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING) +2 + LC_DATA_LEN], &ParaCRC, 2);

		//Send data	
	    SndPacket(&sock_E2PROM, BUF_PCSETTING, PCSettingParaTemp.DATALEN, &recv_addr_E2PROM);
	}	
}



static void 
ReadRoomCode(E2PROM_SETTING PCSettingPara)
{
	int    Index		  = 0;
	WORD   ParaCRC        = 0;
	WORD   VldDataLen     = 0;
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, sizeof(E2PROM_SETTING));
	check_sum_E2PROM = 0;
	if( (PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING)) < size_E2PROM)
	{
		//Head
		PCSettingParaTemp.CMD			= PCSettingParaTemp.CMD + 0x80;
		PCSettingParaTemp.DATALEN		= sizeof(E2PROM_SETTING) + 2 + ROOM_CODE_DATA_LEN + 2;
		PCSettingParaTemp.ERR          = 0x00;
		memcpy(BUF_PCSETTING, &PCSettingParaTemp, sizeof(E2PROM_SETTING));
		//Data
		VldDataLen = ROOM_CODE_DATA_LEN;
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)], &VldDataLen,	2);	
		
		for(Index = 0; Index < 13; Index ++)
		{
			memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+19*Index], g_SysConfig.CallCode[Index],	19);
		}
		
        ParaCRC = MoxCRC(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2], ROOM_CODE_DATA_LEN);
	    memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING) +2 + ROOM_CODE_DATA_LEN], &ParaCRC, 2);

		//Send data	
	    SndPacket(&sock_E2PROM, BUF_PCSETTING, PCSettingParaTemp.DATALEN, &recv_addr_E2PROM);
	}	
}




static void 
WriteASPara(E2PROM_SETTING PCSettingPara)
{
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, sizeof(E2PROM_SETTING));
	check_sum_E2PROM = 0;
	
	if( (PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING)) < size_E2PROM)
	{
		//Password Unlock
		if (BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+PU_FUNVLD_OFFSET]) 
		{
			g_PUPara.bFunEnabled = TRUE;
		}
		else
		{
			g_PUPara.bFunEnabled = FALSE;
		}

		if (memcmp((void*)g_PUPara.ResiPwdDefault, (void*)&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+PU_RESIPWD_OFFSET],1 + strlen(( char *)&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+PU_RESIPWD_OFFSET]))) 
		{
			memcpy(g_PUPara.ResiPwdDefault, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+PU_RESIPWD_OFFSET], 1 + strlen(( char *)&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+PU_RESIPWD_OFFSET]));
			UpdateRdDefaultPwd();
		}	

		//Access System
		memcpy(&g_ASPara.VagCodeSel, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_WGCODE_OFFSET], AS_WGCODE_LEN);
		memcpy(&g_ASPara.GateOpenOverTime, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GATEOPENDLYT_OFFSET], AS_GATEOPENDLYT_LEN);
		
		if (BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GOOTA_OFFSET]) 
		{
			g_ASPara.GateOpenAlmFlag = TRUE;
		}
		else
		{
			g_ASPara.GateOpenAlmFlag = FALSE;
		}
		memcpy(&g_ASPara.InvldCardSwipeRptSet, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_IVLDCARDT_OFFSET], AS_IVLDCARDT_LEN);

		if (BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_IVLDCARDA_OFFSET]) 
		{
			g_ASPara.InvldCardSwipeAlmFlag = TRUE;
		}
		else
		{
			g_ASPara.InvldCardSwipeAlmFlag = FALSE;
		}

		if (BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_DOORMANOPEN_OFFSET]) 
		{
			g_ASPara.GateOpenManFlag = TRUE;
		}
		else
		{
			g_ASPara.GateOpenManFlag = FALSE;
		}
		
		g_ASPara.ASOpenMode = BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GATEOPENMODE_OFFSET];

		memcpy(&g_ASPara.RelayPulseWith, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GATERELAYPULSE_OFFSET], AS_GATERELAYPULSE_LEN);

		memcpy(&g_ASPara.MCUnlockEnable, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_MCUNLOCKENABLE_OFFSET], AS_MCUNLOCKENABLE_LEN);

		memcpy(&g_ASPara.GateOpenContactor, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GATECONTACTOR_OFFSET], AS_GATECONTACTOR_LEN);
		g_ASPara.GateOpenContactor = !g_ASPara.GateOpenContactor;

		if (BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ENABLEAS_OFFSET]) 
		{
			g_ASPara.bEnableAS = TRUE;
		}
		else
		{
			g_ASPara.bEnableAS = FALSE;
		}
		memcpy(&g_ASPara.dwAccServerIP, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ACCSERVERIP_OFFSET], AS_ACCSERVERIP_LEN);

		memcpy(g_ASPara.szACCDevCode, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ACCDEVCODE_OFFSET], AS_ACCDEVCODE_LEN);
	
#ifdef __SUPPORT_WIEGAND_CARD_READER__
		memcpy(&g_ASPara.WiegandBitNum, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ACCWIEGANDBITNUM_OFFSET], AS_ACCWIEGANDBITNUM_LEN);
		memcpy(&g_ASPara.WiegandReverse, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ACCWIEGANDREVERSE_OFFSET], AS_ACCWIEGANDREVERSE_LEN);
#endif

#ifdef FORCE_ULK_FALG        
        g_ASPara.EnableForceUlkReport = *(BUF_PCSETTING + sizeof(E2PROM_SETTING) + 2 + AS_FORCEULKREPORT_OFFSET);/*whatever type of GM,this field exists*/
        printf("<WriteASPara>EnableForceUlkReport : %u\n",g_ASPara.EnableForceUlkReport);
#endif //FORCE_ULK_FALG

		SendACK(PCSettingParaTemp, 0x00, 0);
		
		SaveMenuPara2Mem();

//		usleep(20*1000);
		
		system("reboot");		
		while (1);
	}
	else
	{
		SendACK(PCSettingParaTemp, 0xff, 0);
	}
}

static void 
ReadASPara(E2PROM_SETTING PCSettingPara)
{
	WORD   ParaCRC        = 0;
	WORD   VldDataLen     = 0;
	BOOL   bValue = 0;
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, sizeof(E2PROM_SETTING));
	check_sum_E2PROM = 0;

	if( (PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING)) < size_E2PROM)
	{
		//Head
		PCSettingParaTemp.CMD			= PCSettingParaTemp.CMD + 0x80;
		PCSettingParaTemp.DATALEN		= sizeof(E2PROM_SETTING) + 2 + AS_DATA_LEN + 2 + 1/*data length of forcing ulk report*/;
		PCSettingParaTemp.ERR          = 0x00;
		memcpy(BUF_PCSETTING, &PCSettingParaTemp, sizeof(E2PROM_SETTING));
		//Data
		VldDataLen = AS_DATA_LEN + 1/*data length of forcing ulk report*/;
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)], &VldDataLen,	2);		
		//Password Unlock
		if (g_PUPara.bFunEnabled)
		{
			BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+PU_FUNVLD_OFFSET] = 1;
		}
		else
		{
			BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+PU_FUNVLD_OFFSET] = 0;
		}

		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+PU_MNGPWD_OFFSET], g_SysConfig.SysPwd, PU_MNGPWD_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+PU_RESIPWD_OFFSET], g_PUPara.ResiPwdDefault, PU_RESIPWD_LEN);
		
		//Access System
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_WGCODE_OFFSET],&g_ASPara.VagCodeSel, AS_WGCODE_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GATEOPENDLYT_OFFSET], &g_ASPara.GateOpenOverTime, AS_GATEOPENDLYT_LEN);
		if (g_ASPara.GateOpenAlmFlag)
		{
			BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GOOTA_OFFSET] = 1;
		}
		else
		{
			BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GOOTA_OFFSET] = 0;
		}
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_IVLDCARDT_OFFSET], &g_ASPara.InvldCardSwipeRptSet, AS_IVLDCARDT_LEN);
		if (g_ASPara.InvldCardSwipeAlmFlag)
		{
			BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_IVLDCARDA_OFFSET] = 1;
		}
		else
		{
			BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_IVLDCARDA_OFFSET] = 0;
		}

		if (g_ASPara.GateOpenManFlag)
		{
			BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_DOORMANOPEN_OFFSET] = 1;
		}
		else
		{
			BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_DOORMANOPEN_OFFSET] = 0;
		}

		BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GATEOPENMODE_OFFSET] = g_ASPara.ASOpenMode;
	
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GATERELAYPULSE_OFFSET], &g_ASPara.RelayPulseWith, AS_GATERELAYPULSE_LEN);

		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_MCUNLOCKENABLE_OFFSET], &g_ASPara.MCUnlockEnable, AS_MCUNLOCKENABLE_LEN);

		bValue = !g_ASPara.GateOpenContactor;
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_GATECONTACTOR_OFFSET], &bValue, AS_GATECONTACTOR_LEN);

		if (g_ASPara.bEnableAS)
		{
			BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ENABLEAS_OFFSET] = 1;
		}
		else
		{
			BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ENABLEAS_OFFSET] = 0;
		}

		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ACCSERVERIP_OFFSET], &g_ASPara.dwAccServerIP,	AS_ACCSERVERIP_LEN);
 
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ACCDEVCODE_OFFSET], g_ASPara.szACCDevCode,AS_ACCDEVCODE_LEN);
 
#ifdef __SUPPORT_WIEGAND_CARD_READER__
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ACCWIEGANDBITNUM_OFFSET], &g_ASPara.WiegandBitNum,AS_ACCWIEGANDBITNUM_LEN);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2+AS_ACCWIEGANDREVERSE_OFFSET], &g_ASPara.WiegandReverse,AS_ACCWIEGANDREVERSE_LEN);
#endif

#ifdef FORCE_ULK_FALG 
        *(BUF_PCSETTING + sizeof(E2PROM_SETTING) + 2 + AS_FORCEULKREPORT_OFFSET) = g_ASPara.EnableForceUlkReport;/*whatever type of GM,this field exists*/
        printf("<ReadASPara>EnableForceUlkReport : %u\n",g_ASPara.EnableForceUlkReport);
#endif //FORCE_ULK_FALG
    
		ParaCRC = MoxCRC(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2], AS_DATA_LEN + 1/*data length of forcing ulk report*/);
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING) +2 + AS_DATA_LEN + 1/*data length of forcing ulk report*/], &ParaCRC, 2);

		printf("%s,%d,,%X,%d,%d\r\n",__func__,__LINE__,recv_addr_E2PROM.sin_addr.s_addr,recv_addr_E2PROM.sin_family,recv_addr_E2PROM.sin_port);
		//Send data	
		SndPacket(&sock_E2PROM, BUF_PCSETTING, PCSettingParaTemp.DATALEN, &recv_addr_E2PROM);
	}
	
}

/**********************************************************************************************/
static void WriteSysPara_V2(E2PROM_SETTING PCSettingPara)
{
    size_t uTypeSize = sizeof(E2PROM_SETTING);
    DWORD   AddrForNet =  0;
    E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, uTypeSize);
	check_sum_E2PROM = 0;
	
	if( (PCSettingParaTemp.DATALEN - uTypeSize) < size_E2PROM)
	{
        /*Offset of value of total data length added*/
        uTypeSize += 2;
        //memcpy(&g_SysConfig.LangSel,        BUF_PCSETTING + uTypeSize + SYS_LANGUAGESET_OFFSET, SYS_LANGUAGESET_LEN);
		memcpy(&g_SysConfig.IPAddr,         BUF_PCSETTING + uTypeSize + SYS_IPADDR_OFFSET,      SYS_IPADDR_LEN);
        memcpy(&g_SysConfig.RingNum,        BUF_PCSETTING + uTypeSize + SYS_RINGSEL_OFFSET,     SYS_RINGSEL_LEN);
		memcpy(&g_SysConfig.SysPwd,         BUF_PCSETTING + uTypeSize + SYS_SYSPWD_OFFSET,      SYS_SYSPWD_LEN);
		//memcpy(g_SysConfig.SysPwd,          g_SysConfig.SysPwd,         PU_MNGPWD_LEN);//系统密码和管理远密码一致
		memcpy(&g_SysConfig.VldCodeDigits,  BUF_PCSETTING + uTypeSize + SYS_RESICODEVLD_OFFSET, SYS_RESICODEVLD_LEN);
		memcpy(&g_SysConfig.EnTamperAlarm,  BUF_PCSETTING + uTypeSize + SYS_TAMPERALARM_OFFSET, SYS_TAMPERALARM_LEN);
		memcpy(&g_SysConfig.ActiveCamera,   BUF_PCSETTING + uTypeSize + SYS_CAMERASEL_OFFSET,	SYS_CAMERASEL_LEN);
		memcpy(&g_SysConfig.DI1,	        BUF_PCSETTING + uTypeSize + SYS_FUNDI1_OFFSET,		SYS_FUNDI1_LEN);
		memcpy(&g_SysConfig.DI2,	        BUF_PCSETTING + uTypeSize + SYS_FUNDI2_OFFSET,		SYS_FUNDI2_LEN);
		memcpy(&g_SysConfig.DO1,	        BUF_PCSETTING + uTypeSize + SYS_FUNDO1_OFFSET,		SYS_FUNDO1_LEN);
		memcpy(&g_SysConfig.DO2,	        BUF_PCSETTING + uTypeSize + SYS_FUNDO2_OFFSET,		SYS_FUNDO2_LEN);
		memcpy(&g_SysConfig.TalkTime,       BUF_PCSETTING + uTypeSize + SYS_FUNTALKTIME_OFFSET,	SYS_FUNTALKTIME_LEN);
		memcpy(&g_SysConfig.RingTime,       BUF_PCSETTING + uTypeSize + SYS_FUNRINGTIME_OFFSET,	SYS_FUNRINGTIME_LEN);
		memcpy(&g_SysConfig.DO1_Pulse_Time, BUF_PCSETTING + uTypeSize + SYS_FUNDO1_PULSETIME_OFFSET,SYS_FUNDO1_PULSETIME_LEN);
		memcpy(&g_SysConfig.DO2_Pulse_Time, BUF_PCSETTING + uTypeSize + SYS_FUNDO2_PULSETIME_OFFSET,SYS_FUNDO2_PULSETIME_LEN);
        /*Only Shell and mini Shell serial GM can be set talking volume now [MichaelMa] at 2013-3-29*/
        if (SERIES_SHELL == g_DevFun.SeriesType || SERIES_MINI_SHELL == g_DevFun.SeriesType) 
		{
			memcpy(&g_SysConfig.TalkVolume, BUF_PCSETTING + uTypeSize + SYS_TALK_VOLUME_OFFSET,		SYS_TALK_VOLUME_LEN);
			memcpy(&g_SysConfig.UnlockTime, BUF_PCSETTING + uTypeSize + SYS_UNLOCK_TIME_OFFSET,		SYS_UNLOCK_TIME_LEN);
        }
		memcpy(&AddrForNet,                 BUF_PCSETTING + uTypeSize + SYS_MCIP_SERVER_OFFSET, SYS_MCIP_SERVER_LEN);
		g_SysConfig.MCIPSERVER = ntohl(AddrForNet);
		memcpy(&AddrForNet,                 BUF_PCSETTING + uTypeSize + SYS_MCIP_CLIENT_OFFSET, SYS_MCIP_CLIENT_LEN);
		g_SysConfig.MCIPCLIENT = ntohl(AddrForNet);
        /*Offset of value of system data*/
        uTypeSize += SYS_DATA_LEN;
        g_PUPara.bFunEnabled = !!(*(        BUF_PCSETTING + uTypeSize + PU_FUNVLD_OFFSET)) ? TRUE : FALSE;
		if (memcmp((void*)g_PUPara.ResiPwdDefault,(void*)(BUF_PCSETTING + uTypeSize + PU_RESIPWD_OFFSET),1 + strlen((char*)&BUF_PCSETTING + uTypeSize + PU_RESIPWD_OFFSET))) 
		{
			memcpy(g_PUPara.ResiPwdDefault, BUF_PCSETTING + uTypeSize + PU_RESIPWD_OFFSET, 1 + strlen((char*)(BUF_PCSETTING + uTypeSize + PU_RESIPWD_OFFSET)));
			UpdateRdDefaultPwd();
		}	
		memcpy(&g_ASPara.VagCodeSel,        BUF_PCSETTING + uTypeSize + AS_WGCODE_OFFSET,       AS_WGCODE_LEN);
		memcpy(&g_ASPara.GateOpenOverTime,  BUF_PCSETTING + uTypeSize + AS_GATEOPENDLYT_OFFSET, AS_GATEOPENDLYT_LEN);
        g_ASPara.GateOpenAlmFlag = !!(*(    BUF_PCSETTING + uTypeSize + AS_GOOTA_OFFSET)) ? TRUE : FALSE;
		memcpy(&g_ASPara.InvldCardSwipeRptSet,BUF_PCSETTING + uTypeSize + AS_IVLDCARDT_OFFSET,  AS_IVLDCARDT_LEN);
        g_ASPara.InvldCardSwipeAlmFlag = !!(*(BUF_PCSETTING + uTypeSize + AS_IVLDCARDA_OFFSET)) ? TRUE : FALSE;
        g_ASPara.GateOpenManFlag = !!(*(    BUF_PCSETTING + uTypeSize + AS_DOORMANOPEN_OFFSET)) ? TRUE : FALSE;
		g_ASPara.ASOpenMode = *(            BUF_PCSETTING + uTypeSize + AS_GATEOPENMODE_OFFSET);
		memcpy(&g_ASPara.RelayPulseWith,    BUF_PCSETTING + uTypeSize + AS_GATERELAYPULSE_OFFSET,   AS_GATERELAYPULSE_LEN);
		memcpy(&g_ASPara.MCUnlockEnable,    BUF_PCSETTING + uTypeSize + AS_MCUNLOCKENABLE_OFFSET,   AS_MCUNLOCKENABLE_LEN);
		memcpy(&g_ASPara.GateOpenContactor, BUF_PCSETTING + uTypeSize + AS_GATECONTACTOR_OFFSET,    AS_GATECONTACTOR_LEN);
		g_ASPara.GateOpenContactor = !g_ASPara.GateOpenContactor;
        g_ASPara.bEnableAS = !!(*(          BUF_PCSETTING + uTypeSize + AS_ENABLEAS_OFFSET)) ? TRUE : FALSE;
		memcpy(&g_ASPara.dwAccServerIP,     BUF_PCSETTING + uTypeSize + AS_ACCSERVERIP_OFFSET,      AS_ACCSERVERIP_LEN);
		memcpy(g_ASPara.szACCDevCode,       BUF_PCSETTING + uTypeSize + AS_ACCDEVCODE_OFFSET,       AS_ACCDEVCODE_LEN);
        memcpy(&g_ASPara.WiegandBitNum,     BUF_PCSETTING + uTypeSize + AS_ACCWIEGANDBITNUM_OFFSET,     AS_ACCWIEGANDBITNUM_LEN);
		memcpy(&g_ASPara.WiegandReverse,    BUF_PCSETTING + uTypeSize + AS_ACCWIEGANDREVERSE_OFFSET,    AS_ACCWIEGANDREVERSE_LEN);
#ifdef FORCE_ULK_FALG        
        g_ASPara.EnableForceUlkReport = *(BUF_PCSETTING + uTypeSize + AS_FORCEULKREPORT_OFFSET);/*whatever type of GM,this field exists*/
        printf("<WriteSysPara_V2>EnableForceUlkReport : %u\n",g_ASPara.EnableForceUlkReport);
#endif //FORCE_ULK_FALG
        
		SendACK(PCSettingParaTemp, 0x00, 0);
		SaveMenuPara2Mem();
		system("reboot");		
		while (1);
	}
	else
	{
		SendACK(PCSettingParaTemp, 0xff, 0);
	}
}

static void ReadSysPara_V2(E2PROM_SETTING PCSettingPara)
{
    WORD    ParaCRC     = 0;
	WORD    VldDataLen  = 0;
	DWORD   AddrForNet  = 0;
    BOOL    bValue      = FALSE;
    size_t  uTypeSize   = sizeof(E2PROM_SETTING);
    printf("uTypeSize : %u\n",uTypeSize);
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, uTypeSize);
	check_sum_E2PROM = 0;
    if( (PCSettingParaTemp.DATALEN - uTypeSize) < size_E2PROM)
    {
        PCSettingParaTemp.CMD       = PCSettingParaTemp.CMD | 0x80;
		PCSettingParaTemp.DATALEN   = uTypeSize + 2/*Length of data*/ + SYS_DATA_LEN + AS_DATA_LEN/*length of AS*/ + 1/*data length of forcing ulk report*/ + 2/*CRC*/;
        printf("len : %u\n",PCSettingParaTemp.DATALEN);
		PCSettingParaTemp.ERR       = 0x00;
		memcpy(BUF_PCSETTING, &PCSettingParaTemp, uTypeSize);
		VldDataLen = SYS_DATA_LEN + AS_DATA_LEN/*length of AS*/ + 1/*data length of forcing ulk report*/;
		memcpy(BUF_PCSETTING + uTypeSize, &VldDataLen, 2);
        /*Offset of value of total data length added*/
        uTypeSize += 2;
		memcpy(BUF_PCSETTING + uTypeSize + SYS_LANGUAGESET_OFFSET,      &g_SysConfig.LangSel,       SYS_LANGUAGESET_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_IPADDR_OFFSET,		    &g_SysConfig.IPAddr,        SYS_IPADDR_LEN);
		AddrForNet = htonl(g_SysConfig.MCIPSERVER);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_MCIP_SERVER_OFFSET,      &AddrForNet,	            SYS_MCIP_SERVER_LEN);
		AddrForNet = htonl(g_SysConfig.MCIPCLIENT);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_MCIP_CLIENT_OFFSET,      &AddrForNet,	            SYS_MCIP_CLIENT_LEN);		
		memcpy(BUF_PCSETTING + uTypeSize + SYS_RINGSEL_OFFSET,		    &g_SysConfig.RingNum,	    SYS_RINGSEL_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_SYSPWD_OFFSET,		    &g_SysConfig.SysPwd,	    SYS_SYSPWD_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_RESICODEVLD_OFFSET,      &g_SysConfig.VldCodeDigits, SYS_RESICODEVLD_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_TAMPERALARM_OFFSET,      &g_SysConfig.EnTamperAlarm, SYS_TAMPERALARM_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_CAMERASEL_OFFSET,	    &g_SysConfig.ActiveCamera,	SYS_CAMERASEL_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_FUNDI1_OFFSET,		    &g_SysConfig.DI1,			SYS_FUNDI1_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_FUNDI2_OFFSET,		    &g_SysConfig.DI2,			SYS_FUNDI2_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_FUNDO1_OFFSET,		    &g_SysConfig.DO1,			SYS_FUNDO1_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_FUNDO2_OFFSET,		    &g_SysConfig.DO2,			SYS_FUNDO2_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_FUNTALKTIME_OFFSET,	    &g_SysConfig.TalkTime,		SYS_FUNTALKTIME_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_FUNRINGTIME_OFFSET,	    &g_SysConfig.RingTime,		SYS_FUNRINGTIME_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_FUNDO1_PULSETIME_OFFSET,	&g_SysConfig.DO1_Pulse_Time,SYS_FUNDO1_PULSETIME_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + SYS_FUNDO2_PULSETIME_OFFSET,	&g_SysConfig.DO2_Pulse_Time,SYS_FUNDO2_PULSETIME_LEN);
		if (SERIES_SHELL == g_DevFun.SeriesType || SERIES_MINI_SHELL == g_DevFun.SeriesType) 
		{
			memcpy(BUF_PCSETTING + uTypeSize + SYS_TALK_VOLUME_OFFSET,	&g_SysConfig.TalkVolume,	SYS_TALK_VOLUME_LEN);
			memcpy(BUF_PCSETTING + uTypeSize + SYS_UNLOCK_TIME_OFFSET,	&g_SysConfig.UnlockTime,	SYS_UNLOCK_TIME_LEN);
		}
        /*Offset of value of system data*/
        uTypeSize += SYS_DATA_LEN;
        *(BUF_PCSETTING + uTypeSize + PU_FUNVLD_OFFSET)                 = g_PUPara.bFunEnabled ? 1 : 0;
		memcpy(BUF_PCSETTING + uTypeSize + PU_MNGPWD_OFFSET,            g_SysConfig.SysPwd,         PU_MNGPWD_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + PU_RESIPWD_OFFSET,           g_PUPara.ResiPwdDefault,    PU_RESIPWD_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + AS_WGCODE_OFFSET,            &g_ASPara.VagCodeSel,       AS_WGCODE_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + AS_GATEOPENDLYT_OFFSET,      &g_ASPara.GateOpenOverTime, AS_GATEOPENDLYT_LEN);
        *(BUF_PCSETTING      + uTypeSize + AS_GOOTA_OFFSET)             = g_ASPara.GateOpenAlmFlag ? 1 : 0;
		memcpy(BUF_PCSETTING + uTypeSize + AS_IVLDCARDT_OFFSET,         &g_ASPara.InvldCardSwipeRptSet, AS_IVLDCARDT_LEN);
        *(BUF_PCSETTING      + uTypeSize + AS_IVLDCARDA_OFFSET)         = g_ASPara.InvldCardSwipeAlmFlag ? 1 : 0;
        *(BUF_PCSETTING      + uTypeSize + AS_DOORMANOPEN_OFFSET)       = g_ASPara.GateOpenManFlag ? 1 : 0;
		*(BUF_PCSETTING      + uTypeSize + AS_GATEOPENMODE_OFFSET)      = g_ASPara.ASOpenMode;
		memcpy(BUF_PCSETTING + uTypeSize + AS_GATERELAYPULSE_OFFSET,    &g_ASPara.RelayPulseWith,   AS_GATERELAYPULSE_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + AS_MCUNLOCKENABLE_OFFSET,    &g_ASPara.MCUnlockEnable,   AS_MCUNLOCKENABLE_LEN);
		bValue = !g_ASPara.GateOpenContactor;
		memcpy(BUF_PCSETTING + uTypeSize + AS_GATECONTACTOR_OFFSET,     &bValue,                    AS_GATECONTACTOR_LEN);
        *(BUF_PCSETTING      + uTypeSize + AS_ENABLEAS_OFFSET)          = g_ASPara.bEnableAS ? 1 : 0;
		memcpy(BUF_PCSETTING + uTypeSize + AS_ACCSERVERIP_OFFSET,       &g_ASPara.dwAccServerIP,	AS_ACCSERVERIP_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + AS_ACCDEVCODE_OFFSET,        g_ASPara.szACCDevCode,      AS_ACCDEVCODE_LEN);

		memcpy(BUF_PCSETTING + uTypeSize + AS_ACCWIEGANDBITNUM_OFFSET,  &g_ASPara.WiegandBitNum,    AS_ACCWIEGANDBITNUM_LEN);
		memcpy(BUF_PCSETTING + uTypeSize + AS_ACCWIEGANDREVERSE_OFFSET, &g_ASPara.WiegandReverse,   AS_ACCWIEGANDREVERSE_LEN);
#ifdef FORCE_ULK_FALG 
        *(BUF_PCSETTING + uTypeSize + AS_FORCEULKREPORT_OFFSET) = g_ASPara.EnableForceUlkReport;/*whatever type of GM,this field exists*/
        printf("<ReadSysPara_V2>EnableForceUlkReport : %u\n",g_ASPara.EnableForceUlkReport);
#endif //FORCE_ULK_FALG

        ParaCRC = MoxCRC(BUF_PCSETTING + uTypeSize, SYS_DATA_LEN + AS_DATA_LEN + 1/*data length of forcing ulk report*/);
		memcpy(BUF_PCSETTING + uTypeSize + AS_DATA_LEN/*length of AS*/ + 1/*data length of forcing ulk report*/,        &ParaCRC,                   2);
		
		SndPacket(&sock_E2PROM, BUF_PCSETTING, PCSettingParaTemp.DATALEN, &recv_addr_E2PROM);
    }
    
}
/**********************************************************************************************/

static void 
ReadFactoryPara(E2PROM_SETTING PCSettingPara)
{
	check_sum_E2PROM = 0;
	WORD	ParaLen	=	0;
	DWORD	dwParaVer	=	0;
	CHAR    HardVer[4] = { 0 };
	int		MacAddrTemp[6] = { 0 };
	BYTE	MacAddr[6] = { 0 };
	CHAR	*pStr	=	NULL;
	int		i = 0;
    int   mainVersion = 0;
    int     subVersion = 0;
	if( (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)) < size_E2PROM)
	{
		//Head
		PC_SETTING_PARAMS.CMD			= PC_SETTING_PARAMS.CMD + 0x80;
		PC_SETTING_PARAMS.DATALEN		= sizeof(PC_SETTING_PARAMS) + FACTORY_DATA_LEN;
		PC_SETTING_PARAMS.ERR          = 0x00;
		memcpy(BUF_PCSETTING, &PC_SETTING_PARAMS, sizeof(PC_SETTING_PARAMS));
        //Data
        if(g_VerContrl.uIsChanged)			
        {/*Please search 'VERSION_CONTROL' to get more information about this judgement*/
			memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+FACTORY_MODULETYPE_OFFSET],g_VerContrl.BackupPN, FACTORY_MODULETYPE_LEN);
			printf("hard version : %s\n",g_VerContrl.BackupHardVer);
			/*Get main version number,Get sub version number*/
			sscanf(g_VerContrl.BackupHardVer,"V%d.%d",&mainVersion,&subVersion);
			printf("mainVer : %d\n",mainVersion);
			printf("subVer : %d\n",subVersion);
			HardVer[3] = (CHAR)mainVersion;
			HardVer[2] = (CHAR)subVersion;
        }
        else
        {
            memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+FACTORY_MODULETYPE_OFFSET],g_FactorySet.ModuleType, FACTORY_MODULETYPE_LEN);
            HardVer[2] = 10 * (g_FactorySet.Hardver[3] - 0x30) + (g_FactorySet.Hardver[4] - 0x30);
		    HardVer[3] = g_FactorySet.Hardver[1] - 0x30;
        }				 
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+FACTORY_PARALEN_OFFSET],		&ParaLen, FACTORY_PARALEN_LEN);					 
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+FACTORY_PARAVER_OFFSET],		&dwParaVer, FACTORY_PARAVER_LEN);
		
		printf("g_FactorySet.Hardver = %s\n", g_FactorySet.Hardver);
		//printf("g_FactorySet.Hardver: %x %x %x %x\n",g_FactorySet.Hardver[0],g_FactorySet.Hardver[1],g_FactorySet.Hardver[2],g_FactorySet.Hardver[3]);
		printf("HardVer: %x %x %x %x\n",HardVer[0],HardVer[1],HardVer[2],HardVer[3]);
		/*example:
            V       2       .        40         .       0       .       0
                HardVer[3]        HardVer[2]        HardVer[1]      HardVer[0]
        */
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+FACTORY_HARDVER_OFFSET], HardVer, FACTORY_HARDVER_LEN);		

		OpenIniFile(MAC);	
		
		pStr = ReadString(MAC_SEC_GLOBAL, MAC_KEY_ADDRESS, MAC_VALUE_ADDRESS);

		if (pStr) 
		{			
			sscanf(pStr, "%x:%x:%x:%x:%x:%x", 
				&MacAddrTemp[0], &MacAddrTemp[1], &MacAddrTemp[2], &MacAddrTemp[3], &MacAddrTemp[4], &MacAddrTemp[5]);
		}

		for(i = 0; i < 6; i++)
		{	
			MacAddr[i] = (BYTE)MacAddrTemp[i];
		}

		CloseIniFile();
		
		memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+FACTORY_MACADDR_OFFSET], MacAddr, FACTORY_MACADDR_LEN );
		strcpy((char*)&BUF_PCSETTING[sizeof(E2PROM_SETTING)+FACTORY_SOFTVER_OFFSET], (char*)g_SysInfo.SoftwareVersion);
		//Send data	
		printf("%s,%d,,%X,%d,%d\n",__func__,__LINE__,recv_addr_E2PROM.sin_addr.s_addr,recv_addr_E2PROM.sin_family,recv_addr_E2PROM.sin_port);
		SndPacket(&sock_E2PROM, BUF_PCSETTING, PC_SETTING_PARAMS.DATALEN, &recv_addr_E2PROM);
	}	
}

static void 
WriteFactoryPara(E2PROM_SETTING PCSettingPara)
{
	DWORD	ConfigVer = 0;
	DWORD	AddrForNet =  0;
	CHAR    HardVer[4] = { 0 };
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, sizeof(E2PROM_SETTING));
	check_sum_E2PROM = 0;
	
#if 0
	int offset=0;
	int i=0;
	int len=PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING);
	BYTE *pData=(BYTE *)&BUF_PCSETTING[sizeof(E2PROM_SETTING)+2];
	printf("%s,%d start...len:%d,size_E2PROM:%d\n",__func__,__LINE__,len,size_E2PROM);
	for(i=0;i<len;i++)
	{
		printf("%.2x  ",*(pData+i));
		offset++;
		if(offset==16)
		{
			offset=0;
			printf("\n");
		}
	}
	printf("\n");
#endif

	memcpy(&ConfigVer, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+2], 2);

	if(1!=ConfigVer)
	{
		printf("%s,%d,not support ConfigVer %d\n",__func__,__LINE__,ConfigVer);
		SendACK(PCSettingParaTemp, 0xff, 0);
		return;
	}
	
	memcpy(HardVer, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+FACTORY_HARDVER_WRITE_OFFSET], FACTORY_HARDVER_LEN);
	
	if( (PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING)) < size_E2PROM)
	{				
		OpenIniFile(HW);
		
		if(g_VerContrl.uIsChanged)
		{			
        	memcpy(g_VerContrl.BackupPN, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+FACTORY_MODULETYPE_WRITE_OFFSET], FACTORY_MODULETYPE_LEN);

			sprintf(g_VerContrl.BackupHardVer,"V%d.%02d",HardVer[3],HardVer[2]);

			printf("%s,%d,%s,%s\n",__func__,__LINE__,g_VerContrl.BackupPN,g_VerContrl.BackupHardVer);
			
			WriteString(HW_SEC_GLOBAL, HW_KEY_MODULETYPE, g_VerContrl.BackupPN);
			WriteString(HW_SEC_GLOBAL, SW_KEY_SWVER, g_VerContrl.BackupHardVer);
		
		}
		else
		{		
			memcpy(g_FactorySet.ModuleType, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+FACTORY_MODULETYPE_WRITE_OFFSET], FACTORY_MODULETYPE_LEN);

			sprintf(g_FactorySet.Hardver,"V%d.%02d",HardVer[3],HardVer[2]);

			printf("%s,%d,%s,%s\n",__func__,__LINE__,g_FactorySet.ModuleType,g_FactorySet.Hardver);

			WriteString(HW_SEC_GLOBAL, HW_KEY_MODULETYPE, g_FactorySet.ModuleType);
			WriteString(HW_SEC_GLOBAL, HW_KEY_HARVER, g_FactorySet.Hardver);
		}				
				
		SendACK(PCSettingParaTemp, 0x00, 0);//为了防止响应超时，这个动作必须放在写文件系统之前

		system("mount -n -o remount rw /mox");
        system("mv /mox/rdwr/DI1.pcm /mox");
        system("mv /mox/rdwr/DI2.pcm /mox");
		system("rm -f /mox/rdwr/*");
        system("mv /mox/DI1.pcm /mox/rdwr");
        system("mv /mox/DI2.pcm /mox/rdwr");
        system("ls /mox/rdwr/");

		WriteIniFile(HW);
		CloseIniFile();

		printf("%s,%d,success\n",__func__,__LINE__);
		
		g_bNeedRestart = TRUE;
		usleep(10*1000);
		
		system("reboot");	
		while (1);
	}
	else
	{
		printf("%s,%d,length error\n",__func__,__LINE__);
		SendACK(PCSettingParaTemp, 0xff, 0);
	}

}


static void 
SendCardInfo(E2PROM_SETTING PCSettingPara)
{
	DWORD dwOffset = 0;
	WORD DataPktLen = 0;

	check_sum_E2PROM = 0;

	if( (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)) < size_E2PROM)
	{
		//Head
		PC_SETTING_PARAMS.CMD			= PC_SETTING_PARAMS.CMD + 0x80;

		if (g_CardUldInfo.bCardUpLoad) 
		{
			PC_SETTING_PARAMS.ERR          = 0x00;
			g_CardUldInfo.bCardUpLoad	   = FALSE;
			//Data
			dwOffset = 0;
			DataPktLen = (WORD)(CARD_REQ_INFO_LEN + CARD_REQ_LENTH_LEN + g_CardUldInfo.nCardLen);
			memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+dwOffset],	&DataPktLen, CARD_REQ_INFO_LEN);					 
			dwOffset += CARD_REQ_INFO_LEN;
			memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+dwOffset],	&g_CardUldInfo.nCardLen, CARD_REQ_LENTH_LEN);					 
			dwOffset += CARD_REQ_LENTH_LEN;
			memcpy(&BUF_PCSETTING[sizeof(E2PROM_SETTING)+dwOffset],	g_CardUldInfo.CSN, g_CardUldInfo.nCardLen);
			PC_SETTING_PARAMS.DATALEN		= sizeof(PC_SETTING_PARAMS) + DataPktLen;
			memcpy(BUF_PCSETTING, &PC_SETTING_PARAMS, sizeof(PC_SETTING_PARAMS));
		}
		else
		{
			PC_SETTING_PARAMS.ERR           = 0x01;//无卡
			PC_SETTING_PARAMS.DATALEN		= sizeof(PC_SETTING_PARAMS);
			memcpy(BUF_PCSETTING, &PC_SETTING_PARAMS, sizeof(PC_SETTING_PARAMS));
		}
		
		//Send data
		
		SndPacket(&sock_E2PROM, BUF_PCSETTING, PC_SETTING_PARAMS.DATALEN, &recv_addr_E2PROM);
	}	
}

static void 
CardInfoDownload(E2PROM_SETTING PCSettingPara)
{
	DWORD dwOffset = 0;
	WORD DataPktLen = 0;
	UCHAR  CardType = 0;
	char	RdCode[CARD_DLD_RDCODE_LEN] = { 0 };
	int     nRdLen = 0;
	
	CDINFO_HV_t CdInfo = 
	{
		"",
		CARD_LOCALNUM_DEFAULT,
		CARD_PASSWORD_DEFAULT,

		RD_NUM_DEFAULT,
		GROUP_NUM_DEFAULT,		
		TYPE_NORMAL_CARD,
		CARD_STATUS_ENABLED,
		CARD_MODE_DEFAULT,		
		GATE_NUMBER_DEFAULT		
	};	
	
	check_sum_E2PROM = 0;
	
	if( (PC_SETTING_PARAMS.DATALEN - sizeof(PC_SETTING_PARAMS)) < size_E2PROM)
	{
		//Head
		PC_SETTING_PARAMS.CMD			= PC_SETTING_PARAMS.CMD + 0x80;
		
		PC_SETTING_PARAMS.ERR          = 0x00;
		PC_SETTING_PARAMS.DATALEN		= sizeof(PC_SETTING_PARAMS);
		memcpy(BUF_PCSETTING, &PC_SETTING_PARAMS, sizeof(PC_SETTING_PARAMS));
		
		//Send data
		SndPacket(&sock_E2PROM, BUF_PCSETTING, PC_SETTING_PARAMS.DATALEN, &recv_addr_E2PROM);

		//Data
		dwOffset = 0;
		memcpy(&DataPktLen, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+dwOffset],	CARD_DLD_INFO_LEN);					 
		dwOffset += CARD_DLD_INFO_LEN;

		nRdLen = strlen((char*)&BUF_PCSETTING[sizeof(E2PROM_SETTING)+dwOffset]);

		printf("nRdLen = %d\n",nRdLen);

		if (nRdLen > 0) 
		{
			memcpy(RdCode, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+dwOffset], nRdLen);
		}
		else
		{
			strcpy(RdCode, "00");
		}

		dwOffset += CARD_DLD_RDCODE_LEN;
		//memcpy(&CdInfo.nCardLen, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+dwOffset],	CARD_DLD_LENTH_LEN);					 
		dwOffset += CARD_DLD_LENTH_LEN;

		memcpy(CdInfo.CSN, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+dwOffset],	g_CardUldInfo.nCardLen);
		dwOffset += g_CardUldInfo.nCardLen;
		
		memcpy(CdInfo.UlkPwd, &BUF_PCSETTING[sizeof(E2PROM_SETTING)+dwOffset],	PWD_LEN);
		dwOffset += CARD_DLD_PWD_LEN;

		CardType = BUF_PCSETTING[sizeof(E2PROM_SETTING)+dwOffset];

//		CdInfo.LocalNum = g_LocalNum ++;

		strcpy(CdInfo.RdNum,RdCode);
		
		if (1 == CardType) 
		{
			CdInfo.CardType = TYPE_AUTHORIZE_CARD;
            if(isAST_HVFileExisted())
            {
                if (AsAddAuthorizeCdHV(&CdInfo))
    			{
    				SaveAuthorizeCdHVInfo2Mem();
    			}
            }
            else
            {
				printf("%s,%d,editor 下载卡片 \r",__func__,__LINE__);	
				//editor 下载卡片
                if(AsAddAuthorizeCd((CDINFO_OLD*)&CdInfo))
                {
                    SaveAuthorizeCdInfo2Mem();
                }
            }
		}
		else
		{			
			printf("Normal Card\n");
		}
	}	
}

static void 
ClearHelpInfo(E2PROM_SETTING PCSettingPara)
{
	E2PROM_SETTING PCSettingParaTemp;
	memcpy(&PCSettingParaTemp, &PCSettingPara, sizeof(E2PROM_SETTING));
	check_sum_E2PROM = 0;
	if( (PCSettingParaTemp.DATALEN - sizeof(E2PROM_SETTING)) < size_E2PROM)
	{		
		SendACK(PCSettingParaTemp, 0x00, 0);
		g_SysConfig.bHelpInfoDld	=	FALSE;
		SaveMenuPara2Mem();
	}
	else
	{
		SendACK(PCSettingParaTemp, 0xff, 0);
	}	
}

int /*0 - false; 1 - OK*/
SndPacket(INT  * pSkt, PBYTE pSd, int nLen, struct sockaddr_in * pAddrTo)
{
	static unsigned long nSleepMask = 1;
	UCHAR bSdOK = 0;
	int nSendRslt= 0;
	UCHAR nSendCount = 0;
	int nAddrLen = sizeof(struct sockaddr_in);
	if ((pSd == NULL) || (pSkt == NULL) || (pAddrTo == NULL) || (pAddrTo->sin_addr.s_addr == 0))
	{
		printf("%s,%d\n",__func__,__LINE__);
		return bSdOK;
	}
    nSendCount = 0;
	
	while((!bSdOK)&&(nSendCount<10)) // send video data stream, if sending fail,try again,the max time is 3
	{
		nSendRslt = sendto(*pSkt, pSd, nLen, 0, pAddrTo, nAddrLen);		
		if(0 >= nSendRslt)
		{
//			printf("SndPacket error, ret=%d, errno = %d, nAddrLen = %d\n", nSendRslt, fdError(), nAddrLen);
		}		
//		dwDiag[DIAG_ETH_SENT]++;
		bSdOK = (nSendRslt == nLen)? 1 : 0;
		if (!bSdOK)
		{	
//			dwDiag[DIAG_ETH_SENT_ERR]++;
			
/*			if(errno == 55)
			{       
			}
*/
			if(9 == nSendCount)
			{
			}
			usleep(10*1000);
			
		}
		nSendCount++;
	}
	if (!(nSleepMask++ % 2))
	{
		usleep(1*1000);
	} 
	return bSdOK;
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:
**	AUTHOR:		 
**	DATE:		 
**
**	DESCRIPTION:	
** 
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/
unsigned short MoxCRC(unsigned char *pData, unsigned short nDataLen)
{
	 unsigned int nMask;
	 unsigned int nBit;
	 unsigned int nCRC;
	 unsigned int nMem;
	 unsigned char pusTableLo[0x400];
	 unsigned char pusTableHi[0x400];
	 unsigned char cCRCHi = 0xFF;
	 unsigned char cCRCLo = 0xFF; 
	 unsigned uIndex;

	 //Init tables
	 for (nMask = 0; nMask < 0x100; nMask++)
	 {
	  nCRC = nMask;
	  for (nBit = 0; nBit < 8; nBit++)
	  {
	   nMem = nCRC & 0x0001;
	   nCRC = nCRC >> 1;
	   if (nMem != 0)
	   {
		nCRC = nCRC ^ 0xA001;
	   }
	  }
	  pusTableHi[nMask] = nCRC & 0xFF;
	  pusTableLo[nMask] = nCRC >> 8;
	 }

	 //Create the CRC sum
	 while (nDataLen--)
	 {
	  uIndex = cCRCHi ^ *pData++;
	  cCRCHi = cCRCLo ^ pusTableHi[uIndex];
	  cCRCLo = pusTableLo[uIndex];
	 }

	 return (cCRCHi << 8 | cCRCLo);
}

/*****************************************************************************
 * Copyright Mox Products, Australia

 * FUNCTION
 *  SendConfInfoEth
 * DESCRIPTION
 *  Send Configure Information From Ethernet.
 * PARAMETERS
 *  pRcvData    [IN]    U8
 *  nSrcIPAddr  [IN]    U32
 * RETURNS
 *  void
 *****************************************************************************/
void SendConfInfoEth(UCHAR* pRcvData, UINT nSrcIPAddr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    UINT Function = 0;
    USHORT ParaLen = 0;
    USHORT FileLen = 1;
    USHORT ConfCnt = 2;
    MXMSG   msgSend;
    UCHAR* pData = NULL;

	UCHAR ONECARD = 1;
	UCHAR SORTINGALGORITHM = 1;
	
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
	
    memcpy(&Function,pRcvData,sizeof(UINT));
    memset(&msgSend, 0, sizeof(MXMSG));
    msgSend.dwSrcMd = MXMDID_NULL;
    msgSend.dwDestMd = MXMDID_ETH;
    msgSend.dwMsg = FC_ACK_GET_CONFINFO;	
    msgSend.dwParam = nSrcIPAddr;

#ifdef __SUPPORT_WIEGAND_CARD_READER__
    ParaLen = FileLen + 5 + 4 + 4;
	ConfCnt=3;
#else
	ParaLen = FileLen + 5 + 4 ;
#endif
    msgSend.pParam = (UCHAR*)malloc(ParaLen + sizeof(USHORT));
    if (NULL == msgSend.pParam) 
    {
        return;
    }
    memset(msgSend.pParam, 0, ParaLen + sizeof(USHORT));
    pData = msgSend.pParam;
    memcpy(pData, &ParaLen, sizeof(USHORT));
    pData += 2;
    memcpy(pData,&ConfCnt,2);
    pData += 2;
#ifdef __SUPPORT_WIEGAND_CARD_READER__	

    *pData = CONF_TYPE_WIEGAND;//CONF_TYPE_WIEGAND;
    pData++;
    memcpy(pData, &FileLen, 2);
    pData += 2;
    *pData = g_ASPara.WiegandBitNum;
    pData++;
#endif

	*pData = CONF_TYPE_SORTINGALGORITHM;
	pData++;	
	memcpy(pData, &FileLen, 2);
    pData += 2;
    *pData = SORTINGALGORITHM;
	pData++;

	*pData = CONF_TYPE_ONECARD;
	pData++;	
	memcpy(pData, &FileLen, 2);
	pData += 2;
	*pData = ONECARD;
	pData++;
    pData = msgSend.pParam;
    MxPutMsg(&msgSend);
}

#ifdef __SUPPORT_PROTOCOL_900_1A__
BOOL MOX_PROTOCOL_SUPPORT(MOX_PROTOCOL_TYPE nType)
{
	BOOL nResult = FALSE;
	//printf("[%s]g_NewSysConfig.Version %d\n",__FUNCTION__,g_NewSysConfig.Version);
	switch(nType)
	{
		case MOX_P_900_1:
			nResult = TRUE;
			break;
		case MOX_P_900_1A:
			if(g_NewSysConfig.Version)
				nResult = TRUE;
			break;
		default:
			printf("[%s] parameter is error, please check!\n", __FUNCTION__);
	}
	return nResult;
}
static void Revert2OldVer ()
{
	if(g_NewSysConfig.Version != 0)
	{
		g_NewSysConfig.Version = 0;
		SetNewSysConfigVersion(g_NewSysConfig.Version);
	}
}

#endif 

