/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Multimedia.h
**
**	AUTHOR:		Betty Gao
**
**	DATE:		19 - Oct - 2006
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	NOTES:
**	
*/

#ifndef MULTIMEDIA_H
#define MULTIMEDIA_H
/************** SYSTEM INCLUDE FILES **************************************************************************/

//Nothing should be included here

/************** USER INCLUDE FILES ****************************************************************************/

#include "MXTypes.h"

/************** ENTRY POINT DECLARATIONS **********************************************************************/

//!!! It is C/C++ file specific, nothing should be defined here

/************** DEFINES ***************************************************************************************/

#define	AV2MM_AUDIO						1
#define	AV2MM_VIDEO						2

#define	MM_AUDIO_TYPE					0x01
#define	MM_VIDEO_TYPE					0x02
#define	MM_VIDEO_CAPTURE_FRAME_30		0x01
#define	MM_VIDEO_CAPTURE_FRAME_25		0x02
#define	MM_VIDEO_FORMAT_JPEG_NTS		0x01
#define	MM_VIDEO_FORMAT_JPEG_PAL		0x02
#define	MM_VIDEO_FORMAT_MPEG4_UNTURE	0x03
#define	MM_VIDEO_FORMAT_MPEG4_NTS		0x04
#define MM_VIDEO_FORMAT_h264			0x06
#define	MM_VIDEO_FORMAT_MPEG4_PAL		0x07
#define MM_VIDEO_FORMAT_ERROR			0xFF


#define	MM_AUDIO_CAPTURE_8KB			0x01
#define	MM_AUDIO_CAPTURE_11KB			0x02
#define	MM_AUDIO_CAPTURE_22KB			0x03
#define	MM_AUDIO_CAPTURE_44KB			0x04
#define	MM_AUDIO_FORMAT_WAV				0x01
#define MM_AUDIO_FORMAT_ERROR			0xFF



#define	MM_VIDEO_CAPTURE_DEFAULT		MM_VIDEO_CAPTURE_FRAME_25
#define	MM_VIDEO_FORMAT_DEFAULT			MM_VIDEO_FORMAT_h264 //MM_VIDEO_FORMAT_MPEG4_PAL
#define	MM_AUDIO_CAPTURE_DEFAULT		MM_AUDIO_CAPTURE_8KB
#define	MM_AUDIO_FORMAT_DEFAULT			MM_AUDIO_FORMAT_WAV

#define	VIDEO_LEN_PER_PACK				1024
#define	AUDIO_LEN_PER_PACK				1000
#define	PHOTO_LEN_PER_PACK				1024


#define TYPE_PLAY_RECYCLE				FALSE
#define TYPE_PLAY_ONCE					TRUE

#define MAX_FRM_CNT						5

#define MM_FORMAT_CNT					3
/************** TYPEDEFS **************************************************************************************/

/*!
* Wave file configuration.
*/
struct wave_config{        
/*!
* SSI
	*/
	unsigned short ssi;
	
	/*!
	* Number of channels (1:MONO, 2:STEREO)
	*/
	unsigned short num_channels;
	
	/*!
	* Sample rate
	*/
	unsigned long sample_rate;
	
	/*!
	* Bits per sample (16-bits mode supported)
	*/
	unsigned short bits_per_sample;
	
	/*!
	* Sample size
	*/
	unsigned long sample_size;
	
	/*!
	* Requested mixing mode
	*/
	unsigned long mix_enabled;
	
	/*!
	* MC13783 device to use
	*/
	unsigned long mc13783_device;
	
	/*!
	* ssi fifo to be used
	*/
	int ssi_fifo;
	
	/*!
	* clock provider
	*/
	int master_clock;
	
};

/************** STRUCTURES ************************************************************************************/

typedef struct _BYTERATE 
{
	INT		nByteOneFrm[MAX_FRM_CNT];
	INT		nCurFrmCnt;
	INT		nTotalByte;
	BOOL	bUpRate;
}BYTERATE;

/************** GLOBAL VARIABLE DEFINITIONS *******************************************************************/

//!!!  It is C/C++ file specific, nothing should be defined here

/************** EXTERNAL DECLARATIONS *************************************************************************/

extern void MultimediaExit();
extern void MultimediaInit();
extern void MxSendAV2Mm(int nType, unsigned char* pBuf, int nBufLen);
extern void MultiTimeProcess(void);
extern	void PutCapData2Buf(void);
extern void SetGMTalkVolume(void);
extern void TalkVolumeChangedNotify2MM(UINT nTalkVol);
extern void GMSetVolumeAPI(int nVolume);
extern BOOL IfCX20707(void);


extern UINT  g_uAreahead;
extern DWORD g_LogReference;

extern BYTE		ucVideoFrameHeader[50];
extern SHORT	nVideoHeaderLen;
extern BOOL		DecInitFlag;
extern int bRequestIFrame;

extern BYTERATE g_ByteRate;
extern unsigned char g_uPlayingRing;
/**************************************************************************************************************/
#endif // MULTIMEDIA_H





















