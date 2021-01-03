#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#pragma comment( lib, "Psapi.lib" )

HWND g_HWND = NULL;
HANDLE g_hProcess;

BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
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
		if (hSnapshot == INVALID_HANDLE_VALUE) return 0;
		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
				if (_wcsicmp(L"falloutnv.exe", pe32.szExeFile) == 0) {				
					g_hProcess = hProcess;
					CloseHandle(hSnapshot);
					printf("FalloutNV.exe found, PID is %d\n", pe32.th32ProcessID);
					return pe32.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &pe32));
		}
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) return 0;
		Sleep(5000);
	}
	return 0;
}

bool MonitorProcess(DWORD pid) {
	EnumWindows(EnumWindowsProcMy, pid);
	if (g_HWND != 0) {
		while (1) {
			LRESULT success = SendMessageTimeout(g_HWND, 0, 0, 0, SMTO_ABORTIFHUNG, 5000, NULL);
			if (!success || GetLastError() == ERROR_TIMEOUT) {
				printf("FalloutNV.exe not responding, terminating process\n");
				TerminateProcess(g_hProcess, 0);
				return true;
				break;
			}
			Sleep(5000);
		}
	}
	return false;
}

int main()
{
	while (1) {
		printf("Looking for FalloutNV.exe process\n");
		DWORD pid = LookForProcess();
		if (pid) {
			if (!MonitorProcess(pid)) break;
		}
	}
	return 0;
}


