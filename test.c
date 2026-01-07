#include <ddk/wdm.h>
#include <ddk/ntddk.h>

#define DEVICE_NAME      L"\\Device\\DemoDevice"
#define SYMLINK_NAME     L"\\DosDevices\\DemoDevice"
#define IOCTL_DEMO_PRINT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)


NTSTATUS DemoDeviceIoControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_DEMO_PRINT) {
        ULONG inLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        ULONG outLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        char* buf = (char*)Irp->AssociatedIrp.SystemBuffer;

        if (inLen > 0 && buf) {
            DbgPrint("Received from user: %.*s\n", (int)inLen, buf);
        }

        // 返回数据给用户层
        const char* reply = "hello from kernel";
        ULONG replyLen = (ULONG)strlen(reply) + 1;
        if (outLen >= replyLen) {
            memcpy(buf, reply, replyLen);
            Irp->IoStatus.Information = replyLen;
        } else {
            Irp->IoStatus.Information = 0;
        }
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_DEVICE_REQUEST;
}

NTSTATUS DemoCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

VOID DemoUnload(_In_ PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING symLink;
    RtlInitUnicodeString(&symLink, SYMLINK_NAME);
    IoDeleteSymbolicLink(&symLink);
    IoDeleteDevice(DriverObject->DeviceObject);
    DbgPrint("Driver unloaded\n");
}

VOID C_Exception_Test()
{
    //clang must add -fasync-exceptions, if not, __try block begin/end will incorrect in exception directory
    //see https://github.com/llvm/llvm-project/commit/797ad701522988e212495285dade8efac41a24d4
    //and add -fms-extensions can enable __try/__except support
    __try {
        int *p = NULL;
        *p = 42;  // 引发访问冲突异常
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint("Caught an exception in kernel mode!\n");
    }
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    UNICODE_STRING devName, symLink;
    PDEVICE_OBJECT deviceObject = NULL;

    RtlInitUnicodeString(&devName, DEVICE_NAME);
    RtlInitUnicodeString(&symLink, SYMLINK_NAME);

    NTSTATUS status = IoCreateDevice(
        DriverObject,
        0,
        &devName,
        FILE_DEVICE_UNKNOWN,
        0,
        FALSE,
        &deviceObject
    );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    IoCreateSymbolicLink(&symLink, &devName);

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DemoDeviceIoControl;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DemoCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DemoCreateClose;
    DriverObject->DriverUnload = DemoUnload;
    C_Exception_Test();
    DbgPrint("Driver loaded\n");
    return STATUS_SUCCESS;
}