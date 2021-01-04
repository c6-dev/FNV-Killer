#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#pragma comment( lib, "Psapi.lib" )

HWND g_HWND = NULL;
HANDLE g_hProcess = NULL;
const wchar_t* PROCESS_NAME = L"falloutnv.exe";
const char* PROCESS_FOUND = "FalloutNV.exe found, PID is %d\n";
const char* PROCESS_NOT_RESPONDING = "FalloutNV.exe not responding, terminating process\n";
const char* PROCESS_SEARCH = "Looking for FalloutNV.exe process\n";

BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
	g_HWND = NULL;
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lParam)
	{
		g_HWND = hwnd;
		return FALSE;
	}
	return TRUE;
}

DWORD LookForProcess() {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) return 0;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	while (1) {
		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
				if (_wcsicmp(PROCESS_NAME, pe32.szExeFile) == 0) {				
					g_hProcess = hProcess;
					CloseHandle(hSnapshot);
					printf(PROCESS_FOUND, pe32.th32ProcessID);
					return pe32.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &pe32));
		}
		Sleep(5000);
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) return 0;
	}
	return 0;
}

bool MonitorProcess(DWORD pid) {
	EnumWindows(EnumWindowsProcMy, pid);
	if (g_HWND != 0) {
		while (1) {
			LRESULT success = SendMessageTimeout(g_HWND, 0, 0, 0, SMTO_ABORTIFHUNG, 5000, NULL);
			if (!success || GetLastError() == ERROR_TIMEOUT) {
				Sleep(5000);
				LRESULT success = SendMessageTimeout(g_HWND, 0, 0, 0, SMTO_ABORTIFHUNG, 5000, NULL);
				if (!success || GetLastError() == ERROR_TIMEOUT) {
					printf(PROCESS_NOT_RESPONDING);
					TerminateProcess(g_hProcess, 0);
					return true;
					break;
				}
			}
			Sleep(5000);
		}
	}
	return false;
}

int main()
{
	while (1) {
		printf(PROCESS_SEARCH);
		DWORD pid = LookForProcess();
		if (pid) {
			if (!MonitorProcess(pid)) break;
		}
	}
	return 0;
}


