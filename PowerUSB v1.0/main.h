//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <dbt.h> //DBT_常數 註冊要取得的裝置消息

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "FileControl.h"
#include "ConstantString.h"
#include "MainThread.h"
#include <ExtCtrls.hpp>
#include <pngimage.hpp>
//---------------------------------------------------------------------------
class TfrmMain : public TForm
{
__published:	// IDE-managed Components
	TPanel *plItem1;
	TPanel *Panel8;
	TScrollBox *ScrollBox1;
	TPanel *Panel4;
	TPanel *Panel5;
	TPanel *plVoltageName13;
	TPanel *plItemVoltage13;
	TPanel *Panel6;
	TPanel *Panel7;
	TPanel *plVoltageName12;
	TPanel *plItemVoltage12;
	TPanel *Panel9;
	TPanel *Panel10;
	TPanel *plVoltageName11;
	TPanel *plItemVoltage11;
	TPanel *Panel12;
	TPanel *Panel13;
	TPanel *plVoltageName10;
	TPanel *plItemVoltage10;
	TPanel *Panel15;
	TPanel *Panel16;
	TPanel *plVoltageName9;
	TPanel *plItemVoltage9;
	TPanel *Panel18;
	TPanel *Panel19;
	TPanel *plVoltageName8;
	TPanel *plItemVoltage8;
	TPanel *Panel21;
	TPanel *Panel22;
	TPanel *plVoltageName7;
	TPanel *plItemVoltage7;
	TPanel *Panel24;
	TPanel *Panel25;
	TPanel *plVoltageName6;
	TPanel *plItemVoltage6;
	TPanel *Panel27;
	TPanel *Panel28;
	TPanel *plVoltageName5;
	TPanel *plItemVoltage5;
	TPanel *Panel30;
	TPanel *Panel31;
	TPanel *plVoltageName4;
	TPanel *plItemVoltage4;
	TPanel *Panel33;
	TPanel *Panel34;
	TPanel *plVoltageName3;
	TPanel *plItemVoltage3;
	TPanel *Panel36;
	TPanel *Panel37;
	TPanel *plVoltageName2;
	TPanel *plItemVoltage2;
	TPanel *Panel39;
	TPanel *Panel40;
	TPanel *plVoltageName1;
	TPanel *plItemVoltage1;
	TPanel *Panel2;
	TPanel *Panel11;
	TPanel *Panel14;
	TPanel *Panel20;
	TPanel *Panel3;
	TPanel *Panel23;
	TPanel *Panel26;
	TScrollBox *ScrollBox2;
	TPanel *plDisk1;
	TPanel *plDisk2;
	TPanel *plDisk3;
	TPanel *plDisk4;
	TPanel *plDisk5;
	TPanel *plDisk6;
	TPanel *plDisk7;
	TPanel *plDisk8;
	TPanel *plDisk9;
	TPanel *plDisk10;
	TPanel *plDisk11;
	TPanel *plDisk12;
	TPanel *plDisk13;
	TPanel *plDisk14;
	TPanel *plStatus;
	TPanel *Panel1;
	TPanel *Panel29;
	TPanel *Panel35;
	TPanel *Panel41;
	TPanel *plVoltageResult;
	TPanel *plDiskResult;
	TImage *Image1;
	TPanel *Panel17;
	TPanel *Panel32;
private:	// User declarations
	INI_DESCRIPOR INI_Desc;
	cFileControl *CL_FileControl;

	void FindIniFile();
	TMainThread *mainThread;
public:		// User declarations
	__fastcall TfrmMain(TComponent* Owner);

protected:
	TWndMethod OldFormWndProc;
	void __fastcall FormWndProc(TMessage& Message);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
