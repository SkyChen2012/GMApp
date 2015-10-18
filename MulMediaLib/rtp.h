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

#ifndef _RTP_H
#define _RTP_H

#include "udp.h"

#define RTP_TYPE_ULAW		0
#define RTP_TYPE_ALAW		8
#define RTP_TYPE_L16S44		10
#define RTP_TYPE_L16M44		11
#define RTP_TYPE_H263		34
#define RTP_TYPE_DYNAMIC	96

typedef struct {
	char *name;		/* local session id */
	char *ip;		/* server ip */
	short port;		/* server port */
	unsigned short size;	/* rtp frame size */
	unsigned char type;	/* RTP_TYPE_xxx */
	int clock;		/* RTP clock rate */

	/* stats */
	unsigned int err;	/* #corrupted pkts */
	unsigned int skip;	/* #out of seq skipped */

	/* reserved - zeroed */
	struct sockaddr_in to;
	int sock;
	char *rxbuf;
	char *txbuf;
} rtp_t;

/*
 	bufsize is in # of RTP pkts to buffer in rx (to cut delays)
	0 - default socket buffer size
*/
extern int rtp_init(rtp_t *, int bufsize);
extern int rtp_tx(rtp_t *, char *buf, int len);
extern int rtp_rx(rtp_t *, char *buf, int len);
extern int rtp_exit(rtp_t *);

#endif
