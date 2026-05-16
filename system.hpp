#ifndef _RAR_SYSTEM_
#define _RAR_SYSTEM_

void InitSystemOptions(int SleepTime);
void SetPriority(int Priority);
void SetProcessPriority(int Priority);
void SetThreadPriority(int Priority);
void Shutdown();
bool ShutdownCheckAnother(bool Shutdown);
clock_t MonoClock();
void Wait();

enum WINNT_VERSION {
  WNT_NONE=0,WNT_NT351=0x0333,WNT_NT4=0x0400,WNT_W2000=0x0500,
  WNT_WXP=0x0501,WNT_W2003=0x0502,WNT_VISTA=0x0600,WNT_W7=0x0601,
  WNT_W8=0x0602,WNT_W81=0x0603,WNT_W10=0x0a00
};

DWORD WinNT();
bool IsWindows11OrGreater();

#ifdef _WIN_ALL
bool SetPrivilege(LPCTSTR PrivName);
HMODULE WINAPI LoadSysLibrary(const wchar *Name);
bool IsUserAdmin();
#endif

#ifdef USE_SSE
enum SSE_VERSION {SSE_NONE,SSE_SSE,SSE_SSE2,SSE_SSSE3,SSE_SSE41,SSE_AVX2};
SSE_VERSION GetSSEVersion();
extern SSE_VERSION _SSE_Version;
#endif

#endif
