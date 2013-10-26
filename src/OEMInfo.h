#pragma once

TCHAR szLogoFile[MAX_PATH] = {0};
HBITMAP hbmpLogo = NULL;
TCHAR szOEMDir[MAX_PATH];

static const TCHAR *szSubKey[] = 
{
	TEXT("Logo"),
	TEXT("Manufacturer"),
	TEXT("Model"),
	TEXT("SupportPhone"),
	TEXT("SupportHours"),
	TEXT("SupportURL")
};

INT_PTR CALLBACK OEMInfoDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL OnInitDlg(HWND hWnd, HWND hwndFocus, LPARAM lParam);
void OnPaint(HWND hWnd);
void OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify);
void OnDropFiles(HWND hWnd, HDROP hDrop);
void OnClose(HWND hWnd);

void doOK(HWND hWnd);
void doEmpty(HWND hWnd);
void doSelectOEM(HWND hWnd);

BOOL IsBmpFile(LPCTSTR lpFile);

LONG ReadOEMInfo(HWND hWnd);
LONG WriteOEMInfo(HWND hWnd);

void LoadOEMList(HWND hWnd);

BOOL IsValidOEMFile(LPCTSTR lpFile);