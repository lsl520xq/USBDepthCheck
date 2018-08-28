// USBDepthCheck.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "USBDepthCheck.h"

USBDEPTHCHECK_API int USBDepthCheck(const int magic, const char *CellName, const char *ReadPath, PFCallbackUSBRecord usbFuction)
{
	CFuncs::Init();
	bool bRet=false;
	if(VL_MAGIC_NUMBER != magic)
	{
		CFuncs::WriteLogInfo(SLT_ERROR, "USBDepthCheck 传入magic的值错误");
		return RVT_MAGIC;
	}
	CUsbCheck *pUsbCheck=new CUsbCheck();
	bRet = pUsbCheck->GetUsbDepthData(CellName, ReadPath, usbFuction);
	if(!bRet)
	{ 
		CFuncs::WriteLogInfo(SLT_ERROR, "USBDepthCheck GetUsbDepthData获取USB数据失败");
		delete pUsbCheck;
		pUsbCheck = NULL;
		return RVT_FALSE;
	}
	delete pUsbCheck;
	pUsbCheck = NULL;
	return RVT_TRUE;
}
