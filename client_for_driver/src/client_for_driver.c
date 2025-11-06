#include <windows.h>
#include <fileapi.h>
#include <stdio.h>
#include <ioapiset.h>
#include "common.h"
#define PRINT_MSG_DEVICE 0x8000
#define IOCTL_PRINT_MSG CTL_CODE(PRINT_MSG_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

/*
Properties → C/C++ → Code Generation → Runtime Library:
- Change to "Multi-threaded (/MT)" for static linking
*/

int main(void) {

	// 1. Open Handle to device

	HANDLE hDevice = CreateFile(TEXT("\\\\.\\AmerDriver"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE) {
		printf("Failed to open device, err -> %d\n", GetLastError());
		return 1;
	}

	ToPrintMessage msg = { "HERE IS JOHNNY!" };


	DWORD returned;
	BOOL success = DeviceIoControl(hDevice,
		(DWORD)IOCTL_PRINT_MSG, // control code
		&msg, sizeof(msg), // input buffer and length
		NULL, 0, // output buffer and length
		&returned, NULL);
	if (success)
		printf("[+] message sent successfuly!\n");
	else
		printf("[x] Couldn't send message, err -> %d\n",GetLastError());
	CloseHandle(hDevice);

	return 0;
}