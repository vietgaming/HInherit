#include "ChildProcess.h"

ChildProcess::ChildProcess() {}

ChildProcess::ChildProcess(string childProcPath, string childProcDirectory, HANDLE leakedHandle)
{
	this->childProcPath = childProcPath;
	this->childProcDirectory = childProcDirectory;
	this->leakedHandle = leakedHandle;
}

void ChildProcess::SetInheritance(DWORD flags)
{
	// set the handle to inheritable
	SetHandleInformation(this->leakedHandle, HANDLE_FLAG_INHERIT, flags);
}

void ChildProcess::StealUserToken()
{
	// vulnerable session 1 process handle
	// we need this because we want to execute our child process in session 1 instead of session 0
	HANDLE userProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, false, System::GetProcessList()["Steam.exe"]); 

	OpenProcessToken(userProcessHandle, TOKEN_ALL_ACCESS, &this->vulnerableToken);
	DuplicateTokenEx(this->vulnerableToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &this->userToken);

	CloseHandle(userProcessHandle);
}

void ChildProcess::InitEnvironment()
{
	CreateEnvironmentBlock(&pEnvironment, this->userToken, TRUE);
}

void ChildProcess::InitAttributeList()
{
	SIZE_T cbAttributeListSize;

	// Determine the buffer size for the attribute list
	InitializeProcThreadAttributeList(NULL, 1, 0, &cbAttributeListSize);

	this->pAttributeList = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(VirtualAlloc(NULL, cbAttributeListSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	InitializeProcThreadAttributeList(this->pAttributeList, 1, 0, &cbAttributeListSize);

	HANDLE parentProcessHandle = GetCurrentProcess();

	// Prepare the process we will start to be child of svchost
	UpdateProcThreadAttribute(this->pAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &parentProcessHandle, sizeof(HANDLE), NULL, NULL); 

	CloseHandle(parentProcessHandle);
}

void ChildProcess::Execute()
{
	SetInheritance(HANDLE_FLAG_INHERIT);

	StealUserToken();
	InitEnvironment();
	InitAttributeList();

	// Initialize necessary process structures
	PROCESS_INFORMATION pi = { 0, 0, 0, 0 };
	STARTUPINFOEXA si = { 0 };
	ZeroMemory(&si, sizeof(si));
	si.StartupInfo.cb = sizeof(STARTUPINFOEXA);
	si.StartupInfo.lpDesktop = "winsta0\\default";
	si.lpAttributeList = this->pAttributeList;

	CreateProcessAsUserA(userToken,
						 childProcPath.c_str(),
						 (LPSTR)to_string((DWORD)this->leakedHandle).c_str(), // pass the handle as program argument
						 NULL,
						 NULL,
						 TRUE,
						 CREATE_UNICODE_ENVIRONMENT | EXTENDED_STARTUPINFO_PRESENT,
						 pEnvironment,
						 childProcDirectory.c_str(),
						 reinterpret_cast<LPSTARTUPINFOA>(&si),
						 &pi);

	// Reset the handle inheritance flag
	SetInheritance(NULL);
	
	// Clean up our mess
	CloseHandle(this->vulnerableToken);
	CloseHandle(this->userToken);
	DeleteProcThreadAttributeList(this->pAttributeList);
	DestroyEnvironmentBlock(this->pEnvironment);
}