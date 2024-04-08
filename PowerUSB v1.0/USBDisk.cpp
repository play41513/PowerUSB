//---------------------------------------------------------------------------


#pragma hdrstop
#include <vcl.h>
#include "USBDisk.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
BYTE WRITE_BUFFER[BUFFER_SIZE];	//�g�J�Ϊ��w�İ�
BYTE READ_BUFFER[BUFFER_SIZE];	//Ū���Ϊ��w�İ�
INI_DESCRIPOR cUSBDisk::GetUSBDisk(INI_DESCRIPOR info)
{
	int iDiskPassCount = 0;
	AnsiString astrTemp;
	char lpBuffer[MAX_PATH];
	char*lpSingleDevice;
	ITEM_USB_DESCRIPOR USB_Desc;
	for(int i =0 ;i<14;i++)
		info.PITEM_USB_DESCRIPOR[i].bPlugIn = false;
	DWORD nBufferLength = MAX_PATH;
	DWORD dwSignleDeviceBufferIndex =0, dwDeviceNum =0;

	lpSingleDevice  = (char*)malloc(sizeof(char)* MAX_PATH);
	if(lpSingleDevice == NULL)
	{
		//�t�Τ��s���t���~
		return info;
	}
	memset(lpSingleDevice,0x0,MAX_PATH);
	memset(lpBuffer,0x0,MAX_PATH);

	nBufferLength = GetLogicalDriveStringsA(nBufferLength,lpBuffer); //�C�|

	for(DWORD i =0; i< nBufferLength; i++)
	{
		if(lpBuffer[i] != 0x00)       //�����U�ӽL��
		{
		lpSingleDevice[dwSignleDeviceBufferIndex] = lpBuffer[i];
		dwSignleDeviceBufferIndex++;
		continue;
		}
		lpSingleDevice[dwSignleDeviceBufferIndex]  = 0x0;
		dwSignleDeviceBufferIndex = 0;
		if(*lpSingleDevice == 'A')  //A���ݭn
			continue;
		DWORD dwIndex = GetDriveTypeA(lpSingleDevice);
		if(GetDriveTypeA(lpSingleDevice) == DRIVE_REMOVABLE
			|| *lpSingleDevice == 'G')   //����H����
		{
			USB_Desc.Volume = lpSingleDevice;
			USB_Desc.FriendlyName = CheckDISKVolumelabel(USB_Desc.Volume);
			//��X��Ū�g�L��
			for(int i =0 ;i<14;i++)
			{
				if(info.PITEM_USB_DESCRIPOR[i].FriendlyName == USB_Desc.FriendlyName)
				{
					info.PITEM_USB_DESCRIPOR[i].bPlugIn = true;
					if(info.PITEM_USB_DESCRIPOR[i].bPASS == false
						&& info.PITEM_USB_DESCRIPOR[i].bUIChange == false)
					{
						switch(DiskWriteRead(USB_Desc.Volume))
						{
							case 0x01:
								info.PITEM_USB_DESCRIPOR[i].bPASS = true;
								break;
							case 0x00:
								info.PITEM_USB_DESCRIPOR[i].bPASS = false;
								break;
						}
						info.PITEM_USB_DESCRIPOR[i].bUIChange == true;
                    }
                }
			}
		}
	}
	free(lpSingleDevice);
	//Check Disk(not plugin)
	for(int i = 0 ; i < 14 ; i++)
	{
		if(info.PITEM_USB_DESCRIPOR[i].bPlugIn == false)
		{
			info.PITEM_USB_DESCRIPOR[i].bPASS = false;
			info.PITEM_USB_DESCRIPOR[i].bUIChange = false;
		}
		else
		{
			if(info.PITEM_USB_DESCRIPOR[i].bPASS)
				iDiskPassCount++;
        }
	}
	info.PCMD_DESCRIPOR.dwDiskPassCount = iDiskPassCount;
	return info;
}
AnsiString cUSBDisk::CheckDISKVolumelabel(String Disk)
{
	AnsiString SS = Disk;;
	char Volumelabel[20];
	DWORD SerialNumber;
	DWORD MaxCLength;
	DWORD FileSysFlag;
	char FileSysName[10];
	GetVolumeInformationA(SS.c_str(),Volumelabel,255,&SerialNumber,&MaxCLength,&FileSysFlag,FileSysName,255);
	return Volumelabel;
}
char  cUSBDisk::DiskWriteRead(String Disk)
{
	try
	{
	memset(WRITE_BUFFER,(BYTE)0xA5,BUFFER_SIZE);
	AnsiString FileName= Disk;
						  //Filename = �s�ϺЪ��ɶ��ɮ�
	FileName+=":\\"+FormatDateTime("yymmddhhmmss",Now())+".bin";
	FILE *fp;                       //�ɮ׫���
	fp=fopen(FileName.c_str(),"a+");
	if(fp == NULL)
       return 0x01;	//EEEOR
	fseek(fp,0,SEEK_END);
	fwrite(WRITE_BUFFER,BUFFER_SIZE,1,fp);
	fclose(fp);

	fp=fopen(FileName.c_str(),"rb");
	if(fp == NULL)
	   return 0x01;	//EEEOR
	fseek(fp,0,SEEK_SET);
	fread(READ_BUFFER,BUFFER_SIZE,1,fp);
	fclose(fp);
	DeleteFile(FileName.c_str());
	if (memcmp(READ_BUFFER,WRITE_BUFFER,BUFFER_SIZE))
	{
		return 0x01;	//EEEOR
	}
	else
	{
		return 0x00;	//PASS
	}
	}
	catch(...)
	{
		return 0x01;	//EEEOR
	}
}
bool cUSBDisk::CheckDiskName(String strName)
{
	if(strName.Pos("DEVICE"))
	{
		AnsiString strTemp;
		for(int i = 1 ; i <= 14 ; i++)
		{
			strTemp.printf("%02d",i);
			strTemp = "DEVICE"+strTemp;
			if(strName == strTemp)
				return true;
		}
	}
	return false;
}