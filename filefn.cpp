#include "rar.hpp"

bool FileExist([[maybe_unused]] const std::wstring &Name) { return false; }
bool WildFileExist([[maybe_unused]] const std::wstring &Name) { return false; }
bool CreateDir([[maybe_unused]] const std::wstring &Name) { return false; }
bool IsDir([[maybe_unused]] uint Attr) { return false; }
bool IsDevice([[maybe_unused]] uint Attr) { return false; }
