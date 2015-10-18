
#include <sys/timeb.h>

#include "MXTypes.h"

extern VOID RtcInit();
extern time_t	GetRtcTime();
extern void ReadRtcTime(DWORD *RTCTimeBuf);
extern time_t	GetSysTime();
extern VOID SetRtcTime(time_t* pInCurTime);
extern VOID RtcExit();
extern	VOID	TestRtc();
extern	VOID	GetStrRtc(CHAR* pOut);
extern	VOID	SetRtcProc();


