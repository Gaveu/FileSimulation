#include"driver.h"

//普通派遣函数,主要执行输出IRP请求对应的类型
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	static PCHAR irpType[] = {
		"IRP_MJ_CREATE					",
		"IRP_MJ_CREATE_NAMED_PIPE		",
		"IRP_MJ_CLOSE					",
		"IRP_MJ_READ					",
		"IRP_MJ_WRITE					",
		"IRP_MJ_QUERY_INFORMATION		",
		"IRP_MJ_SET_INFORMATION			",
		"IRP_MJ_QUERY_EA				",
		"IRP_MJ_SET_EA					",
		"IRP_MJ_FLUSH_BUFFERS			",
		"IRP_MJ_QUERY_VOLUME_INFORMATION",
		"IRP_MJ_SET_VOLUME_INFORMATION	",
		"IRP_MJ_DIRECTORY_CONTROL		",
		"IRP_MJ_FILE_SYSTEM_CONTROL		",
		"IRP_MJ_DEVICE_CONTROL			",
		"IRP_MJ_INTERNAL_DEVICE_CONTROL	",
		"IRP_MJ_SHUTDOWN				",
		"IRP_MJ_LOCK_CONTROL			",
		"IRP_MJ_CLEANUP					",
		"IRP_MJ_CREATE_MAILSLOT			",
		"IRP_MJ_QUERY_SECURITY			",
		"IRP_MJ_SET_SECURITY			",
		"IRP_MJ_POWER					",
		"IRP_MJ_SYSTEM_CONTROL			",
		"IRP_MJ_DEVICE_CHANGE			",
		"IRP_MJ_QUERY_QUOTA				",
		"IRP_MJ_SET_QUOTA				",
		"IRP_MJ_PNP						"};

	KdPrint(("%s\n", irpType[stack->MajorFunction]));
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	return STATUS_SUCCESS;
}

//读派遣函数,用于将信息通过IRP请求输出到应用程序中
NTSTATUS DispatchRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	ULONG offset;		//待读取数据在当前设备对象扩展的起始偏移
	ULONG uLength;		//待读取数据的长度
	ULONG uReadLength;	//从设备扩展的数据缓冲区中读到的数据长度
	NTSTATUS status;	
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;

	KdPrint(("Dispatch read!\n"));

	__try
	{
		uLength = stack->Parameters.Read.Length;
		offset = stack->Parameters.Read.ByteOffset.LowPart;

		//offset需在[0,uLength)的区间内,并且数据的读取不能超越缓冲区间
		if (offset + uLength <= MAX_FILE_LENGTH && offset < DeviceExtension->Length)
		{
			if (offset + uLength >= DeviceExtension->Length)
			{
				uReadLength = DeviceExtension->Length - offset;
			}
			else
			{
				uReadLength = uLength;
			}
			RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, DeviceExtension->Buffer + offset, uReadLength);
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_BUFFER_TOO_SMALL;
			uReadLength = 0;
			KdPrint(("Dispatch read failed! 0x%x\n", status));
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
		uReadLength = 0;
		KdPrint(("Dispatch write failed! 0x%x\n", status));
	}

	Irp->IoStatus.Information = uReadLength;
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

//写派遣函数，用于将应用程序的信息通过IRP请求输入到驱动创建的设备对象中
NTSTATUS DispatchWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	ULONG uLength;			//待写入数据长度	
	ULONG uWritedLength;	//写入结束后设备对象扩展的数据长度
	ULONG offset;			//待写入数据在当前设备对象扩展的起始偏移
	NTSTATUS status;
	PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	KdPrint(("Dispatch write!\n"));

	__try
	{
		uLength = stack->Parameters.Write.Length;
		offset = stack->Parameters.Write.ByteOffset.LowPart;
		
		//检查待写入数据会不会超过当前设备扩展的缓冲区长度
		if (offset + uLength <= MAX_FILE_LENGTH)
		{
			//若数据写入后超出原来设备扩展的数据长度，则更新设备扩展的数据长度
			if (offset + uLength > DeviceExtension->Length)
			{
				DeviceExtension->Length = uLength + offset;
			}
			RtlCopyMemory(DeviceExtension + offset, Irp->AssociatedIrp.SystemBuffer, uLength);
			uWritedLength = DeviceExtension->Length;
			status = STATUS_SUCCESS;
		}
		//检查待写入数据会超过当前设备扩展的缓冲区长度，置状态为STATUS_BUFFER_TOO_SMALL
		else
		{
			status = STATUS_BUFFER_TOO_SMALL;
			uWritedLength = 0;
			KdPrint(("Dispatch write failed! 0x%x\n", status));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
		uWritedLength = 0;
		KdPrint(("Dispatch write failed! 0x%x\n", status));
	}

	Irp->IoStatus.Information = uWritedLength;
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

//获取信息派遣函数，用于将设备扩展的数据长度输出给应用程序
NTSTATUS DispatchQueryInfo(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	ULONG uLength;			//获取设备扩展的数据长度	
	NTSTATUS status;
	PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	KdPrint(("Dispatch query information!\n"));

	__try
	{
		uLength = stack->Parameters.QueryFile.Length;
		if (stack->Parameters.QueryFile.FileInformationClass == FileStandardInformation &&
			uLength >= sizeof(FILE_STANDARD_INFORMATION))
		{
			PFILE_STANDARD_INFORMATION pfsi = (PFILE_STANDARD_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
			pfsi->EndOfFile.LowPart = DeviceExtension->Length;
			uLength = sizeof(FILE_STANDARD_INFORMATION);
			status = STATUS_SUCCESS;
		}
		else
		{
			uLength = 0;
			status = STATUS_INVALID_PARAMETER;
			KdPrint(("Dispatch query failed! 0x%x\n",status));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
		uLength = 0;
		KdPrint(("Dispatch query failed! 0x%x\n", status));
	}

	Irp->IoStatus.Information = uLength;
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}