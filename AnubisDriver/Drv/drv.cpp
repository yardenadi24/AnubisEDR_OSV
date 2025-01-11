#include "drv.h"
#include "../ProcessMonitor/ProcessMonior.h"


NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    DbgInfo("Driver load (0x%p, %wZ)", DriverObject, RegistryPath);

    NTSTATUS Status = STATUS_SUCCESS;

    Status = DriverInitialize();

    return Status;
}


NTSTATUS
DriverInitialize()
{
    NTSTATUS Status = STATUS_SUCCESS;
    DbgInfo("Initializing driver");
    Status = InitializeProcessMonitor();
    if (!NT_SUCCESS(Status))
    {
        DbgError("Failed to initialize process monitor");
    }
    return Status;
}

VOID
DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UnInitializeProcessMonitor();
}