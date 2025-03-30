#include "process.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <string>

std::vector<ProcessInfo> GetProcessesAndThreads() {
    std::vector<ProcessInfo> processes;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return processes;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(hSnapshot, &pe)) {
        do {
            ProcessInfo pi;
            pi.processId = pe.th32ProcessID;
            pi.processName = pe.szExeFile;
            pi.affinityMask = 0;

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
            if (hProcess != NULL) {
                DWORD_PTR processAffinity, systemAffinity;
                if (GetProcessAffinityMask(hProcess, &processAffinity, &systemAffinity)) {
                    pi.affinityMask = processAffinity;
                }
                CloseHandle(hProcess);
            }

            HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
            if (hThreadSnapshot != INVALID_HANDLE_VALUE) {
                THREADENTRY32 te;
                te.dwSize = sizeof(THREADENTRY32);
                if (Thread32First(hThreadSnapshot, &te)) {
                    do {
                        if (te.th32OwnerProcessID == pe.th32ProcessID) {
                            pi.threadIds.push_back(te.th32ThreadID);
                        }
                    } while (Thread32Next(hThreadSnapshot, &te));
                }
                CloseHandle(hThreadSnapshot);
            }
            processes.push_back(pi);
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return processes;
}

std::string AffinityMaskToString(DWORD_PTR affinityMask) {
    if (affinityMask == 0) return "N/A";
    
    std::string result;
    for (int i = 0; i < sizeof(DWORD_PTR) * 8; i++) {
        if (affinityMask & (1 << i)) {
            if (!result.empty()) result += ",";
            char buffer[20];
            _itoa(i, buffer, 10);
            result += buffer;
        }
    }
    return result.empty() ? "N/A" : result;
}

bool KillProcess(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess == NULL) return false;
    
    bool result = TerminateProcess(hProcess, 0) != 0;
    CloseHandle(hProcess);
    return result;
}