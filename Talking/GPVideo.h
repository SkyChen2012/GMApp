/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	800Video.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		01 - Sep - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
** 
*/
#ifndef	GPVIDEO_H
#define GPVIDEO_H
/************** SYSTEM INCLUDE FILES **************************************************************************/


/************** USER INCLUDE FILES ****************************************************************************/


/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/
#define CHECK_GP_RESULT_ERROR		0
#define CHECK_GP_RESULT_GP2DEST		1
#define CHECK_GP_RESULT_DEST2GP		2


#define GP_CONFIG_FILE	"/mox/rdwr/GPConfig.ini"
#define GP_CONFIG_SECTION		"Global"
#define KEY_GPFILE_GP_IP		"GP_IP"
#define KEY_GPFILE_GP_PORT	"GP_Port"
#define KEY_GPFILE_IP_Cnt		"IP_Cnt"

#define	VALUE_GPFILE_GP_IP							"172.16.1.1"
#define	VALUE_GPFILE_GP_PORT							1
#define	VALUE_GPFILE_IP_Cnt							0

#define MAX_GPFILE_IP_CNT		10000
/************** TYPEDEFS *************************************************************/


/************** STRUCTURES ***********************************************************/


/************** EXTERNAL DECLARATIONS ************************************************/

extern void StartGPVideo(unsigned long nSrcIPAddr);
extern void StopGPVideo(void);
extern void InitGPVideo(void);
int CheckGPVideoCMD(unsigned short nFunCode,unsigned long nSrcIPAddr);
extern DWORD GetGPIP(void);
extern BOOL IsUseGPVideo(void);
/***********************************************************************************/

#endif //GPVIDEO_H


