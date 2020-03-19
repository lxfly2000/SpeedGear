#include<Windows.h>
#include"..\minhook\include\MinHook.h"

#include"custom_time.h"

#pragma comment(lib,"winmm.lib")

static HMODULE hDllModule;

LPVOID pfOldSetTimer, pfOldGetMessageTime, pfOldGetTickCount,pfOldGetTickCount64,
pfOldQueryPerformanceCounter, pfOldTimeSetEvent, pfOldTimeGetTime, pfOldSleep, pfOldSleepEx;

DWORD GetDLLPath(LPTSTR path, DWORD max_length)
{
	return GetModuleFileName(hDllModule, path, max_length);
}

HRESULT hrLastPresent = S_OK;

#define MHC(x) if(x!=MH_OK)return FALSE

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StartHook()
{
	MHC(MH_Initialize());
	MHC(MH_CreateHook(&SetTimer, HookedSetTimer, &pfOldSetTimer));
	MHC(MH_CreateHook(&GetMessageTime, HookedGetMessageTime, &pfOldGetMessageTime));
	MHC(MH_CreateHook(&GetTickCount, HookedGetTickCount, &pfOldGetTickCount));
	MHC(MH_CreateHook(&GetTickCount64, HookedGetTickCount64, &pfOldGetTickCount64));
	MHC(MH_CreateHook(&QueryPerformanceCounter, HookedQueryPerformanceCounter, &pfOldQueryPerformanceCounter));
	MHC(MH_CreateHook(&timeSetEvent, HookedtimeSetEvent, &pfOldTimeSetEvent));
	MHC(MH_CreateHook(&timeGetTime, HookedtimeGetTime, &pfOldTimeGetTime));
	MHC(MH_CreateHook(&Sleep, HookedSleep, &pfOldSleep));
	MHC(MH_CreateHook(&SleepEx, HookedSleepEx, &pfOldSleepEx));
	MHC(MH_EnableHook(&SetTimer));
	MHC(MH_EnableHook(&GetMessageTime));
	MHC(MH_EnableHook(&GetTickCount));
	MHC(MH_EnableHook(&GetTickCount64));
	MHC(MH_EnableHook(&QueryPerformanceCounter));
	MHC(MH_EnableHook(&timeSetEvent));
	MHC(MH_EnableHook(&timeGetTime));
	MHC(MH_EnableHook(&Sleep));
	MHC(MH_EnableHook(&SleepEx));
	return InitCustomTime();
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StopHook()
{
	if (!UninitCustomTime())
		return FALSE;
	MHC(MH_RemoveHook(&SetTimer));
	MHC(MH_RemoveHook(&GetMessageTime));
	MHC(MH_RemoveHook(&GetTickCount));
	MHC(MH_RemoveHook(&GetTickCount64));
	MHC(MH_RemoveHook(&QueryPerformanceCounter));
	MHC(MH_RemoveHook(&timeSetEvent));
	MHC(MH_RemoveHook(&timeGetTime));
	MHC(MH_RemoveHook(&Sleep));
	MHC(MH_RemoveHook(&SleepEx));
	MHC(MH_Uninitialize());
	return TRUE;
}

DWORD WINAPI TInitHook(LPVOID param)
{
	return StartHook();
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	hDllModule = hInstDll;
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hInstDll);
		StartHook();//Hook不能单独开线程使用
		break;
	case DLL_PROCESS_DETACH:
		StopHook();
		break;
	case DLL_THREAD_ATTACH:break;
	case DLL_THREAD_DETACH:break;
	}
	return TRUE;
}

//SetWindowHookEx需要一个导出函数，否则DLL不会被加载
extern "C" __declspec(dllexport) LRESULT WINAPI HookProc(int code, WPARAM w, LPARAM l)
{
	return CallNextHookEx(NULL, code, w, l);
}
