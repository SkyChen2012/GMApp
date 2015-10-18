/*hdr
 **
 **	Copyright Mox Products, Australia
 **
 **	FILE NAME:	DIDOClient.c
 **
 **	AUTHOR:
 **
 **	DATE:
 **
 **	FILE DESCRIPTION:
 **
 **
 **	FUNCTIONS:
 **
 **
 **
 **
 **
 **	NOTES:
 **
 */
/************** SYSTEM INCLUDE FILES **************************************************************************/
/************** USER INCLUDE FILES ****************************************************************************/
//__SUPPORT_PROTOCOL_900_1A__
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>

#include "MXList.h"
#include "modnet.h"
#include "MoxDido.h"

/************** GLOBAL DECLARATIONS ***************************************************/
#define DIDO_CLIENT_DEBUG

#define BUF_SIZE 257

#define IOC_COIL_ON 0xff00
#define IOC_COIL_OFF 0x0000
#define LOGE	printf
/************** STRUCTURES *******************************************************/
//
//typedef struct tag_DIDOCLIENT {
//	DWORD dwIP;
//	int port;
//	int devID;
//	char* strPN;
//
//	MXListHead pChannelList;
//} DIDOCLIENT_t;

typedef	struct tagRunManStruct
{
	pthread_t					thread;
	BOOL						bThreadQuit;
} RunManStruct;

/************** LOCAL DECLARATIONS ***************************************************/

/*************************************************************************************/
/************** TYPEDEFS *************************************************************/
/************** STRUCTURES ***********************************************************/
int modnet_udp_sock = 0;

/**********************************************************************************/

static void runUdpThread(void);
static void runUdpSocket();
static void do_modnet_cmd(const UCHAR sock, const UCHAR *buf, const USHORT buf_len, struct sockaddr_in *remote);
static void response_modnet_cmd(const UCHAR sock, const UCHAR *buf, const USHORT buf_len, struct sockaddr_in *remote);
static int udp_send(int socket, const UCHAR *buf, int buflen, struct sockaddr_in *remote);

static RunManStruct g_RUN_UdpMan;

/**********************************************************************************/
void DIDOClient_Init() {
	LOGE("[%s]\n",__FUNCTION__);
	runUdpThread();
}

void DIDOClient_Exit() {
	g_RUN_UdpMan.bThreadQuit = TRUE;

	close(modnet_udp_sock);
}

void *
UdpThread(void *arg) {

	while (g_RUN_UdpMan.bThreadQuit == FALSE) {
		runUdpSocket();
	}

	pthread_exit(0);
	return NULL ;
}

static void runUdpThread(void) {

	g_RUN_UdpMan.bThreadQuit = FALSE;
	g_RUN_UdpMan.thread = 0;
	if ((pthread_create(&g_RUN_UdpMan.thread, NULL, UdpThread, NULL)) != 0) {
		LOGE("Run: create Vision thread fail\n");
		g_RUN_UdpMan.bThreadQuit = TRUE;
	}
}

static void runUdpSocket() {
	struct sockaddr_in LocalAddr;
	struct sockaddr_in cliaddr;
	int ret;
	socklen_t clilen;
	int recLen;
	UCHAR mesg[1000] = { 0 };
	int nBlockFlag = 0;

	modnet_udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (modnet_udp_sock < 0) {
		LOGE("[DIDO Client] UDP create socket error");
	}
	memset(&LocalAddr, 0, sizeof(LocalAddr));
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	LocalAddr.sin_port = htons(modnet_udp_port);
	ret = bind(modnet_udp_sock, (struct sockaddr*) &LocalAddr, sizeof(LocalAddr));
	LOGE("[DIDO Client] UDP bind return %d\n", ret);
	if (ret == 0) {
		ret = ioctl(modnet_udp_sock, FIONBIO, &nBlockFlag);
		LOGE("[DIDO Client] ioctl return %d\n", ret);
		for (;;) {
			clilen = sizeof(cliaddr);
			recLen = recvfrom(modnet_udp_sock, mesg, 1000, 0, (struct sockaddr *) &cliaddr, (socklen_t *) &clilen);
//			LOGE("udp receive %d,%s", recLen, mesg);
			do_modnet_cmd(modnet_udp_sock, mesg, recLen,&cliaddr);
		}
	}
}

static void send_modnet_exception(const UCHAR sock, const UCHAR *cmd, const UCHAR exp_code, struct sockaddr_in *remote) {
	UCHAR ack_cmd[BUF_SIZE] = { 0 };

	LOGE("send_modnet_exception");
	memcpy(&ack_cmd, cmd, 5);
	ack_cmd[5] = 3;
	ack_cmd[6] = cmd[6]; // addr
	ack_cmd[7] = cmd[7] | 0x80; // function code
	ack_cmd[8] = exp_code;

	udp_send(sock, ack_cmd, 9, remote);
}

static void response_modnet_cmd(const UCHAR sock, const UCHAR *buf, const USHORT buf_len, struct sockaddr_in *remote) {
	udp_send(sock, buf, buf_len, remote);
}

static void do_modnet_cmd(const UCHAR sock, const UCHAR *buf, const USHORT buf_len, struct sockaddr_in *remote) {
	UCHAR ack_cmd[BUF_SIZE];
	UCHAR func = 0;
	USHORT addr = (buf[8] << 8) + buf[9];
	USHORT count = (buf[10] << 8) + buf[11];

	func = buf[7];

	//check buf len
	if (buf_len < 12) {
		return;
	}

//	LOGE("func is %d",func);
	//check func code
	if (0x03 != func && 0x04 != func && 0x05 != func) {
		send_modnet_exception(sock, buf, 0x02, remote);
		return;
	}

	switch (func) {
	case 0x04:
	case 0x03: {
		UCHAR i = 0;
		USHORT reg_addr = (buf[8] << 8) + buf[9];
		USHORT reg_count = (buf[10] << 8) + buf[11];

		ack_cmd[8] = reg_count * 2;
//		LOGE("reg_addr %d",reg_addr);
//		LOGE("reg_count %d",reg_count);
		for (i = 0; i < reg_count; i++) {
			ack_cmd[2 * i + 9] = 0;
			ack_cmd[2 * i + 10] = MOX_DEV_DIRead(reg_addr+i + 1);
		}
		memcpy(ack_cmd, buf, 5);
		ack_cmd[5] = ack_cmd[8] + 3;
		memcpy(&ack_cmd[6], &buf[6], 2);
		response_modnet_cmd(sock, ack_cmd, 2 * i + 9, remote);
	}
		break;
	case 0x05:{
		USHORT coil_addr = (buf[8] << 8) + buf[9];
		USHORT coil_pre_val = (buf[10] << 8) + buf[11];
		LOGE("coil_addr %d",coil_addr);
		if (IOC_COIL_ON == coil_pre_val) {
			MOX_DEV_DOSet(coil_addr + 1);
		} else if (IOC_COIL_OFF == coil_pre_val) {
			MOX_DEV_DOClear(coil_addr + 1);
		} else {
			; // do nothing
		}
		memcpy(ack_cmd, buf, 5);
		ack_cmd[5] = 6;
		ack_cmd[6] = buf[6]; // addr
		ack_cmd[7] = buf[7]; // func
		ack_cmd[8] = buf[8];
		ack_cmd[9] = buf[9];
		if (IOC_COIL_ON == coil_pre_val) {	//get do status ,in here ,we need not to do it
			ack_cmd[10] = 0xff;
			ack_cmd[11] = 0x00;
		} else // coil off
		{
			ack_cmd[10] = 0x00;
			ack_cmd[11] = 0x00;
		}
		response_modnet_cmd(sock, ack_cmd, 12, remote);
	}
		break;
	default:
		send_modnet_exception(sock, buf, 0x01, remote); // invaild function code
		break;
	}
}

static int udp_send(int socket, const UCHAR *buf, int buflen, struct sockaddr_in *remote)
{
	//int index ;
	int sendlen = sendto(socket,
						 buf,
						 buflen,
						 0,
						 (struct sockaddr*)remote,
						 sizeof (struct sockaddr_in));
#ifdef MODBUS_FRAME_DEBUG
	DebugFrame("udp_send",buf,buflen);
	printf("udp_send length is %d",sendlen);
#endif
	return sendlen;
}


