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
 * @file vpu_io.c
 *
 * @brief VPU system ioctrl implementation
 *
 * @ingroup VPU
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <string.h>
#include <strings.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>

#include "vpu_io.h"
#include "vpu_lib.h"

#ifdef DEBUG
#define DPRINTF(fmt, args...) printf("%s: " fmt , __FUNCTION__, ## args)
#else
#define DPRINTF(fmt, args...)
#endif

#define GET_PAGE_ALIGN(x)    ((x)&0xfffff000)

static int vpu_fd = -1;
static unsigned long vpu_reg_base = 0;
#ifdef	INT_CALLBACK
static vpu_callback vpu_io_callback = NULL;
#endif
/* will remove the interrupt handler later */
#undef USERSPACE_HANDLER

static int vpuInit = 0;				

int isVpuInitialized(void)
{
	return vpuInit;
}

/* make consideration for both register and physical mem access */
inline unsigned long *reg_map(unsigned long offset)
{
	return (unsigned long *)(offset + (unsigned long)vpu_reg_base);
}

/* user space interrupt handler */
int vpu_InterruptHandler(void)
{
	int status;
#ifdef	INT_CALLBACK
	if (vpu_io_callback) {
		status = VpuReadReg(BIT_INT_REASON);
		VpuWriteReg(BIT_INT_REASON, 0);
		vpu_io_callback(status);
	}
#else
	status = VpuReadReg(BIT_INT_REASON);
	DPRINTF("VPU Int Status = 0x%08x\n", status);
	VpuWriteReg(BIT_INT_REASON, 0);
#endif

	return 0;
}

/*!
 * @brief IO system initialization.
 *  When user wants to start up the codec system, 
 *  this function call is needed, to open the codec device, 
 *  map the register into user space, 
 *  get the working buffer/code buffer/parameter buffer, 
 *  download the firmware, and then set up the interrupt signal path.
 *
 * @param callback vpu interrupt callback.
 *
 * @return
 * @li  0	          System initialization success.
 * @li -1		System initialization failure.
 */
int IOSystemInit(void *callback)
{
	errno = 0;

	/* check if the device has been opened */
	if (vpu_fd < 0) {
		vpu_fd = open("/dev/mxc_vpu", O_RDWR);
		if (vpu_fd < 0) {
			printf("Can't open mxc codec deivce.\n");
			return -1;
		}
/*
#ifdef USERSPACE_HANDLER
		// interrupt handler in user space
		int o_flags;
		struct sigaction vpu_sig;
		vpu_sig.sa_handler = (__sighandler_t) vpu_InterruptHandler;
		sigemptyset(&vpu_sig.sa_mask);
		vpu_sig.sa_flags = 0;
		sigaction(SIGIO, &vpu_sig, NULL);

		fcntl(vpu_fd, F_SETOWN, getpid());
		o_flags = fcntl(vpu_fd, F_GETFL);
		fcntl(vpu_fd, F_SETFL, FASYNC | o_flags);
#endif
*/
		printf("***************** Vpu_io.c : mmap begin!\n");

		vpu_reg_base = (unsigned long)mmap(NULL, BIT_REG_MARGIN,
						   PROT_READ | PROT_WRITE,
						   MAP_SHARED, vpu_fd, 0);
		printf("vpu user reg base %d, errno: -%d\n", (int)vpu_reg_base, errno);
		if (vpu_reg_base == 0) {
			printf("Can't map register\n");
			return -1;
		}
#ifdef	INT_CALLBACK
		if (vpu_io_callback == NULL && callback != NULL)
			vpu_io_callback = (vpu_callback) callback;
#endif
		if (vpuInit == 0) {
			bit_work_addr.size = WORK_BUF_SIZE + PARA_BUF_SIZE + 
				CODE_BUF_SIZE + PARA_BUF2_SIZE;
			IOGetPhyMem(&bit_work_addr);
			IOGetVirtMem(&bit_work_addr);
			vpu_Init(bit_work_addr.phy_addr);
			vpuInit = 1;
		}
	}

	return 0;
}

/*!
 * @brief IO system shut down.
 *
 * When user wants to stop the codec system, this 
 * function call is needed, to release the interrupt 
 * signal, free the working buffer/code buffer/parameter 
 * buffer, unmap the register into user space, and 
 * close the codec device.
 *
 * @param none
 *
 * @return
 * @li   0	System shutting down success.
 * @li   -1		System shutting down failure.
 */
int IOSystemShutdown(void)
{
	/* check if the device has been opened */
	DPRINTF("mxc_vpu close begin\n");
#ifdef INT_CALLBACK
	if (vpu_io_callback) {
		vpu_io_callback = NULL;
	}
#endif
	IOFreeVirtMem(&bit_work_addr);
	IOFreePhyMem(&bit_work_addr);

	VpuWriteReg(BIT_INT_ENABLE, 0);	/* PIC_RUN irq disable */

	munmap(vpu_reg_base, BIT_REG_MARGIN);

	if (vpu_fd >= 0) {
		close(vpu_fd);
		vpu_fd = -1;
	}

	vpuInit = 0;

	DPRINTF("mxc_vpu close end\n");

	return 0;

}

unsigned long VpuWriteReg(unsigned long addr, unsigned int data)
{
	unsigned long *reg_addr = reg_map(addr);
	*(volatile unsigned long *)reg_addr = data;

	return 0;
}

unsigned long VpuReadReg(unsigned long addr)
{
	unsigned long *reg_addr = reg_map(addr);
	return *(volatile unsigned long *)reg_addr;
}

/*!
 * @brief
 * When the system starts up, resetting is needed in advance.
 */
void ResetVpu(void)
{
	unsigned long *reg_addr = reg_map(BIT_CODE_RST);
	(*(volatile unsigned long *)reg_addr) |= 0x1;
	usleep(10);
	(*(volatile unsigned long *)reg_addr) &= ~0x1;

	return;
}

/*!
 * @brief Allocated buffer of requested size
 * When user wants to get massive memory 
 * for the system, they needs to fill the required 
 * size in buff structure, and if this function 
 * succeeds in allocating memory and returns 0, 
 * the returned physical memory is filled in 
 * phy_addr of buff structure. If the function fails 
 * and return -1,  the phy_addr remains the same as before.
 * memory size is in byte.
 *
 * @param buff	the structure contains the memory information to be got;
 *
 * @return
 * @li 0	          Allocation memory success.
 * @li -1		Allocation memory failure.
 */
int IOGetPhyMem(vpu_mem_desc * buff)
{
	if (ioctl(vpu_fd, VPU_IOC_PHYMEM_ALLOC, buff) < 0) {
		printf("mem allocation failed!\n");
		return -1;
	}
	DPRINTF("allocated at 0x%x\n", (unsigned int)buff->phy_addr);

	return 0;
}

/*!
 * @brief Free specified memory
 * When user wants to free massive memory for the system, 
 * they needs to fill the physical address and size to be freed 
 * in buff structure.
 *
 * @param buff	the structure containing memory information to be freed;
 *
 * @return
 * @li 0            Freeing memory success.
 * @li -1		Freeing memory failure.
 */
int IOFreePhyMem(vpu_mem_desc * buff)
{
	if (buff->phy_addr != 0)
		ioctl(vpu_fd, VPU_IOC_PHYMEM_FREE, buff);
	DPRINTF("freed %x\n", (unsigned int)buff->phy_addr);
	buff->phy_addr = 0;
	buff->cpu_addr = 0;

	return 0;
}

/*!
 * @brief Map physical memory to user space.
 *
 * @param	buff	the structure containing memory information to be mapped.
 *
 * @return	user space address.
 */
 
int IOGetVirtMem(vpu_mem_desc * buff)
{
	if (buff->virt_uaddr == 0)
		buff->virt_uaddr =
		    (int)mmap(NULL, buff->size, PROT_READ | PROT_WRITE,
			      MAP_SHARED, vpu_fd,
			      GET_PAGE_ALIGN(buff->phy_addr));
		  
	return buff->virt_uaddr;
}

/*!
 * @brief Unmap  physical memory to user space.
 *
 * @param	buff	the structure containing memory information to be unmapped;
 *
 * @return	
 * @li 0        Success
 * @li Others 	Failure
 */
int IOFreeVirtMem(vpu_mem_desc * buff)
{
	if (buff->virt_uaddr != 0)
		munmap((void *)buff->virt_uaddr, buff->size);

	buff->virt_uaddr = 0;

	return 0;
}

int IOWaitForInt(int timeout_in_ms)
{
	int ret = 0;
	if (timeout_in_ms < 0) {
		printf("the paramater timeout is not valid.\n");
	}

	if (vpu_fd < 0) {
		printf("File Descriptor is not valid.\n");
	}

	ret= ioctl(vpu_fd, VPU_IOC_WAIT4INT, timeout_in_ms);

	return ret;
}

/*!
 * @brief Latency Hiding Disable/Enable, to fix MPEG4 issue on MX27 TO2
 *
 * @param	disable	enable/disable the LHD bit in the ESDMISC register.
 *
 * @return	
 * @li 0        Success
 * @li Others 	Failure
 */
int IOLHD(int disable)
{
        int ret = 0;
        if (disable < 0) {
                printf("the paramater disable is not valid.\n");
        }

        if (vpu_fd < 0) {
                printf("File Descriptor is not valid.\n");
        }

        ret= ioctl(vpu_fd, VPU_IOC_LHD, disable);

        return ret;
}

/*!
 * @brief Get the chip info from /proc/cpuinfo
 */
int getChipVersion(void)
{
	char line[80];
	FILE *fp;
	char cpu[32], rev[8];
	char *str;
	static int nChipVersion=0;
	if(nChipVersion)
	{
		return nChipVersion;
	}
	
	if ((fp = fopen("/proc/cpuinfo", "r")) == NULL) {
		printf("can't access /proc/cpuinfo\n");
		return -1;
	}
	
	while (fgets(line, 80, fp) != NULL) {
		if (strncmp(line, "Hardware", 8) == 0) {
			str = index(line, ':');	
			strcpy(cpu, str + 2);
		}
		if (strncmp(line, "Revision", 8) == 0) {
			str = index(line, ':');	
			strcpy(rev, str + 2);
		}
	}
	fclose (fp);
	if (strlen(cpu) == 0) {
		printf("can't get the chip info.\n");
		return -1;
	} else if (strstr(cpu, "MX27ADS") == NULL) {
		printf("VPU does not support %s\n", cpu);
		return -1;
	}

	if (strncmp(rev, "0020", 4) == 0) {
		DPRINTF("MX27 TO2\n");
		nChipVersion= MX27_REV2;
	} else {
		DPRINTF("MX27 TO1\n");
		nChipVersion= MX27_REV1;
	}
	return nChipVersion;
}
