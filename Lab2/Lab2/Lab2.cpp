#include "stdafx.h"
#include "Lab2.h"

#define MAX_LOADSTRING 100

#define PEN_STYLE PS_SOLID
#define PEN_WIDTH 1
#define PEN_COLOR 0x000000
#define MAX_COLUMNS 10

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16
#define INDENT 5

typedef std::vector<std::string> STRINGVECTOR;
typedef std::vector<STRINGVECTOR> STRINGTABLE;

typedef struct _TABLE
{
	INT numOfColums = 0; 
	INT numOfRows = 0;   
	STRINGTABLE text;
} TABLE;

HINSTANCE hInst;                               
WCHAR szTitle[MAX_LOADSTRING];                 
WCHAR szWindowClass[MAX_LOADSTRING];           
WCHAR szFileName[MAX_LOADSTRING];              
TABLE table;                                   

INT scrolledWidth = 0; 
INT tableBottomY = 0;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, INT);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Edit(HWND, UINT, WPARAM, LPARAM);
VOID			    GetUserFileName(HWND, WCHAR []);
BOOL				LoadTextFromFile(WCHAR szFileName[], STRINGTABLE *table, INT numOfColumns);
VOID				DrawTable(HWND hWnd, HDC hdc, TABLE table);
VOID				WriteText(HDC hdc, RECT clientRect, TABLE table, INT columnWidth);
VOID				WriteRow(HDC hdc, RECT clientRect, TABLE table, INT rowIndex, INT columnWidth);
VOID				RefreshWindow(HWND hWnd);
INT					GetNumOfCharsToWrite(HDC hdc, INT columnWidth, std::wstring str);
BOOL				IsScrolling(HWND hWnd, WPARAM wParam);
VOID				UpdateTable(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB2));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)){
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd) return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndEdit = NULL;

    switch (message){
		case WM_COMMAND:
			{
				int wmId = LOWORD(wParam);

				switch (wmId){
					case IDM_OPEN:
						{
							GetUserFileName(hWnd, szFileName);
							if (szFileName[0] != '\0' && DialogBox(hInst, MAKEINTRESOURCE(IDD_EDITBOX), hWnd, Edit)){
								table.numOfRows = LoadTextFromFile(szFileName, &(table.text), table.numOfColums);
								RefreshWindow(hWnd);
							}
						}
						break;            
					case IDM_EXIT:
						DestroyWindow(hWnd);
						break;
					default:
						return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				if (table.numOfColums != 0) DrawTable(hWnd, hdc, table);
				EndPaint(hWnd, &ps);
			}
			break;
		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED){
				scrolledWidth = 0;
				UpdateTable(hWnd);
			}
			break;
		case WM_MOUSEWHEEL:
			if (IsScrolling(hWnd, wParam)) UpdateTable(hWnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


INT_PTR CALLBACK Edit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	WCHAR lpstrColums[3];   
	WORD cchColums;
	UNREFERENCED_PARAMETER(lParam);

	switch (message){
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK){			
				cchColums = (WORD)SendDlgItemMessage(hDlg, IDEDIT, EM_LINELENGTH, (WPARAM)0, (LPARAM)0);
			
				if (cchColums == 0) {
					MessageBox(hDlg, L"С пустыми данными программы не запускают",	L"Error", MB_OK);
					return (INT_PTR)FALSE;
				}	
				*((LPWORD)lpstrColums) = cchColums;

				SendDlgItemMessage(hDlg, IDEDIT, EM_GETLINE, (WPARAM)0,	(LPARAM)lpstrColums);
		
				lpstrColums[cchColums] = 0;
						
				table.numOfColums = _wtoi(lpstrColums);
		
				if (table.numOfColums == 0){
					MessageBox(hDlg, L"Нужно больше колонок", L"Error", MB_OK);
					return (INT_PTR)FALSE;
				}
				if (table.numOfColums > MAX_COLUMNS) {
					MessageBox(hDlg, L"Чересчур мне кажется", L"Error", MB_OK);
					return (INT_PTR)FALSE;
				}

				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
 
VOID GetUserFileName(HWND hWnd, WCHAR szFileName[])
{	
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	WCHAR szFile[MAX_LOADSTRING];

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile; 
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile); 
	ofn.lpstrFilter = L"Text\0*.txt\0";
	ofn.nFilterIndex = 1;  
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	GetOpenFileName(&ofn);	
	wcscpy_s(szFileName, wcslen(szFile) + 1, szFile);
}

INT LoadTextFromFile(WCHAR szFileName[], STRINGTABLE *table, INT numOfColumns)
{

	std::wstring ws(szFileName);
	std::string fileName(ws.begin(), ws.end());
	std::ifstream fin(fileName.c_str());
	if (!fin) return false;

	std::string str;
	STRINGVECTOR row;
	INT rowsCount = 0;     
	
	while (!fin.eof()){
		for (int i = 0; i < numOfColumns; i++){
			if (fin.eof()) str = ""; else std::getline(fin, str);			
			row.push_back(str);
		}
		
		(*table).push_back(row);
		rowsCount++;
		row.clear();
	}

	fin.close();
	return rowsCount;
}

VOID RefreshWindow(HWND hWnd)
{
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	InvalidateRect(hWnd, &clientRect, TRUE);
}

VOID DrawTable(HWND hWnd, HDC hdc, TABLE table)
{	
	HPEN pen = CreatePen(PEN_STYLE, PEN_WIDTH, PEN_COLOR);
	HGDIOBJ oldPen = SelectObject(hdc, pen);

	HFONT font = CreateFont(CHAR_HEIGHT, CHAR_WIDTH, 0, 0, FW_NORMAL, false, false, false, 0, 0, 0, 0, 0, NULL);
	HGDIOBJ oldFont = SelectObject(hdc, font);
	
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	int columnWidth = clientRect.right / table.numOfColums;
	int x = 0;

	WriteText(hdc, clientRect, table, columnWidth);

	for (int i = 0; i < table.numOfColums - 1; i++){
		x += columnWidth;
		MoveToEx(hdc, x, 0, NULL);
		LineTo(hdc, x, tableBottomY);
	}
	SelectObject(hdc, oldPen);
	SelectObject(hdc, oldFont);
}

VOID WriteText(HDC hdc, RECT clientRect, TABLE table, INT columnWidth)
{
	for (int i = 0; i < table.numOfRows; i++) WriteRow(hdc, clientRect, table, i, columnWidth);
}

VOID WriteRow(HDC hdc, RECT clientRect, TABLE table, INT rowIndex, INT columnWidth)
{
	int maxY = tableBottomY;

	for (int i = 0; i < table.numOfColums; i++){

		SIZE textSize;   
		std::string strObj = table.text[rowIndex][i];
		std::wstring str(strObj.begin(), strObj.end());		
		GetTextExtentPoint32(hdc, str.c_str(), str.length(), &textSize);

		if (textSize.cx <= columnWidth - 2 * INDENT){			
			int x = columnWidth * i + INDENT;
			int y = tableBottomY - scrolledWidth + INDENT;
			TextOut(hdc, x, y, str.c_str(), str.length());
			
			if (tableBottomY + textSize.cy > maxY)maxY = tableBottomY + textSize.cy;
		} else {
			SIZE stringSize;
			std::wstring substr = str;
			std::wstring restStr;

			int strWidth = textSize.cx;
			int restWidth = 0;
			int strLength = str.length();
			
			int y = tableBottomY + INDENT;
			do {
				int substrLen = GetNumOfCharsToWrite(hdc, columnWidth, str);
				substr = str.substr(0, substrLen);

				if (strLength >= substrLen){
					restStr = str.substr(substrLen, strLength);
					GetTextExtentPoint32(hdc, substr.c_str(), substrLen, &stringSize);
					restWidth = strWidth - stringSize.cx;
					TextOut(hdc, columnWidth * i + INDENT, y - scrolledWidth, substr.c_str(), substr.length());

					y += stringSize.cy + INDENT;
					str = restStr;
					strLength = str.length();
					strWidth = restWidth;
				}
			} while (restWidth > columnWidth - 2 * INDENT);

			if (restWidth > 0) TextOut(hdc, columnWidth * i + INDENT, y - scrolledWidth, str.c_str(), str.length());
			if (y + stringSize.cy > maxY) maxY = y + stringSize.cy;
		}
	}

	tableBottomY = maxY + INDENT;
	MoveToEx(hdc, clientRect.left, tableBottomY - scrolledWidth, NULL);
	LineTo(hdc, clientRect.right, tableBottomY - scrolledWidth);
}


INT GetNumOfCharsToWrite(HDC hdc, INT columnWidth, std::wstring str)
{
	SIZE stringSize;
	INT charCount = (columnWidth - 2 * INDENT) / CHAR_WIDTH;
	std::wstring substr = str.substr(0, charCount);
	GetTextExtentPoint32(hdc, substr.c_str(), substr.length(), &stringSize);

	while (stringSize.cx < columnWidth - 2 * INDENT && charCount < str.length()){
		charCount++;
		substr = str.substr(0, charCount);
		GetTextExtentPoint32(hdc, substr.c_str(), substr.length(), &stringSize);
	}
	while (stringSize.cx > columnWidth - 2 * INDENT){
		charCount--;
		substr = str.substr(0, charCount);
		GetTextExtentPoint32(hdc, substr.c_str(), substr.length(), &stringSize);
	}

	if (charCount == 0) return 1;

	return charCount;
}


VOID UpdateTable(HWND hWnd)
{
	tableBottomY = 0;
	InvalidateRect(hWnd, NULL, TRUE);
}


BOOL IsScrolling(HWND hWnd, WPARAM wParam)
{

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	SHORT delta = GET_WHEEL_DELTA_WPARAM(wParam);

	if (tableBottomY < clientRect.bottom) return false;
	if (scrolledWidth == 0 && delta > 0) return false;
	if (scrolledWidth + clientRect.bottom == tableBottomY && delta < 0) return false;

	if (scrolledWidth + clientRect.bottom - delta >= tableBottomY && delta < 0){
		scrolledWidth = tableBottomY - clientRect.bottom;
		return true;
	}

	scrolledWidth += (-delta);
	scrolledWidth = (scrolledWidth > 0) ? scrolledWidth : 0;

	return true;
}