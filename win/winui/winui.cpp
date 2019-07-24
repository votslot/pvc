//


#include "stdio.h"
#include <iostream>
#include "windows.h"
#include "..\..\app\app-events.h"

#pragma warning( disable : 4311)
#pragma warning( disable : 4302)

static pcrapp::IAppEvents *psApp = NULL;
WNDPROC prevWndProc;
enum {
	OPEN_DIALOG_ID = 124,
	HELP_ID = 125,
	VIEW_ID = 126,
	RUN_TEST_ID = 127,
	CAMERA_PIVOT = 128,
	CAMERA_FIRST_PERSON = 128
};

static void OpenFileDialog()
{
	OPENFILENAME ofn;
	WCHAR szFileName[MAX_PATH]=L"";
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn); 
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = L"las Files (*.las)\0*.las\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"txt";
	if (GetOpenFileName(&ofn))
	{
		std::wstring ws(ofn.lpstrFile);
		std::string str(ws.begin(), ws.end());
		std::cout << str.c_str() << "\n";
		if(psApp) psApp->openLasFile(str.c_str());
	}
}

LRESULT CALLBACK myNewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int nn = 0;

	switch (uMsg)
	{
	case WM_COMMAND:
	{
		switch (wParam)
		{
		    case OPEN_DIALOG_ID:
				OpenFileDialog();
			break;
			case RUN_TEST_ID:
			{
				if( psApp) psApp->testCloud();
			}
			break;
			case HELP_ID:
				std::cout << "HELP!" << std::endl;
			break;
		}
	}
	break;

	case WM_PAINT:
	{
		nn++;
	}
	break;
	case WM_LBUTTONDOWN:
		printf("Button down!\n");
		break;
	}
	return CallWindowProc(prevWndProc, hwnd, uMsg, wParam, lParam);
}

void AddUI(HWND hwnd, pcrapp::IAppEvents *pApp)
{
	psApp = pApp;
	HMENU hMenu = CreateMenu();
	{
		HMENU hSubMenu = CreatePopupMenu();
		AppendMenu(hSubMenu, MF_STRING, OPEN_DIALOG_ID, L"&Open");
		AppendMenu(hSubMenu, MF_STRING, RUN_TEST_ID, L"&RunTest");
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, L"&File");
	}
	{
		HMENU hSubMenu = CreatePopupMenu();
		AppendMenu(hSubMenu, MF_STRING, HELP_ID, L"&About");
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, L"&Help");
	}
	{
		HMENU hSubMenu = CreatePopupMenu();
		AppendMenu(hSubMenu, MF_STRING, VIEW_ID, L"&Rgb");
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, L"&View");
	}
	{
		HMENU hSubMenu = CreatePopupMenu();
		AppendMenu(hSubMenu, MF_STRING, CAMERA_PIVOT, L"&Pivot");
		AppendMenu(hSubMenu, MF_STRING|MF_CHECKED, CAMERA_FIRST_PERSON, L"&FirstPerson");
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, L"Camera");
	}
	
	SetMenu(hwnd, hMenu);
	prevWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)&myNewWndProc); 
}

