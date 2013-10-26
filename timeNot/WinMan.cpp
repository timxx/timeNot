//====================================================================
#include "WinMain.h"
#include "../hookDll/hook.h"
#include <Tlhelp32.h>
#include <shlwapi.h>

#pragma comment(lib, "strsafe.lib")
#pragma comment(lib, "hookdll.lib")
#pragma comment(lib, "shlwapi.lib")

//====================================================================

int APIENTRY _tWinMain(HINSTANCE hInst, HINSTANCE hInstPrv, LPTSTR lpCmdLine , int nCmdShow)
{
	MSG msg = {0};

	if (IsTimeUpNoTip())
	{
		Shutdown();
		return 0;
	}
	else if (IsTimeUpWithTip())
	{
		if (lstrlen(lpCmdLine) > 0 )
		{
			fShutdown12 = true;
		}
	}

	if (!Register(hInst)){
		//ReportLastErr(TEXT("Failed to Register Class"));
		return 0;
	}

	GdiplusStartup(&token, &input, NULL);

// 	TCHAR szImgPath[MAX_PATH] = {0};
// 	GetModuleFileName(hInst, szImgPath, MAX_PATH);
// 	PathRemoveFileSpec(szImgPath);
// 
// 	StringCchCat(szImgPath, MAX_PATH, TEXT("\\bobo.gif"));

	//pImg = new ImageEx(szImgPath);

	pImg = new ImageEx;
	pImg->Load(hInst, TEXT("IMG_GIF"), MAKEINTRESOURCE(IDR_IMG_GIF1));

	if (!InitWnd()){
		//ReportLastErr(TEXT("Create Window Failed"));
		return 0;
	}
	
	EnableDebugPrivilege();

	HideProcess();

	InstallHook(GetCurrentProcessId(), 0);

	DWORD dwThreadId = 0;
	hThread = CreateThread(NULL, 0, GuardThread, NULL, NULL, &dwThreadId);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (token) GdiplusShutdown(token);
	delete pImg;

	return msg.wParam;
}

ATOM Register(HINSTANCE hInst)
{
	WNDCLASSEX wcex = {0};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hbrBackground = CreateSolidBrush(ALPHA_COLOR);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP));
	wcex.hInstance = hInst;
	wcex.lpfnWndProc = WndProc;
	wcex.lpszClassName = szClsName;
	wcex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;

	ghInst = hInst;

	return RegisterClassEx(&wcex);
}

BOOL InitWnd()
{
	HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED, \
		szClsName, TEXT("time_Not_allowed"),
		WS_POPUP, CW_USEDEFAULT, 0, WND_WIDTH, WND_HEIGHT,
		NULL, NULL, ghInst, 0);

	if (!hWnd)	return FALSE;

//	CenterWnd(hWnd, GetDesktopWindow());
	MoveToLeftBottom(hWnd);
	SetLayeredWindowAttributes(hWnd, ALPHA_COLOR, 0, LWA_COLORKEY);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	SetTimer(hWnd, DEF_TIMER_ID, 1000, NULL); 

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		HANDLE_MSG(hWnd, WM_DESTROY,	OnDestroy);
		HANDLE_MSG(hWnd, WM_TIMER,		OnTimer);
		HANDLE_MSG(hWnd, WM_PAINT,		OnPaint);
		//HANDLE_MSG(hWnd, WM_COPYDATA,	OnCopydata);

	case WM_USER + 103:
		if (lParam == 0x101)
		{
			dwBeginTime = wParam;
			
			if (IsTimeUpWithTip())
			{
				KillTimer(hWnd, DEF_TIMER_ID);
				KillTimer(hWnd, FLASH_TIMER_ID);
				SetTimer(hWnd, FLASH_TIMER_ID, 1000, NULL);
			}
		}
		break;

	case WM_USER + 104:
		if (wParam == 1 && lParam == 0x10)
		{
			TerminateThread(hThread, 0);
			SetAutoRun(false);
			SendMessage(hWnd, WM_DESTROY, 0, 0);
		}
// 		else if (wParam == 2 && lParam == 0x11)
// 		{
// 			TerminateThread(hThread, 0);
// 
// 			HWND hWndA = FindWindow(TEXT("GA"), TEXT("GuardA"));
// 
// 			if (hWndA)
// 				SendMessage(hWndA, WM_USER + 204, 2, (LPARAM)0x2010);
// 
// 			KillTimer(hWnd, FLASH_TIMER_ID);
// 			Shutdown();
// 			SendMessage(hWnd, WM_DESTROY, 0, 0);
// 		}
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void ReportLastErr(TCHAR *lpTitle)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf, 0, NULL );

	MessageBox(GetActiveWindow(), (TCHAR*)lpMsgBuf, lpTitle, MB_ICONERROR);

	LocalFree(lpMsgBuf);
}

void HideProcess()
{
// 	typedef int (__stdcall *RSP)(DWORD, DWORD);
// 
// 	RSP RegisterServiceProcess = (RSP)GetProcAddress(LoadLibrary(TEXT("kernel32.dll")), "RegisterServiceProcess");
// 
// 	if (RegisterServiceProcess)
// 		RegisterServiceProcess(NULL, 1);
//  	else
//  		ReportLastErr(L"what?");
}

BOOL IsTimeUpNoTip()
{
	SYSTEMTIME sysTime = {0};

	GetLocalTime(&sysTime);
	
	return (sysTime.wHour>=0 && sysTime.wHour<= 5 );
}

BOOL IsTimeUpWithTip()
{
	SYSTEMTIME sysTime = {0};

	GetLocalTime(&sysTime);

	if (oldHur> sysTime.wHour)
	{
		return TRUE;
	}
	else if (oldHur == sysTime.wHour && oldMin> sysTime.wMinute)
		return TRUE;
	else
	{
		oldHur = sysTime.wHour;
		oldMin = sysTime.wMinute;
	}

	return (sysTime.wHour>=23 && sysTime.wMinute >=30);
}

void OnDestroy(HWND hWnd)
{
	InstallHook(0, 0, FALSE);
	PostQuitMessage(0);
}

void OnTimer(HWND hWnd, UINT id)
{
	if (id == DEF_TIMER_ID)
	{
		if (IsTimeUpWithTip())
		{
			if (!fShutdown12)
			{
				if (dwBeginTime >0 )
				{
					KillTimer(hWnd, DEF_TIMER_ID);

					SetTimer(hWnd, FLASH_TIMER_ID, 1000, NULL);

					HWND hWndA = FindWindow(TEXT("GA"), TEXT("GuardA"));

					if (hWndA)
						SendMessage(hWndA, WM_USER + 203, dwBeginTime, (LPARAM)0x100);

					return ;
				}

				StringCchCopy(szInfo, 512, TEXT("请做好保存工作，系统将在30分后自动关机！"));

				InvalidateRect(hWnd, NULL, TRUE);

				dwBeginTime = GetTickCount();

				HWND hWndA = FindWindow(TEXT("GA"), TEXT("GuardA"));

				if (hWndA)
					SendMessage(hWndA, WM_USER + 203, dwBeginTime, (LPARAM)0x100);

				KillTimer(hWnd, DEF_TIMER_ID);

				SetTimer(hWnd, FLASH_TIMER_ID, 1000, NULL);
			}
			else
			{
				SYSTEMTIME st = {0};
				GetLocalTime(&st);

				if (st.wHour == 0 || st.wHour == 24)
					Shutdown();
				else
				{
					StringCchCopy(szInfo, 512, TEXT("请做好保存工作，系统将在0点自动关机！"));

					InvalidateRect(hWnd, NULL, TRUE);
				}
			}
			SetTopWindow(hWnd);
		}
		else if (IsTimeUpNoTip())
		{
			Shutdown();
		}
	}
	else if (id == FLASH_TIMER_ID)
	{
		DWORD elapsedTime = (GetTickCount() - dwBeginTime)/1000;

		if (elapsedTime <= 5)
		{
			StringCchCopy(szInfo, 512, TEXT("请做好保存工作，系统将在30分后自动关机！"));
		}
		else if(elapsedTime >= 30*60)
		{
			Shutdown();
			//SendMessage(hWnd, WM_USER + 104, 2, 0x11);
		}
		else
		{
			DWORD time = (30*60 - elapsedTime);
			if(time>60)
			{
				StringCchPrintf(szInfo, 512, TEXT("时间不早了，休息吧\r\n离关机还剩%02ld分%02d秒"), time/60, time%60);
			}
			else
			{
				StringCchPrintf(szInfo, 512, TEXT("时间不早了，休息吧\r\n离关机还剩%02ld秒"), time);
			}

			InvalidateRect(hWnd, NULL, TRUE);

// 			ShowWindow(hWnd, SW_HIDE);
// 			Sleep(500);
// 			ShowWindow(hWnd, SW_SHOW);
		}
	}
}

void OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint(hWnd, &ps);

	if (IsTimeUpWithTip())
	{
		if (pImg)
			pImg->Draw(hWnd, 0, 0, pImg->GetWidth(), pImg->GetHeight());
	}

	DrawInfo(hWnd, szInfo);

	EndPaint(hWnd, &ps);
}

// BOOL OnCopydata(HWND hWnd, HWND hwndFrom, COPYDATASTRUCT *pcds)
// {
// 	if (pcds->dwData == 0x100)
// 	{
// 		if (lstrcmp((TCHAR*)pcds->lpData, TEXT("ally1324")) == 0)
// 		{
// 			HWND hWndA = FindWindow(TEXT("GA"), TEXT("GuardA"));
// 
// 			if (hWndA)
// 				SendMessage(hWndA, WM_USER + 204, 0, 0);
// 
// 			SendMessage(hWnd, WM_CLOSE, 0, 0);
// 		}
// 	}
// 
// 	return TRUE;
// }

void DrawInfo(HWND hWnd, const TCHAR *strInfo)
{
	RECT rect = {0};

	HDC hdc = NULL;
// 	HDC hMemDC = NULL;
// 
// 	HBITMAP hMemBmp = NULL;
// 	HBITMAP hbmpOld = NULL;

	HFONT hFont = NULL;
	HGDIOBJ hOldObj = NULL;

	int len = lstrlen(strInfo);
	if (len <= 0)	return ;

	GetClientRect(hWnd, &rect);

	hdc = GetDC(hWnd);
	if (!hdc)	return ;

// 	hMemDC = CreateCompatibleDC(hdc);
// 	if (!hMemDC)
// 	{
// 		return ;
// 		ReleaseDC(hWnd, hdc);
// 	}
// 
// 	hMemBmp = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
// 
// 	if (!hMemBmp)
// 	{
// 		DeleteDC(hMemDC);
// 		ReleaseDC(hWnd, hdc);
// 		return ;
// 	}
//
// 	hbmpOld = (HBITMAP)SelectObject(hMemDC, hMemBmp);

	hFont = CreateFont(23, 0, 0, 0, FW_BLACK, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("宋体"));
	if (!hFont)	return ;

	hOldObj = SelectObject(hdc, hFont);

	SetTextColor(hdc, RGB(255, 0, 0));
	SetBkMode(hdc, TRANSPARENT);

// 	FillRect(hMemDC, &rect, CreateSolidBrush(ALPHA_COLOR));

	rect.left += 55;
	rect.top += 6;

	DrawText(hdc, strInfo, len, &rect, DT_LEFT | DT_WORDBREAK);

// 	BitBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hMemDC, 0, 0, SRCCOPY);
// 
// 	SelectObject(hMemDC, hOldObj);
// 	SelectObject(hMemDC, hbmpOld);
// 
	SelectObject(hdc, hOldObj);
	DeleteObject(hFont);
// 	DeleteObject(hMemBmp);
// 	DeleteDC(hMemDC);

	ReleaseDC(hWnd, hdc);
}

void Shutdown()
{
	//ShellExecute(NULL, TEXT("open"), TEXT("shutdown.exe"), TEXT("-s -t 30 -c 早点休息吧~~"), NULL, SW_SHOW);

	EnableShutdownPrivilege();
	ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
}

// void CenterWnd(HWND hwndTo, HWND hwndParent)
// {
// 	RECT rcParent;
// 	RECT rcTo;
// 	int x, y;
// 	int nWidth, nHeight;
// 
// 	GetWindowRect(hwndParent, &rcParent);
// 	GetWindowRect(hwndTo, &rcTo);
// 
// 	nWidth = rcTo.right - rcTo.left;
// 	nHeight = rcTo.bottom - rcTo.top;
// 
// 	x = rcParent.left + (rcParent.right - rcParent.left - nWidth)/2;
// 	y = rcParent.top + (rcParent.bottom - rcParent.top - nHeight)/2;
// 
// 	MoveWindow(hwndTo, x, y, nWidth, nHeight, TRUE);
// }

void MoveToLeftBottom(HWND hWnd)
{
	int x = GetSystemMetrics(SM_CXSCREEN) - WND_WIDTH - 20;
	int y = GetSystemMetrics(SM_CYSCREEN) - WND_HEIGHT - 40;

	MoveWindow(hWnd, x, y, WND_WIDTH, WND_HEIGHT, TRUE);
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

BOOL EnableShutdownPrivilege()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	HANDLE hProcess = GetCurrentProcess();

	if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) )
		return FALSE;

	if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid) )
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


BOOL GAStillLive()
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

		if (lstrcmpi(TEXT("GuardA.exe"), pe.szExeFile)==0)
		{
			CloseHandle(hSnapshot);
			return TRUE;
		}

	}while(Process32Next(hSnapshot, &pe));

	CloseHandle(hSnapshot);

	return FALSE;
}

void RunA()
{
	TCHAR szPath[MAX_PATH] = {0};
	GetModuleFileName(ghInst, szPath, MAX_PATH);

	PathRemoveFileSpec(szPath);

	StringCchCat(szPath, MAX_PATH, TEXT("\\GuardA.exe"));

	if (PathFileExists(szPath))
	{
		ShellExecute(NULL, TEXT("open"), szPath, NULL, NULL, SW_HIDE);

		Sleep(50);

		HWND hWndA = FindWindow(TEXT("GA"), TEXT("GuardA"));

		if (hWndA)
			SendMessage(hWndA, WM_USER + 203, dwBeginTime, (LPARAM)0x100);
	}
}
DWORD WINAPI GuardThread(LPVOID lParam)
{
	while(1)
	{
		if (!GAStillLive())
			RunA();

		if (!IsKeyExists())
			SetAutoRun(TRUE);

		Sleep(50);
	}
}

BOOL SetAutoRun(BOOL fSet)
{
	HKEY hKey;
	long lResult = ERROR_SUCCESS;
	bool fOk = FALSE;

	lResult = RegCreateKeyEx(HKEY_CURRENT_USER,\
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, NULL,\
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,  &hKey, NULL);

	if (lResult != ERROR_SUCCESS)
		goto _exit;

	if (fSet)
	{
		TCHAR exePath[MAX_PATH];
		GetModuleFileName(ghInst, exePath, MAX_PATH);

		if (!PathFileExists(exePath))
			goto _exit;

		StringCchCat(exePath, MAX_PATH, TEXT(" /aush"));

		lResult = RegSetValueEx(hKey, _T("zzzzzzzz"),
			0, REG_SZ, (const BYTE*)exePath, (lstrlen(exePath)+1)*sizeof(TCHAR));
	}
	else
	{
		if (IsKeyExists())
			lResult = RegDeleteValue(hKey, _T("zzzzzzzz"));
	}

	if (lResult == ERROR_SUCCESS)
		fOk = TRUE;

_exit:
	RegCloseKey(hKey);

	return fOk;
}

BOOL IsKeyExists()
{
	HKEY hKey;
	long lResult = ERROR_SUCCESS;

	lResult = RegCreateKeyEx(HKEY_CURRENT_USER,\
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, NULL,\
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,  &hKey, NULL);

	lResult = RegQueryValueEx(hKey, _T("zzzzzzzz"), 0, NULL, NULL, NULL);

	if (lResult != ERROR_SUCCESS)
	{
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			RegCloseKey(hKey);
			return false;
		}
	}

	RegCloseKey(hKey);

	return true;
}

void SetTopWindow(HWND hWnd)
{
	BringWindowToTop(hWnd);
	SetForegroundWindow(hWnd);
}