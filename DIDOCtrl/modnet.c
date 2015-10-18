/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	modnet.c
**
**	AUTHOR:		Jason Wang
**
**	DATE:		2012-04-10
**
**	FILE DESCRIPTION:
**		Use modnet protocol to control DI/DO module
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

 
/************** USER INCLUDE FILES ***************************************************/
#include "modnet.h" 
#include "MXMem.h"
#include "MXList.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"

/************** DEFINES **************************************************************/
// #define DO_DBG
/************** TYPEDEFS *************************************************************/
 
/************** STRUCTURES ***********************************************************/
 
/************** EXTERNAL DECLARATIONS ************************************************/
 
//!!!  It is H/H++ file specific, nothing should be defined here
 
/************** ENTRY POINT DECLARATIONS *********************************************/
 
/************** LOCAL DECLARATIONS ***************************************************/
static INT modnet_sock = 0; 
// static pthread_t modnet_pid = 0;
static BYTE modnet_header[5] = {0, 0, 0, 0, 0};
static unsigned int dest_ip = 0; // use in analysis_ack func, to judge if the correct slave respond
static unsigned short dest_port = 0; // use in analysis_ack func, to judge if the correct slave respond

// static void* modnet_thread_fun(void* arg);



/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    modnet_init
**   Owner: Jason Wang
**	DATE:		2012-04-11 
**	
**	DESCRIPTION:	
**		create modnet sock, then create the work thread
**	ARGUMENTS:	
**		NULL
**	RETURNED VALUE:	
**		NULL
**	NOTES:
**	
*/
BOOL modnet_init()
{
	int block_flag = 1;
	struct sockaddr_in sock_addr;
	int sock_addr_len = sizeof(struct sockaddr_in);
	int rst = 0;
	
	modnet_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == modnet_sock)
	{
		printf("%s:%s:%dn error", __FILE__, __FUNCTION__, __LINE__);
		perror("");
		return FALSE;
	}
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);	
	sock_addr.sin_port = htons(modnet_udp_port);	
   
    rst = bind(modnet_sock, (struct sockaddr*) &sock_addr, sock_addr_len);
	if (-1 == rst)
	{
		printf("%s:%s:%dn error", __FILE__, __FUNCTION__, __LINE__);
		perror("");
		return FALSE;
	}	
    rst = ioctl(modnet_sock, FIONBIO, &block_flag); 
	if (-1 == rst)
	{
		printf("%s:%s:%dn error", __FILE__, __FUNCTION__, __LINE__);
		perror("");
		return FALSE;
	}	
	return TRUE;
}

void send_cmd_to_DI_by_eth(unsigned int rip, unsigned short rport, BYTE cmd, unsigned short start_addr, unsigned short count)
{
	BYTE send_buf[BUF_LEN] = {0};
	struct sockaddr_in remote_addr;
	int remote_addr_len;	
	memcpy(send_buf, modnet_header, 5);

	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr	= htonl(rip);
	remote_addr.sin_port = htons(rport);
	remote_addr_len = sizeof(struct sockaddr_in);	
	dest_ip = rip;	
	dest_port = rport;
	switch (cmd)
	{
	case 0x03:
	case 0x04:
		{
			send_buf[5] = 0x06;
			send_buf[6] = 0x00;
			send_buf[7] = 0x03;
			send_buf[8] = (BYTE)(start_addr >> 8);
			send_buf[9] = (BYTE)(start_addr);
			send_buf[10] = (BYTE)(count >> 8);
			send_buf[11] = (BYTE)(count);
			sendto(modnet_sock, send_buf, 12, 0, (struct sockaddr*)&remote_addr, remote_addr_len);
		}
		break;
	default:
		{
			printf("%s:%d command error\n", __FUNCTION__, __LINE__);
		}
	}
}

/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    send_cmd_to_DO_by_eth
**   Owner: Jason Wang
**	DATE:		2012-04-12 
**	
**	DESCRIPTION:	
**		control DO module through modbus/udp protocol
**	ARGUMENTS:	
**		rip: DO's ip which want to control
**		rport: DO's app port which want to control
**		cmd: 0x01; 0x05; 0x0f; according modbus protocol
**		start_addr: the operating DO's port starting addr
**		count: the coperating DO's ports' number
**		value: when cmd as write coils, this is the pre_status of the operating DOs' ports
**	RETURNED VALUE:	
**		NULL
**	NOTES:
**		value :when cmd is 0x0f, value bit low~high  express addr~addr+count DOs' status
*/
void send_cmd_to_DO_by_eth(unsigned int rip, unsigned short rport, BYTE cmd, unsigned short start_addr, unsigned short count, unsigned short value) 
{
	BYTE send_buf[BUF_LEN] = {0};
	struct sockaddr_in remote_addr;
	int remote_addr_len;	
	int rst = 0;

#ifdef DO_DBG
				printf("%s: 1\n", __FUNCTION__);
				printf("rip=%x \t rport=%x\n", rip, rport);
#endif	

	
	memcpy(send_buf, modnet_header, 5);
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr	= htonl(rip);
	remote_addr.sin_port = htons(rport);
	remote_addr_len = sizeof(struct sockaddr_in);	
	dest_ip = rip;	
	dest_port = rport;	

#ifdef DO_DBG
			printf("%s: 2\n", __FUNCTION__);
#endif	
	switch (cmd)
	{
	case 0x01: // 读多个线圈
		{
			send_buf[5] = 0x06;
			send_buf[6] = 0x00; // slave address
			send_buf[7] = 0x01; // function code
			send_buf[8] = (BYTE)(start_addr >> 8); // start address HI
			send_buf[9] = (BYTE)(start_addr);      // start address LO		
			send_buf[10] = (BYTE)(count >> 8);     // count HI		
			send_buf[11] = (BYTE)(count);          // count LO 
			sendto(modnet_sock, send_buf, 12, 0, (struct sockaddr*)&remote_addr, remote_addr_len);
		}
		break;
	case 0x05: // 写单个线圈
		{
			send_buf[5] = 0x06;
			send_buf[6] = 0x00; // slave address
			send_buf[7] = 0x05;
			send_buf[8] = (BYTE)(start_addr >> 8);
			send_buf[9] = (BYTE)(start_addr);
			if (COIL_ON == value) 
			{
				send_buf[10] = 0xff;
				send_buf[11] = 0x00;
			}
			else if (COIL_OFF == value)
			{
				send_buf[10] = 0x00;
				send_buf[11] = 0x00;
			}
			else
			{
				printf("%s:%d coil value error\n", __FUNCTION__, __LINE__);	
				return;
			}
#ifdef DO_DBG
			printf("%s: 3\n", __FUNCTION__);
#endif	

			rst = sendto(modnet_sock, send_buf, 12, 0, (struct sockaddr*)&remote_addr, remote_addr_len);
			if (-1 == rst)
			{
				perror("sendto msg error:\n");
			}
#ifdef DO_DBG
			printf("%s: 4\n", __FUNCTION__);
			int i = 0;
			for (i=0; i<12; i++)
			{
				printf("send_buf[%d]=%x\t", i, send_buf[i]);
			}
			printf("\n");
#endif	

	}
		break;	
	case 0x0f: // 写多个coil
		{
			send_buf[5] = 0x09;
			send_buf[6] = 0x00; // slave address
			send_buf[7] = 0x0f;
			send_buf[8] = (BYTE)(start_addr >> 8);
			send_buf[9] = (BYTE)(start_addr);
			send_buf[10] = (BYTE)(count >> 8);
			send_buf[11] = (BYTE)(count);
			send_buf[12] = (count + 7) / 8; // byte count
			memcpy(&send_buf[13], (BYTE *)&value, 2);
			sendto(modnet_sock, send_buf, 15, 0, (struct sockaddr*)&remote_addr, remote_addr_len);
		}
		break;			
	default:
		{
			printf("%s:%d command error\n", __FUNCTION__, __LINE__);
		}
	}	
}


/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    wait_slave_ack_eth
**   Owner: Jason Wang
**	DATE:		2012-04-12 
**	
**	DESCRIPTION:	
**			wait ack from the slave after send cmd to the slave
**	ARGUMENTS:	
**		 	buf: ack content from the slave
**			buf_len: the length of the buf 
**			rip: the ip addr where ack come from
**			rport: the port where ack com from
**			time_out: wait ack time out time (seconds)
**	RETURNED VALUE:	
**			the length of the slave ack, or ACK_TIME_OUT indicate has wait ack time out
**	NOTES:
**	
*/
unsigned int wait_slave_ack_eth(BYTE *buf, unsigned int buf_len, unsigned int *rip, unsigned short *rport, unsigned int time_out)
{
	fd_set rfd;
	struct timeval timeout;	
	struct sockaddr_in remote_addr;
	socklen_t remote_addr_len;	
	unsigned int ack_len = 0;

	FD_ZERO(&rfd);	
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	FD_SET(modnet_sock, &rfd); 
	if(0 == select(modnet_sock+1,&rfd, 0, 0, &timeout)) // 等待ack 超时
	{
		return ACK_TIME_OUT;
	}
	else
	{
		if(FD_ISSET(modnet_sock,&rfd))
		{
			ack_len = recvfrom(modnet_sock, buf, buf_len, 0, (struct sockaddr*)&remote_addr, &remote_addr_len);
			*rip = remote_addr.sin_addr.s_addr;
			*rport = remote_addr.sin_port;
			return ack_len;
		}
		else
		{
			printf("file:%s\tfunction:%s\tline:%d error\n", __FILE__, __FUNCTION__, __LINE__);
			perror("error:\n");
			return ACK_TIME_OUT;
		}
	}
}


/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    analysis_ack_eth
**   Owner: Jason Wang
**	DATE:		2012-04-12 
**	
**	DESCRIPTION:	
**			analysis the ack from the slave
**	ARGUMENTS:	
**			buf: the ack content received
**			buf_len: the length of the ack content
**			rip: the ip where the ack come from
**			rport: the port where the ack come from
**			value: the coil or regs value 
**	RETURNED VALUE:	
**			TRUE: operation successed, FALSE: operation failure
**	NOTES:
**	
*/
BOOL analysis_ack_eth(BYTE *buf, unsigned int buf_len, unsigned int rip, unsigned short rport, unsigned int *value)
{
	BYTE cmd = buf[7];
#if 0
	if (dest_ip != ntohl(rip) || dest_port != ntohs(rport)) // ack not from the corresponding slave
	{
		printf("dest_ip=%x \t rip=%x \t dest_port=%x \t rport=%x \n", dest_ip, rip, dest_port, rport);
		return FALSE;
	}
#endif
	if ((cmd >> 7) & 1) // 0x8* indicate execption
	{
		printf("file:%s\tfunction:%s\tline:%d error\n", __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	switch (cmd)
	{
	case 0x01:
		{
			unsigned short byte_count = buf[8]; // value byte count
			memcpy((BYTE *)value, &buf[9], byte_count);
			return TRUE;
		}
		break;
	case 0x03:
	case 0x04:		
		{
			return TRUE;
		}
		break;	
	case 0x05:
		{
			return TRUE;
		}
		break;	
	case 0x0f:
		{
			return TRUE;
		}
		break;	
	default:
		{
			return FALSE;
		}
		break;				
	}
}

