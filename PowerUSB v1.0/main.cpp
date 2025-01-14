//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;
//---------------------------------------------------------------------------
__fastcall TfrmMain::TfrmMain(TComponent* Owner)
	: TForm(Owner)
{
	frmMain->Caption = APP_TITLE;
	CL_FileControl = new cFileControl();
	FindIniFile();

	mainThread = new TMainThread(false,INI_Desc,Handle);

	//����
	OldFormWndProc = this->WindowProc;
	this->WindowProc = FormWndProc;
}
//---------------------------------------------------------------------------
void TfrmMain::FindIniFile()
{
	if(CL_FileControl->FindINIFile())
		INI_Desc =  CL_FileControl->ReadINIFile();
	for(int i = 0 ; i < 13 ; i++)
	{
		((TPanel*)frmMain->FindComponent("plVoltageName"+AnsiString(i+1)))->Caption
			= INI_Desc.PITEM_AD_DESCRIPOR[i].Name;
	}
}
void __fastcall TfrmMain::FormWndProc(TMessage& Message)
{
	switch(Message.Msg)
	{
		case WM_MEMO:
			//Memo->Lines->Add(Message.WParam);
			//if(!Memo->Lines->Text.Pos("99"))
			//	Memo->Lines->Add(Message.WParam);
			break;
		case WM_AD_VOLTAGE_VALUE:
		{
			INI_Desc =  mainThread->ReadDesc();
			TPanel * plItemVoltage
				= ((TPanel*)frmMain->FindComponent("plItemVoltage"+AnsiString(Message.WParam+1)));
			if(INI_Desc.bHIDPlugIn == false)
			{
				plItemVoltage->Caption = "NULL";
				plItemVoltage->Color = clYellow;
			}
			else
			{
				if(INI_Desc.PITEM_AD_DESCRIPOR[Message.WParam].bPASS)
				{
					plItemVoltage->Color = clLime;
				}
				else
					plItemVoltage->Color = clRed;
				plItemVoltage->Caption = INI_Desc.PITEM_AD_DESCRIPOR[Message.WParam].dbVoltageValue;
			}
			frmMain->Refresh();
			break;
		}
		case WM_USB_DISK:
		{
			TPanel * plUSBDisk
				= ((TPanel*)frmMain->FindComponent("plDisk"+AnsiString(Message.LParam)));
			switch(Message.WParam)
			{
				case WM_USB_CLEAR:
					plUSBDisk->Color = clCream;
					break;
				case WM_USB_PASS:
					plUSBDisk->Color = clLime;
					break;
				case WM_USB_FAIL:
					plUSBDisk->Color = clRed;
					break;
			}
			frmMain->Refresh();
			break;
		}
		case WM_VOLTAGE_RESULT:
		{
			switch(Message.WParam)
			{
				case WM_VOLTAGE_PASS:
					plVoltageResult->Caption = "PASS";
					plVoltageResult->Font->Color    = clGreen;
					break;
				case WM_VOLTAGE_FAIL:
					plVoltageResult->Caption = "FAIL";
					plVoltageResult->Font->Color    = clRed;
					break;
				default:
					plVoltageResult->Caption = "";
					plVoltageResult->Font->Color    = clBlack;
					break;
			}
			frmMain->Refresh();
			break;
		}
		case WM_DISK_RESULT:
		{
			switch(Message.WParam)
			{
				case WM_DISK_PASS:
					plDiskResult->Caption = "PASS";
					plDiskResult->Font->Color    = clGreen;
					break;
				case WM_DISK_FAIL:
					plDiskResult->Caption = "WAIT";
					plDiskResult->Font->Color    = clHotLight;
					break;
				default:
					plDiskResult->Caption = "";
					plDiskResult->Font->Color    = clBlack;
					break;
			}
			frmMain->Refresh();
			break;
		}
	}
	TForm :: WndProc(Message);
}
