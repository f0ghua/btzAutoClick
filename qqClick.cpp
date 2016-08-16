#include "stdafx.h"
#include "resource.h"

#define TRAYICONID  1//             ID number for the Notify Icon
#define SWM_TRAYMSG WM_APP//        the message ID sent to our window

#define SWM_SHOW    WM_APP + 1//    show the window
#define SWM_HIDE    WM_APP + 2//    hide the window
#define SWM_EXIT    WM_APP + 3//    close the window

// Global Variables:
HINSTANCE       hInst;  // current instance
NOTIFYICONDATA  niData; // notify icon data
HHOOK myhook;

#define MSGBOX_POS_X    642
#define MSGBOX_POS_Y    435

const int CLICKPAUSE = 500; //Pause between Clicks in ms

const char tercentMainClass1[] = "TXGuiFoundation";
const char tercentMainClass2[] = "Internet Explorer_Server";

static char threadTerminate = FALSE;

// Forward declarations of functions included in this code module:
BOOL                InitInstance(HINSTANCE, int);
BOOL                OnInitDialog(HWND hWnd);
void                ShowContextMenu(HWND hWnd);
ULONGLONG           GetDllVersion(LPCTSTR lpszDllName);

INT_PTR CALLBACK    DlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    MSG msg;
    HACCEL hAccelTable;

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow)) return FALSE;

    // 设置键盘全局监听
    myhook = SetWindowsHookEx(
        WH_KEYBOARD_LL, // 监听类型【键盘】
        KeyboardProc,   // 处理函数
        hInstance,      // 当前实例句柄
        0               // 监听窗口句柄(NULL为全局监听)
    );

    if(myhook == NULL)
    {
        //wsprintf(text, "hook keyboard error : %d \n", GetLastError());
        //MessageBox(hwnd, text, TEXT("error"), MB_OK);
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDI_ICON1);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)||
            !IsDialogMessage(msg.hwnd,&msg) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//  Initialize the window and tray icon
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    // prepare for XP style controls
    InitCommonControls();

     // store instance handle and create dialog
    hInst = hInstance;
    HWND hWnd = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_DIALOG1),
        NULL, (DLGPROC)DlgProc );
    if (!hWnd) return FALSE;

    // Fill the NOTIFYICONDATA structure and call Shell_NotifyIcon

    // zero the structure - note:   Some Windows funtions require this but
    //                              I can't be bothered which ones do and
    //                              which ones don't.
    ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

    // get Shell32 version number and set the size of the structure
    //      note:   the MSDN documentation about this is a little
    //              dubious and I'm not at all sure if the method
    //              bellow is correct
    ULONGLONG ullVersion = GetDllVersion(_T("Shell32.dll"));
    if(ullVersion >= MAKEDLLVERULL(5, 0,0,0))
        niData.cbSize = sizeof(NOTIFYICONDATA);
    else niData.cbSize = NOTIFYICONDATA_V2_SIZE;

    // the ID number can be anything you choose
    niData.uID = TRAYICONID;

    // state which structure members are valid
    niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

    // load the icon
    niData.hIcon = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(IDI_ICON1),
        IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);

    // the window to send messages to and the message to send
    //      note:   the message value should be in the
    //              range of WM_APP through 0xBFFF
    niData.hWnd = hWnd;
    niData.uCallbackMessage = SWM_TRAYMSG;

    // tooltip message
    lstrcpyn(niData.szTip, _T("Time flies like an arrow but\n   fruit flies like a banana!"), sizeof(niData.szTip)/sizeof(TCHAR));

    Shell_NotifyIcon(NIM_ADD,&niData);

    // free icon handle
    if(niData.hIcon && DestroyIcon(niData.hIcon))
        niData.hIcon = NULL;

    // call ShowWindow here to make the dialog initially visible

    return TRUE;
}

ULONGLONG GetDllVersion(LPCTSTR lpszDllName)
{
    ULONGLONG ullVersion = 0;
    HINSTANCE hinstDll;
    hinstDll = LoadLibrary(lpszDllName);
    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");
        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;
            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);
            hr = (*pDllGetVersion)(&dvi);
            if(SUCCEEDED(hr))
                ullVersion = MAKEDLLVERULL(dvi.dwMajorVersion, dvi.dwMinorVersion,0,0);
        }
        FreeLibrary(hinstDll);
    }
    return ullVersion;
}

// Message handler for the app
INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;

    switch (message)
    {
    case SWM_TRAYMSG:
        switch(lParam)
        {
        case WM_LBUTTONDBLCLK:
            ShowWindow(hWnd, SW_RESTORE);
            break;
        case WM_RBUTTONDOWN:
        case WM_CONTEXTMENU:
            ShowContextMenu(hWnd);
        }
        break;
    case WM_SYSCOMMAND:
        if((wParam & 0xFFF0) == SC_MINIMIZE)
        {
            ShowWindow(hWnd, SW_HIDE);
            return 1;
        }
       // else if(wParam == IDM_ABOUT)
       //     DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
        break;
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);

        switch (wmId)
        {
        case SWM_SHOW:
            ShowWindow(hWnd, SW_RESTORE);
            break;
        case SWM_HIDE:
        case IDOK:
            ShowWindow(hWnd, SW_HIDE);
            break;
        case SWM_EXIT:
            DestroyWindow(hWnd);
            break;
        //case IDM_ABOUT:
            //DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
          //  break;
        }
        return 1;
    case WM_INITDIALOG:
        return OnInitDialog(hWnd);
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        niData.uFlags = 0;
        Shell_NotifyIcon(NIM_DELETE,&niData);
        PostQuitMessage(0);
        break;
    }
    return 0;
}

BOOL OnInitDialog(HWND hWnd)
{
    HMENU hMenu = GetSystemMenu(hWnd,FALSE);
    if (hMenu)
    {
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        //AppendMenu(hMenu, MF_STRING, IDM_ABOUT, _T("About"));
    }
    HICON hIcon = (HICON)LoadImage(hInst,
        MAKEINTRESOURCE(IDI_ICON1),
        IMAGE_ICON, 0,0, LR_SHARED|LR_DEFAULTSIZE);
    SendMessage(hWnd,WM_SETICON,ICON_BIG,(LPARAM)hIcon);
    SendMessage(hWnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon);
    return TRUE;
}

// Name says it all
void ShowContextMenu(HWND hWnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    if(hMenu)
    {
        if( IsWindowVisible(hWnd) )
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_HIDE, _T("Hide"));
        else
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOW, _T("Show"));
        InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, _T("Exit"));

        // note:    must set window to the foreground or the
        //          menu won't disappear when it should
        SetForegroundWindow(hWnd);

        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN,
            pt.x, pt.y, 0, hWnd, NULL );
        DestroyMenu(hMenu);
    }
}

void LeftClick ( const int x, const int y)
{
    POINT pos;
    GetCursorPos(&pos);
    SetCursorPos(x,y);

    mouse_event (MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0 );

    SetCursorPos(pos.x, pos.y);
}

void LeftDoubleClick ( const int x, const int y)
{
    POINT pos;
    GetCursorPos(&pos);
    SetCursorPos(x,y);

    mouse_event (MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0 );
    mouse_event (MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0 );

    SetCursorPos(pos.x, pos.y);
}

DWORD WINAPI AutoClickThreadProc(LPVOID lpThreadParameter)
{
    POINT pos;
    HWND hwndMain, hwnd;
    char className[32];
    int x, y;

    GetCursorPos(&pos);
    x = (int) pos.x;
    y = (int) pos.y;

    hwndMain = WindowFromPoint(pos);

    while(!threadTerminate)
    {
        LeftDoubleClick(x, y);

        Sleep(CLICKPAUSE);

        hwnd = GetForegroundWindow();
        GetClassName(hwnd, className, sizeof(className));
        if ((hwnd == hwndMain)||
            (!strncmp(className, tercentMainClass2, sizeof(tercentMainClass2))))
        {
            //printf("finished\n");
            break;
        }
        else
        {
            LeftClick(MSGBOX_POS_X, MSGBOX_POS_Y);
        }

        Sleep(CLICKPAUSE);
    }

    ExitThread(0);
}

/****************************************************************
  WH_KEYBOARD hook procedure
  鍵盤钩子处理过程
 ****************************************************************/
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
    const char *info = NULL;
    char text[64], data[32];
    static HANDLE hThread = NULL;

    PAINTSTRUCT ps;

    if (nCode >= 0)
    {
        if      (wParam == WM_KEYDOWN)      info = "Normal Key Up";
        else if (wParam == WM_KEYUP)        info = "Normal Key Down";
        else if (wParam == WM_SYSKEYDOWN)   info = "System Key Up";
        else if (wParam == WM_SYSKEYUP)     info = "System Key Down";

        if ((lParam&0x80000000) == 0)
        {
            DWORD tid;

            if ((GetKeyState(VK_CONTROL) < 0) && (p->vkCode == (int)('E')))
            {
                threadTerminate = FALSE;

                if (hThread == NULL)
                hThread = CreateThread(
                    NULL,       // 不能被子进程继承
                    0,          // 默认堆栈大小
                    AutoClickThreadProc, // 线程调用函数过程
                    NULL,   // 传递参数
                    0,          // 创建后立即执行
                    &tid        // 保存创建后的线程ID
                    );
            }
            else if ((GetKeyState(VK_CONTROL) < 0) && (p->vkCode == (int)('D')))
            {
                threadTerminate = TRUE;
                if (hThread)
                {
                    CloseHandle(hThread);
                    hThread = NULL;
                }
            }
        }
    }

    return CallNextHookEx(myhook, nCode, wParam, lParam);
}

