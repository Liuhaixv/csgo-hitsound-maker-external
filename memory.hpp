#ifndef MEMORY_HPP
#define MEMORY_HPP

#pragma once
#pragma warning(disable: 6276)

#include <Windows.h>
#include <TlHelp32.h>
#include "misc/config.hpp"

struct MODULE {
	/*BYTE* */DWORD base;
	DWORD size;
};

class Memory {
public:
	DWORD tPID;
	DWORD clientBaseAddr;
	DWORD clientSize;
	DWORD engineBaseAddr;
	DWORD engineSize;
	HANDLE tProcess;
	HWND tHWND;

	Memory() {
		this->tPID = NULL;
		this->tProcess = NULL;

		if (!this->handle_process(TARGET)) return;

		this->tHWND = FindWindowA(0, WINDOW_NAME);
		MODULE client_dll = get_module(CLIENT_DLL);
		MODULE engine_dll = get_module(ENGINE_DLL);

		this->clientBaseAddr = client_dll.base;
		this->engineBaseAddr = engine_dll.base;
		this->clientSize = client_dll.size;
		this->engineSize = engine_dll.size;
	}

	~Memory() {
		CloseHandle(tProcess);
	}

	/*
		HANDLE handle_process(const std::string_view processName) {
		HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);

		do {
			if (!(processName.compare(entry.szExeFile))) {
				tPID = entry.th32ProcessID;
				std::cout << "tPID:" << tPID << std::endl;
				CloseHandle(handle);
				tProcess = OpenProcess(PROCESS_ALL_ACCESS, false, tPID);
				return tProcess;
			}
		} while (Process32Next(handle, &entry));
		CloseHandle(handle);
		return NULL;
	}
	*/
	HANDLE handle_process(const std::string_view targetProcess)
	{
		DWORD procId = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 procEntry;
			procEntry.dwSize = sizeof(procEntry);
			if (Process32First(hSnap, &procEntry))
			{
				do
				{
					if (!targetProcess.compare(procEntry.szExeFile))
					{
						procId = procEntry.th32ProcessID;
						std::cout << "current pID:" << procId << std::endl;
						//break;
					}
				} while (Process32Next(hSnap, &procEntry));
			}
		}
		CloseHandle(hSnap);

		tPID = procId;
		std::cout << "pID:" << tPID << std::endl;
		tProcess = OpenProcess(PROCESS_ALL_ACCESS, false, tPID);
		return tProcess;
	}


	MODULE get_module(const std::string_view moduleName) {
		HANDLE hmodule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, tPID);
		MODULEENTRY32 me32;
		me32.dwSize = sizeof(me32);

		do {
			if (!(moduleName.compare(me32.szModule))){
				CloseHandle(hmodule);
				return { (DWORD)me32.modBaseAddr, me32.modBaseSize };
			}
		} while (Module32Next(hmodule, &me32));

		CloseHandle(hmodule);
		return { NULL, NULL };
	}

	template <typename var>
	bool write_mem(DWORD address, var value) {
		return WriteProcessMemory(tProcess, (LPVOID)address, &value, sizeof(var), NULL);
	}

	template <typename var>
	var read_mem(DWORD address) {
		var value;
		ReadProcessMemory(tProcess, (LPCVOID)address, &value, sizeof(var), NULL);
		return value;
	}
};
#endif