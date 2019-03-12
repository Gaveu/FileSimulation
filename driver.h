#include<ntddk.h>

typedef struct _DEVICE_EXTENSION
{
	PCHAR Buffer;
	ULONG Length;
#define MAX_FILE_LENGTH 4096
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;

//普通派遣函数,主要执行输出IRP请求对应的类型
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

//读派遣函数,用于将信息通过IRP请求输出到应用程序中
NTSTATUS DispatchRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

//写派遣函数，用于将应用程序的信息通过IRP请求输入到驱动创建的设备对象中
NTSTATUS DispatchWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

//获取信息派遣函数，用于将设备扩展的数据长度输出给应用程序
NTSTATUS DispatchQueryInfo(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);