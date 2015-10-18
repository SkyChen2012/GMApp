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
 * @file vpu_display.c
 *
 * @brief This file implements display and pp function using v4l2 interface for vpu test.
 *
 * @ingroup VPU
 */

/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* Verification Test Environment Include Files */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>
#include <linux/videodev.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include <malloc.h>

#include "vpu_display.h"

static int g_frame_size;
static int g_num_buffers;

static int g_output;
static int g_in_width;
static int g_in_height;
static int g_display_width;
static int g_display_height;
static int g_display_top;
static int g_display_left;
static int g_rotate;

static u_int32_t g_in_fmt;
static int g_frame_count = 0;
//static int bAgain = 0;
struct timeval tv_start;
#define FRAMERATE	100

int mxc_v4l_output_setup(int fd_v4l, struct v4l2_format *fmt, int frame_num)
{
	struct v4l2_requestbuffers buf_req;

	if (ioctl(fd_v4l, VIDIOC_S_FMT, fmt) < 0) {
		printf("%s: VIDIOC_S_FMT failed\n", __FUNCTION__);
		return TFAIL;
	}

	if (ioctl(fd_v4l, VIDIOC_G_FMT, fmt) < 0) {
		printf("%s: VIDIOC_G_FMT failed\n", __FUNCTION__);
		return TFAIL;
	}

	memset(&buf_req, 0, sizeof(buf_req));
	buf_req.count = frame_num;
	buf_req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	buf_req.memory = V4L2_MEMORY_MMAP;
	if (ioctl(fd_v4l, VIDIOC_REQBUFS, &buf_req) < 0) {
		printf("%s: VIDIOC_REQBUFS failed\n", __FUNCTION__);
		return TFAIL;
	}
	g_num_buffers = buf_req.count;
	printf("%s: Allocated %d buffers\n", __FUNCTION__, buf_req.count);

	return TPASS;
}

int v4l_display_setup(int width, int height, int out_width, int out_height, int frame_num)
{
	int i;
	int fd_v4l;
	struct v4l2_buffer buf;
	int y_size;

	int retval = TPASS;
	char v4l_device[100] = "/dev/v4l/video16";

	struct v4l2_control ctrl;
	struct v4l2_format fmt;
	struct v4l2_framebuffer fb;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;

//	g_frame_count = 0;

	g_in_width = width;
	g_in_height = height;
	g_display_width = out_width;
	g_display_height = out_height;
	g_display_top = 0;
	g_display_left = 0;
	g_output = 0;
	g_rotate = 0;
	g_in_fmt = V4L2_PIX_FMT_YUV420;

	printf("%s: width=%d, height=%d,out_width=%d, out_height=%d, frame_num=%d.\n",
		__FUNCTION__, width, height, out_width, out_height, frame_num);

	if ((fd_v4l = open(v4l_device, O_RDWR, 0)) < 0) {
		printf("%s: Unable to open %s\n", __FUNCTION__, v4l_device);
		retval = TFAIL;
		return -1;

	}

	if (ioctl(fd_v4l, VIDIOC_S_OUTPUT, &g_output) < 0) {
		printf("%s: VIDIOC_S_OUTPUT error\n", __FUNCTION__);
		return -2;
	}

	memset(&cropcap, 0, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if (ioctl(fd_v4l, VIDIOC_CROPCAP, &cropcap) < 0) {
		printf("%s: VIDIOC_CROPCAP failed\n", __FUNCTION__);
		retval = TFAIL;
		return -3;
	}
	printf("cropcap.bounds.width = %d\ncropcap.bound.height = %d\n"
	       "cropcap.defrect.width = %d\ncropcap.defrect.height = %d\n",
	       cropcap.bounds.width, cropcap.bounds.height,
	       cropcap.defrect.width, cropcap.defrect.height);

	crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	crop.c.top = g_display_top;
	crop.c.left = g_display_left;
	crop.c.width = g_display_width;
	crop.c.height = g_display_height;
	if (ioctl(fd_v4l, VIDIOC_S_CROP, &crop) < 0) {
		printf("%s: VIDIOC_S_CROP failed\n", __FUNCTION__);
		retval = TFAIL;
		return -4;
	}
	/* Set rotation */
	ctrl.id = V4L2_CID_PRIVATE_BASE;
	ctrl.value = g_rotate;
	if (ioctl(fd_v4l, VIDIOC_S_CTRL, &ctrl) < 0) {
		printf("%s: VIDIOC_S_CTRL failed\n", __FUNCTION__);
		retval = TFAIL;
		return -1;
	}

	fb.capability = V4L2_FBUF_CAP_EXTERNOVERLAY;
//	fb.flags = V4L2_FBUF_FLAG_OVERLAY;
	fb.flags = V4L2_FBUF_FLAG_PRIMARY;
	ioctl(fd_v4l, VIDIOC_S_FBUF, &fb);

	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	fmt.fmt.pix.width = g_in_width;
	fmt.fmt.pix.height = g_in_height;
	fmt.fmt.pix.pixelformat = g_in_fmt;

	mxc_v4l_output_setup(fd_v4l, &fmt, frame_num);
	g_frame_size = fmt.fmt.pix.sizeimage;

	y_size = (g_frame_size * 2) / 3;
	memset(&buf, 0, sizeof(buf));

	for (i = 0; i < g_num_buffers; i++) {
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if (ioctl(fd_v4l, VIDIOC_QUERYBUF, &buf) < 0) {
			printf("%s: VIDIOC_QUERYBUF error\n", __FUNCTION__);
			return -1;
		}

		buffers[i].length = buf.length;
		buffers[i].offset = (size_t) buf.m.offset;
		printf("VIDIOC_QUERYBUF: length = %d, offset = %#x\n",
		       buffers[i].length, (buffers[i].offset));
		buffers[i].start =
		    mmap(NULL, buffers[i].length,
			 PROT_READ | PROT_WRITE, MAP_SHARED, fd_v4l,
			 buffers[i].offset);
		if (buffers[i].start == NULL) {
			printf("v4l2_out test: mmap failed\n");
		}
	}
//g_frame_count = 0;
	return fd_v4l;
}


//当第一次进入此函数的时候，先填充满3个buffer后，然后开始显示，
//结束后再次显示视频进入此函数后，同样也是先填充满3个buffer后，再显示，不过这次起点是
//g_frame_count = 3后的3帧，也就是4，5，6帧。
int v4l_put_data(int fd_v4l, void *input_buf)
{
	int type;
	struct v4l2_buffer *buf;
	static time_t sec;
	static suseconds_t usec;
	

	buf = (struct v4l2_buffer *)input_buf;

	if (g_frame_count == 0) {
/*		gettimeofday(&tv_start, 0);
		buf->timestamp.tv_sec = tv_start.tv_sec;
		buf->timestamp.tv_usec = tv_start.tv_usec;
		sec = tv_start.tv_sec;
		usec = tv_start.tv_usec;
*/
	}

	if (g_frame_count >= 1) {
	/*	usec = usec + (1000000 / FRAMERATE);
		while (usec >= 1000000) {
			sec += 1;
			usec -= 1000000;
		}
		buf->timestamp.tv_sec = sec;
		buf->timestamp.tv_usec = usec;
*/
		if (ioctl(fd_v4l, VIDIOC_QBUF, buf) < 0) {
			printf("%s: VIDIOC_QBUF failed.\n", __FUNCTION__);
			return -1;
		}
		if (g_frame_count == 1) {	/* Start playback after buffers queued */
			type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
			if (ioctl(fd_v4l, VIDIOC_STREAMON, &type) < 0) {
				printf("%s: Could not start stream.\n", __FUNCTION__);
				return -1;
			}
		}
	}
	buf->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	buf->memory = V4L2_MEMORY_MMAP;
	if (g_frame_count < g_num_buffers) {
/*		buf->index = g_frame_count;
		if (ioctl(fd_v4l, VIDIOC_QUERYBUF, buf) < 0) {
			printf("%s: VIDIOC_QUERYBUF failed.\n", __FUNCTION__);
			return -1;
		}
*/
	} else //if (!bAgain) 
	{
		buf->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf->memory = V4L2_MEMORY_MMAP;
		if (ioctl(fd_v4l, VIDIOC_DQBUF, buf) < 0) {
			printf("%s: VIDIOC_DQBUF failed.\n", __FUNCTION__);
			return -1; 
		}

	}

	g_frame_count++;
	//
//	if(g_frame_count == g_num_buffers * 2)
//	{
//		bAgain = 0;
//	}

	return 0;
}
void stop_display(int fd_v4l)
{
	enum v4l2_buf_type type;
	
	type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ioctl(fd_v4l, VIDIOC_STREAMOFF, &type);
	
	close(fd_v4l);
}


void
Display_reset(int fd_v4l)
{
	static int bFirst = 0;
	int i = 0;
	struct v4l2_buffer *buf;

//	if (!bFirst)
//	{
//		bFirst = 1;
//		return;
//	}

	printf("start to reset display....g_num_buffer=%d..\n", g_num_buffers);

//	g_frame_count = g_num_buffers;
//	bAgain = 1;

}




