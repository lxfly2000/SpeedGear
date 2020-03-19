#pragma once
#include<Windows.h>
BOOL InitCustomTime();
BOOL UninitCustomTime();
DWORD GetDLLPath(LPTSTR path, DWORD max_length);
HMODULE GetDLLModule();

UINT_PTR WINAPI HookedSetTimer(HWND, UINT_PTR, UINT, TIMERPROC);
LONG WINAPI HookedGetMessageTime();
DWORD WINAPI HookedGetTickCount();
ULONGLONG WINAPI HookedGetTickCount64();
BOOL WINAPI HookedQueryPerformanceCounter(LARGE_INTEGER*);
MMRESULT WINAPI HookedtimeSetEvent(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);
DWORD WINAPI HookedtimeGetTime();
VOID WINAPI HookedSleep(DWORD);
DWORD WINAPI HookedSleepEx(DWORD, BOOL);