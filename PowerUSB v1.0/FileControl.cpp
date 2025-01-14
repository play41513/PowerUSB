//---------------------------------------------------------------------------


#pragma hdrstop

#include <vcl.h>
#include "FileControl.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

bool cFileControl::FindINIFile()
{
	TSearchRec Sr;
	TStringList*FileList=new TStringList();
	if(FindFirst("*.ini",faAnyFile,Sr)==0)
	{
		do
		{
			FileList->Add(Sr.Name);
		}
		while(FindNext(Sr)==0);
		FindClose(Sr);
	}
	if(FileList->Count>1||FileList->Count==0)
	{
		delete FileList;
		return false;
	}
	else
	{
		strSettingINI = ExtractFilePath(Application->ExeName)+FileList->Strings[0];
		delete FileList;
		return true;
	}
}
INI_DESCRIPOR cFileControl::ReadINIFile()
{
	AnsiString value;
	TIniFile *ini;
	String fn = ChangeFileExt(strSettingINI, ".ini");
	ini = new TIniFile(fn);
	INI_Desc.dbH_BitVoltage = ini->ReadFloat("Parameter","H_BitVoltage",0.0247);
	INI_Desc.dbL_BitVoltage = ini->ReadFloat("Parameter","L_BitVoltage",0.0153);
	INI_Desc.dwLoopDelay    = ini->ReadInteger("Parameter","MainThreadLoopDalay",50);

	INI_Desc.PCMD_DESCRIPOR.bExitWhenPASS 	= ini->ReadBool("CMD","ExitWhenPASS",false);
	INI_Desc.PCMD_DESCRIPOR.dwDiskCount	 	= ini->ReadInteger("CMD","USBDiskCount",0);

	for(int i = 0 ; i < 13 ; i++)
	{
		INI_Desc.PITEM_AD_DESCRIPOR[i].bTest
			= ini->ReadBool("PowerUSB_"+AnsiString(i+1),"Test",true);
		INI_Desc.PITEM_AD_DESCRIPOR[i].dbVoltageMax
			= ini->ReadFloat("PowerUSB_"+AnsiString(i+1),"MaxVoltage",12);
		INI_Desc.PITEM_AD_DESCRIPOR[i].dbVoltageMin
			= ini->ReadFloat("PowerUSB_"+AnsiString(i+1),"MinVoltage",0);
		INI_Desc.PITEM_AD_DESCRIPOR[i].dbLossVoltage
			= ini->ReadFloat("PowerUSB_"+AnsiString(i+1),"LossVoltage",0);
		value = ini->ReadString("PowerUSB_"+AnsiString(i+1),"BitVoltage","L");
		INI_Desc.PITEM_AD_DESCRIPOR[i].dbBitVoltage
			= value == "H" ? INI_Desc.dbH_BitVoltage : INI_Desc.dbL_BitVoltage;
		INI_Desc.PITEM_AD_DESCRIPOR[i].dwADPortNum
			= ini->ReadInteger("PowerUSB_"+AnsiString(i+1),"ADNum",0);
		INI_Desc.PITEM_AD_DESCRIPOR[i].Name
			= ini->ReadString("PowerUSB_"+AnsiString(i+1),"Name","Port");

		value.printf("%02d",i+1);
		value = "DEVICE"+value;
		INI_Desc.PITEM_USB_DESCRIPOR[i].FriendlyName
			=	value;
	}
	INI_Desc.PITEM_USB_DESCRIPOR[13].FriendlyName = "DEVICE14";
	delete ini;
	return INI_Desc;
}
