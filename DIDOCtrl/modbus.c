/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	modbus.c
**
**	AUTHOR:		Jason Wang
**
**	DATE:		2012-04-10
**
**	FILE DESCRIPTION:
**		Use modbus protocol to control DI/DO module
**
**	FUNCTIONS:
**
**	NOTES:
**
*/
/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>  
#include <termios.h> 
 
/************** USER INCLUDE FILES ***************************************************/
#include "crc32.h" 
#include "MXTypes.h"
#include "MXCommon.h"
#include "modbus.h"
/************** DEFINES **************************************************************/
#define BUF_LEN 256  
#define ACK_TIME_OUT 0xffffffff
#define COIL_ON 1
#define COIL_OFF 0


/************** TYPEDEFS *************************************************************/
 
/************** STRUCTURES ***********************************************************/
 
/************** EXTERNAL DECLARATIONS ************************************************/
 
//!!!  It is H/H++ file specific, nothing should be defined here
 
/************** ENTRY POINT DECLARATIONS *********************************************/
 
/************** LOCAL DECLARATIONS ***************************************************/
static int serial_fd = 0; 
static BYTE stop_addr = 0;

static INT open_serial_port(void);

/*************************************************************************************/

BOOL 
modbus_init()
{
	serial_fd = open_serial_port();	
	if (-1 == serial_fd)
	{
		return FALSE;
	}
	set_serial_para(9600, 8, 0, 2);	
	return TRUE;
}

static INT 
open_serial_port()
{
	char			tcPort[16] = "/dev/ttymxc3";
	struct termios	oldtio;
	struct termios	newtio;	
	int				nFd = -1;

	if(access(SPEFILE, F_OK) == 0)
		strcpy(tcPort, "/dev/ttymxc1");
		
	nFd = open(tcPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (nFd < 0) 
	{
		printf("Uart Open Error\n");
		perror("");
		return -1;
	}
	return nFd;
} 

void
set_serial_para(unsigned int baud_rate, BYTE data_bits, BYTE parity_flag, BYTE stop_bits)
{
	struct termios	oldtio;
	struct termios	newtio;	

	tcgetattr(serial_fd,&newtio); 
	cfsetispeed(&newtio, B9600);
	cfsetospeed(&newtio, B9600);

	newtio.c_cflag &= ~CSIZE; /* Mask the character size bits */
	newtio.c_cflag |= CS8;    /* Select 8 data bits */

	newtio.c_cflag &= ~PARENB;//mode:8N1
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_cflag &= ~CSIZE;
	
	newtio.c_cflag |= CS8;

	newtio.c_cflag |= (CLOCAL | CREAD);

	newtio.c_cflag &= ~CRTSCTS; /*disable hardware flow control*/
	newtio.c_iflag &= ~(IXON | IXOFF | IXANY); /*disable software flow control */
	
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /*Input*/
	newtio.c_oflag &= ~OPOST; /*Output*/
	newtio.c_iflag &=~(INLCR|IGNCR|ICRNL);
    newtio.c_oflag &=~(ONLCR|OCRNL);
	
	newtio.c_cc[VTIME] = 0;//timeout = 2seconds
	newtio.c_cc[VMIN] = 0;
	tcflush(serial_fd, TCIFLUSH);
	//tcflush(nFd, TCIOFLUSH);

	tcsetattr(serial_fd, TCSANOW, &newtio);	
}

void
send_cmd_to_DI_serial(BYTE stop_code, BYTE cmd, BYTE start_addr, BYTE count)
{
	BYTE send_buf[BUF_LEN] = {0};
	unsigned short crc = 0;
	int len = 0;

	stop_addr = stop_code;
	switch (cmd)
	{
	case 0x03:
	case 0x04:
		{
			send_buf[0] = stop_code;
			send_buf[1] = cmd;
			send_buf[2] = 0x00;
			send_buf[3] = start_addr;
			send_buf[4] = 0x00;
			send_buf[5] = count;
			crc = CRC32(send_buf, send_buf + 6);
			send_buf[6] = crc >> 8;
			send_buf[7] = (BYTE)crc;
			len = write(serial_fd, send_buf, 8);	
			if (-1 == len)
			{
				perror("send data by modbus error\n");
			}
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
**	FUNCTION NAME:    send_cmd_to_DO_serial
**   Owner: Jason Wang
**	DATE:		2012-04-26 
**	
**	DESCRIPTION:	
**		send cmd to DO module 
**	ARGUMENTS:	
**		stop_code: 要控制的DO 模块的站号
**		cmd: 发给DO 模块的modbus 命令
**		start_addr: 要控制的DO port 的起始地址
**		count: 要控制的DO port 的个数
**		value: 设置DO ports 为何种状态,0x05: value=0xff表示设置为高,value=0x00表示设置为低
**		
**	RETURNED VALUE:	
**		xx
**	NOTES:
**	
*/
void 
send_cmd_to_DO_serial(BYTE stop_code, BYTE cmd, BYTE start_addr, BYTE count, unsigned short value)
{
	BYTE send_buf[BUF_LEN] = {0};
	unsigned short crc = 0;
	int len = 0;
	int i = 0;
	BYTE *tmp = (BYTE *)&value;

	stop_addr = stop_code;
	switch (cmd)
	{
	case 0x05: // 写单个线圈
		{
			send_buf[0] = stop_code;
			send_buf[1] = cmd;
			send_buf[2] = 0x00;
			send_buf[3] = start_addr;
			if (COIL_ON == value)
			{
				send_buf[4] = 0xff;
			}
			else
			{
				send_buf[4] = 0x00;
			}
			send_buf[5] = 0x00;
			crc = CRC32(send_buf, send_buf + 6);
			send_buf[6] = crc >> 8;
			send_buf[7] = (BYTE)crc;	
			len = write(serial_fd, send_buf, 8);	
			if (-1 == len)
			{
				perror("send data by modbus error\n");
			}			

		}
		break;
	case 0x0f:
		{
			send_buf[0] = stop_code;
			send_buf[1] = cmd;	
			send_buf[2] = 0x00;
			send_buf[3] = start_addr;	
			send_buf[4] = 0x00;
			send_buf[5] = count;
			send_buf[6] = (count + 7) / 8; // 字节数
			for (i=0; i<send_buf[6]; i++)
			{
				send_buf[7+i] = tmp[i];
			}
			crc = CRC32(send_buf, send_buf + 7 + i);
			send_buf[7+i] = crc >> 8;
			send_buf[7+i+1] = (BYTE)crc;	
			len = write(serial_fd, send_buf, 9+i);	
			if (-1 == len)
			{
				perror("send data by modbus error\n");
			}				
		}
		break;	
	default:
		{

		}
		break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**	
**	FUNCTION NAME:    wait_slave_ack_serial
**   Owner: Jason Wang
**	DATE:		2012-04-26 
**	
**	DESCRIPTION:	
**		wait slave's response after send cmd by serial
**	ARGUMENTS:	
**		buf: response content from slave
**		buf_len: the buf's length
**		time: wait time out about response
**	RETURNED VALUE:	
**		response length from slave
**	NOTES:
**	
*/
unsigned int 
wait_slave_ack_serial(BYTE *buf, unsigned int buf_len, unsigned int time_out)
{
	fd_set rfd;
	struct timeval timeout;	
	unsigned int ack_len = 0;

	FD_ZERO(&rfd);	
	timeout.tv_sec = time_out;        
	timeout.tv_usec = 0;
	FD_SET(serial_fd, &rfd); 
	if(0 == select(serial_fd+1, &rfd, 0, 0, &timeout)) // 等待ack 超时
	{
		return ACK_TIME_OUT;
	}
	else
	{
		if(FD_ISSET(serial_fd, &rfd))
		{
			ack_len = read(serial_fd, buf, buf_len);	
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

BOOL 
analysis_ack_serial(BYTE *buf, unsigned int buf_len, unsigned int *value)
{
	BYTE addr = buf[0];
	BYTE cmd = buf[1];
	if (addr !=stop_addr)
	{
		return FALSE;
	}
	if ((cmd >> 7) & 1) // 0x8* indicate execption
	{
		printf("file:%s\tfunction:%s\tline:%d error\n", __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	switch (cmd)
	{
	case 0x01:
		{
			unsigned short byte_count = buf[2]; // value byte count
			memcpy((BYTE *)value, &buf[3], byte_count);
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


