/*
 * Copyright 2004-2006 Freescale Semiconductor, Inc. All Rights Reserved.
 * 
 * Copyright (c) 2006, Chips & Media.  All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU Lesser General 
 * Public License.  You may obtain a copy of the GNU Lesser General 
 * Public License Version 2.1 or later at the following locations:
 *
 * http://www.opensource.org/licenses/lgpl-license.html
 * http://www.gnu.org/copyleft/lgpl.html
 */

/*!
 * @file vpu_lib.h
 *
 * @brief header file for codec API funcitons for VPU
 *
 * @ingroup VPU
 */

#ifndef __VPU__LIB__H
#define __VPU__LIB__H

typedef struct vpu_versioninfo {
	int fw_major;		/* firmware major version */
	int fw_minor;		/* firmware minor version */
	int fw_release;		/* firmware release version */
	int lib_major;		/* library major version */
	int lib_minor;		/* library minor version */
	int lib_release;	/* library release version */
} vpu_versioninfo;

#define VPU_FW_VERSION(major, minor, release)	 (((major) << 12) + ((minor) << 8) + (release)) 
#define VPU_LIB_VERSION(major, minor, release)	 (((major) << 12) + ((minor) << 8) + (release)) 
#define VPU_LIB_VERSION_CODE	VPU_LIB_VERSION(2, 0, 0)

/*------------------------------------------------------------------------------*/
/* HARDWARE REGISTER    							*/
/*------------------------------------------------------------------------------*/
#define BIT_CODE_RUN			0x000
#define BIT_CODE_DOWN			0x004
#define BIT_INT_REQ			0x008
#define BIT_INT_CLEAR			0x00C
#define	BIT_INT_STS			0x010
#define	BIT_CODE_RST			0x014
#define BIT_CUR_PC			0x018

/*------------------------------------------------------------------------------*/
/* GLOBAL REGISTER								*/
/*------------------------------------------------------------------------------*/
#define BIT_CODE_BUF_ADDR		0x100
#define BIT_WORK_BUF_ADDR		0x104
#define BIT_PARA_BUF_ADDR		0x108
#define BIT_BIT_STREAM_CTRL		0x10C
#define BIT_FRAME_MEM_CTRL		0x110
#define CMD_DEC_DISPLAY_REORDER		0x114

#define BIT_RD_PTR_0			0x120
#define BIT_WR_PTR_0			0x124
#define BIT_RD_PTR_1			0x128
#define BIT_WR_PTR_1			0x12C
#define BIT_RD_PTR_2			0x130
#define BIT_WR_PTR_2			0x134
#define BIT_RD_PTR_3			0x138
#define BIT_WR_PTR_3			0x13C
#define BIT_SEARCH_RAM_BASE_ADDR	0x140	// ME search ram base address
#define BIT_SEARCH_RAM_SIZE		0x144	// ME search ram size ( size = ( EndPicX*36) + 2048 )

#define BIT_BUSY_FLAG			0x160
#define BIT_RUN_COMMAND			0x164
#define BIT_RUN_INDEX			0x168
#define BIT_RUN_COD_STD			0x16C
#define BIT_INT_ENABLE			0x170
#define BIT_INT_REASON			0x174

#define BIT_CMD_0			0x1E0
#define BIT_CMD_1			0x1E4

#define BIT_MSG_0			0x1F0
#define BIT_MSG_1			0x1F4
#define BIT_MSG_2			0x1F8
#define BIT_MSG_3			0x1FC

/*------------------------------------------------------------------------------*/
/* [DEC SEQ INIT] COMMAND  							*/
/*------------------------------------------------------------------------------*/
#define CMD_DEC_SEQ_BB_START		0x180
#define CMD_DEC_SEQ_BB_SIZE		0x184
#define CMD_DEC_SEQ_OPTION		0x188

#define CMD_DEC_SEQ_INIT_ESCAPE		0x114

#define RET_DEC_SEQ_SUCCESS		0x1C0
#define RET_DEC_SEQ_SRC_FMT		0x1C4
#define RET_DEC_SEQ_SRC_SIZE		0x1C4
#define RET_DEC_SEQ_SRC_F_RATE		0x1C8
#define RET_DEC_SEQ_FRAME_NEED		0x1CC
#define RET_DEC_SEQ_FRAME_DELAY		0x1D0
#define RET_DEC_SEQ_INFO		0x1D4

/*------------------------------------------------------------------------------*/
/* [ENC SEQ INIT] COMMAND							*/
/*------------------------------------------------------------------------------*/
#define CMD_ENC_SEQ_BB_START		0x180
#define CMD_ENC_SEQ_BB_SIZE		0x184
#define CMD_ENC_SEQ_OPTION		0x188	/* FMO, QPREP, AUD, SLICE, MB BIT */

#define CMD_ENC_SEQ_COD_STD		0x18C
#define CMD_ENC_SEQ_SRC_SIZE		0x190
#define CMD_ENC_SEQ_SRC_F_RATE		0x194
#define CMD_ENC_SEQ_MP4_PARA		0x198
#define CMD_ENC_SEQ_263_PARA		0x19C
#define CMD_ENC_SEQ_264_PARA		0x1A0
#define CMD_ENC_SEQ_SLICE_MODE		0x1A4
#define CMD_ENC_SEQ_GOP_NUM		0x1A8
#define CMD_ENC_SEQ_RC_PARA		0x1AC
#define CMD_ENC_SEQ_RC_BUF_SIZE		0x1B0
#define CMD_ENC_SEQ_INTRA_REFRESH	0x1B4
#define CMD_ENC_SEQ_FMO			0x1B8

#define RET_ENC_SEQ_SUCCESS		0x1C0

/*------------------------------------------------------------------------------*/
/* [DEC PIC RUN] COMMAND							*/
/*------------------------------------------------------------------------------*/
#define CMD_DEC_PIC_ROT_MODE		0x180
#define CMD_DEC_PIC_ROT_ADDR_Y		0x184
#define CMD_DEC_PIC_ROT_ADDR_CB		0x188
#define CMD_DEC_PIC_ROT_ADDR_CR		0x18C
#define CMD_DEC_PIC_ROT_STRIDE		0x190

#define CMD_DEC_PIC_OPTION		0x194
#define CMD_DEC_PIC_SKIP_NUM		0x198

#define CMD_DEC_PIC_CHUNK_SIZE		0x19C

#define RET_DEC_PIC_FRAME_NUM		0x1C0
#define RET_DEC_PIC_FRAME_IDX		0x1C4
#define RET_DEC_PIC_ERR_MB		0x1C8
#define RET_DEC_PIC_TYPE		0x1CC
#define RET_DEC_PIC_OPTION		0x1D0

/*------------------------------------------------------------------------------*/
/* [ENC PIC RUN] COMMAND							*/
/*------------------------------------------------------------------------------*/
#define CMD_ENC_PIC_SRC_ADDR_Y		0x180
#define CMD_ENC_PIC_SRC_ADDR_CB		0x184
#define CMD_ENC_PIC_SRC_ADDR_CR		0x188
#define CMD_ENC_PIC_QS			0x18C
#define CMD_ENC_PIC_ROT_MODE		0x190
#define CMD_ENC_PIC_OPTION		0x194

#define RET_ENC_PIC_FRAME_NUM		0x1C0
#define RET_ENC_PIC_TYPE		0x1C4
#define RET_ENC_PIC_FRAME_IDX		0x1C8
#define RET_ENC_PIC_SLICE_NUM		0x1CC

/*------------------------------------------------------------------------------*/
/* [SET FRAME BUF] COMMAND							*/
/*------------------------------------------------------------------------------*/
#define CMD_SET_FRAME_BUF_NUM		0x180
#define CMD_SET_FRAME_BUF_STRIDE	0x184

/*------------------------------------------------------------------------------*/
/* [ENC HEADER] COMMAND								*/
/*------------------------------------------------------------------------------*/
#define CMD_ENC_HEADER_CODE		0x180

/*------------------------------------------------------------------------------*/
/* [DEC_PARA_SET] COMMAND							*/
/*------------------------------------------------------------------------------*/
#define CMD_DEC_PARA_SET_TYPE		0x180
#define CMD_DEC_PARA_SET_SIZE		0x184

/*------------------------------------------------------------------------------*/
/* [ENC_PARA_SET] COMMAND 							*/
/*------------------------------------------------------------------------------*/
#define CMD_ENC_PARA_SET_TYPE		0x180
#define RET_ENC_PARA_SET_SIZE		0x1c0

/*------------------------------------------------------------------------------*/
/* [FIRMWARE VERSION] COMMAND 							*/ 
/* [32:16] project number =>							*/ 
/* [16:0]  version => xxxx.xxxx.xxxxxxxx					*/ 
/*------------------------------------------------------------------------------*/
#define RET_VER_NUM			0x1c0

#define BIT_REG_MARGIN			0x1000

#define BSFORMAT_AVC			0x3

#define PRJ_TRISTAN     		0xF000
#define PRJ_CODA_DX_6			0xF001
#define PRJ_PRISM_CX			0xF002
#define PRJ_SHIVA       		0xF003
#define PRJ_PRISM_EX			0xF004
#define PRJ_BODA_DX_4			0xF005
#define PRJ_CODA_DX_6M			0xF100
#define PRJ_CODA_DX_8			0xF306
                              
#define PRJ_BODADX_4V			0xF405


typedef unsigned char Uint8;
typedef unsigned long Uint32;
typedef unsigned short Uint16;

typedef Uint32 PhysicalAddress;

typedef enum {
	STD_MPEG4 = 0,
	STD_H263,
	STD_AVC
} CodStd;

typedef struct {
	Uint32 *paraSet;
	int size;
} DecParamSet;

typedef enum {
	ENABLE_ROTATION,
	DISABLE_ROTATION,
	ENABLE_MIRRORING,
	DISABLE_MIRRORING,
	SET_MIRROR_DIRECTION,
	SET_ROTATION_ANGLE,
	SET_ROTATOR_OUTPUT,
	SET_ROTATOR_STRIDE,
	ENC_GET_SPS_RBSP,
	ENC_GET_PPS_RBSP,                        
	DEC_SET_SPS_RBSP,                        
	DEC_SET_PPS_RBSP,                        
	ENC_PUT_MP4_HEADER,                      
	ENC_PUT_AVC_HEADER,                      
	ENC_SET_SEARCHRAM_PARAM,                 
	ENC_GET_VOS_HEADER,                      
	ENC_GET_VO_HEADER,                       
	ENC_GET_VOL_HEADER                       
} CodecCommand;

typedef enum {
	RETCODE_SUCCESS = 0,
	RETCODE_FAILURE = -1,
	RETCODE_INVALID_HANDLE = -2,
	RETCODE_INVALID_PARAM = -3,
	RETCODE_INVALID_COMMAND = -4,
	RETCODE_ROTATOR_OUTPUT_NOT_SET = -5,
	RETCODE_ROTATOR_STRIDE_NOT_SET = -11,
	RETCODE_FRAME_NOT_COMPLETE = -6,
	RETCODE_INVALID_FRAME_BUFFER = -7,
	RETCODE_INSUFFICIENT_FRAME_BUFFERS = -8,
	RETCODE_INVALID_STRIDE = -9,
	RETCODE_WRONG_CALL_SEQUENCE = -10,
	RETCODE_CALLED_BEFORE = -12,
	RETCODE_NOT_INITIALIZED = -13,
	RETCODE_DEBLOCKING_OUTPUT_NOT_SET = -14
} RetCode;

typedef struct {
	PhysicalAddress bufY;
	PhysicalAddress bufCb;
	PhysicalAddress bufCr;
} FrameBuffer;

typedef enum {
	MIRDIR_NONE,
	MIRDIR_VER,
	MIRDIR_HOR,
	MIRDIR_HOR_VER
} MirrorDirection;

struct CodecInst;
typedef struct CodecInst EncInst;
typedef struct CodecInst DecInst;
typedef EncInst *EncHandle;
typedef DecInst *DecHandle;

typedef struct {
	int mp4_dataPartitionEnable;
	int mp4_reversibleVlcEnable;
	int mp4_intraDcVlcThr;
} EncMp4Param;

typedef struct {
	int h263_annexJEnable;
	int h263_annexKEnable;
	int h263_annexTEnable;
} EncH263Param;

typedef struct {
	int avc_constrainedIntraPredFlag;
	int avc_disableDeblk;
	int avc_deblkFilterOffsetAlpha;
	int avc_deblkFilterOffsetBeta;
	int avc_chromaQpOffset;
	int avc_audEnable;
	int avc_fmoEnable;
	int avc_fmoSliceNum;
	int avc_fmoType;
} EncAvcParam;

typedef struct {
	PhysicalAddress bitstreamBuffer;
	Uint32 bitstreamBufferSize;
	CodStd bitstreamFormat;

	int picWidth;
	int picHeight;
	Uint32 frameRateInfo;
	int bitRate;
	int initialDelay;
	int vbvBufferSize;
	int enableAutoSkip;
	int gopSize;
	int sliceMode;
	int sliceSizeMode;
	int sliceSize;
	int intraRefresh;

	int sliceReport;
	int mbReport;
	int mbQpReport;

	union {
		EncMp4Param mp4Param;
		EncH263Param h263Param;
		EncAvcParam avcParam;
	} EncStdParam;
} EncOpenParam;

typedef struct {
	int minFrameBufferCount;
} EncInitialInfo;

typedef struct {
	FrameBuffer *sourceFrame;
	int forceIPicture;
	int skipPicture;
	int quantParam;
} EncParam;

typedef struct {
	PhysicalAddress bitstreamBuffer;
	Uint32 bitstreamSize;
	int picType;
	int numOfSlices;
	Uint32 *sliceInfo;
	Uint32 *mbInfo;
	Uint32 *mbQpInfo;
} EncOutputInfo;

typedef struct {
	Uint32 *paraSet;
	int size;
} EncParamSet;

typedef struct {
	PhysicalAddress searchRamAddr;
} SearchRamParam;

typedef struct {
	PhysicalAddress buf;
	int size;
	int headerType;
} EncHeaderParam;

typedef enum {
	VOL_HEADER,	/* video object layer header */
	VOS_HEADER,	/* visual object sequence header */
	VIS_HEADER	/* video object header */
} Mp4HeaderType;

typedef enum {
	SPS_RBSP,
	PPS_RBSP
} AvcHeaderType;	

typedef struct {
	CodStd bitstreamFormat;
	PhysicalAddress bitstreamBuffer;
	int bitstreamBufferSize;
	int qpReport;
	int reorderEnable;
} DecOpenParam;

typedef struct {
	int picWidth;		// {(PicX+15)/16} * 16
	int picHeight;		// {(PicY+15)/16} * 16
	Uint32 frameRateInfo;

	int mp4_dataPartitionEnable;
	int mp4_reversibleVlcEnable;
	int mp4_shortVideoHeader;
	int h263_annexJEnable;

	int minFrameBufferCount;
	int frameBufDelay;
} DecInitialInfo;

typedef struct {
	int prescanEnable;
	int prescanMode;
	int dispReorderBuf;
	int iframeSearchEnable;
	int skipframeMode;
	int skipframeNum;
} DecParam;

typedef struct {
	int indexDecoded;
	int picType;
	int numOfErrMBs;
	PhysicalAddress qpInfo;
	int prescanresult;
} DecOutputInfo;

#define IMAGE_ENDIAN			0	/* 0 (little endian), 1 (big endian) */
#define STREAM_ENDIAN			0	/* 0 (little endian), 1 (big endian) */
#define STREAM_ENC_PIC_RESET		1
#define STREAM_ENC_PIC_FLUSH		0
#define STREAM_FULL_EMPTY_CHECK_DISABLE 0
#define ENC_MIN_BUFCOUNT		2
#define CODE_BUF_SIZE			(64 * 1024)
/* If use FMO, size is enough to be 512KByte.  dont use FMO, size should be much more than 256KByte. */
#define WORK_BUF_SIZE			(256 * 1024)
//#define WORK_BUF_SIZE			(512 * 1024) 
#define PARA_BUF2_SIZE			(4 * 1024)	/* PARA_BUF2_SIZE(1728) modified for page align */
#define PARA_BUF_SIZE			(8 * 1024)

#define SLICE_OFFSET			0x1200

#define MAX_ENC_PIC_WIDTH		720
#define MAX_ENC_PIC_HEIGHT		576

#define MAX_NUM_INSTANCE		4
#define	DEFAULT_SEARCHRAM_ADDR		0xFFFF4C00

#if STREAM_ENDIAN == 0 
	#define STD_AVC_ENDBUFDATA	0x0B01
	#define STD_MP4_ENDBUFDATA	0xB101
	#define STD_H263_ENDBUFDATA	0xFC00
#else
	#define STD_AVC_ENDBUFDATA	0x010B
	#define STD_MP4_ENDBUFDATA	0x01B1
	#define STD_H263_ENDBUFDATA	0x00FC
#endif

enum {
	MP4_DEC = 0,
	MP4_ENC = 1,
	AVC_DEC = 2,
	AVC_ENC = 3
};

enum {
	SEQ_INIT = 1,
	SEQ_END = 2,
	PIC_RUN = 3,
	SET_FRAME_BUF = 4,
	ENCODE_HEADER = 5,
	ENC_PARA_SET = 6,
	DEC_PARA_SET = 7,
	DEC_BUF_FLUSH = 8,
	FIRMWARE_GET = 0xf 
};

#define	IRQ_LSB				(1<<0)
#define	IRQ_SEQ_INIT			(1<<1)
#define	IRQ_SEQ_END			(1<<2)
#define	IRQ_PIC_RUN			(1<<3)
#define	IRQ_DEC_BUF_EMPTY		(1<<4)
#define	IRQ_ENC_BUF_FULL		(1<<5)

typedef struct {
	EncOpenParam openParam;
	EncInitialInfo initialInfo;

	PhysicalAddress streamRdPtr;
	PhysicalAddress streamRdPtrRegAddr;
	PhysicalAddress streamWrPtrRegAddr;
	PhysicalAddress streamBufStartAddr;
	PhysicalAddress streamBufEndAddr;
	int streamBufSize;
	FrameBuffer *frameBufPool;
	int numFrameBuffers;
	int stride;
	int rotationEnable;
	int mirrorEnable;
	MirrorDirection mirrorDirection;
	int rotationAngle;
	int initialInfoObtained;
} EncInfo;

typedef struct {
	DecOpenParam openParam;
	DecInitialInfo initialInfo;

	PhysicalAddress streamWrPtr;
	PhysicalAddress streamRdPtrRegAddr;
	PhysicalAddress streamWrPtrRegAddr;
	PhysicalAddress streamBufStartAddr;
	PhysicalAddress streamBufEndAddr;
	int streamBufSize;
	FrameBuffer *frameBufPool;
	int numFrameBuffers;
	FrameBuffer *recFrame;
	int stride;
	int rotationEnable;
	int mirrorEnable;
	MirrorDirection mirrorDirection;
	int rotationAngle;
	FrameBuffer rotatorOutput;
	int rotatorStride;
	int rotatorOutputValid;
	int initialInfoObtained;
} DecInfo;

typedef struct CodecInst {
	int instIndex;
	int inUse;
	int codecMode;
	union {
		EncInfo encInfo;
		DecInfo decInfo;
	} CodecInfo;
} CodecInst;

RetCode vpu_Init(PhysicalAddress workBuf);
RetCode vpu_GetVersionInfo();
int vpu_IsBusy(void);

RetCode vpu_EncOpen(EncHandle *, EncOpenParam *);
RetCode vpu_EncClose(EncHandle);
RetCode vpu_EncGetInitialInfo(EncHandle, EncInitialInfo *);
RetCode vpu_EncRegisterFrameBuffer(EncHandle handle,
				   FrameBuffer * bufArray, int num, int stride);
RetCode vpu_EncGetBitstreamBuffer(EncHandle handle, PhysicalAddress * prdPrt,
				  PhysicalAddress * pwrPtr, Uint32 * size);
RetCode vpu_EncUpdateBitstreamBuffer(EncHandle handle, Uint32 size);
RetCode vpu_EncStartOneFrame(EncHandle handle, EncParam * param);
RetCode vpu_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info);
RetCode vpu_EncGiveCommand(EncHandle handle, CodecCommand cmd, void *parameter);

RetCode vpu_DecOpen(DecHandle *, DecOpenParam *);
RetCode vpu_DecClose(DecHandle);
RetCode vpu_DecGetBitstreamBuffer(DecHandle handle, PhysicalAddress * paRdPtr, 
				PhysicalAddress * paWrPtr, Uint32 * size);
RetCode vpu_DecUpdateBitstreamBuffer(DecHandle handle, Uint32 size);
RetCode vpu_DecSetEscSeqInit(DecHandle handle, int escape);
RetCode vpu_DecGetInitialInfo(DecHandle handle, DecInitialInfo * info);
RetCode vpu_DecRegisterFrameBuffer(DecHandle handle,
				   FrameBuffer * bufArray, int num, int stride);
RetCode vpu_DecStartOneFrame(DecHandle handle, DecParam *param);
RetCode vpu_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info);
RetCode vpu_DecBitBufferFlush(DecHandle handle);
RetCode vpu_DecGiveCommand(DecHandle handle, CodecCommand cmd, void *parameter);

extern int vpu_WaitForInt(int timeout_in_ms);
extern int vpu_ESDMISC_LHD(int disable);

#endif
