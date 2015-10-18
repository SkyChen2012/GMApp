/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	FM1182.c
**
**	AUTHOR:		Alan Huang
**
**	DATE:		23 - Jan - 2007
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**							
**				
**				
**				
**	NOTES:
** 
*/

/************** SYSTEM INCLUDE FILES **************************************************/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXTypes.h"
#include "MXCommon.h"
#include "Dispatch.h"
#include "MXMem.h"
#include "MXList.h"
#include "MXMdId.h"
#include "MXMsg.h"
#include "Multimedia.h"
#include "BacpNetComm.h"
#include "PioApi.h"
#include "Eth.h"
#include "IniFile.h"
#include "MenuParaProc.h"
#include "FM1182.h"
/************** DEFINES **************************************************************/

#define FM1182_TEST			

#define FM1182_SERIAL_PORT			1//10" = 3, 7" = 2
#define FM1182_MAX_PARA_LEN			1024
#define FM1182_PARA_FILE_NAME		"/mox/rdwr/fm1182"

/************** TYPEDEFS *************************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static unsigned char  ucFM1182ParaDefault[] = {

                  0xFC,0xF3,0x3B,0x1E,0x3E,0x01,0x00, 

                  0xFC,0xF3,0x3B,0x1E,0xF,0x5A,0x0,       

                  0xFC,0xF3,0x3B,0x1E,0x0,0x78,0xFF,

                  0xFC,0xF3,0x3B,0x1E,0x1,0x20,0x00,        //           0xFC,0xF3,0x3B,0x1E,0x1,0x4f,0xff,

                  0xFC,0xF3,0x3B,0x1E,0x7,0x78,0xFF,

                  0xFC,0xF3,0x3B,0x1E,0x8,0x18,0x0,

                  0xFC,0xF3,0x3B,0x1E,0x30,0x3,0x3B,

                  0xFC,0xF3,0x3B,0x1E,0x34,0x0,0xCC,

                  0xFC,0xF3,0x3B,0x1E,0x38,0x00,0xF,

                  0xFC,0xF3,0x3B,0x1E,0x3D,0x03,0x0,

                  0xFC,0xF3,0x3B,0x1E,0x41,0x0,0x2,

                  0xFC,0xF3,0x3B,0x1E,0x44,0x1,0x41, //       0xFC,0xF3,0x3B,0x1E,0x44,0x1,0x45,

                  0xFC,0xF3,0x3B,0x1E,0x45,0x47,0xFC,// 0xFC,0xF3,0x3B,0x1E,0x45,0x4f,0xFC,

                  0xFC,0xF3,0x3B,0x1E,0x46,0x80,0x71,				  

                  0xFC,0xF3,0x3B,0x1E,0x47,0x30,0x00,

                  0xFC,0xF3,0x3B,0x1E,0x48,0x8,0x0,

                  0xFC,0xF3,0x3B,0x1E,0x49,0x8,0x0,

                  0xFC,0xF3,0x3B,0x1E,0x4D,0x3,0xA0,

//				  0xFC,0xF3,0x3B,0x1E,0x50,0x01,0x00,

                  0xFC,0xF3,0x3B,0x1E,0x52,0x0,0x17,

                  0xFC,0xF3,0x3B,0x1E,0x57,0x78,0xff, //  0xFC,0xF3,0x3B,0x1E,0x57,0x2e,0x00

				  0xFC,0xF3,0x3B,0x1E,0x5a,0x78,0xff, 

                  0xFC,0xF3,0x3B,0x1E,0x63,0x0,0x9,

                  0xFC,0xF3,0x3B,0x1E,0x70,0x7,0xC0,

                  0xFC,0xF3,0x3B,0x1E,0x86,0x0,0x6,

                  0xFC,0xF3,0x3B,0x1E,0x87,0x0,0x2,

                  0xFC,0xF3,0x3B,0x1E,0x88,0x0,0x20,

                  0xFC,0xF3,0x3B,0x1E,0x8B,0x12,0x0,

                  0xFC,0xF3,0x3B,0x1E,0x8C,0x3,0x80,

                  0xFC,0xF3,0x3B,0x1E,0x9A,0x3,0x0,

                  0xFC,0xF3,0x3B,0x1E,0x9C,0x1C,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xA8,0x1,0x50,

                  0xFC,0xF3,0x3B,0x1E,0xA9,0x2,0x0,    //  0xFC,0xF3,0x3B,0x1E,0xA9,0x1,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xC5,0x8,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xC6,0x1A,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xC7,0x1A,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xD4,0x10,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xD5,0x30,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xD6,0x20,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xD7,0x7,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xDA,0x21,0x00, // 0xFC,0xF3,0x3B,0x1E,0xDA,0x30,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xDB,0x10,0x1d, // 0xFC,0xF3,0x3B,0x1E,0xDB,0x21,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xDE,0x50,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xED,0x1C,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xEE,0x40,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xF4,0x40,0x1,

                  0xFC,0xF3,0x3B,0x1E,0xF7,0x0,0x0,

                  0xFC,0xF3,0x3B,0x1E,0xFC,0x0,0x80,

                  0xFC,0xF3,0x3B,0x1E,0xFE,0x0,0xA0,

                  0xFC,0xF3,0x3B,0x1E,0x3A,0x0,0x0

};


static int		serial_port_open(int nPort);
static int		LoadFM1182ParaFormFile(unsigned char* pPara, int nMaxLen, char* pFileName);
static int		ReadRegFm1182(int nFd, UCHAR* pRegAddr, UCHAR* pRegValue);

#ifdef FM1182_TEST
static void		PrintBuf(char*pDesc, unsigned char* pBuff, int nLen);
static int		FM1182Test(int nFd);
#endif

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendtoFM1182
**	AUTHOR:			Alan Huang / Jerry Huang
**	DATE:			23 - Jan - 2007
**
**	DESCRIPTION:	
**			Set FM1182 parameters
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pBuffer		[IN]		UCHAR*
**				nLen		[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
int
SendtoFM1182(UCHAR* pBuffer, int nLen)
{
	int					nFd;
	unsigned char*		pPara = NULL;
	int					nParaLen = 0;
	unsigned char		ucPara[FM1182_MAX_PARA_LEN];
	int ret = 0;
	unsigned char hwVer[3];	

	unsigned char		ucReset[7] = {0xFC,0xF3,0x3B,0x3F,0xE4,0x0,0x1D};
	int i = 0;
	
	nFd = serial_port_open(FM1182_SERIAL_PORT);
	if (-1  == nFd)
	{
		printf("Open serial port (FM1182) fail 1\n");
		return;
	}

	// Load parameters from file
	nParaLen = LoadFM1182ParaFormFile(ucPara, FM1182_MAX_PARA_LEN, FM1182_PARA_FILE_NAME);
	
	//for (i = 0; i < nParaLen; i++) printf("%x ",ucPara[i]);
	//printf("\n");

	// If load parameters from file fail, use default parameters
	if (nParaLen <= 0)
	{
		pPara = ucFM1182ParaDefault;
		nParaLen = sizeof (ucFM1182ParaDefault);
		printf("Use default FM1182 param, %d\n", nParaLen);
	}
	else
	{
		pPara = ucPara;
	}

	write(nFd, pPara, nParaLen);

	usleep(1000 * 1000);

#ifdef FM1182_TEST
/*	hwVer[0] = g_FactorySet.Hardver[1] - 0x30;
	hwVer[1] = 10 * (g_FactorySet.Hardver[3] - 0x30) + 
			(g_FactorySet.Hardver[4] - 0x30);
	hwVer[2] = 10 * (g_FactorySet.Hardver[6] - 0x30) + 
			(g_FactorySet.Hardver[7] - 0x30);
	printf("************* HWver: %d, %d, %d\n", hwVer[0], hwVer[1], hwVer[2]);
	if ((hwVer[0] >= 2) && (hwVer[1] >= 10)) 
	{
		ret = FM1182Test(nFd);
	}
	else ret = 0;
*/	
	ret = FM1182Test(nFd);
#endif

	printf("Write FM1182 param %d finished, OK\n", nParaLen);
	close(nFd);

	return ret;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadRegFm1182
**	AUTHOR:			Alan Huang / Jerry Huang
**	DATE:			23 - Jan - 2007
**
**	DESCRIPTION:	
**			Set FM1182 parameters
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pBuffer		[IN]		UCHAR*
**				nLen		[IN]		int
**	RETURNED VALUE:	
**				int, 0:OK, -1 fail
**	NOTES:
**			
*/
int
ReadRegFm1182(int nFd, UCHAR* pRegAddr, UCHAR* pRegValue)
{
	UCHAR	ucReadData1[] = {0xFC, 0xF3, 0x37, 0x00, 0x00};
	UCHAR	ucReadData2[] = {0xFC, 0xF3, 0x60, 0x26};
	UCHAR	ucReadData3[] = {0xFC, 0xF3, 0x60, 0x25};
	UCHAR	ucReadBuffer[128];
	int		nRead = 0;
	DWORD	dwTick = GetTickCount();
	int		nRet = 0;

	if (-1  == nFd)
	{
		printf("indvalid FD (FM1182) fail\n");
		return -1;
	}
	ucReadData1[3] = pRegAddr[0];
	ucReadData1[4] = pRegAddr[1];
	write(nFd, ucReadData1, 5);
	write(nFd, ucReadData2, 4);
	write(nFd, ucReadData3, 4);
	usleep(1 * 1000);
	while(1)
	{
		nRead = read(nFd,ucReadBuffer,12);
		if (nRead > 0)
		{
			printf("Read FM1182 param finished len%d , %x, %x\n", nRead, ucReadBuffer[0], ucReadBuffer[1]);
			pRegValue[0] = ucReadBuffer[0];
			pRegValue[1] = ucReadBuffer[1];
			break;
		}
		usleep(1);
		if (GetTickCount() - dwTick > 300)
		{
			printf("no data\n");
			nRet = -1;
			break;
		}
	}
	close(nFd);
	return nRet;
	

//	
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SetFM1182SpkVolume
**	AUTHOR:			Alan Huang
**	DATE:			29 - Feb - 2008
**
**	DESCRIPTION:	
**			Set FM1182 parameters-- speak volume
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pBuffer		[IN]		UCHAR*
**				nLen		[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
SetFM1182SpkVolume(USHORT nVolume, int nMode)
{
	int					nFd;

	int					nParaLen = 7;
	unsigned char		ucPara[]={0xFC, 0xF3, 0x3B, 0x1E, 0x3E, 0x02, 0x00}; //SPK VOL		
	int					i;
	USHORT				nParamRing[]={0x0010, 0x0030, 0X0050, 0x0070, 0x00A0, 0x0C0, 0x00E0};
	USHORT				nParamTalk[]={0x0020, 0x0040, 0x0070, 0X0090, 0x00B0, 0x00F0, 0x0100};

	USHORT	nData = 0;
	USHORT nLen = 0;


	nVolume = nVolume / 10 - 3; //normalize to 0-6, pre value is 30, 40,...90.

	if (0 == nMode)
	{
		ucPara[5] = (nParamRing[nVolume] & 0xFF00) >> 8;
		ucPara[6] = nParamRing[nVolume] & 0x00FF;
	}
	else
	{
		ucPara[5] = (nParamRing[nVolume] & 0xFF00) >> 8;
		ucPara[6] = nParamRing[nVolume] & 0x00FF;
	}
	

	nFd = serial_port_open(FM1182_SERIAL_PORT);
	if (-1  == nFd)
	{
		printf("Open serial port (FM1182) fail\n");
		return;
	}


	write(nFd, ucPara, nParaLen);

	usleep(1 * 1000);
//	printf("Write FM1182 param finished %x, %x\n", ucPara[5], ucPara[6]);	

//	ReadFM1182Reg(nData, &nLen);

	close(nFd);

}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SetFM1182SpkVolume
**	AUTHOR:			Alan Huang
**	DATE:			29 - Feb - 2008
**
**	DESCRIPTION:	
**			Set FM1182 parameters-- speak volume
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pBuffer		[IN]		UCHAR*
**				nLen		[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
SetMicVolForHdb()
{
	int nMic = 47; 

	int					nFd;

	int					nParaLen = 7;
	unsigned char		ucPara[]= {0xFC,0xF3,0x3B,0x1E,0x8,0x40,0x0};


	nFd = serial_port_open(FM1182_SERIAL_PORT);
	if (-1  == nFd)
	{
		printf("Open serial port (FM1182) fail\n");
		return;
	}


	write(nFd, ucPara, nParaLen);

	usleep(1 * 1000);	

	close(nFd);

	//AudioUsbMicVolumeAdjest(nMic);

}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SetFM1182SpkVolume
**	AUTHOR:			Alan Huang
**	DATE:			29 - Feb - 2008
**
**	DESCRIPTION:	
**			Set FM1182 parameters-- speak volume
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pBuffer		[IN]		UCHAR*
**				nLen		[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
RetMicVolForHdb()
{
	int					nFd;

	int					nParaLen = 7;
	unsigned char		ucPara[]= {0xFC,0xF3,0x3B,0x1E,0x8,0x18,0x0};
	

	nFd = serial_port_open(FM1182_SERIAL_PORT);
	if (-1  == nFd)
	{
		printf("Open serial port (FM1182) fail 3 \n");
		return;
	}


	write(nFd, ucPara, nParaLen);

	usleep(1 * 1000);
	
	printf("HDB Write FM1182 param finished %x, %x\n", ucPara[5], ucPara[6]);

	close(nFd);

	//AudioUsbMicVolumeAdjest(34);

 }
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	serial_port_open
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
static int 
serial_port_open(int nPort)
{
	char			tcPort[16];
	struct termios	oldtio;
	struct termios	newtio;	
	int				nFd = -1;
	
	sprintf(tcPort, "/dev/ttymxc%d", nPort);

	nFd = open(tcPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (nFd < 0) 
	{
		return -1;
	}
	
	/* save current port settings */
	tcgetattr(nFd , &newtio); 
	
	newtio.c_iflag		= IGNPAR;
	newtio.c_oflag		= 0;
	newtio.c_lflag		= 0;
	newtio.c_cc[VTIME]	= 0;
	newtio.c_cc[VMIN]	= 1;
	newtio.c_cflag		= B9600 | 
		0 |
		0 |
		0 |
		CS8 |
		CLOCAL | CREAD;
	newtio.c_iflag &=~(INLCR|IGNCR|ICRNL);
    newtio.c_oflag &=~(ONLCR|OCRNL);
	
    tcflush(nFd  , TCIFLUSH);
    tcsetattr(nFd, TCSANOW, &newtio);
	
	return nFd;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	LoadFM1182ParaFormFile
**	AUTHOR:			Jerry Huang
**	DATE:			23 - Jan - 2007
**
**	DESCRIPTION:	
**			Load FM1182 parameters frome file
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pPara		[OUT]		unsigned char*
**				nMaxLen		[IN]		int
**				pFileName	[IN]		char*
**	RETURNED VALUE:	
**				parameters length
**	NOTES:
**			
*/
static int
LoadFM1182ParaFormFile(unsigned char* pPara, int nMaxLen, char* pFileName)
{
	int		nLen = 0;
	FILE*	f;

	if ((f = fopen(pFileName, "rb")) == NULL)
	{
		return 0;
	}

	nLen = fread(pPara, 1, nMaxLen, f);
	fclose(f);
	
	return nLen;
}

#ifdef FM1182_TEST

static void
PrintBuf(char*pDesc, unsigned char* pBuff, int nLen)
{
	int	nLoop;

	printf("%s: ", pDesc);
	for (nLoop = 0; nLoop < nLen; nLoop++)
	{
		printf("%02X ", pBuff[nLoop]);
	}
	printf("\n");
}

static int
FM1182Test(int nFd)
{
#define CMD_1_LEN			5
//#define CMD_2_LEN			5
#define CMD_3_LEN			4
#define CMD_4_LEN			4

	unsigned char	ucCmd1[CMD_1_LEN] = {0xFC, 0xF3, 0x37, 0x1E, 0x65};
//	unsigned char	ucCmd2[CMD_2_LEN] = {0xFC, 0xF3, 0x37, 0x1E, 0x3D};
	unsigned char	ucCmd3[CMD_3_LEN] = {0xFC, 0xF3, 0x60, 0x25};
	unsigned char	ucCmd4[CMD_4_LEN] = {0xFC, 0xF3, 0x60, 0x26};

	unsigned short	ucRet1;
	unsigned short	ucRet2;
	int nRetLen;

	write(nFd, ucCmd1, CMD_1_LEN);
	write(nFd, ucCmd3, CMD_3_LEN);
	write(nFd, ucCmd4, CMD_4_LEN);
//	PrintBuf("FM1182 Commond", ucCmd1, CMD_1_LEN);
//	PrintBuf("FM1182 Commond", ucCmd3, CMD_3_LEN);
//	PrintBuf("FM1182 Commond", ucCmd4, CMD_4_LEN);

	usleep(1000 * 1000);
	
	nRetLen = read(nFd, &ucRet1, 2);
	
	if (2 != nRetLen) return -1;
	
	usleep(1000 * 1000);
	
	write(nFd, ucCmd1, CMD_1_LEN);
	write(nFd, ucCmd3, CMD_3_LEN);
	write(nFd, ucCmd4, CMD_4_LEN);
//	PrintBuf("FM1182 Commond", ucCmd1, CMD_1_LEN);
//	PrintBuf("FM1182 Commond", ucCmd3, CMD_3_LEN);
//	PrintBuf("FM1182 Commond", ucCmd4, CMD_4_LEN);

	usleep(1000 * 1000);
	
	nRetLen = read(nFd, &ucRet2, 2);
	
	if (2 != nRetLen) return -1;	

	if (ucRet2 == ucRet1) return -1;
	else return 0;

//	PrintBuf("FM1182 Return", ucRet, nRetLen);
}


#endif

int	ReadFM1182Reg(USHORT nAddr, USHORT* nValue)
{
	int					nFd;
	
	nFd = serial_port_open(FM1182_SERIAL_PORT);
	if (-1  == nFd)
	{
		printf("Open serial port (FM1182) fail\n");
		return 0;
	}
	
	ReadRegFm1182(nFd, (UCHAR*)&nAddr, (UCHAR*)nValue);	
	
	printf("read FM1182 param finished %x, %x\n", nAddr, *nValue);
	
	close(nFd);
	return 0;	
}

