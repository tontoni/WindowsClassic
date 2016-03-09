
#ifdef _PROG_REALWINVER

#include <stdio.h>
#include "windowsclassic.h"

struct NTOSInfo
{
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	WORD wServicePackMajor;
};

struct NTOSInfoEx : NTOSInfo
{
	BYTE ProductType;
};

typedef NTOSInfo	*LPNTOSInfo;
typedef NTOSInfoEx	*LPNTOSInfoEx;

static const NTOSInfo KnownVersionsOfWindows[] =
{
	{ 6, 3, 0, }, // Windows 8.1, Windows Server 2012 R2
	{ 6, 2, 0, }, // Windows 8.0, Windows Server 2012

	{ 6, 1, 1, }, // Windows 7, Windows Server 2008 R2 SP1
	{ 6, 1, 0, }, // Windows 7, Windows Server 2008 R2

	{ 6, 0, 2, }, // Windows Vista, Windows Server 2008 SP2
	{ 6, 0, 1, }, // Windows Vista, Windows Server 2008 SP1
	{ 6, 0, 0, }, // Windows Vista, Windows Server 2008

	{ 5, 1, 3, }, // Windows XP SP3
	{ 5, 1, 2, }, // Windows XP SP2
	{ 5, 1, 1, }, // Windows XP SP1
	{ 5, 1, 0, }, // Windows XP

	{ 5, 2, 2, }, // Windows Server 2003 SP2
	{ 5, 2, 1, }, // Windows Server 2003 SP1
	{ 5, 2, 0, }, // Windows Server 2003

	{ 5, 1, 4, }, // Windows Server 2000 SP4
	{ 5, 1, 3, }, // Windows Server 2000 SP3
	{ 5, 1, 2, }, // Windows Server 2000 SP2
	{ 5, 1, 2, }, // Windows Server 2000 SP1
	{ 5, 1, 0, }, // Windows Server 2000
};

static const size_t n_KnownVersionofWindows = (sizeof(KnownVersionsOfWindows) / sizeof(NTOSInfo));

static BOOL EqualsMajorVersion(DWORD majorVersion)
{
	OSVERSIONINFOEX osVersionInfo;
	ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
	
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osVersionInfo.dwMajorVersion = majorVersion;
	
	ULONGLONG maskCondition = VerSetConditionMask(0, VER_MAJORVERSION, VER_EQUAL);
	
	return VerifyVersionInfo(&osVersionInfo, VER_MAJORVERSION, maskCondition);
}

static BOOL EqualsMinorVersion(DWORD minorVersion)
{
	OSVERSIONINFOEX osVersionInfo;
	ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));

	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osVersionInfo.dwMinorVersion = minorVersion;

	ULONGLONG maskCondition = VerSetConditionMask(0, VER_MINORVERSION, VER_EQUAL);
	
	return VerifyVersionInfo(&osVersionInfo, VER_MINORVERSION, maskCondition);
}

static BOOL EqualsServicePack(WORD servicePackMajor)
{
	OSVERSIONINFOEX osVersionInfo;
	ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
	
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osVersionInfo.wServicePackMajor = servicePackMajor;
	
	ULONGLONG maskCondition = VerSetConditionMask(0, VER_SERVICEPACKMAJOR, VER_EQUAL);
	
	return VerifyVersionInfo(&osVersionInfo, VER_SERVICEPACKMAJOR, maskCondition);
}

static BOOL EqualsProductType(BYTE productType)
{
	OSVERSIONINFOEX osVersionInfo;
	ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));

	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osVersionInfo.wProductType = productType;
	
	ULONGLONG maskCondition = VerSetConditionMask(0, VER_PRODUCT_TYPE, VER_EQUAL);
	
	return VerifyVersionInfo(&osVersionInfo, VER_PRODUCT_TYPE, maskCondition);
}

static BYTE GetProductType()
{
	if (EqualsProductType(VER_NT_WORKSTATION))
	{
		return VER_NT_WORKSTATION;
	}
	else if (EqualsProductType(VER_NT_SERVER))
	{
		return VER_NT_SERVER;
	}
	else if (EqualsProductType(VER_NT_DOMAIN_CONTROLLER))
	{
		return VER_NT_DOMAIN_CONTROLLER;
	}

	return 0; // Unkown Product
}

static bool GetKnownWindowsVersion(LPNTOSInfoEx osInfo)
{
	for (size_t i = 0; i < n_KnownVersionofWindows; i++)
	{
		if (EqualsMajorVersion(KnownVersionsOfWindows[i].dwMajorVersion))
		{
			if (EqualsMinorVersion(KnownVersionsOfWindows[i].dwMinorVersion))
			{
				if (EqualsServicePack(KnownVersionsOfWindows[i].wServicePackMajor))
				{
					osInfo->dwMajorVersion = KnownVersionsOfWindows[i].dwMajorVersion;
					osInfo->dwMinorVersion = KnownVersionsOfWindows[i].dwMinorVersion;
					osInfo->wServicePackMajor = KnownVersionsOfWindows[i].wServicePackMajor;
					osInfo->ProductType = GetProductType();

					return true;
				}
			}
		}
	}

	return false;
}

// Windows Version Registry key
static const TSTRING REG_NT_CURRVER = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

static LPCClassicWnd main_window;

static LPCClassicBitmap header_image;
static LPCClassicLabel info_text;

static LPCClassicButton ok_button;

static void GlobalEventListener(LPCClassicComponent source,
								UINT event,
								WPARAM wParam,
								LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nCmdShow)
{
	HICON icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWS)),
		  icon_small = NULL;

	main_window = new CClassicWnd(
		hInstance,
		GenerateNewClassName("RealWinVer"),
		GenerateNewClassName("RealWinVer_Client"),
		icon,
		icon_small
	);
	
	main_window->SetTitle("About Windows");

	main_window->SetMinimizable(false);
	main_window->SetResizable(false);

	main_window->SetSize(419, 329);

	RECT desktop;
	GetWindowRect(GetDesktopWindow(), &desktop);

	main_window->SetPositionRelativeTo(desktop);

	RECT client_bounds = main_window->GetClientBounds();

	header_image = new CClassicBitmap(
		hInstance,
		GenerateNewClassName("RealWinVer_HeaderImage"),
		LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WINNT))
	);

	header_image->SetSize(client_bounds.right, 77);
	header_image->SetPosition(0, 0);
	header_image->SetBitmapPosition((client_bounds.right / 2) - (307 / 2), 10);
	header_image->SetBackgroundColor(0xFFFFFF);

	main_window->AddComponent(header_image);

	NTOSInfoEx os_info;
	
	if (!GetKnownWindowsVersion(&os_info))
	{
		MessageBoxClassic(
			NULL, 
			hInstance, 
			"This program requires a valid version of Windows NT.", 
			"OSINFO Error", 
			MB_OK | MB_ICONERROR
		);
		
		return 1;
	}

	HKEY key_ntcurrver;

	if (RegOpenKeyEx(
			HKEY_LOCAL_MACHINE, 
			REG_NT_CURRVER, 
			0, 
			// For the newer versions of Windows...
			KEY_QUERY_VALUE | KEY_WOW64_64KEY, 
			&key_ntcurrver
		) != ERROR_SUCCESS)
	{
		MessageBoxClassic(
			NULL,
			hInstance,
			"Unable to retrieve Windows NT specified version info from Registry!\n\
			Perhaps you are trying to run this application on a Windows 9x Architecture?",
			"Registry Error",
			MB_OK | MB_ICONERROR
		);

		return 1;
	}

	TCHAR NT_Build_Num[16];
	DWORD NT_Build_Num_Len = ARRAYSIZE(NT_Build_Num);

	// Getting the current NT build number from the registry
	RegQueryValueEx(
		key_ntcurrver, 
		"CurrentBuildNumber", 
		NULL, 
		NULL,  
		(LPBYTE)&NT_Build_Num, 
		(LPDWORD)&NT_Build_Num_Len
	);

	TCHAR NT_Reg_Owner[32];
	DWORD NT_Reg_Owner_Len = ARRAYSIZE(NT_Reg_Owner);

	// Getting the registered owner of this product...
	RegQueryValueEx(
		key_ntcurrver, 
		"RegisteredOwner", 
		NULL, 
		NULL, 
		(LPBYTE)&NT_Reg_Owner, 
		(LPDWORD)&NT_Reg_Owner_Len
	);

	TCHAR NT_Reg_Organization[32];
	DWORD NT_Reg_Organization_Len = ARRAYSIZE(NT_Reg_Organization);

	// Getting the registered organization of this product...
	RegQueryValueEx(
		key_ntcurrver,
		"RegisteredOrganization",
		NULL,
		NULL,
		(LPBYTE)&NT_Reg_Organization,
		(LPDWORD)&NT_Reg_Organization_Len
	);

	// Deallocating the Key handle
	RegCloseKey(key_ntcurrver);
	
	MEMORYSTATUSEX mem_status;
	mem_status.dwLength = sizeof(mem_status);

	GlobalMemoryStatusEx(&mem_status);

	TCHAR text_buffer[320];

	sprintf(
		text_buffer, 
		"Microsoft (R) Windows NT %s\n\
		Version %d.%d (Build %s)\n\
		Copyright (C) 1981 - 2016 Microsoft Corp.\n \n\
		Yep, that\'s right! This is not a stupid joke!\n\
		You are actually running Windows NT!\n \n\
		This product is licensed to:\n\
		%s\n\
		%s\n \n\
		Physical memory available to Windows: %I64d KB", 
		((GetProductType() == VER_NT_SERVER) ? "Server" : "Workstation"), 
		os_info.dwMajorVersion, 
		os_info.dwMinorVersion, 
		NT_Build_Num, 
		NT_Reg_Owner, 
		NT_Reg_Organization, 
		mem_status.ullTotalPhys
	);

	info_text = new CClassicLabel(
		hInstance, 
		GenerateNewClassName("RealWinVer_Text"), 
		text_buffer
	);

	info_text->SetSize(client_bounds.right, 160);
	info_text->SetPosition(104, 91);

	main_window->AddComponent(info_text);

	ok_button = new CClassicButton(
		hInstance, 
		GenerateNewClassName("RealWinVer_OKBTN"), 
		"OK"
	);

	ok_button->SetSize(75, 23);

	ok_button->SetPosition(
		(client_bounds.right - 75) - 9, 
		(client_bounds.bottom - 23) - 9
	);

	ok_button->event_listener = GlobalEventListener;

	main_window->AddComponent(ok_button);

	return main_window->CreateAndShow();
}

void GlobalEventListener(LPCClassicComponent source,
						UINT event,
						WPARAM wParam,
						LPARAM lParam)
{
	if (source == ok_button)
	{
		main_window->Destroy();
	}
}

#endif // _PROG_REALWINVER