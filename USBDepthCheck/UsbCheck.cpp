#include "StdAfx.h"
#include "UsbCheck.h"


CUsbCheck::CUsbCheck(void)
{
}

CUsbCheck::~CUsbCheck(void)
{
}
bool  CUsbCheck::FileTimeConver(UCHAR* szFileTime, string& strTime)
{
	if(NULL == szFileTime)
	{
		return false;
	}
	LPFILETIME pfileTime = (LPFILETIME)szFileTime;
	SYSTEMTIME systemTime = {0};
	BOOL bTime = FileTimeToSystemTime(pfileTime, &systemTime);
	if (bTime)
	{
		char szTime[32] = { 0 };
		sprintf_s(szTime, _countof(szTime), "%04d-%02d-%02d %02d:%02d:%02d", systemTime.wYear, systemTime.wMonth,
			systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
		strTime.assign(szTime);
		return true;
	}
	return false;
}
bool CUsbCheck::UnicodeToZifu(UCHAR* Source_Unico, string& fileList, DWORD Size)
{
	if (Size > 4)
	{
		Size -= 4;
	}
	else
	{
		CFuncs::WriteLogInfo(SLT_ERROR, "UnicodeToZifu::Size小于4失败!");
		return false;
	}
	
	wchar_t *toasiclls = new wchar_t[(Size/2+1)];
	if (NULL == toasiclls)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, "UnicodeToZifu::new::分配toasiclls内存失败!");
		return false;
	}

	memset(toasiclls, 0, (Size/2+1)  * 2);

	char *str = (char*)malloc(Size+2);
	if (NULL == str)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, "UnicodeToZifu::malloc::分配str内存失败!");
		delete toasiclls;
		toasiclls = NULL;
		return false;
	}
	memset(str,0,Size+2);
	
	for (DWORD nl = 0; nl < Size; nl += 2)
	{
		toasiclls[nl/2] = Source_Unico[nl+1] << 8 | Source_Unico[nl];
	}
	
	int nRet=WideCharToMultiByte(CP_OEMCP, 0, toasiclls, Size/2, str, Size, NULL, NULL); 
	if(nRet<=0)  
	{  
		CFuncs::WriteLogInfo(SLT_ERROR, "Unicode_To_Zifu::WideCharToMultiByte::转换失败失败!");
		free(str);
		str = NULL;
		delete toasiclls;
		toasiclls = NULL;
		return false;
	}  
	else  
	{  
			
		fileList.append(str, (Size/2));
		

	}   

	free(str);
	str = NULL;
	delete toasiclls;
	toasiclls = NULL;
	return true;
}
bool  CUsbCheck::ReadSQData(HANDLE hDevice, UCHAR* Buffer, DWORD SIZE, DWORD64 addr, DWORD *BackBytesCount)
{
	LARGE_INTEGER LiAddr = {0};	
	LiAddr.QuadPart = addr;
	DWORD dwError = 0;

	BOOL bRet = SetFilePointerEx(hDevice, LiAddr, NULL, FILE_BEGIN);
	if(!bRet)
	{
		dwError = GetLastError();
		CFuncs::WriteLogInfo(SLT_ERROR, "ReadSQData::SetFilePointerEx失败!, 错误返回码: dwError = %d", dwError);
		return false;	
	}
	bRet = ReadFile(hDevice, Buffer, SIZE, BackBytesCount, NULL);
	if(!bRet)
	{
		//printf("SIZE %lu\n",SIZE);
		//printf("addr %llu\n",addr);
		dwError = GetLastError();
		CFuncs::WriteLogInfo(SLT_ERROR, "ReadSQData::ReadFile失败!, 错误返回码: dwError = %d", dwError);					
		return false;	
	}

	return true;
}
bool CUsbCheck::GetNkListAddr(HANDLE Hdrive,DWORD filestarAddr, DWORD NkstarAddr, DWORD NkNumber, DWORD64 HbinSector, vector<DWORD> &NkAddr)
{
	bool Ret = false;
	DWORD BeyteNumber = NULL;
	DWORD ReadnkSize = NULL;
	if (NkNumber > 0xffff)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetNkListAddr:NkNumber > 0xffff 失败!"));
		return false;
	}
	if ((NkNumber * 8) > SECTOR_SIZE)
	{
		ReadnkSize = 1 + (NkNumber * 8)/SECTOR_SIZE;
	}
	else
	{
		ReadnkSize = 1;
	}
	UCHAR *lhBuffer = (UCHAR*)malloc(ReadnkSize * SECTOR_SIZE + SECTOR_SIZE + 1);
	if (NULL == lhBuffer)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetNkListAddr:malloc:lhBuffer 失败!"));
		return false;
	}
	memset(lhBuffer, 0, (ReadnkSize * SECTOR_SIZE + SECTOR_SIZE + 1));
	DWORD64 ReadDataAddr = NULL;
	DWORD AddrOffset = NULL;
	if (!SectorAddrChange(&ReadDataAddr, NkstarAddr, filestarAddr, HbinSector, &AddrOffset))
	{
		free(lhBuffer);
		lhBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetNkListAddr:SectorAddrChange 失败!"));
		return true;
	}
	
	//ReadDataAddr = (HbinSector * SECTOR_SIZE) + (NkstarAddr - filestarAddr);
	Ret = ReadSQData(Hdrive, lhBuffer, (ReadnkSize * SECTOR_SIZE + SECTOR_SIZE), ReadDataAddr , &BeyteNumber);
	if (!Ret)
	{		
		free(lhBuffer);
		lhBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetNkListAddr:ReadSQData:lhBuffer失败!"));
		return false;
	}
	if (lhBuffer[4 + AddrOffset] != 0x6c && lhBuffer[5 + AddrOffset] != 0x68)
	{
		free(lhBuffer);
		lhBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_INFORMATION, _T("GetNkListAddr:标志不是lh失败!"));
		return true;
	}
	DWORD *lhListAddr = NULL;
	lhListAddr = (DWORD*)&lhBuffer[8 + AddrOffset];//跳过lh标志位，取地址段
	for (DWORD listNum = NULL; listNum < (NkNumber * 2); listNum += 2)//Nk地址是由地址加校验组成的8字节
	{
		NkAddr.push_back(lhListAddr[listNum]);
	}

	free(lhBuffer);
	lhBuffer = NULL;

	return true;
}
bool  CUsbCheck::AnalysisHbinInfo(HANDLE Hdrive, UCHAR *HbinBuff, DWORD HbinSize, const char *CellName, vector<DWORD> CellNameLen, vector<DWORD> &NkAddr
	,int *FoundCell, DWORD64 HbinSector, DWORD HbinFileStartAddr)
{
	LHIVE_nk_head nk_head = NULL;
	LHIVE_vk_head vk_head = NULL;
	DWORD namesize = 0;
	DWORD datasize = 0;

	for (DWORD offset = 32; offset < HbinSize; offset += 8)
	{
		 if (HbinBuff[offset+1] == 0xff && HbinBuff[offset+2] == 0xff && HbinBuff[offset+3] == 0xff
			&& HbinBuff[offset+4] == 0x6e && HbinBuff[offset+5] == 0x6b)  //判断nk
		{
			nk_head = (LHIVE_nk_head)&HbinBuff[offset];
			namesize = 0;
			namesize = namesize | (nk_head->_NameLength[1] << 8);
			namesize = namesize | nk_head->_NameLength[0];
			DWORD CellNameIndex = NULL;
			for (DWORD NameNum = 0; NameNum < CellNameLen.size(); NameNum++)
			{
				for(DWORD fNameLen = 0; fNameLen < CellNameLen[NameNum]; fNameLen++)
				{
					if (HbinBuff[offset+80+fNameLen] == CellName[fNameLen + CellNameIndex])
					{
						(*FoundCell) = 3;
					}
					else
					{
						(*FoundCell) = -1;
						break;
					}
				}
				if ((*FoundCell) > 0)
				{
					if (namesize == CellNameLen[NameNum])
					{
						CFuncs::WriteLogInfo(SLT_INFORMATION, _T("此扇区是%lu"), HbinSector);
						DWORD NkNumber = NULL;
						DWORD NkStartAddr = NULL;
						NkNumber = nk_head->_SubKeyCounts[0];
						if (NkNumber > 0)
						{
							NkStartAddr = nk_head->_SubKeyListOffset[0];
							if (NULL == NkStartAddr )
							{
								(*FoundCell) = -1;
							}
							else
							{
								if (namesize == 3)//USB标记为1
								{
									(*FoundCell) = 1;
								}
								else if (namesize == 7)//USBSTOR标记为2
								{
									(*FoundCell) = 2;
								}
								else if (namesize == 10)
								{
									(*FoundCell) = 4;//WindowsNT
								}
								if (!GetNkListAddr(Hdrive, HbinFileStartAddr, NkStartAddr, NkNumber, HbinSector, NkAddr))
								{
									CFuncs::WriteLogInfo(SLT_ERROR, _T("AnalysisHbinInfo:GetNkListAddr失败!"));
									return false;
								}
							}
						}else
						{
							(*FoundCell) = -1;
						}
						
						return true;
					}
					else
					{
						(*FoundCell) = -1;
					}
					
				}
				if (CellName[CellNameLen[NameNum] + CellNameIndex] == ';')
				{
					CellNameIndex += (CellNameLen[NameNum] + 1);
				}				
			}	
			if (namesize % 8 != 0)
			{
				namesize = namesize + (8 - namesize % 8);
			}
			offset = offset + 80 + namesize - 8;
		}
	}
	return true;

}

bool CUsbCheck::GetVkListAddr(HANDLE Hdrive, DWORD vkNumber, DWORD64 vkstartAddr, vector<DWORD> &vkaddr, DWORD VKAddrOffset)
{
	bool Ret = false;
	DWORD BeyteNumber = NULL;
	DWORD ReadSector = NULL;
	if (vkNumber > 0xffff)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListAddr:(vkNumber > 0xffff)失败!"));
		return false;
	}
	if ((vkNumber * 4 + 4) > SECTOR_SIZE)
	{
		ReadSector = (vkNumber * 4 + 4)/SECTOR_SIZE + 2;
	}
	else
	{
		ReadSector = 2;
	}
	UCHAR *VKBuffer = (UCHAR*)malloc(ReadSector * SECTOR_SIZE); 
	if (NULL == VKBuffer)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:malloc:VKBuffer失败!"));
		return false;
	}
	memset(VKBuffer, 0, (ReadSector * SECTOR_SIZE));
	Ret = ReadSQData(Hdrive, VKBuffer, (ReadSector * SECTOR_SIZE), vkstartAddr , &BeyteNumber);
	if (!Ret)
	{		
		free(VKBuffer);
		VKBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListAddr:ReadSQData:VKBuffer失败!"));
		return false;
	}
	if (VKBuffer[1 + VKAddrOffset] != 0xff && VKBuffer[2 + VKAddrOffset] !=  0xff && VKBuffer[3 + VKAddrOffset] !=  0xff)
	{
		free(VKBuffer);
		VKBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_INFORMATION, _T("GetVkListAddr:ReadSQData:VKBuffer !=  0xff!"));
		return true;
	} 
	DWORD *vklistAddr = NULL;
	vklistAddr = (DWORD*)&VKBuffer[4 + VKAddrOffset];
	for (DWORD listNum = NULL; listNum < vkNumber; listNum ++)
	{
		vkaddr.push_back(vklistAddr[listNum]);
	}

	free(VKBuffer);
	VKBuffer = NULL;

	return true;
}
bool CUsbCheck::GetVkListHostInfo(HANDLE Hdrive, DWORD64 vkAddr, int usbJudge, DWORD VKlistAddrOffset, DWORD hbinFileAddr, DWORD64 HbinSector
	, string &OperatingInfo, string &RootPath, string &InstallTime, string &productid, string &Username, string NkName)
{
	bool Ret = false;
	DWORD BeyteNumber = NULL;
	DWORD NameSize = NULL;
	LHIVE_vk_head Vk_Head = NULL;



	UCHAR *vk_Buff = (UCHAR*)malloc(HIVE_SECTOR_SIZE + 1);
	if (NULL == vk_Buff)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListHostInfo:malloc:vk_Buff失败!"));
		return false;
	}
	memset(vk_Buff, 0, (HIVE_SECTOR_SIZE + 1));

	Ret = ReadSQData(Hdrive, vk_Buff, HIVE_SECTOR_SIZE, vkAddr , &BeyteNumber);
	if (!Ret)
	{		
		free(vk_Buff);
		vk_Buff = NULL;
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListHostInfo:ReadSQData:vk_Buff失败!"));
		return false;
	}
	if(vk_Buff[1 + VKlistAddrOffset] == 0xff && vk_Buff[2 + VKlistAddrOffset] == 0xff && vk_Buff[3 + VKlistAddrOffset] == 0xff
		&& vk_Buff[4 + VKlistAddrOffset] == 0x76 && vk_Buff[5 + VKlistAddrOffset] == 0x6b)//判断vk
	{
		Vk_Head=(LHIVE_vk_head)&vk_Buff[0 + VKlistAddrOffset];
		NameSize=NameSize | (Vk_Head->_NameLength[1] << 8);
		NameSize=NameSize | Vk_Head->_NameLength[0];

		string VkName;
		if (NameSize < 4000)
		{
			
			VkName.append((char*)&vk_Buff[24 + VKlistAddrOffset],NameSize);
		}
		else
		{
			free(vk_Buff);
			vk_Buff = NULL;
			CFuncs::WriteLogInfo(SLT_INFORMATION, _T("GetVkListHostInfo:NameSize > 4000返回!"));
			return true;
		}
		string Vk_Data;
		if (Vk_Head->_DataLength > 4096)
		{
			Vk_Data.append((char*)&vk_Buff[12 + VKlistAddrOffset], 4);

		}
		else
		{
			DWORD vkdataaddr = Vk_Head->_Data;
			DWORD vkdatalen = Vk_Head->_DataLength;
			if (NameSize%8!=0)
			{
				NameSize=NameSize + (8-NameSize%8);
			}
			DWORD VKDataOffset = NULL;
			DWORD64 VKDataReadDataAddr = NULL;
			memset(vk_Buff, 0, (HIVE_SECTOR_SIZE + 1));
			if (!SectorAddrChange(&VKDataReadDataAddr, vkdataaddr , hbinFileAddr, HbinSector, &VKDataOffset))
			{
				free(vk_Buff);
				vk_Buff = NULL;
				CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListHostInfo:SectorAddrChange失败!"));
				return true;
			}
			Ret = ReadSQData(Hdrive, vk_Buff, HIVE_SECTOR_SIZE, VKDataReadDataAddr , &BeyteNumber);
			if (!Ret)
			{		
				free(vk_Buff);
				vk_Buff = NULL;
				CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListHostInfo:ReadSQData:vk_Buff失败!"));
				return false;
			}
			if(!UnicodeToZifu(&vk_Buff[VKDataOffset + 4],Vk_Data, vkdatalen))
			{
				free(vk_Buff);
				vk_Buff = NULL;
				CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListHostInfo:UnicodeToZifu失败!"));
				return false;
			}
		}
		 if (usbJudge == 4)
		{
			if (NkName == "CurrentVersion")
			{
				if (VkName == "ProductName")
				{
					OperatingInfo.append(Vk_Data);				
				}
				else if (VkName == "SystemRoot")
				{
					RootPath.append(Vk_Data);
				}
				else if (VkName == "ProductId")
				{
					productid.append(Vk_Data);
				}
				else if (VkName == "InstallTime")
				{
					if (Vk_Data.length() == 8)
					{

						if (!FileTimeConver((UCHAR*)&Vk_Data[0], InstallTime))
						{
							CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListHostInfo:FileTimeConver失败!"));
						}
					}
				}

			}
			

			if (NkName == "Winlogon")
			{
				if (VkName == "DefaultUserName")
				{
					Username.append(Vk_Data);
				}
			}

		}
		
		

	}



	free(vk_Buff);
	vk_Buff = NULL;
	return true;
}
bool CUsbCheck::GetVkListDataInfo(HANDLE Hdrive, LUSBRecovery UsbData, LUSBSTORRecovery UsbStorData, DWORD64 vkAddr, int usbJudge, DWORD VKlistAddrOffset
	, DWORD hbinFileAddr, DWORD64 HbinSector)
{
	bool Ret = false;
	DWORD BeyteNumber = NULL;
	DWORD NameSize = NULL;
	LHIVE_vk_head Vk_Head = NULL;



	UCHAR *vk_Buff = (UCHAR*)malloc(HIVE_SECTOR_SIZE + 1);
	if (NULL == vk_Buff)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListDataInfo:malloc:vk_Buff失败!"));
		return false;
	}
	memset(vk_Buff, 0, (HIVE_SECTOR_SIZE + 1));

	Ret = ReadSQData(Hdrive, vk_Buff, HIVE_SECTOR_SIZE, vkAddr , &BeyteNumber);
	if (!Ret)
	{		
		free(vk_Buff);
		vk_Buff = NULL;
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListDataInfo:ReadSQData:vk_Buff失败!"));
		return false;
	}
	if(vk_Buff[1 + VKlistAddrOffset] == 0xff && vk_Buff[2 + VKlistAddrOffset] == 0xff && vk_Buff[3 + VKlistAddrOffset] == 0xff
		&& vk_Buff[4 + VKlistAddrOffset] == 0x76 && vk_Buff[5 + VKlistAddrOffset] == 0x6b)//判断vk
	{
		Vk_Head=(LHIVE_vk_head)&vk_Buff[0 + VKlistAddrOffset];
		NameSize=NameSize | (Vk_Head->_NameLength[1] << 8);
		NameSize=NameSize | Vk_Head->_NameLength[0];

	
		string VkName;
		if (NameSize < 4000)
		{
			
			VkName.append((char*)&vk_Buff[24 + VKlistAddrOffset],NameSize);
		}
		else
		{
			free(vk_Buff);
			vk_Buff = NULL;
			CFuncs::WriteLogInfo(SLT_INFORMATION, _T("GetVkListDataInfoNameSize > 4000返回!"));
			return true;
		}
		string Vk_Data;
		if (Vk_Head->_DataLength > 4096)
		{
			Vk_Data.append((char*)&vk_Buff[12 + VKlistAddrOffset], 4);

		}
		else
		{
			DWORD vkdataaddr = Vk_Head->_Data;
			DWORD vkdatalen = Vk_Head->_DataLength;
			if (NameSize%8!=0)
			{
				NameSize=NameSize + (8-NameSize%8);
			}
			DWORD VKDataOffset = NULL;
			DWORD64 VKDataReadDataAddr = NULL;
			memset(vk_Buff, 0, (HIVE_SECTOR_SIZE + 1));
			if (!SectorAddrChange(&VKDataReadDataAddr, vkdataaddr , hbinFileAddr, HbinSector, &VKDataOffset))
			{
				free(vk_Buff);
				vk_Buff = NULL;
				CFuncs::WriteLogInfo(SLT_ERROR, _T("GetHbinNkDataInfo:SectorAddrChange失败!"));
				return true;
			}
			Ret = ReadSQData(Hdrive, vk_Buff, HIVE_SECTOR_SIZE, VKDataReadDataAddr , &BeyteNumber);
			if (!Ret)
			{		
				free(vk_Buff);
				vk_Buff = NULL;
				CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkListDataInfo:ReadSQData:vk_Buff失败!"));
				return false;
			}
			if(!UnicodeToZifu(&vk_Buff[VKDataOffset + 4],Vk_Data, vkdatalen))
			{
				free(vk_Buff);
				vk_Buff = NULL;
				CFuncs::WriteLogInfo(SLT_ERROR, _T("GetHbinVkAndNkDataL:UnicodeToZifu失败!"));
				return false;
			}
		}
		if (usbJudge == 1)//为USB
		{
			if (VkName == "Service")
			{
				RtlCopyMemory(UsbData->Service, &Vk_Data[0], Vk_Data.length());
			}
			else if (VkName == "CompatibleIDs")
			{
				RtlCopyMemory(UsbData->CompatibleIDs, &Vk_Data[0], Vk_Data.length());
			}
			else if (VkName == "LocationInformation")
			{
				RtlCopyMemory(UsbData->LocationInformation, &Vk_Data[0], Vk_Data.length());
			}
			else if (VkName == "DeviceDesc")
			{
				RtlCopyMemory(UsbData->DeviceDesc, &Vk_Data[0], Vk_Data.length());
			}
			else if (VkName == "USBFirstFloorKeyCreatime")
			{
				RtlCopyMemory(UsbData->USBFirstFloorKeyCreatime, &Vk_Data[0], Vk_Data.length());
			}
		}
		else if (usbJudge == 2)//为USBSTOR
		{
			if (VkName == "Service")
			{
				RtlCopyMemory(UsbStorData->Service, &Vk_Data[0], Vk_Data.length());
			}
			else if (VkName == "CompatibleIDs")
			{
				RtlCopyMemory(UsbStorData->CompatibleIDs, &Vk_Data[0], Vk_Data.length());
			}
			else if (VkName == "FriendlyName")
			{
				RtlCopyMemory(UsbStorData->FriendlyName, &Vk_Data[0], Vk_Data.length());
			}
			else if (VkName == "DeviceDesc")
			{
				RtlCopyMemory(UsbStorData->DeviceDesc, &Vk_Data[0], Vk_Data.length());
			}
			else if (VkName == "USBSTORFirstFloorKeyCreateTime")
			{
				RtlCopyMemory(UsbStorData->USBSTORFirstFloorKeyCreateTime, &Vk_Data[0], Vk_Data.length());
			}
		}
		
		

	}



	free(vk_Buff);
	vk_Buff = NULL;
	return true;
}
bool CUsbCheck::SectorAddrChange(DWORD64 *Out_Addr, DWORD Data_Offset, DWORD File_Offset, DWORD64 StarAddr, DWORD *Out_OffsetAddr)
{
	if (NULL == Out_Addr || NULL == Out_OffsetAddr)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("SectorAddrChange:NULL == Out_Addr || NULL == Out_OffsetAddr"));
		return false;
	}
	if (Data_Offset > File_Offset)
	{
		if ((Data_Offset - File_Offset) > SECTOR_SIZE)
		{
			(*Out_OffsetAddr) = (Data_Offset - File_Offset) % SECTOR_SIZE;
			(*Out_Addr) = (StarAddr * SECTOR_SIZE) + ((Data_Offset - File_Offset) / SECTOR_SIZE) * SECTOR_SIZE;
		} 
		else if ((Data_Offset - File_Offset) < SECTOR_SIZE)
		{
			(*Out_Addr) = (StarAddr * SECTOR_SIZE);
			(*Out_OffsetAddr) = (Data_Offset - File_Offset);
		}
		
	} 
	else if (Data_Offset < File_Offset)
	{
		if ((File_Offset - Data_Offset) > SECTOR_SIZE)
		{
			if (((File_Offset - Data_Offset) % SECTOR_SIZE) > 0)
			{
				(*Out_Addr) = (StarAddr * SECTOR_SIZE) - ((File_Offset - Data_Offset) / SECTOR_SIZE) * SECTOR_SIZE - SECTOR_SIZE;
				(*Out_OffsetAddr) = (SECTOR_SIZE - (File_Offset - Data_Offset) % SECTOR_SIZE);
			} 
			else
			{
				(*Out_Addr) = (StarAddr * SECTOR_SIZE) - ((File_Offset - Data_Offset) / SECTOR_SIZE) * SECTOR_SIZE;
				(*Out_OffsetAddr) = 0;
			}
		}
		else if ((File_Offset - Data_Offset) < SECTOR_SIZE)
		{
			(*Out_Addr) = (StarAddr * SECTOR_SIZE) - SECTOR_SIZE;
			(*Out_OffsetAddr) = (SECTOR_SIZE - (File_Offset - Data_Offset));
		}
	}

	return true;
}
bool CUsbCheck::GetVkDataInfo(HANDLE Hdrive, DWORD64 NKaddr, LUSBRecovery UsbData, LUSBSTORRecovery UsbStorData, DWORD hbinFileAddr, DWORD64 HbinSector
	, int usbJudge, DWORD *UsbFuctionCounts, DWORD AddrOffset)
{
	bool Ret = false;
	DWORD BeyteNumber = NULL;
	LHIVE_nk_head Nk_Head = NULL;
	DWORD NameSize = NULL;
	(*UsbFuctionCounts) ++;
	if ((*UsbFuctionCounts) > 100)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:递归超过100失败!"));
		return false;
	}
	UCHAR *NKBuffer = (UCHAR*)malloc(HIVE_SECTOR_SIZE);
	if (NULL == NKBuffer)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:malloc:NKBuffer失败!"));
		return false;
	}
	memset(NKBuffer, 0, HIVE_SECTOR_SIZE);
	Ret = ReadSQData(Hdrive, NKBuffer, HIVE_SECTOR_SIZE, NKaddr , &BeyteNumber);
	if (!Ret)
	{		
		free(NKBuffer);
		NKBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:ReadSQData:NKBuffer失败!"));
		return false;
	}
	if (NKBuffer[1 + AddrOffset] != 0xff && NKBuffer[2 + AddrOffset] != 0xff && NKBuffer[3 + AddrOffset] != 0xff
		&& NKBuffer[4 + AddrOffset] != 0x6e && NKBuffer[5 + AddrOffset] != 0x6b)  //判断nk
	{
		free(NKBuffer);
		NKBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_INFORMATION, _T("GetVkDataInfo:NK标志错误!"));
		return true;
	}
	Nk_Head=(LHIVE_nk_head)&NKBuffer[0 + AddrOffset];
	//printf("%0.2x\n",Nk_Head->_NameLength[1]);
	//printf("%0.2x\n",Nk_Head->_NameLength[0]);
	NameSize=NameSize | (Nk_Head->_NameLength[1] << 8);
	NameSize=NameSize | Nk_Head->_NameLength[0];
	if (NameSize < 0)
	{
		CFuncs::WriteLogInfo(SLT_INFORMATION, _T("GetVkDataInfo:NameSize < 0!"));
		return true;
	}
	/*if (NameSize%8!=0)
	{
		NameSize=NameSize+(8-NameSize%8);
	}*/
	if ((80 + NameSize) > SECTOR_SIZE)
	{
		free(NKBuffer);
		NKBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo :(80 + NameSize) > SECTOR_SIZE失败!"));
		return true;
	}
	if ((*UsbFuctionCounts) == 1)
	{
		string NkName;
		NkName.append((char*)&NKBuffer[80 + AddrOffset],NameSize);
		if (NkName.length() > 0)
		{
			if (usbJudge == 1)
			{
				RtlCopyMemory(UsbData->USBFirstFloorKey, &NkName[0], NkName.length());
			}
			else if (usbJudge == 2)
			{
				RtlCopyMemory(UsbStorData->USBSTORFirstFloorKey, &NkName[0], NkName.length());
			}
			
		}
		
	}
	else if ((*UsbFuctionCounts) == 2)
	{
		string NkName;
		NkName.append((char*)&NKBuffer[80 + AddrOffset],NameSize);
		if (NkName.length() > 0)
		{
			if (usbJudge == 1)
			{
				RtlCopyMemory(UsbData->USBSecondFloorKey, &NkName[0], NkName.length());
			}
			else if (usbJudge == 2)
			{
				RtlCopyMemory(UsbStorData->USBSTORSecondFloorKey, &NkName[0], NkName.length());
			}
		}
		
	}
	
	
	
	if (Nk_Head->VK_Count > 0)
	{
		DWORD VkStartAddr = NULL;
		VkStartAddr = Nk_Head->VK_ListOffset;
		
		vector<DWORD> vkAddr;
		DWORD VKAddrOffset = NULL;
		DWORD64 VKReadDataAddr = NULL;

		if (!SectorAddrChange(&VKReadDataAddr, Nk_Head->VK_ListOffset, hbinFileAddr, HbinSector, &VKAddrOffset))
		{
			free(NKBuffer);
			NKBuffer = NULL;
			CFuncs::WriteLogInfo(SLT_ERROR, _T("GetHbinNkDataInfo:SectorAddrChange失败!"));
			return true;
		}
		if (!GetVkListAddr(Hdrive, Nk_Head->VK_Count, VKReadDataAddr, vkAddr, VKAddrOffset))
		{
			free(NKBuffer);
			NKBuffer = NULL;
			CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:GetVkListAddr失败!"));
			return true;
		}
		vector<DWORD>::iterator vkiter;
		for (vkiter = vkAddr.begin(); vkiter != vkAddr.end(); vkiter ++)
		{
			DWORD VKlistAddrOffset = NULL;
			DWORD64 VKlistReadDataAddr = NULL;

			if (!SectorAddrChange(&VKlistReadDataAddr, *vkiter , hbinFileAddr, HbinSector, &VKlistAddrOffset))
			{
				free(NKBuffer);
				NKBuffer = NULL;
				CFuncs::WriteLogInfo(SLT_ERROR, _T("GetHbinNkDataInfo:SectorAddrChange失败!"));
				return true;
			}
								
			if (!GetVkListDataInfo(Hdrive, UsbData, UsbStorData, VKlistReadDataAddr, usbJudge, VKlistAddrOffset, hbinFileAddr, HbinSector))
			{
				free(NKBuffer);
				NKBuffer = NULL;
				CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:GetVkListDataInfo 失败!"));
				return false;
			}
				
		}
		
	}
	else
	{
		vector<DWORD> nkchildAddr;
		DWORD NkStartAddr = NULL;
		DWORD NkNumber = NULL;
		NkStartAddr = Nk_Head->_SubKeyListOffset[0];
		NkNumber = Nk_Head->_SubKeyCounts[0];
		if (NkNumber > 0)
		{
			if (NULL != NkStartAddr)
			{
				if (!GetNkListAddr(Hdrive, hbinFileAddr, NkStartAddr, NkNumber, HbinSector, nkchildAddr))
				{
					free(NKBuffer);
					NKBuffer = NULL;
					CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:GetNkListAddr失败!"));
					return false;
				}
				if (nkchildAddr.size() > 0)
				{
					vector<DWORD>::iterator usbChildnkiter;
					for (usbChildnkiter = nkchildAddr.begin(); usbChildnkiter != nkchildAddr.end(); usbChildnkiter ++)
					{
						if(!GetHbinNkDataInfo(Hdrive, hbinFileAddr, HbinSector, UsbData, UsbFuctionCounts, UsbStorData, usbJudge, *usbChildnkiter))
						{
							free(NKBuffer);
							NKBuffer = NULL;
							CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:GetHbinNkDataInfo失败!"));
							return false;
						}
					}
				}
			}
			
		}
		
	}
	


	free(NKBuffer);
	NKBuffer = NULL;
	return true;
}
bool CUsbCheck::GetHostVkDataInfo(HANDLE Hdrive, DWORD64 NKaddr, DWORD hbinFileAddr, DWORD64 HbinSector, int usbJudge, DWORD *UsbFuctionCounts
	, DWORD AddrOffset, string &OperatingInfo, string &RootPath, string &InstallTime, string &productid, string &Username)
{
	bool Ret = false;
	DWORD BeyteNumber = NULL;
	LHIVE_nk_head Nk_Head = NULL;
	DWORD NameSize = NULL;
	(*UsbFuctionCounts) ++;
	if ((*UsbFuctionCounts) > 500)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:递归超过500失败!"));
		return false;
	}
	UCHAR *NKBuffer = (UCHAR*)malloc(HIVE_SECTOR_SIZE);
	if (NULL == NKBuffer)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:malloc:NKBuffer失败!"));
		return false;
	}
	memset(NKBuffer, 0, HIVE_SECTOR_SIZE);
	Ret = ReadSQData(Hdrive, NKBuffer, HIVE_SECTOR_SIZE, NKaddr , &BeyteNumber);
	if (!Ret)
	{		
		free(NKBuffer);
		NKBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:ReadSQData:NKBuffer失败!"));
		return false;
	}
	if (NKBuffer[1 + AddrOffset] != 0xff && NKBuffer[2 + AddrOffset] != 0xff && NKBuffer[3 + AddrOffset] != 0xff
		&& NKBuffer[4 + AddrOffset] != 0x6e && NKBuffer[5 + AddrOffset] != 0x6b)  //判断nk
	{
		free(NKBuffer);
		NKBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_INFORMATION, _T("GetVkDataInfo:NK标志错误!"));
		return true;
	}
	Nk_Head=(LHIVE_nk_head)&NKBuffer[0 + AddrOffset];

	NameSize=NameSize | (Nk_Head->_NameLength[1] << 8);
	NameSize=NameSize | Nk_Head->_NameLength[0];
	if (NameSize < 0)
	{
		CFuncs::WriteLogInfo(SLT_INFORMATION, _T("GetVkDataInfo:NameSize < 0!"));
		return true;
	}
	
	if ((80 + NameSize) > SECTOR_SIZE)
	{
		free(NKBuffer);
		NKBuffer = NULL;
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo :(80 + NameSize) > SECTOR_SIZE失败!"));
		return true;
	}
	string NkName;
	NkName.append((char*)&NKBuffer[80 + AddrOffset],NameSize);
		
	if (Nk_Head->VK_Count > 0)
	{
		DWORD VkStartAddr = NULL;
		VkStartAddr = Nk_Head->VK_ListOffset;
		
		vector<DWORD> vkAddr;
		DWORD VKAddrOffset = NULL;
		DWORD64 VKReadDataAddr = NULL;

		if (!SectorAddrChange(&VKReadDataAddr, Nk_Head->VK_ListOffset, hbinFileAddr, HbinSector, &VKAddrOffset))
		{
			free(NKBuffer);
			NKBuffer = NULL;
			CFuncs::WriteLogInfo(SLT_ERROR, _T("GetHbinNkDataInfo:SectorAddrChange失败!"));
			return true;
		}
		if (!GetVkListAddr(Hdrive, Nk_Head->VK_Count, VKReadDataAddr, vkAddr, VKAddrOffset))
		{
			free(NKBuffer);
			NKBuffer = NULL;
			CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:GetVkListAddr失败!"));
			return true;
		}
		vector<DWORD>::iterator vkiter;
		for (vkiter = vkAddr.begin(); vkiter != vkAddr.end(); vkiter ++)
		{
			DWORD VKlistAddrOffset = NULL;
			DWORD64 VKlistReadDataAddr = NULL;

			if (!SectorAddrChange(&VKlistReadDataAddr, *vkiter , hbinFileAddr, HbinSector, &VKlistAddrOffset))
			{
				free(NKBuffer);
				NKBuffer = NULL;
				CFuncs::WriteLogInfo(SLT_ERROR, _T("GetHbinNkDataInfo:SectorAddrChange失败!"));
				return true;
			}
								
			if (!GetVkListHostInfo(Hdrive, VKlistReadDataAddr, usbJudge, VKlistAddrOffset, hbinFileAddr, HbinSector, OperatingInfo, RootPath
				, InstallTime, productid, Username, NkName))
			{
				free(NKBuffer);
				NKBuffer = NULL;
				CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:GetVkListDataInfo 失败!"));
				return false;
			}
				
		}
		
	}
	if (*UsbFuctionCounts == 1)
	{
		vector<DWORD> nkchildAddr;
		DWORD NkStartAddr = NULL;
		DWORD NkNumber = NULL;
		NkStartAddr = Nk_Head->_SubKeyListOffset[0];
		NkNumber = Nk_Head->_SubKeyCounts[0];
		if (NkNumber > 0)
		{
			if (NULL != NkStartAddr)
			{
				if (!GetNkListAddr(Hdrive, hbinFileAddr, NkStartAddr, NkNumber, HbinSector, nkchildAddr))
				{
					free(NKBuffer);
					NKBuffer = NULL;
					CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:GetNkListAddr失败!"));
					return false;
				}
				if (nkchildAddr.size() > 0)
				{
					vector<DWORD>::iterator usbChildnkiter;
					for (usbChildnkiter = nkchildAddr.begin(); usbChildnkiter != nkchildAddr.end(); usbChildnkiter ++)
					{
						if(!GetHbinNkHostInfo(Hdrive, hbinFileAddr, HbinSector, UsbFuctionCounts, usbJudge, *usbChildnkiter, OperatingInfo
							, RootPath, InstallTime, productid, Username))
						{
							free(NKBuffer);
							NKBuffer = NULL;
							CFuncs::WriteLogInfo(SLT_ERROR, _T("GetVkDataInfo:GetHbinNkDataInfo失败!"));
							return false;
						}
					}
				}
			}

		}	
	}
	
		
	
	


	free(NKBuffer);
	NKBuffer = NULL;
	return true;
}
bool CUsbCheck::GetHbinNkHostInfo(HANDLE Hdrive, DWORD hbinFileAddr, DWORD64 HbinSector, DWORD *UsbFuctionCounts
	,  int usbJudge, DWORD usbnkAddr, string &OperatingInfo, string &RootPath, string &InstallTime, string &productid, string &Username)
{


	DWORD AddrOffset = NULL;
	DWORD64 ReadDataAddr = NULL;
	if (!SectorAddrChange(&ReadDataAddr, usbnkAddr , hbinFileAddr, HbinSector, &AddrOffset))
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetHbinNkHostInfo:SectorAddrChange失败!"));
		return true;
	}

	if (!GetHostVkDataInfo(Hdrive, ReadDataAddr, hbinFileAddr, HbinSector, usbJudge
		, UsbFuctionCounts, AddrOffset, OperatingInfo, RootPath, InstallTime, productid, Username))
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetHbinNkHostInfo:GetHostVkDataInfo失败!"));
		return false;

	}


	return true;
}
bool CUsbCheck::GetHbinNkDataInfo(HANDLE Hdrive, DWORD hbinFileAddr, DWORD64 HbinSector, LUSBRecovery UsbData, DWORD *UsbFuctionCounts
	, LUSBSTORRecovery UsbStorData, int usbJudge, DWORD usbnkAddr)
{
	
		
		DWORD AddrOffset = NULL;
		DWORD64 ReadDataAddr = NULL;
		if (!SectorAddrChange(&ReadDataAddr, usbnkAddr , hbinFileAddr, HbinSector, &AddrOffset))
		{
			CFuncs::WriteLogInfo(SLT_ERROR, _T("GetHbinNkDataInfo:SectorAddrChange失败!"));
			return true;
		}
		
		if (!GetVkDataInfo(Hdrive, ReadDataAddr, UsbData, UsbStorData, hbinFileAddr, HbinSector, usbJudge
			, UsbFuctionCounts, AddrOffset))
		{
			CFuncs::WriteLogInfo(SLT_ERROR, _T("GetHbinNkDataInfo:GetVkDataInfo失败!"));
			return false;

		}
	

	return true;
}
bool  CUsbCheck::GetUsbDepthData(const char *CellName, const char *ReadPath, PFCallbackUSBRecord UsbDataFuction)
{
	DWORD dwError=NULL;//获取lasterror信息
	HANDLE h_Drive=NULL;
	int  DiskOrFile = 0;
	string readpath_str;
	readpath_str = string(ReadPath);
	if (readpath_str.length() < 6)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:readpath_str.length() < 6失败!"));
		return false;
	}
	char *readpath_ch = (char*)malloc(readpath_str.length() + 1);
	if (NULL == readpath_ch)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:malloc:readpath_ch失败!"));
		return false;
	}
	memset(readpath_ch, 0 , (readpath_str.length() + 1));
	
	if (readpath_str.find("File:") != string::npos)
	{
		DiskOrFile = 2; 
		RtlCopyMemory(readpath_ch, &readpath_str[5], (readpath_str.length() - 5));
	} 
	else if(readpath_str.find("Disk:") != string::npos)
	{
		DiskOrFile = 1;
		RtlCopyMemory(readpath_ch, &readpath_str[5], (readpath_str.length() - 5));
	}
	else
	{
		free(readpath_ch);
		readpath_ch = NULL;
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:输入字符路径有错误,失败!"));
		return false;
	}
	//\\\\.\\c:
	h_Drive=CreateFile(readpath_ch,
		GENERIC_READ,
		FILE_SHARE_READ  |  
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (h_Drive == INVALID_HANDLE_VALUE) 
	{
		free(readpath_ch);
		readpath_ch = NULL;
		dwError=GetLastError();
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:CreateFile:失败!,\
										   错误返回码: dwError = %d"), dwError);
		return false;
	}
	free(readpath_ch);
	readpath_ch = NULL;
	LARGE_INTEGER SectorNumber={NULL};//存储c盘扇区总数

	if (DiskOrFile == 1)//disk
	{
		DWORD nDiskBufferSize=NULL;
		LPDWORD nDiskBytesRead=NULL;
		GET_LENGTH_INFORMATION PatitionLength;  
		BOOL  ret = DeviceIoControl(  
			h_Drive,  
			IOCTL_DISK_GET_LENGTH_INFO,  
			NULL,  
			0,  
			&PatitionLength,  
			sizeof(PatitionLength),  
			(LPDWORD)&nDiskBytesRead,  
			NULL  
			); 
		if (!ret)
		{
			dwError=GetLastError();
			CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:DeviceIoControl:失败!,\
											   错误返回码: dwError = %d"), dwError);
			return false;
		}

		SectorNumber.QuadPart = PatitionLength.Length.QuadPart / SECTOR_SIZE;
	} 
	else if(DiskOrFile == 2)//file
	{
		SectorNumber.QuadPart  = GetFileSize(h_Drive,NULL);
		SectorNumber.QuadPart = SectorNumber.QuadPart / SECTOR_SIZE;
	}
	if (SectorNumber.QuadPart < 8)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:SectorNumber.QuadPart < 8失败!"));
		return false;
	}
	

	UCHAR* HbinHeadBuffer = (UCHAR*)malloc(HIVE_SECTOR_SIZE + SECTOR_SIZE);
	if (NULL == HbinHeadBuffer)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:malloc:HbinHeadBuffer失败!"));
		return false;
	}
	memset(HbinHeadBuffer, 0, HIVE_SECTOR_SIZE + SECTOR_SIZE);

	DWORD BeyteNumber=NULL;
	bool Ret=NULL;
	LARGE_INTEGER ReadNumber={NULL};
	LHIVE_HBIN_HEAD hbin_head=NULL;
	hbin_head=(LHIVE_HBIN_HEAD)&HbinHeadBuffer[0];	
	
	vector<DWORD> CellNameLen;
	DWORD checkfist=NULL;
	for (DWORD namel=0;namel<strlen(CellName);namel++)
	{
		if (CellName[namel] == ';')
		{
			CellNameLen.push_back(namel-checkfist);
			checkfist=namel+1;
			
		}
		if (namel == (strlen(CellName)-1))
		{
			CellNameLen.push_back(namel-checkfist+1);
			
		}
	}
	int  CellFound = 0;
	DWORD64 LastSector=NULL;
	
	CFuncs::WriteLogInfo(SLT_INFORMATION, "一共%llu\n扇区",SectorNumber.QuadPart);

	USBRecoveryVector v_UsbData;
	USBSTORRecoveryVector v_UsbStorData;
	
   	for (ReadNumber.QuadPart = NULL; ReadNumber.QuadPart < (SectorNumber.QuadPart - 8); (ReadNumber.QuadPart) += 8)
	{
		//printf("第%llu\n扇区",ReadNumber.QuadPart);
		memset(HbinHeadBuffer, 0, (HIVE_SECTOR_SIZE + SECTOR_SIZE));
		Ret=ReadSQData(h_Drive,HbinHeadBuffer,HIVE_SECTOR_SIZE,ReadNumber.QuadPart * SECTOR_SIZE,&BeyteNumber);
		if (!Ret)
		{
			//printf("第%llu\n扇区",ReadNumber.QuadPart);
			CFuncs::WriteLogInfo(SLT_INFORMATION, "第%llu\n扇区",ReadNumber.QuadPart);
			//free(HbinHeadBuffer);
			//HbinHeadBuffer = NULL;
			CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:ReadSQData:RBuff失败!"));
			//return false;
		}
		else
		{
			if (HbinHeadBuffer[0] == 0x68 && HbinHeadBuffer[1] == 0x62 && HbinHeadBuffer[2] == 0x69 && HbinHeadBuffer[3] == 0x6e )
			{
				if (hbin_head->_SIZE < HIVE_SECTOR_SIZE * 5)
				{
					UCHAR *HbinBuffer = (UCHAR*)malloc(hbin_head->_SIZE + HIVE_SECTOR_SIZE);
					if (NULL == HbinBuffer)
					{
						free(HbinBuffer);
						HbinBuffer = NULL;
						CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:malloc:HbinHeadBuffer失败!"));
						return false;
					}
					memset(HbinBuffer, 0, hbin_head->_SIZE + HIVE_SECTOR_SIZE);
					CellFound = 0;
					if (hbin_head->_Signature == 0x6e696268)
					{

						Ret=ReadSQData(h_Drive,HbinBuffer, hbin_head->_SIZE, ReadNumber.QuadPart * SECTOR_SIZE, &BeyteNumber);
						if (!Ret)
						{
							free(HbinBuffer);
							HbinBuffer = NULL;
							free(HbinHeadBuffer);
							HbinHeadBuffer = NULL;
							CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:ReadSQData:HbinBuffer失败!"));
							return false;
						}
						vector<DWORD> NkListAddr;
						DWORD HbinfileAddr = NULL;
						HbinfileAddr = hbin_head->_FileOffset;
						if(!AnalysisHbinInfo(h_Drive, HbinBuffer, hbin_head->_SIZE, CellName, CellNameLen, NkListAddr, &CellFound, ReadNumber.QuadPart
							, HbinfileAddr))
						{
							free(HbinBuffer);
							HbinBuffer = NULL;
							free(HbinHeadBuffer);
							HbinHeadBuffer = NULL;
							CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:AnalysisHbinInfo:失败!"));
							return false;
						}
						free(HbinBuffer);
						HbinBuffer = NULL;
						if (LastSector != ReadNumber.QuadPart)
						{
							LastSector = ReadNumber.QuadPart;
							if (CellFound > 0 && CellFound < 4)
							{


								vector<DWORD>::iterator usbnkiter;
								for (usbnkiter = NkListAddr.begin(); usbnkiter != NkListAddr.end(); usbnkiter ++)
								{
									DWORD USBCounts = NULL;
									USBRecovery UsbData = {NULL};
									USBSTORRecovery UsbStorData = {NULL};
									if (!GetHbinNkDataInfo(h_Drive, HbinfileAddr, ReadNumber.QuadPart, &UsbData, &USBCounts, &UsbStorData, CellFound, *usbnkiter))
									{
										CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:GetHbinNkDataInfo:失败!"));
										return false;
									}
									//UsbDataFuction(&UsbData, &UsbStorData);
									if (strlen((const char*)&UsbData.CompatibleIDs) > 0 || strlen((const char*)&UsbData.Service) > 0
										|| strlen((const char*)&UsbData.LocationInformation) > 0|| strlen((const char*)&UsbData.DeviceDesc) > 0)
									{

										v_UsbData.push_back(UsbData);

									}else if (strlen((const char*)&UsbStorData.CompatibleIDs) > 0 || strlen((const char*)&UsbStorData.FriendlyName) > 0
										|| strlen((const char*)&UsbStorData.Service) > 0 || strlen((const char*)&UsbStorData.DeviceDesc) > 0)
									{
										v_UsbStorData.push_back(UsbStorData);
									}
								}	
								if (v_UsbData.size() > 0 || v_UsbStorData.size() > 0)
								{
									if(!GetUsbRecord(v_UsbData, v_UsbStorData, UsbDataFuction))
									{
										CFuncs::WriteLogInfo(SLT_ERROR, "获取深度恢复的USB记录失败");
									}
								}
								
								/*UsbDataFuction(v_UsbData, v_UsbStorData);	*/
								
								v_UsbData.clear();
								v_UsbStorData.clear();
							}
							else if(CellFound == 4)
							{
								string OperatingInfo;
								string RootPath;
								string InstallTime;
								string productid;
								string Username;
								vector<DWORD>::iterator windowsiter;
								for (windowsiter = NkListAddr.begin(); windowsiter != NkListAddr.end(); windowsiter ++)
								{
									DWORD WindowsCounts = NULL;
								
									if (!GetHbinNkHostInfo(h_Drive, HbinfileAddr, ReadNumber.QuadPart, &WindowsCounts, CellFound, *windowsiter
										, OperatingInfo, RootPath, InstallTime, productid, Username))
									{
										CFuncs::WriteLogInfo(SLT_ERROR, _T("GetUsbDepthData:GetHbinNkDataInfo:失败!"));
										return false;
									}
									
								}
								UsbDataFuction(CDS_PC, OperatingInfo.c_str(), RootPath.c_str(), InstallTime.c_str(), productid.c_str(), Username.c_str()
									, "", "", "", "");
							}
						}		
					}

				}

			}
		}
		
	}
	//UsbDataFuction(v_UsbData, v_UsbStorData);	
	/************************************************************************/	
	(void)CloseHandle(h_Drive);	
	free(HbinHeadBuffer);
	HbinHeadBuffer = NULL;

	/************************************************************************/
	return true;
}

bool CUsbCheck::GetUsbRecord(USBRecoveryVector vUsbData, USBSTORRecoveryVector vUsbStorData, PFCallbackUSBRecord UsbDataFuction)
 {
	 DWORD iUsbSize = vUsbData.size();
	 DWORD iUsbStorSize = vUsbStorData.size();
	 if (iUsbSize > 0)
	 {
		 for (DWORD usbNum = 0; usbNum < iUsbSize; usbNum ++)
		 {
			 /* CFuncs::WriteLogInfo(SLT_INFORMATION, "\nUSBFirstFloorKey:%s\nUSBSecondFloorKey:%s\nService:%s\nCompatibleIDs:%s\nLocationInformation:%s\nDeviceDesc:%s\nUSBFirstFloorKeyCreatime:%s\n",\
			 vUsbData[usbNum].USBFirstFloorKey, vUsbData[usbNum].USBSecondFloorKey, vUsbData[usbNum].Service
			 , vUsbData[usbNum].CompatibleIDs, vUsbData[usbNum].LocationInformation, vUsbData[usbNum].DeviceDesc, vUsbData[usbNum].USBFirstFloorKeyCreatime);*/
			 if(!ParseUsbRecord((char*)vUsbData[usbNum].USBFirstFloorKey, (char*)vUsbData[usbNum].USBSecondFloorKey, (char*)vUsbData[usbNum].Service
				 , (char*)vUsbData[usbNum].CompatibleIDs, (char*)vUsbData[usbNum].LocationInformation, (char*)vUsbData[usbNum].DeviceDesc, (char*)vUsbData[usbNum].USBFirstFloorKeyCreatime, UsbDataFuction))
			 {
				 
				 CFuncs::WriteLogInfo(SLT_ERROR, "GetUsbRecord调用ParseUsbRecord解析usb记录失败");
			 }
		 }
	 }
	 else if (iUsbStorSize > 0)
	 {
		 //USBSTORRecoveryVectorIterator UsbStoriter;
		 for (DWORD usbStorNum = 0; usbStorNum < iUsbStorSize; usbStorNum ++)
		 {
			 /*CFuncs::WriteLogInfo(SLT_INFORMATION, "\nUSBSTORFirstFloorKey:%s\nUSBSTORSecondFloorKey:%s\nService:%s\nCompatibleIDs:%s\nDeviceDesc:%s\nFriendlyName:%s\nUSBSTORFirstFloorKeyCreateTime:%s\n",\
			 vUsbStorData[usbStorNum].USBSTORFirstFloorKey, vUsbStorData[usbStorNum].USBSTORSecondFloorKey, vUsbStorData[usbStorNum].Service
			 , vUsbStorData[usbStorNum].CompatibleIDs, vUsbStorData[usbStorNum].FriendlyName, vUsbStorData[usbStorNum].DeviceDesc, vUsbStorData[usbStorNum].USBSTORFirstFloorKeyCreateTime);*/
			 if(!ParseUsbRecord((char*)vUsbStorData[usbStorNum].USBSTORFirstFloorKey, (char*)vUsbStorData[usbStorNum].USBSTORSecondFloorKey, (char*)vUsbStorData[usbStorNum].Service
				 , (char*)vUsbStorData[usbStorNum].CompatibleIDs, (char*)vUsbStorData[usbStorNum].FriendlyName, (char*)vUsbStorData[usbStorNum].DeviceDesc,(char*) vUsbStorData[usbStorNum].USBSTORFirstFloorKeyCreateTime, UsbDataFuction))
			 {
				 CFuncs::WriteLogInfo(SLT_ERROR, "GetUsbRecord调用ParseUsbRecord解析usbstor记录失败");
			 }
		 }

	 }
	 
	 return true;
 }

bool CUsbCheck::ParseUsbRecord(const char* USBFirstFloorKey, const char* USBSecondFloorKey, const char* Service, const char* CompatibleIDs,
	const char* LocationInformation, const char* DeviceDesc, const char* USBFirstFloorKeyCreatime, PFCallbackUSBRecord UsbDataFuction)
{
	vstring strVidAndPid;
	strVidAndPid.assign(USBFirstFloorKey);
	vstring tmpReg = CFuncs::UpperToLower(strVidAndPid);
	if (string::npos == tmpReg.find("vid"))
	{
		CFuncs::WriteLogInfo(SLT_ERROR, "ParseUsbRecord 传入的USBFirstFloorKey参数不是USB记录");
		return true;
	}
	if (/*0 != _stricmp(Service, _T("USBSTOR")) &&*/ 0 == _stricmp(Service, "USBSTOR"))
	{
		return false;
	}
	vstring strCompatibleIDs = string(CompatibleIDs);
	vstring strUsbType;
	if(!GetUsbType(strCompatibleIDs, strUsbType))
	{
		CFuncs::WriteLogInfo(SLT_ERROR, "strVidAndPid");
		return false;
	}
	vstring strPid;
	vstring strVid;
	if(!GetVidAndPid(strVidAndPid, strVid, strPid))
	{
		CFuncs::WriteLogInfo(SLT_ERROR, "ParseUsbRecord 调用strVidAndPid获取vid和pid失败");
		return  false;
	}
	UsbDataFuction(CDS_PC, "", strUsbType.c_str(), Service,  USBSecondFloorKey, strVid.c_str(), strPid.c_str(), "", "", "");
	return true;

}


bool CUsbCheck::GetVidAndPid(vstring& strVidAndPid, vstring& vid, vstring& pid)
{
	if (strVidAndPid.empty())
	{
		return false;
	}
	if (strVidAndPid.length() > 23)
	{
		return false;
	}

	size_t pos = strVidAndPid.find("&");
	if (string::npos == pos)
	{
		return false;
	}
	vid = strVidAndPid.substr(0, pos);
	pid = strVidAndPid.substr(pos + 1, strVidAndPid.length() - pos - 1);
	return true;
}


bool CUsbCheck::GetUsbType(const vstring& compatibleIDs, vstring& usbType)
{
	if (compatibleIDs.empty())
	{
		return false;
	}
	vstring  temp = CFuncs::UpperToLower(compatibleIDs);
	size_t beginPos = temp.find("class_");
	if (string::npos == beginPos)
	{
		return false;
	}
	size_t endPos = temp.find( "&", beginPos + 6);
	if (string::npos == endPos)
	{
		return false;
	}
	vstring tempID = temp.substr(beginPos + 6, endPos - beginPos - 6);
	if (0 == tempID.compare("00"))
	{
		usbType = _T("类信息描述");
	}
	else if (0 == tempID.compare("01"))
	{
		usbType = _T("音频设备");
	}
	else if (0 == tempID.compare("02"))
	{
		usbType = _T("通信及通信类控制设备");
	}
	else if (0 == tempID.compare("03"))
	{
		usbType = _T("人体工学设备");
	}
	else if (0 == tempID.compare("05"))
	{
		usbType = _T("物理设备");
	}
	else if (0 == tempID.compare("06"))
	{
		usbType = _T("图像");
	}
	else if (0 == tempID.compare("07"))
	{
		usbType = _T("打印设备");
	}
	else if (0 == tempID.compare("08"))
	{
		usbType = _T("大容量存储设备");
	}
	else if (0 == tempID.compare("09"))
	{
		usbType = _T("集线器");
	}
	else if (0 == tempID.compare("0a"))
	{
		usbType = _T("通信类数据设备");
	}
	else if (0 == tempID.compare("0b"))
	{
		usbType = _T("智能卡");
	}
	else if (0 == tempID.compare("0d"))
	{
		usbType = _T("内容安全");
	}
	else if (0 == tempID.compare("0e"))
	{
		usbType = _T("视频");
	}
	else if (0 == tempID.compare("0f"))
	{
		usbType = _T("个人医疗");
	}
	else if (0 == tempID.compare("dc"))
	{
		usbType = _T("诊断设备");
	}
	else if (0 == tempID.compare("e0"))
	{
		usbType = _T("无线控制器");
	}
	else if (0 == tempID.compare("ef"))
	{
		usbType = _T("其他设备");
	}
	else if (0 == tempID.compare("fe"))
	{
		usbType = _T("应用程序");
	}
	else if (0 == tempID.compare("ff"))
	{
		usbType = _T("厂家自定义");
	}
	else
	{
		usbType = "未知";
	}

	return true;
}