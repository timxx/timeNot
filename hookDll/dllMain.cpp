
#include <Windows.h>

#pragma data_seg("Shared")
HHOOK g_hhook = NULL;
HINSTANCE g_hInst = NULL;
#pragma data_seg()
#pragma comment(linker, "/Section:Shared,rws")

BOOL APIENTRY DllMain(HINSTANCE hDllInst, DWORD dwReason, LPVOID _Reserved)
{
	g_hInst = hDllInst;
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		return TRUE;
	}

	return TRUE;
}