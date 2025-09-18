#include <ddk/wdm.h>
#include <ddk/ntddk.h>

VOID DriverUnload(_In_ PDRIVER_OBJECT driverObject)
{
    UNREFERENCED_PARAMETER(driverObject);

    DbgPrint("Driver unloaded\n");
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING registryPath)
{
    UNREFERENCED_PARAMETER(registryPath);

    DbgPrint("Driver loaded\n");

    driverObject->DriverUnload = DriverUnload;
    return STATUS_SUCCESS;
}