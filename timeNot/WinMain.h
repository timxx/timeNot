//====================================================================
#pragma warning(disable : 4995)

#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>
#include <strsafe.h>

#include "resource.h"
#include "ImageEx.h"

//====================================================================
#define DEF_TIMER_ID	1
#define FLASH_TIMER_ID	2

#define ALPHA_COLOR RGB(254, 254, 254)

#define WND_WIDTH	300
#define WND_HEIGHT	60

//====================================================================
HINSTANCE ghInst = NULL;
const TCHAR szClsName[] = TEXT("timeNot");

DWORD dwBeginTime = 0;

TCHAR szInfo[512] = {0};

ImageEx *pImg = NULL;
ULONG_PTR token;
GdiplusStartupInput input;

HANDLE hThread = NULL;

bool fShutdown12 = false;

int oldHur = 0;
int oldMin = 0;
//====================================================================
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

ATOM Register(HINSTANCE hInst);
BOOL InitWnd();
void ReportLastErr(TCHAR *lpTitle);

void HideProcess();
BOOL IsTimeUpNoTip();
BOOL IsTimeUpWithTip();
void DrawInfo(HWND hWnd, const TCHAR *strInfo);
void Shutdown();
BOOL EnableShutdownPrivilege();
BOOL EnableShutdownPrivilege();

void RunA();
BOOL GAStillLive();

//void CenterWnd(HWND hwndTo, HWND hwndParent);
BOOL IsKeyExists();
BOOL SetAutoRun(BOOL fSet);

void SetTopWindow(HWND hWnd);

DWORD WINAPI GuardThread(LPVOID lParam);
void MoveToLeftBottom(HWND hWnd);
BOOL EnableDebugPrivilege();

void OnDestroy(HWND hWnd);
void OnTimer(HWND hWnd, UINT id);
void OnPaint(HWND hWnd);
//BOOL OnCopydata(HWND hWnd, HWND hwndFrom, COPYDATASTRUCT *pcds);
//====================================================================
