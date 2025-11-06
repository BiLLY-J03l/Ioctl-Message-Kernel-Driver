#include <ntifs.h>
#include <ntddk.h>
#include "common.h"
#define PRINT_MSG_DEVICE 0x8000
#define IOCTL_PRINT_MSG CTL_CODE(PRINT_MSG_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
#define DRIVER_TAG 'rema' // can view it in PoolMon

//UNICODE_STRING g_RegistryPath;

/*
sc.exe create MyDriver type= kernel binPath= c:\users\victim\downloads\MyDriver.sys
Confirm-SecureBootUEFI	(this should be false)
bcdedit /set testsigning on    (REQUIRES REBOOT)
sc.exe start MyFirstDriver
*/

/*
Properties → C/C++ → Code Generation → Runtime Library:
- Change to "Multi-threaded (/MT)" for static linking
*/

void UnloadDriver(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS MyFirstDriverCreate(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS MyFirstDriverClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS MyFirstDriverDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

/*
* IRP (I/O Request Packet)
	* An IRP is the primary object where the request information is stored, for all types of requests.
	* Required for a driver.

*/

/*
* Most software drivers need to do the following in DriverEntry:
	1. Set an Unload routine.
	2. Set dispatch routines the driver supports.
	3. Create a device object.
	4. Create a symbolic link to the device object.
*/


NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	KdPrint(("Sample driver initialized successfully\n"));

	// 1. Unload Routine
	DriverObject->DriverUnload = UnloadDriver;

	// 2. Dispatch Routines
	DriverObject->MajorFunction[IRP_MJ_CREATE] = MyFirstDriverCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = MyFirstDriverClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MyFirstDriverDeviceControl;

	// 3. Create Device Object 
	PDEVICE_OBJECT DeviceObject;
	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\AmerDriver");
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create device object (0x%08X)\n", status));
		return status;
	}
	KdPrint(("CREATED DEVICE OBJECT SUCCESSFULLY!\n"));

	// 4. Create a symbolic link to the device object to be accessed in user mode.
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\AmerDriver");
	status = IoCreateSymbolicLink(&symLink, &DeviceName);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create symbolic link (0x%08X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}

	KdPrint(("CREATED SYMLINK SUCCESSFULLY!\n"));

	// READY TO ACCEPT REQUESTS
	return STATUS_SUCCESS;
}




void UnloadDriver(_In_ PDRIVER_OBJECT DriverObject) {
	// The steps must be in reverse order of the DriverEntry code

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\AmerDriver");
	// delete symbolic link
	IoDeleteSymbolicLink(&symLink);
	// delete device object
	IoDeleteDevice(DriverObject->DeviceObject);
	return;
}

_Use_decl_annotations_ NTSTATUS MyFirstDriverCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

_Use_decl_annotations_ NTSTATUS MyFirstDriverClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

//where the actual work haappens
// 1. Check the control code
_Use_decl_annotations_ NTSTATUS MyFirstDriverDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_PRINT_MSG:
		// do the work
		KdPrint(("IOCTL Code achieved: 0x%X\n", stack->Parameters.DeviceIoControl.IoControlCode));
		
		if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ToPrintMessage)) {
			status = STATUS_BUFFER_TOO_SMALL;
			KdPrint(("STATUS_BUFFER_TOO_SMALL, error 0x%lx\n", status));
			break;
		}
		PVOID data_to_print = (ToPrintMessage*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
		if (data_to_print == NULL) {
			status = STATUS_INVALID_PARAMETER;
			KdPrint(("STATUS_INVALID_PARAMTER, error 0x%lx\n", status));
			break;
		}
		KdPrint(("CLIENT SAYS: %s", data_to_print));
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}


	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}