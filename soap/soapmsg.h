

//gsoap ns service name:	ns  service
//gsoap ns service protocol:	SOAP
//gsoap ns service style:	rpc
//gsoap ns service encoding:	encoded
//gsoap ns service namespace:	http://localhost/ns.wsdl
//gsoap ns service location:	http://localhost:2298

//gsoap ns schema namespace:	urn:ns


/*********************************900.1a protocol**************************************************/
struct ns__MsgDataPointer {
	unsigned char * __ptr;
	int __size;
};

struct stMsg
{
	unsigned long				function;
	struct ns__MsgDataPointer	data;
	int	   						len;
	unsigned long 				ip;
};


int ns__SendMsg(struct stMsg in, struct stMsg *out);


/*********************************web diagnose**************************************************/

struct ns__StrPointer {
	unsigned char * __ptr;
	int __size;
};


struct stStrList
{
	struct ns__StrPointer * __ptr;
	int __size;
};

int ns__ATTReflash(unsigned int ID,unsigned int IP,unsigned int Port, struct ns__StrPointer TraceName, struct ns__StrPointer *TraceStr);
int ns__ATTGetSessionID(unsigned int * pID);
int ns__ATTGetTraceName(unsigned int IP,unsigned int Port ,struct stStrList * out);
int ns__ATTGetAnalyseName(unsigned int IP,unsigned int Port ,struct stStrList * out);
int ns__ATTGetAnalyse(unsigned int IP,unsigned int Port, struct ns__StrPointer AnalyseName, struct ns__StrPointer *AnalyseStr);
