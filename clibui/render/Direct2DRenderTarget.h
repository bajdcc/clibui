﻿#ifndef RENDER_D2D_RENDERTARGET_H
#define RENDER_D2D_RENDERTARGET_H

#include <ui/gdi/Gdi.h>

class Window;

struct D2DTextFormatPackage : std::enable_shared_from_this<D2DTextFormatPackage>
{
    CComPtr<IDWriteTextFormat>		textFormat;
    DWRITE_TRIMMING					trimming;
    CComPtr<IDWriteInlineObject>	ellipseInlineObject;
};

class Direct2DRenderTarget : public std::enable_shared_from_this<Direct2DRenderTarget>
{
public:
    Direct2DRenderTarget(std::weak_ptr<Window> _window);
    ~Direct2DRenderTarget();

    void Init();

    CComPtr<ID2D1RenderTarget> GetDirect2DRenderTarget();
    CComPtr<ID2D1RenderTarget> SetDirect2DRenderTarget(CComPtr<ID2D1RenderTarget>);
    void SetTextAntialias(bool antialias, bool verticalAntialias);
    void StartRendering();
    HRESULT StopRendering();
    HRESULT Present();

    CComPtr<ID2D1SolidColorBrush> CreateDirect2DBrush(CColor color);
    CComPtr<ID2D1LinearGradientBrush> CreateDirect2DLinearBrush(CColor c1, CColor c2);
    std::shared_ptr<D2DTextFormatPackage> CreateDirect2DTextFormat(const Font& font);
    CComPtr<ID2D1BitmapRenderTarget> CreateBitmapRenderTarget(D2D1_SIZE_F size);

    CComPtr<IWICBitmap> CreateBitmap(UINT width, UINT height);
    CComPtr<IWICBitmap> CreateImageFromFile(const CStringA& path, int index = 0);
    CComPtr<IWICBitmap> CreateImageFromMemory(LPVOID buffer, int length, int index = 0);
    CComPtr<IWICBitmap> CreateImageFromBitmap(HBITMAP handle, int index = 0);
    CComPtr<IWICBitmap> CreateImageFromIcon(HICON handle, int index = 0);

    CComPtr<ID2D1Bitmap> GetBitmapFromWIC(CComPtr<IWICBitmap> bitmap);

    CComPtr<ID2D1PathGeometry> CreatePathGeometry();

protected:
    CComPtr<IDWriteRenderingParams> CreateRenderingParams(DWRITE_RENDERING_MODE renderingMode, CComPtr<IDWriteRenderingParams> defaultParams, CComPtr<IDWriteFactory> dwriteFactory);
    CComPtr<IWICBitmap> GetBitmap(CComPtr<IWICBitmapDecoder> source, int index);

    std::weak_ptr<Window> window;
    CComPtr<ID2D1RenderTarget> d2dRenderTarget;

    CComPtr<IDWriteRenderingParams> noAntialiasParams;
    CComPtr<IDWriteRenderingParams> horizontalAntialiasParams;
    CComPtr<IDWriteRenderingParams> bidirectionalAntialiasParams;

    CComPtr<IWICImagingFactory> imagingFactory;
};

#endif