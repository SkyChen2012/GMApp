#ifndef BMP_H
#define BMP_H

#include "vpu_voip_app.h"
#include "MXTypes.h"


#define BMP_PIC_NAME	"/mox/rdwr/c.bmp"


#define PHOTO_WIDTH			SCREEN_MAX_WIDTH
#define PHOTO_HEIGHT		SCREEN_MAX_HEIGHT



#define BI_RGB		0L
#define BI_RLE8		1L
#define BI_RLE4		2L
#define BI_BITFIELDS	3L

#define RMASK565	0xf800
#define GMASK565	0x07e0
#define BMASK565	0x001f

#define BMP_HEAD_SIZE sizeof(BMPHEAD)
#define BMP_TOTAL_SIZE (BMP_HEAD_SIZE+BMP_HEAD_SIZE+PHOTO_WIDTH * PHOTO_HEIGHT*3)

#pragma pack(1)
/* windows style*/
typedef struct {
	/* BITMAPFILEHEADER*/
	BYTE	bfType[2];
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
	/* BITMAPINFOHEADER*/
	DWORD	BiSize;
	LONG	BiWidth;
	LONG	BiHeight;
	WORD	BiPlanes;
	WORD	BiBitCount;
	DWORD	BiCompression;
	DWORD	BiSizeImage;
	LONG	BiXpelsPerMeter;
	LONG	BiYpelsPerMeter;
	DWORD	BiClrUsed;
	DWORD	BiClrImportant;
} BMPHEAD;

/* os/2 style*/
typedef struct {
	/* BITMAPFILEHEADER*/
	BYTE	bfType[2];
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
	/* BITMAPCOREHEADER*/
	DWORD	bcSize;
	WORD	bcWidth;
	WORD	bcHeight;
	WORD	bcPlanes;
	WORD	bcBitCount;
} BMPCOREHEAD;
#pragma pack()

extern	VOID	ConvertYUV2Bmp();

extern	BYTE	BmpPicBuf[BMP_TOTAL_SIZE];
extern	INT		nBmpBufLen;
#endif

