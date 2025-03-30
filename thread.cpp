#include "thread.h"
#include "process.h"  // For ProcessInfo definition
#include "symbols.h"  // For g_symbolsInitialized and GetStackTrace
#include <tlhelp32.h>
#include <string>

std::string GetThreadStatus(DWORD threadId) {
    HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, threadId);
    if (hThread == NULL) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) return "Access denied";
        return "Unknown";
    }

    DWORD exitCode;
    if (GetExitCodeThread(hThread, &exitCode)) {
        CloseHandle(hThread);
        return (exitCode == STILL_ACTIVE) ? "Running" : "Terminated";
    }
    
    CloseHandle(hThread);
    return "Status unknown";
}

std::string GetThreadPurpose(DWORD threadId, DWORD processId) {
    if (threadId == GetCurrentThreadId() && processId == GetCurrentProcessId()) {
        return "Main Thread";
    }
    return "Worker Thread";
}

ThreadInfo GetExtendedThreadInfo(DWORD threadId, DWORD processId, const std::string& processName) {
    ThreadInfo info;
    info.threadId = threadId;
    info.processId = processId;
    info.processName = processName;
    info.purpose = GetThreadPurpose(threadId, processId);
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) {
        info.stackInfo = "Process access denied";
        return info;
    }

    HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, threadId);
    if (hThread == NULL) {
        info.stackInfo = "Thread access denied";
        CloseHandle(hProcess);
        return info;
    }

    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(hThread, &ctx)) {
        info.stackInfo = "Failed to get context";
    } else {
        char buffer[256];
        sprintf(buffer, "ESP:0x%08X EIP:0x%08X", ctx.Esp, ctx.Eip);
        info.stackInfo = buffer;

        if (g_symbolsInitialized) {
            GetStackTrace(hProcess, hThread, ctx, info.stackTrace);
        }
    }

    CloseHandle(hThread);
    CloseHandle(hProcess);
    return info;
}

std::vector<ThreadInfo> GetAllThreads(const std::vector<ProcessInfo>& processes) {
    std::vector<ThreadInfo> allThreads;
    
    // Replace range-based for with traditional for loop
    for (size_t i = 0; i < processes.size(); ++i) {
        const ProcessInfo& process = processes[i];
        HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hThreadSnapshot == INVALID_HANDLE_VALUE) continue;

        THREADENTRY32 te;
        te.dwSize = sizeof(THREADENTRY32);
        
        if (Thread32First(hThreadSnapshot, &te)) {
            do {
                if (te.th32OwnerProcessID == process.processId) {
                    allThreads.push_back(GetExtendedThreadInfo(te.th32ThreadID, 
                                              process.processId, 
                                              process.processName));
                }
            } while (Thread32Next(hThreadSnapshot, &te));
        }
        CloseHandle(hThreadSnapshot);
    }
    
    return allThreads;
}

bool KillThread(DWORD threadId) {
    HANDLE hThread = OpenThread(THREAD_TERMINATE, FALSE, threadId);
    if (hThread == NULL) return false;
    
    bool result = TerminateThread(hThread, 0) != 0;
    CloseHandle(hThread);
    return result;
}
