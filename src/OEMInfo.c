//===============================================================
#pragma comment(linker, "/manifestdependency:\"type='win32'"\
	"name='Microsoft.Windows.Common-Controls' "				\
	"version='6.0.0.0' processorArchitecture='*'"			\
	"publicKeyToken='6595b64144ccf1df' language='*'\"")
//===============================================================

#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>
#include <Strsafe.h>
#include <shlwapi.h>

#include "OEMInfo.h"
#include "res\resource.h"

#pragma comment(lib, "strsafe.lib")
#pragma comment(lib, "shlwapi.lib")

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
#ifdef __VISTA_LATER_OS__

	// 0x0049 ==> WM_COPYGLOBEDATA
	// ChangeWindowMessageFilter - exists only after Vista
	// when UAC enabled && run as admin,
	// drop file will fails if remove the below two lines
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);

#endif

	return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_OEM_INFO), NULL, OEMInfoDlgProc, 0);
}

INT_PTR CALLBACK OEMInfoDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		HANDLE_MSG(hWnd, WM_INITDIALOG, OnInitDlg);
		HANDLE_MSG(hWnd, WM_COMMAND,	OnCommand);
		HANDLE_MSG(hWnd, WM_DROPFILES,	OnDropFiles);
		HANDLE_MSG(hWnd, WM_CLOSE,		OnClose);
	}

	return FALSE;
}

BOOL OnInitDlg(HWND hWnd, HWND hwndFocus, LPARAM lParam)
{
	HICON hIcon = LoadIcon((HINSTANCE)
		GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		MAKEINTRESOURCE(IDI_APP));

	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	SendDlgItemMessage(hWnd, IDC_COMBO_OEM_LIST, CB_ADDSTRING,	\
		0, (LPARAM)TEXT("Default"));
	SendDlgItemMessage(hWnd, IDC_COMBO_OEM_LIST, CB_SETCURSEL,	\
		0, 0);

	LoadOEMList(hWnd);

	if (ReadOEMInfo(hWnd) == ERROR_ACCESS_DENIED)
	{
		MessageBox(hWnd, TEXT("Access denied! Run as administrator please!"),
			TEXT("OEM Information"), MB_ICONERROR);
	}

	return TRUE;
}

void OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id)
	{
	case IDOK:
		doOK(hWnd);
		break;

	case IDC_BUTTON_EMPTY:
		doEmpty(hWnd);
		break;

	case IDC_COMBO_OEM_LIST:
		if (codeNotify == CBN_SELCHANGE)
			doSelectOEM(hWnd);
		break;

	case IDC_BUTTON_ABOUT:
		MessageBox(hWnd, TEXT("<==Just Enjoy It==>\r\n")
			TEXT("by Just Fancy(Just_Fancy@live.com)\r\n")
			TEXT("\t\tJul. 18, 2011"),
			TEXT("About"), MB_ICONINFORMATION);
		break;
	}
}

void OnDropFiles(HWND hWnd, HDROP hDrop)
{
	int count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	int len;
	TCHAR *filePath;

	if (count > 1)
	{
		MessageBox(hWnd, TEXT("Drop one file at a time please"),
			TEXT("OEM Information"), MB_ICONINFORMATION);

		return ;
	}

	len = DragQueryFile(hDrop, 0, NULL, 0);

	filePath = (TCHAR *)calloc(len + 1, sizeof(TCHAR));

	DragQueryFile(hDrop, 0, filePath, len + 1);

	if (!IsBmpFile(filePath))
	{
		MessageBox(hWnd, TEXT("Hey guy, that's not a BITMAP file."),
			TEXT("OEM Information"), MB_ICONINFORMATION);
	}
	else
	{
		HBITMAP hbmpTmp;

		hbmpTmp = (HBITMAP)LoadImage(NULL, filePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		if (hbmpTmp == NULL)
		{
			MessageBox(hWnd, TEXT("Hey guy, that's not a valid BITMAP file."),
				TEXT("OEM Information"), MB_ICONINFORMATION);
		}
		else
		{
			StringCchCopy(szLogoFile, MAX_PATH, filePath);
			hbmpLogo = hbmpTmp;

			hbmpTmp = (HBITMAP)SendDlgItemMessage(hWnd, IDC_LOGO, \
				STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpLogo);

			if (hbmpTmp)
				DeleteBitmap(hbmpTmp);
		}
	}

	free(filePath);

	DragFinish(hDrop);
}

void OnClose(HWND hWnd)
{
	if (hbmpLogo != NULL)
		DeleteBitmap(hbmpLogo);

	EndDialog(hWnd, 0);
}

void doOK(HWND hWnd)
{
	LONG err = WriteOEMInfo(hWnd);

	if (err != ERROR_SUCCESS)
	{
		if (err == ERROR_ACCESS_DENIED)
		{
			MessageBox(hWnd, TEXT("Unable to write valure, access is denied.\r\n")
				TEXT("Run as administrator please."),
				TEXT("OEM Information"), MB_ICONERROR);
		}
		else
		{
			MessageBox(hWnd, TEXT("Failed to write OEM Information!"),
				TEXT("OEM Information"), MB_ICONERROR);
		}
	}
}

void doEmpty(HWND hWnd)
{
	int id = IDC_EDIT_MANUFACTURER;
	for(; id<=IDC_EDIT_URL; id++)
		SetDlgItemText(hWnd, id, TEXT(""));

	SetFocus(GetDlgItem(hWnd, IDC_EDIT_MANUFACTURER));
}

void doSelectOEM(HWND hWnd)
{
	TCHAR szValue[MAX_PATH];
	int id;
	int index = SendDlgItemMessage(hWnd, IDC_COMBO_OEM_LIST,\
		CB_GETCURSEL, 0, 0);

	if (index == 0)
	{
		ReadOEMInfo(hWnd);
	}
	else if (index != CB_ERR)
	{
		TCHAR szIniFile[MAX_PATH];
		SendDlgItemMessage(hWnd, IDC_COMBO_OEM_LIST, CB_GETLBTEXT,
			index, (LPARAM)szValue);

		StringCchPrintf(szIniFile, MAX_PATH, TEXT("%s\\%s"),\
			szOEMDir, szValue);

		for (id=IDC_EDIT_MANUFACTURER; id<=IDC_EDIT_URL; id++)
		{
			GetPrivateProfileString(TEXT("OEMInfo"), \
				szSubKey[id - IDC_LOGO], NULL,	\
				szValue, MAX_PATH, szIniFile);

			SetDlgItemText(hWnd, id, szValue);
		}

		// no matter exists or not
		if (GetPrivateProfileString(TEXT("OEMInfo"), \
			szSubKey[0], NULL,	\
			szValue, MAX_PATH, szIniFile) > 0)
		{
			StringCchPrintf(szIniFile, MAX_PATH, TEXT("%s\\%s"), szOEMDir, szValue);
		}

		hbmpLogo = (HBITMAP)LoadImage(NULL, szIniFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		StringCchCopy(szLogoFile, MAX_PATH, szIniFile);
		DeleteBitmap( (HBITMAP)
			SendDlgItemMessage(hWnd, IDC_LOGO, \
			STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpLogo)
			);
	}
}

BOOL IsBmpFile(LPCTSTR lpFile)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwRead;
	unsigned char signature[2] = { 0 };

	hFile = CreateFile(lpFile, GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	if (!ReadFile(hFile, signature, 2, &dwRead, NULL))
	{
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);

	if (signature[0] == 'B' && signature[1] == 'M')
		return TRUE;

	return FALSE;
}

LONG ReadOEMInfo(HWND hWnd)
{
	HKEY hKey;
	DWORD regType;
	TCHAR szValue[MAX_PATH];
	DWORD cbSize = MAX_PATH;
	int id;

	LSTATUS ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,	\
		TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation\\"), \
		0, KEY_READ, &hKey);

	if (ret != ERROR_SUCCESS)
		return ret;

	regType = REG_SZ;

	ret = RegQueryValueEx(hKey, szSubKey[0], NULL, &regType, (LPBYTE)szLogoFile, &cbSize);

	if (ret == ERROR_SUCCESS)
	{
		hbmpLogo = (HBITMAP)LoadImage(NULL, szLogoFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		if (hbmpLogo)
		{
			SendDlgItemMessage(hWnd, IDC_LOGO, \
				STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpLogo);
		}
	}

	for (id=IDC_EDIT_MANUFACTURER; id<=IDC_EDIT_URL; id++)
	{
		cbSize = MAX_PATH;
		regType = REG_SZ;

		ret = RegQueryValueEx(hKey, szSubKey[id - IDC_EDIT_MANUFACTURER + 1],	\
			NULL, &regType, (LPBYTE)szValue, &cbSize);

		if (ret == ERROR_SUCCESS)
			SetDlgItemText(hWnd, id, szValue);
	}

	RegCloseKey(hKey);

	return ret;
}

LONG WriteOEMInfo(HWND hWnd)
{
	HKEY hKey;
	DWORD dwDisposition;
	int id;
	TCHAR szValue[MAX_PATH];

	LSTATUS ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE,	\
		TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation\\"), \
		0, NULL, REG_OPTION_NON_VOLATILE,	\
		KEY_ALL_ACCESS, NULL,	\
		&hKey, &dwDisposition);

	if (ret != ERROR_SUCCESS)
		return ret;

	RegSetValueEx(hKey, szSubKey[0], 0, REG_SZ, (const BYTE*)szLogoFile, \
		(lstrlen(szLogoFile) + 1) * sizeof(TCHAR));

	for (id=IDC_EDIT_MANUFACTURER; id<=IDC_EDIT_URL; id++)
	{
		int len = GetDlgItemText(hWnd, id, szValue, MAX_PATH);
		if (len > 0)
		{
			ret = RegSetValueEx(hKey, szSubKey[id - IDC_LOGO], \
				0, REG_SZ, (const BYTE*)szValue, 
				(len + 1) * sizeof(TCHAR));
		}
	}

	RegCloseKey(hKey);

	return ret;
}

void LoadOEMList(HWND hWnd)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind;

	TCHAR szIniFile[MAX_PATH];

	if (GetModuleFileName(NULL, szOEMDir, MAX_PATH) == 0)
		return ;

	PathRemoveFileSpec(szOEMDir);

	StringCchCat(szOEMDir, MAX_PATH, TEXT("\\OEM"));

	StringCchPrintf(szIniFile, MAX_PATH, TEXT("%s\\*"),	szOEMDir);

	hFind = FindFirstFile(szIniFile, &wfd);

	if (hFind == INVALID_HANDLE_VALUE)
		return ;

	do 
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue ;

		StringCchPrintf(szIniFile, MAX_PATH, TEXT("%s\\%s"),	\
			szOEMDir, wfd.cFileName);

		if (IsValidOEMFile(szIniFile))
		{
			SendDlgItemMessage(hWnd, IDC_COMBO_OEM_LIST, CB_ADDSTRING,	\
				0, (LPARAM)wfd.cFileName);
		}

	} while (FindNextFile(hFind, &wfd));
		
	FindClose(hFind);
}

BOOL IsValidOEMFile(LPCTSTR lpFile)
{
	TCHAR szSection[10];
	
	if (GetPrivateProfileString(NULL, NULL, NULL, szSection, 10, lpFile) == 0)
		return FALSE;

	if (StrCmpI(szSection, TEXT("OEMInfo")) == 0)
		return TRUE;

	return FALSE;
}
