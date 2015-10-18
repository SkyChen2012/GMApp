

#ifndef SOAPCOMMON_H
#define SOAPCOMMON_H

#include "Dispatch.h"
#include "soapStub.h"


#define	SOAP_MSG_LISTEN_PORT1	0x8FA
#define	SOAP_MSG_LISTEN_PORT2	0x8FB

#define SOAP_SEND_TIMEOUT	3
#define SOAP_RECV_TIMEOUT	3

#define SOAP_SRV_ENDPOINT "http://%s:%d"

//extern void * SoapMsgServerMain(void *arg);
extern void * SoapMsgServerMain1(void *arg);
extern void * SoapMsgServerMain2(void *arg);
extern void MoxSoapExit(void);
extern void MoxSoapInit(void);
extern int SoapSendMessage(MXMSG *msg);
extern void SendMsg(struct soap *soap,struct stMsg in, struct stMsg *out);

//get continuous send error count
extern int MOX_SOAP_GetSendErrCnt(void);
#endif

/* End of soapcommon.h */
