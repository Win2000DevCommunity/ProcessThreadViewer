#include "ui.h"
#include "resources.h"
#include <commctrl.h>
#include <string>

HWND hwndTab, hwndProcessListView, hwndThreadListView;
HWND hRefreshBtn, hKillProcessBtn, hKillThreadBtn;

std::string to_string(DWORD num) {
    char buffer[20];
    _itoa(num, buffer, 10);
    return buffer;
}

void CreateProcessListView(HWND hwndListView, const std::vector<ProcessInfo>& processes) {
    while (ListView_DeleteColumn(hwndListView, 0)) {}

    LVCOLUMN lvCol;
    lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
    
    lvCol.pszText = (LPSTR)"Process";
    lvCol.cx = 200;
    ListView_InsertColumn(hwndListView, 0, &lvCol);
    
    lvCol.pszText = (LPSTR)"PID";
    lvCol.cx = 100;
    ListView_InsertColumn(hwndListView, 1, &lvCol);
    
    lvCol.pszText = (LPSTR)"Threads";
    lvCol.cx = 100;
    ListView_InsertColumn(hwndListView, 2, &lvCol);
    
    lvCol.pszText = (LPSTR)"Affinity";
    lvCol.cx = 100;
    ListView_InsertColumn(hwndListView, 3, &lvCol);

    ListView_DeleteAllItems(hwndListView);

    for (size_t i = 0; i < processes.size(); ++i) {
        LVITEM lvItem = {0};
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = (int)i;
        lvItem.iSubItem = 0;
        lvItem.pszText = const_cast<char*>(processes[i].processName.c_str());
        ListView_InsertItem(hwndListView, &lvItem);

        lvItem.iSubItem = 1;
        lvItem.pszText = const_cast<char*>(to_string(processes[i].processId).c_str());
        ListView_SetItem(hwndListView, &lvItem);

        lvItem.iSubItem = 2;
        lvItem.pszText = const_cast<char*>(to_string(processes[i].threadIds.size()).c_str());
        ListView_SetItem(hwndListView, &lvItem);

        lvItem.iSubItem = 3;
        std::string affinityStr = AffinityMaskToString(processes[i].affinityMask);
        lvItem.pszText = const_cast<char*>(affinityStr.c_str());
        ListView_SetItem(hwndListView, &lvItem);
    }
}

void PopulateThreadListView() {
    while (ListView_DeleteColumn(hwndThreadListView, 0)) {}

    LVCOLUMN lvCol;
    lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
    
    lvCol.pszText = (LPSTR)"Process ID";
    lvCol.cx = 80;
    ListView_InsertColumn(hwndThreadListView, 0, &lvCol);
    
    lvCol.pszText = (LPSTR)"Process Name";
    lvCol.cx = 150;
    ListView_InsertColumn(hwndThreadListView, 1, &lvCol);
    
    lvCol.pszText = (LPSTR)"Thread ID";
    lvCol.cx = 80;
    ListView_InsertColumn(hwndThreadListView, 2, &lvCol);
    
    lvCol.pszText = (LPSTR)"Purpose";
    lvCol.cx = 120;
    ListView_InsertColumn(hwndThreadListView, 3, &lvCol);
    
    lvCol.pszText = (LPSTR)"Status";
    lvCol.cx = 120;
    ListView_InsertColumn(hwndThreadListView, 4, &lvCol);
    
    lvCol.pszText = (LPSTR)"Stack Info";
    lvCol.cx = 250;
    ListView_InsertColumn(hwndThreadListView, 5, &lvCol);

    ListView_DeleteAllItems(hwndThreadListView);

    std::vector<ProcessInfo> processes = GetProcessesAndThreads();
    std::vector<ThreadInfo> threads = GetAllThreads(processes);

    for (size_t i = 0; i < threads.size(); ++i) {
        LVITEM lvItem = {0};
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = (int)i;
        lvItem.iSubItem = 0;
        
        char pidStr[20];
        _itoa(threads[i].processId, pidStr, 10);
        lvItem.pszText = pidStr;
        ListView_InsertItem(hwndThreadListView, &lvItem);

        ListView_SetItemText(hwndThreadListView, i, 1, (LPSTR)threads[i].processName.c_str());
        
        char tidStr[20];
        _itoa(threads[i].threadId, tidStr, 10);
        ListView_SetItemText(hwndThreadListView, i, 2, tidStr);
        
        ListView_SetItemText(hwndThreadListView, i, 3, (LPSTR)threads[i].purpose.c_str());
        
        std::string status = GetThreadStatus(threads[i].threadId);
        ListView_SetItemText(hwndThreadListView, i, 4, (LPSTR)status.c_str());
        
        ListView_SetItemText(hwndThreadListView, i, 5, (LPSTR)threads[i].stackInfo.c_str());
    }
}

void RefreshData(HWND hwnd) {
    TabCtrl_DeleteAllItems(hwndTab);
    
    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPSTR)"Processes";
    TabCtrl_InsertItem(hwndTab, 0, &tie);
    tie.pszText = (LPSTR)"Threads";
    TabCtrl_InsertItem(hwndTab, 1, &tie);
    tie.pszText = (LPSTR)"About";
    TabCtrl_InsertItem(hwndTab, 2, &tie); 
    
    std::vector<ProcessInfo> processes = GetProcessesAndThreads();
    CreateProcessListView(hwndProcessListView, processes);
    PopulateThreadListView();
    
    int itemCount = ListView_GetItemCount(hwndProcessListView);
    RECT rc;
    GetClientRect(hwnd, &rc);
    int newHeight = std::min(800, 150 + itemCount * 20);
    SetWindowPos(hwnd, NULL, 0, 0, rc.right, newHeight, SWP_NOMOVE | SWP_NOZORDER);
    
    TabCtrl_SetCurSel(hwndTab, 0);
    ShowWindow(hwndProcessListView, SW_SHOW);
    ShowWindow(hwndThreadListView, SW_HIDE);
    ShowWindow(GetDlgItem(hwndTab, IDC_ABOUT_TAB), SW_HIDE);  // Hide About panel
    ShowWindow(hRefreshBtn, SW_SHOW);
    ShowWindow(hKillProcessBtn, SW_SHOW);
    ShowWindow(hKillThreadBtn, SW_HIDE);
}
