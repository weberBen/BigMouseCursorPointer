
#include <Windows.h>
#include <windows.h>
#include <windowsx.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <objidl.h>
#include <gdiplus.h>
#include <atlimage.h>
#include <stdio.h>
#include <tchar.h>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <utility>
#include <atlstr.h>

#include "resource.h"

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"User32.Lib")
#pragma comment (lib,"Gdi32.Lib")
#pragma comment (lib,"Msimg32.lib")


// Common controls manifest entry is required for using custom draw.
// Remove this pragma if you have already included this in the manifest of your project.
/*#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
*/

#define CONF_FILE_FILENAME "configuration.conf"


/*
std::stringstream ss;
ss << "to debug console in VS"
OutputDebugStringA(ss.str().c_str());
*/


HWND Cursor_hwnd = NULL;
std::map<std::string, std::string> App_parms = {};
std::string SelfDir;
std::string PathConfFile;

std::string GetSelfDir()
{
    char selfdir[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, selfdir, MAX_PATH);
    PathRemoveFileSpecA(selfdir);


    std::stringstream ss;
    ss << selfdir;

    return ss.str();
}

std::string AppendPath(std::string base_path, std::string name)
{
    std::stringstream ss;
    ss << base_path << "\\" << name;

    return ss.str();
}

std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
        return "Error(errornum=0, msg=<<No error>>)"; //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    std::stringstream ss;
    ss << "Error(errornum=" << errorMessageID << " , msg=<<" << message << ">>)";

    return ss.str();
}

std::wstring GetUtf16(const std::string& str, int codepage)
{
    if (str.empty()) return std::wstring();
    int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
    std::wstring res(sz, 0);
    MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
    return res;
}

void ShowCustomError(std::string msg)
{
    MessageBox(NULL, GetUtf16(msg, GetACP()).c_str(), L"Error", MB_OK | MB_ICONWARNING);
}

void ShowError(std::string prev_msg)
{
    std::string err_string = GetLastErrorAsString();
    std::stringstream ss;
    ss << prev_msg << std::endl << err_string;
    ShowCustomError(ss.str());
}



HBITMAP GdiplusImageToHBitmap(Gdiplus::Bitmap* bitmap)
{
    HBITMAP result = NULL;

    if (bitmap)
    {
        bitmap->GetHBITMAP(NULL, &result);
    }

    return result;
}


Gdiplus::Bitmap* RescaleBitmap(Gdiplus::Bitmap* originalBitmap, float horizontalScalingFactor, float verticalScalingFactor)
{
    float newWidth = horizontalScalingFactor * (float)originalBitmap->GetWidth();
    float newHeight = verticalScalingFactor * (float)originalBitmap->GetHeight();

    Gdiplus::Bitmap* img = new Bitmap((int)newWidth, (int)newHeight);

    Graphics g(img);
    g.ScaleTransform(horizontalScalingFactor, verticalScalingFactor);
    g.DrawImage(originalBitmap, 0, 0);

    return img;
}

Gdiplus::Bitmap* RescaleBitmap(Gdiplus::Bitmap* originalBitmap, float scale_factor)
{
    return RescaleBitmap(originalBitmap, scale_factor, scale_factor);
}

boolean UpdateParams(std::string path_file)
{
    std::ofstream stream(path_file);

    for (auto& kv : App_parms)
    {
        stream << kv.first << "=" << kv.second << "\n";
    }
    stream.close();

    return true;
}

boolean LoadParams(std::string path_file)
{
    //read configuration file
    std::ifstream infile(path_file);
    if (!infile)
    {
        std::stringstream ss;
        ss << "Cannot load configuration file at : <<" << path_file << ">>";
        ShowError(ss.str());

        return false;
    }

    std::map<std::string, std::string> dic;

    std::string line;
    while (std::getline(infile, line))
    {
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '='))
        {
            std::string value;
            if (std::getline(is_line, value))
                App_parms.insert({ key, value });
        }
    }

    return true;
}


boolean SetSplashImage(HWND hwndSplash, HBITMAP hbmpSplash)
{
    boolean error = false;
    // get the size of the bitmap

    BITMAP bm;
    GetObject(hbmpSplash, sizeof(bm), &bm);
    SIZE sizeSplash = { bm.bmWidth, bm.bmHeight };

    // get the primary monitor's info

    POINT ptZero = { 0 };
    HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorinfo = { 0 };
    monitorinfo.cbSize = sizeof(monitorinfo);
    GetMonitorInfo(hmonPrimary, &monitorinfo);

    // center the splash screen in the middle of the primary work area

    const RECT& rcWork = monitorinfo.rcWork;
    POINT ptOrigin;
    ptOrigin.x = rcWork.left + (rcWork.right - rcWork.left - sizeSplash.cx) / 2;
    ptOrigin.y = rcWork.top + (rcWork.bottom - rcWork.top - sizeSplash.cy) / 2;

    // create a memory DC holding the splash bitmap

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmpSplash);

    // use the source image's alpha channel for blending

    BLENDFUNCTION blend = { 0 };
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    // paint the window (in the right location) with the alpha-blended bitmap

    int res = UpdateLayeredWindow(hwndSplash, hdcScreen, &ptOrigin, &sizeSplash,
        hdcMem, &ptZero, RGB(0, 0, 0), &blend, ULW_ALPHA);
    if (res == 0)
    {
        ShowError("Cannot update window layer");
        error = true;
    }

    // delete temporary objects

    SelectObject(hdcMem, hbmpOld);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return !error;
}

boolean UpdateWindow(HWND hwnd, std::string image_path, float image_scale_factor)
{
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(GetUtf16(image_path, GetACP()).c_str(), false);
    if (!bitmap)
    {
        ShowError("Cannot load cursor image");
        return false;
    }


    if (image_scale_factor <= 0)
    {
        std::stringstream ss;
        ss << image_scale_factor << " is not a correct scale factor";
        ShowCustomError(ss.str());

        image_scale_factor = 1;
    }


    if (image_scale_factor != 1)
    {
        Gdiplus::Bitmap* resclaed_bitmap = RescaleBitmap(bitmap, image_scale_factor);
        delete bitmap;
        bitmap = resclaed_bitmap;
    }

    HBITMAP img = GdiplusImageToHBitmap(bitmap);
    delete bitmap;

    return SetSplashImage(hwnd, img);
}

boolean UpdateWindowFromConfFile(HWND hwnd)
{
    //image path
    if (App_parms.find("image_filename") == App_parms.end())
    {
        ShowCustomError("Cannot get cursor image filename from configuration file");
        return false;
    }
    std::string image_path = AppendPath(SelfDir, App_parms.at("image_filename"));

    //image scale factor
    if (App_parms.find("scale_factor") == App_parms.end())
    {
        ShowCustomError("Cannot get cursor image scale factor from configuration file");
        return false;
    }
    std::string tmp = App_parms.at("scale_factor");
    float scale_factor = (float)::atof(tmp.c_str());

    return UpdateWindow(hwnd, image_path, scale_factor);
}



//--------------------------------------------------------------------------

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && wParam == WM_MOUSEMOVE)
    {
        MSLLHOOKSTRUCT* mh = (MSLLHOOKSTRUCT*)lParam;

        RECT rec;
        GetWindowRect(Cursor_hwnd, &rec);
        SetWindowPos(Cursor_hwnd, HWND_TOPMOST, mh->pt.x + 1, mh->pt.y + 1, 0, 0, SWP_NOSIZE);

    }


    return CallNextHookEx(NULL, nCode, wParam, lParam);
}



LRESULT CALLBACK WindowProcCursor(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_CREATE:
    {
        if (!UpdateWindowFromConfFile(hwnd))
        {
            PostQuitMessage(0);
            return 0;
        }
    }
    break;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    break;

    case WM_PAINT:
    {

    }
    break;

    case WM_ERASEBKGND:
    {

    }
    break;

    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
        return 0;
    }

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#define ID_BUTTON_CHANGE_PTR 0
#define ID_TEXT_BOX_SCALE 1
#define ID_BUTTON_VALIDATE_SCALE 2

HWND SCALE_TEXT_BOX;
WNDPROC oldEditProc;

boolean ReadNewScaleFactor(HWND textbox)
{
    boolean error = false;

    int len = GetWindowTextLengthW(textbox) + 1;
    wchar_t* text = new wchar_t[len];
    GetWindowTextW(textbox, text, len);

    float scale_factor = (float)::atof(CW2A(text));
    if (scale_factor <= 0)
    {
        ShowCustomError("The input scale factor is not a valide one");
        SetWindowTextW(textbox, GetUtf16(App_parms.at("scale_factor"), GetACP()).c_str());
        error = true;
    }
    else
    {
        App_parms["scale_factor"] = CW2A(text);
        UpdateParams(PathConfFile);
        if (!UpdateWindowFromConfFile(Cursor_hwnd))
        {
            error = true;
        }
    }

    delete text;

    return !error;
}

LRESULT CALLBACK subEditProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_RETURN:
        {
            ReadNewScaleFactor(SCALE_TEXT_BOX);

        }
        break;  //or return 0; if you don't want to pass it further to def proc
    //If not your key, skip to default:
        }
    default:
        return CallWindowProc(oldEditProc, wnd, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WindowProcMenu(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_CREATE:
    {
        int x, y, h, w;
        int x_tmp;
        const int x_space = 10;
        const int y_space = 10;

        w = 300;
        h = 50;
        x = 10;
        y = 10;

        HWND button_image = CreateWindow(L"BUTTON", L"Change cursor pointer", BS_TEXT | WS_CHILD | WS_VISIBLE,
            x, y, w, h, hwnd, (HMENU)ID_BUTTON_CHANGE_PTR, GetModuleHandle(NULL), NULL);

        y += h + y_space;
        h = 50;
        w = 105;
        HWND label_edit = CreateWindow(L"static", L"ST_U", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            x, y, w, h, hwnd, NULL, GetModuleHandle(NULL), NULL);
        SetWindowText(label_edit, L"Image scale : ");

        x_tmp = x;
        x += w + x_space;
        w = 50;
        h = 50;
        SCALE_TEXT_BOX = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", GetUtf16(App_parms.at("scale_factor"), GetACP()).c_str(),
            WS_CHILD | WS_VISIBLE, x, y, w, h, hwnd, (HMENU)ID_TEXT_BOX_SCALE, GetModuleHandle(NULL), NULL);

        oldEditProc = (WNDPROC)SetWindowLongPtr(SCALE_TEXT_BOX, GWLP_WNDPROC, (LONG_PTR)subEditProc);

        x += w + x_space;
        w = 125;
        h = 50;
        HWND button_validate_scale = CreateWindow(L"BUTTON", L"Validate", BS_TEXT | WS_CHILD | WS_VISIBLE,
            x, y, w, h, hwnd, (HMENU)ID_BUTTON_VALIDATE_SCALE, GetModuleHandle(NULL), NULL);
        x = x_tmp;
    }
    break;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case ID_BUTTON_CHANGE_PTR:
        {

            //make sure this is commented out in all code (usually stdafx.h)
            // #define WIN32_LEAN_AND_MEAN 

            OPENFILENAME ofn;       // common dialog box structure
            TCHAR szFile[MAX_PATH] = { 0 };       // if using TCHAR macros

            // Initialize OPENFILENAME
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = _T("Image\0*.PNG\0*.JPG\0*.BMP\0");
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn) == TRUE)
            {
                // use ofn.lpstrFile
                std::string img_path = AppendPath(SelfDir, App_parms.at("image_filename"));
                if (!CopyFile(ofn.lpstrFile, GetUtf16(img_path, GetACP()).c_str(), FALSE))
                {
                    ShowError("Cannot copy the file");
                    break;
                }

                if (!UpdateWindowFromConfFile(Cursor_hwnd))
                {
                    PostQuitMessage(0);
                    return 0;
                }
            }
        }
        break;

        case ID_BUTTON_VALIDATE_SCALE:
        {
            ReadNewScaleFactor(SCALE_TEXT_BOX);
        }
        break;
        }
    }
    break;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    break;

    case WM_PAINT:
    {

    }
    break;

    case WM_ERASEBKGND:
    {

    }
    break;

    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
        return 0;
    }

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

boolean createCursorWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    //create window
    const wchar_t CLASS_NAME[] = L"CursorWindowClass";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProcCursor;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);


    Cursor_hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,   // Optional window styles.
        CLASS_NAME,                     // Window class
        L"cursor windows",    // Window text
        WS_POPUP,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT,
        20, 20,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );
    //WS_OVERLAPPEDWINDOW
    if (Cursor_hwnd == NULL)
    {
        ShowError("Cannot create window");

        return false;
    }

    ShowWindow(Cursor_hwnd, nCmdShow);

    return true;
}



boolean createMenuWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    //create window
    const wchar_t CLASS_NAME[] = L"MenuWindowClass";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProcMenu;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

    RegisterClass(&wc);


    HWND hwnd = CreateWindowEx(
        0,   // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Menu custom cursor pointer",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT,
        350, 200,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );
    //WS_OVERLAPPEDWINDOW
    if (hwnd == NULL)
    {
        ShowError("Cannot create window");

        return false;
    }


    ShowWindow(hwnd, nCmdShow);

    return true;
}


bool CheckOneInstance()
{

    HANDLE  m_hStartEvent = CreateEventW(NULL, FALSE, FALSE, L"Global\\CSAPP");

    if (m_hStartEvent == NULL)
    {
        CloseHandle(m_hStartEvent);
        return false;
    }


    if (GetLastError() == ERROR_ALREADY_EXISTS) {

        CloseHandle(m_hStartEvent);
        m_hStartEvent = NULL;
        // already exist
        // send message from here to existing copy of the application
        return false;
    }
    // the only instance, start in a usual way
    return true;
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    if (!CheckOneInstance())//app is already running
    {
        return 0;
    }

    //load parameters
    SelfDir = GetSelfDir();
    PathConfFile = AppendPath(SelfDir, CONF_FILE_FILENAME);

    if (!LoadParams(PathConfFile))//error
    {
        return 0;
    }


    //start process

    if (SetProcessDPIAware() == 0)
    {
        ShowError("Cannot set DPI aware app");
        return 0;
    }

    //start gdiplus
    GdiplusStartupInput gpStartupInput;
    ULONG_PTR gpToken;
    Status status = GdiplusStartup(&gpToken, &gpStartupInput, NULL);
    if (status != Ok)
    {
        std::stringstream ss;
        ss << "Cannot start Gdiplus : " << status;

        ShowCustomError(ss.str());

        return 0;
    }

    if (!createCursorWindow(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
    {
        GdiplusShutdown(gpToken);

        return 0;
    }

    if (!createMenuWindow(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
    {
        GdiplusShutdown(gpToken);

        return 0;
    }



    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, hInstance, NULL);
    if (mouseHook == NULL)
    {
        ShowError("Cannot set mouse move hook");

        return 0;
    }


    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }



    if (UnhookWindowsHookEx(mouseHook) == 0)
    {
        ShowError("Cannot unset mouse move hook");
    }
    GdiplusShutdown(gpToken);

    return 0;
}
