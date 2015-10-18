/*hdr
 **
 **	Copyright Mox Products, Australia
 **
 **	FILE NAME:	ClientManagement.c
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
 **	NOTES:
 **
 */

#include "soapH.h"
#include "ns.h"
#include "soapcommon.h"
#include "MXTypes.h"
#include "MXMdId.h"

//const char server[] = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
/* = "http://localhost:8080"; to test against samples/webserver */

//const char server[] = "http://localhost:8080";

static void GetSoapEndPoint(DWORD dwIP, USHORT port, char * strEndPoint);
static int	iErrCnt = 0;
static DWORD mStartOfflineTickets;

void SendMsg(struct soap *soap, struct stMsg in, struct stMsg *out) {
	char server[255];

	GetSoapEndPoint(in.ip, SOAP_MSG_LISTEN_PORT2, server);

	printf("soap end point=%s\n", server);
	soap_call_ns__SendMsg(soap, server, "", in, out);

	if (soap->error) {
		DWORD currentTickets =  GetTickCount();
		printf("[error]soap send %lx message fail,error %d,error count %d",in.function,soap->error,iErrCnt);
		if(iErrCnt == 0){
			mStartOfflineTickets = GetTickCount();
			iErrCnt++;
		}
		else if(currentTickets - mStartOfflineTickets >= 3000){
			iErrCnt++;
		}
	}
	else {
		iErrCnt = 0;
	}
}

int MOX_SOAP_GetSendErrCnt(void){
	return iErrCnt;
}

static void GetSoapEndPoint(DWORD dwIP, USHORT port, char * strEndPoint) {
	struct in_addr addr;
	addr.s_addr = htonl(dwIP);
	sprintf(strEndPoint, SOAP_SRV_ENDPOINT, inet_ntoa(addr), port);
}
