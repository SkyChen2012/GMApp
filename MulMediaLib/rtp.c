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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <sys/time.h>
#include <errno.h>
#include "udp.h"
#include "rtp.h"

#define RTP_VER		2
#define RTP_PAD		(1 << 2)
#define RTP_EXT		(1 << 3)
#define RTP_CC(count)	((count) << 4)
#define RTP_MRK		(1 << 8)
#define RTP_TYP(typ)	((typ) << 9)

#pragma pack(1)
typedef struct {
	unsigned short flag;
	unsigned short seq;
	unsigned int time;
	unsigned int ssrc;
	/* out of spec additions */
	unsigned short len;
	char chk;
} rtph_t;
#pragma pack()

int rtp_init(rtp_t * t, int bufsize)
{
	rtph_t *h;
	int rt;

	if (t->size < 4 || t->clock < 1000) {
		return -2;
	}

	rt = t->size + sizeof(rtph_t);
	t->rxbuf = calloc(2, rt);
	if (!t->rxbuf) {
		return -1;
	}
	t->txbuf = t->rxbuf + rt;
	h = (rtph_t *) t->txbuf;
	h->flag = RTP_VER | RTP_TYP(t->type);

	t->sock = udp_open(t->port, t->ip, &t->to);
	if (t->sock < 0) {
		free(t->rxbuf);
		return -3;
	}

	if (bufsize > 0) {
		bufsize *= rt;
		setsockopt(t->sock, SOL_SOCKET, SO_RCVBUF, &bufsize, 4);
	}

	return 0;
}

int rtp_exit(rtp_t * t)
{
	free(t->rxbuf);
	close(t->sock);
	return 0;
}

#if 0
char chk_gen(char *buf, int len)
{
	char chk;

	chk = 0;
	while (len--) {
		chk ^= *buf++;
	}
	return chk;
}
#else
#define chk_gen(buf, len)	0
#endif

extern int g_state;
int rtp_tx(rtp_t * t, char *buf, int len)
{
	rtph_t *h;
	int tlen;
	int rt;
	struct timeval tv;

	h = (rtph_t *) t->txbuf;
	h->flag &= ~RTP_MRK;
	gettimeofday(&tv, 0);
	h->time = (tv.tv_sec * 1000000) + tv.tv_usec;
	h->len = t->size;
	for (tlen = 0; len > 0; tlen += h->len) {
		if (len <= t->size) {
			h->len = len;
			h->flag |= RTP_MRK;
		}

		memcpy(h + 1, buf, h->len);
		rt = t->size + sizeof(rtph_t);
		h->chk = 0;
//g_state=123;
		h->chk = chk_gen((char *)h, rt);
//g_state=124;
		rt = sendto(t->sock, h, rt, 0,
			    (struct sockaddr *)&t->to, sizeof(t->to));
		if (rt < 0) {
			printf("%s: rtp_tx err %d %d\n", t->name, rt, errno);
			return rt;
		}

		h->seq++;
		buf += h->len;
//		tlen += h->len;
		len -= h->len;
	}
	return tlen;
}

int rtp_rx(rtp_t * t, char *buf, int len)
{
	rtph_t *h;
	int tlen;
	int rt;
	unsigned short seq;

	tlen = 0;
	h = (rtph_t *) t->rxbuf;
	seq = h->seq;
	do {
		int slen;

		slen = sizeof(t->to);
		rt = recvfrom(t->sock, h, t->size + sizeof(rtph_t),
			      MSG_WAITALL, (struct sockaddr *)&t->to, &slen);
		if (rt < 0) {
			printf("%s: rtp_rx err %d\n", t->name, rt);
			return rt;
		}
		if (!rt) {
			printf("%s: rtp_rx warn 0-size-pkt\n", t->name);
			t->err++;
			continue;
		}

		rt = chk_gen((char *)h, t->size + sizeof(rtph_t));
		if (rt) {
//                      printf("%s: rtp_rx warn pkt %d corrupt chk=%02X\n", 
//                              t->name, seq, rt);
			t->err++;
		      lp_skip:
			h->flag &= ~RTP_MRK;
			continue;
		}

		if (h->len > t->size) {
			printf("%s: rtp_rx warn pktlen=%d > buflen=%d\n",
			       t->name, h->len, t->size);
			t->err++;
			goto lp_skip;
		}

		if (seq != h->seq) {
//                      printf("%s: rtp_rx exp %d got %d\n", 
//                              t->name, seq, h->seq);
			t->skip++;
#if 0
			if ((seq < h->seq && h->seq - seq > 5) ||
			    (seq > h->seq && seq - h->seq < 65531)) {
				/* out of order packet */
				printf("%s: rtp_rx dropping %d\n",
				       t->name, h->seq);
				continue;
			}
#endif
			/* skip remaining frags of pkt */
			while (!(h->flag & RTP_MRK)) {
				slen = sizeof(t->to);
				rt = recvfrom(t->sock, h,
					      t->size + sizeof(rtph_t),
					      MSG_WAITALL,
					      (struct sockaddr *)&t->to, &slen);
				if (rt < 0) {
//                                      printf("%s: rtp_rx(skip) err %d\n", 
//                                              t->name, rt);
					return rt;
				}
				rt = chk_gen((char *)h,
					     t->size + sizeof(rtph_t));
				if (rt) {
					printf("%s: rtp_rx(skip) warn pkt "
					       "corrupt chk=%02X\n",
					       t->name, rt);
					h->flag &= ~RTP_MRK;
					t->err++;
				} else {
					t->skip++;
				}
			}
			tlen = 0;
			h->flag &= ~RTP_MRK;
			h->seq++;
			tlen = 0;	/* discard previous frags of pkt */
			seq = h->seq;
			continue;
		}

		h->seq++;
		seq = h->seq;
		tlen += h->len;
		if (tlen > len) {
			/* more data to read */
			printf("%s: rtp_rx warn size %d < %d\n",
			       t->name, len, tlen);
			return tlen;
		}

		memcpy(buf + tlen - h->len, h + 1, h->len);
	}
	while (!(h->flag & RTP_MRK));
	return tlen;
}
