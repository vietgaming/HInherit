#pragma once

#include <Windows.h>
#include <string>

using namespace std;

class Module
{
private:
	DWORD_PTR address;
	DWORD_PTR size;
	string name;

public:
	Module()
	{
		this->address = NULL;
		this->size = NULL;
		this->name = "";
	}

	Module(DWORD_PTR address, DWORD_PTR size, string name)
	{
		this->address = address;
		this->size = size;
		this->name = name;
	}

	DWORD_PTR GetBaseAddress() { return address; }
	DWORD_PTR GetModuleSize() { return size; }
	string GetName() { return name; }
};