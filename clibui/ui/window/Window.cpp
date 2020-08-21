#include "stdafx.h"
#include "Window.h"
#include "WindowMsgLoop.h"
#include <cassert>
#include <render\Direct2D.h>
#include <clibjs\cjsgui.h>

#define REPORT_ERROR 1
#define REPORT_ERROR_FILE "error.log"

static bool IsKeyPressing(int code)
{
    return (GetKeyState((int)code) & 0xF0) != 0;
}

static bool IsKeyToggled(int code)
{
    return (GetKeyState((int)code) & 0x0F) != 0;
}

static CString GetLastErrorStr() // Error Notification
{
    LPSTR format_string = NULL; // format string

    DWORD dwRet = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED),
        (LPSTR)&format_string,
        0,
        NULL);

    CString buf(format_string);

    if (dwRet)
    {
        LocalFree(format_string);
    }

    return buf;
}

Window* window;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    if (window->HandleMessage(hwnd, uMsg, wParam, lParam, result))
    {
        return result;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

Window::Window(HWND parent, CString className, CString windowTitle, HINSTANCE hInstance)
    : wndClass(className, false, false, WndProc, hInstance)
{
    window = this;
    DWORD exStyle = WS_EX_APPWINDOW | WS_EX_CONTROLPARENT;
    DWORD style = WS_VISIBLE | WS_BORDER | WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    CreateWindowEx(exStyle, className, windowTitle, style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        parent, NULL, hInstance, NULL);
}


Window::~Window()
{
    Destroyed();
    DestroyWindow(handle);
    RestClient::disable();
}

void Window::Init()
{
    d2dRenderTarget = std::make_shared<Direct2DRenderTarget>(shared_from_this());
    d2dRenderTarget->Init();
    engine.Initialize(d2dRenderTarget);
}

void Window::Run()
{
    Init();
    Center();
    Show();
    winMsgLoop.Run();
}

void Window::Center()
{
    CSize screenSize(GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN));
    CRect windowBounds = GetBounds();
    windowBounds = CRect(windowBounds.TopLeft(), windowBounds.Size().Max(minSize + GetNonClientSize()));
    CSize size = windowBounds.Size();
    CPoint topLeft = (screenSize - size) / 2;
    SetWindowPos(handle, HWND_TOPMOST, topLeft.x, topLeft.y, size.cx, size.cy, SWP_NOZORDER);
    SetWindowPos(handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

bool Window::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
#ifdef _DEBUG
    if (uMsg != WM_PAINT &&
        uMsg != WM_SETCURSOR &&
        uMsg != WM_NCHITTEST &&
        uMsg != WM_MOUSEMOVE &&
        uMsg != WM_MOUSEHOVER &&
        uMsg != WM_NCMOUSELEAVE &&
        uMsg != WM_SYSTIMER &&
        uMsg != WM_TIMER &&
        uMsg != WM_NCMOUSEMOVE)
    {
        ATLTRACE(atlTraceHosting, 0, "hwnd: 0x%08x message: %-30S[0x%04x] {W:0x%08X,L:0x%08X}\n", hwnd,
            winMsgLoop.DebugGetMessageName(uMsg), uMsg, wParam, lParam);
    }
#endif // _DEBUG
    switch (uMsg)
    {
    case WM_DESTROY:
        Destroying();
        PostQuitMessage(0);
        break;
    }
    return HandleMessageInternal(hwnd, uMsg, wParam, lParam, result);
}

HWND Window::GetWindowHandle() const
{
    return handle;
}

CSize Window::GetNonClientSize()
{
    CRect rtWindow, rtClient;
    GetWindowRect(handle, &rtWindow);
    GetClientRect(handle, &rtClient);
    return rtWindow.Size() - rtClient.Size();
}

CRect Window::GetBounds()
{
    CRect rect;
    GetWindowRect(handle, &rect);
    return rect;
}

void Window::SetBounds(const CRect& bounds)
{
    CRect newBounds = bounds;
    MoveWindow(handle, (int)newBounds.left, (int)newBounds.top, (int)newBounds.Width(), (int)newBounds.Height(), FALSE);
}

CSize Window::GetClientSize()
{
    return GetClientBoundsInScreen().Size();
}

void Window::SetClientSize(CSize size)
{
    CRect required(CPoint(), size);
    CRect bounds;
    GetWindowRect(handle, &bounds);
    AdjustWindowRect(&required, (DWORD)GetWindowLongPtr(handle, GWL_STYLE), FALSE);
    SetBounds(CRect(bounds.TopLeft(), required.Size()));
}

CRect Window::GetClientBoundsInScreen()
{
    CRect required;
    CRect bounds;
    GetWindowRect(handle, &bounds);
    AdjustWindowRect(&required, (DWORD)GetWindowLongPtr(handle, GWL_STYLE), FALSE);
    return CRect(bounds.TopLeft() + (-required.TopLeft()),
        bounds.Size() - required.Size());
}

CSize Window::GetClientWindowSize()
{
    CRect rect;
    GetClientRect(handle, &rect);
    return minSize.Max(rect.Size());
}

CSize Window::GetNonClientWindowSize()
{
    CRect rect;
    GetClientRect(handle, &rect);
    return (minSize + GetNonClientSize()).Max(rect.Size());
}

CString Window::GetTitle()
{
    return title;
}

void Window::SetTitle(CString _title)
{
    title = _title;
    SetWindowText(handle, title);
}

Window::WindowSizeState Window::GetSizeState()
{
    return IsIconic(handle) ? Minimized :
        IsZoomed(handle) ? Maximized : Restored;
}

void Window::SetMinSize(CSize size)
{
    minSize = size;
}

void Window::Show()
{
    ShowWindow(handle, SW_SHOWNORMAL);
}

void Window::Show(int nCmdShow)
{
    ShowWindow(handle, nCmdShow);
}

void Window::ShowDeactivated()
{
    ShowWindow(handle, SW_SHOWNA);
}

void Window::ShowRestored()
{
    ShowWindow(handle, SW_RESTORE);
}

void Window::ShowMaximized()
{
    ShowWindow(handle, SW_SHOWMAXIMIZED);
}

void Window::ShowMinimized()
{
    ShowWindow(handle, SW_SHOWMINIMIZED);
}

void Window::Hide()
{
    PostMessage(handle, WM_CLOSE, NULL, NULL);
}

bool Window::IsVisible()
{
    return IsWindowVisible(handle) != 0;
}

void Window::Enable()
{
    EnableWindow(handle, TRUE);
}

void Window::Disable()
{
    EnableWindow(handle, FALSE);
}

bool Window::IsEnabled()
{
    return IsWindowEnabled(handle) != 0;
}

void Window::SetFocus()
{
    ::SetFocus(handle);
}

bool Window::IsFocused()
{
    return GetFocus() == handle;
}

void Window::SetActivate()
{
    SetActiveWindow(handle);
}

bool Window::IsActivated()
{
    return GetActiveWindow() == handle;
}

void Window::ShowInTaskBar()
{
    SetExStyle(WS_EX_APPWINDOW, true);
}

void Window::HideInTaskBar()
{
    SetExStyle(WS_EX_APPWINDOW, false);
}

bool Window::IsAppearedInTaskBar()
{
    return GetExStyle(WS_EX_APPWINDOW);
}

void Window::EnableActivate()
{
    SetExStyle(WS_EX_NOACTIVATE, false);
}

void Window::DisableActivate()
{
    SetExStyle(WS_EX_NOACTIVATE, true);
}

bool Window::IsEnabledActivate()
{
    return !GetExStyle(WS_EX_NOACTIVATE);
}

bool Window::RequireCapture()
{
    SetCapture(handle);
    return true;
}

bool Window::ReleaseCapture()
{
    ::ReleaseCapture();
    return true;
}

bool Window::IsCapturing()
{
    return GetCapture() == handle;
}

bool Window::GetMaximizedBox()
{
    return GetStyle(WS_MAXIMIZEBOX);
}

void Window::SetMaximizedBox(bool visible)
{
    SetStyle(WS_MAXIMIZEBOX, visible);
}

bool Window::GetMinimizedBox()
{
    return GetStyle(WS_MINIMIZEBOX);
}

void Window::SetMinimizedBox(bool visible)
{
    SetStyle(WS_MINIMIZEBOX, visible);
}

bool Window::GetBorder()
{
    return GetStyle(WS_BORDER);
}

void Window::SetBorder(bool visible)
{
    SetStyle(WS_BORDER, visible);
}

bool Window::GetSizeBox()
{
    return GetStyle(WS_SIZEBOX);
}

void Window::SetSizeBox(bool visible)
{
    SetStyle(WS_SIZEBOX, visible);
}

bool Window::GetIconVisible()
{
    return GetStyle(WS_SYSMENU);
}

void Window::SetIconVisible(bool visible)
{
    SetStyle(WS_SYSMENU, visible);
}

bool Window::GetTitleBar()
{
    return GetStyle(WS_CAPTION);
}

void Window::SetTitleBar(bool visible)
{
    SetStyle(WS_CAPTION, visible);
}

bool Window::GetTopMost()
{
    return GetExStyle(WS_EX_TOPMOST);
}

void Window::SetTopMost(bool topmost)
{
    SetWindowPos(handle, (topmost ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
}

void Window::RedrawContent()
{
    PostMessage(handle, WM_PAINT, NULL, NULL);
}

DWORD Window::InternalGetExStyle()
{
    return (DWORD)GetWindowLongPtr(handle, GWL_EXSTYLE);
}

void Window::InternalSetExStyle(DWORD exStyle)
{
    LONG_PTR result = SetWindowLongPtr(handle, GWL_EXSTYLE, exStyle);
}

bool Window::GetExStyle(DWORD exStyle)
{
    LONG_PTR Long = InternalGetExStyle();
    return (Long & exStyle) != 0;
}

void Window::SetExStyle(DWORD exStyle, bool available)
{
    DWORD Long = InternalGetExStyle();
    if (available)
    {
        Long |= exStyle;
    }
    else
    {
        Long &= ~exStyle;
    }
    InternalSetExStyle((DWORD)Long);
}

bool Window::GetStyle(DWORD style)
{
    LONG_PTR Long = GetWindowLongPtr(handle, GWL_STYLE);
    return (Long & style) != 0;
}

void Window::SetStyle(DWORD style, bool available)
{
    LONG_PTR Long = GetWindowLongPtr(handle, GWL_STYLE);
    if (available)
    {
        Long |= style;
    }
    else
    {
        Long &= ~style;
    }
    SetWindowLongPtr(handle, GWL_STYLE, Long);
}

bool Window::GetClassStyle(DWORD style)
{
    DWORD Long = GetClassLong(handle, GCL_STYLE);
    return (Long & style) != 0;
}

void Window::SetClassStyle(DWORD style, bool available)
{
    DWORD Long = GetClassLong(handle, GCL_STYLE);
    if (available)
    {
        Long |= style;
    }
    else
    {
        Long &= ~style;
    }
    SetClassLong(handle, GCL_STYLE, Long);
}

void Window::SetTimer(int id, int elapse)
{
    if (setTimer.find(id) != setTimer.end())
        return;
    setTimer.insert(id);
    ::SetTimer(handle, id, elapse, NULL);
}

void Window::KillTimer(int id)
{
    if (setTimer.find(id) == setTimer.end())
        return;
    setTimer.erase(id);
    ::KillTimer(handle, id);
}

void Window::Redraw()
{
    Render();
}

MouseInfo Window::ConvertMouse(WPARAM wParam, LPARAM lParam, bool wheelMessage, bool nonClient)
{
    auto& info = GLOBAL_STATE.mouseInfo;

    info.nonClient = false;
    if (nonClient)
    {
        switch (wParam)
        {
        case HTMINBUTTON:
        case HTMAXBUTTON:
        case HTCLOSE:
            break;
        default:
            info.nonClient = true;
            break;
        }
    }

    if (wheelMessage)
    {
        info.wheel = GET_WHEEL_DELTA_WPARAM(wParam);
        wParam = GET_KEYSTATE_WPARAM(wParam);
    }
    else
    {
        info.wheel = 0;
    }

    if (nonClient)
    {
        info.ctrl = IsKeyPressing(VK_CONTROL);
        info.shift = IsKeyPressing(VK_SHIFT);
        info.left = IsKeyPressing(MK_LBUTTON);
        info.middle = IsKeyPressing(MK_MBUTTON);
        info.right = IsKeyPressing(MK_RBUTTON);

        POINTS point = MAKEPOINTS(lParam);
        CPoint offset = GetClientBoundsInScreen().TopLeft();
        info.pt.x = point.x - offset.x;
        info.pt.y = point.y - offset.y;
    }
    else
    {
        info.ctrl = (wParam & MK_CONTROL) != 0;
        info.shift = (wParam & MK_SHIFT) != 0;
        info.left = (wParam & MK_LBUTTON) != 0;
        info.middle = (wParam & MK_MBUTTON) != 0;
        info.right = (wParam & MK_RBUTTON) != 0;

        POINTS point = MAKEPOINTS(lParam);

        if (wheelMessage)
        {
            CPoint offset = GetClientBoundsInScreen().TopLeft();
            info.pt.x = point.x - offset.x;
            info.pt.y = point.y - offset.y;
        }
        else
        {
            info.pt.x = point.x;
            info.pt.y = point.y;
        }
    }
    return info;
}

KeyInfo Window::ConvertKey(WPARAM wParam, LPARAM lParam)
{
    auto& info = GLOBAL_STATE.keyInfo;
    info.code = wParam;
    info.scan = HIWORD(lParam) & 0x01FF;
    info.ctrl = IsKeyPressing(VK_CONTROL);
    info.shift = IsKeyPressing(VK_SHIFT);
    info.alt = IsKeyPressing(VK_MENU);
    info.capslock = IsKeyToggled(VK_CAPITAL);
    return info;
}

void Window::TrackMouse(bool enable)
{
    TRACKMOUSEEVENT trackMouseEvent;
    trackMouseEvent.cbSize = sizeof(trackMouseEvent);
    trackMouseEvent.hwndTrack = handle;
    trackMouseEvent.dwFlags = (enable ? 0 : TME_CANCEL) | TME_HOVER | TME_LEAVE;
    trackMouseEvent.dwHoverTime = HOVER_DEFAULT;
    TrackMouseEvent(&trackMouseEvent);
}

bool Window::HandleMessageInternal(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
    bool transferFocusEvent = false;
    bool nonClient = false;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
        transferFocusEvent = true;
    }
    switch (uMsg)
    {
        // ************************************** timer
    case WM_TIMER:
        Timer(wParam);
        break;
        // ************************************** moving and sizing
    case WM_MOVING:
    case WM_SIZING:
    {
        LPRECT rawBounds = (LPRECT)lParam;
        CRect bounds(rawBounds);
        Moving(bounds);
        if (!bounds.EqualRect(rawBounds))
        {
            *rawBounds = bounds;
            result = TRUE;
        }
    }
    break;
    case WM_NCCREATE:
        handle = hwnd;
        break;
    case WM_CREATE:
        Created();
        break;
    case WM_MOVE:
    case WM_SIZE:
        Moved();
        break;
        // ************************************** state
    case WM_ENABLE:
        wParam == TRUE ? Enabled() : Disabled();
        break;
    case WM_SETFOCUS:
        GotFocus();
        break;
    case WM_KILLFOCUS:
        LostFocus();
        break;
    case WM_ACTIVATE:
        wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE ? Activated() : Deactivated();
        break;
    case WM_SHOWWINDOW:
        wParam == TRUE ? Opened() : Closed();
        break;
    case WM_CLOSE:
    {
        bool cancel = false;
        Closing(cancel);
        return cancel;
    }
    break;
    // ************************************** mouse
    case WM_NCLBUTTONDOWN:
        nonClient = true;
    case WM_LBUTTONDOWN:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, false, nonClient);
        LeftButtonDown();
    }
    break;
    case WM_NCLBUTTONUP:
        nonClient = true;
    case WM_LBUTTONUP:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, false, nonClient);
        LeftButtonUp();
    }
    break;
    case WM_NCLBUTTONDBLCLK:
        nonClient = true;
    case WM_LBUTTONDBLCLK:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, false, nonClient);
        LeftButtonDoubleClick();
    }
    break;
    case WM_NCRBUTTONDOWN:
        nonClient = true;
    case WM_RBUTTONDOWN:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, false, nonClient);
        RightButtonDown();
    }
    break;
    case WM_NCRBUTTONUP:
        nonClient = true;
    case WM_RBUTTONUP:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, false, nonClient);
        RightButtonUp();
    }
    break;
    case WM_NCRBUTTONDBLCLK:
        nonClient = true;
    case WM_RBUTTONDBLCLK:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, false, nonClient);
        RightButtonDoubleClick();
    }
    break;
    case WM_NCMBUTTONDOWN:
        nonClient = true;
    case WM_MBUTTONDOWN:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, false, nonClient);
        MiddleButtonDown();
    }
    break;
    case WM_NCMBUTTONUP:
        nonClient = true;
    case WM_MBUTTONUP:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, false, nonClient);
        MiddleButtonUp();
    }
    break;
    case WM_NCMBUTTONDBLCLK:
        nonClient = true;
    case WM_MBUTTONDBLCLK:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, false, nonClient);
        MiddleButtonDoubleClick();
    }
    break;
    case WM_NCMOUSEMOVE:
        nonClient = true;
    case WM_MOUSEMOVE:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, false, nonClient);
        if (GLOBAL_STATE.mouseInfo.pt != mouseLast)
        {
            MouseMoving();
            if (!mouseHoving)
            {
                mouseHoving = true;
                MouseEntered();
                TrackMouse(true);
            }
        }
    }
    break;
    // ************************************** wheel
    case WM_MOUSEHWHEEL:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, true, false);
        HorizontalWheel();
    }
    break;
    case WM_MOUSEWHEEL:
    {
        GLOBAL_STATE.mouseInfo = ConvertMouse(wParam, lParam, true, false);
        VerticalWheel();
    }
    break;
    // ************************************** mouse state
    case WM_NCMOUSELEAVE:
        nonClient = true;
    case WM_MOUSELEAVE:
        if (!nonClient)
        {
            mouseLast.x = -1;
            mouseLast.y = -1;
            mouseHoving = false;
            MouseLeaved();
        }
        break;
    case WM_NCMOUSEHOVER:
    case WM_MOUSEHOVER:
        MouseHover();
        TrackMouse(true);
        break;
        // ************************************** key
    case WM_KEYUP:
    {
        GLOBAL_STATE.keyInfo = ConvertKey(wParam, lParam);
        KeyUp();
    }
    break;
    case WM_KEYDOWN:
    {
        GLOBAL_STATE.keyInfo = ConvertKey(wParam, lParam);
        KeyDown();
    }
    break;
    case WM_SYSKEYUP:
    {
        GLOBAL_STATE.keyInfo = ConvertKey(wParam, lParam);
        SysKeyUp();
    }
    break;
    case WM_SYSKEYDOWN:
    {
        GLOBAL_STATE.keyInfo = ConvertKey(wParam, lParam);
        SysKeyDown();
    }
    break;
    case WM_CHAR:
    {
        GLOBAL_STATE.keyInfo = ConvertKey(wParam, lParam);
        Char();
    }
    break;
    // ************************************** painting
    case WM_PAINT:
        Paint();
        break;
    case WM_ERASEBKGND:
        result = 0;
        return true;
    case WM_NCPAINT:
    case WM_SYNCPAINT:
        break;
        // ************************************** IME
    case WM_IME_SETCONTEXT:
        if (wParam == TRUE)
        {
            HIMC imc = ImmGetContext(handle);
            ImmAssociateContext(hwnd, imc);
            ImmReleaseContext(handle, imc);
        }
        break;
        // ************************************** hit test
    case WM_NCHITTEST:
    {
        POINTS location = MAKEPOINTS(lParam);
        CPoint windowLocation = GetBounds().TopLeft();
        location.x -= (SHORT)windowLocation.x;
        location.y -= (SHORT)windowLocation.y;
        switch (HitTest(CPoint(location.x, location.y)))
        {
        case BorderNoSizing:
            result = HTBORDER;
            return true;
        case BorderLeft:
            result = HTLEFT;
            return true;
        case BorderRight:
            result = HTRIGHT;
            return true;
        case BorderTop:
            result = HTTOP;
            return true;
        case BorderBottom:
            result = HTBOTTOM;
            return true;
        case BorderLeftTop:
            result = HTTOPLEFT;
            return true;
        case BorderRightTop:
            result = HTTOPRIGHT;
            return true;
        case BorderLeftBottom:
            result = HTBOTTOMLEFT;
            return true;
        case BorderRightBottom:
            result = HTBOTTOMRIGHT;
            return true;
        case Title:
            result = HTCAPTION;
            return true;
        case ButtonMinimum:
            result = HTMINBUTTON;
            return true;
        case ButtonMaximum:
            result = HTMAXBUTTON;
            return true;
        case ButtonClose:
            result = HTCLOSE;
            return true;
        case Client:
            result = HTCLIENT;
            return true;
        case Icon:
            result = HTSYSMENU;
            return true;
        }
    }
    break;
    // ************************************** MISC
    case WM_SETCURSOR:
    {
        DWORD hitTestResult = LOWORD(lParam);
        if (hitTestResult == HTCLIENT)
        {
            decltype(IDC_ARROW) idc = IDC_ARROW;
            HCURSOR cursorHandle = LoadCursor(NULL, idc);
            if (::GetCursor() != cursorHandle)
            {
                ::SetCursor(cursorHandle);
            }
            result = TRUE;
            return true;
        }
    }
    break;
    case WM_NCACTIVATE:
        break;
    case WM_MOUSEACTIVATE:
        break;
    }

    if (IsWindow(hwnd) != 0 && !GetClassStyle(CS_DROPSHADOW))
    {
        if (transferFocusEvent && IsFocused())
        {
            SetFocus();
        }
    }

    return false;
}

void Window::Render()
{
    if (window && IsVisible() && !rendering)
    {
        rendering = true;
        RenderInternal();
        rendering = false;
    }
}

void Window::RenderInternal()
{
    using namespace std::chrono_literals;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - timer) < 10ms) {
        return;
    }
    timer = std::chrono::system_clock::now();

    d2dRenderTarget->StartRendering();

    engine.Render(d2dRenderTarget->GetDirect2DRenderTarget(), CRect(CPoint(), GetClientWindowSize()));
    auto result = d2dRenderTarget->StopRendering();
    if (FAILED(result))
    {
        ATLVERIFY(!"D2D ERR: Render");
    }

    result = d2dRenderTarget->Present();
    if (FAILED(result)) {
        ATLVERIFY(!"D2D ERR: Present");
    }
}

Window::HitTestResult Window::HitTest(CPoint location)
{
    return NoDecision;
}

static void PostNoArgMsg(WindowEvent evt)
{
}

void Window::Created()
{
    PostNoArgMsg(WE_Created);
}

void Window::Moving(CRect& bounds)
{
    CRect oldBounds = GetBounds();
    CSize minWindowSize = GetNonClientWindowSize();
    if (bounds.Width() < minWindowSize.cx)
    {
        bounds.right = bounds.left + minWindowSize.cx;
    }
    if (bounds.Height() < minWindowSize.cy)
    {
        bounds.bottom = bounds.top + minWindowSize.cy;
    }
    PostNoArgMsg(WE_Moving);
}

void Window::Moved()
{
    static CSize size;
    if (d2dRenderTarget) {
        if (size != GetClientWindowSize()) {
            size = GetClientWindowSize();
            Direct2D::Singleton().Resize();
            Render();
        }
    }
    PostNoArgMsg(WE_Moved);
}

void Window::Enabled()
{
    PostNoArgMsg(WE_Enabled);
}

void Window::Disabled()
{
    PostNoArgMsg(WE_Disabled);
}

void Window::GotFocus()
{
    clib::cjsgui::singleton().hit(WE_GotFocus);
    PostNoArgMsg(WE_GotFocus);
}

void Window::LostFocus()
{
    clib::cjsgui::singleton().hit(WE_GotFocus);
    PostNoArgMsg(WE_LostFocus);
}

void Window::Activated()
{
    PostNoArgMsg(WE_Activated);
}

void Window::Deactivated()
{
    PostNoArgMsg(WE_Deactivated);
}

void Window::Opened()
{
    PostNoArgMsg(WE_Opened);
}

void Window::Closing(bool& cancel)
{
    PostNoArgMsg(WE_Closing);
}

void Window::Closed()
{
    PostNoArgMsg(WE_Closed);
}

void Window::Paint()
{
    Render();
    PostNoArgMsg(WE_Paint);
}

void Window::Destroying()
{
    PostNoArgMsg(WE_Destroying);
}

void Window::Destroyed()
{
    for (auto& timer : setTimer)
    {
        ::KillTimer(handle, timer);
    }
    setTimer.clear();
    engine.Finalize();
    
    clib::cjsgui::singleton().reset();
    PostNoArgMsg(WE_Destroyed);
}

static void PostMouseMsg(WindowEvent evt)
{
    clib::cjsgui::singleton().hit(evt);
}

void Window::LeftButtonDown()
{
    PostMouseMsg(WE_LeftButtonDown);
}

void Window::LeftButtonUp()
{
    PostMouseMsg(WE_LeftButtonUp);
}

void Window::LeftButtonDoubleClick()
{
    PostMouseMsg(WE_LeftButtonDoubleClick);
}

void Window::RightButtonDown()
{
    PostMouseMsg(WE_RightButtonDown);
}

void Window::RightButtonUp()
{
    PostMouseMsg(WE_RightButtonUp);
}

void Window::RightButtonDoubleClick()
{
    PostMouseMsg(WE_RightButtonDoubleClick);
}

void Window::MiddleButtonDown()
{
    PostMouseMsg(WE_MiddleButtonDown);
}

void Window::MiddleButtonUp()
{
    PostMouseMsg(WE_MiddleButtonUp);
}

void Window::MiddleButtonDoubleClick()
{
    PostMouseMsg(WE_MiddleButtonDoubleClick);
}

void Window::HorizontalWheel()
{
    PostMouseMsg(WE_HorizontalWheel);
}

void Window::VerticalWheel()
{
    PostMouseMsg(WE_VerticalWheel);
}

void Window::MouseMoving()
{
    PostMouseMsg(WE_MouseMoving);
}

void Window::MouseEntered()
{
    clib::cjsgui::singleton().hit(WE_MouseMoving);
    PostNoArgMsg(WE_MouseEntered);
}

void Window::MouseLeaved()
{
    clib::cjsgui::singleton().hit(WE_MouseMoving);
    PostNoArgMsg(WE_MouseLeaved);
}

void Window::MouseHover()
{
    clib::cjsgui::singleton().hit(WE_MouseHover);
    PostNoArgMsg(WE_MouseHover);
}

static void PostKeyMsg(WindowEvent evt)
{
}

void Window::KeyDown()
{
    if (GLOBAL_STATE.keyInfo.code == VK_F11) {
        ToggleFullscreen();
    }
    clib::cjsgui::singleton().input(GLOBAL_STATE.keyInfo.code | GUI_SPECIAL_MASK);
    PostKeyMsg(WE_KeyDown);
}

void Window::KeyUp()
{
    PostKeyMsg(WE_KeyUp);
}

void Window::SysKeyDown()
{
    clib::cjsgui::singleton().input(GLOBAL_STATE.keyInfo.code | GUI_SPECIAL_MASK);
    PostKeyMsg(WE_SysKeyDown);
}

void Window::SysKeyUp()
{
    PostKeyMsg(WE_SysKeyUp);
}

void Window::Char()
{
    clib::cjsgui::singleton().input(GLOBAL_STATE.keyInfo.code);
    PostKeyMsg(WE_Char);
}

void Window::Timer(int id)
{
}

void Window::ToggleFullscreen()
{
    fullScreen = !fullScreen;
    if (fullScreen) {
        fullScreenSize = GetNonClientWindowSize();
        SetStyle(WS_OVERLAPPEDWINDOW, false);
        SetWindowPos(handle, NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL);
    }
    else {
        SetStyle(WS_OVERLAPPEDWINDOW, true);
        SetWindowPos(handle, NULL, 0, 0, fullScreenSize.cx, fullScreenSize.cy, SWP_SHOWWINDOW);
        Center();
    }
}
