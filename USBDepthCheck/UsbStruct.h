#include <vector>
#include <string>

#pragma pack(1)
#define MAX_PROTO_USBRECORD_LEN 512
typedef struct _HIVE_HEAD
{
	DWORD   _Signature;
	DWORD   _Sequence1;
	DWORD   _Sequence2;
	LARGE_INTEGER _TimeStamp;
	DWORD   _Major;
	DWORD   _Minor;
	DWORD   _Type;
	DWORD   _Format;
	DWORD   _RootCell;
	DWORD   _Length;
	DWORD   _Cluster;
	UCHAR   _Name[64];
	DWORD   _Reserve[99];
	DWORD   _CheckSum;
	DWORD   _Reserve2[894];
	DWORD   _BootType;
	DWORD   _BootRecover;
}HIVE_HEAD,*LHIVE_HEAD;
typedef struct _HIVE_HBIN_HEAD
{
	DWORD   _Signature;
	DWORD   _FileOffset;
	DWORD   _SIZE;
	DWORD   _Reserved[2];
	LARGE_INTEGER _TimeStamp;
	DWORD   _Spare;
}HIVE_HBIN_HEAD,*LHIVE_HBIN_HEAD;
typedef struct _HIVE_nk_head
{
	DWORD   _Begin_Rserved;
	UCHAR   _Signature[2];
	UCHAR   _Flags[2];
	LARGE_INTEGER _LastWriteTime;
	DWORD   _Spare;
	DWORD   _Parent;//父键的偏移
	DWORD   _SubKeyCounts[2];//子键的个数
	union
	{
		struct
		{
			DWORD _SubKeyListOffset[2];
			DWORD VK_Count;
			DWORD VK_ListOffset;
		};
		DWORD  _ChildHiveRefernce[4];
	};
	DWORD   _Security;//安全描述记录的偏移
	DWORD   _Class;//类名的偏移
	DWORD   _MaxNameLen:16;
	DWORD   _UserFlags:4;
	DWORD   _VirtControlFlags:4;
	DWORD   _Debug:8;
	DWORD   _MaxClassLen;
	DWORD   _MaxValueNameLen;
	DWORD   _MaxValueDataLen;
	DWORD   _WorkVar;
	UCHAR   _NameLength[2];//键名长度
	UCHAR   _ClassLength[2];//类名长度
}HIVE_nk_head,*LHIVE_nk_head;
typedef struct _HIVE_vk_head
{
	DWORD   _Begin_Rserved;
	UCHAR   _Signature[2];
	UCHAR   _NameLength[2];//名称长度
	DWORD   _DataLength;//数据长度
	DWORD   _Data;//数据偏移或数据，如果DataLength最高位为1，那么它就是数据，且DataLength&0x7fffffff为数据长度
	DWORD   _Type;//值类型
	UCHAR   _Flags[2];
	UCHAR   _Spare[2];

}HIVE_vk_head,*LHIVE_vk_head;
typedef struct _USBRecovery_
{
	unsigned char        USBFirstFloorKey[MAX_PROTO_USBRECORD_LEN];  
	unsigned char		 USBSecondFloorKey[MAX_PROTO_USBRECORD_LEN];
	unsigned char        Service[MAX_PROTO_USBRECORD_LEN];                 
	unsigned char        CompatibleIDs[MAX_PROTO_USBRECORD_LEN];                      
	unsigned char        LocationInformation[MAX_PROTO_USBRECORD_LEN]; 
	unsigned char        DeviceDesc[MAX_PROTO_USBRECORD_LEN]; 
	unsigned char        USBFirstFloorKeyCreatime[MAX_PROTO_USBRECORD_LEN];   //USBFirstFloorKey 创建时间 （尽量取到）             
}USBRecovery, *LUSBRecovery;

typedef struct _USBSTORRecovery_
{
	unsigned char        USBSTORFirstFloorKey[MAX_PROTO_USBRECORD_LEN];  
	unsigned char		 USBSTORSecondFloorKey[MAX_PROTO_USBRECORD_LEN];             
	unsigned char        Service[MAX_PROTO_USBRECORD_LEN];                 
	unsigned char        CompatibleIDs[MAX_PROTO_USBRECORD_LEN];                     
	unsigned char        FriendlyName[MAX_PROTO_USBRECORD_LEN];  
	unsigned char        DeviceDesc[MAX_PROTO_USBRECORD_LEN]; 
	unsigned char        USBSTORFirstFloorKeyCreateTime[MAX_PROTO_USBRECORD_LEN];  //USBSTORFirstFloorKey 创建时间 （尽量取到）
}USBSTORRecovery, *LUSBSTORRecovery;

typedef std::vector<USBRecovery> USBRecoveryVector;
typedef USBRecoveryVector::iterator USBRecoveryVectorIterator;

typedef std::vector<USBSTORRecovery> USBSTORRecoveryVector;
typedef USBSTORRecoveryVector::iterator USBSTORRecoveryVectorIterator;