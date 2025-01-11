#pragma once
#include <ntifs.h>
#include <ntdef.h>
#include <ntimage.h>

#define DLL_PATH64 L"C:\\Windows\\System32\\HookDllx64.dll"
#define DLL_PATH86 L"C:\\Windows\\System32\\HookDllx86.dll"

// Process structure to hold information about
// loaded images and state
typedef struct _PROCESS_INFO {

    LIST_ENTRY ListEntry;

    HANDLE ProcessId;
    BOOLEAN IsInjected = FALSE;
    BOOLEAN ForceUserApc;
    ULONG LoadedDlls;
    PVOID LdrLoadDllAddress = NULL;
    PUNICODE_STRING DllPath;

} PROCESS_INFO, * PPROCESS_INFO;

typedef enum _SYSTEM_DLL
{
    NOTHING_LOADED = 0x0000,
    SYSTEM32_NTDLL_LOADED = 0x0001,
    SYSTEM32_KERNEL32_LOADED = 0x0002,
    SYSWOW64_NTDLL_LOADED = 0x0004,
    SYSTEM32_WOW64_LOADED = 0x0008,
    SYSTEM32_WOW64WIN_LOADED = 0x0010,
    SYSTEM32_WOW64CPU_LOADED = 0x0020,
    SYSTEM32_WOWARMHW_LOADED = 0x0040,
} SYSTEM_DLL, * PSYSTEM_DLL;

typedef struct _SYSTEM_DLL_DESCRIPTOR
{
    UNICODE_STRING  DllPath; 
    SYSTEM_DLL  Flag;
} SYSTEM_DLL_DESCRIPTOR, * PSYSTEM_DLL_DESCRIPTOR;

// List used to help tracking dll loaded to each process
SYSTEM_DLL_DESCRIPTOR g_SystemDlls[] = {
  { RTL_CONSTANT_STRING(L"\\SysWow64\\ntdll.dll"),    SYSWOW64_NTDLL_LOADED    },
  { RTL_CONSTANT_STRING(L"\\System32\\ntdll.dll"),    SYSTEM32_NTDLL_LOADED    },
  { RTL_CONSTANT_STRING(L"\\System32\\kernel32.dll"), SYSTEM32_KERNEL32_LOADED },
  { RTL_CONSTANT_STRING(L"\\System32\\wow64.dll"),    SYSTEM32_WOW64_LOADED    },
  { RTL_CONSTANT_STRING(L"\\System32\\wow64win.dll"), SYSTEM32_WOW64WIN_LOADED },
  { RTL_CONSTANT_STRING(L"\\System32\\wow64cpu.dll"), SYSTEM32_WOW64CPU_LOADED },
};


