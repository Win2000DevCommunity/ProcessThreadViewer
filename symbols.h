#pragma once
#include <windows.h>
#include <vector>
#include <string>

extern bool g_symbolsInitialized;

bool InitializeSymbols();
void CleanupSymbols();
void GetStackTrace(HANDLE hProcess, HANDLE hThread, CONTEXT& ctx, std::vector<std::string>& stackTrace);