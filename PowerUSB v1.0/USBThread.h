//---------------------------------------------------------------------------

#ifndef USBThreadH
#define USBThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "USBDisk.h"
#include "ConstantString.h"
//---------------------------------------------------------------------------
class TUSBThread : public TThread
{
protected:
	void __fastcall Execute();
public:
	__fastcall TUSBThread(bool CreateSuspended,INI_DESCRIPOR info,HWND hWnd);
	DWORD dwDiskPassCount;
private:
	cUSBDisk *mUSBDisk;
	INI_DESCRIPOR INI_Desc;
	INI_DESCRIPOR INI_DescTemp;
	HWND mHwnd;

	INI_DESCRIPOR UpdateUSBDesc(INI_DESCRIPOR infoDesc , INI_DESCRIPOR tempDesc);
};
//---------------------------------------------------------------------------
#endif
