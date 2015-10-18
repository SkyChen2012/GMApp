/**************************************************************************
File:               dsys0soc.c
Author:             CJ International
Creation date:      19-October-2000
***************************************************************************
Attached documents: 

***************************************************************************
Description:        Socket abstraction layer.
                    File for Windows NT.

***************************************************************************
Modifications: (who / date / description)

***************************************************************************/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/timeb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <termios.h>
#include <errno.h>

#include "MXTypes.h"
#include "dsys0soc.h"

/* constants **************************************************************/
//#define  _DEBUG_SYSSOC
/* Bad return value of socket function */
#define SOCKET_ERROR			-1
#define _SOC_ERROR               SOCKET_ERROR

/* Parameter for shutdown function */
#define _SOC_SHUTDOWN            SD_BOTH

/* Macro to handle socket error */
#define _SOC_EWOULDBLOCK         (errno == EWOULDBLOCK)
#define _SOC_ESHUTDOWN           (errno == ESHUTDOWN)
#define _SOC_EHOSTDOWN           (errno == EHOSTDOWN)
#define _SOC_ECONNRESET          (errno == ECONNRESET)

#ifdef _DEBUG_SYSSOC
#define _SOC_ERRNO               (errno)
#endif

#define	dsysFctErrnoSet

/* types ******************************************************************/

/* static data ************************************************************/
//static uchar _cuSOC_NBRINIT = 0;

/* exported data [NOT RECOMMENDED!] ***************************************/

/* imported data [NOT RECOMMENDED!] ***************************************/

/* callback services implemented in this file *****************************/

/* exported services ******************************************************/

/* imported services ******************************************************/

/* static services ********************************************************/



/**************************************************************************** 
function    : dsysSocInit
description : Initialize socket subsystem.
 This routine is reentrant.
return value: 0 if successful, BAD_RET if an error occurs.
warning     : 
****************************************************************************/
typSTATUS dsysSocInit(void)
{
    return(0);
}

/**************************************************************************** 
function    : dsysSocExit
description : Terminates use of the sockets.
return value: None.
warning     : 
****************************************************************************/
void dsysSocExit(void)
{
#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Exit socket layer\n");
#endif
}

/****************************************************************************
function    : dsysSocAddressSet
description : Solve a TCP/IP address given by a string (A.B.C.D or name of
 computer)
return value: 0 if successful, BAD_RET if an error occurs.
warning     : 
****************************************************************************/
typSTATUS dsysSocAddressSet
   (
   char*          psIpAddress,   /* In: IP address */
   uint16         huPort,        /* In: Port */
   typSOC_ADD*    pSocAdd        /* Out: Socket address */
   )
{
   //struct hostent* pHostDef;

#ifdef _DEBUG_SYSSOC
   if (psIpAddress == 0)
      printf("SYS-SOC: Set socket address port %hu\n", huPort);
   else
      printf("SYS-SOC: Set socket address %s port %hu\n", psIpAddress, huPort);
#endif

   memset(pSocAdd, 0, sizeof(typSOC_ADD));
   
   pSocAdd->sin_family = AF_INET; 

   if (psIpAddress == 0)
   {
      /* Use default address */
      pSocAdd->sin_addr.s_addr = INADDR_ANY;
   }
   else
   {
      pSocAdd->sin_addr.s_addr= htonl(INADDR_ANY);   
   }

   pSocAdd->sin_port = htons(huPort);

   return(0);
}

/****************************************************************************
function    : dsysSocAddressGet
description : 
return value: 0 if successful, BAD_RET if an error occurs.
warning     : 
****************************************************************************/
typSTATUS dsysSocAddressGet
   (
   typSOC_ADD*    pSocAdd,       /* In: Socket address */
   char*          psIpAddress,   /* Out: IP address */
   uint16         huIpAddSz,     /* In: Size of IP address buffer */
   uint16*        phuPort        /* Out: Port */
   )
{
//   char* psBuffer;
/*
#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Retrieve address\n");   
#endif

   *phuPort = ntohs(pSocAdd->sin_port);

   psBuffer = inet_ntoa(pSocAdd->sin_addr);
   
   if (psBuffer != 0)
   {
      if (strlen(psBuffer) < huIpAddSz)
      {
         strcpy(psIpAddress, psBuffer);
      }
   }
*/
//   dsysFctErrnoSet(ISA_ER_SOC_ADDRESS);
   return(BAD_RET);
}

/****************************************************************************
function    : dsysSocCreate
description : Create a new unaffected socket.
  
  ARPA Internet address family     AF_INET
  Stream sockets (TCP)             SOCK_STREAM
  Internet protocol (IP)           IPPROTO_TCP

return value: 0 if successful, BAD_RET if an error occurs.
warning     : 
****************************************************************************/
typSTATUS dsysSocCreate
   (
   typSOC_ID*     pSocId,        /* Out: Socket identifier */
   uchar          cuBlocking     /* In: Blocking option */
   )
{
#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Create socket\n");
#endif

   *pSocId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

   if (ISA_SOC_ISINVALID(pSocId))
   {
#ifdef _DEBUG_SYSSOC
      printf("SYS-SOC: Create socket failed, fd=%d\n", *pSocId);
#endif
//      dsysFctErrnoSet(ISA_ER_SOC_CREATE);
      return(BAD_RET);
   }

   /* Change socket mode */
   if (dsysSocOptionSet(pSocId, ISA_SOC_OPT_BLOCKING, cuBlocking) == BAD_RET)
   {
      dsysSocClose(pSocId, FALSE);
      return(BAD_RET);
   }

   return(0);
}

/****************************************************************************
function    : dsysSocClose
description : Close and remove a socket descriptor.
return value: None. 
warning     : 
****************************************************************************/
void dsysSocClose
   (
   typSOC_ID*     pSocId,        /* In: Socket identifier */
   uchar          cuShutdown     /* In: TRUE if shutdown is required */
   )
{
#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Close socket\n");
#endif

   /* Sends and receives are disallowed */
   if (cuShutdown == TRUE)
   {
      shutdown(*pSocId, 0x02);
   }
   
   /* Close path */
   close(*pSocId);

   /* Reset socket identifier */
   ISA_SOC_RESETID(pSocId);
}

/****************************************************************************
function    : dsysSocBind
description : Affect a name to a socket.
return value: 0 if successful, BAD_RET if an error occurs. 
warning     : 
****************************************************************************/
typSTATUS dsysSocBind
   (
   typSOC_ID*     pSocId,        /* In: Socket identifier */
   typSOC_ADD*    pSocAdd        /* In: Socket address */
   )
{
   int OpStatus;
   int						nBlockFlag	= 1;   

#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Bind socket\n");
#endif
   
   /* Assign a name to the socket */
   OpStatus = bind(*pSocId, (struct sockaddr*)pSocAdd, sizeof(typSOC_ADD));
   
   ioctl(*pSocId, FIONBIO, &nBlockFlag);

   if (OpStatus == _SOC_ERROR)
   {
#ifdef _DEBUG_SYSSOC
      printf("SYS-SOC: Bind socket failed\n");
#endif
//      dsysFctErrnoSet(ISA_ER_SOC_BIND);
      return(BAD_RET);
   }

   return(0);
}

/****************************************************************************
function    : dsysSocListen
description : Affect a socket to the reception of connection requests.
return value: 0 if successful, BAD_RET if an error occurs. 
warning     : 
****************************************************************************/
typSTATUS dsysSocListen
   (
   typSOC_ID*     pSocId,        /* In: Socket identifier */
   int32          ldNbrCnx       /* In: Max nbr of pending connections */
   )
{
   int OpStatus;

#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Listen socket\n");
#endif

   /* Make the socket ready to accept connections */
   OpStatus = listen(*pSocId, ldNbrCnx);
   
   if (OpStatus == _SOC_ERROR)
   {
#ifdef _DEBUG_SYSSOC
      printf("SYS-SOC: Listen socket failed %i\n", _SOC_ERRNO);
#endif
//      dsysFctErrnoSet(ISA_ER_SOC_LISTEN);
      return(BAD_RET);
   }

   return(0);
}

/****************************************************************************
function    : dsysSocAccept
description : Accept a new connection from a remote host.
return value:
   1 if a new connection is accepted
   0 if no new connection
   BAD_RET if an error occurs
warning     : The pSocAdd parameters can be null.
****************************************************************************/
int32 dsysSocAccept
   (
   typSOC_ID*     pSocId,        /* In: Socket identifier */
   typSOC_ID*     pSocAccepted,  /* Out: New socket identifier */
   typSOC_ADD*    pSocAdd        /* Out: Socket address */
   )
{
   int SocAddSz = sizeof(typSOC_ADD);
   typSOC_ADD SocAdd;
   typSOC_ADD* pSocAddress;

   if (pSocAdd == 0) pSocAddress = &SocAdd;
   else pSocAddress = pSocAdd;
   
   /* Accept a new connection */
   *pSocAccepted = accept(*pSocId, (struct sockaddr *)pSocAddress, &SocAddSz);

   if (ISA_SOC_ISINVALID(pSocAccepted))
   {
      if (_SOC_EWOULDBLOCK)
      {
         /* Non blocking call */
         return(0);
      }
      else
      {
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Accept socket failed\n");
#endif
//         dsysFctErrnoSet(ISA_ER_SOC_ACCEPT);
         return(BAD_RET);
      }
   }

#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: A new socket is accepted\n");
#endif

   return(1);
}

/****************************************************************************
function    : dsysSocConnect
description : Connect an unaffected socket to a remote node. The socket 
connection completion can be checked with the socketCheckConnect function.
return value: 
   1 if a connect successfully completed.
   0 if in progress.
   BAD_RET if an error occurs.
warning     : 
****************************************************************************/
int32 dsysSocConnect
   (
   typSOC_ID*     pSocId,        /* In: Socket identifier */
   typSOC_ADD*    pSocAdd        /* In: Socket address */
   )
{
   int OpStatus;

#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Connect to a socket\n");   
#endif

   /* Connect to remote node, this operation will not block */
   OpStatus = connect(*pSocId, (struct sockaddr*)pSocAdd, sizeof(typSOC_ADD));

   if (OpStatus == _SOC_ERROR)
   {
      if (_SOC_EWOULDBLOCK)
      {
         /* Non blocking call */
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Connect in progress\n");   
#endif
         return(0);
      }
      else
      {
#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Connect failed\n");   
#endif
//         dsysFctErrnoSet(ISA_ER_SOC_CONNECT);
         return(BAD_RET);
      }
   }

#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Connect successfully completed\n");   
#endif

   return(1);
} 

/****************************************************************************
function    : dsysSocCheckConnect
description : Check that the socket has been correctly connected.
return value: 
   1 if socket connected
   0 if socket not yet connected or connection in progress
   BAD_RET if socket connection fail
warning     : 
****************************************************************************/
int32 dsysSocCheckConnect
   (
   typSOC_ID*     pSocId,        /* In: Socket identifier */
   uint32         luTimeOut      /* In: Time Out */
   )
{
   fd_set CheckForConnect;
   fd_set CheckForError;
   struct timeval Delay;
   int OpStatus;

   FD_ZERO(&CheckForConnect);
   FD_ZERO(&CheckForError);
   FD_SET(*pSocId, &CheckForConnect);
   FD_SET(*pSocId, &CheckForError);

#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Check connect status\n");   
#endif

   Delay.tv_sec = luTimeOut / 1000;
   Delay.tv_usec = (luTimeOut - (Delay.tv_sec * 1000)) * 1000;

   OpStatus = select(FD_SETSIZE, NULL, &CheckForConnect, &CheckForError, &Delay);

   if (OpStatus == _SOC_ERROR)
   {
#ifdef _DEBUG_SYSSOC
      printf("SYS-SOC: Connect failed\n");   
#endif
//      dsysFctErrnoSet(ISA_ER_SOC_CONNECT);
      return(BAD_RET);
   }
   else if (OpStatus > 0)
   {
      if (FD_ISSET(*pSocId, &CheckForConnect))
      {
         /* Connected */
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Connect successfully completed\n");   
#endif
         return(1);
      }

      if (FD_ISSET(*pSocId, &CheckForError))
      {
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Connect failed\n");   
#endif
//         dsysFctErrnoSet(ISA_ER_SOC_CONNECT);
         return(BAD_RET);
      }
   }
   return(0);
}

/****************************************************************************
function    : dsysSocReceive
description : Read data from a connected socket.
return value: Number of bytes really received, BAD_RET if an error occurs.
warning     : 
****************************************************************************/
int32 dsysSocReceive
   (
   typSOC_ID*     pSocId,        /* In: Socket identifier */
   void*          pvDest,        /* Out: Location to store data */
   int32          ldBytesNbr     /* In: Number of bytes to receive */
   )
{
   int32 ldReturn;

   ldReturn = recv(*pSocId, (char*)pvDest, ldBytesNbr, 0);

   if (ldReturn == 0)
   {
      if (ldBytesNbr != 0)
      {
         /* Remote socket has been gracefully closed */
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Remote socket is closed\n");   
#endif
//         dsysFctErrnoSet(ISA_ER_SOC_BROKEN);
         return(BAD_RET);
      }
      else
      {
         return(0);
      }
   }

   if (ldReturn == _SOC_ERROR)
   {      
      if (_SOC_EWOULDBLOCK)
      {
         /* Non blocking call */
         return(0);
      }

      if ((_SOC_ESHUTDOWN) || (_SOC_EHOSTDOWN) || (_SOC_ECONNRESET))
      {
         /* Connection broken */
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Socket connection broken %i\n",_SOC_ERRNO);   
#endif
//         dsysFctErrnoSet(ISA_ER_SOC_BROKEN);
         return(BAD_RET);
      }

#ifdef _DEBUG_SYSSOC
      printf("SYS-SOC: Socket receive error %i\n", _SOC_ERRNO);
#endif

//      dsysFctErrnoSet(ISA_ER_SOC_RECEIVE);
      return(BAD_RET);
   }

   /* Return number of bytes read */
#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Socket receive %ld bytes\n", ldReturn);   
#endif

   return(ldReturn);
}

/****************************************************************************
function    : dsysSocCheckReceive
description : Check that the socket has data to be read.
parameters  :
   (input) typETCPSOCKET* pSocket
      Socket to check for connection
return value: int16
   1 if socket ready to be read
   0 if no data available on this socket
   BAD_RET if socket error.
warning     : 
****************************************************************************/
int32 dsysSocCheckReceive
   (
   typSOC_ID*     pSocId,        /* In: Socket identifier */
   uint32         luTimeOut      /* In: Time Out */
   )
{
   fd_set CheckForRead;
   fd_set CheckForExcept;
   struct timeval Delay;
   int OpStatus;

   FD_ZERO(&CheckForRead);
   FD_ZERO(&CheckForExcept);
   FD_SET(*pSocId, &CheckForRead);
   FD_SET(*pSocId, &CheckForExcept);

#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Check receive \n");   
#endif

   Delay.tv_sec = luTimeOut / 1000;
   Delay.tv_usec = (luTimeOut - (Delay.tv_sec * 1000)) * 1000;

   OpStatus = select(FD_SETSIZE, &CheckForRead, NULL, &CheckForExcept, &Delay);

   if (OpStatus == SOCKET_ERROR) 
   {
#ifdef _DEBUG_SYSSOC
      printf("SYS-SOC: Check receive failed\n");
#endif
//      dsysFctErrnoSet(ISA_ER_SOC_BROKEN);
      return(BAD_RET);
   }
   else if (OpStatus > 0)
   {
      if (FD_ISSET(*pSocId, &CheckForExcept))
      {
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Connection broken\n");   
#endif
//         dsysFctErrnoSet(ISA_ER_SOC_BROKEN);
         return(BAD_RET);
      }

      if (FD_ISSET(*pSocId, &CheckForRead))
      {
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Data available to read\n");   
#endif
         return(1);
      }
   }

#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Check received  No Data - No Except \n");
#endif

   return(0);
}

/****************************************************************************
function    : dsysSocSend
description : This routine sends data to a socket.
return value: The number of bytes read, BAD_RET if an error occurs.
warning     : 
****************************************************************************/
int32 dsysSocSend
   (
   typSOC_ID*     pSocId,        /* In: Socket identifier */
   void*          pvSrc,         /* In: Location to get data */
   int32          ldBytesNbr     /* In: Number of byes to send */
   )
{
   int32 ldReturn;
   
   ldReturn = send(*pSocId, (char*)pvSrc, ldBytesNbr, 0);

   if (ldReturn == _SOC_ERROR)
   {
      if (_SOC_EWOULDBLOCK)
      {
         /* Non blocking call */
         return(0);
      }

      if ((_SOC_ESHUTDOWN) || (_SOC_EHOSTDOWN) || (_SOC_ECONNRESET))
      {
         /* Connection broken */
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Connection broken %i\n", _SOC_ERRNO);
#endif
//         dsysFctErrnoSet(ISA_ER_SOC_BROKEN);
         return(BAD_RET);
      }

#ifdef _DEBUG_SYSSOC
      printf("SYS-SOC: Send failed %i\n", _SOC_ERRNO);
#endif

//      dsysFctErrnoSet(ISA_ER_SOC_SEND);
      return(BAD_RET);
   }

   /* Return number of bytes written */
#ifdef _DEBUG_SYSSOC
   printf("SYS-SOC: Send %ld of %ld bytes\n", ldReturn, ldBytesNbr);   
#endif
   return(ldReturn);
}


/****************************************************************************
function    : dsysSocOptionSet
description : This routine sets specific option to a socket.
return value: 0 if successful, BAD_RET if an error occurs.
warning     : 
****************************************************************************/
typSTATUS dsysSocOptionSet
   (
   typSOC_ID*     pSocId,        /* In: Socket identifier */
   int32          ldOption,      /* In: Option */
   int32          ldParam        /* In: Option value */
   )
{
   typSTATUS statError = 0;
   //unsigned long luParam;
   BOOL bParam;
   int iParam;
   struct   timeval timeout;
   struct linger Linger;

   switch (ldOption)
   {
      case ISA_SOC_OPT_BLOCKING:
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Set socket %s mode\n",
                (ldParam == ISA_SOC_BLOCKING) ? "Blocking" : "Non-blocking");   
#endif
//         luParam = (ldParam == ISA_SOC_BLOCKING) ? 0 : 1;
 
		timeout.tv_sec  = 0;
		timeout.tv_usec = 0;
      	//setsockopt(*pSocId, SOL_SOCKET, SO_BLOCKING, &timeout, sizeof(timeout));         
         break;

      case ISA_SOC_OPT_NODELAY:
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Set %s\n",
                (ldParam == ISA_SOC_NODELAY) ? "No delay" : "Packets by packets");   
#endif
         /* Configure Naggle algorithm depends on dynamic configuration */
         bParam = (ldParam == ISA_SOC_NODELAY) ? TRUE : FALSE;

         if (setsockopt(*pSocId, IPPROTO_TCP, TCP_NODELAY,
                        (char *)&bParam, sizeof(bParam)) == _SOC_ERROR)
         {
#ifdef _DEBUG_SYSSOC
            printf("SYS-SOC: Change TCP NO DELAY failed\n");   
#endif
//            dsysFctErrnoSet(ISA_ER_SOC_OPTION);
            statError = BAD_RET;
         }
         break;

      case ISA_SOC_OPT_KEEPALIVE:
         /* Keep connections alive */
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: %s keep alive\n",
                (ldParam == ISA_SOC_KEEPALIVE_ON) ? "Activate" : "Deactivate");   
#endif
         bParam = (ldParam == ISA_SOC_KEEPALIVE_ON) ? TRUE : FALSE;
         
         if (setsockopt(*pSocId, SOL_SOCKET, SO_KEEPALIVE,
                        (char *)&bParam, sizeof(bParam)) == _SOC_ERROR)
         {
#ifdef _DEBUG_SYSSOC
            printf("SYS-SOC: Change Keep Alive failed\n");   
#endif
//            dsysFctErrnoSet(ISA_ER_SOC_OPTION);
            statError = BAD_RET;
         }
         break;

      case ISA_SOC_OPT_SNDBUF:
         /* Configure send buffer */
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Set send buffer size to %ld\n", ldParam);   
#endif
         iParam = (int)ldParam;
         
         if (setsockopt(*pSocId, SOL_SOCKET, SO_RCVBUF,
                        (char *)&iParam, sizeof(iParam)) == _SOC_ERROR)
         {
#ifdef _DEBUG_SYSSOC
            printf("SYS-SOC: Failed to change sending buffer size\n");   
#endif
//            dsysFctErrnoSet(ISA_ER_SOC_OPTION);
            statError = BAD_RET;
         }
         break;

      case ISA_SOC_OPT_RCVBUF:
         /* Configure receive buffer */
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Set receive buffer size to %ld\n", ldParam);   
#endif
         iParam = (int)ldParam;
         
         if (setsockopt(*pSocId, SOL_SOCKET, SO_SNDBUF,
                        (char *)&iParam, sizeof(iParam)) == _SOC_ERROR)
         {
#ifdef _DEBUG_SYSSOC
            printf("SYS-SOC: Failed to change receiving buffer size\n");   
#endif
//            dsysFctErrnoSet(ISA_ER_SOC_OPTION);
            statError = BAD_RET;
         }
         break;
   
      case ISA_SOC_OPT_LINGER:
         /* Enable linger option */
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Enable linger option (Timeout %ld s)\n", ldParam);
#endif
         Linger.l_onoff = 1;
         Linger.l_linger = (unsigned short)ldParam;
         
         if (setsockopt(*pSocId, SOL_SOCKET, SO_LINGER,
                        (char *)&Linger, sizeof(Linger)) == _SOC_ERROR)
         {
#ifdef _DEBUG_SYSSOC
            printf("SYS-SOC: Enable linger failed\n");   
#endif
//            dsysFctErrnoSet(ISA_ER_SOC_OPTION);
            statError = BAD_RET;
         }
         break;

      case ISA_SOC_OPT_DONTLINGER:
         /* Disable linger option */
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Disable linger option\n");   
#endif

         /* ldParam no meaning */
         bParam = TRUE;
/*
         if (setsockopt(*pSocId, SOL_SOCKET, SO_DONTLINGER,
                        (char *)&bParam, sizeof(bParam)) == _SOC_ERROR)
         {
#ifdef _DEBUG_SYSSOC
            printf("SYS-SOC: Disable linger failed\n");   
#endif
//            dsysFctErrnoSet(ISA_ER_SOC_OPTION);
            statError = BAD_RET;
         }
        */ 
         break;
      case ISA_SOC_OPT_REUSEADDR:
		  /* add by Alan Huang, set reuse address*/
		  
#ifdef _DEBUG_SYSSOC
		  printf("SYS-SOC: set reuse address\n");   
#endif
		  /* ldParam */
		  iParam = (int)ldParam;
		  if (setsockopt(*pSocId, SOL_SOCKET, SO_REUSEADDR,
			  (char*)&iParam, sizeof(iParam)) == _SOC_ERROR)
		  {
#ifdef _DEBUG_SYSSOC
			  printf("SYS-SOC: Disable linger failed\n");   
#endif
			  //            dsysFctErrnoSet(ISA_ER_SOC_OPTION);
			  statError = BAD_RET;
		  }
		  break;
      default:
#ifdef _DEBUG_SYSSOC
         printf("SYS-SOC: Not implemented option\n");   
#endif
//         dsysFctErrnoSet(ISA_ER_SOC_NOTIMPLEMENTED);
         statError = BAD_RET;
         break;
   }

   return(statError);
}

/* eof ********************************************************************/
