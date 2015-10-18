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
 * @file vpu_voip_app.c
 *
 * @brief This file implements codec tests from different input: file/EMMA/RTP.
 *
 * @ingroup VPU
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev.h>

#include "rtp.h"
#include "vpu_display.h"
#include "vpu_capture.h"
#include "vpu_voip_app.h"
#include "vpu_lib.h"

struct codec_file multi_bitstream[MAX_NUM_INSTANCE] = {
	{"/test/1.264", NULL},
	{"/test/2.264", NULL},
	{"/tmp/3.264", NULL},
	{"/tmp/4.264", NULL}
};

struct codec_file multi_yuv[MAX_NUM_INSTANCE] = {
	{"/test/1.yuv", NULL},
	{"/test/2.yuv", NULL},
	{"/test/3.yuv", NULL},
	{"/test/4.yuv", NULL}
};

pthread_t sig_thread_id;
sigset_t mask;

int		FlagEnCapYuv = 0;

#define RTP_PORT    6666

rtp_t *rtp_session_open(char *ip_addr, int read_flag)
{
	int rt;
	rtp_t *rtp;
	rtp = (rtp_t *) malloc(sizeof(rtp_t));
	int fh;
	fh = -1;
	memset(rtp, 0, sizeof(rtp_t));
	if (read_flag == CODEC_READING) {
		DPRINTF("rtp play\n");
		rtp->name = "play";
		rtp->ip = 0;
		rtp->port = RTP_PORT;
	} else {
		DPRINTF("rtp rec\n");
		rtp->name = "rec";
		rtp->ip = ip_addr;
		rtp->port = RTP_PORT;
	}
	rtp->type = RTP_TYPE_DYNAMIC;
	rtp->size = 512;
	rtp->clock = 1000000;
	rt = rtp_init(rtp, 0);
	if (rt < 0) {
		printf("%s: rtp_init err %d\n", rtp->name, rt);
		return NULL;
	}

	return rtp;
}

int emma_dev_open(int width, int height, int read_flag, int output_ratio, int frame_num)
{
	int fd_v4l;
	if (read_flag == CODEC_READING) {
		fd_v4l = v4l_capture_setup(width, height, 25);
	} else {
		fd_v4l = v4l_display_setup(width, height,
					   width / output_ratio,
					   height / output_ratio, frame_num);
	}
	return fd_v4l;

}

int FillBsBufMulti(int src, char *src_name, int targetAddr,
		   int bsBufStartAddr, int bsBufEndAddr, int size,
		   int index, int checkeof, int *streameof, int read_flag)
{
	size_t byte_rw = 0;
	rtp_t *rtp = NULL;
	switch (src) {
	case PATH_FILE:
		{
			int nreaded = 0;

			if (!multi_bitstream[index].fd) {
				if (read_flag == CODEC_READING)
					multi_bitstream[index].fd =
					    fopen(src_name, "ro");
				else
					multi_bitstream[index].fd =
					    fopen(src_name, "wb");
				if (!multi_bitstream[index].fd) {
					printf("bs can not open file %s.\n",
					       src_name);
					return -1;
				}
			}
			if (read_flag == CODEC_READING) {
				/* Looks ugly. but to void to use malloc/mmap/memcpy */
				if ((targetAddr + size) > bsBufEndAddr) {
					int room = bsBufEndAddr - targetAddr;
					nreaded =
					    fread((unsigned char *)targetAddr,
						  1, room,
						  multi_bitstream[index].fd);
					if (nreaded != room) {
						if (checkeof) {
							if (nreaded == 0) {
								*streameof = 1;
								return -2;
							}
						} else {
							fseek(multi_bitstream
							      [index].fd, 0,
							      SEEK_SET);
							if (!fread
							    ((unsigned char *)
							     targetAddr +
							     nreaded, 1,
							     room - nreaded,
							     multi_bitstream
							     [index].fd))
								return -3;
						}
					}

					nreaded = fread((unsigned char *)
							bsBufStartAddr, 1,
							size - room,
							multi_bitstream[index].
							fd);
					if (nreaded != (size - room)) {
						if (checkeof) {
							if (nreaded == 0) {
								*streameof = 1;
								return -4;
							}
						} else {
							fseek(multi_bitstream
							      [index].fd, 0,
							      SEEK_SET);
							if (!fread
							    ((unsigned char *)
							     bsBufStartAddr +
							     nreaded, 1,
							     (size - room) -
							     nreaded,
							     multi_bitstream
							     [index].fd))
								return -5;
						}
					}
					return 0;

				} else {
					nreaded =
					    fread((unsigned char *)targetAddr,
						  1, size,
						  multi_bitstream[index].fd);
					if (nreaded != size) {
						if (checkeof) {
							if (nreaded == 0) {
								*streameof = 1;
								return -6;
							}
						} else {
							fseek(multi_bitstream
							      [index].fd, 0,
							      SEEK_SET);
							if (!fread
							    ((unsigned char *)
							     targetAddr +
							     nreaded, 1,
							     size - nreaded,
							     multi_bitstream
							     [index].fd))
								return -7;
						}
					}

					return 0;
				}

			} else {
#if STREAM_ENC_PIC_RESET == 1
				byte_rw =
				    fwrite((unsigned char *)targetAddr,
					   1, size, multi_bitstream[index].fd);
#else
				int room, sizeby4, roomby4;
				
				if ((targetAddr + size) > bsBufEndAddr) {
					room = bsBufEndAddr - targetAddr;
					roomby4 = ( ( room + 3 ) / 4 ) * 4;
					
					byte_rw = fwrite((unsigned char *)targetAddr,
							1, roomby4, multi_bitstream[index].fd);


					sizeby4 = ( ( ( size-room ) + 3) / 4 ) * 4;

					byte_rw += fwrite((unsigned char *)bsBufStartAddr,
								1, sizeby4, multi_bitstream[index].fd);

				} else {
					sizeby4 = ( ( (size + 3) / 4 ) * 4 );

					byte_rw = fwrite((unsigned char *)targetAddr,
						1, sizeby4, multi_bitstream[index].fd);
				}
#endif
			}

			break;
		}
	case PATH_NET:
		{
			if (!multi_bitstream[index].fd) {
				rtp = rtp_session_open(src_name, read_flag);
				if (rtp == NULL) {
					printf
					    ("bs can not open file %s.\n",
					     src_name);
					return -1;
				}
				multi_bitstream[index].fd = (FILE *) rtp;	/*1: rtp open success */
				
				if (read_flag == CODEC_READING) {
					do {
					byte_rw += rtp_rx((rtp_t*) (multi_bitstream[index].fd),
							(char *)targetAddr + byte_rw, size);
					} while (byte_rw < size);
				} else {
					byte_rw = rtp_tx((rtp_t*) (multi_bitstream[index].fd), 
							(char *)targetAddr, size);
				}
			}
			else
			{
				if (read_flag == CODEC_READING) {
					byte_rw = rtp_rx((rtp_t*) (multi_bitstream[index].fd),
							(char *)targetAddr + byte_rw, size);
				} else {
					byte_rw = rtp_tx((rtp_t*) (multi_bitstream[index].fd), 
							(char *)targetAddr, size);
				}
			}
			break;
		}
	default:
		printf("fill bs error!\n");
		break;
	}

	/* DPRINTF("targetAddr %x, rw %x, index %d\n", targetAddr, byte_rw, index); */
	return byte_rw;
}

/* Read/Write one frame raw data each time, for Enc/Dec respectively */
int FillYuvImageMulti(int src, char *src_name, int buf, void *emma_buf,
		      int inwidth, int inheight, int index,
		      int read_flag, int rot_en, int output_ratio, int frame_num)
{
	int ret = -1;
	rtp_t *rtp = NULL;
	int width;
	int height;
	if (rot_en) {
		width = inheight;
		height = inwidth;
	} else {
		width = inwidth;
		height = inheight;
	}
	size_t byte_rw = 0;
	int image_size = width * height * 3 / 2;

	switch (src) {
	case PATH_FILE:
		{
			if (!multi_yuv[index].fd) {
				if (read_flag == CODEC_READING)
					multi_yuv[index].fd =
					    fopen(src_name, "ro");
				else
					multi_yuv[index].fd =
					    fopen(src_name, "wb");
				if (!multi_yuv[index].fd) {
					printf
					    ("yuv can not open yuv file %s.\n",
					     src_name);
					return -1;
				}
			}
			if (!feof(multi_yuv[index].fd)) {
				if (read_flag == CODEC_READING)
					/*for encoder case, stride = width, but for decoder it's not */
					byte_rw =
					    fread((unsigned char *)buf,
						  1, image_size,
						  multi_yuv[index].fd);
				else
					byte_rw =
					    fwrite((unsigned char *)buf,
						   1, image_size,
						   multi_yuv[index].fd);
			}
			if (byte_rw == 0)
				return -1;
			break;
		}
	case PATH_NET:
		{
			if (!multi_yuv[index].fd) {
				rtp = rtp_session_open(src_name, read_flag);
				if (rtp == NULL) {
					printf
					    ("yuv can not open file %s.\n",
					     src_name);
					return -1;
				}
				multi_yuv[index].fd = (FILE *) rtp;	/* 1: rtp open success */
			}

			if (read_flag == CODEC_READING)
				byte_rw =
				    rtp_rx((rtp_t *) (multi_yuv[index].
						      fd), (char *)buf,
					   image_size);
			else
				byte_rw =
				    rtp_tx((rtp_t *) (multi_yuv[index].
						      fd), (char *)buf,
					   image_size);
			break;
		}

	case PATH_EMMA:
		{
			if (!multi_yuv[index].fd) {
				if ((read_flag == CODEC_READING)||(read_flag == CODEC_READING1)||(read_flag == CODEC_READING2))
					ret =  emma_dev_open(width, height,
						  CODEC_READING, output_ratio, frame_num);
				else
					ret =  emma_dev_open(width, height,
						  CODEC_WRITING, output_ratio, frame_num);

			//	usleep(1000);				
				printf("==========>>>>emma_dev_open index = %d.\n", index);			
				if (ret < 0) {
					printf("yuv can not open device %s.\n", src_name);
					return -1;
				}
				multi_yuv[index].fd = (FILE *) ret;
			}
			
			if (read_flag == CODEC_READING1)
				ret =
				    v4l_get_data1((int)multi_yuv[index].
						 fd, emma_buf);
			else if(read_flag == CODEC_READING2)
				ret =
				    v4l_get_data2((int)multi_yuv[index].
						 fd, emma_buf);
			else if(read_flag == CODEC_READING)
				ret =
				    v4l_get_data((int)multi_yuv[index].
						 fd, emma_buf);	
			else
				ret =
				    v4l_put_data((int)multi_yuv[index].
						 fd, emma_buf);
			return ret;
		}
	default:
		printf("fill bs error!\n");
		return -1;
	}
	/* DPRINTF("buf %x, rw %x, index %d\n", buf, byte_rw, index); */
	return 0;
}

/* read enc/dec config file */
static int vpu_getcfg(int *codec_num,
		      struct codec_config *usr_config, FILE * cfg_file)
{
	char buf[120];
	int i, j;
	int tmp_width, tmp_height;
	printf("Number of Codec: ");
	if (!fgets(buf, sizeof(buf), cfg_file)) {
		return -1;
	}
	*codec_num = strtoul(buf, NULL, 0);
	printf("%d\n ", *codec_num);

	if (*codec_num > MAX_NUM_INSTANCE) {
		printf("Instance can't exceed %d\n", MAX_NUM_INSTANCE);
		return -1;
	}

	for (i = 0; i < *codec_num; i++) {
		printf("NO.: ");
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].index = strtoul(buf, NULL, 0);
		printf("%d\nSrc: ", (int)usr_config[i].index);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].src = strtoul(buf, NULL, 0);
		printf("%d\nsrc name: ", (int)usr_config[i].src);

		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		for (j = 0; j < sizeof(buf); j++) {
			if (buf[j] == ' ') {
				usr_config[i].src_name[j] = '\0';
				break;
			}
			usr_config[i].src_name[j] = buf[j];
		}
		printf("%s\ndst: ", usr_config[i].src_name);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].dst = strtoul(buf, NULL, 0);
		printf("%d\ndst name: ", (int)usr_config[i].dst);

		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		for (j = 0; j < sizeof(buf); j++) {
			if (buf[j] == ' ') {
				usr_config[i].dst_name[j] = '\0';
				break;
			}
			usr_config[i].dst_name[j] = buf[j];
		}
		printf("%s\nEnc or Dec: ", usr_config[i].dst_name);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].enc_flag = strtoul(buf, NULL, 0);
		printf("%d\nframe rate : ", (int)usr_config[i].enc_flag);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].fps = strtoul(buf, NULL, 0);
		printf("%d\nbit rate: ", (int)usr_config[i].fps);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].bps = strtoul(buf, NULL, 0);
		printf("%d\nmode: ", (int)usr_config[i].bps);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].mode = strtoul(buf, NULL, 0);
		printf("%d\nwidth: ", (int)usr_config[i].mode);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].width = strtoul(buf, NULL, 0);
		printf("%d\nheight: ", (int)usr_config[i].width);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].height = strtoul(buf, NULL, 0);
		printf("%d\ngop: ", (int)usr_config[i].height);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].gop = strtoul(buf, NULL, 0);
		printf("%d\nframe count ", (int)usr_config[i].gop);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].frame_count = strtoul(buf, NULL, 0);
		printf("%d\nrot_angle ", (int)usr_config[i].frame_count);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].rot_angle = strtoul(buf, NULL, 0);
		printf("%d\nout_ratio ", (int)usr_config[i].rot_angle);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].out_ratio = strtoul(buf, NULL, 0);
		printf("%d\nmirror_angle ", (int)usr_config[i].out_ratio);
		if (!fgets(buf, sizeof(buf), cfg_file)) {
			return -1;
		}
		usr_config[i].mirror_angle = strtoul(buf, NULL, 0);
		printf("%d\n", (int)usr_config[i].mirror_angle);
		if (usr_config[i].enc_flag) {
			if ((usr_config[i].width > MAX_WIDTH) ||
			    (usr_config[i].height > MAX_HEIGHT)) {
				printf
				    ("Codec Image can't be larger than %d*%d\n",
				     MAX_WIDTH, MAX_HEIGHT);
				return -1;
			}
		} else {
			if (usr_config[i].rot_angle == 90 ||
			    usr_config[i].rot_angle == 270) {
				tmp_width = usr_config[i].height;
				tmp_height = usr_config[i].width;
			} else {
				tmp_width = usr_config[i].width;
				tmp_height = usr_config[i].height;
			}
			if (usr_config[i].out_ratio <= 0) {
				printf("Output ratio can't be less than 0\n");
				return -1;
			}
			if ((tmp_width / usr_config[i].out_ratio >
			     SCREEN_MAX_WIDTH)
			    || (tmp_height / usr_config[i].out_ratio >
				SCREEN_MAX_HEIGHT)) {
				printf
				    ("Output image can't be larger than %d*%d\n",
				     SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
				return -1;
			}
		}
	}
	return 0;
}

//
//int main(int argc, char *argv[])
//{
//	int ret = -1;
//	int i;
//
//	char codec_name[80] = { 0 };	/ * enc/dec config file name * /
//	FILE *_codec_fp = NULL;
//
//	int codec_num;
//	struct codec_config codec_param[MAX_NUM_INSTANCE];
//
//	vpu_versioninfo vpu_ver;
//
//	printf("VOIP demo on Freescale i.mx27\n");
//
//	while ((ret = getopt(argc, argv, "h:c:")) >= 0) {
//		switch (ret) {
//		case 'h':
//			printf
//			    ("Usage: %s [-h] [-c codec cfg] [-r emma cfg] <ip_addr>\n"
//			     "\t-h	prints this message\n"
//			     "\t-c		codec config file", argv[0]);
//			return 0;
//		case 'c':
//			strcpy(codec_name, optarg);
//			break;
//		}
//	}
//
//	_codec_fp = fopen(codec_name, "r");
//	if (!_codec_fp) {
//		printf("codec: cannot open codec config file\n");
//		return 1;
//	}
//	ret = vpu_getcfg(&codec_num, codec_param, _codec_fp);
//	fclose(_codec_fp);
//	if (ret < 0)
//		return -1;
//
//	/* Note: currently it's only one enc/dec pair here under this condition. */
//	if (codec_num == 2) {
//		int enc_idx, dec_idx;
//		for (i = 0; i < codec_num; i++) {
//			if (codec_param[i].enc_flag == 0) {
//				dec_idx = i;
//			} else {
//				enc_idx = i;
//			}
//		}
//		if (strcmp(codec_param[enc_idx].dst_name, codec_param[dec_idx].src_name)) {
//			printf("Warning: not setting the loopback mode.\n");
//		}
//
//		if (access(codec_param[enc_idx].dst_name, F_OK) == 0) {
//			struct stat buf;
//			if (lstat(codec_param[enc_idx].dst_name, &buf) < 0) {
//				printf("lstat error\n");
//				return -1;
//			} else {
//				if (S_ISFIFO(buf.st_mode)) {
//					printf("FIFO already exists.\n");
//				} else {	/ * if file exists but no FIFO, delete it first. * /
//					if (unlink(codec_param[enc_idx].dst_name) < 0) {
//						printf("delete file error.\n");
//					} else {
//						goto MKFIFO;
//					}
//				}
//			}
//		} else {
//MKFIFO:
//			if(mkfifo(codec_param[enc_idx].dst_name,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0) {
//				printf("make fifo failed.\n");
//				return -1;
//			}
//		}
//
//	}
//
//	ret = vpu_SystemInit();
//	if (ret < 0)
//		return -1;
//	DPRINTF("codec_num %d\n", codec_num);
//
//	ret = vpu_GetVersionInfo(&vpu_ver);
//	printf("VPU Firmware Version: %d.%d.%d\n", vpu_ver.fw_major, vpu_ver.fw_minor, vpu_ver.fw_release);
//	printf("VPU Library Version: %d.%d.%d\n", vpu_ver.lib_major, vpu_ver.lib_minor, vpu_ver.lib_release);
//
//	if (ret != 0) {
//		printf("Can't get the firmware version info. Error code is %d\n", ret);
//	}
//
//	sigemptyset(&mask);
//	sigaddset(&mask, SIGINT);
//	if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {
//		printf("SIG BLOCK error\n");
//	}
//
//	ret = pthread_create(&sig_thread_id, NULL, sig_thread, NULL);
//	if (ret < 0) {
//		printf("Create signal monitor thread Error!\n");
//		return -1;
//	}
//
//	for (i = 0; i < codec_num; i++) {
//		codec_param[i].index = i;
//		switch (codec_param[i].enc_flag) {
//		case 0:{
//				ret =
//				    pthread_create(&codec_thread[i],
//						   NULL, DecodeTest,
//						   (void *)(&(codec_param[i])));
//				if (ret < 0) {
//					printf("Create Dec thread Error!\n");
//					vpu_SystemShutdown();
//					return -1;
//				}
//				break;
//			}
//		case 1:{
//				ret =
//				    pthread_create(&codec_thread[i],
//						   NULL, EncodeTest,
//						   (void *)(&(codec_param[i])));
//				if (ret < 0) {
//					printf("Create Enc thread Error!\n");
//					vpu_SystemShutdown();
//					return -1;
//				}
//				break;
//			}
//		default:
//			printf("Please input 0 to 2\n");
//			return -1;
//		}
//	}
//	for (i = 0; i < codec_num; i++) {
//		codec_param[i].index = i;
//		switch (codec_param[i].enc_flag) {
//		case 1:{	/* Note: we put this order deliberately here. */
//				ret = pthread_join(codec_thread[i], NULL);
//				if (ret < 0) {
//					printf("Create Dec thread Error!\n");
//					vpu_SystemShutdown();
//					return -1;
//				}
//				break;
//			}
//		case 0:{
//				ret = pthread_join(codec_thread[i], NULL);
//				if (ret < 0) {
//					printf("Create Enc thread Error!\n");
//					vpu_SystemShutdown();
//					return -1;
//				}
//				break;
//			}
//		default:
//			printf("Please input 0 to 2\n");
//			return -1;
//		}
//	}
//
//	vpu_SystemShutdown();
//	DPRINTF("codec main ended\n");
//	return 0;
//}
//
//*/
