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
 * @file vpu_io.h
 *
 * @brief VPU system ioctrl definition
 *
 * @ingroup VPU
 */

#ifndef __VPU__IO__H
#define __VPU__IO__H

/*********  VRAM base definition ************/
#define	VRAM_BASE		0xFFFF4C00
#define	BIT_MEM_PHY_START	0xA0000000

/*!
 * @brief  vpu memory description structure
 */
typedef struct vpu_mem_desc {
	int size;			/*!requested memory size */
	unsigned long phy_addr;		/*!physical memory address allocated */
	unsigned long cpu_addr;		/*!cpu addr for system free usage */
	unsigned long virt_uaddr;	/*!virtual user space address */
} vpu_mem_desc;

#define	VPU_IOC_MAGIC		'V'

#define	VPU_IOC_PHYMEM_ALLOC	_IO(VPU_IOC_MAGIC, 0)
#define	VPU_IOC_PHYMEM_FREE	_IO(VPU_IOC_MAGIC, 1)
#define VPU_IOC_WAIT4INT	_IO(VPU_IOC_MAGIC, 2)
#define	VPU_IOC_PHYMEM_DUMP	_IO(VPU_IOC_MAGIC, 3)
#define	VPU_IOC_REG_DUMP	_IO(VPU_IOC_MAGIC, 4)
#define VPU_IOC_LHD		_IO(VPU_IOC_MAGIC, 5)

#define MX27_REV1		0x0882101D
#define MX27_REV2		0x1882101D

typedef void (*vpu_callback) (int status);

vpu_mem_desc bit_work_addr;

extern int IOSystemInit(void *callback);
extern int IOSystemShutdown(void);
extern int IOGetPhyMem(vpu_mem_desc * buff);
extern int IOFreePhyMem(vpu_mem_desc * buff);
extern int IOGetVirtMem(vpu_mem_desc * buff);
extern int IOFreeVirtMem(vpu_mem_desc * buff);
extern int IOWaitForInt(int timeout_in_ms);
extern int IOLHD(int disable);

extern unsigned long VpuWriteReg(unsigned long addr, unsigned int data);
extern unsigned long VpuReadReg(unsigned long addr);
#define	VpuWriteMem(addr, data)	VpuWriteReg(addr, data)
#define	VpuReadMem(addr)	VpuReadReg(addr)
extern void ResetVpu(void);
extern int isVpuInitialized(void);
extern int getChipVersion(void);

#endif
