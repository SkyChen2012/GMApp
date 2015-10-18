/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	SystemInfoAPI.h
**
**	AUTHOR:		Harry Qian
**
**	DATE:		16 - Jun - 2008
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef SYSTEMINFOAPI_H
#define SYSTEMINFOAPI_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

/************** USER INCLUDE FILES ****************************************************************************/

/************** DEFINES ***************************************************************************************/

/************** TYPEDEFS **************************************************************************************/

/************** STRUCTURES ************************************************************************************/

// General list structure

#define AV_TYPE_AUDIO		1
#define AV_TYPE_VIDEO		2


#define AUDIO_SPEED_8	1
#define AUDIO_SPEED_11	2
#define AUDIO_SPEED_22	3
#define AUDIO_SPEED_44	4

#define AUDIO_TYPE_WAV	1


#define VIDEO_FRAME_30	1
#define VIDEO_FRAME_25	2

#define VIDEO_MODE_JPEG_NTSC	1
#define VIDEO_MODE_JPEG_PAL		2
#define VIDEO_MODE_MPEG4_PAL	3
#define VIDEO_MODE_MPEG4_NTSC	4
#define VIDEO_MODE_BITMAP		5



#define FUNCQUERY_TALK_LEN		21
#define FUNCQUERY_MON_lEN		21
#define FUNCQUERY_IRISMSG_LEN	11



/************** EXTERNAL DECLARATIONS *************************************************************************/
extern	void	SendFuncInfoQuery(DWORD dwIP, DWORD dwFunID);
extern void		SendSystemInfoQuery(DWORD dwIP);
extern void		PublicFuncMdInit(void);
extern void		PublicFuncMdExit(void);

extern void		PublicFuncProc(void);
/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/**************************************************************************************************************/
#endif //






















