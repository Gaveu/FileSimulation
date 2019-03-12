#include"driver.h"

VOID Unload(IN PDRIVER_OBJECT DriverObject)
{
	KdPrint(("Driver unload!\n"));
	UNICODE_STRING SymbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\HelloDDK");
	PDEVICE_EXTENSION DeviceExtension = DriverObject->DeviceObject->DeviceExtension;
	ExFreePool(DeviceExtension->Buffer);
	IoDeleteDevice(DriverObject->DeviceObject);
	IoDeleteSymbolicLink(&SymbolicLinkName);

}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\HelloDDK");
	UNICODE_STRING SymbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\HelloDDK");
	PDEVICE_OBJECT DeviceObject; 
	PDEVICE_EXTENSION DeviceExtension;
	NTSTATUS status;
	ULONG i;

	KdPrint(("Driver Load!\n"));

	//�����������豸���󡢷������ӳ�ʼ��
	status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), &DeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Device Create failed! 0x%x\n", status));
		return status;
	}
	status = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("SymbolicLink Create failed! 0x%x\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}

	//��ǲ������ʼ��
	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; ++i)
	{
		DriverObject->MajorFunction[i] = DispatchRoutine;
	}
	DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
	DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = DispatchQueryInfo;

	//�豸�����־λ����
	DeviceObject->Flags |= DO_BUFFERED_IO;
	DeviceObject->Flags ^= DO_DEVICE_INITIALIZING;

	//�豸������չ�ĳ�ʼ��
	DeviceExtension = DeviceObject->DeviceExtension;
	DeviceExtension->Buffer = (PCHAR)ExAllocatePool(NonPagedPool, MAX_FILE_LENGTH);
	DeviceExtension->Length = 0;

	return STATUS_SUCCESS;
}