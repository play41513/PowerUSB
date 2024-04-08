//---------------------------------------------------------------------------


#pragma hdrstop

#include <vcl.h>
#include "LogMsg.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
cLogMsg::cLogMsg(void) {
	dwWriteLogCount = 0;
}

void cLogMsg::AddLineToLogQueue(AnsiString Line)
{
	queMsg.push(Line);
}
AnsiString cLogMsg::ReadQueMsg()
{
	if(queMsg.size())
	{
		AnsiString strTemp;
		strTemp = queMsg.front();
		queMsg.pop();
		return strTemp;
	}
	return "";
}
void cLogMsg::AddLocalFilePath(AnsiString Path)
{
	if(!DirectoryExists(Path.c_str()))
	{
		AnsiString SS,Temp = "";
		while(true)
		{
			SS = Path.SubString(1,Path.Pos("\\"));
			Path.Delete(1,Path.Pos("\\"));
			Temp+=SS;
			_mkdir( Temp.c_str());
			if(Path == "")
				break;
		}
	}
	strLogName = Path;
	strLogName += AnsiString(FormatDateTime("yyyy-mm-dd_hh-nn-ss", Now()))+".csv";
}
void cLogMsg::WriteErrorMsg(AnsiString Line)
{
	strErrorMsg = ",ErrorMsg,"+Line;
}
void cLogMsg::SetStartTimeMsg()
{
	strStartTime = ",StartTime,"+AnsiString(FormatDateTime("hh:nn:ss", Now()));
}
void cLogMsg::WriteLocalLOG(AnsiString Line,bool bPass,WNet_LOG_DESCRIPOR WNet)
{
	AddLineToLogQueue("Writing to the Log...");
	dwWriteLogCount++;
	AnsiString Temp;
	Temp = AnsiString(dwWriteLogCount)+strStartTime
		+",EndTime,"+AnsiString(FormatDateTime("hh:nn:ss", Now()))
		+","+Line;
	Temp += bPass ? ",Result,PASS" : ",Result,FAIL";
	if(!bPass)
		Temp += strErrorMsg;

	if(WNet.bWNetLog)
		WNetWriteLOG(Temp,WNet);


	FILE * fp;
	fp = fopen(strLogName.c_str(),"a+");
	fseek( fp, 0, SEEK_END);
	fwrite(Temp.c_str(),Temp.Length(),1,fp); //寫入一筆資料
	fclose(fp);

	AddLineToLogQueue("Finished writing the log");
}
void cLogMsg::Local_MC12X_LOG(AnsiString Line)
{
	FILE * fp;
	AnsiString Temp = ExtractFilePath(Application->ExeName)+"PowerUSB_Disk.log";
	fp = fopen(Temp.c_str(),"a+");
	fseek( fp, 0, SEEK_END);
	fwrite(Line.c_str(),Line.Length(),1,fp); //寫入一筆資料
	fclose(fp);
}
bool cLogMsg::WNetFileConnection(WNet_LOG_DESCRIPOR WNet)
{
	bool bPASS = false;
	if(!DirectoryExists("\\\\"+WNet.IP+"\\"+WNet.ufolder))
	{
		//目標根目錄
		AnsiString MapSharedPath = "\\\\"+WNet.IP+"\\"+WNet.ufolder;
		//目標用戶名
		AnsiString MapUser = WNet.User;
		//目標密碼
		AnsiString MapPassword = WNet.Password;

		NETRESOURCEA nr;
		DWORD res;
		nr.dwType = RESOURCETYPE_ANY;
		nr.lpLocalName = "";
		AnsiString SS =  "\\\\"+WNet.IP+"\\"+WNet.ufolder;
		nr.lpRemoteName = SS.c_str();
		nr.lpProvider = NULL;

		res = WNetCancelConnection2A(nr.lpRemoteName,0,false);
		if(NO_ERROR!=res)
		{
			//WinExec("NET USE * /DELETE /Y",SW_HIDE);
		}

		res = WNetAddConnection2A(&nr, MapPassword.c_str(), MapUser.c_str(), CONNECT_UPDATE_PROFILE);
		switch(res)
		{
			case NO_ERROR:
				WNetERROR_MSG = "Connection Succeeded\n";
				bPASS = true;
				break;
			case ERROR_BAD_PROFILE:
				WNetERROR_MSG = "ERROR_BAD_PROFILE\n";
				break;
			case ERROR_CANNOT_OPEN_PROFILE:
				WNetERROR_MSG = "ERROR_CANNOT_OPEN_PROFILE\n";
				break;
			case ERROR_DEVICE_IN_USE:
				WNetERROR_MSG = "ERROR_DEVICE_IN_USE\n";
				break;
			case ERROR_EXTENDED_ERROR:
				WNetERROR_MSG = "ERROR_EXTENDED_ERROR\n";
				break;
			case ERROR_NOT_CONNECTED:
				WNetERROR_MSG = "ERROR_NOT_CONNECTED\n";
				break;
			case ERROR_LOGON_FAILURE:
				WNetERROR_MSG = "ERROR_LOGON_FAILURE\n";
				break;
			case ERROR_NO_NETWORK:
				WNetERROR_MSG = "ERROR_NO_NETWORK\n";
				break;
			case ERROR_OPEN_FILES:
				WNetERROR_MSG = "ERROR_OPEN_FILES\n";
				break;
			case ERROR_INVALID_PASSWORD:
				WNetERROR_MSG = "ERROR_INVALID_PASSWORD\n";
				break;
			default:
				WNetERROR_MSG = "Unknown Error\n";
				//WNetERROR_MSG = "未知錯誤 可能需要帳密 或路徑不存在";
				break;
		}
		if(!bPASS)
			AddLineToLogQueue("(!)"+WNetERROR_MSG);
		return bPASS;
	}
	else return true;
}
bool cLogMsg::WNetWriteLOG(AnsiString Line,WNet_LOG_DESCRIPOR WNet)
{
	AddLineToLogQueue("Writing to the WNetLog...");
	WNetERROR_MSG = "";
	if(AddWNetFilePath(WNet))
	{
		AnsiString FileName = "\\\\"+WNet.IP+"\\"+WNet.ufolder;
		AnsiString FileLine = Line+"\r\n";
		FILE * fp;
		fp = fopen(FileName.c_str(),"a+");
		fseek( fp, 0, SEEK_END);
		fwrite(FileLine.c_str(),FileLine.Length(),1,fp); //寫入一筆資料
		fclose(fp);
		return true;
	}
	else
		return false;
}
bool cLogMsg::AddWNetFilePath(WNet_LOG_DESCRIPOR WNet)
{
	bool bPASS = false;
	if(WNetFileConnection(WNet))
	{
		AnsiString FileName = "\\\\"+WNet.IP+"\\"+WNet.ufolder;
		if(!FileExists(FileName.c_str())){
			TStringList *sList = new TStringList();
			sList->Delimiter = '\\';
			sList->DelimitedText = AnsiString(WNet.ufolder);
			FileName = "\\\\"+WNet.IP+"\\"+sList->Strings[0];
			_mkdir(FileName.c_str());
			FileName = FileName+"\\"+sList->Strings[1];
			_mkdir(FileName.c_str());
			FileName = FileName+"\\"+sList->Strings[2];
			_mkdir(FileName.c_str());
			FileName = FileName+"\\"+sList->Strings[3];
			_mkdir(FileName.c_str());
			delete  sList;
			if(DirectoryExists(FileName))
				bPASS = true;
			else
			{
				bPASS = false;
			}
		}
		else bPASS = true;
	}
	else bPASS = false;
	return bPASS;
}
