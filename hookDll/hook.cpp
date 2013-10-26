
#define _HOOK_API extern "C" __declspec(dllexport)

#include <Windows.h>

#include "hook.h"
#include "APIHook.h"
#include <stdio.h>
#include "resource.h"

#pragma warning(disable : 4995)

extern HHOOK g_hhook;
extern HINSTANCE g_hInst;

#pragma data_seg("Shared")
DWORD myPID = 0;
HANDLE myProcess = NULL;
bool fOk = false;
#pragma data_seg()
#pragma comment(linker, "/Section:Shared,rws")

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam , LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		fOk = false;
		SendMessage(hDlg, EM_SETLIMITTEXT, 32, 0);
		return TRUE;

	case WM_COMMAND:
		SendMessage(hDlg, DM_SETDEFID, IDOK, 0);

		if (wParam == IDOK)
		{
			HWND hWnd = FindWindow(TEXT("timeNot"), TEXT("time_Not_allowed"));

			if (hWnd)
			{
				TCHAR szPw[50] = {0};
				GetDlgItemText(hDlg, IDC_EDIT1, szPw, 50);

				if (lstrcmp(szPw, TEXT("ally1324")) == 0)
				{
					fOk = true;
					HWND hWndA = FindWindow(TEXT("GA"), TEXT("GuardA"));

					if (hWndA)
						SendMessage(hWndA, WM_USER + 204, 2, (LPARAM)0x2010);

					SendMessage(hWnd, WM_USER + 104, 1, (LPARAM)0x10);
				}

// 				COPYDATASTRUCT cds = {0};
// 
// 				cds.dwData = 0x100;
// 				cds.cbData = (lstrlen(szPw)+1)*sizeof(TCHAR);
// 				cds.lpData = szPw;
// 
// 				SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
			}

			SendMessage(hDlg, WM_CLOSE, 0, 0);
		}
		return FALSE;

	case WM_CLOSE:
		EndDialog(hDlg, IDCANCEL);
		return FALSE;
	}

	return FALSE;
}

HANDLE WINAPI MyOpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
{
	HANDLE hProcess = ::OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);

	if (dwProcessId == myPID)
	{
		myProcess = hProcess;
	}

	return hProcess;
}
BOOL WINAPI MyTerminateProcess(HANDLE hProcess, UINT uExitCode)
{
	if (hProcess == myProcess)
	{
		DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc, NULL);

		if (!fOk)
		{
			MessageBox(GetActiveWindow(), TEXT("请不要强行关闭我好不好？"), TEXT("汗~"), MB_ICONINFORMATION);
			return FALSE;
		}
	}
	return ::TerminateProcess(hProcess, uExitCode);
}

CAPIHook OPHook("Kernel32.dll", "OpenProcess", (PROC)MyOpenProcess, TRUE);
CAPIHook TPHook("Kernel32.dll", "TerminateProcess", (PROC)MyTerminateProcess, TRUE);

static LRESULT WINAPI GetMsgProc(int code, WPARAM wParam, LPARAM lParam) {
	return(CallNextHookEx(g_hhook, code, wParam, lParam));
}

BOOL InstallHook(DWORD dwMyPID, DWORD dwThreadId, BOOL fInstall /* = TRUE */)
{
	BOOL fOk = FALSE;

	myPID = dwMyPID;

	if (fInstall)
	{
		if (g_hhook == NULL)
		{
			g_hhook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hInst, 0/*dwThreadId*/);

			fOk = (g_hhook != NULL);
		}
	}
	else
	{
		if (g_hhook != NULL)
		{
			fOk = UnhookWindowsHookEx(g_hhook);
// 			MessageBox(0, L"UnhookWindowsHookEx", L"from dll", MB_ICONINFORMATION);

			g_hhook = NULL;
		}
	}

	return(fOk);
}