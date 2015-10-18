/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	PatrolProc.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		10 - April - 2009
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
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>  
#include <termios.h>

/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"

/************** DEFINES **************************************************************/

/************** TYPEDEFS *************************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/