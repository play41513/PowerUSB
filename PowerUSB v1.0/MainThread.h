//---------------------------------------------------------------------------

#ifndef MainThreadH
#define MainThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <vcl.h>
#include "HID.h"
#include "ConstantString.h"
#include "LogMsg.h"
#include <cmath>
#include <iostream>
#include "USBThread.h"

#define STEP_CHECK_HIDBOARD						1
#define STEP_CHECK_TEST_ITEM					2
#define STEP_READ_VALUE							3
#define STEP_ADVALUE_CHANGE_TO_VOLTAGE_VALUE	4
#define STEP_CHECK_VOLTAGE_VALUE				5
#define STEP_CHECK_NEXT_STEP					6
#define STEP_HIDERROR			100
//---------------------------------------------------------------------------
class TMainThread : public TThread
{
private:
	INI_DESCRIPOR INI_Desc;
	HID m_hid;
	HWND mHwnd;
	cLogMsg *mLogMsg;
	void Show();

	TUSBThread *USBThread;
protected:
	void __fastcall Execute();
	void __fastcall ShowResult();        //�۩w�q���
public:
	__fastcall TMainThread(bool CreateSuspended,INI_DESCRIPOR info,HWND hWnd);
	INI_DESCRIPOR ReadDesc();
};
//---------------------------------------------------------------------------
#endif
