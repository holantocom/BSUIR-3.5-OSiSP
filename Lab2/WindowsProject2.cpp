// WindowsProject2.cpp : Defines the entry point for the application.
//

#include "Windows.h"
#include <commdlg.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "framework.h"
#include "targetver.h"
#include "WindowsProject2.h"

#define MAX_LOADSTRING 100

typedef std::vector<std::string> STRINGVECTOR;

typedef struct _TABLE
{
	INT numOfColums = 0; // Number of colums in the table
	INT numOfRows = 0;   // Number of rows in the table
	STRINGVECTOR text;    // String to be dispalayed as a table 
} TABLE;

template<typename T, typename P>
T remove_if(T beg, T end, P pred)
{
	T dest = beg;
	for (T itr = beg; itr != end; ++itr)
		if (!pred(*itr))
			*(dest++) = *itr;
	return dest;
}

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
WCHAR szFileName[MAX_LOADSTRING];               // Name of the file with strings to be showed in the table
TABLE table;                                    // Struct that contains all info about table

INT scrolledWidth = 0;      // Width of the scrolled part of the table
INT tableBottomY = 0;       // Y coordinate of the table's bottom border

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Edit(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	EditSecond(HWND, UINT, WPARAM, LPARAM);
VOID			    GetUserFileName(HWND, WCHAR[]);
VOID				LoadTextFromFile(WCHAR szFileName[], STRINGVECTOR* str);
VOID				DrawTable(HWND hWnd, RECT wndRect, HDC hdc);
VOID				DrawLine(HDC hdc, COLORREF color, int x1, int y1, int x2, int y2);
VOID			    DrawVerticalTableLines(HDC hdc, COLORREF color, INT cellSizeX, INT tableSizeY);
int					TryToPlace(std::string str, HWND hWnd, HDC hdc, RECT cellForText, int j);
VOID				RefreshWindow(HWND hWnd);
BOOL				IsScrolling(HWND hWnd, WPARAM wParam);
VOID				UpdateTable(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndEdit = NULL;
	HWND hWndButton;

	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_OPEN:
		{
			// Show open file dialog and return the file name 
			GetUserFileName(hWnd, szFileName);
			// Show the edit dialog for entering the number of columns in the table
			if (szFileName[0] != '\0' && DialogBox(hInst, MAKEINTRESOURCE(IDD_EDITBOX), hWnd, Edit))
			{
				if (szFileName[0] != '\0' && DialogBox(hInst, MAKEINTRESOURCE(IDD_EDITSECONDBOX), hWnd, EditSecond))
				{
					LoadTextFromFile(szFileName, &(table.text));
					RefreshWindow(hWnd);
				}
			}
		}
		break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = 300;
		lpMMI->ptMinTrackSize.y = 220;
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		
		if (table.numOfColums != 0 && table.numOfRows != 0) {
			RECT wndRect;
			GetClientRect(hWnd, &wndRect);

			DrawTable(hWnd, wndRect, hdc);
		}
		EndPaint(hWnd, &ps);
		
	}
	break;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			scrolledWidth = 0;
			UpdateTable(hWnd);
		}
		break;
	case WM_MOUSEWHEEL:
		if (IsScrolling(hWnd, wParam))
		{
			UpdateTable(hWnd);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for edit box
INT_PTR CALLBACK Edit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Entered string
	WCHAR lpstrColums[3];
	// Number of entered characters
	WORD cchColums;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			// Get number of characters
			cchColums = (WORD)SendDlgItemMessage(hDlg, IDEDIT, EM_LINELENGTH, (WPARAM)0, (LPARAM)0);

			//  Check number of entered characters
			if (cchColums >= 2)
			{
				MessageBox(hDlg, L"Многовато будет, мне кажется.", L"Ошибка", MB_OK);
				return (INT_PTR)FALSE;
			}
			else if (cchColums == 0)
			{
				MessageBox(hDlg, L"Ничего не введено. Повторите попытку.", L"Ошибка", MB_OK);
				return (INT_PTR)FALSE;
			}

			// Put the number of characters into first word of buffer 
			*((LPWORD)lpstrColums) = cchColums;

			// Get the characters. The size in the first word of the lpstrColumns is overwritten by the copied line
			SendDlgItemMessage(hDlg, IDEDIT, EM_GETLINE, (WPARAM)0, (LPARAM)lpstrColums);

			// Null-terminate the string 
			lpstrColums[cchColums] = 0;

			// Convert string with number of colums to int value
			table.numOfColums = _wtoi(lpstrColums);

			// Check the conversion result
			if (table.numOfColums == 0)
			{
				MessageBox(hDlg, L"Невалидные данные. Повторите попытку.", L"Ошибка", MB_OK);
				return (INT_PTR)FALSE;
			}

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for edit box
INT_PTR CALLBACK EditSecond(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Entered string
	WCHAR lpstrRows[3];
	// Number of entered characters
	WORD cchRows;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			// Get number of characters
			cchRows = (WORD)SendDlgItemMessage(hDlg, IDEDITSECOND, EM_LINELENGTH, (WPARAM)0, (LPARAM)0);

			//  Check number of entered characters
			if (cchRows >= 2)
			{
				MessageBox(hDlg, L"Многовато будет, мне кажется.", L"Ошибка", MB_OK);
				return (INT_PTR)FALSE;
			}
			else if (cchRows == 0)
			{
				MessageBox(hDlg, L"Ничего не введено. Повторите попытку.", L"Ошибка", MB_OK);
				return (INT_PTR)FALSE;
			}

			// Put the number of characters into first word of buffer 
			*((LPWORD)lpstrRows) = cchRows;

			// Get the characters. The size in the first word of the lpstrColumns is overwritten by the copied line
			SendDlgItemMessage(hDlg, IDEDITSECOND, EM_GETLINE, (WPARAM)0, (LPARAM)lpstrRows);

			// Null-terminate the string 
			lpstrRows[cchRows] = 0;

			// Convert string with number of colums to int value
			table.numOfRows = _wtoi(lpstrRows);

			// Check the conversion result
			if (table.numOfRows == 0)
			{
				MessageBox(hDlg, L"Невалидные данные. Повторите попытку.", L"Ошибка", MB_OK);
				return (INT_PTR)FALSE;
			}

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Show the open file dialog box and put the file name to the szFileName parameter 
VOID GetUserFileName(HWND hWnd, WCHAR szFileName[])
{
	// Structure that contains information used to display the open file dialog box
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	WCHAR szFile[MAX_LOADSTRING];

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile; // name of the selected file
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile); // max lenght of the file name
	ofn.lpstrFilter = L"Text\0*.txt\0"; // only text files
	ofn.nFilterIndex = 1;  // text files
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Show the open file dialog box
	GetOpenFileName(&ofn);
	// Copy the file name to the szFileName
	wcscpy_s(szFileName, wcslen(szFile) + 1, szFile);
}

// Read the text file and put it's lines to the STRINGVECTOR
void LoadTextFromFile(WCHAR szFileName[], STRINGVECTOR* str)
{
	std::ifstream t(szFileName);
	std::string string((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	string.erase(remove_if(string.begin(), string.end(), isspace), string.end());

	const int num = table.numOfColums * table.numOfRows;
	const int cellsInRow = string.size() / num;

	for (int i = 0; i < num; i++)
	{
		if (i != num - 1) {
			table.text.push_back(string.substr(i * cellsInRow, cellsInRow));
		}
		else {
			table.text.push_back(string.substr(i * cellsInRow, string.size() - (i - 1) * cellsInRow));
		}
	}
}

// Set the window to be redrawn
VOID RefreshWindow(HWND hWnd)
{
	InvalidateRect(hWnd, NULL, TRUE);
}


VOID DrawTable(HWND hWnd, RECT wndRect, HDC hdc)
{
	INT indent = 5,
		maxRowHight = 0,
		sizeOfColumn;
	PAINTSTRUCT ps;
	//HDC hdc = BeginPaint(hWnd, &ps);
	RECT rect, cellForText;
	HBRUSH brush;
	COLORREF colorText = RGB(0, 0, 0),
		colorBack = RGB(255, 255, 255),
		colorLine = RGB(0, 0, 0);

	brush = CreateSolidBrush(colorBack);
	SelectObject(hdc, brush);
	Rectangle(hdc, wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
	DeleteObject(brush);

	sizeOfColumn = wndRect.right / table.numOfColums;

	for (int i = 0; i < table.numOfRows; i++) {

		rect.top = maxRowHight;

		for (int j = 0; j < table.numOfColums; j++) {

			rect.left = sizeOfColumn * j;
			rect.right = wndRect.right / table.numOfColums * (j + 1);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, colorText);

			cellForText.top = rect.top + indent;
			cellForText.right = rect.right - indent;
			cellForText.left = rect.left + indent;

			std::string str = table.text[table.numOfColums * i + j];


			int rectBottom = TryToPlace(str, hWnd, hdc, cellForText, j);
			if (rectBottom > maxRowHight)
				maxRowHight = rectBottom;

		}

		DrawLine(hdc, colorLine, wndRect.left, maxRowHight, wndRect.right, maxRowHight);
	}

	DrawVerticalTableLines(hdc, colorLine, sizeOfColumn, maxRowHight);

	SetBkMode(hdc, OPAQUE);
	//EndPaint(hWnd, &ps);
}


int TryToPlace(std::string str, HWND hWnd, HDC hdc, RECT cellForText, int j) {
	HFONT oldFont, newFont;

	int indent = 75;

	RECT rect, winRect;
	GetWindowRect(hWnd, &winRect);

	int height = (winRect.bottom - winRect.top - indent) / table.numOfRows;

	int width = (winRect.right - winRect.left) / table.numOfColums;
	int weight = width * height / str.length();
	int charWidth = sqrt(weight / 2);
	newFont = CreateFont(charWidth*2, charWidth, 0, 0, 500, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Arial");

	oldFont = (HFONT)SelectObject(hdc, newFont);



	DrawTextA(hdc, str.c_str(), -1, &cellForText, DT_CALCRECT | DT_WORDBREAK | DT_LEFT | DT_EDITCONTROL);
	rect.right = winRect.right / table.numOfColums * (j + 1);

	cellForText.bottom = cellForText.top + height;

	DrawTextA(hdc, str.c_str(), -1, &cellForText, DT_WORDBREAK | DT_EDITCONTROL);

	return cellForText.bottom;
}

VOID DrawLine(HDC hdc, COLORREF color, int x1, int y1, int x2, int y2)
{
	HPEN pen = CreatePen(PS_INSIDEFRAME, 1, color);
	POINT pt;
	SelectObject(hdc, pen);
	MoveToEx(hdc, x1, y1, &pt);
	LineTo(hdc, x2, y2);
	DeleteObject(pen);
}

VOID DrawVerticalTableLines(HDC hdc, COLORREF color, INT cellSizeX, INT tableSizeY)
{
	for (int i = 1; i < table.numOfColums; i++) {
		DrawLine(hdc, color, i * cellSizeX, 0, i * cellSizeX, tableSizeY);
	}
}

// Set the window to be redrawn
VOID UpdateTable(HWND hWnd)
{
	InvalidateRect(hWnd, NULL, TRUE);
}

// Checks whether the table needs to be redrawn if the user rotates the mouse wheel
BOOL IsScrolling(HWND hWnd, WPARAM wParam)
{
	// Contain dimentions of the window
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	// Indicates the amount that the mouse wheel has changed
	SHORT delta = GET_WHEEL_DELTA_WPARAM(wParam);

	// The table is shorter than the window
	if (tableBottomY < clientRect.bottom)
		return false;

	// User is scrolling up but it's the upper position of the table
	if (scrolledWidth == 0 && delta > 0)
		return false;

	// The bottom border of the table coincides with the bottom border of the window 
	// and the user is scrolling the mouse wheel up
	if (scrolledWidth + clientRect.bottom == tableBottomY && delta < 0)
		return false;

	// Reached the bottom border of the table
	if (scrolledWidth + clientRect.bottom - delta >= tableBottomY && delta < 0)
	{
		scrolledWidth = tableBottomY - clientRect.bottom;
		return true;
	}

	// If the user is scrolling down, delta is negative
	scrolledWidth += (-delta);
	// Correct the value of scrolledY
	scrolledWidth = (scrolledWidth > 0) ? scrolledWidth : 0;

	return true;
}