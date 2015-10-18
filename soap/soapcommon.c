#include <stdio.h>
#include "MXMem.h"
#include "MXList.h"
//#include "MXTypes.h"

#include "MXMsg.h"
#include "MXMdId.h"
#include "BacpNetCtrl.h"
#include "Dispatch.h"
#include "soapcommon.h"
#include "soapH.h"
#include "soapStub.h"
#include "ParaSetting.h"
typedef	struct tagRunManStruct
{
	pthread_t					thread;
	BOOL						bThreadQuit;
} RunManStruct;

typedef struct tagPTSOAPDATA {
	RunManStruct 		stRunMan;
	RunManStruct 		stRunMan1;
	RunManStruct 		stRunMan2;
	struct 	soap 		stSoapClient;
	struct 	soap 		stSoapServer;
	struct 	soap 		stSoapServer1;
	struct 	soap 		stSoapServer2;
} PTSOAPDATA;

static PTSOAPDATA g_PtSoap;

void _SoapInitSever();
void MoxSoapInit(void)
{
//	MXRunThreadWithParam(&(g_PtSoap.stRunMan),SoapMsgServerMain,(void *)( &g_PtSoap.stSoapServer),"SoapMsgServer");
	soap_init2(&g_PtSoap.stSoapClient, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);
//	soap.connect_timeout = 0;
//	g_PtSoap.stSoapClient.send_timeout = SOAP_SEND_TIMEOUT;
//	g_PtSoap.stSoapClient.recv_timeout= SOAP_RECV_TIMEOUT;
//	g_PtSoap.stSoapClient.tcp_keep_alive = 0;
//	g_PtSoap.stSoapClient.keep_alive = 0;
	g_PtSoap.stSoapClient.send_timeout = -1000*300;	//SOAP_SEND_TIMEOUT;
	g_PtSoap.stSoapClient.recv_timeout = -1000*300;	//SOAP_RECV_TIMEOUT;
	g_PtSoap.stSoapClient.connect_timeout = -1000*300;

	_SoapInitSever();
}

void _SoapInitSever()
{
	//server port1(0x8FA)
	if ((pthread_create(&g_PtSoap.stRunMan1.thread, NULL, SoapMsgServerMain1,(void *)( &g_PtSoap.stSoapServer1))) != 0)
	{
		g_PtSoap.stRunMan1.bThreadQuit = TRUE;
	}
	//server port2(0x8FB)
	if ((pthread_create(&g_PtSoap.stRunMan2.thread, NULL, SoapMsgServerMain2,(void *)( &g_PtSoap.stSoapServer2))) != 0)
	{
		g_PtSoap.stRunMan2.bThreadQuit = TRUE;
	}

}

void MoxSoapExit(void)
{
	soap_destroy(&g_PtSoap.stSoapClient);
	soap_end(&g_PtSoap.stSoapClient);
	soap_done(&g_PtSoap.stSoapClient);
}

//1:已处理，0:未处理
int SoapSendMessage(MXMSG *msg)
{
	struct stMsg in;
	struct stMsg out;
	int ret=1;

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	switch (msg->dwMsg)
	{
		case FCA_PB_DATA_SYNC:
		case FCA_ACK_PB_DATA_SYNC:
		case FCA_PB_RECORD_UPDATE:
		case FCA_ACK_PB_RECORD_UPDATE:
		case FCA_PB_S2C_HEART_BEAT:
		case FCA_PB_C2S_HEART_BEAT:
		case FCA_PB_DEV_ONLINE:
		case FCA_ACK_PB_DEV_ONLINE:
		case FCA_PB_VER_NOTIFY:
		case FCA_ACK_PB_VER_NOTIFY:
		case FCA_CG_READ_PARA:
		case FCA_ACK_CG_READ_PARA:
		case FCA_CG_WRITE_PARA:
		case FCA_ACK_CG_WRITE_PARA:
		case FCA_CG_UPLD_END:
		case FCA_ACK_CG_UPLD_END:
		case FCA_CG_UPDATE_END:
		case FCA_ACK_CG_UPDATE_END:
		case FCA_PB_FILE_SYNC:
		case FCA_ACK_PB_FILE_SYNC:
			//security alarm
		case FCA_SA_CONFIRM_ALARM:
		case FCA_ACK_SA_CONFIRM_ALARM:
		case FCA_SA_ARM_DISARM:
		case FCA_ACK_SA_ARM_DISARM:
		case FCA_SA_NTY_EMERGENCY_ALARM:
		case FCA_ACK_SA_NTY_EMERGENCY_ALARM:
			//talking
		case FCA_IC_CALL:
		case FCA_IC_PICKUP:
		case FCA_IC_HANGUP:
		case FCA_IC_UNLK:
		case FCA_IC_RECORD:
		case FCA_IC_NTY_RING:
		case FCA_IC_NTY_CANCEL:
		case FCA_IC_NTY_UPDATE:
		case FCA_ACK_IC_CALL:
		case FCA_ACK_IC_PICKUP:
		case FCA_ACK_IC_HANGUP:
		case FCA_ACK_IC_UNLK:
		case FCA_ACK_IC_RECORD:
		case FCA_ACK_IC_NTY_RING:
		case FCA_ACK_IC_NTY_CANCEL:
		case FCA_ACK_IC_NTY_UPDATE:
		//access
		case FCA_AC_REQUEST_ISSUE_CARD:
		case FCA_ACK_AC_REQUEST_ISSUE_CARD:
		case FCA_AC_SWIPE_CARD:
		//vision control
		case FCA_VISION_DEV_REGISTER:
		case FCA_VISION_TAG_SET:
		case FCA_VISION_TAG_NOTIFY:
		case FCA_VISION_TAG_ADD:
		case FCA_ACK_VISION_DEV_REGISTER:
		case FCA_ACK_VISION_TAG_SET:
		case FCA_ACK_VISION_TAG_NOTIFY:
		case FCA_ACK_VISION_TAG_ADD:
		{
			if(MOX_PROTOCOL_SUPPORT(MOX_P_900_1A))
			{
				in.function = (unsigned short)msg->dwMsg;
				in.data.__ptr = msg->pParam;
				in.data.__size = msg->wDataLen;
				in.ip = msg->dwParam;
				printf("SoapSendMessage dwmsg %lx,dataLen %d\n",in.function,in.data.__size);
				SendMsg(&g_PtSoap.stSoapClient,in, &out);
			}
			else
				ret = 0;
			break;
		}
		case FCA_CG_READ_GM_PARA:
		case FCA_ACK_CG_READ_GM_PARA:
		case FCA_CG_WRITE_GM_PARA:
		case FCA_ACK_CG_WRITE_GM_PARA:
		{
			in.function = (unsigned short)msg->dwMsg;
			in.data.__ptr = msg->pParam;
			in.data.__size = msg->wDataLen;
			in.ip = msg->dwParam;
			printf("SoapSendMessage dwmsg %lx,dataLen %d\n",in.function,in.data.__size);
			SendMsg(&g_PtSoap.stSoapClient,in, &out);
			break;
		}
		default:
			ret=0;
			break;

	}
	return ret;
}
