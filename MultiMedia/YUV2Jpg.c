/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	YUV2Jpg.c
**
**	AUTHOR:		Mike Zhang
**
**	DATE:		01 - Dec - 2010
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

//#include <windows.h>
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
#include "MXMsg.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXMdId.h"
//#include "MXCommon.h"

#include "YUV2Jpg.h"
#include "MenuParaProc.h"
//#include "JpegApp.h"
#include "DiagTelnetd.h"

/************** DEFINES **************************************************************/

#define DEBUG_YUV_CONFIG

/************** TYPEDEFS *************************************************************/


/************** STRUCTURES ***********************************************************/
pthread_mutex_t		MutexCovCapVideo;

/************** EXTERNAL DECLARATIONS ************************************************/
/************** ENTRY POINT DECLARATIONS *********************************************/


/************** LOCAL DECLARATIONS ***************************************************/



static	 pthread_t					JpgthWork;	

void*	JpgWorkThreadFun(void* arg);




/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	YUV2JpgInit
**	AUTHOR:			Mike Zhang
**	DATE:			01 - Dec - 2010
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
void 
YUV2JpgInit()
{
			  
	DpcAddMd(MXMDID_YUV2JPG, NULL);

	if ((pthread_create(&JpgthWork, NULL, JpgWorkThreadFun, NULL)) != 0)	
	{
		printf("YUV2JPG: create thread fail\n");
	}


	pthread_mutex_init(&MutexCovCapVideo, NULL);
	

#ifdef DEBUG_YUV_CONFIG
	printf("YUV2JPG: Initialize success\n");
#endif
}



/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	JpgWorkThreadFun
**	AUTHOR:			Mike Zhang
**	DATE:			01 - Dec - 2010
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				
**	NOTES:
**			
*/
void* 
JpgWorkThreadFun(void* arg)
{			

	MXMSG	msgRecev;
	     
     while(TRUE)
     {
		 memset(&msgRecev, 0, sizeof(msgRecev));
		 msgRecev.dwDestMd	= MXMDID_YUV2JPG;
		 msgRecev.pParam		= NULL;

		 if (MxGetMsg(&msgRecev))
		 {
 			 if (COMM_YUV2JPG == msgRecev.dwMsg)
 			 {
				 //Disable
				 pthread_mutex_lock(&MutexCovCapVideo);				 
//				 ConvertYUV2Bmp();
				 ConvertYUV2Jpg();
				 pthread_mutex_unlock(&MutexCovCapVideo);
				 printf("JPG Convert Success\n");
 			 }
			 if (NULL != msgRecev.pParam)
			 {
				free(msgRecev.pParam);
				msgRecev.pParam = NULL;
			 }
		 }
		 usleep(1 * 1000); 	
      }	  
	pthread_exit(0);
}









