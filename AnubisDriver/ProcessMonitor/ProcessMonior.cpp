#include "ProcessMonior.h"

#pragma warning(disable : 4996)

NTSTATUS
InitializeProcessMonitor()
{
	NTSTATUS Status = STATUS_SUCCESS;
	g_Monitor = FALSE;
	g_AgentPID = -1;

	// Initialize queue head and mutex
	InitializeListHead(&g_ProcessQueue);
	ExInitializeFastMutex(&g_QueueLock);

	// Register for process creation
	Status = PsSetCreateProcessNotifyRoutineEx(ProcessNotifyCallback, FALSE);

	if(!NT_SUCCESS(Status))
	{
		DbgError("Failed to register for process creation notifications");
		UnInitializeProcessMonitor();
	}

	return Status;
}

VOID
UnInitializeProcessMonitor()
{
	// Unregister from process creation callback
	PsSetCreateProcessNotifyRoutineEx(ProcessNotifyCallback, TRUE);

	// Clean process queue
	ExAcquireFastMutex(&g_QueueLock);
	while (!IsListEmpty(&g_ProcessQueue))
	{
		PLIST_ENTRY pEntry = RemoveHeadList(&g_ProcessQueue);
		PPROCESS_EVENT pItem = CONTAINING_RECORD(pEntry, PROCESS_EVENT, ListEntry);
		ExFreePool(pItem);

	}
	ExReleaseFastMutex(&g_QueueLock);
}

PPROCESS_EVENT FindProcessSafe(
	ULONG ProcessIdToFind
)
{
	PPROCESS_EVENT FoundProcess = NULL;

	ExAcquireFastMutex(&g_QueueLock);
	FoundProcess = FindProcessInList(ProcessIdToFind);
	ExReleaseFastMutex(&g_QueueLock);
	return FoundProcess;
}

PPROCESS_EVENT FindProcessInList(
	ULONG ProcessIdToFind
)
{
	PLIST_ENTRY CurrentEntry;
	PPROCESS_EVENT CurrentProcess;

	// Start from the first entry
	CurrentEntry = g_ProcessQueue.Flink;

	// Traverse the list
	while (CurrentEntry != &g_ProcessQueue) {
		// Get the PROCCESS_EVENT structure containing this entry
		CurrentProcess = CONTAINING_RECORD(CurrentEntry, PROCESS_EVENT, ListEntry);

		// Check if this is the process we're looking for
		if (CurrentProcess->ProcessId == ProcessIdToFind) {
			return CurrentProcess;
		}

		// Move to next entry
		CurrentEntry = CurrentEntry->Flink;
	}

	// Process not found
	return NULL;
}

VOID ProcessNotifyCallback(
	PEPROCESS Process,
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
	UNREFERENCED_PARAMETER(Process);

	if (!CreateInfo) {
		DbgInfo("Process terminated : %lu\n", HandleToULong(ProcessId));
		// Insert process item to the queue
		PPROCESS_EVENT pProcessEventToRemove = FindProcessSafe(HandleToULong(ProcessId));
		
		if (pProcessEventToRemove)
		{
			ExAcquireFastMutex(&g_QueueLock);
			RemoveEntryList(&pProcessEventToRemove->ListEntry);
			ExReleaseFastMutex(&g_QueueLock);
			DbgInfo("Removed from list process [PID:%lu], %wZ", HandleToULong(ProcessId), pProcessEventToRemove->pImageFileName);
			// Free the image filename if it exists
			if (pProcessEventToRemove->pImageFileName != NULL) {
				ExFreePoolWithTag(pProcessEventToRemove->pImageFileName, EDR_MEMORY_TAG);
			}
		}

		return;
	}

	DbgInfo("New process creation: %wZ, PID: %lu\n",
		CreateInfo->ImageFileName,
		HandleToULong(ProcessId));

	// Add to queue
	PPROCESS_EVENT pProcEvent = (PPROCESS_EVENT)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PROCESS_EVENT), EDR_MEMORY_TAG);
	if (!pProcEvent) {
		DbgError("Failed to allocate process event\n");
		return;
	}
	RtlZeroMemory(pProcEvent, sizeof(PROCESS_EVENT));

	// Copy relevant info
	pProcEvent->ProcessId = HandleToULong(ProcessId);
	pProcEvent->cbImageFileName = CreateInfo->ImageFileName->Length;
	pProcEvent->pImageFileName = (PWCHAR)ExAllocatePoolWithTag(NonPagedPool,
		pProcEvent->cbImageFileName,
		EDR_MEMORY_TAG);

	if (pProcEvent->pImageFileName != NULL) {
		// Copy the image file name
		RtlCopyMemory(pProcEvent->pImageFileName,
			CreateInfo->ImageFileName->Buffer,
			CreateInfo->ImageFileName->Length);

		// Null terminate the string
		pProcEvent->pImageFileName[CreateInfo->ImageFileName->Length / sizeof(WCHAR)] = L'\0';
	}

	pProcEvent->ProcessState = PROCESS_PENDING;

	// Initialize syncronization event
	KeInitializeEvent(&pProcEvent->Event, NotificationEvent, FALSE);

	// Insert process item to the queue
	ExAcquireFastMutex(&g_QueueLock);
	DbgPrint("Inserting process to queue: %ws \n[PID: %lu]\n",
		pProcEvent->pImageFileName,
		pProcEvent->ProcessId);
	InsertTailList(&g_ProcessQueue, &pProcEvent->ListEntry);
	ExReleaseFastMutex(&g_QueueLock);
}