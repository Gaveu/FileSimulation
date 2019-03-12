#include<ntddk.h>

typedef struct _DEVICE_EXTENSION
{
	PCHAR Buffer;
	ULONG Length;
#define MAX_FILE_LENGTH 4096
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;

//��ͨ��ǲ����,��Ҫִ�����IRP�����Ӧ������
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

//����ǲ����,���ڽ���Ϣͨ��IRP���������Ӧ�ó�����
NTSTATUS DispatchRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

//д��ǲ���������ڽ�Ӧ�ó������Ϣͨ��IRP�������뵽�����������豸������
NTSTATUS DispatchWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

//��ȡ��Ϣ��ǲ���������ڽ��豸��չ�����ݳ��������Ӧ�ó���
NTSTATUS DispatchQueryInfo(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);