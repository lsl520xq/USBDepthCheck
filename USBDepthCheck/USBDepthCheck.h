// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� USBDEPTHCHECK_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// USBDEPTHCHECK_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef USBDEPTHCHECK_EXPORTS
#define USBDEPTHCHECK_API EXTERN_C __declspec(dllexport)
#else
#define USBDEPTHCHECK_API EXTERN_C __declspec(dllimport)
#endif

#include "UsbCheck.h"
#include "../Common/Funcs.h"

#define VL_MAGIC_NUMBER (0x23E72DAC)


USBDEPTHCHECK_API int USBDepthCheck(const int magic, const char *CellName, const char *ReadPath, PFCallbackUSBRecord usbFuction);
