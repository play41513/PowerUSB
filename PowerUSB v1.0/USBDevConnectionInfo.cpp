//---------------------------------------------------------------------------
#include <windows.h>    // 安全移除USB裝置用 *要比 vcl.h 早編譯
#include <SetupAPI.h> // 安全移除USB裝置用 *要比 vcl.h 早編譯
#include <cfgmgr32.h>
#pragma hdrstop
#include "USBDevConnectionInfo.h"
#define BUFSIZE 2048   //For buf buffer size
//---------------------------------------------------------------------------

#pragma package(smart_init)
//共用變數
static bool bInit;
static DWORD ComparisonUSBDiskU2,ComparisonUSBDiskU3;
static DWORD dwCountUSBDiskU2,dwCountUSBDiskU3;
static AnsiString USBDevString,USBDiskString;
static AnsiString astrUSBDevComparison;


USBDevConnectionInfo::USBDevConnectionInfo(void)
{
	//
	bInit = true;
	EnmuUSBView();
	bInit = false;
	//
}
void USBDevConnectionInfo::EnmuUSBView()
{
	if(bInit)
		vecInitUSBDevDesc.clear();
	vecUSBDevDesc.clear();

	HANDLE HostControllerHandle;
	char HostControllerName[] = "\\\\.\\HCD0";
	PCHAR driverKeyName, deviceDesc;
	AnsiString str1;   //Tree node name

	SA.nLength = sizeof(SECURITY_ATTRIBUTES);
	SA.lpSecurityDescriptor = NULL;
	SA.bInheritHandle = false;

	USB_Device_Information.devicePath=NULL;    //Device Path Clean at begining

	for (DWORD i=0; i<10; i++)
	{
		HostControllerName[7] = i + '0';
		HostControllerHandle = CreateFileA(HostControllerName, GENERIC_WRITE, FILE_SHARE_WRITE, &SA, OPEN_EXISTING, 0, NULL);

		if (HostControllerHandle != INVALID_HANDLE_VALUE)
		{
			EnumerateHostController(HostControllerHandle);
		}
		CloseHandle(HostControllerHandle);
	}

	USBDevString = "";
	for(DWORD i = 0 ;i < vecUSBDevDesc.size();i++)
	{
		USBDevString += vecUSBDevDesc[i]+"\r\n";
	}
	USBDevString = ComparisonUSBDev(USBDevString,astrUSBDevComparison);
}
String USBDevConnectionInfo::ComparisonUSBDev(String OutputStr,String ComparStr)
{
	TStringList *OutputList = new TStringList();
	OutputList->Delimiter = '\n';
	OutputList->DelimitedText = OutputStr;

	TStringList *ComparList = new TStringList();
	ComparList->Delimiter = '\n';
	ComparList->DelimitedText = ComparStr;

	String ReturnString = "";
	bool bHave;
	for(int i = 0;i < ComparList->Count ;i++)
	{
		bHave = false;
		for(int j = 0;j < OutputList->Count;j++)
		{
			if(OutputList->Strings[j].Pos(ComparList->Strings[i]))
			{
				ReturnString += OutputList->Strings[j]+"[O]\r\n";
				OutputList->Delete(j);
				bHave = true;
			}
		}
		if(bHave == false)
			ReturnString += ComparList->Strings[i]+"[X]\r\n";
	}
	ReturnString += OutputList->Text;
	delete OutputList;
	delete ComparList;
	return ReturnString;
}
DWORD USBDevConnectionInfo::GetUSBType(String USBInfo)
{
	USBInfo    = USBInfo.UpperCase();
	String VID = USBInfo.SubString(USBInfo.Pos("VID")+4,4);
	String PID = USBInfo.SubString(USBInfo.Pos("PID")+4,4);
	AnsiString Path = "";
	if(USBInfo.Pos("{"))
		Path = USBInfo.SubString(USBInfo.Pos("#")+1,USBInfo.Pos("{")-USBInfo.Pos("#")).UpperCase();
	else
		Path = USBInfo.SubString(USBInfo.Pos("#")+1,USBInfo.Length()-USBInfo.Pos("#")).UpperCase();

	for(DWORD j=0;j<vecUSBDevDesc.size();j++)
	{
		if (vecUSBDevDesc[j].Pos(Path))//確認裝置路徑碼
		{
			if (vecUSBDevDesc[j].Pos("VID_"+VID))//確認裝置VID
			{
				if (vecUSBDevDesc[j].Pos("PID_"+PID))//確認裝置PID
				{
					if(vecUSBDevDesc[j].Pos("[GEN2]"))
						return USBTYPE_SUPER_SPEED_PLUS;
					else if(vecUSBDevDesc[j].Pos("[USB3]"))
						return USBTYPE_SUPER_SPEED;
					else
						return USBTYPE_HI_SPEED;
				}
			}
		}
	}
	return USBTYPE_NOT_FIND_DEV;
}
DWORD USBDevConnectionInfo::EnumerateHostController(HANDLE HostControllerHandle)
{
  DWORD BytesReturned;
  bool Success;
  char RootHubName[256] = "\\\\.\\";
  HANDLE RootHubHandle;
  PUSBDEVICEINFO  info;

  info = NULL;
  info = (PUSBDEVICEINFO) alloca(sizeof(USBDEVICEINFO));
  info->HubInfo = (PUSB_NODE_INFORMATION)alloca(sizeof(USB_NODE_INFORMATION));


  // First get the system name of the host controller for display
  Success = DeviceIoControl(HostControllerHandle, IOCTL_GET_HCD_DRIVERKEY_NAME,
		   &UnicodeName,sizeof(UnicodeName),&UnicodeName, sizeof(UnicodeName), &BytesReturned, NULL);

  if(Success)
  {
	// Now get the system name of it's root hub for interrogation
	Success = DeviceIoControl(HostControllerHandle, IOCTL_USB_GET_ROOT_HUB_NAME, &UnicodeName,
            sizeof(UnicodeName),&UnicodeName, sizeof(UnicodeName), &BytesReturned, NULL);

	if(Success)
	{
	  // Now open the root hub. Need to construct a char name from "\\.\" + UnicodeName
      WideCharToMultiByte(CP_ACP, 0, &UnicodeName.Name[0], (UnicodeName.Length)/2, &RootHubName[4], 252, NULL,NULL);

	  RootHubHandle = CreateFileA(RootHubName, GENERIC_WRITE, FILE_SHARE_WRITE, &SA, OPEN_EXISTING, 0, NULL);
	  if(RootHubHandle != INVALID_HANDLE_VALUE)
	  {
        // Root hub is open. Collect the node information
        Success = DeviceIoControl(RootHubHandle, IOCTL_USB_GET_NODE_INFORMATION, info->HubInfo,
                  sizeof(USB_NODE_INFORMATION), info->HubInfo, sizeof(USB_NODE_INFORMATION), &BytesReturned, NULL);

		if (!Success)
          return GetLastError();
        else
		{
		  // Get the ports and take the data
		  GetPortData(RootHubHandle, info->HubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts, 0);
        }
      }
	  CloseHandle(RootHubHandle);
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
void USBDevConnectionInfo::GetPortData(HANDLE HubHandle, UCHAR PortCount, int HubDepth)
{
  DWORD BytesReturned;
  bool Success;
  ULONG PortIndex;
  USHORT LanguageID;
  UCHAR PortStatus;
  char ConnectedHubName[256] = "\\\\.\\";
  HANDLE ConnectedHubHandle;
  NODE_INFORMATION NodeInformation;
  NODE_CONNECTION_INFORMATION ConnectionInformation;
  AnsiString str1;
  PCHAR driverKeyName, deviceDesc;
  PUSBDEVICEINFO  info;

  info = NULL;
  info = (PUSBDEVICEINFO) alloca(sizeof(USBDEVICEINFO));
  info->HubInfo = (PUSB_NODE_INFORMATION)alloca(sizeof(USB_NODE_INFORMATION));

  // Iterate over the ports to discover what is connected to each one
  for (PortIndex = 1; PortIndex < (ULONG)PortCount + 1; PortIndex++)
  {
	LanguageID = 0; // Reset for each port
	ConnectionInformation.ConnectionIndex = PortIndex;

	//Get Connection's Node Informations
	Success = DeviceIoControl(HubHandle, IOCTL_USB_GET_NODE_CONNECTION_INFORMATION, &ConnectionInformation,
		 sizeof(ConnectionInformation), &ConnectionInformation, sizeof(ConnectionInformation), &BytesReturned, NULL);

	PortStatus = ConnectionInformation.ConnectionStatus[0]; // Save some typing!

	str1= "";

	// Now the connection specific information
	if (PortStatus == DeviceConnected)
	{
	  //Have a device or a hub connected to this port
	  if (!ConnectionInformation.DeviceIsHub)
	  {
		USB_Device_Information.devicePath="";

        // There is an I/O device connected. Print out it's descriptors
        // Note that many current devices do not respond correctly if ConfigID != 0.
        // So only request the first configuration
        driverKeyName = GetDriverKeyName(HubHandle, PortIndex);
        if (driverKeyName)
        {
		   DriverNameToDeviceDescInfo(driverKeyName);

		   if (USB_Device_Information.deviceDesc != NULL)
           {
			 str1=str1 + USB_Device_Information.deviceDesc;
           }
		}
		//vecUSBDevDesc.push_back(str1);
		//vecUSBDevDesc.push_back("Bus Speed : "+GetSpeed(HubHandle,PortIndex));
		LanguageID = DisplayDeviceDescriptor(LanguageID,
					 &ConnectionInformation.DeviceDescriptor.bLength
					 ,GetSpeed(HubHandle,PortIndex),false);

	  }
	  else
      {
		//Have no a device or a hub connected to this port
		// There is a hub connected and we need to iterate over it's ports

        // Get the system name of the connected hub so that we can make a connection to it
		ConnectedHub.ConnectionIndex = PortIndex;

        driverKeyName = GetDriverKeyName(HubHandle, PortIndex);
        if (driverKeyName)
        {
           DriverNameToDeviceDescInfo(driverKeyName);

           if (USB_Device_Information.deviceDesc != NULL)
			 str1=str1 + USB_Device_Information.deviceDesc;
		}
		//vecUSBDevDesc.push_back(str1);
		//try to connect nest node
		Success = DeviceIoControl(HubHandle, IOCTL_USB_GET_NODE_CONNECTION_NAME, &ConnectedHub,
				sizeof(ConnectedHub), &ConnectedHub, sizeof(ConnectedHub), &BytesReturned, NULL);
		//vecUSBDevDesc.push_back("Bus Speed : "+GetSpeed(HubHandle,PortIndex));
		LanguageID = DisplayDeviceDescriptor(LanguageID,
					 &ConnectionInformation.DeviceDescriptor.bLength
					 ,GetSpeed(HubHandle,PortIndex),true);

		if (Success)
		{
		  WideCharToMultiByte(CP_ACP, 0, &ConnectedHub.Name[0], (ConnectedHub.ActualLength)/2, &ConnectedHubName[4], 252, NULL, NULL);

		  ConnectedHubHandle = CreateFileA(ConnectedHubName, GENERIC_WRITE, FILE_SHARE_WRITE, &SA, OPEN_EXISTING, 0, NULL);

		  if(ConnectedHubHandle != INVALID_HANDLE_VALUE)
		  {
            // Root hub is open. Collect the node information
			Success = DeviceIoControl(ConnectedHubHandle, IOCTL_USB_GET_NODE_INFORMATION, info->HubInfo,
                    sizeof(USB_NODE_INFORMATION), info->HubInfo, sizeof(USB_NODE_INFORMATION), &BytesReturned, NULL);

			if (Success)
			{
				GetPortData(ConnectedHubHandle, info->HubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts, HubDepth+1);
			}
		  }
		  CloseHandle(ConnectedHubHandle);
		}
      };
    };
  };
}
//---------------------------------------------------------------------------
AnsiString USBDevConnectionInfo::GetSpeed (HANDLE HubHandle,DWORD PortIndex)
{
	bool Success;
	PUSB_NODE_CONNECTION_INFORMATION_EX_V2 connectionInfoExV2;
	PUSB_NODE_CONNECTION_INFORMATION_EX	 connectionInfoEx;
	connectionInfoEx 	 = (PUSB_NODE_CONNECTION_INFORMATION_EX)
							alloca(sizeof(USB_NODE_CONNECTION_INFORMATION_EX));
	connectionInfoExV2 = (PUSB_NODE_CONNECTION_INFORMATION_EX_V2)
							alloca(sizeof(USB_NODE_CONNECTION_INFORMATION_EX_V2));

	ULONG nBytes = 0;
	connectionInfoEx->ConnectionIndex	  = PortIndex;
	connectionInfoExV2->ConnectionIndex   = PortIndex;
	connectionInfoExV2->Length = sizeof(USB_NODE_CONNECTION_INFORMATION_EX_V2);
	connectionInfoExV2->SupportedUsbProtocols.Usb300 = 1;

	//Get Connection's Node Informations
	Success = DeviceIoControl(HubHandle,
							  IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
							  connectionInfoEx,
							  sizeof(USB_NODE_CONNECTION_INFORMATION_EX),
							  connectionInfoEx,
							  sizeof(USB_NODE_CONNECTION_INFORMATION_EX),
							  &nBytes,
							  NULL);
	Success = DeviceIoControl(HubHandle,
							  IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX_V2,
							  connectionInfoExV2,
							  sizeof(USB_NODE_CONNECTION_INFORMATION_EX_V2),
							  connectionInfoExV2,
							  sizeof(USB_NODE_CONNECTION_INFORMATION_EX_V2),
							  &nBytes,
							  NULL);
	if (!Success || nBytes < sizeof(USB_NODE_CONNECTION_INFORMATION_EX_V2))
	{
		free(connectionInfoExV2);
		connectionInfoExV2 = NULL;
	}
	AnsiString SSpeed = "";
	switch(connectionInfoEx->Speed)
	{
		case UsbLowSpeed:
			SSpeed = "LowSpeed";
			break;
		case UsbFullSpeed:
			SSpeed = "FullSpeed";
			break;
		case UsbHighSpeed:
			SSpeed = "Hi-Speed";
			if (connectionInfoExV2 != NULL
                && (connectionInfoExV2->Flags.DeviceIsOperatingAtSuperSpeedOrHigher ||
					connectionInfoExV2->Flags.DeviceIsOperatingAtSuperSpeedPlusOrHigher))
			{
				SSpeed = "SuperSpeed";
			}
			break;
		case UsbSuperSpeed:
			SSpeed = "SuperSpeed";
			break;
	}
	if(SSpeed == "SuperSpeed")
	{
		if(connectionInfoExV2->Flags.DeviceIsOperatingAtSuperSpeedPlusOrHigher)
		{
			SSpeed = "SuperSpeedPlus";
		}
	}
	return SSpeed;
}
USHORT USBDevConnectionInfo::DisplayDeviceDescriptor(USHORT LanguageID, PUCHAR BufferPtr,AnsiString BusSpeed,bool bHUB)
{
  AnsiString strVendor,str,strTemp;
  USHORT tmp;
  UCHAR LowByte;
  char *buf;

  strTemp = bHUB ? "[HUB]":"[USB]";

  if(BusSpeed == "SuperSpeedPlus")
	BusSpeed = "[GEN2]";
  else	if(BusSpeed == "SuperSpeed")
	BusSpeed = "[USB3]";
  else
	BusSpeed = "[USB2]";

  BufferPtr--; // Backup pointer to prepare for pre-increment
  for(int i =0;i<8;i++)
	*++BufferPtr;
  LowByte = *++BufferPtr;

  tmp=LowByte + (*++BufferPtr << 8);
  str=IntToHex(tmp,4);
  strTemp += BusSpeed+"VID_"+str;

  LowByte = *++BufferPtr;
  strTemp += "&PID_"+IntToHex(LowByte + (*++BufferPtr << 8),4);

  LowByte = *++BufferPtr;
  strTemp += "&REV_"+IntToHex(LowByte + (*++BufferPtr << 8),4);

  strTemp += "(" + LookingString(USB_Device_Information.DeviceID,"\\") + ")";

  if(bInit)
	vecInitUSBDevDesc.push_back(strTemp);
  else
  {
	bool bSame = false;
	for(DWORD j = 0 ;j < vecInitUSBDevDesc.size();j++)
	{
		if(strTemp == vecInitUSBDevDesc[j])
		{
			bSame = true;
			break;
		}
	}
	if(!bSame)
	{
		vecUSBDevDesc.push_back(strTemp);
    }
	//
  }
  return LanguageID;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void USBDevConnectionInfo::DriverNameToDeviceDescInfo(PCHAR DriverName)
{
	CHAR buf[BUFSIZE]; // XXXXX How big does this have to be? Dynamically size it?

	DEVINST     devInst;
	DEVINSTID_A   pDeviceID;
    DEVINST     devInstNext;
    CONFIGRET   cr;
	ULONG       walkDone = 0;
	ULONG       len;
    GUID        GuidPtr;

	//Symbolic Name for all USB device interface
    GuidPtr.Data1=2782707472;
    GuidPtr.Data2=25904;
    GuidPtr.Data3 = 4562;
    GuidPtr.Data4[0] = 144;
    GuidPtr.Data4[1] = 31;
    GuidPtr.Data4[2] = 0;
    GuidPtr.Data4[3] = 192;
    GuidPtr.Data4[4] = 79;
    GuidPtr.Data4[5] = 185;
	GuidPtr.Data4[6] = 81;
    GuidPtr.Data4[7] = 237;

    USB_Device_Information.deviceDesc=NULL;
    USB_Device_Information.localInfo=NULL;
    USB_Device_Information.DeviceID=NULL;

    // Get Root DevNode
    cr = CM_Locate_DevNode(&devInst, NULL, 0);

    if (cr != CR_SUCCESS)
      USB_Device_Information.deviceDesc=NULL;

    // Do a depth first search for the DevNode with a matching
    // DriverName value
    while (!walkDone)
    {
        // Get the DriverName value
        len = sizeof(buf);
        cr = CM_Get_DevNode_Registry_Property(devInst, CM_DRP_DRIVER, NULL, buf, &len, 0);

        // If the DriverName value matches, return the DeviceDescription
        if (cr == CR_SUCCESS && strcmp(DriverName, buf) == 0)
        {
            //Location Information
            len = sizeof(buf);
            cr = CM_Get_DevNode_Registry_Property(devInst, CM_DRP_LOCATION_INFORMATION, NULL, buf, &len, 0);

            if (cr == CR_SUCCESS)
            {
			  USB_Device_Information.localInfo=buf;
            }

            //Device Decsription
            len = sizeof(buf);
            cr = CM_Get_DevNode_Registry_Property(devInst, CM_DRP_DEVICEDESC, NULL, buf, &len, 0);

            if (cr == CR_SUCCESS)
              USB_Device_Information.deviceDesc=buf;

            //Device ID
            len = sizeof(buf);
			cr= CM_Get_Device_IDA(devInst, buf, len, 0);

            if (cr == CR_SUCCESS)
            {
              USB_Device_Information.DeviceID=buf;
              pDeviceID=buf;
            }

            //Device Interface Path
            len = sizeof(buf);
			cr =  CM_Get_Device_Interface_ListA(&GuidPtr, pDeviceID, buf, len, 0);

			if (cr == CR_SUCCESS)
              USB_Device_Information.devicePath=buf;

        }

        // This DevNode didn't match, go down a level to the first child.
        cr = CM_Get_Child(&devInstNext, devInst, 0);

        if (cr == CR_SUCCESS)
        {
          devInst = devInstNext;
          continue;
        }

        // Can't go down any further, go across to the next sibling.  If
        // there are no more siblings, go back up until there is a sibling.
        // If we can't go up any further, we're back at the root and we're
        // done.
        for (;;)
        {
            cr = CM_Get_Sibling(&devInstNext, devInst, 0);

            if (cr == CR_SUCCESS)
            {
              devInst = devInstNext;
              break;
            }

            cr = CM_Get_Parent(&devInstNext, devInst, 0);


            if (cr == CR_SUCCESS)
                devInst = devInstNext;
            else
            {
                walkDone = 1;
                break;
            }
        }
    }

}
//---------------------------------------------------------------------------
PCHAR USBDevConnectionInfo::WideStrToMultiStr (PWCHAR WideStr)
{
    ULONG nBytes;
    PCHAR MultiStr;

    // Get the length of the converted string
    //
    nBytes = WideCharToMultiByte(CP_ACP, 0, WideStr, -1, NULL, 0, NULL, NULL);

    if (nBytes == 0)
      return NULL;

    // Allocate space to hold the converted string
    MultiStr =(PCHAR )alloca(nBytes);

    if (MultiStr == NULL)
      return NULL;

    // Convert the string
    //
    nBytes = WideCharToMultiByte(CP_ACP, 0, WideStr, -1, MultiStr, nBytes, NULL, NULL);

    if (nBytes == 0)
    {
        delete(MultiStr);
        return NULL;
    }

    return MultiStr;
}
//---------------------------------------------------------------------------
PCHAR USBDevConnectionInfo::GetDriverKeyName (HANDLE Hub, ULONG ConnectionIndex)
{
    BOOL                                success;
    ULONG                               nBytes;
	USB_NODE_CONNECTION_DRIVERKEY_NAME  driverKeyName;
	PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW;

	driverKeyNameW = NULL;

    // Get the length of the name of the driver key of the device attached to
    // the specified port.
    driverKeyName.ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub, IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME, &driverKeyName,
         sizeof(driverKeyName), &driverKeyName, sizeof(driverKeyName), &nBytes, NULL);

    if (!success)
      return NULL;

    // Allocate space to hold the driver key name
	nBytes = driverKeyName.ActualLength;

    if (nBytes <= sizeof(driverKeyName))
      return NULL;

	driverKeyNameW = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME) alloca(nBytes);

    if (driverKeyNameW == NULL)
      return NULL;
    // Get the name of the driver key of the device attached to
	// the specified port.

	driverKeyNameW->ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub, IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
              driverKeyNameW, nBytes, driverKeyNameW, nBytes, &nBytes, NULL);

    if (success)
	{
	  // All done, free the uncoverted driver key name and return the
	  // converted driver key name
      return WideStrToMultiStr(driverKeyNameW->DriverKeyName);
    }
    else
      return NULL;
}

//---------------------------------------------------------------------------
AnsiString USBDevConnectionInfo::LookingString(AnsiString Sour, AnsiString Dest)
{
  for(int i=0; i < Sour.Length() - Dest.Length(); i++)
  {
	if(Sour.SubString(i+1, Dest.Length())==Dest)
	  Sour=MidStr(Sour,i+2,100);
  }
  return Sour;
}
bool USBDevConnectionInfo::RestartDev(AnsiString PVID)
{
	PVID = StringReplace(PVID, "\\", "#", TReplaceFlags()<<rfReplaceAll);
	PVID.Delete(1,4);
	PVID = PVID.LowerCase();
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i,j;
	AnsiString SS,USBPath;
	PSP_DEVICE_INTERFACE_DETAIL_DATA   pDetail   =NULL;
	GUID GUID_USB_HID =StringToGUID("{A5DCBF10-6530-11D2-901F-00C04FB951ED}");
	//DEBUG("[ 裝置列舉 ]");
	//--------------------------------------------------------------------------
	//   獲取設備資訊
	hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_USB_HID,
	0,   //   Enumerator
	0,
	DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
	if   (hDevInfo   ==   INVALID_HANDLE_VALUE){
		//DEBUG("ERROR - SetupDiGetClassDevs()"); //   查詢資訊失敗
	}
	else
	{
	//--------------------------------------------------------------------------
		SP_DEVICE_INTERFACE_DATA   ifdata;
		SP_DEVINFO_DATA            spdd;
		DeviceInfoData.cbSize   =   sizeof(SP_DEVINFO_DATA);
		for (i=0;SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData);i++)	//   枚舉每個USB設備
		{
			//或許設備的狀態
			DWORD  Status, Problem;
			if (CM_Get_DevNode_Status(&Status, &Problem, DeviceInfoData.DevInst,0) != CR_SUCCESS)
			continue;
			// 設備名
			//
			ULONG   len;
			CONFIGRET cr;
			PNP_VETO_TYPE   pnpvietotype;
			CHAR   vetoname[MAX_PATH];
			ULONG   ulStatus;
			ULONG   ulProblemNumber;
			ifdata.cbSize   =   sizeof(ifdata);
			if (SetupDiEnumDeviceInterfaces(								//   枚舉符合該GUID的設備介面
			hDevInfo,           //   設備資訊集控制碼
			NULL,                         //   不需額外的設備描述
			(LPGUID)&GUID_USB_HID,//GUID_CLASS_USB_DEVICE,                     //   GUID
			(ULONG)i,       //   設備資訊集裏的設備序號
			&ifdata))                 //   設備介面資訊
			{
				ULONG predictedLength   =   0;
				ULONG requiredLength   =   0;
				//   取得該設備介面的細節(設備路徑)
				SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,          //   設備介面資訊
					NULL,             //   設備介面細節(設備路徑)
					0,         	      //   輸出緩衝區大小
					&requiredLength,  //   不需計算輸出緩衝區大小(直接用設定值)
					NULL);            //   不需額外的設備描述
				//   取得該設備介面的細節(設備路徑)
				predictedLength=requiredLength;
				pDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT,   predictedLength);
				pDetail->cbSize   =   sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
				//
				ZeroMemory(&spdd, sizeof(spdd));
				spdd.cbSize = sizeof(spdd);

				if(SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,             //   設備介面資訊
					pDetail,             //   設備介面細節(設備路徑)
					predictedLength,     //   輸出緩衝區大小
					&requiredLength,     //   不需計算輸出緩衝區大小(直接用設定值)
					&spdd))               //   不需額外的設備描述
				{
					try
					{
						char ch[512];
						for(j=0;j<predictedLength;j++)
							ch[j]=*(pDetail->DevicePath+8+j);
						SS=ch;
						//DEBUG(SS);
						if(SS.Pos(PVID))
						{
							if(ControlDevice(i,hDevInfo))
							{
								return true;
							}
							else
							{
								GlobalFree(pDetail);
								SetupDiDestroyDeviceInfoList(hDevInfo);
								return false;
								//DEBUG("FAILED\n");
							}
                        }

					}
					catch(...)
					{
						//DEBUG(SS+"列舉失敗");
					}

				}
				else
				{
					//DEBUG("SetupDiGetInterfaceDeviceDetail F");
				}
				GlobalFree(pDetail);
			}
			else
			{
				//DEBUG("SetupDiEnumDeviceInterfaces F");
			}
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return false;
}
bool USBDevConnectionInfo::StateChange( DWORD dwNewState, DWORD dwDevID, HDEVINFO hDevInfo)
{
	SP_PROPCHANGE_PARAMS PropChangeParams;
    SP_DEVINFO_DATA        DevInfoData = {sizeof(SP_DEVINFO_DATA)};
	SP_DEVINSTALL_PARAMS devParams;

	//查詢設備信息
	if (!SetupDiEnumDeviceInfo( hDevInfo, dwDevID, &DevInfoData))
    {
		//DEBUG("SetupDiEnumDeviceInfo FAILED");
		return FALSE;
	}

	//改變設備屬性變化參數
	PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
	PropChangeParams.Scope = DICS_FLAG_GLOBAL; //使修改的屬性保存在指定的屬性文件
	PropChangeParams.StateChange = dwNewState;
	PropChangeParams.HwProfile = 0;


	//改變設備屬性
	if (!SetupDiSetClassInstallParams( hDevInfo,
                                        &DevInfoData,
                                        (SP_CLASSINSTALL_HEADER *)&PropChangeParams,
                                        sizeof(PropChangeParams)))
	{
		//DEBUG("SetupDiSetClassInstallParams FAILED");
		return false;
    }


    PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
	PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
	PropChangeParams.Scope = DICS_FLAG_CONFIGSPECIFIC;//使修改的屬性保存在指定的屬性文件
	PropChangeParams.StateChange = dwNewState;
    PropChangeParams.HwProfile = 0;

	//改變設備屬性並調用安裝服務
	if (!SetupDiSetClassInstallParams( hDevInfo,
									   &DevInfoData,
									   (SP_CLASSINSTALL_HEADER *)&PropChangeParams,
									   sizeof(PropChangeParams)))
	{
		//DEBUG(GetLastError());
		//DEBUG("SetupDiSetClassInstallParams or SetupDiCallClassInstaller FAILED");
		return false;
	}
	else
	{
		SetupDiChangeState(hDevInfo, &DevInfoData);
	}
	return true;
}
bool USBDevConnectionInfo::ControlDevice(DWORD dwDevID, HDEVINFO hDevInfo)
{
	if(!StateChange(DICS_DISABLE,dwDevID,hDevInfo))
		return false;
	if(!StateChange(DICS_ENABLE,dwDevID,hDevInfo))
		return false;
	return true;
}
bool USBDevConnectionInfo::DevNameGetVPID(AnsiString VID)
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i,j;
	AnsiString SS,USBPath;
	PSP_DEVICE_INTERFACE_DETAIL_DATA   pDetail   =NULL;
	GUID GUID_USB_HID =StringToGUID("{A5DCBF10-6530-11D2-901F-00C04FB951ED}");
	//DEBUG("[ 裝置列舉 ]");
	//--------------------------------------------------------------------------
	//   獲取設備資訊
	hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_USB_HID,
	0,   //   Enumerator
	0,
	DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
	if   (hDevInfo   ==   INVALID_HANDLE_VALUE){
		//DEBUG("ERROR - SetupDiGetClassDevs()"); //   查詢資訊失敗
	}
	else{
	//--------------------------------------------------------------------------
		SP_DEVICE_INTERFACE_DATA   ifdata;
		SP_DEVINFO_DATA            spdd;
		DeviceInfoData.cbSize   =   sizeof(SP_DEVINFO_DATA);
		for (i=0;SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData);i++)	//   枚舉每個USB設備
		{
			ifdata.cbSize   =   sizeof(ifdata);
			if (SetupDiEnumDeviceInterfaces(								//   枚舉符合該GUID的設備介面
			hDevInfo,           //   設備資訊集控制碼
			NULL,                         //   不需額外的設備描述
			(LPGUID)&GUID_USB_HID,//GUID_CLASS_USB_DEVICE,                     //   GUID
			(ULONG)i,       //   設備資訊集裏的設備序號
			&ifdata))                 //   設備介面資訊
			{
				ULONG predictedLength   =   0;
				ULONG requiredLength   =   0;
				//   取得該設備介面的細節(設備路徑)
				SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,          //   設備介面資訊
					NULL,             //   設備介面細節(設備路徑)
					0,         	      //   輸出緩衝區大小
					&requiredLength,  //   不需計算輸出緩衝區大小(直接用設定值)
					NULL);            //   不需額外的設備描述
				//   取得該設備介面的細節(設備路徑)
				predictedLength=requiredLength;
				pDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT,   predictedLength);
				pDetail->cbSize   =   sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
				//
				ZeroMemory(&spdd, sizeof(spdd));
				spdd.cbSize = sizeof(spdd);

				if(SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,             //   設備介面資訊
					pDetail,             //   設備介面細節(設備路徑)
					predictedLength,     //   輸出緩衝區大小
					&requiredLength,     //   不需計算輸出緩衝區大小(直接用設定值)
					&spdd))               //   不需額外的設備描述
				{
					try
					{
						char ch[512];
						for(j=0;j<predictedLength;j++)
							ch[j]=*(pDetail->DevicePath+8+j);
						SS=ch;
						//DEBUG(SS);
						if(SS.Pos(VID))
						{
							if(SS.Pos("vid_0bda")||SS.Pos("vid_056e"))
							{
								SS = SS.SubString(1,SS.Pos("{")-2);
								SS = SS.SubString(1,17).UpperCase()+"\\"+SS.SubString(19,SS.Length()-18);
								if(CheckReltekName(SS))
								{
									//USBHIDForm->g_DevVPID = SS.SubString(1,17);
									GlobalFree(pDetail);
									SetupDiDestroyDeviceInfoList(hDevInfo);
									return true;
								}
							}
						}

					}
					catch(...)
					{
						//DEBUG(SS+"列舉失敗");
					}

				}
				else
				{
					//DEBUG("SetupDiGetInterfaceDeviceDetail F");
				}
				GlobalFree(pDetail);
			}
			else
			{
				//DEBUG("SetupDiEnumDeviceInterfaces F");
			}
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return false;
}
bool USBDevConnectionInfo::CheckReltekName(AnsiString USBPath)
{
	AnsiString SS;
	bool bl = false;
	AnsiString DevInfo="";

	HKEY hSoftKey = NULL;

	hSoftKey = NULL;
	AnsiString temppath = "system\\CurrentControlSet\\Enum\\USB\\"+USBPath;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, temppath.c_str(), 0, KEY_READ,&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dwType=REG_SZ;
		DWORD dwCount=0;
		LONG lResult = RegQueryValueExA(hSoftKey, "FriendlyName", NULL, &dwType,NULL, &dwCount);
		if (lResult == ERROR_SUCCESS)
		{
			LPBYTE company_Get=new BYTE [dwCount];
			lResult = RegQueryValueExA(hSoftKey, "FriendlyName", NULL, &dwType,
			company_Get, &dwCount);
			if(lResult == ERROR_SUCCESS)
			{
				SS = AnsiString((char*)company_Get);
				if(SS.UpperCase().Pos("REALTEK USB") || SS.UpperCase().Pos("EDC-G01"))
					bl = true;
			}

			delete company_Get;
		}
	}
	//
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	return bl;
}
//--------------------------------------------------------------------------------------------
void USBDevConnectionInfo::GetUSBDisk()
{
	AnsiString astrTemp;
	char lpBuffer[MAX_PATH];
	char*lpSingleDevice;
	_USB_DiskDesc rectDiskDesc;
	vecDiskDesc.clear();

	DWORD nBufferLength = MAX_PATH;
	DWORD dwSignleDeviceBufferIndex =0, dwDeviceNum =0;

	lpSingleDevice  = (char*)malloc(sizeof(char)* MAX_PATH);
	if(lpSingleDevice == NULL)
	{
		//系統內存分配錯誤
		return;
	}
	memset(lpSingleDevice,0x0,MAX_PATH);
	memset(lpBuffer,0x0,MAX_PATH);

	nBufferLength = GetLogicalDriveStringsA(nBufferLength,lpBuffer); //列舉

	for(DWORD i =0; i< nBufferLength; i++)
	{
		if(lpBuffer[i] != 0x00)       //分離各個盤符
		{
		lpSingleDevice[dwSignleDeviceBufferIndex] = lpBuffer[i];
		dwSignleDeviceBufferIndex++;
		continue;
		}
		lpSingleDevice[dwSignleDeviceBufferIndex]  = 0x0;
		dwSignleDeviceBufferIndex = 0;
		if(*lpSingleDevice == 'A')  //A不需要
			continue;
		if(GetDriveTypeA(lpSingleDevice) == DRIVE_REMOVABLE)   //獲取隨身碟
		{
			rectDiskDesc.Volume = lpSingleDevice;
			rectDiskDesc.FriendlyName = CheckDISKVolumelabel(rectDiskDesc.Volume);
			rectDiskDesc.USBSpeed     = FindDiskUSBType(rectDiskDesc.Volume);
			//找出未讀寫過的
			astrTemp ="["+rectDiskDesc.Volume.SubString(1,1)+"]"
						+rectDiskDesc.FriendlyName;
			if(USBDiskString.Pos(astrTemp+"[RW3]"))
				rectDiskDesc.USB_RW = USBDISK_RW_U3_PASS;
			else if(USBDiskString.Pos(astrTemp+"[RW2]"))
				rectDiskDesc.USB_RW = USBDISK_RW_U2_PASS;
			else if(USBDiskString.Pos(astrTemp+"[RW0]"))
				rectDiskDesc.USB_RW = USBDISK_RW_FAIL;
			else
			{
				switch(DiskWriteRead(rectDiskDesc.Volume))
				{
					case 0x01:
						if(rectDiskDesc.USBSpeed == USBTYPE_SUPER_SPEED
							|| rectDiskDesc.USBSpeed == USBTYPE_SUPER_SPEED_PLUS)
							rectDiskDesc.USB_RW = USBDISK_RW_U3_PASS;
						else
							rectDiskDesc.USB_RW = USBDISK_RW_U2_PASS;
						break;
					case 0x00:
						rectDiskDesc.USB_RW = USBDISK_RW_FAIL;
						break;
				}
			}
			dwDeviceNum++;
			vecDiskDesc.push_back(rectDiskDesc);
		}
	}
	free(lpSingleDevice);
	//
	dwCountUSBDiskU2 = 0;
	dwCountUSBDiskU3 = 0;
	USBDiskString = "";
	for(DWORD i = 0 ;i < vecDiskDesc.size();i++)
	{
		USBDiskString += "["+vecDiskDesc[i].Volume.SubString(1,1)+"]"
						+vecDiskDesc[i].FriendlyName;
		switch(vecDiskDesc[i].USB_RW)
		{
			case USBDISK_RW_U3_PASS:
				USBDiskString += "[RW3]\r\n";
				dwCountUSBDiskU3++;
				break;
			case USBDISK_RW_U2_PASS:
				USBDiskString += "[RW2]\r\n";
				dwCountUSBDiskU2++;
				break;
			case USBDISK_RW_FAIL:
				USBDiskString += "[RW0]\r\n";
				break;
        }
	}
}
AnsiString USBDevConnectionInfo::CheckDISKVolumelabel(String Disk)
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
DWORD USBDevConnectionInfo::FindDiskUSBType(String Disk)
{
	AnsiString DiskVolumelabel = Disk;
	TRegistry *reg = new TRegistry();
	TStringList *ts = new TStringList;
	AnsiString DevInfo="";

	reg->RootKey = HKEY_LOCAL_MACHINE;
	AnsiString temppath = "system\\CurrentControlSet\\Services\\USBSTOR\\Enum";
	reg->OpenKey(temppath.c_str(), false);
	int DevNum = reg->ReadInteger("Count");
	for(int x=0;x<DevNum;x++)
	{
		ts->Add(reg->ReadString((AnsiString)x));
	}
	reg->CloseKey();
	for(int x=0;x<ts->Count;x++)
	{
		reg->RootKey = HKEY_LOCAL_MACHINE;
		temppath = "system\\MountedDevices";
		reg->OpenKey(temppath.c_str(), false);

		char disbuf[14];
		sprintf(disbuf,"\\DosDevices\\%s:",DiskVolumelabel.SubString(1,1));
		DWORD dwBuf = reg->GetDataSize(disbuf);
		wchar_t *bBuf=(wchar_t*)malloc(dwBuf);
		reg->ReadBinaryData(disbuf,bBuf,dwBuf);
		//String comparison
		AnsiString DevPath=ts->Strings[x].SubString(23,ts->Strings[x].Length());
		AnsiString strChildPath1 = GetUSBChildPath(DevPath);
		AnsiString strChildPath2 = GetUSBChildPath(strChildPath1);
		if(bBuf != NULL)
		{
			AnsiString strChildPath = GetUSBChildPath(DevPath);
			if(strstr(AnsiString(bBuf).c_str(),DevPath.c_str()))
			{
				DevInfo+=AnsiString(ts->Strings[x])+"("+AnsiString(disbuf).SubString(13,1)+")\r\n";
				AnsiString VID = DevInfo.SubString(DevInfo.Pos("VID_")+4,4);
				AnsiString PID = DevInfo.SubString(DevInfo.Pos("PID_")+4,4);
				free(bBuf);
				bBuf=NULL;
				reg->CloseKey();
				delete ts;
				delete reg;
				return GetUSBType("vid_"+VID+"&pid_"+PID+"#"+DevPath);
			}
			else if((AnsiString(bBuf).LowerCase().Pos(GetUSBChildPath(DevPath))))
			{
				DevInfo+=AnsiString(ts->Strings[x])+"("+AnsiString(disbuf).SubString(13,1)+")\r\n";
				AnsiString VID = DevInfo.SubString(DevInfo.Pos("VID_")+4,4);
				AnsiString PID = DevInfo.SubString(DevInfo.Pos("PID_")+4,4);
				free(bBuf);
				bBuf=NULL;
				reg->CloseKey();
				delete ts;
				delete reg;
				return GetUSBType("vid_"+VID+"&pid_"+PID+"#"+DevPath);
			}
		}
		//
		free(bBuf);
		bBuf=NULL;
	}
	if(ts->Count==0) DevInfo="NULL";
	reg->CloseKey();
	delete ts;
	delete reg;
	return USBTYPE_NOT_FIND_DEV;
}
AnsiString USBDevConnectionInfo::GetUSBChildPath(String Path)
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i,j;
	AnsiString SS,USBPath;
	PSP_DEVICE_INTERFACE_DETAIL_DATA   pDetail   =NULL;
	GUID GUID_USB =StringToGUID("{A5DCBF10-6530-11D2-901F-00C04FB951ED}");
	//--------------------------------------------------------------------------
	//   獲取設備資訊
	hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_USB,
	0,   //   Enumerator
	0,
	DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
	if   (hDevInfo   ==   INVALID_HANDLE_VALUE){
		//DEBUG("ERROR - SetupDiGetClassDevs()"); //   查詢資訊失敗
	}
	else{
	//--------------------------------------------------------------------------
		SP_DEVICE_INTERFACE_DATA   ifdata;
		SP_DEVINFO_DATA            spdd;
		DeviceInfoData.cbSize   =   sizeof(SP_DEVINFO_DATA);
		for (i=0;SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData);i++)	//   枚舉每個USB設備
		{
			ifdata.cbSize   =   sizeof(ifdata);
			if (SetupDiEnumDeviceInterfaces(								//   枚舉符合該GUID的設備介面
			hDevInfo,           //   設備資訊集控制碼
			NULL,                         //   不需額外的設備描述
			(LPGUID)&GUID_USB,//GUID_CLASS_USB_DEVICE,                     //   GUID
			(ULONG)i,       //   設備資訊集裏的設備序號
			&ifdata))                 //   設備介面資訊
			{
				ULONG predictedLength   =   0;
				ULONG requiredLength   =   0;
				//   取得該設備介面的細節(設備路徑)
				SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,          //   設備介面資訊
					NULL,             //   設備介面細節(設備路徑)
					0,         	      //   輸出緩衝區大小
					&requiredLength,  //   不需計算輸出緩衝區大小(直接用設定值)
					NULL);            //   不需額外的設備描述
				//   取得該設備介面的細節(設備路徑)
				predictedLength=requiredLength;
				pDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT,   predictedLength);
				pDetail->cbSize   =   sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
				ZeroMemory(&spdd, sizeof(spdd));
				spdd.cbSize = sizeof(spdd);

				if(SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,             //   設備介面資訊
					pDetail,             //   設備介面細節(設備路徑)
					predictedLength,     //   輸出緩衝區大小
					&requiredLength,     //   不需計算輸出緩衝區大小(直接用設定值)
					&spdd))               //   不需額外的設備描述
				{
					try
					{
						char   ch[512];
						for(j=0;j<predictedLength;j++)
						{
						ch[j]=*(pDetail->DevicePath+8+j);
						}
						SS="\\\\?\\USB#"+AnsiString(ch);
						SS=ch;
						SS = SS.SubString(1,SS.Pos("{")-2);
						if(SS.Pos("vid") && SS.Pos(Path))
						{
							DEVINST DevInstParent = 0;
							char szBuff[512] = {0};
							if(CM_Get_Child(&DevInstParent,spdd.DevInst,0) == CR_SUCCESS)
							{
								CM_Get_Device_IDA(DevInstParent, szBuff, 512, 0);
								SS = AnsiString(szBuff);
								SS.Delete(1,SS.Pos("\\"));
								SS.Delete(1,SS.Pos("\\"));
								GlobalFree(pDetail);
								SetupDiDestroyDeviceInfoList(hDevInfo);
								return SS.LowerCase();
							}
						}
						//DEBUG(SS);
					}
					catch(...)
					{
						//DEBUG("列舉失敗");
					}
				}
				GlobalFree(pDetail);
			}
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return "";
}
char  USBDevConnectionInfo::DiskWriteRead(String Disk)
{
	try
	{
	memset(WRITE_BUFFER,(BYTE)0xA5,BUFFER_SIZE);
	AnsiString FileName= Disk;
						  //Filename = 新磁碟的時間檔案
	FileName+=":\\"+FormatDateTime("yymmddhhmmss",Now())+".bin";
	FILE *fp;                       //檔案指標
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
String USBDevConnectionInfo::GetUSBDevString(bool bInit)
{
	String SS = "";
	if(bInit)
	{
		for(DWORD i = 0 ;i < vecInitUSBDevDesc.size();i++)
			SS += vecInitUSBDevDesc[i]+"\r\n";
		return SS;
	}
	else
		return USBDevString;
}
String USBDevConnectionInfo::GetDiskString()
{
	return USBDiskString;
}
DWORD USBDevConnectionInfo::CheckUSBMonitorStatus()
{
	/*DWORD dwResult = 0;
	EnmuUSBView();
	GetUSBDisk();
	if(USBDevString.Pos("[X]"))
		dwResult += WP_USB_STATUS_DEV_FAIL;
	if(dwCountUSBDiskU3 < ComparisonUSBDiskU3
		||dwCountUSBDiskU2 < ComparisonUSBDiskU2)
		dwResult += WP_USB_STATUS_DISK_FAIL;
	if(dwResult == 0)
		return WP_USB_STATUS_PASS;
	if(!USBDevString.Pos("[O]") && dwCountUSBDiskU3 == 0 && dwCountUSBDiskU2  == 0)
		return WP_USB_STATUS_NO_DEV;
	return dwResult;*/
}
void USBDevConnectionInfo::SetINIValue(AnsiString USBDevString,DWORD dwU3_Count,DWORD dwU2_Count)
{
	astrUSBDevComparison = USBDevString;
	ComparisonUSBDiskU3  = dwU3_Count;
	ComparisonUSBDiskU2  = dwU2_Count;
}
