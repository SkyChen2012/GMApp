#include "soapH.h"
#include "MXTypes.h"
#include "MXMem.h"
#include "ns.h"
#include "soapcommon.h"
#include "Dispatch.h"
#include "MXMdId.h"
#include "MXMsg.h"
#include "soapStub.h"

//#define SOAP_MSG_SERVER_DEBUG

//void * SoapMsgServerMain(void *arg) {
//	SOAP_SOCKET m, s; /* master and slave sockets */
//	struct soap *soap;
//	soap = (struct soap *) arg;
//	BOOL bBind = FALSE;
//
//	soap_init(soap);
////	soap_init2(soap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);
//	soap->send_timeout = -1000*300;	//SOAP_SEND_TIMEOUT;
//	soap->recv_timeout = -1000*300;	//SOAP_RECV_TIMEOUT;
//	soap->connect_timeout = -1000*300;
//	soap->tcp_keep_alive = 0;
//	soap->keep_alive = 0;
//	soap->linger_time = 0;
//	soap->bind_flags = SO_REUSEADDR;
//
//	while (!bBind) {
//		if (MOX_MAIN_isServer()) {
//			printf(
//					"[vsa]thread:SoapMsgServerMain port:%d\n", SOAP_MSG_PORT_VSA);
//			m = soap_bind(soap, NULL, SOAP_MSG_PORT_VSA, 100);
//		} else {
//			printf(
//					"[vca]thread:SoapMsgServerMain port:%d\n", SOAP_MSG_PORT_VCA);
//			m = soap_bind(soap, NULL, SOAP_MSG_PORT_VCA, 100);
//		}
//		if (soap_valid_socket(m)) {
//			bBind = TRUE;
//		} else {
//			printf("bind error.");
//			sleep(1);
//		}
//	}
//	printf("Socket connection successful: master socket = %d\n", m);
//
//	for (;;) {
//		s = soap_accept(soap);
//
//		if (soap_valid_socket(s)) {
//		//	printf("Socket connection successful: slave socket = %d\n", s);
//			soap_serve(soap);
//			soap_end(soap);
//		} else {
//			printf("soap_accept sock error.");
//		}
//
//	}
//	return NULL ;
//}

void * SoapMsgServerMain1(void *arg) {
	SOAP_SOCKET m, s; /* master and slave sockets */
	struct soap *soap;
	soap = (struct soap *) arg;
	BOOL bBind = FALSE;

	soap_init(soap);
//	soap_init2(soap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);
	soap->send_timeout = -1000*300;	//SOAP_SEND_TIMEOUT;
	soap->recv_timeout = -1000*300;	//SOAP_RECV_TIMEOUT;
	soap->connect_timeout = -1000*300;
	soap->tcp_keep_alive = 0;
	soap->keep_alive = 0;
	soap->linger_time = 0;
	soap->bind_flags = SO_REUSEADDR;

	while (!bBind) {
		m = soap_bind(soap, NULL, SOAP_MSG_LISTEN_PORT1, 100);
		if (soap_valid_socket(m)) {
			bBind = TRUE;
		} else {
			printf("bind error.");
			sleep(1);
		}
	}
	printf("Socket connection successful: master socket = %d\n", m);

	for (;;) {
		s = soap_accept(soap);

		if (soap_valid_socket(s)) {
		//	printf("Socket connection successful: slave socket = %d\n", s);
			soap_serve(soap);
			soap_end(soap);
		} else {
			if(soap->errnum)
			{
				printf("%s soap_accept sock error num=%d.\n",__FUNCTION__,soap->errnum);
			}
			printf("%s soap_accept sock error.\n",__FUNCTION__);
			sleep(1);
		}

	}
	return NULL ;
}

void * SoapMsgServerMain2(void *arg) {
	SOAP_SOCKET m, s; /* master and slave sockets */
	struct soap *soap;
	soap = (struct soap *) arg;
	BOOL bBind = FALSE;

	soap_init(soap);
//	soap_init2(soap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);
	soap->send_timeout = -1000*300;	//SOAP_SEND_TIMEOUT;
	soap->recv_timeout = -1000*300;	//SOAP_RECV_TIMEOUT;
	soap->connect_timeout = -1000*300;
	soap->tcp_keep_alive = 0;
	soap->keep_alive = 0;
	soap->linger_time = 0;
	soap->bind_flags = SO_REUSEADDR;

	while (!bBind) {
		m = soap_bind(soap, NULL, SOAP_MSG_LISTEN_PORT2, 100);
		if (soap_valid_socket(m)) {
			bBind = TRUE;
		} else {
			printf("bind error.");
			sleep(1);
		}
	}
	printf("Socket connection successful: master socket = %d\n", m);

	for (;;) {
		s = soap_accept(soap);

		if (soap_valid_socket(s)) {
		//	printf("Socket connection successful: slave socket = %d\n", s);
			soap_serve(soap);
			soap_end(soap);
		} else {
			if(soap->errnum)
			{
				printf("%s soap_accept sock error num=%d.\n",__FUNCTION__,soap->errnum);
			}
			printf("%s soap_accept sock error.\n",__FUNCTION__);
			sleep(1);
		}

	}
	return NULL ;
}

int ns__SendMsg(struct soap *soap, struct stMsg in, struct stMsg *out) {
#ifdef SOAP_MSG_SERVER_DEBUG
	printf("ns__SendMsg");
	out->function=in.function+1;
	out->data = (char*)soap_malloc(soap, 100);
	out->data[0] = in.data[0]+1;
	out->data[1] = in.data[1]+1;
	out->data[2] = in.data[2]+1;
#else
	MXMSG MsgSend;
	printf("ns__SendMsg msg:%lx\n",in.function);
	switch (in.function) {
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
	case FCA_CG_READ_GM_PARA:
	case FCA_CG_WRITE_GM_PARA:
	case FCA_ACK_CG_READ_GM_PARA:
	case FCA_ACK_CG_WRITE_GM_PARA:
	{
		MsgSend.dwDestMd = MXMDID_PUBLIC;
		MsgSend.dwSrcMd = MXMDID_ETH;
		MsgSend.dwMsg = in.function;
		MsgSend.dwParam = soap->ip;
		MsgSend.wDataLen = in.data.__size;
		if (in.data.__size) {
			MsgSend.pParam = (unsigned char *) MXAlloc(in.data.__size);
			if (MsgSend.pParam != NULL ) {
				memcpy((unsigned char *) MsgSend.pParam, in.data.__ptr, in.data.__size);
			} else {
				printf("ns__SendMsg MXAlloc error.");
				MsgSend.wDataLen = 0;
			}
		} else {
			MsgSend.pParam = NULL;
		}
		MxPutMsg(&MsgSend);

		printf("[soap] rec msg:%lx,datalen=%d,ip:%lx\n", in.function,in.data.__size,MsgSend.dwParam);

	}
		break;
		//security alarm
	case FCA_SA_CONFIRM_ALARM:
	case FCA_ACK_SA_CONFIRM_ALARM:
	case FCA_SA_ARM_DISARM:
	case FCA_ACK_SA_ARM_DISARM:
	case FCA_SA_NTY_EMERGENCY_ALARM:
	case FCA_ACK_SA_NTY_EMERGENCY_ALARM:
	{
		MsgSend.dwDestMd	= MXMDID_SA;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= in.function;
		MsgSend.dwParam		= soap->ip;
		MsgSend.wDataLen	= in.data.__size;
		MsgSend.pParam		= (unsigned char *) MXAlloc(in.data.__size);
		printf("SOAP Rec Data len %u",MsgSend.wDataLen);
		memcpy((unsigned char *)MsgSend.pParam,in.data.__ptr, in.data.__size);
		MxPutMsg(&MsgSend);
		break;
	}
		//
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
	{
		MsgSend.dwDestMd	= MXMDID_TALKING;
		MsgSend.dwSrcMd		= MXMDID_ETH;
		MsgSend.dwMsg		= in.function;
		MsgSend.dwParam		= soap->ip;
		MsgSend.wDataLen	= in.data.__size;
		MsgSend.pParam		= (unsigned char *) MXAlloc(in.data.__size);
		printf("SOAP Rec Data len %u",MsgSend.wDataLen);
		memcpy((unsigned char *)MsgSend.pParam,in.data.__ptr, in.data.__size);
		MxPutMsg(&MsgSend);
		break;
	}
	default:
		printf("[SOAP] rec msg:%lx not handle", in.function);
		break;
	}


#endif
	return SOAP_OK;
}

int ns__ATTReflash(struct soap *soap, unsigned int ID,unsigned int IP,unsigned int Port, struct ns__StrPointer TraceName, struct ns__StrPointer *TraceStr)
{
	return SOAP_ERR;
}

int ns__ATTGetSessionID(struct soap *soap,UINT * pnID)
{

	return SOAP_ERR;
}

int ns__ATTGetTraceName(struct soap *soap,unsigned int IP,unsigned int Port ,struct stStrList * out)
{
	return SOAP_ERR;
}

int ns__ATTGetAnalyseName(struct soap *soap,unsigned int IP,unsigned int Port ,struct stStrList * out)
{
	return SOAP_ERR;
}
int ns__ATTGetAnalyse(struct soap *soap,unsigned int IP,unsigned int Port, struct ns__StrPointer AnalyseName, struct ns__StrPointer *AnalyseStr)
{
	return SOAP_ERR;
}


