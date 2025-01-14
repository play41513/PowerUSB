﻿#ifndef ConstantStringH
#define ConstantStringH
//---------------------------------------------------------------------------
//標題
#define APP_TITLE "PowerUSB Tool ver 1.0 ( ActionStar.)";

#define WM_AD_VOLTAGE_VALUE				WM_USER+100
#define WM_USB_DISK						WM_USER+200
#define WM_USB_CLEAR					WM_USB_DISK+1
#define WM_USB_PASS						WM_USB_DISK+2
#define WM_USB_FAIL						WM_USB_DISK+3

#define WM_MEMO							WM_USER+300
#define WM_VOLTAGE_RESULT               WM_USER+400
#define WM_VOLTAGE_PASS               	WM_VOLTAGE_RESULT+1
#define WM_VOLTAGE_FAIL               	WM_VOLTAGE_RESULT+2
#define WM_DISK_RESULT               	WM_USER+500
#define WM_DISK_PASS               		WM_DISK_RESULT+1
#define WM_DISK_FAIL               		WM_DISK_RESULT+2

typedef struct _ITEM_AD_DESCRIPOR
{
  bool	 bTest;
  bool   bPASS;
  DWORD	 dwADPortNum;
  DWORD  dwADValue;
  double dbVoltageMax;
  double dbVoltageMin;
  double dbLossVoltage;
  double dbBitVoltage;

  double dbVoltageValue;
  AnsiString Name;
}ITEM_AD_DESCRIPOR;

typedef struct _ITEM_USB_DESCRIPOR
{
  bool   bPASS;
  bool   bPlugIn;
  bool   bUIChange;
  AnsiString Volume;
  AnsiString FriendlyName;
}ITEM_USB_DESCRIPOR;

typedef struct _CMD_DESCRIPOR
{
  bool   bExitWhenPASS;
  DWORD  dwDiskCount;

  DWORD  dwDiskPassCount;
}CMD_DESCRIPOR;

typedef struct _INI_DESCRIPOR
{
  double dbH_BitVoltage;
  double dbL_BitVoltage;
  bool   bHIDPlugIn;
  //
  ITEM_USB_DESCRIPOR PITEM_USB_DESCRIPOR[14];
  ITEM_AD_DESCRIPOR PITEM_AD_DESCRIPOR[13];
  CMD_DESCRIPOR		PCMD_DESCRIPOR;
  DWORD TestItemNum;
  DWORD dwLoopDelay;

}INI_DESCRIPOR;

typedef struct _WNet_LOG_DESCRIPOR
{
  bool bWNetLog;
  AnsiString IP;
  AnsiString User;
  AnsiString Password;
  AnsiString ufolder;

  bool  bRecordMAC;
  AnsiString NetworkCardName;
  AnsiString NetworkCardRecordMAC;
}WNet_LOG_DESCRIPOR;


#define DEBUG(String)    FrmMain->moDebug->Lines->Add(String)


//---------------------------------------------------------------------------




#define MB_WARNING(handle, msg) MessageBox(handle, msg, MB_CAPTIONWARNING, MB_ICONWARNING)
//---------------------------------------------------------------------------
#endif
