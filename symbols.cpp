#include "symbols.h"
#include <dbghelp.h>
#include <windows.h>

#pragma comment(lib, "dbghelp.lib")

bool g_symbolsInitialized = false;

bool InitializeSymbols() {
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    if (!SymInitialize(GetCurrentProcess(), NULL, TRUE)) {
        return false;
    }
    g_symbolsInitialized = true;
    return true;
}

void CleanupSymbols() {
    if (g_symbolsInitialized) {
        SymCleanup(GetCurrentProcess());
    }
}

void GetStackTrace(HANDLE hProcess, HANDLE hThread, CONTEXT& ctx, std::vector<std::string>& stackTrace) {
    if (!g_symbolsInitialized) return;

    STACKFRAME64 stackFrame = { 0 };
    stackFrame.AddrPC.Offset = ctx.Eip;
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = ctx.Ebp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = ctx.Esp;
    stackFrame.AddrStack.Mode = AddrModeFlat;

    BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255] = {0};
    PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
    pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    pSymbol->MaxNameLength = 255;

    for (int i = 0; i < 50; i++) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_I386, hProcess, hThread,
                        &stackFrame, &ctx, NULL,
                        SymFunctionTableAccess64, 
                        SymGetModuleBase64, NULL)) {
            break;
        }

        if (stackFrame.AddrPC.Offset == 0) {
            break;
        }

        DWORD displacement = 0;
        std::string frameInfo;
        
        if (SymGetSymFromAddr(hProcess, (DWORD)stackFrame.AddrPC.Offset, &displacement, pSymbol)) {
            frameInfo = pSymbol->Name;
        } else {
            frameInfo = "Unknown Function";
        }

        stackTrace.push_back(frameInfo);
    }
}