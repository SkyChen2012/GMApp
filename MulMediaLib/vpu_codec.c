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
 * @file vpu_codec.h
 *
 * @brief This file implement codec application.
 *
 * @ingroup VPU
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <linux/videodev.h>

#include "vpu_voip_app.h"
#include "vpu_display.h"
#include "vpu_capture.h"
#include "JpegApp.h"
#include "MenuParaProc.h"
#include "VideoCapture.h"
#include "Multimedia.h" 
#include "SysLogProc.h"

#define IMAGE_WIDTH			SCREEN_MAX_WIDTH
#define IMAGE_HEIGHT		SCREEN_MAX_HEIGHT 
#define QUATER_IMAGE_WIDTH	IMAGE_WIDTH/2

/*
    H264固定码率下I帧压制频率低于每25帧压一个I帧时，数据量会变大
*/
#define SAFE_I_PIC_SEQ_INTERVAL     25
/*
    H264固定码率下没有I帧请求时的一般I帧压制频率
*/
#define NORMAL_I_PIC_SEQ_INTERVAL   75

extern struct codec_file multi_yuv[MAX_NUM_INSTANCE];

/******  Timing Stuff Begin******/
typedef enum evType_t {
	DEC_READ = 1,
	DEC_RD_OVER,
	DEC_START,
	DEC_STOP,
	DEC_OUT,
	DEC_OUT_OVER,
	ENC_READ,
	ENC_RD_OVER,
	ENC_START,
	ENC_STOP,
	ENC_OUT, 
	ENC_OUT_OVER,
	TEST_BEGIN,
	TEST_END,

} evType;


#define MEASURE_COUNT 3000

//#define DEBUG

typedef struct time_rec {
	evType tag;
	struct timeval tval;
} time_rec_t;

extern VOID	MMYUV2JPGNote();
extern pthread_mutex_t		MutexCap;
UINT g_uAreahead = 1;
extern	pthread_mutex_t		MutexFrm;

time_rec_t time_rec_vector[MEASURE_COUNT + 1];

static unsigned long iter;

unsigned int enc_core_time;
unsigned int enc_core_time_max;
unsigned int enc_core_time_min = 100000;
unsigned int enc_idle_time;
unsigned int enc_idle_time_max;
unsigned int enc_idle_time_min = 100000;

unsigned int dec_core_time;
unsigned int dec_core_time_max;
unsigned int dec_core_time_min = 100000;
unsigned int dec_idle_time;
unsigned int dec_idle_time_max;
unsigned int dec_idle_time_min = 100000;

unsigned int ttl_time = 0;

#define	DIFF_TIME_TV_SEC(i, j)	\
	(time_rec_vector[i].tval.tv_sec - time_rec_vector[j].tval.tv_sec)
#define	DIFF_TIME_TV_USEC(i, j)	\
	(time_rec_vector[i].tval.tv_usec - time_rec_vector[j].tval.tv_usec)
#define DIFF_TIME_US(i, j) (DIFF_TIME_TV_SEC(i, j) * 1000000 + DIFF_TIME_TV_USEC(i, j))


const int extra_fb = 1;
struct codec_config *usr_config = NULL;//(struct codec_config *)malloc(sizeof (struct codec_config));
DecHandle dechandle = { 0 };
DecOpenParam decOP = { 0 };
DecInitialInfo initialInfo = { 0 };
DecOutputInfo outputInfo = { 0 };
DecParam decParam = { 0 };
RetCode ret;
PhysicalAddress paWrPtr;
PhysicalAddress paRdPtr;
Uint32 vaBsBufStart;
Uint32 vaBsBufEnd;
int frameIdx = 0, totalNumofErrMbs = 0, fillendBs = 0, decodefinish = 0;
int streameof = 0;
int checkeof = 0;
Uint32 size = 0;
Uint32 fRateInfo = 0;
Uint32 framebufWidth = 0, framebufHeight = 0;
int needFrameBufCount = 0;
FRAME_BUF FrameBufPool[MAX_FRAME];

FRAME_BUF *pFrame[NUM_FRAME_BUF];
FrameBuffer frameBuf[NUM_FRAME_BUF];

int i;
struct v4l2_buffer pp_buffer;
int output_ratio;
int image_size;
int rot_en = 0;
int rot_angle;
int mirror_dir;
int rotStride;
int storeImage;

vpu_mem_desc bit_stream_buf;
int virt_bit_stream_buf = 0;

FRAME_BUF *pDispFrame;
FrameBuffer *emma_buffer;


int saveEncHeader = 1;
PhysicalAddress bsBuf0;
Uint32 size0;

int srcFrameIdx;
int encextra_fb = 1;
int encstride = 0;
Uint32 encframebufWidth = 0, encframebufHeight = 0;

/* FMO(Flexible macroblock ordering) support */ 
int fmoEnable = 0; 
int fmoSliceNum = 2; 
int fmoType = 0;	/* 0 - interleaved, or 1 - dispersed */

EncHandle enchandle = { 0 };
EncOpenParam encOP = { 0 };
EncParam encParam = { 0 };
EncInitialInfo encinitialInfo = { 0 };
EncOutputInfo encoutputInfo = { 0 };
SearchRamParam encsearchPa = { 0 };
RetCode encret = RETCODE_SUCCESS;
EncHeaderParam encHeaderParam = { 0 };

int encmode;
struct codec_config *encusr_config = NULL;
int encpicWidth = 0;
int encpicHeight = 0;
int encbitRate = 0;


FRAME_BUF *encpFrame[NUM_FRAME_BUF];
FrameBuffer encframeBuf[NUM_FRAME_BUF];
FRAME_BUF encFrameBufPool[MAX_FRAME];
int enci;
int encframeIdx;
int encrot_en = 0;
int encoutput_ratio = 4;
struct v4l2_buffer prp_buffer;
int encimage_size;
vpu_mem_desc encbit_stream_buf;

int encvirt_bit_stream_buf = 0;

static void	Enc_error(int mode);

static void Dec_error(int mode);
static void size_of_IPic_log_add(Uint32 uSize);
void timer(evType tag)
{
	if (iter >= MEASURE_COUNT)
		return;

	struct timeval tval;
	gettimeofday(&tval, NULL);
	time_rec_vector[iter].tag = tag;
	time_rec_vector[iter].tval = tval;
	iter++;
}

void PrintTimingData(void)
{
#ifdef DEBUG
	unsigned int i = 0;
	unsigned int j = 0, k = 0, m = 0, n = 0;
	unsigned int s1 = 0;
	unsigned int first_enc_start, last_enc_start;
	unsigned int first_dec_start, last_dec_start;
	unsigned int num_enc = 0, num_dec = 0;
	FILE *fp;

//      printf("iter=%d\n", iter);
	if ((fp = fopen("/tmp/vpu.txt", "w")) == NULL)
		printf("Unable to open log file\n");

	while (i < iter) {
		switch (time_rec_vector[i].tag) {
		case ENC_START:
			j = i;
			if (k > 0) {
				enc_idle_time += DIFF_TIME_US(i, k);
				if (enc_idle_time_max < DIFF_TIME_US(i, k))
					enc_idle_time_max = DIFF_TIME_US(i, k);
				if (enc_idle_time_min > DIFF_TIME_US(i, k))
					enc_idle_time_min = DIFF_TIME_US(i, k);

				printf("%8u\n", DIFF_TIME_US(i, k));
				fprintf(fp, "%8u\n", DIFF_TIME_US(i, k));
			} else {
				first_enc_start = i;
			}
			last_enc_start = i;

			printf("Encoding %4u : ", num_enc);
			fprintf(fp, "Encoding %4u : ", num_enc);

			break;
		case ENC_STOP:
			enc_core_time += DIFF_TIME_US(i, j);
			if (enc_core_time_max < DIFF_TIME_US(i, j))
				enc_core_time_max = DIFF_TIME_US(i, j);
			if (enc_core_time_min > DIFF_TIME_US(i, j))
				enc_core_time_min = DIFF_TIME_US(i, j);
			printf("%8u  ", DIFF_TIME_US(i, j));
			fprintf(fp, "%8u  ", DIFF_TIME_US(i, j));
			k = i;
			ttl_time = DIFF_TIME_US(i, 0);
			num_enc++;
			break;
		case DEC_START:
			m = i;
			if (n > 0) {
				dec_idle_time += DIFF_TIME_US(i, n);
				if (dec_idle_time_max < DIFF_TIME_US(i, n))
					dec_idle_time_max = DIFF_TIME_US(i, n);
				if (dec_idle_time_min > DIFF_TIME_US(i, n))
					dec_idle_time_min = DIFF_TIME_US(i, n);

				printf("%8u\n", DIFF_TIME_US(i, n));
				fprintf(fp, "%8u\n", DIFF_TIME_US(i, n));
			} else {
				first_dec_start = i;
			}
			last_dec_start = i;

			printf("Decoding %4u : ", num_dec);
			fprintf(fp, "Decoding %4u : ", num_dec);

			break;
		case DEC_STOP:
			dec_core_time += DIFF_TIME_US(i, m);
			if (dec_core_time_max < DIFF_TIME_US(i, m))
				dec_core_time_max = DIFF_TIME_US(i, m);
			if (dec_core_time_min > DIFF_TIME_US(i, m))
				dec_core_time_min = DIFF_TIME_US(i, m);
			printf("%8u  ", DIFF_TIME_US(i, m));
			fprintf(fp, "%8u  ", DIFF_TIME_US(i, m));
			n = i;
			ttl_time = DIFF_TIME_US(i, 0);
			num_dec++;
			break;
		case TEST_BEGIN:
			s1 = i;

			printf("<Test %4u :", num_dec);
			fprintf(fp, "<Test %4u :", num_dec);

			break;
		case TEST_END:
			printf("%5u >", DIFF_TIME_US(i, s1));
			fprintf(fp, "%5u >", DIFF_TIME_US(i, s1));
			break;
		}

		i++;
	}

	printf
	    ("\n\nSummary of Testing Result(also can see \"/tmp/vpu.txt\"): \n");
	fprintf(fp, "\n\nSummary of Testing Result: \n");
	if (num_enc > 0) {
		printf("Max Encoding time %6u %9u\n", enc_core_time_max,
		       enc_idle_time_max);
		fprintf(fp, "Max Encoding time %6u %9u\n", enc_core_time_max,
			enc_idle_time_max);
		printf("Min Encoding time %6u %9u\n", enc_core_time_min,
		       enc_idle_time_min);
		fprintf(fp, "Min Encoding time %6u %9u\n", enc_core_time_min,
			enc_idle_time_min);
		printf("Enc average time  %6u %9u\n", enc_core_time / num_enc,
		       enc_idle_time / num_enc);
		fprintf(fp, "Enc average time  %6u %9u\n",
			enc_core_time / num_enc, enc_idle_time / num_enc);
		printf("Enc fps = %.1f\n",
		       (float)num_enc * 1000000 / DIFF_TIME_US(last_enc_start,
							       first_enc_start));
		fprintf(fp, "Enc fps = %.1f\n",
			(float)num_enc * 1000000 / DIFF_TIME_US(last_enc_start,
								first_enc_start));
	}
	if (num_dec > 0) {
		printf("Max Decoding time %6u %9u\n", dec_core_time_max,
		       dec_idle_time_max);
		fprintf(fp, "Max Decoding time %6u %9u\n", dec_core_time_max,
			dec_idle_time_max);
		printf("Min Decoding time %6u %9u\n", dec_core_time_min,
		       dec_idle_time_min);
		fprintf(fp, "Min Decoding time %6u %9u\n", dec_core_time_min,
			dec_idle_time_min);
		printf("Dec average time  %6u %9u\n", dec_core_time / num_dec,
		       dec_idle_time / num_dec);
		fprintf(fp, "Dec average time  %6u %9u\n",
			dec_core_time / num_dec, dec_idle_time / num_dec);
		printf("Dec fps = %.1f\n",
		       (float)num_dec * 1000000 / DIFF_TIME_US(last_dec_start,
							       first_dec_start));
		fprintf(fp, "Dec fps = %.1f\n",
			(float)num_dec * 1000000 / DIFF_TIME_US(last_dec_start,
								first_dec_start));
	}

	if (num_enc > 0 && num_dec > 0) {
		printf("Average Frame Rate: %.1f\n",
		       (float)(num_enc + num_dec) * 1000 * 1000 / ttl_time / 2);
		fprintf(fp, "Average Frame Rate: %.1f\n",
			(float)(num_enc +
				num_dec) * 1000 * 1000 / ttl_time / 2);
	}

	printf("\n");
	fprintf(fp, "\n");

	fclose(fp);
#endif
	return;
}
/******  Timing Stuff End******/

extern struct codec_file multi_bitstream[MAX_NUM_INSTANCE];
#define EOSCHECK_APISCHEME

int quitflag = 0;

typedef struct codec_info {
	CodecInst handle;
	FRAME_BUF *FbPool;
	int FbNumber;
	vpu_mem_desc bitstream_buf;
} codec_info;

static struct codec_info enc_info;
static struct codec_info dec_info;

static pthread_mutex_t codec_mutex;
#ifdef INT_CALLBACK
static pthread_mutex_t vpu_busy_mutex;
static pthread_cond_t current_job_cond;
#endif

extern sigset_t mask;

static RetCode ReadBsBufHelper(EncHandle handle, int src, char *src_name,
				PhysicalAddress paBsBufStart, int bsBufStartAddr, int bsBufEndAddr, int defaultsize, 
				int index, int read_flag)
{
	RetCode ret = RETCODE_SUCCESS;
	int loadSize = 0;
	PhysicalAddress paRdPtr, paWrPtr;
	Uint32 size = 0;

	ret = vpu_EncGetBitstreamBuffer(handle, &paRdPtr, &paWrPtr, &size);
	if (ret != RETCODE_SUCCESS) {
		printf("vpu_EncGetBitstreamBuffer failed. Error code is 0x%x \n", ret);
		goto LOAD_BS_ERROR;
	}
	
	DPRINTF("ReadRingBs wrsize %d | bssize %d | rdPtr 0x%x | WrPtr 0x%x\n", defaultsize, size, paRdPtr, paWrPtr);

	if (size > 0) {
		if (defaultsize > 0) {
			if (size < defaultsize)
				return RETCODE_SUCCESS;
			loadSize = defaultsize;
		} else {
			loadSize = size;
		}
		
		if (loadSize > 0) {
			ret = FillBsBufMulti(src, src_name,
					bsBufStartAddr + paRdPtr -
					paBsBufStart, bsBufStartAddr,
					bsBufEndAddr, loadSize,
					((EncInst *) handle)->instIndex,
					0, 0, CODEC_WRITING);

			ret = vpu_EncUpdateBitstreamBuffer(handle, loadSize);
			if (ret != RETCODE_SUCCESS) {
				printf("vpu_EncUpdateBitstreamBuffer failed. Error code is 0x%x \n", ret);
				goto LOAD_BS_ERROR;
			}
		}
	}
		
LOAD_BS_ERROR:

	return ret;
}

/* FrameBuffer is a round-robin buffer for Current buffer and reference */
int FrameBufferInit(int strideY,int height,FRAME_BUF* FrameBuf,int FrameNumber)
{
	int i;

	for (i = 0; i < FrameNumber; i++)
    {
		memset(&(FrameBuf[i].CurrImage), 0, sizeof(vpu_mem_desc));
		FrameBuf[i].CurrImage.size = (strideY * height * 3 / 2);
		IOGetPhyMem(&(FrameBuf[i].CurrImage));
		if (FrameBuf[i].CurrImage.phy_addr == 0)
        {
			int j;
			for (j = 0; j < i; j++)
            {
				IOFreeVirtMem(&(FrameBuf[j].CurrImage));
				IOFreePhyMem(&(FrameBuf[j].CurrImage));
			}
			printf("No enough mem for framebuffer!\n");
			return -1;
		}
		FrameBuf[i].Index =     i;
		FrameBuf[i].AddrY =     FrameBuf[i].CurrImage.phy_addr;
		FrameBuf[i].AddrCb =    FrameBuf[i].AddrY + strideY * height;
		FrameBuf[i].AddrCr =    FrameBuf[i].AddrCb + strideY / 2 * height / 2;
		FrameBuf[i].StrideY =   strideY;
		FrameBuf[i].StrideC =   strideY / 2;
		FrameBuf[i].DispY =     FrameBuf[i].AddrY;
		FrameBuf[i].DispCb =    FrameBuf[i].AddrCb;
		FrameBuf[i].DispCr =    FrameBuf[i].AddrCr;
		FrameBuf[i].CurrImage.virt_uaddr =  IOGetVirtMem(&(FrameBuf[i].CurrImage));
	}
	return 0;
}

void FrameBufferFree(FRAME_BUF * FrameBuf, int FrameNumber)
{
	int i;
	for (i = 0; i < FrameNumber; i++) {
		IOFreeVirtMem(&(FrameBuf[i].CurrImage));
		IOFreePhyMem(&(FrameBuf[i].CurrImage));
	}
	return;
}

int vpu_BitstreamPad(DecHandle handle, char *wrPtr, int size)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	int i = 0;

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	if (pDecInfo->openParam.bitstreamFormat == STD_AVC) {
		((unsigned int *)wrPtr)[i] = 0x0;
		i += 4;
		((unsigned int *)wrPtr)[i] = STD_AVC_ENDBUFDATA;
		i += 4;
		while (i < size) {
			((unsigned int *)wrPtr)[i] = 0x0;
			i += 4;
		}
	} else {
		if (pDecInfo->initialInfo.mp4_shortVideoHeader == 1) {
			((unsigned int *)wrPtr)[i] = 0x0;
			i += 4;
			((unsigned int *)wrPtr)[i] = STD_H263_ENDBUFDATA;
			i += 4;
			while (i < size) {
				((unsigned int *)wrPtr)[i] = 0x0;
				i += 4;
			}
		} else {
			((unsigned int *)wrPtr)[i] = 0x0;
			i += 4;
			((unsigned int *)wrPtr)[i] = STD_MP4_ENDBUFDATA;
			i += 4;
			while (i < size) {
				((unsigned int *)wrPtr)[i] = 0x0;
				i += 4;
			}
		}
	}

	return size;
}

FRAME_BUF *GetFrameBuffer(int index, FRAME_BUF * FrameBuf)
{
	return &FrameBuf[index];
}

int GetFrameBufIndex(FRAME_BUF ** bufPool, int poolSize, FrameBuffer * frame)
{
	int i;

	for (i = 0; i < poolSize; ++i) {
		if ((*bufPool)->AddrY == frame->bufY) {
			return i;
		}
		bufPool++;
	}
	return i;
}

#ifdef INT_CALLBACK
void vpu_test_callback(int status)
{
#if 1
	if (status & IRQ_PIC_RUN) {
#else
	if ((status & IRQ_PIC_RUN) || (status & IRQ_DEC_BUF_EMPTY)) {
#endif
		pthread_mutex_lock(&vpu_busy_mutex);
		pthread_cond_signal(&current_job_cond);
		pthread_mutex_unlock(&vpu_busy_mutex);
	}

	return;
}
#endif

int vpu_SystemInit(void)
{
	int ret = -1;

	usr_config = (struct codec_config *)malloc(sizeof (struct codec_config));
	encusr_config = (struct codec_config *)malloc(sizeof (struct codec_config));

	pthread_mutex_init(&codec_mutex, NULL);
#ifdef INT_CALLBACK
	pthread_mutex_init(&vpu_busy_mutex, NULL);
	pthread_cond_init(&current_job_cond, NULL);

	ret = IOSystemInit((void *)(vpu_test_callback));
#else
	ret = IOSystemInit(NULL);
#endif
	if (ret < 0) {
		printf("IO system init failed!\n");
		return -1;
	}


	return ret;
}

int vpu_SystemShutdown(void)
{
	if (enc_info.FbNumber) {
		vpu_EncClose(&(enc_info.handle));
		FrameBufferFree(enc_info.FbPool, enc_info.FbNumber);
		IOFreeVirtMem(&(enc_info.bitstream_buf));
		IOFreePhyMem(&(enc_info.bitstream_buf));
		memset(&enc_info, 0, sizeof(struct codec_info));
	}
	if (dec_info.FbNumber) {
		vpu_DecClose(&(dec_info.handle));
		FrameBufferFree(dec_info.FbPool, dec_info.FbNumber);
		IOFreeVirtMem(&(dec_info.bitstream_buf));
		IOFreePhyMem(&(dec_info.bitstream_buf));
		memset(&dec_info, 0, sizeof(struct codec_info));
	}
	IOSystemShutdown();
	PrintTimingData();
	printf("vpu_SystemShutdown :file close end\n");

	free(usr_config);
	free(encusr_config);

	return 0;
}


/* To supporte multi-instance, Rd & Wr Ptr should be assigned to different regs.*/
/* This Decode Test doesn't do the rotation * and no reconstruction buffer is needed*/

int Decode_Init(int nSrcType, char * pBuf, int nDataLen)
{
	  int  i =0;
	  usr_config->index = 0;
	  usr_config->src = nSrcType;
	  strcpy(usr_config->src_name, "0");
	  usr_config->dst = 1;
	  strcpy(usr_config->dst_name, "/dev/null");
	  usr_config->enc_flag = 0;
	  usr_config->fps = 30;
	  usr_config->bps = 0;
	  usr_config->mode = 0;
	  usr_config->width = 640;
	  usr_config->height = 480;
	  usr_config->gop = 1;//no use now
	  usr_config->frame_count = 1200;//not used now
	  usr_config->rot_angle = 0;
	  usr_config->out_ratio = 1;
	  usr_config->mirror_angle = 0;


	checkeof = 0;
	memset(&dec_info, 0, sizeof(struct codec_info));

	switch (usr_config->mode) {
	case 0:
		decOP.bitstreamFormat = STD_MPEG4;
		DPRINTF("Dec mode: MPEG4\n");
		break;
	case 1:
		decOP.bitstreamFormat = STD_H263;
		DPRINTF("Dec mode: H.263\n");
		break;
	case 2:
		decOP.bitstreamFormat = STD_AVC;
		DPRINTF("Dec mode: H.264\n");
		break;
	default:
		printf("Unknown Dec mode\n");
		return 2;
	}
	rot_angle = usr_config->rot_angle;
	mirror_dir = usr_config->mirror_angle;

	pthread_mutex_lock(&codec_mutex);
	memset(&bit_stream_buf, 0, sizeof(vpu_mem_desc));
	bit_stream_buf.size = STREAM_BUF_SIZE;
	IOGetPhyMem(&bit_stream_buf);
	virt_bit_stream_buf = IOGetVirtMem(&bit_stream_buf);
	decOP.bitstreamBuffer = bit_stream_buf.phy_addr;
	decOP.bitstreamBufferSize = STREAM_BUF_SIZE;
	vaBsBufStart = virt_bit_stream_buf;
	vaBsBufEnd = vaBsBufStart + decOP.bitstreamBufferSize;
	decOP.qpReport = 0;
	decOP.reorderEnable = 0;

	/* Get buffer position and Fill Buffer */
	ret = vpu_DecOpen(&dechandle, &decOP);
	if (ret != RETCODE_SUCCESS) {
		printf("vpu_DecOpen failed. Error code is %d \n", ret);
		pthread_mutex_unlock(&codec_mutex);
		Dec_error(1);
		return 2;
	}

	ret = vpu_DecGetBitstreamBuffer(dechandle, &paRdPtr, &paWrPtr, &size);
	if (ret != RETCODE_SUCCESS) {
		printf("vpu_DecGetBitstreamBuffer failed. Error code is %d \n",
		       ret);
		pthread_mutex_unlock(&codec_mutex);
		Dec_error(0);
		return 2;
	}

	pthread_mutex_unlock(&codec_mutex);
			     
	memcpy(virt_bit_stream_buf + paWrPtr - bit_stream_buf.phy_addr, 
		pBuf, nDataLen);
	ret = nDataLen;

	if (ret < 0) {
		printf("FillBsBufMulti failed. Error code is %d\n", ret);
		vpu_SystemShutdown();
		return 2;
		//return &codec_thread[usr_config->index];
	}
	pthread_mutex_lock(&codec_mutex);

	if (usr_config->src != PATH_NET) {
		ret = vpu_DecUpdateBitstreamBuffer(dechandle, STREAM_FILL_SIZE);
	} else {
		ret = vpu_DecUpdateBitstreamBuffer(dechandle, ret);
	}

	if (ret != RETCODE_SUCCESS) {
		printf
		    ("vpu_DecUpdateBitstreamBuffer failed. Error code is %d \n",
		     ret);
		pthread_mutex_unlock(&codec_mutex);
		Dec_error(0);
		return 2;
	}

	vpu_DecSetEscSeqInit(dechandle, 1);

	/* Parse bitstream and get width/height/framerate/h264/mp4 info etc. */
	ret = vpu_DecGetInitialInfo(dechandle, &initialInfo);
	if (ret != RETCODE_SUCCESS) {
		printf
		    ("vpu_DecGetInitialInfo failed. Error code is %d \n",
		     ret);
				pthread_mutex_unlock(&codec_mutex);
		Dec_error(0);
		return 2;
	}

	vpu_DecSetEscSeqInit(dechandle, 0);

	fRateInfo = initialInfo.frameRateInfo;


	framebufWidth = ((initialInfo.picWidth + 15) & ~15);
	framebufHeight = ((initialInfo.picHeight + 15) & ~15);

	if ((usr_config->dst != PATH_EMMA) || (rot_angle != 0) || (mirror_dir != 0)) {
		/* Frame Buffer Pool needed is the minFrameBufferCount +1 loopfilter, 
		   if rotation yes, should also add 1 */
		ret = FrameBufferInit(initialInfo.picWidth, initialInfo.picHeight,
				      FrameBufPool,
				      initialInfo.minFrameBufferCount + extra_fb);
		if (ret < 0) {
			printf("Mem system allocation failed!\n");
			return 2;
			//return &codec_thread[usr_config->index];
		}
	}

	dec_info.FbPool = FrameBufPool;
	dec_info.FbNumber = initialInfo.minFrameBufferCount + extra_fb;
	needFrameBufCount = dec_info.FbNumber;
	memcpy(&(dec_info.bitstream_buf), &bit_stream_buf,
	       sizeof(vpu_mem_desc));
	memcpy(&(dec_info.handle), &dechandle, sizeof(DecHandle));

	if ((usr_config->dst != PATH_EMMA) || 
	    ((usr_config->dst == PATH_EMMA) && ((rot_angle != 0) || (mirror_dir != 0)))) {
		/* Get the image buffer */
		for (i = 0; i < needFrameBufCount; ++i) {
			pFrame[i] = GetFrameBuffer(i, FrameBufPool);
			frameBuf[i].bufY = pFrame[i]->AddrY;
			frameBuf[i].bufCb = pFrame[i]->AddrCb;
			frameBuf[i].bufCr = pFrame[i]->AddrCr;
		}
	}

	memset(&pp_buffer, 0, sizeof(struct v4l2_buffer));
	image_size = initialInfo.picWidth * initialInfo.picHeight;

	output_ratio = usr_config->out_ratio;

	if ((usr_config->dst == PATH_EMMA) && (rot_angle == 0) && (mirror_dir == 0)) {
		if (!multi_yuv[((DecInst *) dechandle)->instIndex].fd) {
			multi_yuv[((DecInst *) dechandle)->instIndex].fd = (FILE *)	
			    emma_dev_open(initialInfo.picWidth, initialInfo.picHeight, CODEC_WRITING,
					  output_ratio, initialInfo.minFrameBufferCount);

		}
	
		for (i = 0; i < initialInfo.minFrameBufferCount; ++i) {
			frameBuf[i].bufY = buffers[i].offset;
			frameBuf[i].bufCb = frameBuf[i].bufY + image_size;
			frameBuf[i].bufCr =
			    frameBuf[i].bufCb + (image_size >> 2);
		}
	}

	/* Set the frame buffer array to the structure */
	ret = vpu_DecRegisterFrameBuffer(dechandle, frameBuf,
					 initialInfo.minFrameBufferCount,
					 initialInfo.picWidth);
	if (ret != RETCODE_SUCCESS) {
		printf("vpu_DecRegisterFrameBuffer failed. Error code is %d \n",
		       ret);
				pthread_mutex_unlock(&codec_mutex);
		Dec_error(0);
		return 2;
	}
#ifdef EOSCHECK_APISCHEME
	decParam.dispReorderBuf = 0;
	decParam.prescanEnable = 1;
	decParam.prescanMode = 0;
#else
	decParam.dispReorderBuf = 0;
	decParam.prescanEnable = 0;
	decParam.prescanMode = 0;
#endif
	decParam.skipframeMode = 0;
	decParam.skipframeNum = 0;
	decParam.iframeSearchEnable = 0;

	pthread_mutex_unlock(&codec_mutex);

	if ((usr_config->dst == PATH_EMMA) && ((rot_angle != 0) || (mirror_dir != 0))) {
		emma_buffer = &frameBuf[initialInfo.minFrameBufferCount + 1];
	}

	rot_en = (rot_angle == 90 || rot_angle == 270) ? 1 : 0;
	if ((usr_config->dst != PATH_EMMA) || 
	    ((usr_config->dst == PATH_EMMA) && ((rot_angle != 0) || (mirror_dir != 0)))) {
		vpu_DecGiveCommand(dechandle, SET_ROTATION_ANGLE, &rot_angle);
	}

//	vpu_DecGiveCommand(handle, SET_MIRROR_DIRECTION, &mirror_dir);
	storeImage = 1;
	rotStride = (rot_angle == 90 || rot_angle == 270) ?
	    framebufHeight : framebufWidth;
	rotStride = storeImage ? rotStride : STRIDE;
	if ((usr_config->dst != PATH_EMMA) || 
	    ((usr_config->dst == PATH_EMMA) && ((rot_angle != 0) || (mirror_dir != 0)))) {
		vpu_DecGiveCommand(dechandle, SET_ROTATOR_STRIDE, &rotStride);
	}
	printf("Decode_Init end...**************\n");
	return 0;
}


void
Dec_Exit(void)
{
//	vpu_DecBitBufferFlush(dechandle);
}

void
Decode_Reset(void)
{
	frameIdx = 0;
//	Display_reset(multi_yuv[((EncInst *) dechandle)->instIndex].fd);
}



int
WriteVideoData(int nType, unsigned char * pBuf, int nBufLen)
{
//	if (frameIdx % 100 == 0)
//			printf(" WriteVideoData:  Inst %d, No. %d, nBufLen = %d\n",
//				((DecInst *) dechandle)->instIndex, frameIdx, nBufLen);
		

#ifdef EOSCHECK_APISCHEME
		if (decOP.reorderEnable == 1 &&
		    decOP.bitstreamFormat == STD_AVC) {
			static int prescanEnable;
			if (frameIdx == 0) {
				prescanEnable = decParam.prescanEnable;
				decParam.prescanEnable = 0;
			} else if (frameIdx == 1)
				decParam.prescanEnable = prescanEnable;
		}
#endif
		pthread_mutex_lock(&codec_mutex);
		if ((usr_config->dst == PATH_EMMA) && (rot_angle == 0) && (mirror_dir == 0)) {
			FillYuvImageMulti(usr_config->dst, usr_config->dst_name,
						0,
					  (void *)&pp_buffer, initialInfo.picWidth,
					  initialInfo.picHeight,
					  ((DecInst *) dechandle)->instIndex,
					  CODEC_WRITING, rot_en, output_ratio, 0);
		}

		// Start decoding a frame. 
		ret = vpu_DecStartOneFrame(dechandle, &decParam);
		if (ret != RETCODE_SUCCESS) {
			printf
			    ("vpu_DecStartOneFrame failed. Error code is %d \n",
			     ret);
					pthread_mutex_unlock(&codec_mutex);
			Dec_error(0);
			return 2;
		}

		// fill the bitstream buffer while the system is busy
		timer(DEC_START);
#ifdef INT_CALLBACK
		pthread_mutex_lock(&vpu_busy_mutex);
		pthread_cond_wait(&current_job_cond, &vpu_busy_mutex);
		pthread_mutex_unlock(&vpu_busy_mutex);
#else
		vpu_WaitForInt(200);
#endif
		timer(DEC_STOP);

		if (quitflag) {
			pthread_mutex_unlock(&codec_mutex);
			printf("WriteVideoData quitflag = 1\n");
			return 0;
		}

		/*
		 * vpu_DecGetOutputInfo() should match vpu_DecStartOneFrame() with
		 * the same handle. No other API functions can intervene between these two
		 * functions, except for vpu_IsBusy(), vpu_DecGetBistreamBuffer(),
		 * and vpu_DecUpdateBitstreamBuffer().
		 */
		ret = vpu_DecGetOutputInfo(dechandle, &outputInfo);
		if (ret != RETCODE_SUCCESS) {
			printf
			    ("vpu_DecGetOutputInfo failed. Error code is %d \n",   ret);
		}
		
		if (outputInfo.indexDecoded == -1 || outputInfo.indexDecoded > needFrameBufCount)
		{
			printf("##  *** ## outputInfo.indexDecoded =%d\n ", outputInfo.indexDecoded);
			decodefinish = 1;
		}
		ret =
		    vpu_DecGetBitstreamBuffer(dechandle, &paRdPtr, &paWrPtr,
					      &size);
//send the data to inner buffer to paly .

		pthread_mutex_unlock(&codec_mutex);

		if (size >= nBufLen) 
		{
			if (paWrPtr - bit_stream_buf.phy_addr + nBufLen >  STREAM_BUF_SIZE)
			{
				memcpy(virt_bit_stream_buf + paWrPtr - bit_stream_buf.phy_addr, pBuf,
					STREAM_BUF_SIZE - paWrPtr + bit_stream_buf.phy_addr);
				memcpy(virt_bit_stream_buf, &pBuf[STREAM_BUF_SIZE - paWrPtr +  bit_stream_buf.phy_addr], 
					nBufLen - STREAM_BUF_SIZE + paWrPtr - bit_stream_buf.phy_addr);
			}
			else
			{
				memcpy(virt_bit_stream_buf + paWrPtr - bit_stream_buf.phy_addr,
					pBuf,
					nBufLen);
			}


			ret = nBufLen;
			
			if (ret < 0) {
				if (ferror
				    (multi_bitstream
				     [((DecInst *) dechandle)->instIndex].fd)) {
					printf
					    ("FillBsBufMulti failed. Error code is %d \n",
					     ret);
					Dec_error(0);
					return 2;
				}
			}

			if (usr_config->src == PATH_NET) {
				ret = vpu_DecUpdateBitstreamBuffer(dechandle,
								   ret);
				if (ret != RETCODE_SUCCESS) {
					printf
					    ("vpu_DecUpdateBitstreamBuffer failed. Error code is %d \n",
					     ret);
					Dec_error(0);
					return 2;
				}
				goto NET_JUMP;
			}

#ifdef EOSCHECK_APISCHEME
			if (streameof == 0) {
				ret = vpu_DecUpdateBitstreamBuffer(dechandle,
								   STREAM_FILL_SIZE);
				if (ret != RETCODE_SUCCESS) {
					printf
					    ("vpu_DecUpdateBitstreamBuffer failed. Error code is %d \n",
					     ret);
					Dec_error(0);
					return 2;
				}

			} else {
				if (!fillendBs) {
					vpu_BitstreamPad(dechandle,
							 (char *)(virt_bit_stream_buf +
							 paWrPtr -
							 bit_stream_buf.
							 phy_addr),
							 STREAM_END_SIZE);
					ret =
					    vpu_DecUpdateBitstreamBuffer(dechandle,
									 STREAM_END_SIZE);
					if (ret != RETCODE_SUCCESS) {
						printf
						    ("vpu_DecUpdateBitstreamBuffer failed. Error code is %d \n",
						     ret);
						Dec_error(0);
						return 2;
					}
					fillendBs = 1;
				}
			}
#else
			if (streameof == 0) {
				ret = vpu_DecUpdateBitstreamBuffer(dechandle,
								   STREAM_FILL_SIZE);
				if (ret != RETCODE_SUCCESS) {
					printf
					    ("vpu_DecUpdateBitstreamBuffer failed. Error code is %d \n",
					     ret);
					Dec_error(0);
					return 2;
				}
			}
#endif

		}

NET_JUMP:
#ifdef EOSCHECK_APISCHEME
		if (outputInfo.prescanresult == 0 &&
		    decParam.dispReorderBuf == 0) {
			if (streameof) {
				if (decOP.reorderEnable == 1 &&
				    decOP.bitstreamFormat == STD_AVC &&
				    (outputInfo.indexDecoded != -1)) {
					decParam.dispReorderBuf = 1;
					printf("NET_JUMP EOSCHECK_APISCHEME!\n");
					return 1;
				} else {
					printf("NET_JUMP EOSCHECK_APISCHEME!: %d, %d, %d, %d\n",
					decOP.reorderEnable, decOP.bitstreamFormat, outputInfo.indexDecoded,decParam.dispReorderBuf  
					);
					decodefinish = 1;
				}
			} else {	/* not enough bitstream data */
				printf("PreScan Result: No enough bitstream data.\n");
				return 1;
			}
		}
#endif
//
//		if (decodefinish)
//		{
//		//	printf("decodefinish!\n");
//	//		return 1;
//		}
	decodefinish = 0;
		frameIdx++;
		if (outputInfo.indexDecoded == -2) /* BIT don't have picture to be displayed */
		{
			printf("**************BIT don't have picture to be displayed!\n");
			return 1;
		}

		return 0;

}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Dec_error
**	AUTHOR:			
**	DATE:			30 - Oct - 2006
**
**	DESCRIPTION:	
**			error process when decoding video data.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				node		[IN]		int		erroe mode
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
Dec_error(int mode)
{
	if (mode == 1) goto ERR_DEC_INIT;
	
      ERR_DEC_OPEN:
	pthread_mutex_lock(&codec_mutex);
	vpu_DecClose(dechandle);
	if (ret == RETCODE_FRAME_NOT_COMPLETE) {
		vpu_DecGetOutputInfo(dechandle, &outputInfo);
		vpu_DecClose(dechandle);
	}
	pthread_mutex_unlock(&codec_mutex);

      ERR_DEC_INIT:
	FrameBufferFree(FrameBufPool,
			initialInfo.minFrameBufferCount + extra_fb);
	IOFreeVirtMem(&bit_stream_buf);
	IOFreePhyMem(&bit_stream_buf);
	memset(&dec_info, 0, sizeof(struct codec_info));
	printf("Dec error: %d. closed\n", mode);

	if (multi_yuv[((EncInst *) dechandle)->instIndex].fd)
	{
		printf("Dec_error:  stop display ..index=%d.\n", (EncInst *) dechandle->instIndex);
		stop_display(multi_yuv[((EncInst *) dechandle)->instIndex].fd);
		multi_yuv[((EncInst *) dechandle)->instIndex].fd = 0;
	}
	//return &codec_thread[usr_config->index];
}



/* for this test, only 1 reference*/
/* no rotation*/
int Encode_Init(unsigned char * pBuf, int nBufLen)
{
	BYTE LocStreamFormat;
	LocStreamFormat=GetTalkLocStreamFormat();
	encusr_config->index = 0;
	encusr_config->src = 1;
	strcpy(encusr_config->src_name, "/dev/null");
	encusr_config->dst = 2;
	strcpy(encusr_config->dst_name, "172.16.9.9");
	encusr_config->enc_flag = 1;
	encusr_config->fps =30;

    
	if(MM_VIDEO_FORMAT_MPEG4_PAL==LocStreamFormat)
	{        
		encusr_config->bps = 1500;
		encusr_config->mode =0;
	}
	else if(MM_VIDEO_FORMAT_h264==LocStreamFormat)
	{
		encusr_config->bps = 1500;
		encusr_config->mode =2;
	}
	
	encusr_config->width =IMAGE_WIDTH;
	encusr_config->height = IMAGE_HEIGHT;//240;
	
	


	encusr_config->gop = 1;//no use now
	encusr_config->frame_count = 1200;//not used now
	encusr_config->rot_angle = 0;
	encusr_config->out_ratio = 0;
	encusr_config->mirror_angle = 0;

	encpicWidth = encusr_config->width;
	encpicHeight = encusr_config->height;
	encbitRate = encusr_config->bps;

	switch (encusr_config->mode) {
	case 0:
		encmode = STD_MPEG4;
		DPRINTF("Enc mode: MPEG4\n");
		break;
	case 1:
		encmode = STD_H263;
		DPRINTF("Enc mode: H.263\n");
		break;
	case 2:
		encmode = STD_AVC;
		DPRINTF("Enc mode: H.264\n");
		break;
	default:
		printf("Unknown Enc mode\n");
		return 2;
	}
	printf("%s:width %d, height %d, bitrate %d Kbps\n",__FUNCTION__, encpicWidth, encpicHeight, encbitRate);
	DPRINTF("width %d, height %d, bitrate %d Kbps\n", encpicWidth, encpicHeight, encbitRate);

	memset(&enc_info, 0, sizeof(struct codec_info));

	/* allocate the bit stream buffer */
	memset(&encbit_stream_buf, 0, sizeof(vpu_mem_desc));
	encbit_stream_buf.size = STREAM_BUF_SIZE;
	pthread_mutex_lock(&codec_mutex);
	IOGetPhyMem(&encbit_stream_buf);
	encvirt_bit_stream_buf = IOGetVirtMem(&encbit_stream_buf);
	pthread_mutex_unlock(&codec_mutex);
	encOP.bitstreamBuffer = encbit_stream_buf.phy_addr;
	encOP.bitstreamBufferSize = STREAM_BUF_SIZE;

	/* framebufWidth and framebufHeight must be a multiple of 16 */
	encframebufWidth = ((encpicWidth + 15) & ~15);  	
	encframebufHeight = ((encpicHeight + 15) & ~15);

	encOP.bitstreamFormat = encmode;
	encOP.picWidth = encpicWidth;
	encOP.picHeight = encpicHeight;
    if(encmode == STD_AVC)
    {
        printf("frameRateInfo = 0x03E888DB\n");
        encOP.frameRateInfo = 0x03E888DB;//[MichaelMa] 35 frames / s
    }	
    else if(encmode == STD_MPEG4)
    {
        printf("frameRateInfo = 20\n");
        encOP.frameRateInfo = 20;
    }
    
	encOP.bitRate = encbitRate;
	encOP.initialDelay = 0;
	encOP.vbvBufferSize = 0;	/* 0 = ignore 8 */
	encOP.enableAutoSkip = 0;
	encOP.gopSize = 0;	/* only first picture is I */
	encOP.sliceMode = 0;	/* 1 slice per picture */
	encOP.sliceSizeMode = 0;
	encOP.sliceSize = 4000;	/* not used if sliceMode is 0 */
	encOP.intraRefresh = 0;
	encOP.sliceReport = 0;
	encOP.mbReport = 0;
	encOP.mbQpReport = 0;
	if (encmode == STD_MPEG4) {
		encOP.EncStdParam.mp4Param.mp4_dataPartitionEnable = 0;
		encOP.EncStdParam.mp4Param.mp4_reversibleVlcEnable = 0;
		encOP.EncStdParam.mp4Param.mp4_intraDcVlcThr = 0;
	} else if (encmode == STD_H263) {
		encOP.EncStdParam.h263Param.h263_annexJEnable = 0;
		encOP.EncStdParam.h263Param.h263_annexKEnable = 0;
		encOP.EncStdParam.h263Param.h263_annexTEnable = 0;
		if (encOP.EncStdParam.h263Param.h263_annexJEnable == 0 &&
		    encOP.EncStdParam.h263Param.h263_annexKEnable == 0 &&
			encOP.EncStdParam.h263Param.h263_annexTEnable == 0 ) {
			encOP.frameRateInfo = 0x3E87530;
		}
	} else if (encmode == STD_AVC){
		encOP.EncStdParam.avcParam.avc_constrainedIntraPredFlag = 0;
		encOP.EncStdParam.avcParam.avc_disableDeblk = 0;
		encOP.EncStdParam.avcParam.avc_deblkFilterOffsetAlpha = 0;
		encOP.EncStdParam.avcParam.avc_deblkFilterOffsetBeta = 0;
		encOP.EncStdParam.avcParam.avc_chromaQpOffset = 0;	// -12 ~ 12
		encOP.EncStdParam.avcParam.avc_audEnable = 0;
		encOP.EncStdParam.avcParam.avc_fmoEnable = fmoEnable;
		encOP.EncStdParam.avcParam.avc_fmoType = fmoType;
		encOP.EncStdParam.avcParam.avc_fmoSliceNum = fmoSliceNum;
	} else {
		printf("Encoder: Invalid codec standard mode \n");
		return 2;
	}
	pthread_mutex_lock(&codec_mutex);
	encret = vpu_EncOpen(&enchandle, &encOP);
	if (encret != RETCODE_SUCCESS) {
		printf("vpu_EncOpen failed. Error code is %d \n", encret);
				pthread_mutex_unlock(&codec_mutex);
		Enc_error(1);
		return 2;
	}
	encsearchPa.searchRamAddr = DEFAULT_SEARCHRAM_ADDR;
	encret = vpu_EncGiveCommand(enchandle, ENC_SET_SEARCHRAM_PARAM, &encsearchPa);
	if (encret != RETCODE_SUCCESS) {
		printf
		    ("vpu_EncGiveCommand ( ENC_SET_SEARCHRAM_PARAM ) failed. Error code is %d \n",
		     encret);
				pthread_mutex_unlock(&codec_mutex);
		Enc_error(0);
		return 2;
	}
	encret = vpu_EncGetInitialInfo(enchandle, &encinitialInfo);
	if (encret != RETCODE_SUCCESS) {
		printf("vpu_EncGetInitialInfo failed. Error code is %d \n",
		       encret);
				pthread_mutex_unlock(&codec_mutex);
		Enc_error(0);
		return 2;
	}
	DPRINTF("Enc: min buffer count= %d\n", encinitialInfo.minFrameBufferCount);
// 
// 	if (saveEncHeader == 1) {
// 		if (encmode == STD_MPEG4) {
// 			SaveGetEncodeHeader(enchandle, ENC_GET_VOS_HEADER, "mp4_vos_header.dat");
// 			SaveGetEncodeHeader(enchandle, ENC_GET_VO_HEADER, "mp4_vo_header.dat");
// 			SaveGetEncodeHeader(enchandle, ENC_GET_VOL_HEADER, "mp4_vol_header.dat");
// 		} else if (encmode == STD_AVC) {
// 			SaveGetEncodeHeader(enchandle, ENC_GET_SPS_RBSP, "avc_sps_rbsp_header.dat");
// 			SaveGetEncodeHeader(enchandle, ENC_GET_PPS_RBSP, "avc_pps_rbsp_header.dat");
// 		}
// 	}
	/* allocate the image buffer, rec buffer/ref buffer plus src buffer */
    /*
        初次调用到此处时，encinitialInfo.minFrameBufferCount值为0，
        encextra_fb值为1
    */
	FrameBufferInit(encpicWidth,encpicHeight,encFrameBufPool,encinitialInfo.minFrameBufferCount + encextra_fb);

	enc_info.FbPool = encFrameBufPool;
	enc_info.FbNumber = encinitialInfo.minFrameBufferCount + encextra_fb;
	memcpy(&(enc_info.bitstream_buf),&encbit_stream_buf,sizeof(struct vpu_mem_desc));
	memcpy(&(enc_info.handle), &enchandle, sizeof(DecHandle));

	srcFrameIdx = encinitialInfo.minFrameBufferCount;
	for (enci = 0; enci < encinitialInfo.minFrameBufferCount + encextra_fb; ++enci)
    {
		encpFrame[enci] = GetFrameBuffer(enci, encFrameBufPool);
		encframeBuf[enci].bufY = encpFrame[enci]->AddrY;
		encframeBuf[enci].bufCb = encpFrame[enci]->AddrCb;
		encframeBuf[enci].bufCr = encpFrame[enci]->AddrCr;
	}

	encstride = encframebufWidth;
    
	encret = vpu_EncRegisterFrameBuffer(enchandle, encframeBuf,encinitialInfo.minFrameBufferCount,encstride);
    
	if (encret != RETCODE_SUCCESS) {
		printf("vpu_EncRegisterFrameBuffer failed.Error code is %d \n",
		       encret);
				pthread_mutex_unlock(&codec_mutex);
		Enc_error(0);
		return 2;
	}
	pthread_mutex_unlock(&codec_mutex);
#if 0
	DPRINTF("Disp %x, %x, %x,\n\tStore buf %x, %x, %x\n",
		encpFrame[srcFrameIdx]->DispY,
		encpFrame[srcFrameIdx]->DispCb,
		encpFrame[srcFrameIdx]->DispCr,
		(unsigned int)encframeBuf[srcFrameIdx].bufY,
		(unsigned int)encframeBuf[srcFrameIdx].bufCb,
		(unsigned int)encframeBuf[srcFrameIdx].bufCr);
#endif
    encframeIdx = 0;
	encParam.sourceFrame = &encframeBuf[srcFrameIdx];
    if(encmode == STD_AVC)
    {
        printf("quantParam = 35\n");
        encParam.quantParam = 35; //not used in CBR.when you use VBR,the larger the value,the worse the quality of video
    }
    else if(encmode == STD_MPEG4)
    {
        printf("quantParam = 30\n");
        encParam.quantParam = 30;
    }
	
	encParam.forceIPicture = 0;
	encParam.skipPicture = 0;

	encimage_size = encpicWidth * encpicHeight;
	memset(&prp_buffer, 0, sizeof(struct v4l2_buffer));

	/* Must put encode header before first picture encoding. */
	if (encmode == STD_MPEG4) {
		encHeaderParam.headerType = VOS_HEADER;
		vpu_EncGiveCommand(enchandle, ENC_PUT_MP4_HEADER, &encHeaderParam); 

		bsBuf0 = encHeaderParam.buf;
		size0 = encHeaderParam.size;
		DPRINTF("MPEG4 encHeaderParam(VOS_HEADER) addr:%p len:%d\n", bsBuf0, size0);
		FillBsBufMulti(encusr_config->dst, encusr_config->dst_name,
				     encvirt_bit_stream_buf + bsBuf0 -
				     encbit_stream_buf.phy_addr, 0,
				     0, size0,
				     ((EncInst *) enchandle)->instIndex,
				     0, 0, CODEC_WRITING);

		if (size0 <= nBufLen)
		{
			memcpy(pBuf, encvirt_bit_stream_buf + bsBuf0 - encbit_stream_buf.phy_addr, size0);
			encret = size0;
		}
		else
		{
			printf("Encode init error: the header infor data lenth larger %d.\n", nBufLen);
		}

		encHeaderParam.headerType = VIS_HEADER;
		vpu_EncGiveCommand(enchandle, ENC_PUT_MP4_HEADER, &encHeaderParam); 

		bsBuf0 = encHeaderParam.buf;
		size0 = encHeaderParam.size;
		DPRINTF("MPEG4 encHeaderParam(VIS_HEADER) addr:%p len:%d\n", bsBuf0, size0);
		FillBsBufMulti(encusr_config->dst, encusr_config->dst_name,
				     encvirt_bit_stream_buf + bsBuf0 -
				     encbit_stream_buf.phy_addr, 0,
				     0, size0,
				     ((EncInst *) enchandle)->instIndex,
				     0, 0, CODEC_WRITING);

		if (size0 + encret <= nBufLen)
		{
			memcpy(&pBuf[encret], encvirt_bit_stream_buf + bsBuf0 - encbit_stream_buf.phy_addr, size0);
			encret += size0;
		}
		else
		{
			printf("Encode init error: the header infor data lenth larger %d.\n", nBufLen);
		}
					 			 
		encHeaderParam.headerType = VOL_HEADER;
		vpu_EncGiveCommand(enchandle, ENC_PUT_MP4_HEADER, &encHeaderParam); 
		
		bsBuf0 = encHeaderParam.buf;
		size0 = encHeaderParam.size;
		DPRINTF("MPEG4 encHeaderParam(VOL_HEADER) addr:%p len:%d\n", bsBuf0, size0);
		FillBsBufMulti(encusr_config->dst, encusr_config->dst_name,
				     encvirt_bit_stream_buf + bsBuf0 -
				     encbit_stream_buf.phy_addr, 0,
				     0, size0,
				     ((EncInst *) enchandle)->instIndex,
				     0, 0, CODEC_WRITING);

		if (size0 + encret <= nBufLen)
		{
			memcpy(&pBuf[encret], encvirt_bit_stream_buf + bsBuf0 - encbit_stream_buf.phy_addr, size0);
			encret += size0;
		}
		else
		{
			printf("Encode init error: the header infor data lenth larger %d.\n", nBufLen);
		}
	} 
	else if (encmode == STD_AVC) 
	{
		encHeaderParam.headerType = SPS_RBSP;
		vpu_EncGiveCommand(enchandle, ENC_PUT_AVC_HEADER, &encHeaderParam); 

		bsBuf0 = encHeaderParam.buf;
		size0 = encHeaderParam.size;
		DPRINTF("AVC encHeaderParam(SPS_RBSP) addr:%p len:%d\n", bsBuf0, size0);
        
		if (size0 <= nBufLen)
		{
			memcpy(pBuf, encvirt_bit_stream_buf + bsBuf0 - encbit_stream_buf.phy_addr, size0);
			encret = size0;
		}
		else
		{
			printf("Encode init error: the header infor data lenth larger %d.\n", nBufLen);
		}

		encHeaderParam.headerType = PPS_RBSP;
		vpu_EncGiveCommand(enchandle, ENC_PUT_AVC_HEADER, &encHeaderParam); 

		bsBuf0 = encHeaderParam.buf;
		size0 = encHeaderParam.size;
		DPRINTF("AVC encHeaderParam(PPS_RBSP) addr:%p len:%d\n", bsBuf0, size0);
        
		if (size0 + encret <= nBufLen)
		{
			memcpy(&pBuf[encret], encvirt_bit_stream_buf + bsBuf0 - encbit_stream_buf.phy_addr, size0);
			encret += size0;
		}
		else
		{
			printf("Encode init error: the header infor data lenth larger %d.\n", nBufLen);
		}
		
	}
	
//	printf("Encode init end....Send H264 Encoding Heading  nLen=%d.\n", encret);
	return encret;
}

void
Enc_Exit(void)
{
	/*if (multi_yuv[((EncInst *) enchandle)->instIndex].fd)
	{

		stop_capturing(multi_yuv[((EncInst *) enchandle)->instIndex].fd);
		multi_yuv[((EncInst *) enchandle)->instIndex].fd = 0;
	} */
}

void 
Encode_Reset(void)
{
	encframeIdx = 1;
;//	ResetCapturing(multi_yuv[((EncInst *) enchandle)->instIndex].fd);
}





/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ReadPicRawData
**	AUTHOR:			Harry Qian
**	DATE:			26 - Nov - 2008
**
**	DESCRIPTION:	
**			Read Raw data from camera.
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				FrmType		[IN]		Frame Type
**				pSendData	[IN/OUT]	MmSendAVData*	
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
int 
ReadPicRawData(unsigned char * pBuf, int nBufLen)
{
	unsigned char* pSrcY = NULL;
	unsigned char* pSrcU = NULL; 
	unsigned char* pSrcV = NULL;
	unsigned char* pOddY = NULL;
	unsigned char* pOddU = NULL;
	unsigned char* pOddV = NULL;
	unsigned char* pEvenY = NULL;
	unsigned char* pEvenU = NULL;
	unsigned char* pEvenV = NULL;
	unsigned char* src = NULL;
	unsigned char* dst = NULL;	
	int n;	
	//static int nVCurCount = 0;
	static		UINT	nOldIFrameTick = 0;
	UINT			nNewIFrameTick = 0;
#ifdef	VIDEO_CAPTURE_LOG_DEBUG	
	char			Data[256];
#endif

	//I frame deciced
	
    //get raw data from camera.

	encret = FillYuvImageMulti(encusr_config->src, encusr_config->src_name,
			      encframeBuf[srcFrameIdx].bufY +
			      encFrameBufPool[srcFrameIdx].CurrImage.
			      virt_uaddr -
			      encFrameBufPool[srcFrameIdx].CurrImage.
			      phy_addr, (void *)&prp_buffer, encpicWidth,
			      encpicHeight,
			      ((EncInst *) enchandle)->instIndex,
			      CODEC_READING1, encrot_en, encoutput_ratio, 0);

					  
		if (encusr_config->src == PATH_EMMA) 
		{

			if (0 == g_SysConfig.VideoFormat || 1 == g_SysConfig.VideoFormat)
			{
				encframeBuf[srcFrameIdx].bufY = encpFrame[srcFrameIdx]->CurrImage.phy_addr;
			}
			else
			{
				encframeBuf[srcFrameIdx].bufY = cap_buffers[prp_buffer.index].offset;
			}

			encframeBuf[srcFrameIdx].bufCb = encframeBuf[srcFrameIdx].bufY + encimage_size;
			encframeBuf[srcFrameIdx].bufCr = encframeBuf[srcFrameIdx].bufCb + (encimage_size >> 2);

			if (0 == g_SysConfig.VideoFormat)
			{	
				pOddY = (unsigned char *)(cap_buffers[prp_buffer.index].start);
				pOddU = (unsigned char *)pOddY + encimage_size;
				pOddV = (unsigned char *)pOddU+ (encimage_size >> 2);

				pEvenY = pOddY + encimage_size / 2;
				pEvenU = pOddU + (encimage_size >> 3);
				pEvenV = pOddV + (encimage_size >> 3);	

				pSrcY = (unsigned char *)(encpFrame[srcFrameIdx]->CurrImage.virt_uaddr);
				pSrcU = pSrcY + encimage_size;
				pSrcV = pSrcU + (encimage_size >> 2);	
			
				
				
				for(n = 0; n < IMAGE_HEIGHT/2; n++)
				{
					src = pEvenY + n * IMAGE_WIDTH;
					dst = pSrcY+2* n * IMAGE_WIDTH;
					memcpy(dst, src, IMAGE_WIDTH);
					src = pEvenY + n * IMAGE_WIDTH;
					memcpy(dst + IMAGE_WIDTH, src, IMAGE_WIDTH);
				}

				for(n = 0; n < IMAGE_HEIGHT/4; n++)
				{
					src = pEvenU + n * QUATER_IMAGE_WIDTH;
					dst = pSrcU + 2* n * QUATER_IMAGE_WIDTH;
					memcpy(dst, src, QUATER_IMAGE_WIDTH);
					src = pEvenU + n * QUATER_IMAGE_WIDTH;
					memcpy(dst + QUATER_IMAGE_WIDTH, src, QUATER_IMAGE_WIDTH);
				}	
				for(n = 0; n < IMAGE_HEIGHT/4; n++)
				{
					src = pEvenV + n * QUATER_IMAGE_WIDTH;
					dst = pSrcV + 2* n * QUATER_IMAGE_WIDTH;
					memcpy(dst, src, QUATER_IMAGE_WIDTH);
					src = pEvenV + n * QUATER_IMAGE_WIDTH;
					memcpy(dst + QUATER_IMAGE_WIDTH, src, QUATER_IMAGE_WIDTH);
				}		
			}
			else if (1 == g_SysConfig.VideoFormat)
			{
				pOddY = (unsigned char *)(cap_buffers[prp_buffer.index].start);
				pOddU = (unsigned char *)pOddY + encimage_size;
				pOddV = (unsigned char *)pOddU+ (encimage_size >> 2);
				
				pEvenY = pOddY + encimage_size / 2;
				pEvenU = pOddU + (encimage_size >> 3);
				pEvenV = pOddV + (encimage_size >> 3);	
				
				pSrcY = (unsigned char *)(encpFrame[srcFrameIdx]->CurrImage.virt_uaddr);
				pSrcU = pSrcY + encimage_size;
				pSrcV = pSrcU + (encimage_size >> 2);
				
				for(n = 0; n < IMAGE_HEIGHT/2; n++)
				{
					src = pOddY + n * IMAGE_WIDTH;
					dst = pSrcY+2* n * IMAGE_WIDTH;
					memcpy(dst, src, IMAGE_WIDTH);
					src = pEvenY + n * IMAGE_WIDTH;
					memcpy(dst + IMAGE_WIDTH, src, IMAGE_WIDTH);
				}
				for(n = 0; n < IMAGE_HEIGHT/4; n++)
				{
					src = pOddU + n * QUATER_IMAGE_WIDTH;
					dst = pSrcU + 2* n * QUATER_IMAGE_WIDTH;
					memcpy(dst, src, QUATER_IMAGE_WIDTH);
					src = pEvenU + n * QUATER_IMAGE_WIDTH;
					memcpy(dst + QUATER_IMAGE_WIDTH, src, QUATER_IMAGE_WIDTH);
				}
				for(n = 0; n < IMAGE_HEIGHT/4; n++)
				{
					src = pOddV + n * QUATER_IMAGE_WIDTH;
					dst = pSrcV + 2* n * QUATER_IMAGE_WIDTH;
					memcpy(dst, src, QUATER_IMAGE_WIDTH);
					src = pEvenV + n * QUATER_IMAGE_WIDTH;
					memcpy(dst + QUATER_IMAGE_WIDTH, src, QUATER_IMAGE_WIDTH);
				}
			}
		}
		if (encret < 0) {
			printf("Encoder finished\n");
			return 0;
		}
		encret = FillYuvImageMulti(encusr_config->src, encusr_config->src_name,
				      encframeBuf[srcFrameIdx].bufY +
				      encFrameBufPool[srcFrameIdx].CurrImage.
				      virt_uaddr -
				      encFrameBufPool[srcFrameIdx].CurrImage.
				      phy_addr, (void *)&prp_buffer, encpicWidth,
				      encpicHeight,
				      ((EncInst *) enchandle)->instIndex,
				      CODEC_READING2, encrot_en, encoutput_ratio, 0);
		
		/*************Get the image in YUV411 Mode*******************/
		if (encret < 0) {
			printf("Encoder finished\n");
			return 0;
		}
		
	    if (FlagEnCapYuv) {
			FlagEnCapYuv = 0;		

			if (0 == g_SysConfig.VideoFormat)
			{
				memcpy(RawPicBuf, pSrcY, encimage_size * 3 / 2);
			}
			MMYUV2JPGNote();
		}
		/********************************************************************/		

		//pthread_mutex_lock(&codec_mutex);
		/* To fix the MPEG4 issue on MX27 TO2 */
		CodStd codStd = ((EncInst *) enchandle)->CodecInfo.encInfo.openParam.bitstreamFormat;
		//DPRINTF("CodStd = %d\n", codStd);
		if (codStd == STD_MPEG4) {
			vpu_ESDMISC_LHD(1);
		}

		//nVCurCount++;
		pthread_mutex_lock(&MutexFrm);
/*===================================================================================*/  
        if(encmode == STD_MPEG4)
        {
            if (encusr_config->dst == PATH_NET) {
			if (
			encframeIdx % 100 == 0  	
			||encframeIdx < 5
				|| 1 == bRequestIFrame 
	
		    ) 
    			{
    				if (1 == bRequestIFrame) 
    				{
    					bRequestIFrame = 0;
    					nNewIFrameTick=GetTickCount();
    				}
    				encParam.forceIPicture = 1;
    			} 
    			else {
    				encParam.forceIPicture = 0;
    			}
    		}	
    		if(nNewIFrameTick && nNewIFrameTick-nOldIFrameTick<500)
    		{
    			encParam.forceIPicture = 0;
    		}
    		if(encParam.forceIPicture == 1)
    		{
    			nOldIFrameTick=GetTickCount();
    		}
        }       
        else
        {
            if(encusr_config->dst == PATH_NET)
            {
                if((encframeIdx == g_uAreahead) || (encframeIdx == (g_uAreahead + NORMAL_I_PIC_SEQ_INTERVAL)))
                {
                    encParam.forceIPicture = 1;
                    g_uAreahead = encframeIdx;
                }
                else
                {
                    if(bRequestIFrame)
                    {
                        if(encframeIdx >= (g_uAreahead + SAFE_I_PIC_SEQ_INTERVAL))
                        {
                            encParam.forceIPicture = 1;
                            g_uAreahead = encframeIdx;
                            bRequestIFrame = 0;
                        }
                        else
                        {
                            encParam.forceIPicture = 0;
                        }
                    }
                    else
                    {
                        encParam.forceIPicture = 0;
                    }
                }
            }
        }              
/*===================================================================================*/  
		pthread_mutex_unlock(&MutexFrm);
        
		encret = vpu_EncStartOneFrame(enchandle, &encParam);
		if (encret != RETCODE_SUCCESS) {
			printf("vpu_EncStartOneFrame failed. Error code is %d \n",encret);
			//pthread_mutex_unlock(&codec_mutex);
			Enc_error(0);
			return 2;
		}
		//while(vpu_IsBusy());
        vpu_WaitForInt(200);
		encret = vpu_EncGetOutputInfo(enchandle, &encoutputInfo);
        
		if (encret != RETCODE_SUCCESS) {
            printf("vpu_EncGetOutputInfo failed. Error code is %d \n",encret);
			//pthread_mutex_unlock(&codec_mutex);
			Enc_error(0);
			return 2;
		}

		// To fix the MPEG4 issue on MX27 TO2 
		if (codStd == STD_MPEG4) {
			vpu_ESDMISC_LHD(0);
		}
		//pthread_mutex_unlock(&codec_mutex);

		if (quitflag){
			printf("ReadPicRawData: quitflay=1\n");
			return 0;
		}	

		bsBuf0 = encoutputInfo.bitstreamBuffer;
		size0 = encoutputInfo.bitstreamSize;
		if (nBufLen < size0 )
		{
			EthGMLog(TYPE_VIDEO_READ_ERROR);
			printf("Error: the size0 = %d, nBufLen=%d.\n", size0, nBufLen);
#ifdef	VIDEO_CAPTURE_LOG_DEBUG			
			sprintf(Data,"Error: the size0 = %d, nBufLen=%d.\n", size0, nBufLen);
			EthGMLog_videologadd(Data);
			EthGMLog_videodata(pSrcY,704*576*2);
#endif				
			return -1;
		}
        	
		pthread_mutex_lock(&MutexCap);

		memcpy(pBuf, encvirt_bit_stream_buf + bsBuf0 - encbit_stream_buf.phy_addr, size0);
        if(encoutputInfo.picType == 0)
        {
            //printf("%u -_-_-_-_-_-_-_-_-_ size : %u KB\n",encframeIdx,size0 / 1024);
            if((size0 / 1024) > 150)
            {
                size0 = 0;
                size_of_IPic_log_add(size0 / 1024);
            }
        }
		pthread_mutex_unlock(&MutexCap);	

        
		encframeIdx++;
		//if(encframeIdx%100==0)
		//printf("Encoder finished: %d ms, Encode time %d ms\n",GetTickCount()-timeTick,GetTickCount()-timeTick1);
		return size0;
		
}

static void
Enc_error(int mode)
{
	if (mode == 1) goto ERR_ENC_INIT;

      ERR_ENC_OPEN:
	//pthread_mutex_lock(&codec_mutex);
	encret = vpu_EncClose(enchandle);
	if (encret == RETCODE_FRAME_NOT_COMPLETE) {
		vpu_EncGetOutputInfo(enchandle, &encoutputInfo);
		vpu_EncClose(enchandle);
	}
	//pthread_mutex_unlock(&codec_mutex);

      ERR_ENC_INIT:
	FrameBufferFree(encFrameBufPool,
			encinitialInfo.minFrameBufferCount + encextra_fb);
	IOFreeVirtMem(&encbit_stream_buf);
	IOFreePhyMem(&encbit_stream_buf);
	DPRINTF("Enc closed\n");
	memset(&enc_info, 0, sizeof(struct codec_info));

	if (multi_yuv[((EncInst *) enchandle)->instIndex].fd)
	{
		printf("---****************--- Enc_error: stop capturing ..index=%d.\n", ((EncInst *) enchandle)->instIndex);
		stop_capturing(multi_yuv[((EncInst *) enchandle)->instIndex].fd);
		multi_yuv[((EncInst *) enchandle)->instIndex].fd = 0;
	}

	//return &codec_thread[encusr_config->index];
}

void *sig_thread(void *arg)
{
	DPRINTF("Enter signal monitor thread.\n");
	int signo;

	if (sigwait(&mask, &signo) != 0) {
		printf("sigwait error\n");
	}

	switch (signo) {
	case SIGINT:
		DPRINTF("interrupt: SIGINT.\n");
		quitflag = 1;
#ifdef	INT_CALLBACK
		pthread_cond_broadcast(&current_job_cond);
#endif
		break;
	default:
		printf("unexpected signal %d\n", signo);
	}

	return NULL;
}

static void size_of_IPic_log_add(Uint32 uSize)
{
    char Data[256];
    sprintf(Data,"[ERROR] I pic : %u\n",uSize);
    EthGMLog_videologadd(Data);
    EthGMLog_videolog();
}
