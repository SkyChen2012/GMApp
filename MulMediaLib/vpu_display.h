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
 * @file vpu_display.h
 *
 * @brief This file defines display and pp function header.
 *
 * @ingroup VPU
 */

#ifndef VPU_DISPLAY_H
#define VPU_DISPLAY_H

#define MAX_BUFFER_NUM 5

struct testbuffer {
	unsigned char *start;
	size_t offset;
	unsigned int length;
};

struct EncodeVideoParam{
	int encpicWidth ;
	int encpicHeight ;
	int encbitRate ;
}ENCODEVP;



struct testbuffer buffers[5];
#define TFAIL -1
#define TPASS 0

//#define MXCFB_WAIT_FOR_VSYNC    _IO('M', 2)
#define MXCFB_SET_BRIGHTNESS    _IOW('M', 3, __u8)

extern int fb_setup(void);
extern int mxc_v4l_output_setup(int fd_v4l, struct v4l2_format *fmt, int frame_num);

extern int v4l_display_setup(int width, int height, int out_width, int out_height, int frame_num);
extern int v4l_put_data(int fd_v4l, void *input_buf);
extern void stop_display(int fd_v4l);
extern void	Display_reset(int fd_v4l);

#endif				/* VPU_DISPLAY_H */
