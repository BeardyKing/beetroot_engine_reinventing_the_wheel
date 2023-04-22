#include <beet/window.h>
#include <beet/assert.h>
#include <beet/defines.h>
#include <beet/types.h>

#include <beet/input.h>

#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>

//===internal structs========
struct WindowInfo {
    WNDCLASS windowClass;
    HWND handle;
    const wchar_t *applicationName;
    const wchar_t *titleName;
    int32_t width;
    int32_t height;
    int32_t x;
    int32_t y;
    bool shouldWindowClose;
};

WindowInfo *g_windowInfo;

//===internal functions======
LRESULT CALLBACK window_procedure_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            g_windowInfo->shouldWindowClose = true;
            break;
        };
        case WM_KEYDOWN: {
            input_key_down_callback((char) wParam);
            break;
        };
        case WM_KEYUP: {
            input_key_up_callback((char) wParam);
            break;
        };
        case WM_PAINT: {
            ValidateRect(hwnd, nullptr);
            break;
        };
        default:
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//===api=====================
bool window_is_open() {
    return !g_windowInfo->shouldWindowClose;
}

void window_poll() {
    MSG msg = {};
    if (PeekMessage(&msg, g_windowInfo->handle, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

//===init & shutdown=========
void window_create() {
    int32_t screenX = GetSystemMetrics(SM_CXSCREEN) / 2 - (BEET_WINDOW_SIZE_X / 2);
    int32_t screenY = GetSystemMetrics(SM_CYSCREEN) / 2 - (BEET_WINDOW_SIZE_Y / 2);

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    {
        g_windowInfo = new WindowInfo{};
        g_windowInfo->width = BEET_WINDOW_SIZE_X;
        g_windowInfo->height = BEET_WINDOW_SIZE_Y;
        g_windowInfo->x = screenX;
        g_windowInfo->y = screenY;
        g_windowInfo->applicationName = L"" BEET_WINDOW_APPLICATION_NAME;
        g_windowInfo->titleName = L"" BEET_WINDOW_TITLE;
        g_windowInfo->windowClass.lpfnWndProc = window_procedure_callback;
        g_windowInfo->windowClass.hInstance = hInstance;
        g_windowInfo->windowClass.lpszClassName = g_windowInfo->applicationName;
    }

    RegisterClass(&g_windowInfo->windowClass);

    g_windowInfo->handle = CreateWindowEx(
            0,
            g_windowInfo->applicationName,
            g_windowInfo->titleName,
            WS_OVERLAPPEDWINDOW,
            g_windowInfo->x,
            g_windowInfo->y,
            g_windowInfo->width,
            g_windowInfo->height,
            nullptr,
            nullptr,
            hInstance,
            nullptr
    );

    ASSERT_MSG(g_windowInfo->handle, "Err: Failed to create window.")
    ShowWindow(g_windowInfo->handle, SW_SHOWDEFAULT);
    g_windowInfo->shouldWindowClose = false;
}

void window_cleanup() {
    delete g_windowInfo;
    g_windowInfo = nullptr;
}


