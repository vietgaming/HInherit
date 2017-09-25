#pragma once

#include <Windows.h>

using namespace std;

struct ProcessInfo 
{
	ProcessInfo() : handle(nullptr), pid(0) {}
	HANDLE handle;
	DWORD pid;
};