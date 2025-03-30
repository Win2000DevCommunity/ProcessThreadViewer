#pragma once
#include <windows.h>
#include <string>
#include <vector>

struct ProcessInfo {
    DWORD processId;
    std::string processName;
    std::vector<DWORD> threadIds;
    DWORD_PTR affinityMask;
};

std::vector<ProcessInfo> GetProcessesAndThreads();
std::string AffinityMaskToString(DWORD_PTR affinityMask);
bool KillProcess(DWORD processId);