/*
 * Copyright 2005-2006 Freescale Semiconductor, Inc. All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef __TESTMOD_SSI_H__
#define __TESTMOD_SSI_H__

#define SSI_IOCTL             				0x54

#define IOCTL_SSI_GET_PLAYBACK_CALLBACKS				0x0
#define IOCTL_SSI_GET_STATUS			0x1
#define IOCTL_SSI_CONFIGURE_AUDMUX			0x2
#define IOCTL_SSI_CONFIGURE_MC13783			0x3
#define IOCTL_SSI_CONFIGURE_MC13783_MIX			0x4
#define IOCTL_SSI_DMA_TRIGGER			0x5
#define IOCTL_SSI_DMA_STOP 0x6
#define IOCTL_SSI_DMA_PLAYBACK_TRIGGER			0x7
#define IOCTL_SSI_DMA_CAPTURE_TRIGGER			0x8
#define IOCTL_SSI_DMA_PLAYBACK_STOP 0x9
#define IOCTL_SSI_DMA_CAPTURE_STOP 0xa
#define IOCTL_SSI_GET_CAPTURE_CALLBACKS				0xb
#define IOCTL_SSI_RESET				0xc
#define IOCTL_SSI_CX20707                  0xd

#define STEREO_DAC					0x0
#define CODEC						0x1

#define MC13783_MASTER					0x1
#define MC13783_SLAVE					0x2

/**/

#ifdef CONFIG_ARCH_MX3
#define CCM_COSR_OFFSET					(MXC_CCM_BASE + 0x1c)

#define INIT_CKO_CLOCK()				\
do{							\
volatile unsigned long reg;				\
							\
unsigned int mask = 0x00000fff;				\
unsigned int data = 0x00000208;				\
							\
mxc_ccm_modify_reg(CCM_COSR_OFFSET, mask, data);	\
reg = mxc_ccm_get_reg(CCM_COSR_OFFSET);			\
gpio_audio_port_active(4);				\
gpio_audio_port_active(5);				\
}while(0)

/*!
 * This macro is used to normalize master clock (ccm/crm) 
 * divider value, as USBPLL runs at different frequency 
 * on each platform. On MX31 case, there is no need to 
 * normalize it as MX31 dividers are the base for the 
 * other boards.
 */
#define NORMALIZE_CCM_DIVIDER(v)			(v)

#endif /* CONFIG_ARCH_MX3 */


#ifdef CONFIG_ARCH_MX27
#define NORMALIZE_CCM_DIVIDER(v)			( ((v) * 2) / 5 )
/*#define INIT_CKO_CLOCK()  \
do{ \
  mxc_set_clock_output(CKO, CKIH_CLK, 1);\
  mxc_clks_enable(SSI1_BAUD);\
  mxc_clks_enable(SSI2_BAUD);\
  gpio_ssi_active(0);\
  gpio_ssi_active(1);\
  mc13783_audio_output_bias_conf(1, 1);\
}while(0)*/
#endif /* CONFIG_ARCH_MX27 */

#define DEVICE_NAME_SSI1				"/dev/mxc_ssi1"
#define DEVICE_NAME_SSI2				"/dev/mxc_ssi2"



#endif /* __TESTMOD_SSI_H__ */
