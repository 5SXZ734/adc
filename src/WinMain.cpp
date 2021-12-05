#define UNICODE
#define _UNICODE
#include <windows.h>
#include <io.h>
#include <tchar.h> 

#include <fcntl.h>
#include <assert.h> 
#include <exception>
#include <iostream>
//#include <vld.h>

#include "qx/ConvertUTF.h"
#include "qx/MyArgs.h"


int main(int argc, char** argv);//external

static void atExitHandler(void)
{
	_CrtDumpMemoryLeaks();
} 

#ifdef _DEBUG
#define MEM_DBG	0
#endif

#if(1)
#if(MEM_DBG)
#include <crtdbg.h>
#pragma warning(disable:4074)//initializers put in compiler reserved initialization area
#pragma init_seg(compiler)//global objects in this file get constructed very early on
struct CrtBreakAllocSetter {
	CrtBreakAllocSetter() {
		_crtBreakAlloc = 4239;// <allocation number of interest>;
	}
};
CrtBreakAllocSetter g_crtBreakAllocSetter;
#endif

#endif//_DEBUG

static BOOL allocConsole(void)
{
	if (!AllocConsole())
		return FALSE;

	CONSOLE_SCREEN_BUFFER_INFO conInfo;

	// Set the screen buffer to be big enough to let us scroll text.
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
	conInfo.dwSize.Y = 500;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);

	// stdout
	int hCrt = _open_osfhandle((intptr_t)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	FILE *hf = _fdopen(hCrt, "w");
	*stdout = *hf;
	setvbuf(stdout, nullptr, _IONBF, 0);

	// stderr
	hCrt = _open_osfhandle((intptr_t)GetStdHandle(STD_ERROR_HANDLE), _O_TEXT);
	hf = _fdopen(hCrt, "w");
	*stderr = *hf;
	setvbuf(stderr, nullptr, _IONBF, 0);

	std::ios::sync_with_stdio();

	/*_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

	_CrtSetDbgFlag(_CRTDBG_CHECK_CRT_DF);*/

	return TRUE;
}

BOOL allocConsole2(void)
{
	if (!AllocConsole()) {
		// Add some error handling here.
		// You can call GetLastError() to get more info about the error.
		return FALSE;
	}

	// std::cout, std::clog, std::cerr, std::cin
	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	std::cout.clear();
	std::clog.clear();
	std::cerr.clear();
	std::cin.clear();

	// std::wcout, std::wclog, std::wcerr, std::wcin
	HANDLE hConOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	HANDLE hConIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
	SetStdHandle(STD_ERROR_HANDLE, hConOut);
	SetStdHandle(STD_INPUT_HANDLE, hConIn);
	std::wcout.clear();
	std::wclog.clear();
	std::wcerr.clear();
	std::wcin.clear();
	return TRUE;
}

// Attach output of application to parent console
BOOL attachOutputToConsole(void)
{
	if (!AttachConsole(ATTACH_PARENT_PROCESS))
		return FALSE;//Not a console application

	//redirect unbuffered STDOUT to the console
	HANDLE consoleHandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	int fdOut = _open_osfhandle((intptr_t)consoleHandleOut, _O_TEXT);
	FILE *fpOut = _fdopen(fdOut, "w");
	*stdout = *fpOut; setvbuf(stdout, nullptr, _IONBF, 0);

	//redirect unbuffered STDERR to the console
	HANDLE consoleHandleError = GetStdHandle(STD_ERROR_HANDLE);
	int fdError = _open_osfhandle((intptr_t)consoleHandleError, _O_TEXT);
	FILE *fpError = _fdopen(fdError, "w");
	*stderr = *fpError;
	setvbuf(stderr, nullptr, _IONBF, 0);
	return TRUE;
}

//Send the "enter" to the console to release the command prompt on the parent console
void sendEnterKey(void) 
{
	INPUT ip;
	// Set up a generic keyboard event. 
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key 
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	//Send the "Enter" key 
	ip.ki.wVk = 0x0D; // virtual-key code for the "Enter" key 
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the "Enter" key 
	ip.ki.dwFlags = KEYEVENTF_KEYUP;// KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	(void)nCmdShow;
	(void)hPrevInstance;
	(void)hInstance;
	(void)lpCmdLine;

#if(0)
	void __test();
	allocConsole2();
	__test();
	//std::getchar();
	system("pause");
	return 1;
#endif


#if(MEM_DBG)
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(606);
	atexit(atExitHandler);
#endif

/*#ifdef _DEBUG
	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
		MessageBox(nullptr, _T("Here"), _T(PRODUCT_NAME), MB_OK);
#endif*/


	size_t bufSize = _tcslen(GetCommandLine()) * 2;
	char* cmdLineUtf8 = new char[bufSize];

	UTF16toUTF8(GetCommandLine(), cmdLineUtf8, (unsigned)bufSize);
	MyArgs my(cmdLineUtf8);

#if(0)//see: QT_QPA_PLATFORM
	if (my.Find("-b") < 0)
		if (my.Find("-platform") < 0)
		{
			char buf1[32] = "-platform";
			char buf2[] = "windows";
			my.Insert(1, buf1);
			my.Insert(2, buf2);
		}
#endif

	main(my.argc, my.argv);

	delete cmdLineUtf8;

	return 0;
}





















