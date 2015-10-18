/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	Eth.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		19 - Nov - 2008
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

/************** SYSTEM INCLUDE FILES **************************************************/

#include <string.h>
#include <stdio.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXMdId.h"
#include "MXMsg.h"
#include "MXTypes.h"
#include "MXList.h"
#include "MXMem.h"
#include "Dispatch.h"

#include "DiagTelnetd.h"
#include "telnetd.h"

/************** DEFINES **************************************************************/

#define STRTABLELEN   109
#define STRCMDLEN		50

/************** TYPEDEFS *************************************************************/

typedef struct _Command2String
{
	USHORT	 nCommand;
	CHAR		strCommand[STRCMDLEN];
}Command2String;

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static Command2String StringTable[STRTABLELEN]=
{
	{FC_MNT_START				, "FC_MNT_START"			},
	{FC_MNT_CANCEL				, "FC_MNT_CANCEL"			},
	{FC_MNT_INTERRUPT			, "FC_MNT_INTERRUPT"		},
	{FC_CALL_HV_MC				, "FC_CALL_HV_MC"			},
	{FC_CALL_HV_HV				, "FC_CALL_HV_HV"			},
	{FC_CALL_HV_GM				, "FC_CALL_HV_GM"			},
	{FC_CALL_GM_HV				, "FC_CALL_GM_HV"			},
	{FC_CALL_GM_MC				, "FC_CALL_GM_MC"			},
	{FC_CALL_GM_GM				, "FC_CALL_GM_GM"			},
	{FC_CALL_MC_HV				, "FC_CALL_MC_HV"			},
	{FC_CALL_MC_GM				, "FC_CALL_MC_GM"			},
	{FC_PICKUP_HV				, "FC_PICKUP_HV"			},
	{FC_PICKUP_GM				, "FC_PICKUP_GM"			},
	{FC_PICKUP_MC				, "FC_PICKUP_MC"			},
	{FC_HANGUP_HV				, "FC_HANGUP_HV"			},
	{FC_HANGUP_GM				, "FC_HANGUP_GM"			},
	{FC_HANGUP_MC				, "FC_HANGUP_MC"			},
	{FC_UNLK_GATE				, "FC_UNLK_GATE"			},
	{FC_CALL_HV_GM				, "FC_CALL_HV_GM"			},
	{FC_CALL_BROADCAST			, "FC_CALL_BROADCAST"		},
	{FC_CALL_STP_BROADCAST		, "FC_CALL_STP_BROADCAST"	},
	{FC_PICKUP_NTY				, "FC_PICKUP_NTY"			},
	{FC_HANGUP_NTY				, "FC_HANGUP_NTY"			},
	{FC_QUERY_CHILDREN			, "FC_QUERY_CHILDREN"		},
	{FC_REDIRECT_MC				, "FC_REDIRECT_MC"			},
	{FC_CALL_FORWARD			, "FC_CALL_FORWARD"			},
	{FC_ALM_REPORT				, "FC_ALM_REPORT"			},
	{FC_SAL_QUERY_LIST			, "FC_SAL_QUERY_LIST"		},
	{FC_SAL_QUERY_CNT			, "FC_SAL_QUERY_CNT"		},
	{FC_SAL_QUERY_LOG			, "FC_SAL_QUERY_LOG"		},
	{FC_SAL_QUERY_DEL			, "FC_SAL_QUERY_DEL"		},
	{FC_SAL_STATUS_NOTIFY		, "FC_SAL_STATUS_NOTIFY"	},
	{FC_TL_QUERY_LIST			, "FC_TL_QUERY_LIST	"		},
	{FC_TL_QUERY_CNT			, "FC_TL_QUERY_CNT"			},
	{FC_TL_QUERY_LOG			, "FC_TL_QUERY_LOG"			},
	{FC_TL_QUERY_DEL			, "FC_TL_QUERY_DEL"			},
	{FC_TL_STATUS_NOTIFY		, "FC_TL_STATUS_NOTIFY"		},
	{FC_TL_QUERY_PIC			, "FC_TL_QUERY_PIC"			},
	{FC_TL_SEND_PIC				, "FC_TL_SEND_PIC"			},
	{FC_SA_SET_NOTIFY			, "FC_SA_SET_NOTIFY"		},
	{FC_SA_SET_REPORT			, "FC_SA_SET_REPORT"		},
	{FC_SA_ZONE_QUERY			, "FC_SA_ZONE_QUERY"		},
	{FC_SA_ZONE_SET				, "FC_SA_ZONE_SET"			},
	{FC_SA_ALARM_NOTIFY			, "FC_SA_ALARM_NOTIFY"		},
	{FC_SA_ALARM_CONFIRM_NOTIFY	, "FC_SA_ALARM_CONFIRM_NOTIFY"},
	{FC_DTM_ADJUST				, "FC_DTM_ADJUST"			},
	{FC_LINE_LOCK				, "FC_LINE_LOCK"			},
	{FC_LINE_FREE				, "FC_LINE_FREE"			},
	{FC_HO_QUERY				, "FC_HO_QUERY"				},
	{FC_STATE_DIAGNOSE			, "FC_STATE_DIAGNOSE"		},
	{FC_DTM_GET					, "FC_DTM_GET"				},
	{FC_HO_GET_COUNT			, "FC_HO_GET_COUNT"			},
	{FC_HO_GET					, "FC_HO_GET"				},
	{FC_PSW_CHANGE				, "FC_PSW_CHANGE"			},
	{FC_HO_GETCHILDREN_COUNT	, "FC_HO_GETCHILDREN_COUNT"	},
	{FC_HO_GETCHILDREN			, "FC_HO_GETCHILDREN"		},
	{FC_GET_SYSINFO				, "FC_GET_SYSINFO"			},
	{FC_GET_FUNCINFO			, "FC_GET_FUNCINFO"			},
	{FC_AV_SENDDATA				, "FC_AV_SENDDATA"			},
	{FC_AV_REQUEST_VIDEO_IVOP	, "FC_AV_REQUEST_VIDEO_IVOP	"},
	{FC_AV_TAKEPHOTO			, "FC_AV_TAKEPHOTO"			},
	{FC_CNF_DWNLD_PRG			, "FC_CNF_DWNLD_PRG	"		},
	{FC_CNF_CHK_PRG				, "FC_CNF_CHK_PRG"			},
	{FC_CNF_FALSH_BURN_PER		, "FC_CNF_FALSH_BURN_PER"	},
	{FC_REPORT_LOGDATA			, "FC_REPORT_LOGDATA"		},
	{FC_REPORT_PIC				, "FC_REPORT_PIC"			},
	{FC_ACK_IMSG_SEND			, "FC_ACK_IMSG_SEND"		},
	{FC_ACK_MNT_START			, "FC_ACK_MNT_START"		},
	{FC_ACK_MNT_CANCEL			, "FC_ACK_MNT_CANCEL"		},
	{FC_ACK_MNT_INTERRUPT		, "FC_ACK_MNT_INTERRUPT"	},
	{FC_ACK_CALL_HV_MC			, "FC_ACK_CALL_HV_MC"		},
	{FC_ACK_CALL_HV_HV			, "FC_ACK_CALL_HV_HV"		},
	{FC_ACK_CALL_HV_GM			, "FC_ACK_CALL_HV_GM"		},
	{FC_ACK_CALL_GM_HV			, "FC_ACK_CALL_GM_HV"		},
	{FC_ACK_CALL_GM_MC			, "FC_ACK_CALL_GM_MC"		},
	{FC_ACK_CALL_GM_GM			, "FC_ACK_CALL_GM_GM"		},
	{FC_ACK_CALL_MC_HV			, "FC_ACK_CALL_MC_HV"		},
	{FC_ACK_CALL_MC_GM			, "FC_ACK_CALL_MC_GM"		},
	{FC_ACK_PICKUP_HV			, "FC_ACK_PICKUP_HV"		},
	{FC_ACK_PICKUP_GM			, "FC_ACK_PICKUP_GM"		},
	{FC_ACK_PICKUP_MC			, "FC_ACK_PICKUP_MC"		},
	{FC_ACK_HANGUP_HV			, "FC_ACK_HANGUP_HV"		},
	{FC_ACK_HANGUP_GM			, "FC_ACK_HANGUP_GM"		},
	{FC_ACK_HANGUP_MC			, "FC_ACK_HANGUP_MC"		},
	{FC_ACK_UNLK_GATE			, "FC_ACK_UNLK_GATE"		},
	{FC_ACK_QUERY_CHILDREN		, "FC_ACK_QUERY_CHILDREN"	},
	{FC_ACK_CALL_FORWARD		, "FC_ACK_CALL_FORWARD"		},
	{FC_ACK_ALM_REPORT			, "FC_ACK_ALM_REPORT"		},
	{FC_ACK_ALM_NEW_REPORT		, "FC_ACK_ALM_NEW_REPORT"	},
	{FC_ACK_TL_QUERY_LIST		, "FC_ACK_TL_QUERY_LIST"	},
	{FC_ACK_TL_QUERY_CNT		, "FC_ACK_TL_QUERY_CNT"		},
	{FC_ACK_TL_QUERY_LOG		, "FC_ACK_TL_QUERY_LOG"		},
	{FC_ACK_TL_QUERY_DEL		, "FC_ACK_TL_QUERY_DEL"		},
	{FC_ACK_TL_QUERY_PIC		, "FC_ACK_TL_QUERY_PIC"		},
	{FC_ACK_TL_SEND_PIC			, "FC_ACK_TL_SEND_PIC"		},
	{FC_ACK_SA_ZONE_QUERY		, "FC_ACK_SA_ZONE_QUERY"	},
	{FC_ACK_SA_ZONE_SET			, "FC_ACK_SA_ZONE_SET"		},
	{FC_ACK_HO_QUERY			, "FC_ACK_HO_QUERY"			},
	{FC_ACK_STATE_DIAGNOSE		, "FC_ACK_STATE_DIAGNOSE"	},
	{FC_ACK_DTM_GET				, "FC_ACK_DTM_GET"			},
	{FC_ACK_HO_GET_COUNT		, "FC_ACK_HO_GET_COUNT"		},
	{FC_ACK_HO_GET				, "FC_ACK_HO_GET"			},
	{FC_ACK_PSW_CHANGE			, "FC_ACK_PSW_CHANGE"		},
	{FC_ACK_HO_GETCHILDRENCOUNT	, "FC_ACK_HO_GETCHILDRENCOUNT"},
	{FC_ACK_HO_GETCHILDREN		, "FC_ACK_HO_GETCHILDREN"	},
	{FC_ACK_GET_SYSINFO			, "FC_ACK_GET_SYSINFO"		},
	{FC_ACK_GET_FUNCINFO		, "FC_ACK_GET_FUNCINFO"		},
	{FC_ACK_AV_TAKEPHOTO		, "FC_ACK_AV_TAKEPHOTO"		},
	{FC_ACK_REPORT_LOGDATA		, "FC_ACK_REPORT_LOGDATA"	}
};

static VOID Convert2String(UCHAR* InData, INT inLen, CHAR *OutData);

BOOL    FlagShowLogData   =  FALSE;

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TelLogInput
**	AUTHOR:			Jeff Wang
**	DATE:			19 - Nov - 2008
**
**	DESCRIPTION:	
**			Input Log Msg to telnet
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				pMsg		[IN]	MXMSG*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/

void 
TelLogInput(MXMSG *pMsg)
{
	INT i = 0;
	CHAR pStr[MAX_LOG_MSG_LEN-1] = { 0 };
	CHAR pStr1[MAX_LOG_MSG_LEN-1] = { 0 };

	if (!FlagShowLogData
		||pMsg->dwMsg >= 0xF000) 
	{
		return;
	}

	Convert2String((UCHAR*)&pMsg->dwSrcMd, 1, pStr1);
	sprintf(pStr, "Src Md: %s", pStr1);
	DiagPutOneLog(pStr);
	
	Convert2String((UCHAR*)&pMsg->dwDestMd, 1, pStr1);
	sprintf(pStr, "Dst Md: %s", pStr1);
	DiagPutOneLog(pStr);

	for(i=0; i< STRTABLELEN; i++)
	{
		if (pMsg->dwMsg == StringTable[i].nCommand)
		{
			sprintf(pStr, "Command: %s", StringTable[i].strCommand);
			DiagPutOneLog(pStr);
			break;
		}
	}
	sprintf(pStr, "Src Code: %s", pMsg->szSrcDev);
	DiagPutOneLog(pStr);

	sprintf(pStr, "Dst Code: %s", pMsg->szDestDev);
	DiagPutOneLog(pStr);
	
	Convert2String((UCHAR*)&pMsg->dwParam, 4, pStr1);
	sprintf(pStr, "DwParam: %s\n", pStr1);
	DiagPutOneLog(pStr);
}

VOID 
TelLogStr(CHAR* pLogData)
{
	CHAR pStr[MAX_LOG_MSG_LEN-1] = { 0 };

	if (!FlagShowLogData)
	{
		return;
	}
	
	strcpy(pStr, pLogData);
	DiagPutOneLog(pStr);
}
/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	Convert2String
**	AUTHOR:			Jeff Wang
**	DATE:			19 - Nov - 2008
**
**	DESCRIPTION:	
**			Convert data to string
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				InData		[IN]		Uint8*
**				InLen		[IN]		int
**				OutData		[Out]		char*
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static void
Convert2String(UCHAR* InData, INT inLen, CHAR *OutData)
{
	INT i = 0;
	CHAR temp = 0;
	INT   nOutCount = 0;

	for(i = 0; i < inLen; i++)
	{		
		temp = (CHAR)(InData[i]>>4) & 0x0F;
		
		if (temp >= 0 && temp <= 9)
		{
			OutData[nOutCount++] = temp + 0x30;
		}
		else if (temp >= 0x0A && temp <= 0x0F)
		{
			OutData[nOutCount++] = temp - 9 + 0x40;
		}

		temp = InData[i] & 0x0F;

		if (temp >= 0 && temp <= 9)
		{
			OutData[nOutCount++] = temp + 0x30;
		}
		else if (temp >= 10 && temp <= 15)
		{
			OutData[nOutCount++] = temp - 9 + 0x40;
		}

		OutData[nOutCount++] = ' ';
	}
	OutData[nOutCount] = '\0';
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TelInputCSN
**	AUTHOR:			Jeff Wang
**	DATE:			19 - Nov - 2008
**
**	DESCRIPTION:	
**			Input card number to telnet
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				CSN		[IN]		Uint8*
**				nCardLen	[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TelInputCSN(UCHAR* CSN, INT nCardLen)
{
	CHAR pStr[MAX_LOG_MSG_LEN-1] = { 0 };
	CHAR pStr1[MAX_LOG_MSG_LEN-1] = { 0 };
	
	if (!FlagShowLogData) 
	{
		return;
	}
	
	Convert2String(CSN, nCardLen, pStr1);
	sprintf(pStr, "Card Number: %s\n", pStr1);
	DiagPutOneLog(pStr);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	TelSrlData
**	AUTHOR:			Wayde Zeng
**	DATE:			21 - Dec - 2009
**
**	DESCRIPTION:	
**		
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				CSN		[IN]		Uint8*
**				nCardLen	[IN]		int
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
void
TelSrlData(UCHAR* pSrlData, INT nDataLen)
{
	CHAR pStr[MAX_LOG_MSG_LEN-1] = { 0 };
	CHAR pStr1[MAX_LOG_MSG_LEN-1] = { 0 };
	
	if (!FlagShowLogData) 
	{
		return;
	}
	
	Convert2String(pSrlData, nDataLen, pStr1);
	sprintf(pStr, "serial data: %s\n", pStr1);
	DiagPutOneLog(pStr);
}































