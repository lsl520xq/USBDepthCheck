#pragma once

#include "../Common/Funcs.h"
#include "../Include/DataHeader.h"
#include "UsbStruct.h"
#include <windows.h>
#include <winioctl.h>
#include <vector>
#define  SECTOR_SIZE 512
#define  HIVE_SECTOR_SIZE 4096

typedef bool (*PFCallbackUSBData)(USBRecoveryVector UsbData,USBSTORRecoveryVector UsbStorData);

typedef bool (*PFCallbackUSBRecord)(int checkType, const char* checkPath, const char* deviceType, const char* deviceName,
	const char* szSN, const char* szVid, const char* szPid, const char* firstDate, const char* lastDate,
	const char* szDeduction);
//typedef bool (*PFCallbackUSBData)(LUSBRecovery UsbData,LUSBSTORRecovery UsbStorData);
class CUsbCheck
{
private:
	bool  ReadSQData(HANDLE hDevice, UCHAR* Buffer, DWORD SIZE, DWORD64 addr, DWORD *BackBytesCount);

	bool  AnalysisHbinInfo(HANDLE Hdrive,UCHAR *HbinBuff,DWORD HbinSize,const char *CellName,vector<DWORD> CellNameLen, vector<DWORD> &NkAddr,int *FoundCell,DWORD64 HbinSector
		, DWORD HbinFileStartAddr);

	bool UnicodeToZifu(UCHAR* Source_Unico, string& fileList, DWORD Size);

	bool GetHbinNkDataInfo(HANDLE Hdrive, DWORD hbinFileAddr, DWORD64 HbinSector, LUSBRecovery UsbData, DWORD *UsbFuctionCounts
		, LUSBSTORRecovery UsbStorData, int usbJudge, DWORD usbnkAddr);

	bool GetNkListAddr(HANDLE Hdrive,DWORD filestarAddr, DWORD NkstarAddr, DWORD NkNumber, DWORD64 HbinSector, vector<DWORD> &NkAddr);

	bool GetVkDataInfo(HANDLE Hdrive, DWORD64 NKaddr, LUSBRecovery UsbData, LUSBSTORRecovery UsbStorData, DWORD hbinFileAddr, DWORD64 HbinSector
		, int usbJudge, DWORD *UsbFuctionCounts, DWORD AddrOffset);

	bool GetVkListAddr(HANDLE Hdrive, DWORD vkNumber, DWORD64 vkstartAddr, vector<DWORD> &vkaddr, DWORD VKAddrOffset);

	bool GetVkListDataInfo(HANDLE Hdrive, LUSBRecovery UsbData, LUSBSTORRecovery UsbStorData, DWORD64 vkAddr, int usbJudge, DWORD VKlistAddrOffset
		, DWORD hbinFileAddr, DWORD64 HbinSector);

	bool SectorAddrChange(DWORD64 *Out_Addr, DWORD Data_Offset, DWORD File_Offset, DWORD64 StarAddr, DWORD *Out_OffsetAddr);

	bool GetUsbRecord(USBRecoveryVector v_UsbData, USBSTORRecoveryVector v_UsbStorData, PFCallbackUSBRecord UsbDataFuction);	

	bool ParseUsbRecord(const char* USBFirstFloorKey, const char* USBSecondFloorKey, const char* Service, const char* CompatibleIDs, 
		const char* LocationInformation, const char* DeviceDesc, const char* USBFirstFloorKeyCreatime, PFCallbackUSBRecord UsbDataFuction);	

	bool GetUsbType(const vstring& compatibleIDs, vstring& usbType);

	bool GetVidAndPid(vstring& strVidAndPid, vstring& vid, vstring& pid);

	bool GetHbinNkHostInfo(HANDLE Hdrive, DWORD hbinFileAddr, DWORD64 HbinSector, DWORD *UsbFuctionCounts
		,  int usbJudge, DWORD usbnkAddr, string &OperatingInfo, string &RootPath, string &InstallTime, string &productid, string &Username);

	bool GetHostVkDataInfo(HANDLE Hdrive, DWORD64 NKaddr, DWORD hbinFileAddr, DWORD64 HbinSector, int usbJudge, DWORD *UsbFuctionCounts
		, DWORD AddrOffset, string &OperatingInfo, string &RootPath, string &InstallTime, string &productid, string &Username);

	bool GetVkListHostInfo(HANDLE Hdrive, DWORD64 vkAddr, int usbJudge, DWORD VKlistAddrOffset, DWORD hbinFileAddr, DWORD64 HbinSector
		, string &OperatingInfo, string &RootPath, string &InstallTime, string &productid, string &Username, string NkName);

	bool  FileTimeConver(UCHAR* szFileTime, string& strTime);
	
public:
	CUsbCheck(void);
	~CUsbCheck(void);
public:

	//bool  GetUsbDepthData(PFCallbackUSBData UsbDataFuction,const char *CellName, const char *ReadPath);
	bool  GetUsbDepthData(const char *CellName, const char *ReadPath, PFCallbackUSBRecord UsbDataFuction);
};

