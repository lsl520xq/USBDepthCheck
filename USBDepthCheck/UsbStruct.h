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
	DWORD   _Parent;//������ƫ��
	DWORD   _SubKeyCounts[2];//�Ӽ��ĸ���
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
	DWORD   _Security;//��ȫ������¼��ƫ��
	DWORD   _Class;//������ƫ��
	DWORD   _MaxNameLen:16;
	DWORD   _UserFlags:4;
	DWORD   _VirtControlFlags:4;
	DWORD   _Debug:8;
	DWORD   _MaxClassLen;
	DWORD   _MaxValueNameLen;
	DWORD   _MaxValueDataLen;
	DWORD   _WorkVar;
	UCHAR   _NameLength[2];//��������
	UCHAR   _ClassLength[2];//��������
}HIVE_nk_head,*LHIVE_nk_head;
typedef struct _HIVE_vk_head
{
	DWORD   _Begin_Rserved;
	UCHAR   _Signature[2];
	UCHAR   _NameLength[2];//���Ƴ���
	DWORD   _DataLength;//���ݳ���
	DWORD   _Data;//����ƫ�ƻ����ݣ����DataLength���λΪ1����ô���������ݣ���DataLength&0x7fffffffΪ���ݳ���
	DWORD   _Type;//ֵ����
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
	unsigned char        USBFirstFloorKeyCreatime[MAX_PROTO_USBRECORD_LEN];   //USBFirstFloorKey ����ʱ�� ������ȡ����             
}USBRecovery, *LUSBRecovery;

typedef struct _USBSTORRecovery_
{
	unsigned char        USBSTORFirstFloorKey[MAX_PROTO_USBRECORD_LEN];  
	unsigned char		 USBSTORSecondFloorKey[MAX_PROTO_USBRECORD_LEN];             
	unsigned char        Service[MAX_PROTO_USBRECORD_LEN];                 
	unsigned char        CompatibleIDs[MAX_PROTO_USBRECORD_LEN];                     
	unsigned char        FriendlyName[MAX_PROTO_USBRECORD_LEN];  
	unsigned char        DeviceDesc[MAX_PROTO_USBRECORD_LEN]; 
	unsigned char        USBSTORFirstFloorKeyCreateTime[MAX_PROTO_USBRECORD_LEN];  //USBSTORFirstFloorKey ����ʱ�� ������ȡ����
}USBSTORRecovery, *LUSBSTORRecovery;

typedef std::vector<USBRecovery> USBRecoveryVector;
typedef USBRecoveryVector::iterator USBRecoveryVectorIterator;

typedef std::vector<USBSTORRecovery> USBSTORRecoveryVector;
typedef USBSTORRecoveryVector::iterator USBSTORRecoveryVectorIterator;