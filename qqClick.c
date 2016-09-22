#include "stdafx.h"
#include "resource.h"
#include <stdio.h>
#include "utility.h"

#define TRAYICONID  1//             ID number for the Notify Icon
#define SWM_TRAYMSG WM_APP//        the message ID sent to our window

#define SWM_SHOW    WM_APP + 1//    show the window
#define SWM_HIDE    WM_APP + 2//    hide the window
#define SWM_EXIT    WM_APP + 3//    close the window
#define SWM_ZONE_QQ WM_APP + 4//    choose the battlezone
#define SWM_ZONE_HF WM_APP + 5//    choose the battlezone
#
// Global Variables:
HINSTANCE       hInst;  // current instance
NOTIFYICONDATA  niData; // notify icon data
HHOOK myhook;

#define MSGBOX_POS_X    642
#define MSGBOX_POS_Y    435

#define ZONE_HAOFANG    1
#define ZONE_QQ         2

static int g_zoneCurrent = ZONE_QQ;
static UINT g_timerId = 0;
static POINT roomItemPos = {0,0};

const int CLICKPAUSE = 500; //Pause between Clicks in ms

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
    lstrcpyn(niData.szTip, _T("BattleZone Clicker"), sizeof(niData.szTip)/sizeof(TCHAR));

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
        case SWM_ZONE_QQ:
            g_zoneCurrent = ZONE_QQ;
            break;
        case SWM_ZONE_HF:
            g_zoneCurrent = ZONE_HAOFANG;
            break;
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
        if(g_zoneCurrent == ZONE_HAOFANG)
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_ZONE_QQ, _T("HAOFANG(switch to QQ)"));
        else // ZONE_QQ
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_ZONE_HF, _T("QQ(switch to HAOFANG)"));
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

const char tercentMainClass[] = "TXGuiFoundation";
const char explorerClass[] = "Internet Explorer_Server";
const char enteredLeftClass[] = "AfxWnd42";

// 大厅: L: AfxWnd32/SysTreeView32/SysListView32/#32770 (对话框)
// 进入房间 #32770 (对话框)
//  L: Button C: 停止
// 浩方提示对话框 L: #32770 (对话框), C: 浩方电竞平台
//  取消按钮 L: Button, C: 取消
// 进入后: L: RichEdit20A/Internet Explorer_Server/#32770 (对话框)
//
// hwnd = FindWindow("TXGuiFoundation", " 腾讯对战平台");
VOID CALLBACK AutoClickThreadProc_HF(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
    POINT pos;
    RECT rc;
    HWND hwndMain, tempHwnd, fgHwnd;
    char className[32], windowName[128];
    int x, y;

    if ((fgHwnd = GetForegroundWindow()) == NULL)
        return;
    GetClassName(fgHwnd, className, sizeof(className));
#ifndef F_NO_DEBUG
    GetWindowText(fgHwnd, windowName, sizeof(windowName));
    printf("fgHwnd = %08x, className = %s, windowName = <%s>\n", fgHwnd, className, windowName);
#endif
    if ((strstr(className, "Afx:400000:3:10003:1900010") == NULL)&&
        (strstr(className, "32770") == NULL))
    {
        printf("We don't find the HF window\n");
        return;
    }
    GetWindowRect(fgHwnd, &rc);
    printf("left = %d, right = %d, top = %d, bottom = %d\n", rc.left, rc.right, rc.top, rc.bottom);

    pos.x = (rc.left+rc.right)/3;
    pos.y = (rc.top+rc.bottom)/2;
    hwndMain = WindowFromPoint(pos);
    GetClassName(hwndMain, className, sizeof(className));
#ifndef F_NO_DEBUG
    GetWindowText(hwndMain, windowName, sizeof(windowName));
    printf("hwndMain = %08x, className = %s, windowName = <%s>\n", hwndMain, className, windowName);
#endif
    if (!strncmp(className, enteredLeftClass, sizeof(enteredLeftClass)))
    {
#ifndef F_NO_DEBUG
        printf("!!!!success, stop the timer\n");
#endif
        KillTimer(hwnd, idEvent);
        g_timerId = 0;
        return;
    }

    if ((int)roomItemPos.x == 0)
    {
        GetCursorPos(&roomItemPos);
    }
#ifndef F_NO_DEBUG
    GetCursorPos(&pos);
    printf("HF: x = %d, y = %d, xx = %d, yy = %d\n", pos.x, pos.y, roomItemPos.x, roomItemPos.y);
#endif

    tempHwnd = WindowFromPoint(roomItemPos);
    //hwnd = GetForegroundWindow();
    if (tempHwnd == NULL)
    {
        return;
    }
    GetClassName(tempHwnd, className, sizeof(className));
#ifndef F_NO_DEBUG
    GetWindowText(tempHwnd, windowName, sizeof(windowName));
    printf("tempHwnd = %08x, className = %s, windowName = %s\n", tempHwnd, className, windowName);
#endif

    if (!strcmp(className, "SysListView32"))
    {
        printf("it's main windows, double click the room\n");
        LeftDoubleClick((int)roomItemPos.x, (int)roomItemPos.y);
    }
    else
    {
        //tempHwnd = GetForegroundWindow();
        //if (tempHwnd == NULL)
        //{
         //   return;
        //}

        GetClassName(fgHwnd, className, sizeof(className));
#ifndef F_NO_DEBUG
        GetWindowText(fgHwnd, windowName, sizeof(windowName));
        printf("2:fgHwnd = %08x, className = %s, windowName = %s\n", tempHwnd, className, windowName);
#endif
        /*
        if (strstr(className, "32770") == NULL)
        {
            printf("not the msgwindow\n");
            return;
        }
        */

        HWND buttonHwnd = FindWindowEx(fgHwnd, NULL, "Button", NULL);
        // 正在进入房间,请稍候...窗口不包含Edit
        HWND editHwnd = FindWindowEx(fgHwnd, NULL, "Edit", NULL);

        printf("buttonHwnd = %x, editHwnd = %x\n", buttonHwnd, editHwnd);
        if ((buttonHwnd)&&(editHwnd))
        {
#ifndef F_NO_DEBUG
        //GetWindowRect(fgHwnd, &rc);
        //printf("2: left = %d, right = %d, top = %d, bottom = %d\n", rc.left, rc.right, rc.top, rc.bottom);

            printf("find window, click it\n");
#endif

            x = rc.right - (rc.right-rc.left)*(765-695)/(765-260);
            y = rc.bottom - (rc.bottom-rc.top)*(569-541)/(569-155);
            printf("x = %d, y = %d\n", x, y);

            //695 545
            LeftClick(x, y);
        }

    }
}

VOID CALLBACK AutoClickThreadProc_QQ(HWND hwnd, UINT uMsg, UINT_PTR idEvent,DWORD dwTime)
{
    POINT pos;
    RECT rc;
    HWND hwndMain, tempHwnd;
    char className[32], windowName[128];
    int x, y;

    if ((int)roomItemPos.x == 0)
    {
        GetCursorPos(&roomItemPos);
    }

    GetCursorPos(&pos);

#ifndef F_NO_DEBUG
    printf("HF: x = %d, y = %d, xx = %d, yy = %d\n", pos.x, pos.y, roomItemPos.x, roomItemPos.y);
#endif

    //tempHwnd = WindowFromPoint(roomItemPos);
    tempHwnd = GetForegroundWindow();
    if (tempHwnd == NULL)
    {
        return;
    }
    GetWindowRect(tempHwnd, &rc);
    printf("left = %d, right = %d, top = %d, bottom = %d\n", rc.left, rc.right, rc.top, rc.bottom);

    pos.x = (rc.left+rc.right)/3;
    pos.y = (rc.top+rc.bottom)/3;
    hwndMain = WindowFromPoint(pos);

    GetClassName(hwndMain, className, sizeof(className));
#ifndef F_NO_DEBUG
    GetWindowText(hwndMain, windowName, sizeof(windowName));
    printf("0:hwnd = %08x, className = %s, windowName = <%s>\n", hwndMain, className, windowName);
#endif
    if (!strncmp(className, explorerClass, sizeof(explorerClass)))
    {
#ifndef F_NO_DEBUG
        printf("!!!!success, stop the timer\n");
#endif
        KillTimer(hwnd, idEvent);
        g_timerId = 0;
        return;
    }

    GetClassName(tempHwnd, className, sizeof(className));
#ifndef F_NO_DEBUG
    GetWindowText(tempHwnd, windowName, sizeof(windowName));
    printf("1:hwnd = %08x, className = %s, windowName = <%s>\n", tempHwnd, className, windowName);
#endif

    if (!strcmp(className, tercentMainClass) && strcmp(windowName, ""))
    {
        printf("Main window detected\n");
        LeftDoubleClick((int)roomItemPos.x, (int)roomItemPos.y);
    }
    else
    {
        if (strcmp(className, tercentMainClass))
        {
#ifndef F_NO_DEBUG
            printf("not the msgwindow\n");
#endif
            return;
        }

#ifndef F_NO_DEBUG
        GetWindowRect(tempHwnd, &rc);
        printf("2: left = %d, right = %d, top = %d, bottom = %d\n", rc.left, rc.right, rc.top, rc.bottom);

        printf("find window, click it\n");
#endif

        x = rc.right - (rc.right-rc.left)*(703-649)/(703-316);
        y = rc.bottom - (rc.bottom-rc.top)*(458-432)/(458-256);
        printf("x = %d, y = %d\n", x, y);
        LeftClick(x, y);
        //LeftClick(642, 435);

    }
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
                printf("calling timer\n");

                if (g_timerId == 0)
                {
                    roomItemPos.x = 0;
                    roomItemPos.y = 0;

                    if (g_zoneCurrent == ZONE_HAOFANG)
                        g_timerId = SetTimer(NULL, 1, 3000, AutoClickThreadProc_HF);
                    else
                        g_timerId = SetTimer(NULL, 1, 500, AutoClickThreadProc_QQ);
                }

            }
            else if ((GetKeyState(VK_CONTROL) < 0) && (p->vkCode == (int)('D')))
            {
                KillTimer(NULL, g_timerId);
                g_timerId = 0;
            }
        }
    }

    return CallNextHookEx(myhook, nCode, wParam, lParam);
}

