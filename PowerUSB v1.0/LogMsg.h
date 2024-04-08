//---------------------------------------------------------------------------

#ifndef LogMsgH
#define LogMsgH
#include <queue>
#include <direct.h>
#include <fstream.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "ConstantString.h"
//---------------------------------------------------------------------------
class cLogMsg
{
	private:
		std::queue<AnsiString> queMsg;
		AnsiString strLogName;
		AnsiString strLocalLog;
		AnsiString strErrorMsg;
		AnsiString strStartTime;
		AnsiString WNetERROR_MSG;
		DWORD dwWriteLogCount;
		bool WNetFileConnection(WNet_LOG_DESCRIPOR WNet);
		bool AddWNetFilePath(WNet_LOG_DESCRIPOR WNet);
		WNet_LOG_DESCRIPOR WNetLOG;
	public:
		cLogMsg::cLogMsg(void);
		void AddLineToLogQueue(AnsiString Line);
		void AddLocalFilePath(AnsiString Path);
		void Local_MC12X_LOG(AnsiString Line);
		void WriteLocalLOG(AnsiString Line,bool bPass,WNet_LOG_DESCRIPOR WNet);
		void WriteErrorMsg(AnsiString Line);
		void SetStartTimeMsg();
		bool WNetWriteLOG(AnsiString Line,WNet_LOG_DESCRIPOR WNet);
        AnsiString ReadQueMsg();
};
#endif
