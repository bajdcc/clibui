#include "stdafx.h"
#include "Direct2D.h"
#include "../ui/window/Window.h"
#ifdef _DEBUG
#include <DXGIDebug.h>
#endif
#include <imgui\imgui_impl_dx11.h>
#include <imgui\imgui_impl_win32.h>
#include <clibjs\cjsgui.h>

D2D1::ColorF GetD2DColor(CColor color)
{
    return D2D1::ColorF(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

Direct2D::Direct2D()
{
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    HRESULT hr = S_OK;

    IDWriteFactory* dwrite;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dwrite));
    if (SUCCEEDED(hr))
        DWriteFactory.Attach(dwrite);
    else
        ATLVERIFY(!"DWriteCreateFactory failed");
    hr = WICImagingFactory.CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
        ATLVERIFY(!"WICImagingFactory failed");

    Init();
}

void Direct2D::Init()
{
    if (!D2D1Factory.p) {
        ID2D1Factory1* d2d1;
        // create the Direct2D factory
        D2D1_FACTORY_OPTIONS options;
#ifdef DEBUG
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
        options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
        auto hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, (void**)&d2d1);
        if (SUCCEEDED(hr))
            D2D1Factory.Attach(d2d1);
        else
            ATLVERIFY(!"D2D1CreateFactory failed");
    }

    // REFER: https://docs.microsoft.com/zh-cn/windows/desktop/Direct2D/devices-and-device-contexts

    // This flag adds support for surfaces with a different color channel ordering than the API default.
    // You need it for compatibility with Direct2D.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    // This array defines the set of DirectX hardware feature levels this app  supports.
    // The ordering is important and you should  preserve it.
    // Don't forget to declare your app's minimum required feature level in its
    // description.  All apps are assumed to support 9.1 unless otherwise stated.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    // Allocate a descriptor.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
    swapChainDesc.BufferDesc.Width = 0;                           // use automatic sizing
    swapChainDesc.BufferDesc.Height = 0;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
    swapChainDesc.SampleDesc.Count = 1;                // don't use multi-sampling
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;                     // use double buffering to enable flip
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.OutputWindow = window->GetWindowHandle();
    swapChainDesc.Windowed = true;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Create the DX11 API device object, and get a corresponding context.
    ID3D11Device* device3{ nullptr };
    ID3D11DeviceContext* context3{ nullptr };
    IDXGISwapChain* dxgiSwapChain{ nullptr };
    auto hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                    // specify null to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        creationFlags,              // optionally set debug and Direct2D compatibility flags
        featureLevels,              // list of feature levels this app can support
        ARRAYSIZE(featureLevels),   // number of possible feature levels
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &dxgiSwapChain,
        &device3,                    // returns the Direct3D device created
        &FeatureLevel,               // returns feature level of device created
        &context3                    // returns the device immediate context
    );
    if (SUCCEEDED(hr)) {
        D3D11Device.Attach(device3);
        D3D11DeviceContext.Attach(context3);
        DXGISwapChain.Attach(dxgiSwapChain);
    }
    else
        ATLVERIFY(!"D3D11CreateDevice failed");

    IDXGIDevice1* dxgiDevice{ nullptr };
    hr = D3D11Device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
    if (SUCCEEDED(hr))
        DXGIDevice.Attach(dxgiDevice);
    else
        ATLVERIFY(!"D3D11CreateDevice failed");

    ID2D1Device* device{ nullptr };
    ID2D1DeviceContext* context{ nullptr };
    hr = D2D1Factory->CreateDevice(dxgiDevice, &device);
    if (SUCCEEDED(hr))
        D2D1Device.Attach(device);
    else
        ATLVERIFY(!"D2D1CreateDevice failed");

    hr = device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &context);
    if (SUCCEEDED(hr))
        D2D1DeviceContext.Attach(context);
    else
        ATLVERIFY(!"D2D1CreateDeviceContext failed");

    // Ensure that DXGI doesn't queue more than one frame at a time.
    hr = dxgiDevice->SetMaximumFrameLatency(1);
    if (FAILED(hr))
        ATLVERIFY(!"SetMaximumFrameLatency failed");

    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui_ImplWin32_Init(window->GetWindowHandle());
        ImGui_ImplDX11_Init(D3D11Device.p, D3D11DeviceContext.p);
        ImGui::StyleColorsDark();
    }

    Resize();
}

void Direct2D::Resize()
{
    if (DXGISwapChain)
    {
        // Release all outstanding references to the swap chain's buffers.
        D2D1DeviceContext->SetTarget(nullptr);

        D2D1RenderTarget.Release();
        D3D11RenderTargetView.Release();

        // Preserve the existing buffer count and format.
        // Automatically choose the width and height to match the client rect for HWNDs.
        auto hr = DXGISwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr))
            ATLVERIFY(!"ResizeBuffers failed");

        // Direct2D needs the dxgi version of the backbuffer surface pointer.
        CComPtr<IDXGISurface> dxgiBackBuffer;
        hr = DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
        if (FAILED(hr))
            ATLVERIFY(!"GetBuffer failed");

        // Now we set up the Direct2D render target bitmap linked to the swapchain. 
        // Whenever we render to this bitmap, it is directly rendered to the 
        // swap chain associated with the window.
        D2D1_BITMAP_PROPERTIES1 bitmapProperties =
            D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
            );

        // Get a D2D surface from the DXGI back buffer to use as the D2D render target.
        ID2D1Bitmap1* d2dRenderTarget;
        hr = D2D1DeviceContext->CreateBitmapFromDxgiSurface(
            dxgiBackBuffer.p,
            &bitmapProperties,
            &d2dRenderTarget
        );
        if (SUCCEEDED(hr))
            D2D1RenderTarget.Attach(d2dRenderTarget);
        else
            ATLVERIFY(!"CreateBitmapFromDxgiSurface failed");

        // Now we can set the Direct2D render target.
        D2D1DeviceContext->SetTarget(D2D1RenderTarget.p);
        dxgiBackBuffer.Release();

        CComPtr<ID3D11Resource> d3dBackBuffer;
        ID3D11Resource* d3dResource{ nullptr };
        ID3D11RenderTargetView* d3dRenderTargetView{ nullptr };
        hr = DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&d3dResource));
        if (FAILED(hr))
            ATLVERIFY(!"GetBuffer failed");

        d3dBackBuffer.Attach(d3dResource);
        hr = D3D11Device->CreateRenderTargetView(d3dBackBuffer, nullptr, &d3dRenderTargetView);
        if (FAILED(hr))
            ATLVERIFY(!"CreateRenderTargetView failed");

        D3D11RenderTargetView.Attach(d3dRenderTargetView);
        d3dBackBuffer.Release();
    }
}

HRESULT Direct2D::Present()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    static int size_x{ 0 };
    static int size_y{ 0 };

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Appearing);
    ImGui::Begin("Project");
    ImGui::Text("clibui @bajdcc\nScreen #%d", clib::cjsgui::singleton().current_screen());
    ImGui::End();

    const float right = 180;
    const float height = 80;
    auto w = GLOBAL_STATE.bound.Width();
    auto h = GLOBAL_STATE.bound.Height();
    if (w != size_x) {
        size_x = w;
        ImGui::SetNextWindowPos(ImVec2((float)w - right, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(right, height), ImGuiCond_Always);
    }
    else {
        ImGui::SetNextWindowPos(ImVec2((float)w - right, 0), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(right, height), ImGuiCond_Appearing);
    }
    ImGui::Begin("Information");
    ImGui::Text("%.2f ms (%.1f FPS)\n[GC] Alive: %d\n[GC] Cached: %d",
        1000.0f / ImGui::GetIO().Framerate,
        ImGui::GetIO().Framerate,
        GLOBAL_STATE.total_obj, GLOBAL_STATE.cache_obj);
    ImGui::End();

    if (GLOBAL_STATE.is_logging) {
        clib::cjsgui::singleton().render_log();
    }

    ImGui::Render();
    auto p = D3D11RenderTargetView.p;
    Direct2D::Singleton().GetDirect3DDeviceContext()->OMSetRenderTargets(1, &p, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return DXGISwapChain->Present(1, 0);
}

Direct2D::~Direct2D()
{
    WICImagingFactory = nullptr;
    DWriteFactory = nullptr;
    D2D1Factory = nullptr;
    ReportLiveObjects();
    ::CoUninitialize();
}

void Direct2D::ReportLiveObjects()
{
#if 0
    HRESULT hr;
    HMODULE hDxgiDebug = GetModuleHandle(_T("Dxgidebug.dll"));
    if (!hDxgiDebug) return;
    typedef HRESULT(WINAPI* pfnDXGIGetDebugInterface)(REFIID riid, void** ppDebug);
    pfnDXGIGetDebugInterface _DXGIGetDebugInterface = (pfnDXGIGetDebugInterface)GetProcAddress(hDxgiDebug, "DXGIGetDebugInterface");
    if (!_DXGIGetDebugInterface) return;
    CComPtr<IDXGIDebug1> dxgiDebug;
    hr = _DXGIGetDebugInterface(__uuidof(IDXGIDebug1), (void**)&dxgiDebug);
    if (FAILED(hr))	return;
    hr = dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
    if (FAILED(hr))	return;
#endif
    ImGui_ImplDX11_Shutdown();
    ImGui::DestroyContext();
}

CComPtr<ID2D1Factory1> Direct2D::GetDirect2DFactory()
{
    return D2D1Factory;
}

CComPtr<IDWriteFactory> Direct2D::GetDirectWriteFactory()
{
    return DWriteFactory;
}

CComPtr<IWICImagingFactory> Direct2D::GetWICImagingFactory()
{
    return WICImagingFactory;
}

CComPtr<ID2D1Device> Direct2D::GetDirect2DDevice()
{
    return D2D1Device;
}

CComPtr<ID2D1DeviceContext> Direct2D::GetDirect2DDeviceContext()
{
    return D2D1DeviceContext;
}

CComPtr<IDXGIDevice1> Direct2D::GetDXGIDevice()
{
    return DXGIDevice;
}

CComPtr<ID3D11Device> Direct2D::GetDirect3DDevice()
{
    return D3D11Device;
}

CComPtr<ID3D11DeviceContext> Direct2D::GetDirect3DDeviceContext()
{
    return D3D11DeviceContext;
}

CComPtr<ID3D11RenderTargetView> Direct2D::GetDirect3DRenderTargetView()
{
    return D3D11RenderTargetView;
}

CComPtr<IDXGISwapChain> Direct2D::GetDXGISwapChain()
{
    return DXGISwapChain;
}

Direct2D& Direct2D::Singleton()
{
    static Direct2D d2d;
    return d2d;
}