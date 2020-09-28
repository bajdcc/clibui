#ifndef JS2D_JS2D_H
#define JS2D_JS2D_H
#include <memory>
#include <render\Direct2DRenderTarget.h>

class JS2DEngine
{
public:
    void Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    void Initialize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Finalize();
    void Resize();

    struct BrushBag {
        Font cmdFont; std::shared_ptr<D2DTextFormatPackage> cmdTF;
        Font gbkFont; std::shared_ptr<D2DTextFormatPackage> gbkTF;
        CComPtr<ID2D1SolidColorBrush> brush;
    } brushes;

private:

    void reset();

private:
    CComPtr<ID2D1SolidColorBrush> bg;
    CComPtr<ID2D1SolidColorBrush> bg_log;
    CColor bgColor;
    CColor bgColorLog;
    Font logoFont;
    Font loggingFont;
    CColor logoColor;
    std::shared_ptr<D2DTextFormatPackage> logoTF;
    std::shared_ptr<D2DTextFormatPackage> loggingTF;
    CComPtr<ID2D1SolidColorBrush> logoBrush;

private:
    CComPtr<ID2D1Bitmap> bitmap;
    CComPtr<ID2D1Bitmap> bitmap_effect;
    CComPtr<ID2D1BitmapRenderTarget> rt2;
    std::weak_ptr<Direct2DRenderTarget> d2drt;
    WICRect rect;
    D2D1_RECT_U d2drect;
    int auto_fresh{ 1 };

private:
    std::chrono::system_clock::time_point last_clock;
    double dt{ 0 };
    double dt_inv{ 0 };
    int cycles{ 0 };
    double ips{ 0 };
    int frame{ 0 };
    bool paused{ false };

private:
    std::shared_ptr<D2DTextFormatPackage> font_format;
    Font font, backup_font;
};

#endif