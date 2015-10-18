
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SysLogProc.h
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		23 - Jun - 2009
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**				
**				
**	NOTES:
** 
*/
/************** SYSTEM INCLUDE FILES **************************************************/

/************** USER INCLUDE FILES ***************************************************/

/************** DEFINES **************************************************************/
#define VIDEO_CAPTURE_LOG_DEBUG


/************** TYPEDEFS *************************************************************/
typedef enum _SYSLOGTYPE
{
	TYPE_REBOOT = 1,
	TYPE_VIDEO_READ_ERROR,
	TYPE_NO_A_CAP,
	TYPE_VIDEO_SEND_ERROR
}SYSLOGTYPE;


/************** STRUCTURES ***********************************************************/
struct run_status{

	unsigned long reg[13];
	unsigned int playback_total_size;
	int playback_dmaDone;
};

/************** EXTERNAL DECLARATIONS ************************************************/

extern	VOID	SysLogInit();
extern	VOID	EthGMLog(SYSLOGTYPE LogType);
extern	VOID	EthGMLog_ssi_playback(struct run_status ssi_status);
extern	VOID EthGMLog_videolog( );
extern	VOID EthGMLog_videodata(char *str,int len);
extern	VOID EthGMLog_videologopen();
extern	VOID EthGMLog_videologclose();
extern	VOID EthGMLog_videologadd(char *str);



/************** ENTRY POINT DECLARATIONS *********************************************/




