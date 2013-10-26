
#ifndef _HOOK_H_
#define _HOOK_H_

#ifndef _HOOK_API
#define _HOOK_API extern "C" __declspec(dllimport)
#endif

_HOOK_API BOOL InstallHook(DWORD dwMyPID, DWORD dwThreadId, BOOL fInstall = TRUE);

#endif