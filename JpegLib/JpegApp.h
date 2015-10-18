
#include "vpu_voip_app.h"
#include "MXTypes.h"

#define RAW_PIC_NAME	"/mox/rdwr/raw.pic"
#define JPG_PIC_NAME	"/mox/rdwr/b.jpg"

#define PHOTO_WIDTH			SCREEN_MAX_WIDTH
#define PHOTO_HEIGHT		SCREEN_MAX_HEIGHT

extern	VOID	ConvertYUV2Jpg();
extern	BYTE	RawPicBuf[PHOTO_WIDTH * PHOTO_HEIGHT * 3];
extern	BYTE	JpgPicBuf[PHOTO_WIDTH * PHOTO_HEIGHT];
extern	INT		nJpgBufLen;
extern BOOL		bYUV2JPG;

