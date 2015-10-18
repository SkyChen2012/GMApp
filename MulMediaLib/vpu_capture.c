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
 * @file vpu_capture.c
 *
 * @brief This file implements cature function using v4l2 interface for vpu test.
 *
 * @ingroup VPU
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <linux/videodev.h>
#include <string.h>
#include <malloc.h>

#include "vpu_capture.h"

static int g_frame_rate = 0;
static int g_capture_count = 0;
static int g_width = 0;
static int g_height = 0;

int start_capturing(int fd_v4l)
{
	unsigned int i;
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;

	for (i = 0; i < TEST_BUFFER_NUM; i++) {
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if (ioctl(fd_v4l, VIDIOC_QUERYBUF, &buf) < 0) {
			printf("%s: VIDIOC_QUERYBUF error\n", __FUNCTION__);
			return -1;
		}

		cap_buffers[i].length = buf.length;
		cap_buffers[i].offset = (size_t) buf.m.offset;
		cap_buffers[i].start = mmap(NULL, cap_buffers[i].length,
					    PROT_READ | PROT_WRITE, MAP_SHARED,
					    fd_v4l, cap_buffers[i].offset);
	}

	for (i = 0; i < TEST_BUFFER_NUM; i++) {
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if (ioctl(fd_v4l, VIDIOC_QBUF, &buf) < 0) {
			printf("%s: VIDIOC_QBUF error\n", __FUNCTION__);
			return -2;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(fd_v4l, VIDIOC_STREAMON, &type) < 0) {
		printf("%s: VIDIOC_STREAMON error\n", __FUNCTION__);
		return -3;
	}

	return 0;
}

void
ResetCapturing(int fd_v4l)
{
	g_capture_count = 0;
}


void stop_capturing(int fd_v4l)
{
	enum v4l2_buf_type type;
	unsigned int i;
	
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(fd_v4l, VIDIOC_STREAMOFF, &type);

	for (i = 0; i < TEST_BUFFER_NUM; i++) {
		
		munmap(cap_buffers[i].start , cap_buffers[i].length);
	}
	
	close(fd_v4l);
}

int v4l_capture_setup(int width, int height, int frame_rate)
{
	int fd_v4l = 0;
	char v4l_device[100] = "/dev/v4l/video0";
	struct v4l2_format fmt;
	struct v4l2_streamparm parm;
	struct v4l2_requestbuffers req;

	g_width = width;
	g_height = height;
	g_frame_rate = frame_rate;

	if ((fd_v4l = open(v4l_device, O_RDWR, 0)) < 0) {
		printf("%s: Unable to open %s\n", __FUNCTION__, v4l_device);
		return -1;
	}

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.sizeimage = 0;

	if (ioctl(fd_v4l, VIDIOC_S_FMT, &fmt) < 0) {
		printf("%s: VIDIOC_S_FMT failed\n", __FUNCTION__);
		return -2;
	}

	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = frame_rate;

	if (ioctl(fd_v4l, VIDIOC_S_PARM, &parm) < 0) {
		printf("%s: VIDIOC_S_PARM failed\n", __FUNCTION__);
		return -3;
	}
	memset(&req, 0, sizeof(req));
	req.count = TEST_BUFFER_NUM;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd_v4l, VIDIOC_REQBUFS, &req) < 0) {
		printf("%s: VIDIOC_REQBUFS failed\n", __FUNCTION__);
		return -4;
	}
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(fd_v4l, VIDIOC_G_FMT, &fmt) < 0) {
		printf("%s: VIDIOC_G_FMT failed\n", __FUNCTION__);
		return -5;
	} else {
		printf("\t Width = %d", fmt.fmt.pix.width);
		printf("\t Height = %d", fmt.fmt.pix.height);
		printf("\t Image size = %d\n", fmt.fmt.pix.sizeimage);
		printf("\t pixelformat = %d\n", fmt.fmt.pix.pixelformat);
	}

	if (start_capturing(fd_v4l) == 0) {
		printf("%s: Capture start OK.\n", __FUNCTION__);
	} else {
		printf("%s: Capture start ERROR.\n", __FUNCTION__);
		return -6;
	}

	return fd_v4l;
}

int v4l_get_data(int fd, void *input_buf)
{
	struct v4l2_buffer *v4l2_buf;
	v4l2_buf = (struct v4l2_buffer *)input_buf;

	memset(v4l2_buf, 0, sizeof(v4l2_buf));
	v4l2_buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf->memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd, VIDIOC_DQBUF, v4l2_buf) < 0) {
		printf("%s: VIDIOC_DQBUF failed.\n", __FUNCTION__);
		return -1;
	}

	if (ioctl(fd, VIDIOC_QBUF, v4l2_buf) < 0) {
		printf("%s: VIDIOC_QBUF failed\n", __FUNCTION__);
		return -2;
	}

	g_capture_count++;
	return 0;
}

int v4l_get_data1(int fd, void *input_buf)
{
	struct v4l2_buffer *v4l2_buf;
	v4l2_buf = (struct v4l2_buffer *)input_buf;

	memset(v4l2_buf, 0, sizeof(v4l2_buf));
	v4l2_buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf->memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd, VIDIOC_DQBUF, v4l2_buf) < 0) {
		printf("%s: VIDIOC_DQBUF failed.\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

int v4l_get_data2(int fd, void *input_buf)
{
	struct v4l2_buffer *v4l2_buf;
	v4l2_buf = (struct v4l2_buffer *)input_buf;

	if (ioctl(fd, VIDIOC_QBUF, v4l2_buf) < 0) {
		printf("%s: VIDIOC_QBUF failed\n", __FUNCTION__);
		return -2;
	}

	g_capture_count++;
	return 0;
}


