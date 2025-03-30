#pragma once
#include <windows.h>
#include <string>
#include <vector>

// Forward declaration for ProcessInfo since it's used but not defined here
struct ProcessInfo;

struct ThreadInfo {
    DWORD threadId;
    DWORD processId;
    std::string processName;
    std::string purpose;
    std::string status;
    std::string stackInfo;
    std::vector<std::string> stackTrace;
};

std::string GetThreadStatus(DWORD threadId);
std::string GetThreadPurpose(DWORD threadId, DWORD processId);
ThreadInfo GetExtendedThreadInfo(DWORD threadId, DWORD processId, const std::string& processName);
std::vector<ThreadInfo> GetAllThreads(const std::vector<ProcessInfo>& processes);
bool KillThread(DWORD threadId);
