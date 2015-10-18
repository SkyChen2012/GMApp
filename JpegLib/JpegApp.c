

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "MXTypes.h"
#include "MXList.h"
#include "MXMem.h"
#include "Dispatch.h"
#include "Jpeg.h"
#include "JpegApp.h"

//#define SAVE_YUV_JPG_DATA

BYTE	RawPicBuf[PHOTO_WIDTH * PHOTO_HEIGHT * 3];
BYTE	JpgPicBuf[PHOTO_WIDTH * PHOTO_HEIGHT];
INT		nJpgBufLen	=	0;
BOOL	bYUV2JPG	=	FALSE;

/////////////////////////////////////////////////////////////////////////////
// Convert YUV411 to JPEG photo file
static int YUV2Jpg(PBYTE in_Y,PBYTE in_U,PBYTE in_V,int width,int height,int quality,int nStride,PBYTE pOut,DWORD *pnOutSize)
{
	PBYTE pYBuf,pUBuf,pVBuf;
	int nYLen = nStride  * height;
	int nUVLen = nStride  * height / 4;
	int	nDataLen;
	JPEGINFO JpgInfo;
	memset(&JpgInfo, 0, sizeof(JPEGINFO));
	JpgInfo.bytenew = 0;
	JpgInfo.bytepos = 7;
	pYBuf = (PBYTE)malloc(nYLen);
	memcpy(pYBuf,in_Y,nYLen);
	pUBuf = (PBYTE)malloc(nYLen);
	pVBuf = (PBYTE)malloc(nYLen);
	
	ProcessUV(pUBuf,in_U,width,height,nStride);
	ProcessUV(pVBuf,in_V,width,height,nStride);
	//	GetDataFromSource(pYBuf,pUBuf,pVBuf,in_Y,in_U,in_V,width);
	DivBuff(pYBuf,width,height,nStride,DCTSIZE,DCTSIZE);
	DivBuff(pUBuf,width,height,nStride,DCTSIZE,DCTSIZE);
	DivBuff(pVBuf,width,height,nStride,DCTSIZE,DCTSIZE);
	quality = QualityScaling(quality);
	SetQuantTable(std_Y_QT,JpgInfo.YQT, quality); // 设置Y量化表
	SetQuantTable(std_UV_QT,JpgInfo.UVQT,quality); // 设置UV量化表
	
	InitQTForAANDCT(&JpgInfo);            // 初始化AA&N需要的量化表
	JpgInfo.pVLITAB=JpgInfo.VLI_TAB + 2048;                             // 设置VLI_TAB的别名
	BuildVLITable(&JpgInfo);            // 计算VLI表   
	
	
	nDataLen = 0;
	// 写入各段
	nDataLen = WriteSOI(pOut,nDataLen); 
	nDataLen = WriteAPP0(pOut,nDataLen);
	nDataLen = WriteDQT(&JpgInfo,pOut,nDataLen);
	nDataLen = WriteSOF(pOut,nDataLen,width,height);
	nDataLen = WriteDHT(pOut,nDataLen);
	nDataLen = WriteSOS(pOut,nDataLen);
	
	// 计算Y/UV信号的交直分量的huffman表，这里使用标准的huffman表，并不是计算得出，缺点是文件略长，但是速度快
	BuildSTDHuffTab(STD_DC_Y_NRCODES,STD_DC_Y_VALUES,JpgInfo.STD_DC_Y_HT);
	BuildSTDHuffTab(STD_AC_Y_NRCODES,STD_AC_Y_VALUES,JpgInfo.STD_AC_Y_HT);
	BuildSTDHuffTab(STD_DC_UV_NRCODES,STD_DC_UV_VALUES,JpgInfo.STD_DC_UV_HT);
	BuildSTDHuffTab(STD_AC_UV_NRCODES,STD_AC_UV_VALUES,JpgInfo.STD_AC_UV_HT);
	
	// 处理单元数据
	nDataLen = ProcessData(&JpgInfo,pYBuf,pUBuf,pVBuf,width,height,pOut,nDataLen);  
	nDataLen = WriteEOI(pOut,nDataLen);
	
	free(pYBuf);
	free(pUBuf);
	free(pVBuf);
	*pnOutSize = nDataLen;
	return 0;
}

/*
void ConvertYUV2Jpg()
{
	BYTE *in_Y   =	NULL;
	BYTE *in_U   =	NULL;
	BYTE *in_V	  =	NULL;

	BYTE *in_Y2   =	NULL;
	BYTE *in_U2  =	NULL;
	BYTE *in_V2	  =	NULL;
	BYTE *pData = NULL;

	DWORD dwSize = 0;
	FILE *fpYUV = NULL;
	FILE *fpJPG = NULL;

	INT i = 0;
	
	in_Y   =	(BYTE*)malloc(PHOTO_WIDTH * PHOTO_HEIGHT / 2);
	in_U   =	(BYTE*)malloc(PHOTO_WIDTH * PHOTO_HEIGHT / 8);
	in_V	  =	(BYTE*)malloc(PHOTO_WIDTH * PHOTO_HEIGHT / 8);

	in_Y2   =	(BYTE*)malloc(PHOTO_WIDTH * PHOTO_HEIGHT);
	in_U2  =	(BYTE*)malloc(PHOTO_WIDTH * PHOTO_HEIGHT / 4);
	in_V2	  =	(BYTE*)malloc(PHOTO_WIDTH * PHOTO_HEIGHT / 4);

	pData = (BYTE*)malloc(PHOTO_WIDTH * PHOTO_HEIGHT);	

	fpYUV = fopen(RAW_PIC_NAME,"rb");
	if(NULL == fpYUV)
	{
		return;
	}
	
	fseek(fpYUV, 0, SEEK_SET);
	fread(in_Y,PHOTO_WIDTH * PHOTO_HEIGHT / 2,1,fpYUV);
	fseek(fpYUV, 0, SEEK_CUR);
	fread(in_U,PHOTO_WIDTH * PHOTO_HEIGHT / 8, 1,fpYUV);
	fseek(fpYUV, 0, SEEK_CUR);
	fread(in_V,PHOTO_WIDTH * PHOTO_HEIGHT / 8, 1,fpYUV);
	fclose(fpYUV);
	
	for(i = 0; i < PHOTO_HEIGHT / 2; i++)
	{
		memcpy(in_Y2 + 2 * i * PHOTO_WIDTH, in_Y + i * PHOTO_WIDTH, PHOTO_WIDTH);
		memcpy(in_Y2 + (2 * i + 1) * PHOTO_WIDTH, in_Y + i * PHOTO_WIDTH, PHOTO_WIDTH);

		memcpy(in_U2 + 2 * i * PHOTO_WIDTH / 4, in_U + i * PHOTO_WIDTH / 4, PHOTO_WIDTH / 4);
		memcpy(in_U2 + (2 * i + 1) * PHOTO_WIDTH / 4, in_U + i * PHOTO_WIDTH / 4, PHOTO_WIDTH / 4);

		memcpy(in_V2 + 2 * i * PHOTO_WIDTH / 4, in_V + i * PHOTO_WIDTH / 4, PHOTO_WIDTH / 4);
		memcpy(in_V2 + (2 * i + 1) * PHOTO_WIDTH / 4, in_V + i * PHOTO_WIDTH / 4, PHOTO_WIDTH / 4);
	}

	YUV2Jpg(in_Y2,in_U2,in_V2,PHOTO_WIDTH,PHOTO_HEIGHT,75,PHOTO_WIDTH,pData,&dwSize);
	
	fpJPG = fopen(JPG_PIC_NAME,"wb");
	if (NULL == fpJPG)
	{
		return;
	}
	fwrite(pData,dwSize,1,fpJPG);
	fclose(fpJPG);

	free(in_Y);
	free(in_U);
	free(in_V);
	free(in_Y2);
	free(in_U2);
	free(in_V2);
	free(pData);
}
*/



void
ConvertYUV2Jpg()
{
	INT	 time1,time2;
	BYTE *in_Y   =	NULL;
	BYTE *in_U   =	NULL;
	BYTE *in_V	  =	NULL;

#ifdef SAVE_YUV_JPG_DATA
	BYTE	pData[PHOTO_WIDTH * PHOTO_HEIGHT] = { 0 };
	DWORD	dwSize = 0;
	FILE*	fpJPG = NULL;	
	FILE*	FdYUV = NULL;
#endif	
	
	in_Y   =	RawPicBuf;
	in_U   =	in_Y + PHOTO_WIDTH * PHOTO_HEIGHT;
	in_V   =	in_U + PHOTO_WIDTH * PHOTO_HEIGHT / 4;

#ifdef SAVE_YUV_JPG_DATA
	FdYUV =		fopen(RAW_PIC_NAME, "wb");
	if (NULL != FdYUV) {
		fseek(FdYUV, 0, 	SEEK_SET);
		fwrite(in_Y,	1,	 PHOTO_WIDTH * PHOTO_HEIGHT,	FdYUV);
		fseek(FdYUV, 0, 	SEEK_CUR);
		fwrite(in_U,	1,	 PHOTO_WIDTH * PHOTO_HEIGHT / 4,	FdYUV);
		fseek(FdYUV, 0, 	SEEK_CUR);
		fwrite(in_V,	1,	 PHOTO_WIDTH * PHOTO_HEIGHT / 4,	FdYUV);
		fclose(FdYUV);
		FdYUV = NULL;
		printf("Raw pic file saved\n");	
	}
	else
	{
		printf("Raw file open error\n");
	}
#endif
	printf("ConvertYUV2Jpg start\n");
	time1=GetTickCount();

	bYUV2JPG	=	FALSE;
	nJpgBufLen	=	0;
	memset(JpgPicBuf,0,PHOTO_WIDTH * PHOTO_HEIGHT);	
	
	YUV2Jpg(in_Y,in_U,in_V,PHOTO_WIDTH,PHOTO_HEIGHT,75,PHOTO_WIDTH,JpgPicBuf,&nJpgBufLen);
	bYUV2JPG	=	TRUE;
	time2=GetTickCount();
	printf("ConvertYUV2Jpg end time=%d\n",time2-time1);
#ifdef SAVE_YUV_JPG_DATA
	fpJPG = fopen(JPG_PIC_NAME,"wb");
	if (NULL == fpJPG)
	{
		return;
	}
	fwrite(JpgPicBuf,nJpgBufLen,1,fpJPG);
	fclose(fpJPG);
#endif
}
