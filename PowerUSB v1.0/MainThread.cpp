//---------------------------------------------------------------------------
#pragma hdrstop
#include "MainThread.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TMainThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall TMainThread::TMainThread(bool CreateSuspended,INI_DESCRIPOR info,HWND hWnd)
	: TThread(CreateSuspended)
{
	INI_Desc = info;
	mHwnd	 = hWnd;

	USBThread = new TUSBThread(false,info,hWnd);
}
//---------------------------------------------------------------------------
void __fastcall TMainThread::Execute()
{
	DWORD dwStep = STEP_CHECK_HIDBOARD;
	int   iTemp  = 0;
	bool  bError = false;
	AnsiString strTemp;
	//---- Place thread code here ----
	while( !Terminated )
	{
		switch(dwStep)
		{
			case STEP_CHECK_HIDBOARD:
				if(m_hid.HID_TurnOn())
				{
					INI_Desc.bHIDPlugIn = true;
					bError = false;
					INI_Desc.TestItemNum = 0;
					dwStep = STEP_CHECK_TEST_ITEM;
				}
				else
					dwStep = STEP_HIDERROR;
				break;
			case STEP_CHECK_TEST_ITEM:
				if(INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].bTest)
					dwStep = STEP_READ_VALUE;
				else
				{
					INI_Desc.TestItemNum++;
					if(INI_Desc.TestItemNum >= 13)
					{
						bError = false;
						INI_Desc.TestItemNum = 0;
					}
                }
				break;
			case STEP_READ_VALUE:
				iTemp = INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dwADPortNum;
				if(m_hid.ReadHidValue(iTemp))
				{
					INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dwADValue = m_hid.AD_VALUE.ToInt();
					dwStep = STEP_ADVALUE_CHANGE_TO_VOLTAGE_VALUE;
				}
				else
					dwStep = STEP_HIDERROR;
				break;
			case STEP_ADVALUE_CHANGE_TO_VOLTAGE_VALUE:
				INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dbVoltageValue
					=	(INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dwADValue
						* INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dbBitVoltage)
						+ INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dbLossVoltage;

				strTemp.sprintf("%.3f",INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dbVoltageValue);
				if(strTemp.ToDouble() < 0.1) strTemp = "0";
				INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dbVoltageValue = strTemp.ToDouble();

				dwStep = STEP_CHECK_VOLTAGE_VALUE;
				break;
			case STEP_CHECK_VOLTAGE_VALUE:
				if(INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dbVoltageValue
					> INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dbVoltageMax
					|| INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dbVoltageValue
					< INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].dbVoltageMin)
				{
					INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].bPASS = false;
					bError = true;
				}
				else
					INI_Desc.PITEM_AD_DESCRIPOR[INI_Desc.TestItemNum].bPASS = true;

				PostMessage(mHwnd,WM_AD_VOLTAGE_VALUE,INI_Desc.TestItemNum,NULL);
				INI_Desc.TestItemNum++;
				dwStep = STEP_CHECK_NEXT_STEP;
				break;
			case STEP_CHECK_NEXT_STEP:
				if(INI_Desc.TestItemNum >= 13)
				{
					bool bAllPass = true;
					if(bError)
					{
						PostMessage(mHwnd,WM_VOLTAGE_RESULT,WM_VOLTAGE_FAIL,NULL);
						bAllPass = false;
					}
					else
						PostMessage(mHwnd,WM_VOLTAGE_RESULT,WM_VOLTAGE_PASS,NULL);
					if(INI_Desc.PCMD_DESCRIPOR.dwDiskCount <= USBThread->dwDiskPassCount)
					{
						PostMessage(mHwnd,WM_DISK_RESULT,WM_DISK_PASS,NULL);
					}
					else
					{
						PostMessage(mHwnd,WM_DISK_RESULT,WM_DISK_FAIL,NULL);
						bAllPass = false;
                    }
					if(INI_Desc.PCMD_DESCRIPOR.bExitWhenPASS)
					{
						if(bAllPass)
						{
							mLogMsg->Local_MC12X_LOG("PASSED");
							USBThread->Terminate();
							TMainThread::Terminate();
							PostMessage(mHwnd,WM_QUIT,0,0);
						}
					}
					bError = false;
					INI_Desc.TestItemNum = 0;
				}
				dwStep = STEP_CHECK_TEST_ITEM;
				break;
			case STEP_HIDERROR:
				INI_Desc.bHIDPlugIn = false;
				PostMessage(mHwnd,WM_AD_VOLTAGE_VALUE,0,NULL);
				dwStep = STEP_CHECK_HIDBOARD;
				break;

		}
		Sleep(INI_Desc.dwLoopDelay);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainThread::ShowResult()
{

	//edResult->Text=strResult;
}
void TMainThread::Show()
{
	AnsiString SS;
	if(m_hid.ReadHidValue(0))
	{
		SS = m_hid.AD_VALUE;
	}
}
INI_DESCRIPOR TMainThread::ReadDesc()
{
	return	INI_Desc;
}
