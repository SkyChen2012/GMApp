
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	CardRead.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		13 - April - 2009
**
**	FILE DESCRIPTION:
**
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

#include "MXTypes.h"
#include "MXCommon.h"
#include "RS485.h"
#include "BacpNetCtrl.h"

/************** DEFINES **************************************************************/
#define LC_MOX_DEBUG	

/************** TYPEDEFS *************************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static INT nFdRs485_1 = 0;
static INT nFdRs485_2 = 0;
static INT	nFdLF_mox = 0;
static INT	nFdLF_Mitsubishi = 0;
static INT Rs485PortOpen(INT nPort);
static INT LFCOMPortOpen(int nParity);
static INT LFCOMPortMitsubishi(void);

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadCardInit
**	AUTHOR:		Jeff Wang
**	DATE:			10 - April - 2009
**
**	DESCRIPTION:	
**			Read Card Number
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pOutBuf		[IN]		CHAR*		Buffer
**	RETURNED VALUE:	
**				read length
**	NOTES:
**			
*/
VOID
Rs485Init(VOID)
{
	int mode = GetLCMode();
	switch(mode)
	{
	case LC_MOX_MODE_V1:
	case LC_MITSUBISHI_MODE:
		nFdLF_mox = LFCOMPortOpen(PARITY_EVEN);
		break;
	case LC_MOX_MODE_V2:
		nFdLF_mox = LFCOMPortOpen(PARITY_NONE);
		break;

	case LC_HITACHI_MODE:
		nFdRs485_1 = Rs485PortOpen(RS485_PORT_1);
		nFdRs485_2 = Rs485PortOpen(RS485_PORT_2);
		printf("nFdRs485_1 = %d, nFdRs485_2 = %d\n", nFdRs485_1, nFdRs485_2);
		break;

	default:
		break;
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Write485Data
**	AUTHOR:			Jeff Wang
**	DATE:			10 - April - 2009
**
**	DESCRIPTION:	
**			Write to 485 port
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pOutBuf		[IN]		CHAR*		Buffer
**	RETURNED VALUE:	
**				NONE
**	NOTES:
**			
*/
void 
ClearRcv()
{
	int n = 512;
	char chBuff[1024];
	n = read(nFdRs485_2, chBuff, n);
	if (n >0)
	{
		printf("drop n %d\n", n);
	}
}

void 
ClearLCCOMRcv()
{
	int n = 512;
	char chBuff[1024];
	n = read(nFdLF_mox, chBuff, n);
	if (n >0)
	{
		printf("drop n %d\n", n);
	}
}



VOID
Write485Data(CHAR* pOutBuf, unsigned int Length)
{
	int nWriteLen = 0;
	int i;
	
	//ClearRcv();

	nWriteLen = write(nFdRs485_1, pOutBuf, Length);
	if (-1 == nWriteLen) 
	{
		printf("***1 485 Port Write Error***\n");
	}
		
	printf("send\n");

	for (i = 0; i< nWriteLen; i++)
	{
		printf("%x ", pOutBuf[i]);
	}
	printf("\n");
}


VOID
WriteMox485Data(CHAR* pOutBuf, unsigned int Length)
{
	int nWriteLen = 0;
//	int i;
	
	ClearLCCOMRcv();

	nWriteLen = write(nFdLF_mox, pOutBuf, Length);
	if (-1 == nWriteLen) 
	{
		printf("*** 485 Port nFdLF_mox  Write Error***\n");
	}
	/*if(nWriteLen>6)
	{
		for (i = 0; i< nWriteLen; i++)
		{
			printf("%02x ", pOutBuf[i]);
		}
		printf("\n");
	}*/

}



VOID
Write485Data_temp(CHAR* pOutBuf, unsigned int Length)
{
	int nWriteLen = 0;
	char szTemp[] = "0";

	nWriteLen = write(nFdRs485_2, szTemp, strlen(szTemp));
	if (-1 == nWriteLen) 
	{
		printf("***2 485 Port Write Error***\n");
	}	
}

int
Read485Data(unsigned char* pInBuf, unsigned int Length)
{
	int nReadLen = 0;
	int i = 0;
	int nDataLen = 0;
	UINT nReadCnt = 0;

	while (nDataLen < Length && nReadCnt < 50)
	{
		nReadLen = read(nFdRs485_2, &pInBuf[nDataLen], Length - nDataLen);
		nDataLen += nReadLen;
		nReadCnt++;
	}
    
	
	if(nDataLen>0)
	{
        printf("RX:");
        for (i = 0; i< nDataLen; i++)
        {
            printf("%x ", pInBuf[i]);
        }
		//printf("nDataLen=%d\n",nDataLen);
        printf("\n");
	}
	//ClearRcv();

	return nDataLen;
}

int
ReadMox485Data(unsigned char* pInBuf, unsigned int Length)
{
	int nReadLen = 0;
//	int i = 0;
	int nDataLen = 0;
	UINT nReadCnt = 0;
 
	if (nFdLF_mox == 0)
	{
		return 0 ;
	}
	while (nDataLen < Length && nReadCnt < 1)
	{
		nReadLen = read(nFdLF_mox, &pInBuf[nDataLen], Length - nDataLen);
		if (nReadLen > 0)
		{
			nDataLen += nReadLen;
		}

		nReadCnt++;
	}

	/*if (nDataLen > 0)
	{
		printf("read the comm data nLen=%d:\n", nDataLen);
		for (i = 0; i < nDataLen; i++)
		{
			printf("%02x ", pInBuf[i]);
		}
		printf("\n");
	}*/

	return nDataLen;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LFCOMPortMitsubishi
**	AUTHOR:			Harry Qian
**	DATE:			12 - Nov - 2010
**
**	DESCRIPTION:	
**			Open serial port
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				
**	RETURNED VALUE:	
**				port handle if success, otherwise -1
**	NOTES:
**			
*/
static INT 
LFCOMPortMitsubishi(void)
{
	char			tcPort[16];
	struct termios	oldtio;
	struct termios	newtio;	
	int				nFd = -1;
	
	sprintf(tcPort, "/dev/ttymxc%d", 1);
	

	nFd = open(tcPort, O_RDWR |O_NOCTTY | O_NONBLOCK);


	if (nFd < 0) 
	{
		printf("Uart Open Error\n");
		return -1;
	}
	
	tcgetattr(nFd,&newtio); 
	
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
	
	
	tcflush(nFd, TCIFLUSH);
	//tcflush(nFd, TCIOFLUSH);

	tcsetattr(nFd, TCSANOW, &newtio);	
	return nFd;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Rs485PortOpen
**	AUTHOR:			Alan Huang / Jerry Huang
**	DATE:			23 - Jan - 2007
**
**	DESCRIPTION:	
**			Open serial port
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nPort		[IN]		int		port number
**	RETURNED VALUE:	
**				port handle if success, otherwise -1
**	NOTES:
**			
*/
static INT 
Rs485PortOpen(INT nPort)
{
	char			tcPort[16];
	struct termios	oldtio;
	struct termios	newtio;	
	int				nFd = -1;
	
	sprintf(tcPort, "/dev/ttymxc%d", nPort);
	

	nFd = open(tcPort, O_RDWR |O_NOCTTY | O_NONBLOCK);


	if (nFd < 0) 
	{
		printf("Uart Open Error\n");
		return -1;
	}
	
	tcgetattr(nFd,&newtio); 
	
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
	tcflush(nFd, TCIFLUSH);
	//tcflush(nFd, TCIOFLUSH);

	tcsetattr(nFd, TCSANOW, &newtio);	
	return nFd;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LFCOMPortOpen
**	AUTHOR:			Harry Qian
**	DATE:			3 - Nov - 2010
**
**	DESCRIPTION:	
**			Open serial port
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nPort		[IN]		int		port number
**	RETURNED VALUE:	
**				port handle if success, otherwise -1
**	NOTES:
**			
*/
static INT 
LFCOMPortOpen(int nParity)
{
	char			tcPort[16];
	struct termios	oldtio;
	struct termios	newtio;	
	int				nFd = -1;
	int				nPort = 0;
	int				nModeType = 0;

	OpenIniFile(LF_SERIAL_INI_FILE);

	nPort = ReadInt(SEC_LCA, KEY_COMPORT, 1);
	
	if (1 == nPort)
	{
		sprintf(tcPort, "/dev/ttymxc%d", 3);
	}
	else if (2 == nPort)
	{
		sprintf(tcPort, "/dev/ttymxc%d", 1);
	}
	else
	{
		printf("%s: Error, the EHV only have two com port.\n", __FUNCTION__);
		CloseIniFile();
		return 0;
	}

#ifdef LC_MOX_DEBUG
	printf("LC mox mode open the com%d.\n", nPort);
#endif


	nFd = open(tcPort, O_RDWR |O_NOCTTY | O_NONBLOCK);


	if (nFd < 0) 
	{
		printf("Uart Open Error\n");
		CloseIniFile();
		return -1;
	}
	
	tcgetattr(nFd,&newtio); 
	
	nModeType = ReadInt(SEC_LCA, KEY_BAUDRATE, DEFAULT_BAUDRATE);

#ifdef LC_MOX_DEBUG
	printf("LC mox mode nFd=%d.\n", nFd);
	printf("LC mox mode set the baudrate=%d.\n", nModeType);
#endif

	switch(nModeType)
	{
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 19200:
		cfsetispeed(&newtio, B19200);
		cfsetospeed(&newtio, B19200);
		break;
	case 38400:
		cfsetispeed(&newtio, B38400);
		cfsetospeed(&newtio, B38400);
		break;
	case 57600:
		cfsetispeed(&newtio, B57600);
		cfsetospeed(&newtio, B57600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	default:
		printf("the modetype beyond the area. so set baudrate = 9600.\n");
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}

	nModeType = ReadInt(SEC_LCA, KEY_DATABIT, DEFAULT_DATABIT);

#ifdef LC_MOX_DEBUG
	printf("LC mox mode set the databit=%d.\n", nModeType);
#endif

	newtio.c_cflag &= ~CSIZE; /* Mask the character size bits */
	switch(nModeType)
	{
	case 5:
		newtio.c_cflag |= CS5;    /* Select 5 data bits */	
		break;
	case 6:
		newtio.c_cflag |= CS6;    /* Select 6 data bits */	
		break;
	case 7:
		newtio.c_cflag |= CS7;    /* Select 7 data bits */	
		break;
	case 8:
		newtio.c_cflag |= CS8;    /* Select 8 data bits */	
		break;
	default:
		printf("the data bit set to 8.\n");
		newtio.c_cflag |= CS8;    /* Select 8 data bits */	
		break;
	}


	nModeType = ReadInt(SEC_LCA, KEY_PARITY, nParity);
	//nModeType = nParity;
#ifdef LC_MOX_DEBUG
	printf("LC mox mode set the parity=%d.\n", nModeType);
#endif

	switch(nModeType)
	{
	case 0:
		newtio.c_cflag &= ~PARENB; //no parity
		break;
	case 1:						// odd parity
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		break;
	case 2:						// even parity
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	default:
		printf("the parity set to 0.\n");
		newtio.c_cflag &= ~PARENB;//mode:8N1
		break;
	}


	nModeType = ReadInt(SEC_LCA, KEY_STOPBIT, DEFAULT_STOPBIT);

#ifdef LC_MOX_DEBUG
	printf("LC mox mode set the stop bit=%d.\n", nModeType);
#endif

	switch(nModeType)
	{
	case 0:
		newtio.c_cflag &= ~CSTOPB;
		break;
	case 1:
		
		break;
	case 2:
		newtio.c_cflag |= CSTOPB;
		break;
	default:
		printf("the parity set to 0.\n");
		newtio.c_cflag &= ~CSTOPB;
		break;
	}
	
	CloseIniFile();

	newtio.c_cflag |= (CLOCAL | CREAD);

	newtio.c_cflag &= ~CRTSCTS; /*disable hardware flow control*/
	newtio.c_iflag &= ~(IXON | IXOFF | IXANY); /*disable software flow control */
	
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /*Input*/
	newtio.c_oflag &= ~OPOST; /*Output*/
	newtio.c_iflag &=~(INLCR|IGNCR|ICRNL);
    newtio.c_oflag &=~(ONLCR|OCRNL);
	
	newtio.c_cc[VTIME] = 0;//timeout = 2seconds
	newtio.c_cc[VMIN] = 0;
	tcflush(nFd, TCIFLUSH);
	//tcflush(nFd, TCIOFLUSH);

	tcsetattr(nFd, TCSANOW, &newtio);	
	return nFd;
}


