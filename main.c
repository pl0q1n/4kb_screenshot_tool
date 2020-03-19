#include <windows.h>
#include <tchar.h>

#define TITLE_SIZE 1024
int __cdecl abs(int i)
{
	return i < 0 ? -i : i;
}

size_t __cdecl wcslen(const wchar_t* s)
{
	size_t len = 0;
	while (s[len] != L'\0')
	{
		if (s[++len] == L'\0')
			return len;
		if (s[++len] == L'\0')
			return len;
		if (s[++len] == L'\0')
			return len;
		++len;
	}
	return len;
}

void* __cdecl memset(void*, int, UINT);
#pragma intrinsic(memset)

#pragma function(memset)
void* __cdecl memset(void* s, int c, UINT len) {
	unsigned char* dst = s;
	while (len > 0) {
		*dst = (unsigned char)c;
		dst++;
		len--;
	}
	return s;
}

void* __cdecl memcpy(void* dst, const void* src, unsigned int cnt)
{
	char* pszDest = (char*)dst;
	const char* pszSource = (const char*)src;
	if ((pszDest != NULL) && (pszSource != NULL))
	{
		while (cnt) //till cnt
		{
			//Copy byte by byte
			*(pszDest++) = *(pszSource++);
			--cnt;
		}
	}
	return dst;
}

LPWSTR __cdecl _itow(
	int value, /* [I] Value to be converted */
	LPWSTR str, /* [O] Destination for the converted value */
	int radix)  /* [I] Number base for conversion */
{
	ULONG val = value;
	WCHAR buffer[33] = { '\0' };
	PWCHAR pos;
	WCHAR digit;

	pos = &buffer[32];
	*pos = '\0';
	do {
		digit = val % radix;
		val = val / radix;
		if (digit < 10) {
			*--pos = '0' + digit;
		}
		else {
			*--pos = 'a' + digit - 10;
		} /* if */
	} while (val != 0L);

	if (str != NULL) {
		memcpy(str, pos, (&buffer[32] - pos + 1) * sizeof(WCHAR));
	}
	return str;
}

void* __cdecl malloc(size_t n)
{
	void* pv = HeapAlloc(GetProcessHeap(), 0, n);
	return pv;
}

void __cdecl free(void* p)
{
	if (p == NULL) return;
	HeapFree(GetProcessHeap(), 0, p);
}

int __cdecl atoi(const char* str)
{
	int res = 0; // Initialize result 
	int i = 0; // Initialize index of first digit 

	// Iterate through all digits and update the result 
	for (; str[i] != '\0'; ++i)
		res = res * 10 + (str[i] - '0');

	// Return result with sign 
	return res;
}

int __cdecl wtoi(LPCWSTR str)
{
	int res = 0; // Initialize result 
	int i = 0; // Initialize index of first digit 

	// Iterate through all digits and update the result 
	for (; str[i] != '\0' && str[i] != '\r' && str[i] != '\n'; ++i)
		res = res * 10 + str[i] - '0';

	// Return result with sign 
	return res;
}

#pragma warning(disable:4996)
typedef struct WString {
	unsigned int size;
	WCHAR* buffer;
} WString;

LPCTSTR get_ptr(WString* str) {
	return str->buffer;
}

void init_wstring(WString* string, int size, WCHAR* inp) {
	string->buffer = malloc(size * sizeof(WCHAR));
	memcpy(string->buffer, inp, sizeof(WCHAR) * size);
	string->size = size;
}

void destroy_wstring(WString* string) {
	free(string->buffer);
}

typedef struct WStringVec {
	unsigned int capacity;
	unsigned int size;
	WString* buffer;
	HWND* hwnd_list;
} WStringVec;

void init_wstringvec(WStringVec* vec) {
	vec->capacity = 0;
	vec->size = 0;
	vec->buffer = NULL;
	vec->hwnd_list = NULL;
}

void destroy_wstringvec(WStringVec* vec) {
	free(vec->buffer);
	free(vec->hwnd_list);
}

void reserve(unsigned int cap, WStringVec* vec) {
	if (cap < vec->capacity) {
		return;
	}
	WString* new_buffer = malloc(cap * sizeof(WString));
	HWND* new_hwnd_list = malloc(cap * sizeof(HWND));
	for (auto i = 0; i < vec->size; i++) {
		new_buffer[i] = vec->buffer[i];
		new_hwnd_list[i] = vec->hwnd_list[i];
	}
	if (vec->buffer && vec->hwnd_list) {
		free(vec->buffer);
		free(vec->hwnd_list);
	}
	vec->buffer = new_buffer;
	vec->hwnd_list = new_hwnd_list;
	vec->capacity = cap;
}

void push_back(WStringVec* vec, WString* string, HWND* hwnd) {
	// need to reserve some memory
	if (vec->capacity <= vec->size) {
		reserve(vec->size + 10, vec);
	}
	vec->size++;
	vec->buffer[vec->size - 1] = *string;
	vec->hwnd_list[vec->size - 1] = *hwnd;
}

LPCTSTR get_str(unsigned int ind, WStringVec* vec) {
	return get_ptr(&vec->buffer[ind]);
}

HWND* get_HWND(unsigned int ind, WStringVec* vec) {
	return vec->hwnd_list[ind];
}

unsigned int get_size(WStringVec* vec) {
	return vec->size;
}


BOOL CALLBACK get_win_names(HWND hwnd, LPARAM lParam) {
	WCHAR windowTitle[TITLE_SIZE];

	GetWindowTextW(hwnd, windowTitle, TITLE_SIZE);

	// probably this func doesn't support UTF strings
	int length = GetWindowTextLength(hwnd);
	if (length == 0) {
		return TRUE;
	}
	WString title;
	init_wstring(&title, length, windowTitle);

	WStringVec* titles = (WStringVec*)(lParam);
	push_back(titles, &title, &hwnd);

	return TRUE;
}

void get_bit_map(HWND* hwnd)
{
	ShowWindow(hwnd, SW_SHOW);
	HDC hScreen = GetDC(hwnd);

	RECT rcCli;
	GetClientRect(WindowFromDC(hScreen), &rcCli);
	if (!hScreen) {
		char err_msg[] = "Can't get device context";
		DWORD writen = 0;
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), err_msg, sizeof(err_msg), &writen, NULL);
	}
	else {
		HDC hDC = CreateCompatibleDC(hScreen);
		HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, abs(rcCli.right - rcCli.left), abs(rcCli.top - rcCli.bottom));
		HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
		BOOL bRet = BitBlt(hDC, 0, 0, abs(rcCli.right - rcCli.left), abs(rcCli.top - rcCli.bottom), hScreen, 0, 0, SRCCOPY);
		ShowWindow(hwnd, SW_HIDE);

		// save bitmap to clipboard
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, hBitmap);
		CloseClipboard();

		// clean up
		SelectObject(hDC, old_obj);
		DeleteDC(hDC);
		ReleaseDC(hwnd, hScreen);
		DeleteObject(hBitmap);
	}
}

int main()
{
	WStringVec window_names;
	init_wstringvec(&window_names);
	DWORD written = 0;
	EnumWindows(get_win_names, (LPARAM)(&window_names));
	TCHAR buf[32];
	for (auto i = 0; i < get_size(&window_names); i++) {
		// print via WriteConsoleOutSymbols
		auto ind = (LPCTSTR)(_itot(i, buf, 10));
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), ind, _tcslen(ind), &written, NULL);
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), L". ", _tcslen(L". "), &written, NULL);
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), get_str(i, &window_names), _tcslen(get_str(i, &window_names)), &written, NULL);
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), L"\n", _tcslen(L"\n"), &written, NULL);
	}

	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), L"Enter window id: ", _tcslen(L"Enter window id: "), &written, NULL);

	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	TCHAR szBuffer[10];
	int length = 10;
	int read = 0;
	ReadConsole(hIn, szBuffer, length, &read, NULL);

	auto inp = wtoi(szBuffer);
	get_bit_map(get_HWND(inp, &window_names));
	destroy_wstringvec(&window_names);
}