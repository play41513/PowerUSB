//---------------------------------------------------------------------------

#ifndef USBDiskH
#define USBDiskH

#include "ConstantString.h"
#include <iostream>
#include "stdio.h"

#define BUFFER_SIZE 1024*1024	//設定每次測試的檔案大小 1M
#define BUFSIZE 2048   //For buf buffer size
//---------------------------------------------------------------------------
class cUSBDisk
{
	private:
		INI_DESCRIPOR INI_Desc;
		AnsiString CheckDISKVolumelabel(String Disk);
		char DiskWriteRead(String Disk);
		bool CheckDiskName(String strName);
	public:
		INI_DESCRIPOR GetUSBDisk(INI_DESCRIPOR info);
};
#endif
