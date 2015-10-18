/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	PIOApi.c
**
**	AUTHOR:		Jerry Huang
**
**	DATE:		03 - Nov - 2006
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <errno.h>
#include <sys/timeb.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>

/************** USER INCLUDE FILES ***************************************************/

#include "mox_pio.h"
#include "PioApi.h"
#include "MenuParaProc.h"

/************** DEFINES **************************************************************/
//#define		PIOAPI_DEBUG
//#define	INSERT_PIO_MODULE		// if PIO module is build in kernel, disable the macro
//#define	PIO_DIDO_TEST

/************** TYPEDEFS *************************************************************/

#define DO1_PORT							MOX_PIOF
#define DO1_PIN									MOX_PIO_PIN16
#define DO2_PORT							MOX_PIOF
#define DO2_PIN									MOX_PIO_PIN18

#define DI1_PORT								MOX_PIOE
#define DI1_PIN										MOX_PIO_PIN22
#define DI2_PORT								MOX_PIOE
#define DI2_PIN									   MOX_PIO_PIN18

#define UNLOCK_PORT				        MOX_PIOF
#define UNLOCK_PIN					       MOX_PIO_PIN11

#define TAMPER_ALARM_PORT	  MOX_PIOF
#define TAMPER_ALARM_PIN	 	 MOX_PIO_PIN10

#define PHOTOSENSITIVE_PORT	MOX_PIOF
#define PHOTOSENSITIVE_PIN		MOX_PIO_PIN17


#define CCD_LED_PORT					MOX_PIOB
#define CCD_LED_PIN						   MOX_PIO_PIN27

#define KEY_LED_PORT					 MOX_PIOC
#define KEY_LED_PIN							MOX_PIO_PIN21

#define CCD_POWER_PORT						MOX_PIOF
#define CCD_POWER_PIN						   MOX_PIO_PIN13

#define OLED_POWER_PORT						MOX_PIOC
#define OLED_POWER_PIN						   MOX_PIO_PIN19

#define BEEP_CONTROL_PORT						MOX_PIOC
#define BEEP_CONTROL_PIN						   MOX_PIO_PIN23

#define HID_HOLD_PORT						MOX_PIOF
#define HID_HOLD_PIN						   MOX_PIO_PIN19

#define AMP_PWR_PORT					MOX_PIOE
#define AMP_PWR_PIN						MOX_PIO_PIN20

#define RESET_SWITCH_PORT				MOX_PIOF
#define RESET_SWITCH_PIN				MOX_PIO_PIN8


/************** STRUCTURES ***********************************************************/

typedef	struct _HPIManStruct
{
	int					nFd;
} PIOManStruct;

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static PIOManStruct			PIOMan				= 
{
	nFd: -1,
};

#ifdef INSERT_PIO_MODULE
static void			PIOInsDrv();
static void			PIORmDrv();
#endif

static void			PIOOpenInput(int nFd, unsigned char ucPort, unsigned int nPin);
static void			PIOCloseInput(int nFd, unsigned char ucPort, unsigned int nPin);
static int			PIOReadRaw(int nFd, unsigned char ucPort, unsigned int nPin);
static void			PIOOpenOutput(int nFd, unsigned char ucPort, unsigned int nPin, unsigned char ucValue);
static void			PIOCloseOutput(int nFd, unsigned char ucPort, unsigned int nPin, unsigned char ucValue);
static void			PIOWriteRaw(int nFd, unsigned char ucPort, unsigned int nPin, unsigned char ucValue);

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIOInsDrv
**	AUTHOR:			Jerry Huang
**	DATE:			03 - Nov - 2006
**
**	DESCRIPTION:	
**			Insert PIO driver
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
PIOInsDrv()
{
	char* const		argv[]	= {"insmod", "-f", "pio.o", 0};
	int				pid;
	int				status;

	pid = fork();
	if (pid == -1)
	{	
		return;
	}
	if (pid == 0)
	{
		execv("/sbin/insmod", argv);
		exit(127);
	}

	do 
	{
		if (waitpid(pid, &status, 0) == -1)
		{
			if (errno != EINTR)
			{
				return;
			}
		}
		else
		{
			break;
		}
	} while (1);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIORmDrv
**	AUTHOR:			Jerry Huang
**	DATE:			03 - Nov - 2006
**
**	DESCRIPTION:	
**			Remove PIO driver
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
PIORmDrv()
{
	char* const		argv[]	= {"rmmod", "pio", 0};
	int				pid;
	int				status;

	pid = fork();
	if (pid == -1)
	{	
		return;
	}
	if (pid == 0)
	{
		execv("/sbin/rmmod", argv);
		exit(127);
	}

	do 
	{
		if (waitpid(pid, &status, 0) == -1)
		{
			if (errno != EINTR)
			{
				return;
			}
		}
		else
		{
			break;
		}
	} while (1);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIOInit
**	AUTHOR:			Jerry Huang
**	DATE:			03 - Nov - 2006
**
**	DESCRIPTION:	
**			Initalize PIO
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				0 if succeed, otherwise -1
**	NOTES:
**		04 - Dec - 2006: CRT off	
**		13 - Sep - 2007: enable AIC23		
*/
int
PIOInit()
{
#ifdef INSERT_PIO_MODULE
	PIOInsDrv();
#endif

	if ((PIOMan.nFd = open(PIO_DEV_NAME, O_RDWR)) == -1)
	{
		printf("open fail\n");
#ifdef INSERT_PIO_MODULE
		PIORmDrv();
#endif
		return -1;
	}

#ifdef PIO_DIDO_TEST
	PIOOpenOutput(PIOMan.nFd, DO1_PORT, DO1_PIN, 1);
	PIOOpenOutput(PIOMan.nFd, DO2_PORT, DO2_PIN, 1);
	
	usleep(5*1000*1000);
	
	PIOOpenOutput(PIOMan.nFd, DO1_PORT, DO1_PIN, 0);
	PIOOpenOutput(PIOMan.nFd, DO2_PORT, DO2_PIN, 0);
#endif
	
	PIOWrite(PIO_TYPE_OLED_POWER, 0);

	return 0;
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIOExit
**	AUTHOR:			Jerry Huang
**	DATE:			03 - Nov - 2006
**
**	DESCRIPTION:	
**			Exit PIO
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
PIOExit()
{
	PIOCloseOutput(PIOMan.nFd, UNLOCK_PORT, UNLOCK_PIN, 0);
	PIOCloseOutput(PIOMan.nFd, DO1_PORT, DO1_PIN, 0);
	PIOCloseOutput(PIOMan.nFd, DO2_PORT, DO2_PIN, 0);

	if (PIOMan.nFd != -1)
	{
		close(PIOMan.nFd);
		PIOMan.nFd = -1;
	}

#ifdef INSERT_PIO_MODULE
	PIORmDrv();
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIOOpenInput
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			Open PIO for input
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nFd			[IN]		int
**				ucPort		[IN]		unsigned char
**				nPin		[IN]		unsigned int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
PIOOpenInput(int nFd, unsigned char ucPort, unsigned int nPin)
{
	PIOOpenParam	OpenP;

	if (nFd != -1)
	{
		OpenP.nPin = nPin;
		OpenP.ucPort = ucPort;
		OpenP.ucFun = MOX_PIO_FUN_INPUT;
		ioctl(nFd, PIO_IOC_OPEN, &OpenP);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIOCloseInput
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			close PIO for input
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nFd			[IN]		int
**				ucPort		[IN]		unsigned char
**				nPin		[IN]		unsigned int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
PIOCloseInput(int nFd, unsigned char ucPort, unsigned int nPin)
{
	PIOCloseParam	CloseP;

	if (nFd != -1)
	{
		CloseP.nPin = nPin;
		CloseP.ucPort = ucPort;
		CloseP.ucFun = MOX_PIO_FUN_INPUT;
		ioctl(nFd, PIO_IOC_CLOSE, &CloseP);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIOReadRaw
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			Read PIO (raw mode)
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nFd			[IN]		int
**				ucPort		[IN]		unsigned char
**				nPin		[IN]		unsigned int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static int
PIOReadRaw(int nFd, unsigned char ucPort, unsigned int nPin)
{
	int				nStatus	= -1;
	PIOReadParam	ReadP;

	if (nFd != -1)
	{
		ReadP.ucPort = ucPort;
		ReadP.nPin = nPin;
		ioctl(nFd, PIO_IOC_READ, &ReadP);
		nStatus = ReadP.ucValue;
	}

	return nStatus;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIORead
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			Read PIO
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nType		[IN]		int
**	RETURNED VALUE:	
**				status
**	NOTES:
**		14 - Sep - 2007: Add DI1,2 		
*/
int
PIORead(int nType)
{
	int	nStatus	= -1;

	switch (nType)
	{
	case PIO_TYPE_PHOTOSENSITIVE:
		nStatus = PIOReadRaw(PIOMan.nFd, PHOTOSENSITIVE_PORT, PHOTOSENSITIVE_PIN);
		break;

	case PIO_TYPE_DI1:
		nStatus = PIOReadRaw(PIOMan.nFd, DI1_PORT, DI1_PIN);
		break;

	case PIO_TYPE_DI2:
		nStatus = PIOReadRaw(PIOMan.nFd, DI2_PORT, DI2_PIN);
		break;

	case PIO_TYPE_TAMPER_ALARM:
		nStatus = PIOReadRaw(PIOMan.nFd, TAMPER_ALARM_PORT, TAMPER_ALARM_PIN);
		break;

	case RESET_SWITCH_DI:
		nStatus = PIOReadRaw(PIOMan.nFd, RESET_SWITCH_PORT, RESET_SWITCH_PIN);
		break;


	default:
		break;
	}

	return nStatus;
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIOOpenOutput
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			Open PIO for output
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nFd			[IN]		int
**				ucPort		[IN]		unsigned char
**				nPin		[IN]		unsigned int
**				ucValue		[IN]		unsigned char
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
PIOOpenOutput(int nFd, unsigned char ucPort, unsigned int nPin, unsigned char ucValue)
{
	PIOOpenParam	OpenP;

	if (nFd != -1)
	{
		OpenP.nPin = nPin;
		OpenP.ucPort = ucPort;
		OpenP.ucFun = MOX_PIO_FUN_OUTPUT;
		OpenP.ucValue = ucValue;
		ioctl(nFd, PIO_IOC_OPEN, &OpenP);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIOCloseOutput
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			close PIO for output
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nFd			[IN]		int
**				ucPort		[IN]		unsigned char
**				nPin		[IN]		unsigned int
**				ucValue		[IN]		unsigned char
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
PIOCloseOutput(int nFd, unsigned char ucPort, unsigned int nPin, unsigned char ucValue)
{
	PIOCloseParam	CloseP;

	if (nFd != -1)
	{
		CloseP.nPin = nPin;
		CloseP.ucPort = ucPort;
		CloseP.ucFun = MOX_PIO_FUN_OUTPUT;
		CloseP.ucValue = ucValue;
		ioctl(nFd, PIO_IOC_CLOSE, &CloseP);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIOWriteRaw
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			write PIO (raw mode)
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nFd			[IN]		int
**				ucPort		[IN]		unsigned char
**				nPin		[IN]		unsigned int
**				ucValue		[IN]		unsigned char
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
PIOWriteRaw(int nFd, unsigned char ucPort, unsigned int nPin, unsigned char ucValue)
{
	PIOWriteParam	WriteP;

	if (nFd != -1)
	{
		WriteP.nPin = nPin;
		WriteP.ucPort = ucPort;
		WriteP.ucValue = ucValue;
		ioctl(nFd, PIO_IOC_WRITE, &WriteP);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	PIOWrite
**	AUTHOR:			Jerry Huang
**	DATE:			14 - Nov - 2006
**
**	DESCRIPTION:	
**			write PIO
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				nType		[IN]		int
**				ucValue		[IN]		unsigned char
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
PIOWrite(int nType, unsigned char ucValue)
{
	switch (nType)
	{
	case PIO_TYPE_UNLOCK:
		if (SERIES_SHELL == g_DevFun.SeriesType || SERIES_MINI_SHELL == g_DevFun.SeriesType) 
		{
#ifdef	PIOAPI_DEBUG
			printf("%s,PIO_TYPE_UNLOCK type:%d,value:%d\n",__FUNCTION__,g_DevFun.SeriesType,ucValue);
#endif			
			PIOOpenOutput(PIOMan.nFd, UNLOCK_PORT, MOX_PIO_PIN12, ucValue);
		}
		else
		{
			PIOOpenOutput(PIOMan.nFd, UNLOCK_PORT, UNLOCK_PIN, ucValue);
		}
		break;

	case PIO_TYPE_DO_1:
		PIOOpenOutput(PIOMan.nFd, DO1_PORT, DO1_PIN, ucValue);
		break;

	case PIO_TYPE_DO_2:
		PIOOpenOutput(PIOMan.nFd, DO2_PORT, DO2_PIN, ucValue);
		break;

	case PIO_TYPE_CCD_LED:
		PIOOpenOutput(PIOMan.nFd, CCD_LED_PORT, CCD_LED_PIN, ucValue);
		break;
		
	case PIO_TYPE_KEY_LED:
		PIOOpenOutput(PIOMan.nFd, KEY_LED_PORT, KEY_LED_PIN, ucValue);
		break;

	case PIO_TYPE_CCD_POWER:
		PIOOpenOutput(PIOMan.nFd, CCD_POWER_PORT, CCD_POWER_PIN, ucValue);
		break;

	case PIO_TYPE_OLED_POWER:
		PIOOpenOutput(PIOMan.nFd, OLED_POWER_PORT, OLED_POWER_PIN, ucValue);
		break;

	case PIO_TYPE_HID_HOLD:
		PIOOpenOutput(PIOMan.nFd, HID_HOLD_PORT, HID_HOLD_PIN, ucValue);
		break;

	case PIO_TYPE_AMP_PWR:
		PIOOpenOutput(PIOMan.nFd, AMP_PWR_PORT, AMP_PWR_PIN, ucValue);
		break;

	case PIO_TYPE_BEEP_CONTROL:
		PIOOpenOutput(PIOMan.nFd, BEEP_CONTROL_PORT, BEEP_CONTROL_PIN, ucValue);
		break;
	case PIO_TYPE_SHELL_KEYBOARD_POWER:
		PIOOpenOutput(PIOMan.nFd, MOX_PIOC, MOX_PIO_PIN20, ucValue);
		break;
	case PIO_TYPE_SHELL_LCD_POWER:
		PIOOpenOutput(PIOMan.nFd, MOX_PIOC, MOX_PIO_PIN17, ucValue);
		break;
	case PIO_TYPE_SHELL_READCARD_POWER:
		PIOOpenOutput(PIOMan.nFd, MOX_PIOF, MOX_PIO_PIN7, ucValue);
		break;
	default:	
		break;
	}
}
