#pragma once

#include "Includes.h"
#include "System.h"

using namespace std;

class ChildProcess
{
private:
	string childProcPath;
	string childProcDirectory;
	
	HANDLE leakedHandle;
	HANDLE vulnerableToken; // Vulnerable session 1 process token to steal
	HANDLE userToken; // Our duplicated token from the vulnerable session 1 process
	LPVOID pEnvironment; // Pointer to the environment block for our cheat process to execute with
	PPROC_THREAD_ATTRIBUTE_LIST pAttributeList;

	void SetInheritance(DWORD flags);
	void StealUserToken();
	void InitEnvironment();
	void InitAttributeList();

public:
	ChildProcess();
	ChildProcess(string childProcPath, string childProcDirectory, HANDLE leakedHandle);

	void Execute();
};