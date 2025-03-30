#pragma once
#include <windows.h>
#include <commctrl.h>
#include "process.h"
#include "thread.h"

// Global UI handles
extern HWND hwndTab, hwndProcessListView, hwndThreadListView;
extern HWND hRefreshBtn, hKillProcessBtn, hKillThreadBtn;

void CreateProcessListView(HWND hwndListView, const std::vector<ProcessInfo>& processes);
void PopulateThreadListView();
void RefreshData(HWND hwnd);
