﻿#include "stdafx.h"
#include "JS2D.h"
#include "cjstypes.h"
#include "cjsgui.h"
#include "render/Direct2DRenderTarget.h"
#include <ui\window\Window.h>
#include <render\Direct2D.h>

#define FRAME (1.0 / 30)

void JS2DEngine::Initialize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    logoFont.size = 20;
    logoFont.fontFamily = "宋体";
    logoFont.bold = false;
    logoFont.italic = false;
    logoFont.underline = false;
    loggingFont.size = 12;
    loggingFont.fontFamily = "Courier New";
    loggingFont.bold = false;
    loggingFont.italic = false;
    loggingFont.underline = false;
    bgColorLog = CColor(0, 0, 0, 220);
    brushes.cmdFont.size = 16;
    brushes.cmdFont.fontFamily = "Courier New";
    brushes.cmdFont.bold = false;
    brushes.cmdFont.italic = false;
    brushes.cmdFont.underline = false;
    brushes.gbkFont.size = 16;
    brushes.gbkFont.fontFamily = "楷体";
    brushes.gbkFont.bold = false;
    brushes.gbkFont.italic = false;
    brushes.gbkFont.underline = false;
    logoColor = CColor(255, 255, 255);
    last_clock = std::chrono::system_clock::now();
    dt = 30;
    dt_inv = 1.0 / dt;
    rect.X = 0;
    rect.Y = 0;
    font.size = 20;
    font.fontFamily = "宋体";
    font.bold = false;
    font.italic = false;
    font.underline = false;
    backup_font.size = 20;
    backup_font.fontFamily = "宋体";
    backup_font.bold = false;
    backup_font.italic = false;
    backup_font.underline = false;
    bg = rt->CreateDirect2DBrush(bgColor);
    bg_log = rt->CreateDirect2DBrush(bgColorLog);
    logoTF = rt->CreateDirect2DTextFormat(logoFont);
    loggingTF = rt->CreateDirect2DTextFormat(loggingFont);
    logoBrush = rt->CreateDirect2DBrush(logoColor);
    brushes.cmdTF = rt->CreateDirect2DTextFormat(brushes.cmdFont);
    brushes.gbkTF = rt->CreateDirect2DTextFormat(brushes.gbkFont);
    brushes.brush = logoBrush;
    font_format = rt->CreateDirect2DTextFormat(font);
    d2drt = rt;
    clib::cjsgui::singleton().change_target(rt);
}

void JS2DEngine::Finalize()
{
    bg = nullptr;
    bg_log = nullptr;
    logoTF = nullptr;
    loggingTF = nullptr;
    logoBrush = nullptr;
    brushes.cmdTF = nullptr;
    brushes.gbkTF = nullptr;
    brushes.brush = nullptr;
    font_format = nullptr;
}

void JS2DEngine::Resize()
{
    clib::cjsgui::singleton().resize_ui();
}

void JS2DEngine::reset()
{
    clib::cjsgui::singleton().reset();
    auto_fresh = 1;
    GLOBAL_STATE.gui = false;
    rt2.Release();
    bitmap.Release();
    rect.Width = 0;
    rect.Height = 0;
}

static char* ipsf(double ips) {
    static char _ipsf[32];
    if (ips < 1e3) {
        snprintf(_ipsf, sizeof(_ipsf), "%.1f", ips);
    }
    else if (ips < 1e6) {
        snprintf(_ipsf, sizeof(_ipsf), "%.1fK", ips * 1e-3);
    }
    else if (ips < 1e9) {
        snprintf(_ipsf, sizeof(_ipsf), "%.1fM", ips * 1e-6);
    }
    return _ipsf;
}

void JS2DEngine::Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    auto now = std::chrono::system_clock::now();
    // 计算每帧时间间隔
    dt = std::chrono::duration_cast<std::chrono::duration<double>>(now - last_clock).count();
    cycles += clib::cjsgui::singleton().reset_cycles();

    auto inv = 1.0 / dt;
    if (dt > FRAME) {
        frame = clib::cjsgui::singleton().get_frame();
        ips = cycles * dt;
        cycles = 0;
        dt = min(dt, FRAME);
        dt_inv = 1.0 / dt;
        last_clock = now;
        clib::cjsgui::singleton().clear_frame();
    }

    rt->FillRectangle(
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
        bg
    );
    clib::cjsgui::singleton().draw(rt, bounds, brushes, paused, dt_inv * FRAME);
    if (GLOBAL_STATE.gui)
    {
        if (!bitmap)
        {
            if (rect.Width == 0 || rect.Height == 0) {
                auto size = bounds.Size();
                rect.Width = size.cx;
                rect.Height = size.cy;
            }
            rt2 = d2drt.lock()->CreateBitmapRenderTarget(D2D1::SizeF((float)rect.Width, (float)rect.Height));
            rt2->GetBitmap(&bitmap);
            rt2->BeginDraw();
            rt2->Clear(D2D1::ColorF(D2D1::ColorF::Black));
            rt2->EndDraw();
        }
        if (auto_fresh >= 1)
        {
            bitmap = nullptr;
            rt2->GetBitmap(&bitmap);
        }
        if (auto_fresh == 2) {
            auto_fresh = 0;
        }
        if (GLOBAL_STATE.gui_blur > 0.0f) {
            CComPtr<ID2D1Effect> gaussianBlurEffect;
            auto dev = Direct2D::Singleton().GetDirect2DDeviceContext();
            dev->CreateEffect(CLSID_D2D1GaussianBlur, &gaussianBlurEffect);
            gaussianBlurEffect->SetInput(0, bitmap);
            gaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, GLOBAL_STATE.gui_blur);
            dev->DrawImage(
                gaussianBlurEffect.p,
                D2D1::Point2F((FLOAT)bounds.left, (FLOAT)bounds.top),
                D2D1::RectF(0.0f, 0.0f, (FLOAT)bounds.Width(), (FLOAT)bounds.Height()),
                D2D1_INTERPOLATION_MODE_LINEAR
            );
        }
        else
            rt->DrawBitmap(
                bitmap,
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                1.0f,
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
            );
    }
    else if (bitmap) {
        reset();
    }

    if (GLOBAL_STATE.reboot)
        reset();
    else if (GLOBAL_STATE.stop) {
        paused = !paused;
        GLOBAL_STATE.stop = false;
    }
}
