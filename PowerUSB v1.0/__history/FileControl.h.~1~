//---------------------------------------------------------------------------

#ifndef FileControlH
#define FileControlH
#include <fstream.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <IniFiles.hpp>
#include "ConstantString.h"
//---------------------------------------------------------------------------

class cFileControl
{
	private:
		AnsiString strSettingINI;
		INI_DESCRIPOR INI_Desc;

	public:
		bool FindINIFile();
		INI_DESCRIPOR ReadINIFile();
		void 		  WriteINIFile(INI_DESCRIPOR info);

};
#endif
