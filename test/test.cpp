// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include "../Common/Common.h"
#define  CMAX_PROTO_DESCRIPTION_LEN 512
typedef struct _USBRecovery_
{
	unsigned char        USBFirstFloorKey[CMAX_PROTO_DESCRIPTION_LEN];  
	unsigned char		 USBSecondFloorKey[CMAX_PROTO_DESCRIPTION_LEN];
	unsigned char        Service[CMAX_PROTO_DESCRIPTION_LEN];                 
	unsigned char        CompatibleIDs[CMAX_PROTO_DESCRIPTION_LEN];                      
	unsigned char        LocationInformation[CMAX_PROTO_DESCRIPTION_LEN]; 
	unsigned char        DeviceDesc[CMAX_PROTO_DESCRIPTION_LEN]; 
	unsigned char        USBFirstFloorKeyCreatime[CMAX_PROTO_DESCRIPTION_LEN];   //USBFirstFloorKey 创建时间 （尽量取到）             
}USBRecovery, *LUSBRecovery;

typedef struct _USBSTORRecovery_
{
	unsigned char        USBSTORFirstFloorKey[CMAX_PROTO_DESCRIPTION_LEN];  
	unsigned char		 USBSTORSecondFloorKey[CMAX_PROTO_DESCRIPTION_LEN];             
	unsigned char        Service[CMAX_PROTO_DESCRIPTION_LEN];                 
	unsigned char        CompatibleIDs[CMAX_PROTO_DESCRIPTION_LEN];                     
	unsigned char        FriendlyName[CMAX_PROTO_DESCRIPTION_LEN];  
	unsigned char        DeviceDesc[CMAX_PROTO_DESCRIPTION_LEN]; 
	unsigned char        USBSTORFirstFloorKeyCreateTime[CMAX_PROTO_DESCRIPTION_LEN];  //USBSTORFirstFloorKey 创建时间 （尽量取到）
}USBSTORRecovery, *LUSBSTORRecovery;
typedef std::vector<USBRecovery> USBRecoveryVector;
typedef USBRecoveryVector::iterator USBRecoveryVectorIterator;

typedef std::vector<USBSTORRecovery> USBSTORRecoveryVector;
typedef USBSTORRecoveryVector::iterator USBSTORRecoveryVectorIterator;

//typedef bool (*tPFCallbackUSBData)(USBRecoveryVector UsbData,USBSTORRecoveryVector UsbStorData);
typedef bool (*PFCallbackUSBRecord)(int checkType, const char* checkPath, const char* deviceType, const char* deviceName,
	const char* szSN, const char* szVid, const char* szPid, const char* firstDate, const char* lastDate,
	const char* szDeduction);
//typedef bool (*tPFCallbackUSBData)(LUSBRecovery UsbData,LUSBSTORRecovery UsbStorData);
typedef bool (*PFfnUSBDepthCheck)(const int magic,const char *CellName, const char *ReadPath, PFCallbackUSBRecord usbdataf);

#define VL_MAGIC_NUMBER (0x23E72DAC)

bool usbfc(	int checkType, const char* checkPath, const char* deviceType, const char* deviceName,
	const char* szSN, const char* szVid, const char* szPid, const char* firstDate, const char* lastDate,
	const char* szDeduction)
{
	CFuncs::WriteLogInfo(SLT_INFORMATION, "checkType =  %d, checkPath = %s, deviceType = %s,  deviceName = %s,\
										  szSN = %s, szVid = %s, szPid = %s, firstDate = %s, lastDate = %s, szDeduction = %s",
									 checkType,  checkPath,  deviceType,  deviceName, szSN, szVid,  szPid,  firstDate,  lastDate, szDeduction );
	return true;
}
//bool usbfc(USBRecoveryVector UsbData,USBSTORRecoveryVector UsbStorData)
//{
//	
//	if (UsbData.size() > 0)
//	{
//
//		for (DWORD usbNum = 0; usbNum < UsbData.size(); usbNum ++)
//		{
//			
//			/*		CFuncs::WriteLogInfo(SLT_INFORMATION, "\nUSBFirstFloorKey:%s\nUSBSecondFloorKey:%s\nService:%s\nCompatibleIDs:%s\nLocationInformation:%s\nDeviceDesc:%s\nUSBFirstFloorKeyCreatime:%s\n",UsbData[usbNum].USBFirstFloorKey, UsbData[usbNum].USBSecondFloorKey, UsbData[usbNum].Service
//			, UsbData[usbNum].CompatibleIDs, UsbData[usbNum].LocationInformation, UsbData[usbNum].DeviceDesc, UsbData[usbNum].USBFirstFloorKeyCreatime);*/
//
//		}
//		
//	}
//	else if (UsbStorData.size() > 0)
//	{
//		//USBSTORRecoveryVectorIterator UsbStoriter;
//		for (DWORD usbNum = 0; usbNum < UsbStorData.size(); usbNum ++)
//		{
//			CFuncs::WriteLogInfo(SLT_INFORMATION, "\nUSBSTORFirstFloorKey:%s\nUSBSTORSecondFloorKey:%s\nService:%s\nCompatibleIDs:%s\nDeviceDesc:%s\nFriendlyName:%s\nUSBSTORFirstFloorKeyCreateTime:%s\n",UsbStorData[usbNum].USBSTORFirstFloorKey, UsbStorData[usbNum].USBSTORSecondFloorKey, UsbStorData[usbNum].Service
//				, UsbStorData[usbNum].CompatibleIDs, UsbStorData[usbNum].FriendlyName, UsbStorData[usbNum].DeviceDesc, UsbStorData[usbNum].USBSTORFirstFloorKeyCreateTime);
//		}
//
//	}
//	return true;
//}

int _tmain(int argc, _TCHAR* argv[])
{
	HINSTANCE m_usbchek = ::LoadLibrary("USBDepthCheck.dll");
	if (NULL == m_usbchek)
	{
		printf("错误:%d\n",GetLastError());
	}
	PFfnUSBDepthCheck usbchek=(PFfnUSBDepthCheck)::GetProcAddress(m_usbchek,"USBDepthCheck");
	if (NULL == usbchek)
	{
		printf("错误:%d\n",GetLastError());
	}
	int Imagic = VL_MAGIC_NUMBER;
	/*char *cell = "Windows NT";
	char *path = "File:d:\\d\\SOFTWARE";*/
	char *cell = "USBSTOR;USB";
	char *path = "Disk:\\\\.\\c:";

	if(!usbchek(Imagic, cell, path, &usbfc))
	{
		printf("失败\n");
	}
	getchar();
	return 0;
}

