#pragma once

#include "Includes.h"
#include "Module.h"
#include "SystemStructs.h"

using namespace std;

namespace System
{
	map<string, DWORD> GetProcessList();
	map<string, Module> GetProcessModules(HANDLE hProcess, DWORD pid);
	PSYSTEM_HANDLE_INFORMATION QuerySystemInformation();
	bool SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
}