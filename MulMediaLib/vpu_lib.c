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
 * @file vpu_lib.c
 *
 * @brief This file implements codec API funcitons for VPU
 *
 * @ingroup VPU
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "vpu_io.h"
#include "vpu_lib.h"
#include "vpu_codetable.h"

static CodecInst codecInstPool[MAX_NUM_INSTANCE];

/* If a frame is started, pendingInst is set to the proper instance.*/
static CodecInst *pendingInst;
static CodecInst *decpendingInst;
static PhysicalAddress rdPtrRegAddr[] = {
	BIT_RD_PTR_0,
	BIT_RD_PTR_1,
	BIT_RD_PTR_2,
	BIT_RD_PTR_3
};

static PhysicalAddress wrPtrRegAddr[] = {
	BIT_WR_PTR_0,
	BIT_WR_PTR_1,
	BIT_WR_PTR_2,
	BIT_WR_PTR_3
};

/* If a frame is started, pendingInst is set to the proper instance. */
//static CodecInst *pendingInst;

static PhysicalAddress workBuffer;
static PhysicalAddress codeBuffer;
static PhysicalAddress paraBuffer;

static unsigned long *virt_paraBuf;
static unsigned long *virt_paraBuf2;

extern vpu_mem_desc bit_work_addr;

static void BitIssueCommand(int instIdx, int cdcMode, int cmd)
{
	VpuWriteReg(BIT_RUN_INDEX, instIdx);
	VpuWriteReg(BIT_RUN_COD_STD, cdcMode);
	VpuWriteReg(BIT_RUN_COMMAND, cmd);
}

/*!
 * @brief
 * This functure indicate whether processing(encoding/decoding) a frame 
 * is completed or not yet.
 *
 * @return 
 * @li 0: VPU hardware is idle.
 * @li Non-zero value: VPU hardware is busy processing a frame.
 */
int vpu_IsBusy()
{
	return VpuReadReg(BIT_BUSY_FLAG) != 0;
}

/*!
 * @brief VPU initialization.
 * This function initializes VPU hardware and proper data structures/resources. 
 * The user must call this function only once before using VPU codec.
 *
 * @param  workBuf  The physical address of a working space of the codec. 
 *  The size of the space must be at least CODE_BUF_SIZE + WORK_BUF_SIZE
 * + PARA_BUF2_SIZE + PARA_BUF_SIZE in KB.
 *
 * @return  This function always returns RETCODE_SUCCESS.
 */
RetCode vpu_Init(PhysicalAddress workBuf)
{
	int i;
	Uint32 data;
	Uint32 virt_codeBuf;
	CodecInst *pCodecInst;

	VpuWriteReg(BIT_CODE_RUN, 0);

	codeBuffer = workBuf;
	workBuffer = codeBuffer + CODE_BUF_SIZE;
	paraBuffer = workBuffer + WORK_BUF_SIZE + PARA_BUF2_SIZE;

	virt_codeBuf = (Uint32) (bit_work_addr.virt_uaddr);
	virt_paraBuf = (unsigned long *)(virt_codeBuf + CODE_BUF_SIZE +
					 WORK_BUF_SIZE + PARA_BUF2_SIZE);
	virt_paraBuf2 = (unsigned long *)(virt_codeBuf + CODE_BUF_SIZE +
					  WORK_BUF_SIZE);

	if (getChipVersion() == MX27_REV2) {
		/* Copy full Microcode to Code Buffer allocated on SDRAM */
		for (i = 0; i < sizeof(bit_code2) / sizeof(bit_code2[0]); i += 2) {
			data = (unsigned int)((bit_code2[i] << 16) | bit_code2[i + 1]);
			((unsigned int *)virt_codeBuf)[i / 2] = data;
		}

		/* Download BIT Microcode to Program Memory */
		for (i = 0; i < BIT_CODE_SIZE; ++i) {
			data = bit_code2[i];
			VpuWriteReg(BIT_CODE_DOWN, (i << 16) | data);
		}
	} else if (getChipVersion() == MX27_REV1) {
		/* Copy full Microcode to Code Buffer allocated on SDRAM */
		for (i = 0; i < sizeof(bit_code) / sizeof(bit_code[0]); i += 2) {
			data = (unsigned int)((bit_code[i] << 16) | bit_code[i + 1]);
			((unsigned int *)virt_codeBuf)[i / 2] = data;
		}

		/* Download BIT Microcode to Program Memory */
		for (i = 0; i < BIT_CODE_SIZE; ++i) {
			data = bit_code[i];
			VpuWriteReg(BIT_CODE_DOWN, (i << 16) | data);
		}
	}

	VpuWriteReg(BIT_WORK_BUF_ADDR, workBuffer);
	VpuWriteReg(BIT_PARA_BUF_ADDR, paraBuffer);
	VpuWriteReg(BIT_CODE_BUF_ADDR, codeBuffer);

	data = STREAM_FULL_EMPTY_CHECK_DISABLE << 1;
	data |= STREAM_ENDIAN;
	VpuWriteReg(BIT_BIT_STREAM_CTRL, data);
	VpuWriteReg(BIT_FRAME_MEM_CTRL, IMAGE_ENDIAN);
	VpuWriteReg(BIT_INT_ENABLE, IRQ_PIC_RUN);	/* PIC_RUN irq enable */

	ResetVpu();
	VpuWriteReg(BIT_CODE_RUN, 1);

	pCodecInst = &codecInstPool[0];
	for (i = 0; i < MAX_NUM_INSTANCE; ++i, ++pCodecInst) {
		pCodecInst->instIndex = i;
		pCodecInst->inUse = 0;
	}

	return RETCODE_SUCCESS;
}

/*!
 * @brief Get VPU Firmware Version.
 */
RetCode vpu_GetVersionInfo(vpu_versioninfo *verinfo)
{
	Uint32 ver;
	Uint16 pn;
	Uint16 version;
	char productstr[18] = {0};
	
	if (!isVpuInitialized())
		return RETCODE_NOT_INITIALIZED;

	if(pendingInst)
		return RETCODE_FRAME_NOT_COMPLETE;
	
	VpuWriteReg(RET_VER_NUM , 0);

	BitIssueCommand(0, 0, FIRMWARE_GET);
	while (VpuReadReg(BIT_BUSY_FLAG))
		;
		
	ver = VpuReadReg(RET_VER_NUM);
	
	if (ver == 0)
		return RETCODE_FAILURE;
		
	pn = (Uint16)(ver>>16);
	version = (Uint16)ver;

	switch (pn)
	{
		case PRJ_TRISTAN:
			strcpy(productstr, "i.MX27 TO1");
			break;
		case PRJ_CODA_DX_6:
			strcpy(productstr, "i.MX27 TO2");
			break;
		default: 
			strcpy(productstr, "Unknown");
			break;

	}

	printf("Product Info: %s\n", productstr);

	verinfo->fw_major = (version>>(12))&0x0f; 
	verinfo->fw_minor = (version>>(8))&0x0f; 
	verinfo->fw_release = (version)&0xff;
	
	verinfo->lib_major = (VPU_LIB_VERSION_CODE>>(12))&0x0f; 
	verinfo->lib_minor = (VPU_LIB_VERSION_CODE>>(8))&0x0f; 
	verinfo->lib_release = (VPU_LIB_VERSION_CODE)&0xff;

	return RETCODE_SUCCESS;
}

/*
 * GetCodecInstance() obtains an instance.
 * It stores a pointer to the allocated instance in *ppInst
 * and returns RETCODE_SUCCESS on success.
 * Failure results in 0(null pointer) in *ppInst and RETCODE_FAILURE.
 */
static RetCode GetCodecInstance(CodecInst ** ppInst)
{
	int i;
	CodecInst *pCodecInst;

	pCodecInst = &codecInstPool[0];
	for (i = 0; i < MAX_NUM_INSTANCE; ++i, ++pCodecInst) {
		if (!pCodecInst->inUse)
			break;
	}

	if (i == MAX_NUM_INSTANCE) {
		*ppInst = 0;
		return RETCODE_FAILURE;
	}

	pCodecInst->inUse = 1;
	*ppInst = pCodecInst;
	return RETCODE_SUCCESS;
}

static RetCode CheckInstanceValidity(CodecInst * pci)
{
	CodecInst *pCodecInst;
	int i;

	pCodecInst = &codecInstPool[0];
	for (i = 0; i < MAX_NUM_INSTANCE; ++i, ++pCodecInst) {
		if (pCodecInst == pci)
			return RETCODE_SUCCESS;
	}
	return RETCODE_INVALID_HANDLE;
}

static RetCode CheckEncInstanceValidity(EncHandle handle)
{
	CodecInst *pCodecInst;
	RetCode ret;

	pCodecInst = handle;
	ret = CheckInstanceValidity(pCodecInst);
	if (ret != RETCODE_SUCCESS) {
		return RETCODE_INVALID_HANDLE;
	}
	if (!pCodecInst->inUse) {
		return RETCODE_INVALID_HANDLE;
	}
	if (pCodecInst->codecMode != MP4_ENC &&
	    pCodecInst->codecMode != AVC_ENC) {
		return RETCODE_INVALID_HANDLE;
	}
	return RETCODE_SUCCESS;
}

static RetCode CheckDecInstanceValidity(DecHandle handle)
{
	CodecInst *pCodecInst;
	RetCode ret;

	pCodecInst = handle;
	ret = CheckInstanceValidity(pCodecInst);
	if (ret != RETCODE_SUCCESS) {
		return RETCODE_INVALID_HANDLE;
	}
	if (!pCodecInst->inUse) {
		return RETCODE_INVALID_HANDLE;
	}
	if (pCodecInst->codecMode != MP4_DEC &&
	    pCodecInst->codecMode != AVC_DEC) {
		return RETCODE_INVALID_HANDLE;
	}
	return RETCODE_SUCCESS;
}

static void FreeCodecInstance(CodecInst * pCodecInst)
{
	pCodecInst->inUse = 0;
}

static RetCode CheckEncOpenParam(EncOpenParam * pop)
{
	int picWidth;
	int picHeight;
	if (pop == 0) {
		return RETCODE_INVALID_PARAM;
	}
	picWidth = pop->picWidth;
	picHeight = pop->picHeight;
	if (pop->bitstreamBuffer % 4) {	/* not 4-bit aligned */
		return RETCODE_INVALID_PARAM;
	}
	if (pop->bitstreamBufferSize % 1024 ||
	    pop->bitstreamBufferSize < 1024 ||
	    pop->bitstreamBufferSize > 16383 * 1024) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->bitstreamFormat != STD_MPEG4 &&
	    pop->bitstreamFormat != STD_H263 &&
	    pop->bitstreamFormat != STD_AVC) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->bitRate > 32767 || pop->bitRate < 0) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->bitRate !=0 && pop->initialDelay > 32767) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->bitRate !=0 && pop->initialDelay != 0 && pop->vbvBufferSize < 0) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->enableAutoSkip != 0 && pop->enableAutoSkip != 1) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->gopSize > 60) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->sliceMode != 0 && pop->sliceMode != 1) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->sliceMode == 1) {
		if (pop->sliceSizeMode != 0 && pop->sliceSizeMode != 1) {
			return RETCODE_INVALID_PARAM;
		}
		if (pop->sliceSize == 0) {
			return RETCODE_INVALID_PARAM;
		}
	}
	if (pop->sliceReport != 0 && pop->sliceReport != 1) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->mbReport != 0 && pop->mbReport != 1) {
		return RETCODE_INVALID_PARAM;
	}

	if (pop->intraRefresh < 0 || pop->intraRefresh >= (picWidth * picHeight /256)) {
		return RETCODE_INVALID_PARAM;
	}

	if (pop->bitstreamFormat == STD_MPEG4) {
		EncMp4Param *param = &pop->EncStdParam.mp4Param;
		if (param->mp4_dataPartitionEnable != 0 &&
		    param->mp4_dataPartitionEnable != 1) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->mp4_dataPartitionEnable == 1) {
			if (param->mp4_reversibleVlcEnable != 0 &&
			    param->mp4_reversibleVlcEnable != 1) {
				return RETCODE_INVALID_PARAM;
			}
		}
		if (param->mp4_intraDcVlcThr < 0 ||
		    param->mp4_intraDcVlcThr > 7) {
			return RETCODE_INVALID_PARAM;
		}
	} else if (pop->bitstreamFormat == STD_H263) {
		EncH263Param *param = &pop->EncStdParam.h263Param;
		if (param->h263_annexJEnable != 0 &&
		    param->h263_annexJEnable != 1) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->h263_annexKEnable != 0 &&
		    param->h263_annexKEnable != 1) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->h263_annexTEnable != 0 &&
		    param->h263_annexTEnable != 1) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->h263_annexJEnable == 0 &&
		    param->h263_annexKEnable == 0 &&
		    param->h263_annexTEnable == 0) {
			if (!(picWidth == 128 && picHeight == 96) &&
			    !(picWidth == 176 && picHeight == 144) &&
			    !(picWidth == 352 && picHeight == 288) &&
			    !(picWidth == 704 && picHeight == 576)) {
				return RETCODE_INVALID_PARAM;
			}
		}
	} else if (pop->bitstreamFormat == STD_AVC) {
		EncAvcParam *param = &pop->EncStdParam.avcParam;
		if (param->avc_constrainedIntraPredFlag != 0 &&
		    param->avc_constrainedIntraPredFlag != 1) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->avc_disableDeblk != 0 &&
		    param->avc_disableDeblk != 1 &&
		    param->avc_disableDeblk != 2) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->avc_deblkFilterOffsetAlpha < -6 ||
		    param->avc_deblkFilterOffsetAlpha > 6) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->avc_deblkFilterOffsetBeta < -6 ||
		    param->avc_deblkFilterOffsetBeta > 6) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->avc_chromaQpOffset < -12 ||
		    param->avc_chromaQpOffset > 12) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->avc_audEnable != 0 && param->avc_audEnable != 1) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->avc_fmoEnable != 0 && param->avc_fmoEnable != 1) {
			return RETCODE_INVALID_PARAM;
		}
		if (param->avc_fmoEnable == 1) {
			if (param->avc_fmoType != 0 && param->avc_fmoType != 1) {
				return RETCODE_INVALID_PARAM;
			}
			if (param->avc_fmoSliceNum < 2 || 8 < param->avc_fmoSliceNum) {
				return RETCODE_INVALID_PARAM;
			}
		}
	}
	if (picWidth < 32 || picWidth > MAX_ENC_PIC_WIDTH) {
		return RETCODE_INVALID_PARAM;
	}
	if (picHeight < 16 || picHeight > MAX_ENC_PIC_HEIGHT) {
		return RETCODE_INVALID_PARAM;
	}

	return RETCODE_SUCCESS;
}

/*!
 * @brief VPU encoder initialization
 *
 * @param pHandle [Output] Pointer to EncHandle type 
    where a handle will be written by which you can refer 
    to an encoder instance. If no instance is available, 
    null handle is returned via pHandle.
 *
 * @param pop  [Input] Pointer to EncOpenParam type 
 * which describes parameters necessary for encoding.
 *
 * @return 
 * @li RETCODE_SUCCESS: Success in acquisition of an encoder instance.
 * @li RETCODE_FAILURE: Failed in acquisition of an encoder instance. 
 * @li RETCODE_INVALID_PARAM: pop is a null pointer, or some parameter 
 * passed does not satisfy conditions described in the paragraph for 
 * EncOpenParam type.
 */
RetCode vpu_EncOpen(EncHandle * pHandle, EncOpenParam * pop)
{
	CodecInst *pCodecInst;
	EncInfo *pEncInfo;
	int instIdx;
	RetCode ret;
	Uint32 val;

	if (!isVpuInitialized()) {
		return RETCODE_NOT_INITIALIZED;
	}

	ret = CheckEncOpenParam(pop);
	if (ret != RETCODE_SUCCESS) {
		return ret;
	}

	ret = GetCodecInstance(&pCodecInst);
	if (ret == RETCODE_FAILURE) {
		*pHandle = 0;
		return RETCODE_FAILURE;
	}

	*pHandle = pCodecInst;
	instIdx = pCodecInst->instIndex;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;

	pEncInfo->openParam = *pop;

	pCodecInst->codecMode =
	    pop->bitstreamFormat == STD_AVC ? AVC_ENC : MP4_ENC;
	pEncInfo->streamRdPtr = pop->bitstreamBuffer;
	pEncInfo->streamRdPtrRegAddr = rdPtrRegAddr[instIdx];
	pEncInfo->streamWrPtrRegAddr = wrPtrRegAddr[instIdx];
	pEncInfo->streamBufStartAddr = pop->bitstreamBuffer;
	pEncInfo->streamBufSize = pop->bitstreamBufferSize;
	pEncInfo->streamBufEndAddr =
	    pop->bitstreamBuffer + pop->bitstreamBufferSize;
	pEncInfo->frameBufPool = 0;

	pEncInfo->rotationEnable = 0;
	pEncInfo->mirrorEnable = 0;
	pEncInfo->mirrorDirection = MIRDIR_NONE;
	pEncInfo->rotationAngle = 0;

	pEncInfo->initialInfoObtained = 0;

	VpuWriteReg(pEncInfo->streamRdPtrRegAddr, pEncInfo->streamRdPtr);
	VpuWriteReg(pEncInfo->streamWrPtrRegAddr, pEncInfo->streamBufStartAddr);

	val = VpuReadReg(BIT_BIT_STREAM_CTRL);
	val &= 0xFFE3;
	val |= STREAM_ENC_PIC_RESET << 3;
	val |= STREAM_ENC_PIC_FLUSH << 2;
		
	VpuWriteReg(BIT_BIT_STREAM_CTRL, val);

	return RETCODE_SUCCESS;
}

/*!
 * @brief Encoder system close.
 *
 * @param encHandle [Input] The handle obtained from vpu_EncOpen().
 *
 * @return 
 * @li RETCODE_SUCCESS Successful closing.
 * @li RETCODE_INVALID_HANDLE encHandle is invalid. 
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 */
RetCode vpu_EncClose(EncHandle handle)
{
	CodecInst *pCodecInst;
	EncInfo *pEncInfo;
	RetCode ret;

	ret = CheckEncInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (pendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;
	if (pEncInfo->initialInfoObtained) {
		BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode,
				SEQ_END);
		while (VpuReadReg(BIT_BUSY_FLAG)) ;
	}

	FreeCodecInstance(pCodecInst);
	return RETCODE_SUCCESS;
}

/*!
 * @brief user could allocate frame buffers 
 * according to the information obtained from this function.
 * @param handle [Input] The handle obtained from vpu_EncOpen().
 * @param info [Output] The information required before starting 
 * encoding will be put to the data structure pointed to by initialInfo.
 *
 * @return 
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_FAILURE There was an error in getting information and 
 *                                    configuring the encoder.
 * @li RETCODE_INVALID_HANDLE encHandle is invalid. 
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished 
 * @li RETCODE_INVALID_PARAM initialInfo is a null pointer.
 */
RetCode vpu_EncGetInitialInfo(EncHandle handle, EncInitialInfo * info)
{
	CodecInst *pCodecInst;
	EncInfo *pEncInfo;
	EncOpenParam encOP;
	int picWidth;
	int picHeight;
	Uint32 data;
	RetCode ret;

	ret = CheckEncInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (pendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	if (info == 0) {
		return RETCODE_INVALID_PARAM;
	}

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;
	encOP = pEncInfo->openParam;

	if (pEncInfo->initialInfoObtained) {
		return RETCODE_CALLED_BEFORE;
	}

	picWidth = encOP.picWidth;
	picHeight = encOP.picHeight;

	data = (picWidth << 10) | picHeight;
	VpuWriteReg(CMD_ENC_SEQ_SRC_SIZE, data);
	VpuWriteReg(CMD_ENC_SEQ_SRC_F_RATE, encOP.frameRateInfo);

	if (encOP.bitstreamFormat == STD_MPEG4) {
		VpuWriteReg(CMD_ENC_SEQ_COD_STD, 0);
		data = encOP.EncStdParam.mp4Param.mp4_intraDcVlcThr << 2 |
		    encOP.EncStdParam.mp4Param.mp4_reversibleVlcEnable << 1 |
		    encOP.EncStdParam.mp4Param.mp4_dataPartitionEnable;
		VpuWriteReg(CMD_ENC_SEQ_MP4_PARA, data);
	} else if (encOP.bitstreamFormat == STD_H263) {
		VpuWriteReg(CMD_ENC_SEQ_COD_STD, 1);
		data = encOP.EncStdParam.h263Param.h263_annexJEnable << 2 |
		    encOP.EncStdParam.h263Param.h263_annexKEnable << 1 |
		    encOP.EncStdParam.h263Param.h263_annexTEnable;
		VpuWriteReg(CMD_ENC_SEQ_263_PARA, data);
	} else if (encOP.bitstreamFormat == STD_AVC) {
		VpuWriteReg(CMD_ENC_SEQ_COD_STD, 2);
		data =
		    (encOP.EncStdParam.avcParam.avc_deblkFilterOffsetBeta & 15) << 12 |
		    (encOP.EncStdParam.avcParam.avc_deblkFilterOffsetAlpha & 15) << 8 |
		    encOP.EncStdParam.avcParam.avc_disableDeblk << 6 |
		    encOP.EncStdParam.avcParam.avc_constrainedIntraPredFlag << 5 |
		    (encOP.EncStdParam.avcParam.avc_chromaQpOffset & 31);
		VpuWriteReg(CMD_ENC_SEQ_264_PARA, data);
	}

	data = encOP.sliceSize << 2 |
	    encOP.sliceSizeMode << 1 | encOP.sliceMode;
	VpuWriteReg(CMD_ENC_SEQ_SLICE_MODE, data);
	VpuWriteReg(CMD_ENC_SEQ_GOP_NUM, encOP.gopSize);

	if (encOP.bitRate) {	/* rate control enabled */
		data = (!encOP.enableAutoSkip) << 31 |
		    encOP.initialDelay << 16 | encOP.bitRate << 1 | 1;
		VpuWriteReg(CMD_ENC_SEQ_RC_PARA, data);
	} else {
		VpuWriteReg(CMD_ENC_SEQ_RC_PARA, 0);
	}
	VpuWriteReg(CMD_ENC_SEQ_RC_BUF_SIZE, encOP.vbvBufferSize);
	VpuWriteReg(CMD_ENC_SEQ_INTRA_REFRESH, encOP.intraRefresh);

	VpuWriteReg(CMD_ENC_SEQ_BB_START, pEncInfo->streamBufStartAddr);
	VpuWriteReg(CMD_ENC_SEQ_BB_SIZE, pEncInfo->streamBufSize / 1024);	/* size in KB */

	data = (encOP.sliceReport << 1) | encOP.mbReport;
	/* This flag is valid only for MPEG4 / H.263 case. */
	if (pCodecInst->codecMode == MP4_ENC) {
		data |= (encOP.mbQpReport << 3);
	}

	if (pCodecInst->codecMode == AVC_ENC) {
		data |= (encOP.EncStdParam.avcParam.avc_audEnable << 2);
		data |= (encOP.EncStdParam.avcParam.avc_fmoEnable << 4);
	}
	VpuWriteReg(CMD_ENC_SEQ_OPTION, data);

	if (pCodecInst->codecMode == AVC_ENC) {
		data = (encOP.EncStdParam.avcParam.avc_fmoType << 4) | 
			(encOP.EncStdParam.avcParam.avc_fmoSliceNum & 0x0f);
	}
	VpuWriteReg(CMD_ENC_SEQ_FMO, data);

	BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode, SEQ_INIT);
	while (VpuReadReg(BIT_BUSY_FLAG)) ;

	if (VpuReadReg(RET_ENC_SEQ_SUCCESS) == 0) {
		return RETCODE_FAILURE;
	}
	/* reconstructed frame + reference frame */
	info->minFrameBufferCount = ENC_MIN_BUFCOUNT;

	pEncInfo->initialInfo = *info;
	pEncInfo->initialInfoObtained = 1;

	return RETCODE_SUCCESS;
}

/*!
 * @brief Registers frame buffers 
 * @param handle [Input] The handle obtained from vpu_EncOpen().
 * @param bufArray [Input] Pointer to the first element of an array
 *			of FrameBuffer data structures.
 * @param num [Input] Number of elements of the array.
 * @param stride [Input] Stride value of frame buffers being registered.
 *
 * @return 
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE encHandle is invalid. 
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished 
 * @li RETCODE_WRONG_CALL_SEQUENCE Function call in wrong sequence.
 * @li RETCODE_INVALID_FRAME_BUFFER pBuffer is a null pointer.
 * @li RETCODE_INSUFFICIENT_FRAME_BUFFERS num is smaller than requested.
 * @li RETCODE_INVALID_STRIDE stride is smaller than the picture width.
 */
RetCode vpu_EncRegisterFrameBuffer(EncHandle handle,
				   FrameBuffer * bufArray, int num, int stride)
{
	CodecInst *pCodecInst;
	EncInfo *pEncInfo;
	int i;
	RetCode ret;

	ret = CheckEncInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (pendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;

	if (pEncInfo->frameBufPool) {
		return RETCODE_CALLED_BEFORE;
	}

	if (!pEncInfo->initialInfoObtained) {
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	if (bufArray == 0) {
		return RETCODE_INVALID_FRAME_BUFFER;
	}
	if (num < pEncInfo->initialInfo.minFrameBufferCount) {
		return RETCODE_INSUFFICIENT_FRAME_BUFFERS;
	}
	if (stride % 8 != 0 || stride == 0) {
		return RETCODE_INVALID_STRIDE;
	}

	pEncInfo->frameBufPool = bufArray;
	pEncInfo->numFrameBuffers = num;
	pEncInfo->stride = stride;

	/* Let the codec know the addresses of the frame buffers. */
	for (i = 0; i < num; ++i) {
		virt_paraBuf[i * 3] = bufArray[i].bufY;
		virt_paraBuf[i * 3 + 1] = bufArray[i].bufCb;
		virt_paraBuf[i * 3 + 2] = bufArray[i].bufCr;
	}
	/* Tell the codec how much frame buffers were allocated. */
	VpuWriteReg(CMD_SET_FRAME_BUF_NUM, num);
	VpuWriteReg(CMD_SET_FRAME_BUF_STRIDE, stride);

	BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode,
			SET_FRAME_BUF);

	while (vpu_IsBusy()) ;

	return RETCODE_SUCCESS;
}

RetCode vpu_EncGetBitstreamBuffer(EncHandle handle,
		PhysicalAddress *prdPrt,
		PhysicalAddress *pwrPtr,
		Uint32 *size)
{
	CodecInst *pCodecInst;
	EncInfo *pEncInfo;
	PhysicalAddress rdPtr;
	PhysicalAddress wrPtr;
	Uint32 room;
	RetCode ret;

	ret = CheckEncInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (prdPrt == 0 || pwrPtr == 0 || size == 0) {
		return RETCODE_INVALID_PARAM;
	}

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;
	rdPtr = pEncInfo->streamRdPtr;
	wrPtr = VpuReadReg(pEncInfo->streamWrPtrRegAddr);

#if STREAM_ENC_PIC_RESET == 1
	if (rdPtr == pEncInfo->streamBufStartAddr && wrPtr >= rdPtr)
		room = wrPtr - rdPtr;	
	else
		return RETCODE_INVALID_PARAM;
#else
	if (wrPtr >= rdPtr) {
		room = wrPtr - rdPtr;
	} else {
		room = (pEncInfo->streamBufEndAddr - rdPtr) + 
			(wrPtr - pEncInfo->streamBufStartAddr);
	}
#endif

	*prdPrt = rdPtr;
	*pwrPtr = wrPtr;
	*size = room;

	return RETCODE_SUCCESS;
}

RetCode vpu_EncUpdateBitstreamBuffer(
		EncHandle handle,
		Uint32 size)
{
	CodecInst *pCodecInst;
	EncInfo *pEncInfo;
	PhysicalAddress wrPtr;
	PhysicalAddress rdPtr;
	RetCode ret;
#if STREAM_ENC_PIC_RESET == 0
	int room = 0;
#endif

	ret = CheckEncInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;
	rdPtr = pEncInfo->streamRdPtr;

	wrPtr = VpuReadReg(pEncInfo->streamWrPtrRegAddr);
	if (rdPtr < wrPtr) {
		if (rdPtr + size > wrPtr)
			return RETCODE_INVALID_PARAM;
	}

#if STREAM_ENC_PIC_RESET == 1
	rdPtr = pEncInfo->streamBufStartAddr;
#else
	rdPtr += size;
	if (rdPtr > pEncInfo->streamBufEndAddr) 
	{
		room = rdPtr - pEncInfo->streamBufEndAddr;
		rdPtr = pEncInfo->streamBufStartAddr;
		rdPtr += room;
	}
	if (rdPtr == pEncInfo->streamBufEndAddr) {
		rdPtr = pEncInfo->streamBufStartAddr;
	}
#endif

	pEncInfo->streamRdPtr = rdPtr;

	VpuWriteReg(pEncInfo->streamRdPtrRegAddr, rdPtr);

	return RETCODE_SUCCESS;
}

void EncodeHeader(EncHandle handle, EncHeaderParam * encHeaderParam)
{
	CodecInst * pCodecInst;
	EncInfo * pEncInfo;
	PhysicalAddress rdPtr;
	PhysicalAddress wrPtr;		

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;

	VpuWriteReg(CMD_ENC_HEADER_CODE, encHeaderParam->headerType); // 0: SPS, 1: PPS
	BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode, ENCODE_HEADER);
	while (VpuReadReg(BIT_BUSY_FLAG))
		;
	rdPtr = VpuReadReg(pEncInfo->streamRdPtrRegAddr);
	wrPtr = VpuReadReg(pEncInfo->streamWrPtrRegAddr);
	encHeaderParam->buf= rdPtr;
	encHeaderParam->size= wrPtr - rdPtr;
}

static RetCode CheckEncParam(CodecInst * pCodecInst, EncParam * param)
{
	if (param == 0) {
		return RETCODE_INVALID_PARAM;
	}
	if (param->skipPicture != 0 && param->skipPicture != 1) {
		return RETCODE_INVALID_PARAM;
	}
	if (param->skipPicture == 0) {
		if (param->sourceFrame == 0) {
			return RETCODE_INVALID_FRAME_BUFFER;
		}
		if (param->forceIPicture != 0 && param->forceIPicture != 1) {
			return RETCODE_INVALID_PARAM;
		}
	}
	if (pCodecInst->CodecInfo.encInfo.openParam.bitRate == 0) {	/* no rate control */
		if (pCodecInst->codecMode == MP4_ENC) {
			if (param->quantParam < 1 || param->quantParam > 31) {
				return RETCODE_INVALID_PARAM;
			}
		} else {	/* AVC_ENC */
			if (param->quantParam < 0 || param->quantParam > 51) {
				return RETCODE_INVALID_PARAM;
			}
		}
	}
	return RETCODE_SUCCESS;
}

/*!
 * @brief Starts encoding one frame. 
 *
 * @param handle [Input] The handle obtained from vpu_EncOpen().
 * @param pParam [Input] Pointer to EncParam data structure.
 *
 * @return 
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE encHandle is invalid. 
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 * @li RETCODE_WRONG_CALL_SEQUENCE Wrong calling sequence.
 * @li RETCODE_INVALID_PARAM pParam is invalid. 
 * @li RETCODE_INVALID_FRAME_BUFFER skipPicture in EncParam is 0 
 * and sourceFrame in EncParam is a null pointer.
 */
RetCode vpu_EncStartOneFrame(EncHandle handle, EncParam * param)
{
	CodecInst *pCodecInst;
	EncInfo *pEncInfo;
	FrameBuffer *pSrcFrame;
	Uint32 rotMirEnable;
	Uint32 rotMirMode;
	RetCode ret;
	/* When doing pre-rotation, mirroring is applied first and rotation later,
	 * vice versa when doing post-rotation.
	 * For consistency, pre-rotation is converted to post-rotation orientation.*/
	static Uint32 rotatorModeConversion[] = {
		0, 1, 2, 3, 4, 7, 6, 5,
		6, 5, 4, 7, 2, 3, 0, 1
	};

	ret = CheckEncInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (pendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;

	if (pEncInfo->frameBufPool == 0) {	/* This means frame buffers have not been registered. */
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	ret = CheckEncParam(pCodecInst, param);
	if (ret != RETCODE_SUCCESS) {
		return ret;
	}

	pSrcFrame = param->sourceFrame;
	rotMirEnable = 0;
	rotMirMode = 0;
	if (pEncInfo->rotationEnable) {
		rotMirEnable = 0x10;	/* Enable rotator */
		switch (pEncInfo->rotationAngle) {
		case 0:
			rotMirMode |= 0x0;
			break;

		case 90:
			rotMirMode |= 0x1;
			break;

		case 180:
			rotMirMode |= 0x2;
			break;

		case 270:
			rotMirMode |= 0x3;
			break;
		}
	}
	if (pEncInfo->mirrorEnable) {
		rotMirEnable = 0x10;	/* Enable mirror */
		switch (pEncInfo->mirrorDirection) {
		case MIRDIR_NONE:
			rotMirMode |= 0x0;
			break;

		case MIRDIR_VER:
			rotMirMode |= 0x4;
			break;

		case MIRDIR_HOR:
			rotMirMode |= 0x8;
			break;

		case MIRDIR_HOR_VER:
			rotMirMode |= 0xc;
			break;

		}
	}
	rotMirMode = rotatorModeConversion[rotMirMode];
	rotMirMode |= rotMirEnable;
	VpuWriteReg(CMD_ENC_PIC_ROT_MODE, rotMirMode);
	VpuWriteReg(CMD_ENC_PIC_QS, param->quantParam);
	if (param->skipPicture) {
		VpuWriteReg(CMD_ENC_PIC_OPTION, 1);
	} else {
		VpuWriteReg(CMD_ENC_PIC_SRC_ADDR_Y, pSrcFrame->bufY);
		VpuWriteReg(CMD_ENC_PIC_SRC_ADDR_CB, pSrcFrame->bufCb);
		VpuWriteReg(CMD_ENC_PIC_SRC_ADDR_CR, pSrcFrame->bufCr);
		VpuWriteReg(CMD_ENC_PIC_OPTION,
			    param->forceIPicture << 1 & 0x2);
	}

	BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode, PIC_RUN);

	pendingInst = pCodecInst;

	return RETCODE_SUCCESS;
}

/*!
 * @brief Get information of the output of encoding. 
 *
 * @param encHandle [Input] The handle obtained from vpu_EncOpen().
 * @param info [Output] Pointer to EncOutputInfo data structure.
 *
 * @return 
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE encHandle is invalid. 
 * @li RETCODE_WRONG_CALL_SEQUENCE Wrong calling sequence.
 * @li RETCODE_INVALID_PARAM info is a null pointer.
 */
RetCode vpu_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info)
{
	CodecInst *pCodecInst;
	EncInfo *pEncInfo;
	RetCode ret;
#if STREAM_ENC_PIC_RESET == 1
	PhysicalAddress rdPtr;
	PhysicalAddress wrPtr;
#endif

	ret = CheckEncInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (info == 0) {
		return RETCODE_INVALID_PARAM;
	}

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;

	if (pendingInst == 0) {
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	if (pCodecInst != pendingInst) {
		return RETCODE_INVALID_HANDLE;
	}

	info->picType = VpuReadReg(RET_ENC_PIC_TYPE);

#if STREAM_ENC_PIC_RESET == 1
	rdPtr = pEncInfo->streamBufStartAddr;
	wrPtr = VpuReadReg(pEncInfo->streamWrPtrRegAddr);
	info->bitstreamBuffer = rdPtr;
	info->bitstreamSize = wrPtr - rdPtr;
#endif
	info->numOfSlices = VpuReadReg(RET_ENC_PIC_SLICE_NUM);
	info->sliceInfo = virt_paraBuf + SLICE_OFFSET;
	info->mbInfo = virt_paraBuf;

	if (pCodecInst->codecMode == MP4_ENC && 
	    pEncInfo->openParam.mbQpReport == 1) {
		int widthInMB;
		int heightInMB;
		PhysicalAddress readPnt;
		PhysicalAddress writePnt;
		PhysicalAddress mbQpPnt;
		int i;
		int j;
		Uint32 val, val1, val2;

		mbQpPnt = (Uint32)virt_paraBuf + 0x1300;
		widthInMB = pEncInfo->openParam.picWidth / 16;
		heightInMB = pEncInfo->openParam.picHeight / 16;
		writePnt = (Uint32)virt_paraBuf - PARA_BUF2_SIZE;
		for (i = 0; i < heightInMB; ++i) 
		{
			readPnt = mbQpPnt + i * 128;
			for (j = 0; j < widthInMB; j += 4) 
			{
				val1 = VpuReadMem(readPnt);
				readPnt += 4;
				val2 = VpuReadMem(readPnt);
				readPnt += 4;
				val = (val1 << 8 & 0xff000000) | (val1 << 16) |
					(val2 >> 8) | (val2 & 0x000000ff);
				VpuWriteMem(writePnt, val);
				writePnt += 4;
			}
		}
		info->mbQpInfo = virt_paraBuf - PARA_BUF2_SIZE;
	}
	pendingInst = 0;

	return RETCODE_SUCCESS;
}

static void GetParaSet(EncHandle handle, int paraSetType, EncParamSet * para)
{
	CodecInst *pCodecInst;
	EncInfo *pEncInfo;

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;

	VpuWriteReg(CMD_ENC_PARA_SET_TYPE, paraSetType);	/* SPS: 0, PPS: 1, VOS: 1, VO: 2, VOL: 0 */
	BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode,
			ENC_PARA_SET);
	while (VpuReadReg(BIT_BUSY_FLAG)) ;

	para->paraSet = virt_paraBuf;
	para->size = VpuReadReg(RET_ENC_PARA_SET_SIZE);
}

/*!
 * @brief This function gives a command to the encoder. 
 *
 * @param encHandle [Input] The handle obtained from vpu_EncOpen().
 * @param cmd [Intput] user command.
 * @param param [Intput/Output] param  for cmd. 
 *
 * @return 
 * @li RETCODE_INVALID_COMMAND cmd is not one of 8 values above.
 * @li RETCODE_INVALID_HANDLE encHandle is invalid. 
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished
 */
RetCode vpu_EncGiveCommand(EncHandle handle, CodecCommand cmd, void *param)
{
	CodecInst *pCodecInst;
	EncInfo *pEncInfo;
	RetCode ret;

	ret = CheckEncInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (pendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	pCodecInst = handle;
	pEncInfo = &pCodecInst->CodecInfo.encInfo;
	switch (cmd) {
	case ENABLE_ROTATION:
		{
			pEncInfo->rotationEnable = 1;
			break;
		}

	case DISABLE_ROTATION:
		{
			pEncInfo->rotationEnable = 0;
			break;
		}

	case ENABLE_MIRRORING:
		{
			pEncInfo->mirrorEnable = 1;
			break;
		}

	case DISABLE_MIRRORING:
		{
			pEncInfo->mirrorEnable = 0;
			break;
		}

	case SET_MIRROR_DIRECTION:
		{
			MirrorDirection mirDir;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			mirDir = *(MirrorDirection *) param;
			if (mirDir < MIRDIR_NONE || mirDir > MIRDIR_HOR_VER) {
				return RETCODE_INVALID_PARAM;
			}
			pEncInfo->mirrorDirection = mirDir;
			break;
		}

	case SET_ROTATION_ANGLE:
		{
			int angle;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			angle = *(int *)param;
			if (angle != 0 && angle != 90 &&
			    angle != 180 && angle != 270) {
				return RETCODE_INVALID_PARAM;
			}
			if (pEncInfo->initialInfoObtained && (angle == 90 || angle ==270)) {
				return RETCODE_INVALID_PARAM;
			}
			pEncInfo->rotationAngle = angle;
			break;
		}

	case ENC_GET_SPS_RBSP:
		{
			if (pCodecInst->codecMode != AVC_ENC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			GetParaSet(handle, 0, (EncParamSet * )param);
			break;
		}

	case ENC_GET_PPS_RBSP:
		{
			if (pCodecInst->codecMode != AVC_ENC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			GetParaSet(handle, 1, (EncParamSet * )param);
			break;
		}

	case ENC_PUT_MP4_HEADER:
		{
			EncHeaderParam * encHeaderParam;

			if (pCodecInst->codecMode != MP4_ENC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}				
			encHeaderParam = (EncHeaderParam *)param;
			if (!( VOL_HEADER<=encHeaderParam->headerType && 
				encHeaderParam->headerType <= VIS_HEADER)) {
				return RETCODE_INVALID_PARAM;
			}
			EncodeHeader(handle, encHeaderParam);
			break;
		}

	case ENC_PUT_AVC_HEADER:					
		{
			EncHeaderParam * encHeaderParam;
			
			if (pCodecInst->codecMode != AVC_ENC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}				
			encHeaderParam = (EncHeaderParam *)param;
			if (!( SPS_RBSP<=encHeaderParam->headerType && 
				encHeaderParam->headerType <= PPS_RBSP)) {
				return RETCODE_INVALID_PARAM;
			}
			EncodeHeader(handle, encHeaderParam);
			break;
		}
	case ENC_SET_SEARCHRAM_PARAM:
		{
			SearchRamParam *scRamParam = NULL;
			int EncPicX;
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}

			EncPicX = pCodecInst->CodecInfo.encInfo.openParam.picWidth;

			scRamParam = (SearchRamParam *)param;

			VpuWriteReg(BIT_SEARCH_RAM_BASE_ADDR, scRamParam->searchRamAddr);

			break;
		}
	case ENC_GET_VOS_HEADER:
		{
			if (pCodecInst->codecMode != MP4_ENC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			GetParaSet(handle, 1, (EncParamSet * )param); 
			break;
		}
	case ENC_GET_VO_HEADER:
		{
			if (pCodecInst->codecMode != MP4_ENC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			GetParaSet(handle, 2,(EncParamSet * ) param); 
			break;
		}
	case ENC_GET_VOL_HEADER:
		{
			if (pCodecInst->codecMode != MP4_ENC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			GetParaSet(handle, 0, (EncParamSet * )param); 
			break;
		}
	default:
		return RETCODE_INVALID_COMMAND;
	}
	return RETCODE_SUCCESS;
}

static RetCode CheckDecOpenParam(DecOpenParam * pop)
{
	if (pop == 0) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->bitstreamBuffer % 4) {	/* not 4-bit aligned */
		return RETCODE_INVALID_PARAM;
	}
	if (pop->bitstreamBufferSize % 1024 ||
	    pop->bitstreamBufferSize < 1024 ||
	    pop->bitstreamBufferSize > 16383 * 1024) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->bitstreamFormat != STD_MPEG4 &&
	    pop->bitstreamFormat != STD_H263 &&
	    pop->bitstreamFormat != STD_AVC) {
		return RETCODE_INVALID_PARAM;
	}
	if (pop->bitstreamFormat == STD_MPEG4 ||
	    pop->bitstreamFormat == STD_H263) {
		if (pop->qpReport != 0 && pop->qpReport != 1) {
			return RETCODE_INVALID_PARAM;
		}
	}
	return RETCODE_SUCCESS;
}

/*!
 * @brief Decoder initialization
 *
 * @param pHandle [Output] Pointer to DecHandle type
 * @param pop [Input] Pointer to DecOpenParam type.
 *
 * @return 
 * @li RETCODE_SUCCESS Success in acquisition of a decoder instance.
 * @li RETCODE_FAILURE Failed in acquisition of a decoder instance. 
 * @li RETCODE_INVALID_PARAM pop is a null pointer or invalid.
 */
RetCode vpu_DecOpen(DecHandle * pHandle, DecOpenParam * pop)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	int instIdx;
	RetCode ret;

	if (!isVpuInitialized()) {
		return RETCODE_NOT_INITIALIZED;
	}

	ret = CheckDecOpenParam(pop);
	if (ret != RETCODE_SUCCESS) {
		return ret;
	}

	ret = GetCodecInstance(&pCodecInst);
	if (ret == RETCODE_FAILURE) {
		*pHandle = 0;
		return RETCODE_FAILURE;
	}

	*pHandle = pCodecInst;
	instIdx = pCodecInst->instIndex;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	pDecInfo->openParam = *pop;

	pCodecInst->codecMode =
	    pop->bitstreamFormat == STD_AVC ? AVC_DEC : MP4_DEC;
	pDecInfo->streamWrPtr = pop->bitstreamBuffer;
	pDecInfo->streamRdPtrRegAddr = rdPtrRegAddr[instIdx];
	pDecInfo->streamWrPtrRegAddr = wrPtrRegAddr[instIdx];
	pDecInfo->streamBufStartAddr = pop->bitstreamBuffer;
	pDecInfo->streamBufSize = pop->bitstreamBufferSize;
	pDecInfo->streamBufEndAddr =
	    pop->bitstreamBuffer + pop->bitstreamBufferSize;
	pDecInfo->frameBufPool = 0;

	pDecInfo->rotationEnable = 0;
	pDecInfo->mirrorEnable = 0;
	pDecInfo->mirrorDirection = MIRDIR_NONE;
	pDecInfo->rotationAngle = 0;
	pDecInfo->rotatorOutputValid = 0;
	pDecInfo->rotatorStride = 0;

	pDecInfo->initialInfoObtained = 0;

	VpuWriteReg(pDecInfo->streamRdPtrRegAddr, pDecInfo->streamBufStartAddr);
	VpuWriteReg(pDecInfo->streamWrPtrRegAddr, pDecInfo->streamWrPtr);

	return RETCODE_SUCCESS;
}

/*!
 * @brief Decoder close function
 *
 * @param  handle [Input] The handle obtained from vpu_DecOpen().
 *
 * @return 
 * @li RETCODE_SUCCESS Successful closing.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid. 
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 */
RetCode vpu_DecClose(DecHandle handle)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	RetCode ret;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (decpendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	if (pDecInfo->initialInfoObtained) {
		BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode,
				SEQ_END);
		while (VpuReadReg(BIT_BUSY_FLAG)) ;
	}
	FreeCodecInstance(pCodecInst);
	return RETCODE_SUCCESS;
}

/*!
 * @brief Get bitstream for decoder. 
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param bufAddr [Output] Bitstream buffer physical address.
 * @param size [Output] Bitstream size.
 *
 * @return 
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid. 
 * @li RETCODE_INVALID_PARAM buf or size is invalid.
 */
RetCode vpu_DecGetBitstreamBuffer(DecHandle handle,
		PhysicalAddress * paRdPtr,
		PhysicalAddress * paWrPtr,
		Uint32 * size)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	PhysicalAddress rdPtr;
	PhysicalAddress wrPtr;
	Uint32 room;
	RetCode ret;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (paRdPtr == 0 || paWrPtr == 0 || size == 0) {
		return RETCODE_INVALID_PARAM;
	}

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;
	rdPtr = VpuReadReg(pDecInfo->streamRdPtrRegAddr);
	wrPtr = pDecInfo->streamWrPtr;

	if (wrPtr < rdPtr) {
		room = rdPtr - wrPtr - 1;
	} else {
		room = (pDecInfo->streamBufEndAddr - wrPtr) + 
				(rdPtr - pDecInfo->streamBufStartAddr) - 1;
	}

	*paRdPtr = rdPtr;
	*paWrPtr = wrPtr;
	*size = room;

	return RETCODE_SUCCESS;
}

/*!
 * @brief Update the current bit stream position.
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param size [Input] Size of bit stream you put.
 *
 * @return 
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid.
 * @li RETCODE_INVALID_PARAM Invalid input parameters.
 */
RetCode vpu_DecUpdateBitstreamBuffer(DecHandle handle, Uint32 size)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	PhysicalAddress wrPtr;
	PhysicalAddress rdPtr;
	RetCode ret;
//	int i = 0;
	int room = 0;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;
	wrPtr = pDecInfo->streamWrPtr;

#if 0
	if (size == 0) {
		if(pDecInfo->openParam.bitstreamFormat == STD_AVC) {
			((unsigned int *)wrPtr)[i] = 0x0;
			i += 4;
			((unsigned int *)wrPtr)[i] = STD_AVC_ENDBUFDATA;
			i += 4;
			while(i < 512)
			{
				((unsigned int *)wrPtr)[i] = 0x0;
				i += 4;
			}
		} else {
			if (pDecInfo->initialInfo.mp4_shortVideoHeader == 1)
			{
				((unsigned int *)wrPtr)[i] = 0x0;
				i += 4;
				((unsigned int *)wrPtr)[i] = STD_H263_ENDBUFDATA;
				i += 4;
				while(i < 512)
				{
					((unsigned int *)wrPtr)[i] = 0x0;
					i += 4;
				}
			} else {
				((unsigned int *)wrPtr)[i] = 0x0;
				i += 4;
				((unsigned int *)wrPtr)[i] = STD_MP4_ENDBUFDATA;
				i += 4;
				while(i < 512)
				{
					((unsigned int *)wrPtr)[i] = 0x0;
					i += 4;
				}
			}
		}
		
		size = 512;
	}
#endif

	rdPtr = VpuReadReg(pDecInfo->streamRdPtrRegAddr);
	if (wrPtr < rdPtr) {
		if (rdPtr <= wrPtr + size)
			return RETCODE_INVALID_PARAM;
	}
	wrPtr += size;
	if (wrPtr > pDecInfo->streamBufEndAddr) {
		room = wrPtr - pDecInfo->streamBufEndAddr;
		wrPtr = pDecInfo->streamBufStartAddr;
		wrPtr += room;
	}
	if (wrPtr == pDecInfo->streamBufEndAddr) {
		wrPtr = pDecInfo->streamBufStartAddr;
	}

	pDecInfo->streamWrPtr = wrPtr;
	VpuWriteReg(pDecInfo->streamWrPtrRegAddr, wrPtr);

	return RETCODE_SUCCESS;
}

static int DecBitstreamBufEmpty(DecInfo * pDecInfo)
{
	return VpuReadReg(pDecInfo->streamRdPtrRegAddr) ==
	    VpuReadReg(pDecInfo->streamWrPtrRegAddr);
}

RetCode vpu_DecSetEscSeqInit( DecHandle handle, int escape )
{
	CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	RetCode ret;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;
	
	VpuWriteReg(CMD_DEC_SEQ_INIT_ESCAPE, (escape & 0x01));	

	return RETCODE_SUCCESS;
}

/*!
 * @brief Get header information of bitstream.
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param info [Output] Pointer to DecInitialInfo data structure.
 *
 * @return 
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_FAILURE There was an error in getting initial information.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid.
 * @li RETCODE_INVALID_PARAM info is an invalid pointer.
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 * @li RETCODE_WRONG_CALL_SEQUENCE Wrong calling sequence.
 */
RetCode vpu_DecGetInitialInfo(DecHandle handle, DecInitialInfo * info)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	Uint32 val;
	RetCode ret;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (info == 0) {
		return RETCODE_INVALID_PARAM;
	}
	if (decpendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	if (pDecInfo->initialInfoObtained) {
		return RETCODE_CALLED_BEFORE;
	}
	if (DecBitstreamBufEmpty(pDecInfo)) {
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	VpuWriteReg(CMD_DEC_SEQ_BB_START, pDecInfo->streamBufStartAddr);
	VpuWriteReg(CMD_DEC_SEQ_BB_SIZE, pDecInfo->streamBufSize / 1024);	/* size in KBytes */
	val = ((pDecInfo->openParam.reorderEnable << 1) & 0x2) | 
				(pDecInfo->openParam.qpReport & 0x1);
	VpuWriteReg(CMD_DEC_SEQ_OPTION, val & 0x3);	
				
	BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode, SEQ_INIT);
	while (VpuReadReg(BIT_BUSY_FLAG)) ;
	if (VpuReadReg(RET_DEC_SEQ_SUCCESS) == 0)
		return RETCODE_FAILURE;

	val = VpuReadReg(RET_DEC_SEQ_SRC_SIZE);
	info->picWidth = ((val >> 10) & 0x3ff);
	info->picHeight = (val & 0x3ff);
	val = VpuReadReg(RET_DEC_SEQ_SRC_F_RATE);
	info->frameRateInfo = val;

	if (pCodecInst->codecMode != AVC_DEC) {
		val = VpuReadReg(RET_DEC_SEQ_INFO);
		info->mp4_shortVideoHeader = (val >> 2) & 1;
		info->mp4_dataPartitionEnable = val & 1;
		info->mp4_reversibleVlcEnable =
		    info->mp4_dataPartitionEnable ? ((val >> 1) & 1) : 0;
		info->h263_annexJEnable = (val >> 3) & 1;
	}

	info->minFrameBufferCount = VpuReadReg(RET_DEC_SEQ_FRAME_NEED);
	info->frameBufDelay = VpuReadReg(RET_DEC_SEQ_FRAME_DELAY);

	pDecInfo->initialInfo = *info;
	pDecInfo->initialInfoObtained = 1;

	return RETCODE_SUCCESS;
}

/*!
 * @brief Register decoder frame buffers.
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param bufArray [Input] Pointer to the first element of an array of FrameBuffer.
 * @param num [Input] Number of elements of the array.
 * @param stride [Input] Stride value of frame buffers being registered.
 *
 * @return 
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid. 
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 * @li RETCODE_WRONG_CALL_SEQUENCE Wrong calling sequence.
 * @li RETCODE_INVALID_FRAME_BUFFER Buffer is an invalid pointer.
 * @li RETCODE_INSUFFICIENT_FRAME_BUFFERS num is less than 
 * the value requested by vpu_DecGetInitialInfo().
 * @li RETCODE_INVALID_STRIDE stride is less than the picture width.
 */
RetCode vpu_DecRegisterFrameBuffer(DecHandle handle,
				   FrameBuffer * bufArray, int num, int stride)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	int i;
	RetCode ret;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (decpendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	if (pDecInfo->frameBufPool) {
		return RETCODE_CALLED_BEFORE;
	}

	if (!pDecInfo->initialInfoObtained) {
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	if (bufArray == 0) {
		return RETCODE_INVALID_FRAME_BUFFER;
	}
	if (num < pDecInfo->initialInfo.minFrameBufferCount) {
		return RETCODE_INSUFFICIENT_FRAME_BUFFERS;
	}
	if (stride < pDecInfo->initialInfo.picWidth || stride % 8 != 0) {
		return RETCODE_INVALID_STRIDE;
	}

	pDecInfo->frameBufPool = bufArray;
	pDecInfo->numFrameBuffers = num;
	pDecInfo->stride = stride;

	/* Let the codec know the addresses of the frame buffers. */
	for (i = 0; i < num; ++i) {
		virt_paraBuf[i * 3] = bufArray[i].bufY;
		virt_paraBuf[i * 3 + 1] = bufArray[i].bufCb;
		virt_paraBuf[i * 3 + 2] = bufArray[i].bufCr;
	}
	/* Tell the decoder how much frame buffers were allocated. */
	VpuWriteReg(CMD_SET_FRAME_BUF_NUM, num);
	VpuWriteReg(CMD_SET_FRAME_BUF_STRIDE, stride);

	BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode,
			SET_FRAME_BUF);

	while (vpu_IsBusy()) ;

	return RETCODE_SUCCESS;
}

/*!
 * @brief Start decoding one frame. 
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 *
 * @return 
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid. 
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 * @li RETCODE_WRONG_CALL_SEQUENCE Wrong calling sequence.
 */
RetCode vpu_DecStartOneFrame(DecHandle handle, DecParam *param)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	Uint32 rotMir;
	Uint32 val;
	RetCode ret;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (decpendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	if (pDecInfo->frameBufPool == 0) {	/* This means frame buffers have not been registered. */
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

#if 0
	/*
	 * Enable IRQ_DEC_BUF_EMPTY here to avoid this interrupt at the beginning of decoding, 
	 * because at that time the rdPtr is equal to wrPtr which means the decoder buffer is empty.
	 */
	VpuWriteReg(BIT_INT_ENABLE, IRQ_PIC_RUN | IRQ_DEC_BUF_EMPTY);
#endif

	rotMir = 0;
	if (pDecInfo->rotationEnable) {
		rotMir |= 0x10;	/* Enable rotator */
		switch (pDecInfo->rotationAngle) {
		case 0:
			rotMir |= 0x0;
			break;

		case 90:
			rotMir |= 0x1;
			break;

		case 180:
			rotMir |= 0x2;
			break;

		case 270:
			rotMir |= 0x3;
			break;
		}
	}
	if (pDecInfo->mirrorEnable) {
		rotMir |= 0x10;	/* Enable mirror */
		switch (pDecInfo->mirrorDirection) {
		case MIRDIR_NONE:
			rotMir |= 0x0;
			break;

		case MIRDIR_VER:
			rotMir |= 0x4;
			break;

		case MIRDIR_HOR:
			rotMir |= 0x8;
			break;

		case MIRDIR_HOR_VER:
			rotMir |= 0xc;
			break;

		}
	}

	if (rotMir & 0x10) {	/* rotator enabled */
		VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_Y,
			    pDecInfo->rotatorOutput.bufY);
		VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_CB,
			    pDecInfo->rotatorOutput.bufCb);
		VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_CR,
			    pDecInfo->rotatorOutput.bufCr);
		VpuWriteReg(CMD_DEC_PIC_ROT_STRIDE, pDecInfo->rotatorStride);
	}
	VpuWriteReg(CMD_DEC_PIC_ROT_MODE, rotMir);

	if (param->iframeSearchEnable == 1) /* if iframeSearch is Enable, other bit is ignore; */
		val = (param->iframeSearchEnable << 2) & 0x4;
	else
		val = (param->skipframeMode << 3) | 
			(param->iframeSearchEnable << 2) | 
			(param->prescanMode << 1) | 
			param->prescanEnable;
	
	VpuWriteReg( CMD_DEC_PIC_OPTION, val );

	VpuWriteReg( CMD_DEC_PIC_SKIP_NUM, param->skipframeNum );

	if (pDecInfo->openParam.reorderEnable == 1 && 
		pCodecInst->codecMode == AVC_DEC) {
		VpuWriteReg(CMD_DEC_DISPLAY_REORDER, param->dispReorderBuf << 1 | 
					VpuReadReg(CMD_DEC_DISPLAY_REORDER));		
	}

	BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode, PIC_RUN);
	decpendingInst = pCodecInst;

	return RETCODE_SUCCESS;
}

/*!
 * @brief Get the information of output of decoding. 
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param info [Output] Pointer to DecOutputInfo data structure.
 *
 * @return 
 * @li RETCODE_SUCCESS Successful operation.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid.
 * @li RETCODE_WRONG_CALL_SEQUENCE Wrong calling sequence.
 * @li RETCODE_INVALID_PARAM Info is an invalid pointer.
 */
RetCode vpu_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	RetCode ret;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (info == 0) {
		return RETCODE_INVALID_PARAM;
	}

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	if (decpendingInst == 0) {
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	if (pCodecInst != decpendingInst) {
		return RETCODE_INVALID_HANDLE;
	}

	info->indexDecoded = VpuReadReg(RET_DEC_PIC_FRAME_IDX);
	info->picType = VpuReadReg(RET_DEC_PIC_TYPE);
	info->numOfErrMBs = VpuReadReg(RET_DEC_PIC_ERR_MB);

	if (pCodecInst->codecMode == MP4_DEC &&
	    pDecInfo->openParam.qpReport == 1) {
		int widthInMB;
		int heightInMB;
		int readPnt;
		int writePnt;
		int i;
		int j;
		Uint32 val, val1, val2;

		widthInMB = pDecInfo->initialInfo.picWidth / 16;
		heightInMB = pDecInfo->initialInfo.picHeight / 16;
		writePnt = 0;
		for (i = 0; i < heightInMB; ++i) {
			readPnt = i * 32;
			for (j = 0; j < widthInMB; j += 4) {
				val1 = virt_paraBuf[readPnt];
				readPnt += 1;
				val2 = virt_paraBuf[readPnt];
				readPnt += 1;
				val = (val1 << 8 & 0xff000000) | (val1 << 16) |
				    (val2 >> 8) | (val2 & 0x000000ff);
				virt_paraBuf2[writePnt] = val;
				writePnt += 1;
			}
		}
		info->qpInfo = paraBuffer - PARA_BUF2_SIZE;
	}

	info->prescanresult = VpuReadReg(RET_DEC_PIC_OPTION);	
	decpendingInst = 0;

	return RETCODE_SUCCESS;
}

RetCode vpu_DecBitBufferFlush(DecHandle handle)
{
	CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	RetCode ret;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (decpendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	if (pDecInfo->frameBufPool == 0) { /* This means frame buffers have not been registered. */
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode, DEC_BUF_FLUSH);

	while (vpu_IsBusy());

	pDecInfo->streamWrPtr = pDecInfo->streamBufStartAddr;

	VpuWriteReg(pDecInfo->streamWrPtrRegAddr, pDecInfo->streamBufStartAddr);	
	VpuWriteReg(pDecInfo->streamRdPtrRegAddr, pDecInfo->streamBufStartAddr);

	return RETCODE_SUCCESS;
}

static void SetParaSet(DecHandle handle, int paraSetType, DecParamSet * para)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	int i;
	Uint32 *src;
	int byteSize;
	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	src = para->paraSet;
	byteSize = para->size / 4;
	for (i = 0; i < byteSize; i += 1) {
		virt_paraBuf[i] = *src++;
	}
	VpuWriteReg(CMD_DEC_PARA_SET_TYPE, paraSetType);
	VpuWriteReg(CMD_DEC_PARA_SET_SIZE, para->size);
	BitIssueCommand(pCodecInst->instIndex, pCodecInst->codecMode,
			DEC_PARA_SET);
	while (VpuReadReg(BIT_BUSY_FLAG)) ;
}

/*!
 * @brief Give command to the decoder. 
 *
 * @param handle [Input] The handle obtained from vpu_DecOpen().
 * @param cmd [Intput] Command.
 * @param param [Intput/Output] param  for cmd. 
 *
 * @return 
 * @li RETCODE_INVALID_COMMANDcmd is not valid.
 * @li RETCODE_INVALID_HANDLE decHandle is invalid. 
 * @li RETCODE_FRAME_NOT_COMPLETE A frame has not been finished.
 */
RetCode vpu_DecGiveCommand(DecHandle handle, CodecCommand cmd, void *param)
{
	CodecInst *pCodecInst;
	DecInfo *pDecInfo;
	RetCode ret;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (decpendingInst) {
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;
	switch (cmd) {
	case ENABLE_ROTATION:
		{
			if (!pDecInfo->rotatorOutputValid) {
				return RETCODE_ROTATOR_OUTPUT_NOT_SET;
			}
			if (pDecInfo->rotatorStride == 0) {
				return RETCODE_ROTATOR_STRIDE_NOT_SET;
			}
			pDecInfo->rotationEnable = 1;
			break;
		}

	case DISABLE_ROTATION:
		{
			pDecInfo->rotationEnable = 0;
			break;
		}

	case ENABLE_MIRRORING:
		{
			if (!pDecInfo->rotatorOutputValid) {
				return RETCODE_ROTATOR_OUTPUT_NOT_SET;
			}
			if (pDecInfo->rotatorStride == 0) {
				return RETCODE_ROTATOR_STRIDE_NOT_SET;
			}
			pDecInfo->mirrorEnable = 1;
			break;
		}

	case DISABLE_MIRRORING:
		{
			pDecInfo->mirrorEnable = 0;
			break;
		}

	case SET_MIRROR_DIRECTION:
		{
			MirrorDirection mirDir;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			mirDir = *(MirrorDirection *) param;
			if (mirDir < MIRDIR_NONE || mirDir > MIRDIR_HOR_VER) {
				return RETCODE_INVALID_PARAM;
			}
			pDecInfo->mirrorDirection = mirDir;
			break;
		}

	case SET_ROTATION_ANGLE:
		{
			int angle;
			int height, width;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			angle = *(int *)param;
			if (angle != 0 && angle != 90 &&
			    angle != 180 && angle != 270) {
				return RETCODE_INVALID_PARAM;
			}
			if (pDecInfo->rotatorStride != 0) {				
				height = pDecInfo->initialInfo.picHeight;
				width = pDecInfo->initialInfo.picWidth;

				if (angle == 90 || angle ==270) {
					if (height > pDecInfo->rotatorStride) {
						return RETCODE_INVALID_PARAM;
					}
				} else {
					if (width > pDecInfo->rotatorStride) {
						return RETCODE_INVALID_PARAM;
					}
				}
			}

			pDecInfo->rotationAngle = angle;
			break;
		}

	case SET_ROTATOR_OUTPUT:
		{
			FrameBuffer *frame;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			frame = (FrameBuffer *) param;
			pDecInfo->rotatorOutput = *frame;
			pDecInfo->rotatorOutputValid = 1;
			break;
		}

	case SET_ROTATOR_STRIDE:
		{
			int stride;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			stride = *(int *)param;
			if (stride % 8 != 0 || stride == 0) {
				return RETCODE_INVALID_STRIDE;
			}
			if (pDecInfo->rotationAngle == 90 || 
					pDecInfo->rotationAngle == 270) {
				if (pDecInfo->initialInfo.picHeight > stride) {
					return RETCODE_INVALID_STRIDE;
				}
			} else {
				if (pDecInfo->initialInfo.picWidth > stride) {
					return RETCODE_INVALID_STRIDE;
				}					
			}

			pDecInfo->rotatorStride = stride;
			break;
		}
	case DEC_SET_SPS_RBSP:
		{
			if (pCodecInst->codecMode != AVC_DEC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}

			SetParaSet(handle, 0, param);
			break;
		}

	case DEC_SET_PPS_RBSP:
		{
			if (pCodecInst->codecMode != AVC_DEC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}

			SetParaSet(handle, 1, param);
			break;
		}

	default:
		return RETCODE_INVALID_COMMAND;
	}

	return RETCODE_SUCCESS;
}

/* 
 * encHeaderType ENC_GET_VOS_HEADER, ENC_GET_VO_HEADER, ENC_GET_VOL_HEADER, ENC_GET_SPS_RBSP, ENC_GET_PPS_RBSP
 */
void SaveGetEncodeHeader(EncHandle handle, int encHeaderType, char *filename)
{
	FILE *fp = NULL;
	Uint8 *pHeader = NULL;
	EncParamSet encHeaderParam = {0};
	int  i;
	Uint32 dword1, dword2;
	Uint32 *pBuf;
	Uint32 byteSize;

	assert(filename != NULL);
	
	vpu_EncGiveCommand(handle, encHeaderType, &encHeaderParam); 
	byteSize = ((encHeaderParam.size + 3) & ~3);
	pHeader = (Uint8 *)malloc(sizeof(Uint8) * byteSize);
	if (pHeader) {
		memcpy(pHeader, encHeaderParam.paraSet, byteSize); 	

		/* ParaBuffer is big endian*/
		pBuf = (Uint32 *) pHeader;
		for (i = 0; i < byteSize/4; i++) {
			dword1 = pBuf[i];
			dword2 = (dword1 >> 24) & 0xFF;
			dword2 |= ((dword1 >> 16) & 0xFF) <<  8;
			dword2 |= ((dword1 >>  8) & 0xFF) << 16;
			dword2 |= ((dword1 >>  0) & 0xFF) << 24;
			pBuf[i] = dword2;
		}
		
		if (encHeaderParam.size > 0) {
			fp = fopen(filename, "wb");
			if (fp) {
				fwrite(pHeader, sizeof(Uint8), encHeaderParam.size, fp);
				fclose(fp);
			}
		}
		
		free(pHeader);
	}	
}

void SaveQpReport(PhysicalAddress qpReportAddr, int picWidth, int picHeight, int frameIdx, char *fileName)
{

	FILE * fp;
	int i, j;
	int MBx, MBy, MBxof4, MBxof1, MBxx;
	Uint32 qp;
	Uint8  lastQp[4];

	if (frameIdx == 0)
		fp = fopen(fileName, "wb");
	else
		fp = fopen(fileName, "a+b");
	if (!fp) {
		printf("Can't open %s in TargetDecodeTest().\n", fileName);
		return;
	}

	MBx = picWidth/16;
	MBxof1 = MBx % 4;
	MBxof4 = MBx - MBxof1;
	MBy = picHeight/16;
	MBxx = (MBx + 3) / 4 * 4;
	for (i = 0; i < MBy; i++) {
		for (j = 0; j < MBxof4; j = j + 4) {
			qp = VpuReadReg(qpReportAddr + j + MBxx*i);
			fprintf(fp, " %4d %4d %3d \n", frameIdx, MBx*i + j + 0, (Uint8)(qp>>24));
			fprintf(fp, " %4d %4d %3d \n", frameIdx, MBx*i + j + 1, (Uint8)(qp>>16)); 
			fprintf(fp, " %4d %4d %3d \n", frameIdx, MBx*i + j + 2, (Uint8)(qp>>8)); 			
			fprintf(fp, " %4d %4d %3d \n", frameIdx, MBx*i + j + 3, (Uint8)qp);	
			 
		}	
		if (MBxof1 > 0) {
			qp = VpuReadReg(qpReportAddr + MBxx*i + MBxof4);
			lastQp[0] = (Uint8)(qp>>24);
			lastQp[1] = (Uint8)(qp>>16);
			lastQp[2] = (Uint8)(qp>>8);
			lastQp[3] = (Uint8)(qp);
		}
		for (j = 0; j < MBxof1; j++) {
			fprintf(fp, " %4d %4d %3d \n", frameIdx, MBx*i + j + MBxof4, lastQp[j]);	
		}			
	}
	
	fclose(fp);

}

int vpu_WaitForInt(int timeout_in_ms)
{
	return IOWaitForInt(timeout_in_ms);
}

/* Fix MPEG4 issue on MX27 TO2 */
int vpu_ESDMISC_LHD(int disable)
{
	return IOLHD(disable);
}
