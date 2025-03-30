#include "ui.h"
#include "resources.h"
#include "symbols.h"
#include <windows.h>
#include <commctrl.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

   hwndTab = CreateWindow(WC_TABCONTROL, "", 
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_FIXEDWIDTH, 
    10, 10, 780, 550, hwnd, NULL, NULL, NULL);

    // Create tabs
    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPSTR)"Processes";
    TabCtrl_InsertItem(hwndTab, 0, &tie);
    tie.pszText = (LPSTR)"Threads";
    TabCtrl_InsertItem(hwndTab, 1, &tie);
    tie.pszText = (LPSTR)"About";
    TabCtrl_InsertItem(hwndTab, 2, &tie);
    TabCtrl_SetCurSel(hwndTab, 0);

    // Create list views and about panel
    hwndProcessListView = CreateWindow(WC_LISTVIEW, "", 
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, 
        20, 40, 760, 520, hwndTab, (HMENU)IDC_LISTVIEW, NULL, NULL);

    hwndThreadListView = CreateWindow(WC_LISTVIEW, "", 
        WS_CHILD | LVS_REPORT | LVS_SINGLESEL, 
        20, 40, 760, 520, hwndTab, NULL, NULL, NULL);

    // Create About panel
    HWND hwndAboutPanel = CreateWindow("STATIC", "",
        WS_CHILD | SS_BITMAP,
        20, 40, 760, 520, hwndTab, (HMENU)IDC_ABOUT_TAB, NULL, NULL);

    // Load Windows 2000 logo (you'll need to add this bitmap resource)
    HBITMAP hLogo = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_WIN2000_LOGO));
    if (hLogo) {
        SendMessage(hwndAboutPanel, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hLogo);
    }

    // Create credit text
    HWND hwndCreditText = CreateWindow("STATIC", 
        "Process and Thread Viewer\n"
        "By Win2000_dev_community\n\n"
        "Special thanks to all developers\n"
        "contributing to Windows 2000 projects",
        WS_CHILD | SS_CENTER | WS_VISIBLE,
        100, 300, 560, 150, hwndAboutPanel, NULL, NULL, NULL);

    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    if (hFont) {
        SendMessage(hwndCreditText, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    // Create buttons
    hRefreshBtn = CreateWindow("BUTTON", "Refresh", 
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        700, 600, 80, 25, hwnd, (HMENU)IDC_REFRESH_BTN, NULL, NULL);
        
    hKillProcessBtn = CreateWindow("BUTTON", "Kill Process", 
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        600, 600, 90, 25, hwnd, (HMENU)IDC_KILL_PROCESS_BTN, NULL, NULL);
        
    hKillThreadBtn = CreateWindow("BUTTON", "Kill Thread", 
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        490, 600, 90, 25, hwnd, (HMENU)IDC_KILL_THREAD_BTN, NULL, NULL);

    if (!InitializeSymbols()) {
        MessageBox(hwnd, "Symbol initialization failed. Stack traces will not be available.", 
                  "Warning", MB_ICONWARNING);
    }

    RefreshData(hwnd);
    break;
}
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_REFRESH_BTN: {
                    RefreshData(hwnd);
                    break;
                }
                case IDC_KILL_PROCESS_BTN: {
                    int selected = ListView_GetNextItem(hwndProcessListView, -1, LVNI_SELECTED);
                    if (selected != -1) {
                        char pidStr[20];
                        ListView_GetItemText(hwndProcessListView, selected, 1, pidStr, sizeof(pidStr));
                        DWORD pid = atoi(pidStr);
                        
                        if (MessageBox(hwnd, "Are you sure you want to kill this process?", 
                                      "Confirm", MB_YESNO | MB_ICONWARNING) == IDYES) {
                            if (KillProcess(pid)) {
                                RefreshData(hwnd);
                            } else {
                                MessageBox(hwnd, "Failed to kill process", "Error", MB_ICONERROR);
                            }
                        }
                    }
                    break;
                }
                case IDC_KILL_THREAD_BTN: {
                    int selected = ListView_GetNextItem(hwndThreadListView, -1, LVNI_SELECTED);
                    if (selected != -1) {
                        char tidStr[20];
                        ListView_GetItemText(hwndThreadListView, selected, 2, tidStr, sizeof(tidStr));
                        DWORD tid = atoi(tidStr);
                        
                        if (MessageBox(hwnd, "Warning: Killing a thread may cause instability in the owning process.\nContinue?", 
                                      "Warning", MB_YESNO | MB_ICONWARNING) == IDYES) {
                            if (KillThread(tid)) {
                                RefreshData(hwnd);
                            } else {
                                MessageBox(hwnd, "Failed to kill thread", "Error", MB_ICONERROR);
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }

      case WM_NOTIFY: {
    LPNMHDR lpnmh = (LPNMHDR)lParam;
    
    if (lpnmh->hwndFrom == hwndTab && lpnmh->code == TCN_SELCHANGE) {
        int tab = TabCtrl_GetCurSel(hwndTab);
        ShowWindow(hwndProcessListView, tab == 0 ? SW_SHOW : SW_HIDE);
        ShowWindow(hwndThreadListView, tab == 1 ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hwndTab, IDC_ABOUT_TAB), tab == 2 ? SW_SHOW : SW_HIDE); 
        ShowWindow(hRefreshBtn, SW_SHOW);
        ShowWindow(hKillProcessBtn, tab == 0 ? SW_SHOW : SW_HIDE);
        ShowWindow(hKillThreadBtn, tab == 1 ? SW_SHOW : SW_HIDE);
    }
    break;
}

        case WM_SIZE: {
    // Resize controls when window is resized
    int width = LOWORD(lParam);
    int height = HIWORD(lParam);
    
    SetWindowPos(hwndTab, NULL, 10, 10, width - 20, height - 70, SWP_NOZORDER);
    SetWindowPos(hRefreshBtn, NULL, width - 90, height - 30, 80, 25, SWP_NOZORDER);
    SetWindowPos(hKillProcessBtn, NULL, width - 180, height - 30, 90, 25, SWP_NOZORDER);
    SetWindowPos(hKillThreadBtn, NULL, width - 280, height - 30, 90, 25, SWP_NOZORDER);
    
    // Resize all tab contents
    RECT rcTab;
    GetClientRect(hwndTab, &rcTab);
    TabCtrl_AdjustRect(hwndTab, FALSE, &rcTab);
    
    SetWindowPos(hwndProcessListView, NULL, 
                rcTab.left, rcTab.top, 
                rcTab.right - rcTab.left, rcTab.bottom - rcTab.top, 
                SWP_NOZORDER);
    
    SetWindowPos(hwndThreadListView, NULL, 
                rcTab.left, rcTab.top, 
                rcTab.right - rcTab.left, rcTab.bottom - rcTab.top, 
                SWP_NOZORDER);
    
    SetWindowPos(GetDlgItem(hwndTab, IDC_ABOUT_TAB), NULL,
                rcTab.left, rcTab.top,
                rcTab.right - rcTab.left, rcTab.bottom - rcTab.top,
                SWP_NOZORDER);
    break;
}

        case WM_DESTROY:
            CleanupSymbols();
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "ProcessThreadViewer";
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateWindow(wc.lpszClassName, "Process and Thread Viewer with Stack Info",
    WS_OVERLAPPEDWINDOW | WS_SIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 800, 650,  // Increased height to 650
    NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONERROR);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
