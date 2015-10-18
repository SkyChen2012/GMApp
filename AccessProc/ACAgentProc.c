
/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	ACAgentProc.c
**
**	AUTHOR:		Wayde Zeng
**
**	DATE:		28 - February - 2011
**
**	FILE DESCRIPTION:
**
**
**	FUNCTIONS:
**
**	NOTES:
** 
*/
/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

/************** USER INCLUDE FILES ***************************************************/
#include "MXMem.h"
#include "MXMsg.h"
#include "MXMdId.h"
#include "MXTypes.h"
#include "Dispatch.h"
#include "MXCommon.h"

#include "ACAgentProc.h"
#include "AccessLogReport.h"
#include "MenuParaProc.h"
#include "PwdUlk.h"
#include "ModuleTalk.h"
#include "IOControl.h"
#include "TalkLogReport.h"
#include "hash.h"

/************** DEFINES **************************************************************/
#define GATE_PULSE_WIDTH	200
#define ACA_DEBUG

/************** TYPEDEFS *************************************************************/

extern HANDLE*   g_HandleHashCard ;//卡片存储的句柄

/************** EXTERNAL DECLARATIONS ************************************************/

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static UNLOCKCTRL g_UnlockCtrl;

static VOID AsClearResource(MXMSG *pmsg);

static VOID ACASwipeCard(MXMSG* pmsg);
static VOID ACARdPasswordUnlock(MXMSG* pmsg);
static VOID ACARdPasswordCheck(MXMSG* pmsg);
static VOID ACARdPasswordModify(MXMSG* pmsg);
static VOID ACAOpenGate(MXMSG* pmsg);
static VOID ACACsnPwdMod(MXMSG* pmsg);

static VOID SendSwipeCardAck2ACC(DWORD dwIP, BYTE byResult);
static VOID SendRdPasswordAck2ACC(DWORD dwIP, BYTE byResult);
static VOID SendRdPasswordCheckAck2ACC(DWORD dwIP, BYTE byResult);
static VOID SendRdPasswordModifyAck2ACC(DWORD dwIP, BYTE byResult);
static VOID SendOpenGateAck2ACC(DWORD dwIP, BYTE byResult);
static VOID SendCsnPwdModAck2ACC(DWORD dwIP, BYTE byResult);

static VOID UnlockTimeoutCtrl(VOID);

/*************************************************************************************/

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACAgentInit
**	AUTHOR:		   Wayde Zeng
**	DATE:		28 - February - 2011
**
**	DESCRIPTION:
**				
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**
**	NOTES:
**	
*/
VOID
ACAgentInit(VOID)
{
	DpcAddMd(MXMDID_ACA, NULL);

	g_UnlockCtrl.dwUnlockState = ST_ORIGINAL;
	g_UnlockCtrl.dwTickCount = GetTickCount();

#ifdef ACA_DEBUG
	printf("Access Agent Init...\n");
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACAgentProc
**	AUTHOR:		   Wayde Zeng
**	DATE:		28 - February - 2011
**
**	DESCRIPTION:
**				
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**
**	NOTES:
**	
*/
VOID 
ACAgentProc(VOID)
{
	MXMSG	msgRecev;
	BOOL	bRet = FALSE;
	int		nDataLen = 0;
	
	memset(&msgRecev, 0, sizeof(msgRecev));
	msgRecev.dwDestMd	= MXMDID_ACA;
	msgRecev.pParam		= NULL;	

	if (!g_ASPara.bEnableAS)
	{
		return;
	}

	UnlockTimeoutCtrl();
	
	if (MxGetMsg(&msgRecev))
	{
		switch(msgRecev.dwMsg)
		{
#ifdef ACA_DEBUG
			printf("%s get message = %X...\n", __FUNCTION__,msgRecev.dwMsg);
#endif	
			case FC_AC_SWIPE_CARD:
			{
				ACASwipeCard(&msgRecev);			
				break;
			}
			case FC_AC_RSD_PWD:
			{
				ACARdPasswordUnlock(&msgRecev);
				break;
			}
			case FC_AC_PWD_CHECK:
			{
				ACARdPasswordCheck(&msgRecev);				
				break;
			}
			case FC_AC_PWD_MODIFY:
			{
				ACARdPasswordModify(&msgRecev);				
				break;
			}
			case FC_AC_OPEN_GATE:
			{
				ACAOpenGate(&msgRecev);
				break;
			}
            case FC_AC_CARD_PASSWORD_MOD : 
            {
                ACACsnPwdMod(&msgRecev);
                break;
            }
			default:
			{
				break;
			}
		}
		
		AsClearResource(&msgRecev);	
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACAgentExit
**	AUTHOR:		   Wayde Zeng
**	DATE:		28 - February - 2011
**
**	DESCRIPTION:
**				
**
**	ARGUMENTS:		
**	
**	RETURNED VALUE:	
**
**	NOTES:
**	
*/
VOID
ACAgentExit(VOID)
{
	DpcRmMd(MXMDID_ACA);
#ifdef ACA_DEBUG
	printf("#######: Access Agent module exit with 0.\n");
#endif
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	AsClearResource()
**	AUTHOR:		   Wayde Zeng
**	DATE:		28 - February - 2011
**
**	DESCRIPTION:	
**			release the resource of MXMSG  .
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID AsClearResource(MXMSG *pmsg)
{
	if (NULL != pmsg->pParam)
	{
#ifdef ACA_DEBUG
		printf("## Free ## :MSG.pParam = %s\n", pmsg->pParam);
#endif		
		free(pmsg->pParam);
		pmsg->pParam = NULL;
	}
}

static int GetPatrolResult(int result)
{
#ifdef ACA_DEBUG	
	printf("%s get result:%d\n",__FUNCTION__,result);
#endif
	switch(result)
	{
		case SWIPECARD_UNLOCK:
			return SWIPECARD_CARD_PATROL_UNLOCK;
		case SWIPECARD_EXPIRED:
			return SWIPECARD_EXPIRED;
		case SWIPECARD_NOT_IN_TIME_SLICE:
			return SWIPECARD_NOT_IN_TIME_SLICE;
		case SWIPECARD_INVALID:
			return SWIPECARD_INVALID;
		case SWIPECARD_CARD_DISABLED:
			return SWIPECARD_CARD_DISABLED;
		case SWIPECARD_ENTER_PASSWORD:
			return SWIPECARD_CARD_PATROL_UNLOCK;//SWIPECARD_CARD_PATROL_ENTER_PASSWORD;patrol doesn't need PWD.
		case SWIPECARD_CARD_PATROL_UNLOCK:
			return SWIPECARD_CARD_PATROL_UNLOCK;
		default:
			printf("%s error result:%d\n",__FUNCTION__,result);
			break;
	}
	return SWIPECARD_INVALID;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACASwipeCard()
**	AUTHOR:		   Wayde Zeng
**	DATE:		1 - March - 2011
**
**	DESCRIPTION:	
**			release the resource of MXMSG  .
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
ACASwipeCard(MXMSG* pmsg)
{
	CDINFO*	CdInfo			= NULL;
	CDINFO_OLD *CdInfo_Old	= NULL;
	CDINFO  pCDInfo			;
	BYTE	CSN[CSN_LEN]	= { 0 };
	BYTE	tempCSN[CSN_LEN]	= { 0 };
	BYTE	CdSts			= 0;
	BYTE	byResult		= 0;
	BYTE	byVerify		= 0;
	int		nPwdLen			= 0;
	char	CdPwd[PWD_LEN];
	BYTE bInOut=1;
	BYTE byCardMode;
	UINT 	CardID = 0;
	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		DEBUG_CARD_PRINTF(" %s,%d: ERROR, the pParam can not be NULL.\n", __FUNCTION__,__LINE__);
		return;
	}

	memcpy(CSN, pmsg->pParam + sizeof(int) + 1/*Card Mode*/ + 1/*CSN Len*/, CSN_LEN);
	nPwdLen =  *(pmsg->pParam + sizeof(int) + 10);
	memcpy(CdPwd, pmsg->pParam + sizeof(int) + 11, PWD_LEN);
	byVerify = *(pmsg->pParam + sizeof(int) + 25);
	byCardMode= *(pmsg->pParam + sizeof(int));


	
	if (MODE_CARD_ONLY_SET & g_ASPara.ASOpenMode )
	{
		
	}
	else if(MODE_CARD_PASSWORD_SET & g_ASPara.ASOpenMode)
	{
		if(byVerify == 0)
		{
			byVerify=1;		//set status to need password
		}
	}
	else
	{
		SendSwipeCardAck2ACC(pmsg->dwParam,SWIPECARD_FUN_DISABLE );
		return;
	}

/**/if(isAHASHFILEExisted())
	{
		DEBUG_CARD_PRINTF("func=%s,LINE=%d\n" ,__func__,__LINE__);
		
		if( TRUE == ReadCard(g_HandleHashCard,CSN,TYPE_NORMAL_CARD,&pCDInfo) )
		{
			CdInfo = &pCDInfo;
			DEBUG_CARD_PRINTF("%s,%d,TYPE_NORMAL_CARD Card existed\r\n",__func__,__LINE__);	
		}
		else
		{  	
			if(TRUE == ReadCard(g_HandleHashCard,CSN,TYPE_PATROL_CARD,&pCDInfo))
			{
				CdInfo = &pCDInfo;
				DEBUG_CARD_PRINTF("%s,%d,TYPE_PATROL_CARD Card existed\r\n",__func__,__LINE__);	
			}
		}
	}
	if (CdInfo == NULL)//如果HASH 找到了卡片就不用继续寻找卡片
	{
		DEBUG_CARD_PRINTF("%s,%d,card no found in hashfunc\r\n",__func__,__LINE__);
		if(isAST_HVFileExisted())
	    {
	        if(NULL != (CdInfo_Old= (CDINFO_OLD*)AsGetCdHVInfobyCd(CSN)))
	        {
	        	ConversionCdInfoOld2CdInfo(&pCDInfo,CdInfo_Old);
				CdInfo = &pCDInfo;
	            printf("card found HV\n");
	        }
		}
	    else
	    {
	        if(NULL != (CdInfo_Old = AsGetCdInfobyCd(CSN)))
	        {
	        	ConversionCdInfoOld2CdInfo(&pCDInfo,CdInfo_Old);
				CdInfo = &pCDInfo;
	            printf("card found\n");
	        }
	    }
	}
	AsPrintCd(CdInfo);

	if (CdInfo) 
	{
		memcpy(CSN, CdInfo->CSN, CSN_LEN);
		CdSts = JudgeCardInfo(CdInfo);
		
		CardID = CdInfo->Reserved[0];
#ifdef ACA_DEBUG		
		printf("%s,CdSts=%d,CdInfo->GateNumber=%d,verify=%d,card type=%d\n",__FUNCTION__,CdSts,CdInfo->GateNumber,byVerify,CdInfo->CardType);
#endif
		if (CARD_INFO_DISABLED == CdSts)
		{
			byResult = 	SWIPECARD_CARD_DISABLED;
		}
		else if (CARD_INFO_EXPIRED == CdSts)
		{
			byResult = 	SWIPECARD_EXPIRED;
		}
		else if (CARD_INFO_AUTHORIZED == CdSts)
		{
			if (1 == CdInfo->GateNumber || 3 == CdInfo->GateNumber)
			{
#ifdef ACA_DEBUG
				printf("%s swipe card verify = %d,GetCardType()=%d\n", __FUNCTION__, byVerify,CdInfo->CardType);
#endif
				if(CdInfo->CardType == TYPE_AUTHORIZE_CARD)
				{
					byResult = SWIPECARD_CARD_AUTHORIZE;
				}
				else if (CdInfo->CardType == TYPE_PATROL_CARD || 0 == byVerify)		// verify card then unlock,patrol not need password
				{
					UnlockGateStart(FALSE, 0, 0, NULL, NULL);

					g_UnlockCtrl.dwUnlockState = ST_UK_DELAY;
					g_UnlockCtrl.dwTickCount = GetTickCount();
					
					byResult = SWIPECARD_UNLOCK;
				}
				else if (1 == byVerify)	// verify card
				{
					byResult = SWIPECARD_ENTER_PASSWORD;
				}
				else if (2 == byVerify)	// verify card and password then unlock
				{
					if (UkPwdCompare(CdInfo->UlkPwd, CdPwd, nPwdLen))
					{
						UnlockGateStart(FALSE, 0, 0, NULL, NULL);

						g_UnlockCtrl.dwUnlockState = ST_UK_DELAY;
						g_UnlockCtrl.dwTickCount = GetTickCount();
						
						byResult = SWIPECARD_UNLOCK;
					}
					else
					{
						byResult = SWIPECARD_PASSWORD_ERROR;
					}
				}				
			}
			else
			{
				byResult = 	SWIPECARD_INVALID;
			}
		}				
	}
	else
	{	
		byResult = 	SWIPECARD_INVALID;
	}

	if((byResult!=SWIPECARD_ENTER_PASSWORD) && (byResult <= SWIPECARD_PASSWORD_ERROR/*SWIPECARD_PASSWORD_USER*/))
	{
		RecordSwipeCardLog(byCardMode, CSN, READER_PORT_DEFAULT, READER_ID_DEFAULT, GATE_ID_DEFAULT, 
							INOUT_DEFAULT, byResult, CardID);
		if(((byResult == SWIPECARD_CARD_PATROL_UNLOCK) || (byResult == SWIPECARD_UNLOCK)) &&
			 CdInfo != NULL &&
			 CdInfo->CardType == TYPE_PATROL_CARD)
		{
			RecordSwipeCardLog(byCardMode, CSN, READER_PORT_DEFAULT, READER_ID_DEFAULT, GATE_ID_DEFAULT, 
							INOUT_DEFAULT, SWIPECARD_CARD_PATROL_UNLOCK, CardID);
		}
	}


	if((CdInfo != NULL) && 
		(CdInfo->CardType == TYPE_PATROL_CARD))
	{
		SendSwipeCardAck2ACC(pmsg->dwParam, GetPatrolResult(byResult));
	}
	else
	{
		SendSwipeCardAck2ACC(pmsg->dwParam,byResult );
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendSwipeCardAck2ACC()
**	AUTHOR:		   Wayde Zeng
**	DATE:		1 - March - 2011
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
SendSwipeCardAck2ACC(DWORD dwIP, BYTE byResult)
{
	MXMSG		msgSend;
	unsigned char * pData;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
#ifdef ACA_DEBUG
	printf("TX:FC_ACK_AC_SWIPE_CARD dwIP=0x%08x\n",dwIP);	
#endif
	msgSend.dwSrcMd		= MXMDID_ACA;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_AC_SWIPE_CARD;
	msgSend.dwParam		= dwIP;
	msgSend.pParam		= (unsigned char *) malloc(1);
	*msgSend.pParam		= byResult;
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACACsnPwdMod()
**	AUTHOR:		    Michael Ma
**	DATE:		    15 - Oct - 2012
**
**	DESCRIPTION:	
**			        using functionality '*3#' to modify the corresponding PWD of specified card. 
**                  waiting for the net package from the gate sontroling client and then process
**                  this information,at last sending result to the gate sontroling client.
**	ARGUMENTS:	    ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				    pmsg                    MXMSG    
**	RETURNED VALUE:	
**				    None
**	NOTES:
**			        
*/
static VOID ACACsnPwdMod(MXMSG* pmsg)
{
    CDINFO*	CdInfo			= NULL;
	CDINFO  pCDInfo;
	CDINFO_OLD * CdInfo_Old = NULL;
	BYTE	CSN[CSN_LEN]	= { 0 };
	BYTE	byResult		= 0;
	BYTE	byVerify		= 0;
	INT		nPwdLen			= 0;
	char	CdPwd[PWD_LEN]  = { 0 };
    //BYTE    byCardMode      = 0;
    //BYTE    byCSNLength     = 0;
    size_t  nTypeSize       = sizeof(unsigned short);

	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
    
	//byCardMode  = *(pmsg->pParam + nTypeSize + 0);
    //byCSNLength = *(pmsg->pParam + nTypeSize + 1);
	memcpy(CSN,     pmsg->pParam + nTypeSize + 1/*Card Mode*/ + 1/*CSN Len*/, CSN_LEN);
	nPwdLen     = *(pmsg->pParam + nTypeSize + 2 + CSN_LEN);
	memcpy(CdPwd,   pmsg->pParam + nTypeSize + 3 + CSN_LEN, PWD_LEN);
	byVerify    = *(pmsg->pParam + nTypeSize + 3 + CSN_LEN + PWD_LEN);
	
	if( TRUE == ReadCard(g_HandleHashCard,CSN,TYPE_NORMAL_CARD,&pCDInfo) )
		{
			CdInfo = &pCDInfo;
			DEBUG_CARD_PRINTF("%s,%d,TYPE_NORMAL_CARD Card existed\r\n",__func__,__LINE__);	
		}
	else if(TRUE == ReadCard(g_HandleHashCard,CSN,TYPE_PATROL_CARD,&pCDInfo))
		{  	
			CdInfo = &pCDInfo;
			DEBUG_CARD_PRINTF("%s,%d,TYPE_PATROL_CARD Card existed\r\n",__func__,__LINE__);	
		}
	else
		{
			CdInfo_Old = AsGetCdInfobyCd(CSN);
			if(ConversionCdInfoOld2CdInfo(&pCDInfo,CdInfo_Old))
			{
				CdInfo = &pCDInfo;
	        	printf("card found\n");
			}			
		}
    printf("CdInfo exit : %d\n",CdInfo != NULL ? 1 : 0); 
    
    if(NULL != CdInfo)
    {
        if(SWIPECARD_REQUEST_VERIFY_CSN == byVerify)
        {
            switch(JudgeCardInfo(CdInfo))
            {
                case CARD_INFO_AUTHORIZED       : 
                    {
                        if(TYPE_NORMAL_CARD == CdInfo->CardType)
                        {
			                byResult = CSN_PWD_MOD_CARD_EXIST;       //有此卡号
                        }
                        else if(TYPE_PATROL_CARD == CdInfo->CardType)
                        {
                            byResult = CSN_PWD_MOD_PATROL;        //巡更卡      
                        } 
                        else if(TYPE_AUTHORIZE_CARD == CdInfo->CardType)    //有授权功能的卡
                        {   
                            byResult = CSN_PWD_MOD_AUTHORIZING;        
                        }
                    }
                    break;
                case CARD_INFO_DISABLED         :
                    {
                        byResult = 	CSN_PWD_MOD_DISABLED;
                    }
                    break;
                case CARD_INFO_EXPIRED          :
                    {
                        byResult = 	CSN_PWD_MOD_EXPIRED;
                    }
                    break;
                /*case CARD_INFO_OUTOFTIMESLICE   :
                    {
                        byResult = 	CSN_PWD_MOD_NOT_IN_TIME_SLICE;
                    }
                    break;*/
                default                         :
                    {
                        
                    }
            }
        }
        else if(SWIPECARD_REQUEST_VERIFY_PWD == byVerify)
        {
            if (UkPwdCompare(CdInfo->UlkPwd, CdPwd, nPwdLen))
			{
                byResult = CSN_PWD_MOD_PWD_CORRECT;
		    }
			else
			{
			    byResult = CSN_PWD_MOD_PWD_ERROR;
			}
        }
        else if(SWIPECARD_REQUEST_SAVE_NEW_PWD == byVerify)
        {
            memset(CdInfo->UlkPwd,0,PWD_LEN);
            memcpy(CdInfo->UlkPwd, CdPwd, nPwdLen);		
		    SaveCdInfo2Mem();
            AsModCdPwdbyCsn(CdInfo->CSN,CSN_LEN,CdPwd, nPwdLen);
            return ;
        }
        else
        {
            //容错
        }
    }
    else
    {
        byResult = CSN_PWD_MOD_INVALID; //此卡不存在，即卡未授权
    }
    printf("byResult : %d\n",byResult);
    SendCsnPwdModAck2ACC(pmsg->dwParam,byResult);
}

static VOID SendCsnPwdModAck2ACC(DWORD dwIP, BYTE byResult)
{
	MXMSG		msgSend;
	memset(&msgSend, 0, sizeof(msgSend));	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	msgSend.dwSrcMd		= MXMDID_ACA;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_AC_CARD_PASSWORD_MOD;
	msgSend.dwParam		= dwIP;
	msgSend.pParam		= (unsigned char *) malloc(1);
	*msgSend.pParam		= byResult;
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACARdPasswordUnlock()
**	AUTHOR:		   Wayde Zeng
**	DATE:		3 - March - 2011
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
ACARdPasswordUnlock(MXMSG* pmsg)
{
	BYTE byResult = 0;
	char RdCode[RD_CODE_LEN];
	char RdPwd[PWD_LEN];
	int	 nPwdLen = 0;
	RDINFO *pRdInfo = NULL;

	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}

	memcpy(RdCode, pmsg->pParam + sizeof(int), RD_CODE_LEN);
	memcpy(RdPwd, pmsg->pParam + sizeof(int) + RD_CODE_LEN + 1, PWD_LEN);
	nPwdLen = *(pmsg->pParam + sizeof(int) + RD_CODE_LEN);
	if( strlen(RdCode) == 0)
	{
		//只判断密码
		if(PUPasswordCompare(RdPwd,nPwdLen))
		{
			byResult = 0;
			UnlockGateStart(FALSE, 0, 0, NULL, NULL);

			g_UnlockCtrl.dwUnlockState = ST_UK_DELAY;
			g_UnlockCtrl.dwTickCount = GetTickCount();
		}
		else
		{
			byResult = 2;
		}
	}
	else
	{

		pRdInfo = PUGetRdInfobyRd(RdCode, strlen(RdCode));
		if (pRdInfo)
		{
			if (CodeCompare(RdPwd, pRdInfo->RdPwd, nPwdLen))
			{
				UnlockGateStart(FALSE, 0, 0, NULL, NULL);

				g_UnlockCtrl.dwUnlockState = ST_UK_DELAY;
				g_UnlockCtrl.dwTickCount = GetTickCount();

				byResult = 0;
			}		
			else
			{
				byResult = 2;
			}
		}
		else
		{
			byResult = 1;
		}	
	}

	SendRdPasswordAck2ACC(pmsg->dwParam, byResult);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendRdPasswordAck2ACC()
**	AUTHOR:		   Wayde Zeng
**	DATE:		3 - March - 2011
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
SendRdPasswordAck2ACC(DWORD dwIP, BYTE byResult)
{
	MXMSG		msgSend;
	unsigned char * pData;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACA;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_AC_RSD_PWD;
	msgSend.dwParam		= dwIP;
	msgSend.pParam		= (unsigned char *) malloc(1);
	*msgSend.pParam		= byResult;
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACARdPasswordCheck()
**	AUTHOR:		   Wayde Zeng
**	DATE:		3 - March - 2011
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
ACARdPasswordCheck(MXMSG* pmsg)
{
	BYTE byResult = 0;
	char RdCode[RD_CODE_LEN];
	char RdPwd[PWD_LEN];
	int	 nPwdLen = 0;
	RDINFO *pRdInfo = NULL;

	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}

	memcpy(RdCode, pmsg->pParam + sizeof(int), RD_CODE_LEN);
	memcpy(RdPwd, pmsg->pParam + sizeof(int) + RD_CODE_LEN + 1, PWD_LEN);
	nPwdLen = *(pmsg->pParam + sizeof(int) + RD_CODE_LEN);

	pRdInfo = PUGetRdInfobyRd(RdCode, strlen(RdCode));
	if (pRdInfo)
	{
		if (CodeCompare(RdPwd, pRdInfo->RdPwd, nPwdLen))
		{
			byResult = 0;
		}		
		else
		{
			byResult = 2;
		}
	}
	else
	{
		byResult = 1;
	}	

	SendRdPasswordCheckAck2ACC(pmsg->dwParam, byResult);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendRdPasswordCheckAck2ACC()
**	AUTHOR:		   Wayde Zeng
**	DATE:		3 - March - 2011
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
SendRdPasswordCheckAck2ACC(DWORD dwIP, BYTE byResult)
{
	MXMSG		msgSend;
	unsigned char * pData;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACA;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_AC_PWD_CHECK;
	msgSend.dwParam		= dwIP;
	msgSend.pParam		= (unsigned char *) malloc(1);
	*msgSend.pParam		= byResult;
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACARdPasswordModify()
**	AUTHOR:		   Wayde Zeng
**	DATE:		3 - March - 2011
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
ACARdPasswordModify(MXMSG* pmsg)
{
	BYTE byResult = 0;
	char RdCode[RD_CODE_LEN];
	char RdPwd[PWD_LEN];
	int	 nPwdLen = 0;
	RDINFO *pRdInfo = NULL;
	EthResolvInfo 	ResolvInfo;
	int i;
	
	if (pmsg == NULL || pmsg->pParam == NULL)
	{
		printf(" %s: ERROR, the pParam can not be NULL.\n", __FUNCTION__);
		return;
	}
	ResolvInfo.nType=ATM_TYPE_NODEVICE;
	ResolvInfo.nQueryMethod = HO_QUERY_METHOD_IP;
	ResolvInfo.nIP = ChangeIPFormat(pmsg->dwParam);

	printf("pmsg->wDataLen=%d\n",pmsg->wDataLen);
	for(i=0;i<RD_CODE_LEN+PWD_LEN+1;i++)
	{
		printf("Data[%d]=0x%02x ",i,*(pmsg->pParam + sizeof(int) + i));
	}
	printf("\n");
	
	FdFromAMTResolv(&ResolvInfo);

	 if (ATM_TYPE_EHV == ResolvInfo.nType )
	 {
	 	memcpy(RdCode, ResolvInfo.szDevCode, RD_CODE_LEN);
	 }
	 else
	 {
	 	memcpy(RdCode, pmsg->pParam + sizeof(int), RD_CODE_LEN);
	 }

	memcpy(RdPwd, pmsg->pParam + sizeof(int) + RD_CODE_LEN + 1, PWD_LEN);
	nPwdLen = *(pmsg->pParam + sizeof(int) + RD_CODE_LEN);

	pRdInfo = PUGetRdInfobyRd(RdCode, strlen(RdCode));
	if (pRdInfo)
	{
		memset(pRdInfo->RdPwd, 0, PWD_LEN);
		memcpy(pRdInfo->RdPwd, RdPwd, nPwdLen);
		pRdInfo->bPwdCfg = 1;
		
		SaveRdInfo2Mem();

		byResult = 0;
	}
	else
	{
		byResult = 1;
	}	
	SendRdPasswordModifyAck2ACC(pmsg->dwParam, byResult);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendRdPasswordModifyAck2ACC()
**	AUTHOR:		   Wayde Zeng
**	DATE:		3 - March - 2011
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
SendRdPasswordModifyAck2ACC(DWORD dwIP, BYTE byResult)
{
	MXMSG		msgSend;
	unsigned char * pData;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACA;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_AC_PWD_MODIFY;
	msgSend.dwParam		= dwIP;
	msgSend.pParam		= (unsigned char *) malloc(1);
	*msgSend.pParam		= byResult;
	
	MxPutMsg(&msgSend);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	UnlockTimeoutCtrl()
**	AUTHOR:		   Wayde Zeng
**	DATE:		6 - March - 2011
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/


static
VOID
UnlockTimeoutCtrl(VOID)
{
	if (SERIES_SHELL == g_DevFun.SeriesType || SERIES_MINI_SHELL == g_DevFun.SeriesType) 
	{
		if ((g_UnlockCtrl.dwUnlockState == ST_UK_DELAY) && 
			(GetTickCount() - g_UnlockCtrl.dwTickCount > (g_SysConfig.UnlockTime*1000))&&
			(!IsFireAlarmOn()))
		{
#ifdef ACA_DEBUG			
			printf("%s,SERIES_SHELL call UnlockGateEnd\n",__FUNCTION__);
#endif
            printf("SHELL serial close the door\n");
			UnlockGateEnd();
            g_DoorUlkBehav.DoorUlkBehaviour = DOOR_ACTIVE_OPEN_END;           
			g_UnlockCtrl.dwUnlockState = ST_ORIGINAL;
		}
	}
	else
	{
		if (GetTickCount() - g_UnlockCtrl.dwTickCount > GATE_PULSE_WIDTH)
		{
			UnlockGateEnd();

			g_UnlockCtrl.dwUnlockState = ST_ORIGINAL;
			g_UnlockCtrl.dwTickCount = GetTickCount();
		}
        if(g_DoorUlkBehav.DoorUlkBehaviour && (GetTickCount() - g_DoorUlkBehav.ForceUlkDetectTimer > CORAL_AMBER_GM_ULK_DURATION))
        {
            printf("CORAL serial close the door\n");
            g_DoorUlkBehav.DoorUlkBehaviour = DOOR_ACTIVE_OPEN_END;
        }
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ACAOpenGate()
**	AUTHOR:		   Wayde Zeng
**	DATE:		6 - March - 2011
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
ACAOpenGate(MXMSG* pmsg)
{
        
	BYTE byResult = 0;
    BYTE byUnlockType=*(pmsg->pParam +sizeof(unsigned short)+sizeof(unsigned short)+IRIS_CODE_LEN+1+1);
      
   // printf("byUnlockType %d\r\n", *(pmsg->pParam +sizeof(unsigned short)+sizeof(unsigned short)+IRIS_CODE_LEN+1+1));
	UnlockGateStart(FALSE, byUnlockType, 0, NULL, NULL);
	
	g_UnlockCtrl.dwUnlockState = ST_UK_DELAY;
	g_UnlockCtrl.dwTickCount = GetTickCount();

	SendOpenGateAck2ACC(pmsg->dwParam, byResult);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	SendOpenGateAck2ACC()
**	AUTHOR:		   Wayde Zeng
**	DATE:		6 - March - 2011
**
**	DESCRIPTION:	
**			
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static
VOID
SendOpenGateAck2ACC(DWORD dwIP, BYTE byResult)
{
	MXMSG		msgSend;
	unsigned char * pData;
	unsigned short nDataLen = 0;
	
	memset(&msgSend, 0, sizeof(msgSend));
	
	strcpy(msgSend.szSrcDev, "");
	strcpy(msgSend.szDestDev, "");
	
	msgSend.dwSrcMd		= MXMDID_ACA;
	msgSend.dwDestMd	= MXMDID_ETH;
	msgSend.dwMsg		= FC_ACK_AC_OPEN_GATE;
	msgSend.dwParam		= dwIP;
	msgSend.pParam		= (unsigned char *) malloc(1);
	*msgSend.pParam		= byResult;
	
	MxPutMsg(&msgSend);
}
