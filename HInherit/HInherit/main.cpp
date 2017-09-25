#include "Includes.h"
#include "ProcessInfo.h"
#include "System.h"
#include "ChildProcess.h"

#define CHILD "C:\\Users\\Justin\\Desktop\\TeamSecret\\vendor\\Overlayy\\x64\\Release\\Overlayy.exe"
#define CHILD_DIR "C:\\Users\\Justin\\Desktop\\TeamSecret\\vendor\\Overlayy\\x64\\Release"

ProcessInfo GetHandle(string moduleName)
{
	ProcessInfo processInfo;

	map<string, DWORD> processList = System::GetProcessList();

	if (!processList.count(moduleName)) 
	{
		cout << moduleName << " not found in process list" << endl;
		return processInfo;
	}

	PSYSTEM_HANDLE_INFORMATION handleInfo = System::QuerySystemInformation();

	if (handleInfo == nullptr)
		return processInfo;

	cout << "Got handleInfo" << endl;
	cout << "handleCount=" << handleInfo->HandleCount << endl;

	for (ULONG i = 0; i < handleInfo->HandleCount; i++) 
	{
		auto handle = &handleInfo->Handles[i];

		ULONG ownerPID = handle->ProcessId;
		if (ownerPID != GetCurrentProcessId())
			continue;

		if (GetProcessId(reinterpret_cast<HANDLE>(handle->Handle)) != processList[moduleName])
			continue;

		if (!(handle->GrantedAccess & PROCESS_ALL_ACCESS))
			continue;

		processInfo.handle = reinterpret_cast<HANDLE>(handle->Handle);
		processInfo.pid = processList[moduleName];
		return processInfo;
	}

	return processInfo;
}

void WaitForHandle()
{
	bool isGameRunning = false;
	bool hasValidHandle = false;

	string processName = "dota2.exe";

	ProcessInfo processInfo;

	while (true)
	{
		map<string, DWORD> processList = System::GetProcessList();

		isGameRunning = (processList.count(processName) > 0);

		if (!isGameRunning || !hasValidHandle)
		{
			processInfo = GetHandle(processName);
			map<string, Module> mapNameToModule;

			if (processInfo.handle != nullptr)
				mapNameToModule = System::GetProcessModules(processInfo.handle, processInfo.pid);

			if (processInfo.handle != nullptr && mapNameToModule.count(processName)) 
			{
				// Game is running and we just got the handle
				cout << "Game started" << endl;
				cout << "processInfo.handle: " << hex << processInfo.handle << endl;

				ChildProcess cp(CHILD, CHILD_DIR, processInfo.handle);
				cp.Execute();

				hasValidHandle = true;
				break;
			}
		}

		Sleep(2500);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		freopen("C:\\Test\\HInherit.txt", "w", stdout);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)WaitForHandle, NULL, NULL, NULL);
		break;

	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}