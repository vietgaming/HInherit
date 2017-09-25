#include "System.h"

namespace System
{
	map<string, DWORD> GetProcessList()
	{
		map<string, DWORD> processList;
		PROCESSENTRY32 pe32;
		HANDLE hSnapshot = 0;

		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE || hSnapshot == 0)
			return processList;

		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hSnapshot, &pe32))
		{
			CloseHandle(hSnapshot);
			return processList;
		}

		do {
			processList[pe32.szExeFile] = pe32.th32ProcessID;
		} while (Process32Next(hSnapshot, &pe32));

		CloseHandle(hSnapshot);

		return processList;
	}

	map<string, Module> GetProcessModules(HANDLE hProcess, DWORD pid)
	{
		map<string, Module> modules;
		HMODULE hMods[1024];
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
			for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
				TCHAR szModName[MAX_PATH];

				if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
				{
					MODULEINFO modinfo;
					if (GetModuleInformation(hProcess, hMods[i], &modinfo, sizeof(MODULEINFO))) 
					{
						string moduleName(szModName);
						moduleName = moduleName.substr(moduleName.find_last_of("/\\") + 1);
						Module module((DWORD_PTR)modinfo.lpBaseOfDll, (DWORD_PTR)modinfo.SizeOfImage, moduleName);
						modules[moduleName] = module;
					}
				}
			}
		}

		return modules;
	}

	PSYSTEM_HANDLE_INFORMATION QuerySystemInformation()
	{
		typedef NTSTATUS(NTAPI*_NtQuerySystemInformation)(ULONG, PVOID, ULONG, PULONG);
		static _NtQuerySystemInformation NtQuerySystemInformation = reinterpret_cast<_NtQuerySystemInformation>(GetProcAddress(GetModuleHandle("ntdll.dll"), "NtQuerySystemInformation"));

		PVOID buffer = 0;
		ULONG bufferSize = 0;

		while (true) 
		{
			NTSTATUS status = NtQuerySystemInformation(0x10, buffer, bufferSize, &bufferSize);

			if (status) 
			{
				if (status == 0xc0000004) 
				{
					if (buffer != NULL)
						VirtualFree(buffer, bufferSize, MEM_DECOMMIT);

					buffer = VirtualAlloc(0, bufferSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
					continue;
				}
				return nullptr;
			}
			else 
				return reinterpret_cast<PSYSTEM_HANDLE_INFORMATION>(buffer);
			
		}
	}

	bool SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
	{
		TOKEN_PRIVILEGES priv = { 0,0,0,0 };
		HANDLE hToken = NULL;
		LUID luid = { 0,0 };

		if (!OpenProcessToken(GetModuleHandle(NULL), TOKEN_ADJUST_PRIVILEGES, &hToken))
			return false;

		if (!LookupPrivilegeValueA(0, lpszPrivilege, &luid))
		{
			CloseHandle(hToken);
			return false;
		}

		priv.PrivilegeCount = 1;
		priv.Privileges[0].Luid = luid;
		priv.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : SE_PRIVILEGE_REMOVED;

		if (!AdjustTokenPrivileges(hToken, false, &priv, 0, 0, 0))
		{
			CloseHandle(hToken);
			return false;
		}

		CloseHandle(hToken);
		return true;
	}
}