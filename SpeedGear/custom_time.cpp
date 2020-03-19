#include "custom_time.h"
#include<map>
#include<sstream>

#define HOTKEY_STICK_LSHIFT 1
#define HOTKEY_STICK_RSHIFT 2
#define HOTKEY_STICK_LCTRL 4
#define HOTKEY_STICK_RCTRL 8
#define HOTKEY_STICK_LALT 16
#define HOTKEY_STICK_RALT 32

#define MIN_SPEED 0.125f
#define MAX_SPEED 8.0f

float current_speed = 1.0f;
float fPreciseAdjustment = 0.25f;

extern LPVOID pfOldSetTimer, pfOldGetMessageTime, pfOldGetTickCount, pfOldGetTickCount64,
pfOldQueryPerformanceCounter, pfOldTimeSetEvent, pfOldTimeGetTime,pfOldSleep,pfOldSleepEx;

UINT_PTR WINAPI HookedSetTimer(HWND a, UINT_PTR b, UINT c, TIMERPROC d)
{
	return ((decltype(&SetTimer))pfOldSetTimer)(a, b, (UINT)(c / current_speed), d);
}

LONG WINAPI HookedGetMessageTime()
{
	return (LONG)(((decltype(&GetMessageTime))pfOldGetMessageTime)() * current_speed);
}

DWORD WINAPI HookedGetTickCount()
{
	return (DWORD)(((decltype(&GetTickCount))pfOldGetTickCount)() * current_speed);
}

ULONGLONG WINAPI HookedGetTickCount64()
{
	return (ULONGLONG)(((decltype(&GetTickCount64))pfOldGetTickCount64)()*current_speed);
}

BOOL WINAPI HookedQueryPerformanceCounter(LARGE_INTEGER*a)
{
	BOOL r = ((decltype(&QueryPerformanceCounter))pfOldQueryPerformanceCounter)(a);
	a->QuadPart = (LONGLONG)(a->QuadPart * current_speed);
	return r;
}

MMRESULT WINAPI HookedtimeSetEvent(UINT a, UINT b, LPTIMECALLBACK c, DWORD_PTR d, UINT e)
{
	return ((decltype(&timeSetEvent))pfOldTimeSetEvent)((UINT)(a/current_speed), b, c, d, e);
}

DWORD WINAPI HookedtimeGetTime()
{
	return (DWORD)(((decltype(&timeGetTime))pfOldTimeGetTime)()*current_speed);
}

VOID WINAPI HookedSleep(DWORD a)
{
	return ((decltype(&Sleep))pfOldSleep)((DWORD)(a/current_speed));
}

DWORD WINAPI HookedSleepEx(DWORD a,BOOL b)
{
	return ((decltype(&SleepEx))pfOldSleepEx)((DWORD)(a/current_speed), b);
}

DWORD vkSpeedUp, vkSpeedDown, vkPreciseSpeedUp, vkPreciseSpeedDown, vkSpeedReset;
int globalApply;

BOOL IsMyAppFocused()
{
	//获取我的进程PID
	DWORD pid;
	GetWindowThreadProcessId(GetActiveWindow(), &pid);
	return pid == GetCurrentProcessId();
}

void ParseKey(LPCTSTR str,DWORD *vk,DWORD *stick)
{
	std::wistringstream iss(str);
	iss >> *vk;
	*stick = 0;
	while (!iss.eof())
	{
		std::wstring inb;
		iss >> inb;
		if (_wcsicmp(inb.c_str(), L"lshift") == 0)
			*stick |= HOTKEY_STICK_LSHIFT;
		if (_wcsicmp(inb.c_str(), L"rshift") == 0)
			*stick |= HOTKEY_STICK_RSHIFT;
		if (_wcsicmp(inb.c_str(), L"lctrl") == 0)
			*stick |= HOTKEY_STICK_LCTRL;
		if (_wcsicmp(inb.c_str(), L"rctrl") == 0)
			*stick |= HOTKEY_STICK_RCTRL;
		if (_wcsicmp(inb.c_str(), L"lalt") == 0)
			*stick |= HOTKEY_STICK_LALT;
		if (_wcsicmp(inb.c_str(), L"ralt") == 0)
			*stick |= HOTKEY_STICK_RALT;
	}
}

class KeyManager
{
private:
	std::map<DWORD, DWORD>hotkeys;
public:
	void AddHotkey(DWORD key, DWORD sticks)
	{
		hotkeys.insert(std::make_pair(key, sticks));
	}
	//返回相应快捷键，如果无则返回0
	DWORD IsHotkeyHit(DWORD key)
	{
		if (hotkeys.find(key) == hotkeys.end())
			return 0;
		DWORD sticks = hotkeys[key];
		if ((sticks & HOTKEY_STICK_LSHIFT) && (GetAsyncKeyState(VK_LSHIFT) & 0x8000) == 0)
			return 0;
		if ((sticks & HOTKEY_STICK_RSHIFT) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000) == 0)
			return 0;
		if ((sticks & HOTKEY_STICK_LCTRL) && (GetAsyncKeyState(VK_LCONTROL) & 0x8000) == 0)
			return 0;
		if ((sticks & HOTKEY_STICK_RCTRL) && (GetAsyncKeyState(VK_RCONTROL) & 0x8000) == 0)
			return 0;
		if ((sticks & HOTKEY_STICK_LALT) && (GetAsyncKeyState(VK_LMENU) & 0x8000) == 0)
			return 0;
		if ((sticks & HOTKEY_STICK_RALT) && (GetAsyncKeyState(VK_RMENU) & 0x8000) == 0)
			return 0;
		return key;
	}
	void Init()
	{
		hotkeys.clear();
	}
};
static KeyManager km;

#include<thread>
#define BEEP_SPEED_UP 1
#define BEEP_SLOW_DOWN 2
#define BEEP_ORIGINAL 0
#define BEEP_ERROR 3
#define BEEP_PREC_UP 4
#define BEEP_PREC_DOWN 5
void SpeedGearBeep(int type)
{
	if (!IsMyAppFocused())
		return;
	std::thread([](int type)
		{
			switch (type)
			{
			case BEEP_ORIGINAL:Beep(1000,100); break;
			case BEEP_SPEED_UP:Beep(2000,100); break;
			case BEEP_SLOW_DOWN:Beep(500, 100); break;
			case BEEP_ERROR:default:Beep(750, 500); break;
			case BEEP_PREC_UP:Beep(DWORD(1000 * (1 + fPreciseAdjustment)), 100); break;
			case BEEP_PREC_DOWN:Beep(DWORD(1000 * (1 - fPreciseAdjustment)), 100);break;
			}
			if (type != BEEP_ERROR&&type!=BEEP_ORIGINAL)
				Beep(DWORD(1000*current_speed), 100);
		},type).detach();
}

void OnKeydown(DWORD vkCode)
{
	DWORD hk = km.IsHotkeyHit(vkCode);
	if (hk&&(globalApply||IsMyAppFocused()))
	{
		if (hk == vkSpeedUp)
		{
			if (current_speed * 2.0f <= MAX_SPEED)
			{
				current_speed *= 2.0f;
				SpeedGearBeep(BEEP_SPEED_UP);
			}
			else
			{
				SpeedGearBeep(BEEP_ERROR);
			}
		}
		else if (hk == vkSpeedDown)
		{
			if (current_speed / 2.0f >= MIN_SPEED)
			{
				current_speed /= 2.0f;
				SpeedGearBeep(BEEP_SLOW_DOWN);
			}
			else
			{
				SpeedGearBeep(BEEP_ERROR);
			}
		}
		else if (hk == vkSpeedReset)
		{
			current_speed = 1.0f;
			SpeedGearBeep(BEEP_ORIGINAL);
		}
		else if (hk == vkPreciseSpeedUp)
		{
			if (current_speed + fPreciseAdjustment <= MAX_SPEED)
			{
				current_speed += fPreciseAdjustment;
				SpeedGearBeep(BEEP_PREC_UP);
			}
			else
			{
				SpeedGearBeep(BEEP_ERROR);
			}
		}
		else if (hk == vkPreciseSpeedDown)
		{
			if (current_speed - fPreciseAdjustment >= MIN_SPEED)
			{
				current_speed -= fPreciseAdjustment;
				SpeedGearBeep(BEEP_PREC_DOWN);
			}
			else
			{
				SpeedGearBeep(BEEP_ERROR);
			}
		}
	}
}

LRESULT CALLBACK ProcessHook(int c, WPARAM w, LPARAM l)
{
	if (c == HC_ACTION)
		switch (w)
		{
		case WM_KEYUP:case WM_SYSKEYUP:
			PKBDLLHOOKSTRUCT pk = (PKBDLLHOOKSTRUCT)l;
			if ((pk->vkCode == VK_RETURN) && (pk->flags & LLKHF_EXTENDED))
				pk->vkCode = VK_SEPARATOR;
			OnKeydown(pk->vkCode);
			break;
		}
	return CallNextHookEx(NULL, c, w, l);
}

static HHOOK hhook = nullptr;

BOOL InitCustomTime()
{
	TCHAR szConfPath[MAX_PATH];
	GetDLLPath(szConfPath, ARRAYSIZE(szConfPath));
	lstrcpy(wcsrchr(szConfPath, '.'), TEXT(".ini"));
#define GetInitConfStr(key,def) GetPrivateProfileString(TEXT("Init"),TEXT(_STRINGIZE(key)),def,key,ARRAYSIZE(key),szConfPath)
#define GetInitConfInt(key,def) key=GetPrivateProfileInt(TEXT("Init"),TEXT(_STRINGIZE(key)),def,szConfPath)
#define F(_i_str) (float)_wtof(_i_str)
	TCHAR keySpeedUp[50], keySpeedDown[50], keyPreciseSpeedUp[50], keyPreciseSpeedDown[50], keySpeedReset[50],preciseAdjustment[20];
	GetInitConfStr(keySpeedUp, TEXT("187"));
	GetInitConfStr(keySpeedDown, TEXT("189"));
	GetInitConfStr(keySpeedReset, TEXT("48"));
	GetInitConfStr(keyPreciseSpeedUp, TEXT("221"));
	GetInitConfStr(keyPreciseSpeedDown, TEXT("219"));
	GetInitConfStr(preciseAdjustment, TEXT("0.25"));
	GetInitConfInt(globalApply, 0);
	km.Init();
	DWORD vStick;
	ParseKey(keySpeedUp, &vkSpeedUp, &vStick);
	km.AddHotkey(vkSpeedUp, vStick);
	ParseKey(keySpeedDown, &vkSpeedDown, &vStick);
	km.AddHotkey(vkSpeedDown, vStick);
	ParseKey(keySpeedReset, &vkSpeedReset, &vStick);
	km.AddHotkey(vkSpeedReset, vStick);
	ParseKey(keyPreciseSpeedUp, &vkPreciseSpeedUp, &vStick);
	km.AddHotkey(vkPreciseSpeedUp, vStick);
	ParseKey(keyPreciseSpeedDown, &vkPreciseSpeedDown, &vStick);
	km.AddHotkey(vkPreciseSpeedDown, vStick);
	fPreciseAdjustment = F(preciseAdjustment);

	hhook = SetWindowsHookEx(WH_KEYBOARD_LL, ProcessHook, GetModuleHandle(NULL), 0);
	return hhook != nullptr;
}

BOOL UninitCustomTime()
{
	return UnhookWindowsHookEx(hhook);
}

