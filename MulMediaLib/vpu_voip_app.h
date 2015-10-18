/*
 * Copyright 2004-2006 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * Copyright (c) 2006, Chips & Media.  All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file vpu_voip_app.h
 *
 * @brief This file is codec test header.
 *
 * @ingroup VPU
 */

#ifndef __VPU__VOIP__TEST__H
#define __VPU__VOIP__TEST__H

#include "vpu_io.h"
#include "vpu_lib.h"

#define PAL_MODE		0
#define NTSC_MODE		1

//#define	STREAM_BUF_SIZE		0x40000
#define	STREAM_BUF_SIZE		0xA0000

#define SCREEN_MAX_WIDTH	704
#define SCREEN_MAX_HEIGHT	576
#define MAX_WIDTH		SCREEN_MAX_WIDTH
#define MAX_HEIGHT		SCREEN_MAX_HEIGHT

#define STRIDE			MAX_WIDTH

#define STREAM_FILL_SIZE	0x8000
#define	STREAM_END_SIZE		512	// 0

enum {
	COMMAND_NONE = 0,
	COMMAND_DECODE,
	COMMAND_ENCODE,
	COMMAND_MULTI_DEC,
	COMMAND_MULTI_CODEC
};

typedef struct {
	int Index;
	int AddrY;
	int AddrCb;
	int AddrCr;
	int StrideY;
	int StrideC;		/* Stride Cb/Cr */

	int DispY;		/* DispY is the page aligned AddrY */
	int DispCb;		/* DispCb is the page aligned AddrCb */
	int DispCr;
	vpu_mem_desc CurrImage;	/* Current memory descriptor for user space */
} FRAME_BUF;

struct codec_file {
	char name[50];
	FILE *fd;
};

#define MAX_NUM_INSTANCE	4

/*display1/reference16/reconstruction1/loopfilter1/rot1 for 1 video, 20 in total*/
/*display4/reference16/reconstruction1/loopfilter1/rot1 for multi video, 23 in total */
/*why 22 here? */
#define NUM_FRAME_BUF	(1+17+2)
#define MAX_FRAME	(16+2+4)

struct codec_config {
	Uint32 index;
	Uint32 src;
	char src_name[80];
	Uint32 dst;
	char dst_name[80];
	Uint32 enc_flag;
	Uint32 fps;
	int bps;
	Uint32 mode;
	Uint32 width;
	Uint32 height;
	Uint32 gop;
	Uint32 frame_count;
	Uint32 rot_angle;
	Uint32 out_ratio;
	Uint32 mirror_angle;
};

void *DecodeTest(void *param);
void *EncodeTest(void *param);
void *sig_thread(void *arg);	/* the thread is used to monitor signal */
int emma_dev_open(int width, int height, int read_flag, int output_ratio, int frame_num);
int FillBsBufMulti(int src, char *src_name, int targetAddr,
		   int bsBufStartAddr, int bsBufEndAddr, int size,
		   int index, int checkeof, int *streameof, int read_flag);
/* Read/Write one frame raw data each time, for Encoder/Decoder respectively */
int FillYuvImageMulti(int src, char *src_name, int buf, void *emma_buf,
		      int inwidth, int inheight, int index, int read_flag,
		      int rot_en, int output_ratio, int frame_num);

extern int vpu_SystemInit(void);
extern int vpu_SystemShutdown(void);///
extern int Decode_Init(int nType, char * pBuf, int nDataLen);
extern int Encode_Init(unsigned char * pBuf, int nDataLen);//
extern int ReadPicRawData(unsigned char * pBuf, int nBufLen);//
extern int WriteVideoData(int nType, unsigned char * pBuf, int nBufLen);//
extern void	Dec_Exit(void);
extern void	Enc_Exit(void);
extern void Encode_Reset(void);
extern void	Decode_Reset(void);

extern int		FlagEnCapYuv;

#define TEST_BUFFER_NUM		3

#define CODEC_READING		0
#define CODEC_WRITING		1
#define CODEC_READING1		2
#define CODEC_READING2		3

#define PATH_FILE		0
#define PATH_EMMA		1
#define PATH_NET		2

pthread_t codec_thread[MAX_NUM_INSTANCE];

#ifdef DEBUG
#define DPRINTF(fmt, args...) printf("%s: " fmt , __FUNCTION__, ## args)
#else
#define DPRINTF(fmt, args...)
#endif

#endif
