#include"driver.h"

//��ͨ��ǲ����,��Ҫִ�����IRP�����Ӧ������
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

//����ǲ����,���ڽ���Ϣͨ��IRP���������Ӧ�ó�����
NTSTATUS DispatchRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	ULONG offset;		//����ȡ�����ڵ�ǰ�豸������չ����ʼƫ��
	ULONG uLength;		//����ȡ���ݵĳ���
	ULONG uReadLength;	//���豸��չ�����ݻ������ж��������ݳ���
	NTSTATUS status;	
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;

	KdPrint(("Dispatch read!\n"));

	__try
	{
		uLength = stack->Parameters.Read.Length;
		offset = stack->Parameters.Read.ByteOffset.LowPart;

		//offset����[0,uLength)��������,�������ݵĶ�ȡ���ܳ�Խ��������
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

//д��ǲ���������ڽ�Ӧ�ó������Ϣͨ��IRP�������뵽�����������豸������
NTSTATUS DispatchWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	ULONG uLength;			//��д�����ݳ���	
	ULONG uWritedLength;	//д��������豸������չ�����ݳ���
	ULONG offset;			//��д�������ڵ�ǰ�豸������չ����ʼƫ��
	NTSTATUS status;
	PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	KdPrint(("Dispatch write!\n"));

	__try
	{
		uLength = stack->Parameters.Write.Length;
		offset = stack->Parameters.Write.ByteOffset.LowPart;
		
		//����д�����ݻ᲻�ᳬ����ǰ�豸��չ�Ļ���������
		if (offset + uLength <= MAX_FILE_LENGTH)
		{
			//������д��󳬳�ԭ���豸��չ�����ݳ��ȣ�������豸��չ�����ݳ���
			if (offset + uLength > DeviceExtension->Length)
			{
				DeviceExtension->Length = uLength + offset;
			}
			RtlCopyMemory(DeviceExtension + offset, Irp->AssociatedIrp.SystemBuffer, uLength);
			uWritedLength = DeviceExtension->Length;
			status = STATUS_SUCCESS;
		}
		//����д�����ݻᳬ����ǰ�豸��չ�Ļ��������ȣ���״̬ΪSTATUS_BUFFER_TOO_SMALL
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

//��ȡ��Ϣ��ǲ���������ڽ��豸��չ�����ݳ��������Ӧ�ó���
NTSTATUS DispatchQueryInfo(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	ULONG uLength;			//��ȡ�豸��չ�����ݳ���	
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