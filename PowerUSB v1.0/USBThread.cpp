//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "USBThread.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TUSBThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall TUSBThread::TUSBThread(bool CreateSuspended,INI_DESCRIPOR info,HWND hWnd)
	: TThread(CreateSuspended)
{
	INI_Desc = info;
	mHwnd	 = hWnd;
}
//---------------------------------------------------------------------------
void __fastcall TUSBThread::Execute()
{
	while( !Terminated )
	{
		INI_DescTemp = mUSBDisk->GetUSBDisk(INI_Desc);
		INI_Desc     = UpdateUSBDesc(INI_Desc,INI_DescTemp);
		Sleep(500);
    }
}
//---------------------------------------------------------------------------
INI_DESCRIPOR TUSBThread::UpdateUSBDesc(INI_DESCRIPOR infoDesc , INI_DESCRIPOR tempDesc)
{
	for(int i = 0 ; i < 14 ; i++)
	{
		//空白 跳過
		if(tempDesc.PITEM_USB_DESCRIPOR[i].bPlugIn == false
			&& infoDesc.PITEM_USB_DESCRIPOR[i].bPlugIn == false)
			continue;
		//原本有 被拔起
		if(tempDesc.PITEM_USB_DESCRIPOR[i].bPlugIn == false
			&& infoDesc.PITEM_USB_DESCRIPOR[i].bPlugIn == true)
		{
			infoDesc.PITEM_USB_DESCRIPOR[i].bPlugIn = false;
			PostMessage(mHwnd,WM_USB_DISK,WM_USB_CLEAR,i+1);//UI清空
		}
		//原本沒有 新插入
		if(tempDesc.PITEM_USB_DESCRIPOR[i].bPlugIn == true
			&& infoDesc.PITEM_USB_DESCRIPOR[i].bPlugIn == false)
		{
			infoDesc.PITEM_USB_DESCRIPOR[i].bPlugIn = true;
			infoDesc.PITEM_USB_DESCRIPOR[i].bPASS = tempDesc.PITEM_USB_DESCRIPOR[i].bPASS;
			infoDesc.PITEM_USB_DESCRIPOR[i].bUIChange = tempDesc.PITEM_USB_DESCRIPOR[i].bUIChange;
			if(infoDesc.PITEM_USB_DESCRIPOR[i].bPASS)
				PostMessage(mHwnd,WM_USB_DISK,WM_USB_PASS,i+1);//UI PASS
			else
				PostMessage(mHwnd,WM_USB_DISK,WM_USB_FAIL,i+1);//UI FAIL
        }
	}
	dwDiskPassCount = tempDesc.PCMD_DESCRIPOR.dwDiskPassCount;
	return infoDesc;
}

