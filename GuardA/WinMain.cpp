
#pragma warning(disable : 4995)

#include <Windows.h>
#include <tchar.h>
#include <Tlhelp32.h>
#include <shlwapi.h>
#include <strsafe.h>

#pragma comment(lib, "strsafe.lib")
#pragma comment(lib, "shlwapi.lib")

HINSTANCE ghInst = NULL;
DWORD dwBeginTime = 0;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM Register(HINSTANCE hInst);
BOOL InitWnd();
BOOL EnableDebugPrivilege();
BOOL MainStillLive()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (!hSnapshot)
		return FALSE ;

	PROCESSENTRY32 pe = {0};
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &pe))
		return FALSE;
	do
	{
		if (lstrcmpi(TEXT("timeNot.exe"), pe.szExeFile)==0)
		{
			CloseHandle(hSnapshot);
			return TRUE;
		}

	}while(Process32Next(hSnapshot, &pe));

	CloseHandle(hSnapshot);

	return FALSE;
}

void RunMain()
{
	TCHAR szPath[MAX_PATH] = {0};
	GetModuleFileName(ghInst, szPath, MAX_PATH);

	PathRemoveFileSpec(szPath);

	StringCchCat(szPath, MAX_PATH, TEXT("\\timeNot.exe"));

	if (PathFileExists(szPath))
	{
		ShellExecute(NULL, TEXT("open"), szPath, NULL, NULL, SW_HIDE);

		Sleep(100);

		HWND hWnd = FindWindow(TEXT("timeNot"), TEXT("time_Not_allowed"));
		if (hWnd)
		{
			SendMessage(hWnd, WM_USER + 103, dwBeginTime, 0x101);
		}
	}
}
DWORD WINAPI GuardThread(LPVOID lParam)
{
	while(1)
	{
		if (!MainStillLive())
			RunMain();

		Sleep(50);
	}
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd )
{
	MSG msg = {0};

	if (!Register(hInst))
		return 0;

	if (!InitWnd())
		return 0;

	EnableDebugPrivilege();

	DWORD dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, 0, GuardThread, NULL, NULL, &dwThreadId);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

ATOM Register(HINSTANCE hInst)
{
	WNDCLASSEX wcex = {0};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hInstance = hInst;
	wcex.lpfnWndProc = WndProc;
	wcex.lpszClassName = TEXT("GA");
	wcex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;

	ghInst = hInst;

	return RegisterClassEx(&wcex);
}

BOOL InitWnd()
{
	HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, \
		TEXT("GA"), TEXT("GuardA"),
		WS_POPUP, CW_USEDEFAULT, 0, 0, 0,
		NULL, NULL, ghInst, 0);

	if (!hWnd)	return FALSE;

	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_USER + 204:
		if (wParam == 2 && lParam == 0x2010)
			PostQuitMessage(0);
		break;

	case WM_USER + 203:
		if (lParam == 0x100)
			dwBeginTime = wParam;
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

BOOL EnableDebugPrivilege()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	HANDLE hProcess = GetCurrentProcess();

	if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) )
		return FALSE;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid) )
		return FALSE;

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0))
	{
		CloseHandle(hToken);
		return FALSE;
	}

	CloseHandle(hToken);

	return TRUE;
}