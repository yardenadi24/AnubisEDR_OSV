#pragma once
#include "../Commons/commons.h"

#include <ntifs.h>
#include <ntdef.h>
#include <ntimage.h>

extern "C"
NTSTATUS
DriverEntry(
	PDRIVER_OBJECT DriverObject, 
	PUNICODE_STRING RegistryPath);

VOID 
DriverUnload(PDRIVER_OBJECT DriverObject);

NTSTATUS 
DriverInitialize();

NTSTATUS
DispatchCreateClose(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp);