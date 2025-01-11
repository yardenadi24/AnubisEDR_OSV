#pragma once
#include "../Commons/commons.h"

// Globals
static LIST_ENTRY g_ProcessQueue;
static FAST_MUTEX g_QueueLock;
static BOOLEAN g_Monitor;
static LONG g_AgentPID;

enum PROCESS_STATE {
	PROCESS_PENDING = 0,
	PROCESS_IN_PROGRESS = 1,
	PROCESS_PROCESSED = 2
};

typedef struct _PROCESS_EVENT {
	LIST_ENTRY ListEntry;
	ULONG ProcessId;
	PWCHAR pImageFileName;
	ULONG cbImageFileName;
	KEVENT Event;
	BOOLEAN AllowProcess;
	PROCESS_STATE ProcessState;
}PROCESS_EVENT, * PPROCESS_EVENT;


// Process notification callback
VOID ProcessNotifyCallback(
	PEPROCESS Process,
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
);

NTSTATUS
InitializeProcessMonitor();

VOID
UnInitializeProcessMonitor();

PPROCESS_EVENT FindProcessInList(
	ULONG ProcessIdToFind
);

PPROCESS_EVENT FindProcessSafe(
	ULONG ProcessIdToFind
);
