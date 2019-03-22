#include<stdio.h>
#include<windows.h>
#include<winioctl.h>

#define IOTEST CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)

int main()
{
	char buffer[4096] = { 0 };
	DWORD dwSize;
	DWORD dwRet;
	HANDLE hDevice = CreateFile(
		TEXT("\\\\.\\HelloDDK"),
		GENERIC_ALL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Device open failed! %d\n", GetLastError());
		return -1;
	}

	//Ӧ�ó�����ģ���ļ���д������
	WriteFile(hDevice, "This is a string from user", strlen("This is a string from user"), &dwRet, NULL);
	printf("UserData Write Len: %d\n", dwRet);


	//Ӧ�ó����ģ���ļ��ж�ȡ����
	ReadFile(hDevice, buffer, sizeof(buffer), &dwRet, NULL);
	printf("UserData Read: %s\n", buffer);
	printf("UserData Read Len: %d\n", dwRet);

	//Ӧ�ó����ģ���ļ��л�ȡ�ļ����ݳ���
	dwSize = GetFileSize(hDevice, NULL);
	printf("File Len: %d\n", dwSize);


	DeviceIoControl(hDevice, IOTEST, NULL, 0, NULL, 0, &dwRet, NULL);


	CloseHandle(hDevice);

	return 0;
}