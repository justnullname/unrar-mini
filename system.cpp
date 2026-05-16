#include "rar.hpp"

void InitSystem()
{
}

void SetProcessPriority([[maybe_unused]] int Priority)
{
}

void Wait()
{
}

bool IsWindows11OrGreater()
{
  // Assume modern Windows for QuickView
  return true;
}

DWORD WinNT()
{
  // Always NT on modern systems
  return WNT_W10; // Windows 10+
}

void SetLowPriority()
{
}

void SleepMsg([[maybe_unused]] uint MS)
{
}
