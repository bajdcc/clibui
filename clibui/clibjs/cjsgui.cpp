﻿//
// Project: cliblisp
// Created by bajdcc
//

#include "stdafx.h"
#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>
#include "cjsgui.h"
#include "cjsnet.h"
#include "ui/window/Window.h"

#ifdef REPORT_ERROR
#undef REPORT_ERROR
#endif
#define REPORT_ERROR 1
#ifdef REPORT_ERROR_FILE
#undef REPORT_ERROR_FILE
#endif
#define REPORT_ERROR_FILE "js_runtime.log"
#define REPORT_STAT 1
#define REPORT_STAT_FILE "js_stat.log"
#define STAT_DELAY_N 50
#define STAT_MAX_N 10

#define AST_FILE "js_ast.log"

#define IPS_STAT_TIME 1s

#define ENTRY_FILE "src/main.js"

#define MAKE_ARGB(a,r,g,b) ((uint32_t)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(a))<<24)))
#define MAKE_RGB(r,g,b) MAKE_ARGB(255,r,g,b)
#define GET_R(rgb) (LOBYTE(rgb))
#define GET_G(rgb) (LOBYTE(((WORD)(rgb)) >> 8))
#define GET_B(rgb) (LOBYTE((WORD)((rgb)>>16)))
#define GET_A(rgb) (LOBYTE((rgb)>>24))

namespace clib {

    cjsgui::cjsgui() {
        std::fill(screen_ref.begin(), screen_ref.end(), 0);
        init_screen(0);
        switch_screen(0);
        switch_screen_display(0);
        init_cubic_bezier();
    }

    cjsgui& cjsgui::singleton() {
        static clib::cjsgui gui;
        return gui;
    }

    void cjsgui::reset() {
        if (vm) {
            vm.reset();
            std::fill(screens.begin(), screens.end(), nullptr);
            for (auto& s : screens) {
                s.reset(nullptr);
            }
            init_screen(0);
            screen_ptr = -1;
            switch_screen(0);
            screen_id = -1;
            switch_screen_display(0);
            std::fill(screen_ref.begin(), screen_ref.end(), 0);
            reset_cycles();
            reset_ips();
        }
        stop_music();
        running = false;
        exited = false;
        global_state.drawing = false;
        global_state.render_queue.clear();
        global_state.render_queue_auto.clear();
        global_state.render_queue_bk.clear();
        global_state.render_queue_auto_bk.clear();
        global_state.ui_focus.reset();
        global_state.ui_hover.reset();
        global_state.reboot = false;
        global_state.stop = false;
    }

    void cjsgui::clear_cache()
    {
        if (vm)
            vm->clear_cache();
    }

    bool cjsgui::begin_render()
    {
        global_state.drawing = false;
        global_state.painting = false;
        return global_state.renderTarget;
    }

    void cjsgui::end_render()
    {
        std::swap(global_state.render_queue, global_state.render_queue_auto);
        global_state.render_queue_auto.clear();
        std::swap(global_state.render_queue_bk, global_state.render_queue_auto_bk);
        global_state.render_queue_auto_bk.clear();
        global_state.painting = true;
    }

    void cjsgui::change_target(std::shared_ptr<Direct2DRenderTarget> renderTarget)
    {
        global_state.canvas = renderTarget;
        init_render_target();
        if (vm)
            vm->change_target();
    }

    void cjsgui::draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, const JS2DEngine::BrushBag& brushes, bool paused, decimal fps) {
        if (!paused && !entered) {
            if (!cycle_set) {
                if (cycle_stable > 0) {
                    if (fps > GUI_MAX_FPS_RATE) {
                        cycle = min(cycle << 1, GUI_MAX_CYCLE);
                    }
                    else if (fps < GUI_MIN_FPS_RATE) {
                        cycle_stable--;
                    }
                }
                else if (fps > GUI_MAX_FPS_RATE) {
                    if (cycle_speed >= 0) {
                        cycle_speed = min(cycle_speed + 1, GUI_MAX_SPEED);
                        cycle = min(cycle << cycle_speed, GUI_MAX_CYCLE);
                    }
                    else {
                        cycle_speed = 0;
                    }
                }
                else if (fps < GUI_MIN_FPS_RATE) {
                    if (cycle_speed <= 0) {
                        cycle_speed = max(cycle_speed - 1, -GUI_MAX_SPEED);
                        cycle = max(cycle >> (-cycle_speed), GUI_MIN_CYCLE);
                    }
                    else {
                        cycle_speed = 0;
                    }
                }
                else {
                    if (cycle_stable == 0) {
                        cycle_speed = 0;
                        cycle_stable = GUI_CYCLE_STABLE;
                    }
                }
            }
            entered = true;
            for (int i = 0; i < ticks + cycle_speed; ++i) {
                tick();
            }
            entered = false;
        }
        using namespace std::chrono_literals;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - last_time) >= IPS_STAT_TIME) {
            last_time = std::chrono::system_clock::now();
            reset_ips();
        }
        draw_window(bounds, brushes);
    }
    
    static int js_get_num_length(int x)
    {
        int len = 0;
        while (x)
        {
            x /= 10;
            len++;
        }
        return len;
    }

    void cjsgui::draw_text(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, const JS2DEngine::BrushBag& brushes, bool cached) {
        if (!screens[screen_id])
            return;
        auto& scr = *screens[screen_id].get();
        auto& view = scr.view;
        auto& line = scr.line;
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& color_bg_stack = scr.color_bg_stack;
        auto& color_fg_stack = scr.color_fg_stack;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;

        int w = bounds.Width();
        int h = bounds.Height();
        int width = cols * GUI_FONT_W;
        int height = rows * GUI_FONT_H;

        int x = max((w - width) / 2, 0);
        int y = max((h - height) / 2, 0);
        auto x0 = x;
        auto y0 = y;
        auto old_x = x;
        char c;

        CComPtr<ID2D1SolidColorBrush> b;
        rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &b);
        TCHAR s[2] = { 0 };
        char sc[3] = { 0 };
        bool ascii = true;
        bool ascii_head = false;
        auto view_end = min(view + rows, line + 1);

        auto num_k = max(js_get_num_length(view_end - 1), 1);
        CString line_no;

        if (!cached) {
            for (auto i = view; i < view_end; ++i) {
                if (input_state) {
                    if (i >= ptr_my && i <= ptr_ry) {
                        b->SetColor(D2D1::ColorF(87.0f / 255.0f, 116.0f / 255.0f, 48.0f / 255.0f, 0.4f));
                        rt->FillRectangle(
                            D2D1::RectF((float)x - num_k * GUI_FONT_W - 5, (float)y + GUI_FONT_H_1,
                                (float)x, (float)y + GUI_FONT_H_2), b);
                    }
                }
                line_no.Format(L"%d", i);
                while (line_no.GetLength() != num_k)
                    line_no = L" " + line_no;
                if (ptr_y == i)
                    b->SetColor(D2D1::ColorF(55.0f / 255.0f, 173.0f / 255.0f, 231.0f / 255.0f));
                else
                    b->SetColor(D2D1::ColorF(43.0f / 255.0f, 145.0f / 255.0f, 175.0f / 255.0f));
                rt->DrawText(line_no.GetBuffer(0), line_no.GetLength(), brushes.cmdTF->textFormat,
                    D2D1::RectF((float)x - num_k * GUI_FONT_W - 5, (float)y + GUI_FONT_H_1,
                        (float)x, (float)y + GUI_FONT_H_2), b);
                for (auto j = 0; j < cols; ++j) {
                    ascii = true;
                    c = buffer[i * cols + j];
                    if (c < 0 && (i != line || j != cols - 1)) {
                        WORD wd = (((BYTE)c) << 8) | ((BYTE)buffer[i * cols + j + 1]);
                        if (wd >= 0x8140 && wd <= 0xFEFE) { // GBK
                            if (j < cols - 1)
                                ascii = false;
                            else
                                ascii_head = true;
                        }
                    }
                    if (j == 0 && ascii_head) {
                        ascii_head = false;
                        ascii = true;
                    }
                    if (ascii) {
                        if (colors_bg[i * cols + j]) {
                            b->SetColor(D2D1::ColorF(colors_bg[i * cols + j]));
                            rt->FillRectangle(
                                D2D1::RectF((float)x, (float)y + GUI_FONT_H_1,
                                    (float)x + GUI_FONT_W, (float)y + GUI_FONT_H_2), b);
                        }
                        if (c > 0) {
                            if (std::isprint(buffer[i * cols + j])) {
                                b->SetColor(D2D1::ColorF(colors_fg[i * cols + j]));
                                s[0] = c;
                                rt->DrawText(s, 1, brushes.cmdTF->textFormat,
                                    D2D1::RectF((float)x, (float)y + GUI_FONT_H_1,
                                        (float)x + GUI_FONT_W, (float)y + GUI_FONT_H_2), b);
                            }
                            else if (c == '\7') {
                                b->SetColor(D2D1::ColorF(colors_fg[i * cols + j]));
                                rt->FillRectangle(
                                    D2D1::RectF((float)x, (float)y + GUI_FONT_H_1,
                                        (float)x + GUI_FONT_W, (float)y + GUI_FONT_H_2), b);
                            }
                        }
                        else if (c < 0) {
                            b->SetColor(D2D1::ColorF(colors_fg[i * cols + j]));
                            rt->FillRectangle(
                                D2D1::RectF((float)x, (float)y + GUI_FONT_H_1,
                                    (float)x + GUI_FONT_W, (float)y + GUI_FONT_H_2), b);
                        }
                        x += GUI_FONT_W;
                    }
                    else {
                        if (colors_bg[i * cols + j]) {
                            b->SetColor(D2D1::ColorF(colors_bg[i * cols + j]));
                            rt->FillRectangle(
                                D2D1::RectF((float)x, (float)y + GUI_FONT_H_1,
                                    (float)x + GUI_FONT_W * 2, (float)y + GUI_FONT_H_2), b);
                        }
                        sc[0] = c;
                        sc[1] = buffer[i * cols + j + 1];
                        auto utf = cnet::GBKToStringT(sc);
                        s[0] = (TCHAR)(utf[0]);
                        j++;
                        b->SetColor(D2D1::ColorF(colors_fg[i * cols + j]));
                        rt->DrawText(s, 1, brushes.gbkTF->textFormat,
                            D2D1::RectF((float)x + GUI_FONT_W_C1, (float)y + GUI_FONT_H_C1,
                                (float)x + GUI_FONT_W_C2, (float)y + GUI_FONT_H_C2), b);
                        x += GUI_FONT_W * 2;
                    }
                }
                x = old_x;
                y += GUI_FONT_H;
            }
            return;
        }

        if (input_state) {
            input_ticks++;
            if (input_ticks > GUI_INPUT_CARET) {
                input_caret = !input_caret;
                input_ticks = 0;
            }
            if (input_caret) {
                if (ptr_y >= view && ptr_y <= view_end) {
                    auto __y = ptr_y - view;
                    int _x = max((w - width) / 2, 0) + ptr_x * GUI_FONT_W;
                    int _y =max((h - height) / 2, 0) + __y * GUI_FONT_H;
                    b->SetColor(D2D1::ColorF(color_fg_stack.back()));
                    rt->DrawText(_T("_"), 1, brushes.cmdTF->textFormat,
                        D2D1::RectF((float)bounds.left + _x, (float)bounds.top + _y + GUI_FONT_H_1,
                            (float)bounds.left + _x + GUI_FONT_W, (float)bounds.top + _y + GUI_FONT_H_2), b);
                }
            }
        }

        // Draw scroll
        // 0:stop,1-0xf:fade in,0x10-0x1f:show,0x20-0x2f:fade out
        auto& scroll_fade = scr.scroll_fade;
        auto& old_line = scr.old_line;
        auto& old_view = scr.old_view;
        if (old_line != line || old_view != view) {
            auto&& k = (scroll_fade & 0xf0) >> 4;
            auto&& p = scroll_fade & 0xf;
            switch (k) {
            case 0:
                if (scroll_fade == 0)
                    scroll_fade = 1;
                break;
            case 1:
                scroll_fade = 0x10;
                break;
            case 2:
                scroll_fade = 0xf - p;
                if (scroll_fade == 0)
                    scroll_fade = 1;
                break;
            default:
                break;
            }
            old_line = line;
            old_view = view;
        }
        if (scroll_fade) {
            global_state.paintingConsole = true;
            auto&& k = (scroll_fade & 0xf0) >> 4;
            auto&& p = scroll_fade & 0xf;
            auto a = 0.0f;
            switch (k) {
            case 0:
                a = animation.easy_in[p];
                scroll_fade++;
                break;
            case 1:
                a = 1.0f;
                scroll_fade++;
                break;
            case 2:
                a = animation.easy_out[p];
                scroll_fade++;
                if (scroll_fade == 0x30)
                    scroll_fade = 0;
                break;
            default:
                break;
            }
            if (a > 0.0f) {
                int _x = bounds.left + x + cols * GUI_FONT_W;
                int _y = bounds.top + y;
                int _x2 = _x + 20;
                int _y2 = _y + rows * GUI_FONT_H;
                auto r = D2D1::RectF((float)_x, (float)_y, (float)_x2, (float)_y2);
                b->SetColor(D2D1::ColorF(62.0f / 255.0f, 62.0f / 255.0f, 66.0f / 255.0f, a));
                rt->FillRectangle(r, b);
                auto h2 = rows * GUI_FONT_H - 10;
                auto start_y = 1.0f * min(view, line) / line;
                auto end_y = 1.0f * min(line, view + rows) / line;
                r = D2D1::RectF((float)_x + 5, (float)_y + 5 + start_y * h2,
                    (float)_x + 15, (float)_y + 5 + end_y * h2);
                b->SetColor(D2D1::ColorF(104.0f / 255.0f, 104.0f / 255.0f, 104.0f / 255.0f, a));
                rt->FillRectangle(r, b);
            }
        }
    }

    static void cubic_bezier(int n, float x1, float y1, float x2, float y2, std::vector<D2D1_POINT_2F> &out) {
        auto p = 1.0f / n;
        for (auto i = 0; i < n; i++) {
            auto t = p * i;
            auto k = t * t * t;
            auto q = 3 * t * (1 - t);
            out[i] = D2D1::Point2F(
                k + (q * (x1 + t * (x2 - x1))),
                k + (q * (y1 + t * (y2 - y1))));
        }
    }

    void cjsgui::init_cubic_bezier()
    {
        const auto N = 128;
        const auto TARGET = 16;
        std::vector<D2D1_POINT_2F> v(N);
        cubic_bezier(N, .42f, 0, 1.0f, 1.0f, v);
        auto gap = 1.0f / TARGET;
        auto k = 0.0f;
        animation.easy_in.resize(TARGET);
        std::fill(animation.easy_in.begin(), animation.easy_in.end(), 1.0f);
        for (auto i = 0, j = 0; i < N; i++) {
            if (v[i].x >= k) {
                animation.easy_in[j++] = max(0.0f, min(v[i].y, 1.0f));
                if (j == TARGET)
                    break;
                k = gap * j;
            }
        }
        animation.easy_out.resize(TARGET);
        for (auto i = 0; i < TARGET; i++) {
            animation.easy_out[i] = max(0, 1.0f - animation.easy_in[i]);
        }
    }

    void cjsgui::draw_window(const CRect& bounds, const JS2DEngine::BrushBag& brushes)
    {
        if (!bounds.IsRectEmpty() && global_state.bound != bounds) {
            global_state.bound = bounds;
            init_render_target();
            if (vm)
                vm->resize();
            global_state.painting = true;
            global_state.paintingConsole = true;
        }
        auto rt = global_state.canvas.lock()->GetDirect2DRenderTarget();
        if (global_state.canvas.lock() && !global_state.bound.IsRectEmpty()) {
            if (global_state.renderTarget) {
                if (global_state.painting) {
                    global_state.painting = false;
                    global_state.renderTarget->BeginDraw();
                    global_state.renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White, 0));
                    for (const auto& s : global_state.render_queue) {
                        if (s.lock()) {
                            s.lock()->render();
                        }
                    }
                    global_state.renderTarget->EndDraw();
                    global_state.bitmap = nullptr;
                    global_state.renderTarget_bitmap->GetBitmap(&global_state.bitmap);
                }
            }
            if (global_state.bitmap) {
                rt->DrawBitmap(
                    global_state.bitmap,
                    D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                    1.0f,
                    D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
                );
            }
            if (global_state.renderTargetConsole) {
                if (global_state.paintingConsole) {
                    global_state.paintingConsole = false;
                    global_state.renderTargetConsole->BeginDraw();
                    global_state.renderTargetConsole->Clear(D2D1::ColorF(D2D1::ColorF::White, 0));
                    draw_text(global_state.renderTargetConsole, bounds, brushes, false);
                    global_state.renderTargetConsole->EndDraw();
                    global_state.bitmapConsole = nullptr;
                    global_state.renderTarget_bitmapConsole->GetBitmap(&global_state.bitmapConsole);
                }
            }
            if (global_state.bitmapConsole) {
                rt->DrawBitmap(
                    global_state.bitmapConsole,
                    D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                    1.0f,
                    D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
                );
            }
            draw_text(rt, bounds, brushes, true);
        }
    }

    void cjsgui::init_render_target()
    {
        if (global_state.canvas.lock() && !global_state.bound.IsRectEmpty()) {
            global_state.drawing = true;
            global_state.painting = true;
            global_state.renderTarget_bitmap = global_state.canvas.lock()->CreateBitmapRenderTarget(
                D2D1::SizeF((float)global_state.bound.Width(), (float)global_state.bound.Height()));
            global_state.renderTarget = global_state.renderTarget_bitmap;
            global_state.drawingConsole = true;
            global_state.paintingConsole = true;
            global_state.paintingScroll = false;
            global_state.renderTarget_bitmapConsole = global_state.canvas.lock()->CreateBitmapRenderTarget(
                D2D1::SizeF((float)global_state.bound.Width(), (float)global_state.bound.Height()));
            global_state.renderTargetConsole = global_state.renderTarget_bitmapConsole;
        }
    }

    void cjsgui::reset_ips()
    {
        if (vm)
            vm->reset_ips();
    }

    void cjsgui::tick() {
        if (exited)
            return;
        if (running) {
            if (vm->get_state() == 1) {
                vm->set_state(2);
                std::vector<std::string> args;
                if (compile(ENTRY_FILE, args, decltype(args)()) == -1) {
                    running = false;
                }
            }
            try {
                if (!vm->run(cycle, cycles)) {
                    running = false;
                    exited = true;
                    put_string("\n[!] clibjs exited.");
                    vm.reset();
                }
            }
            catch (const cjs_exception& e) {
                ATLTRACE("[SYSTEM] ERR  | RUNTIME ERROR: %s\n", e.message().c_str());
#if REPORT_ERROR
                {
                    std::ofstream log(REPORT_ERROR_FILE, std::ios::app | std::ios::out);
                    log << "[SYSTEM] ERR  | RUNTIME ERROR: " << e.message() << std::endl;
                }
#endif
                exited = true;
                running = false;
                //vm.reset();
                //gen.reset();
                //running = false;
            }
        }
        else {
            if (!vm) {
                vm = std::make_unique<cjs>();
                running = true;
            }
        }
    }

    void cjsgui::put_string(const std::string& str) {
        auto& scr = *screens[screen_ptr].get();
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& input_state = scr.input_state;
        auto& input_delay = scr.input_delay;
        if (input_state) {
            if (ptr_x == ptr_rx && ptr_x == ptr_mx &&
                ptr_y == ptr_ry && ptr_y == ptr_my) {
                input_set(false);
                for (auto& s : str) {
                    put_char(s);
                }
                input_set(true);
            }
            else {
                for (auto& s : str) {
                    input_delay.push_back(s);
                }
            }
        }
        else {
            for (auto& s : str) {
                put_char(s);
            }
        }
    }

    void cjsgui::put_char(int c) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& view = scr.view;
        auto& line = scr.line;
        auto& valid = scr.valid;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& color_bg_stack = scr.color_bg_stack;
        auto& color_fg_stack = scr.color_fg_stack;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& input_delay = scr.input_delay;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& cmd_state = scr.cmd_state;
        auto& cmd_string = scr.cmd_string;
        auto& size = scr.size;

        if (!global_state.paintingConsole)
            global_state.paintingConsole = true;

        if (cmd_state) {
            if (c == '\033') {
                static std::string pat{ R"([A-Za-z][0-9a-f]{1,8})" };
                static std::regex re(pat);
                std::smatch res;
                std::string s(cmd_string.begin(), cmd_string.end());
                if (std::regex_search(s, res, re)) {
                    try {
                        exec_cmd(s);
                    }
                    catch (const std::invalid_argument&) {
                        // '/dev/random' : cause error
                    }
                }
                cmd_string.clear();
                cmd_state = false;
            }
            else {
                cmd_string.push_back(c);
            }
            return;
        }
        else if (c == '\033') {
            cmd_state = true;
            return;
        }

        if (!input_state && !input_delay.empty()) {
            auto delay = input_delay;
            delay.push_back(0);
            input_delay.clear();
            put_string(delay.data());
        }

        if (c == 0)
            return;
        if (c == '\n') {
            new_line();
        }
        else if (c == '\b') {
            auto ascii = true;
            if ((ptr_x != 0 || ptr_y != 0) &&
                ptr_mx + ptr_my * cols < ptr_x + ptr_y * cols &&
                ptr_x + ptr_y * cols <= ptr_rx + ptr_ry * cols) {
                auto cc = buffer[ptr_y * cols + ptr_x - 1];
                if (cc < 0) {
                    WORD wd = (((BYTE)cc) << 8) | ((BYTE)buffer[ptr_y * cols + ptr_x]);
                    if (wd >= 0x8140 && wd <= 0xFEFE) { // GBK
                        ascii = false;
                    }
                }
                if (!input_state) {
                    forward(ptr_x, ptr_y, false);
                    if (ptr_x == cols)
                        ptr_x--;
                    valid[ptr_y]--;
                    draw_char('\u0000');
                    if (!ascii) {
                        forward(ptr_x, ptr_y, false);
                        if (ptr_x == cols)
                            ptr_x--;
                        valid[ptr_y]--;
                        draw_char('\u0000');
                    }
                }
                else {
                    if (ptr_mx + ptr_my * cols < ptr_x + ptr_y * cols && ptr_x + ptr_y * cols <= ptr_rx + ptr_ry * cols) {
                        auto last = ptr_y;
                        for (auto i = last; i <= line; i++) {
                            if (valid[i] != cols) {
                                last = i;
                                break;
                            }
                            last = i;
                        }
                        auto t = 0;
                        auto x2 = ptr_x;
                        auto y2 = ptr_y;
                        forward(ptr_x, ptr_y, false);
                        if (ptr_x == cols) {
                            ptr_x--;
                        }
                        if (!ascii && ptr_mx + ptr_my * cols < ptr_x + ptr_y * cols) {
                            auto x = ptr_x;
                            auto y = ptr_y;
                            forward(ptr_x, ptr_y, false);
                            if (ptr_x == cols) {
                                ptr_x--;
                            }
                        }
                        if (!ascii) {
                            t = 2;
                        }
                        else {
                            if (x2 == 0)
                                t = 0;
                            else
                                t = 1;
                        }
                        auto k = cols * y2 + x2 - (cols * ptr_y + ptr_x);
                        view = max(0, ptr_y - rows + 1);
                        auto x = valid[last];
                        auto end_x = x;
                        auto end_y = last;
                        std::copy(buffer.begin() + cols * y2 + x2, buffer.begin() + cols * end_y + end_x, buffer.begin() + cols * ptr_y + ptr_x);
                        std::copy(colors_bg.begin() + cols * y2 + x2, colors_bg.begin() + cols * end_y + end_x, colors_bg.begin() + cols * ptr_y + ptr_x);
                        std::copy(colors_fg.begin() + cols * y2 + x2, colors_fg.begin() + cols * end_y + end_x, colors_fg.begin() + cols * ptr_y + ptr_x);
                        if (x < k) {
                            line--;
                            if (last == y2) {
                                valid[ptr_y] = min(cols, valid[ptr_y] + x - t);
                            }
                            else {
                                valid[ptr_y] = cols;
                                valid[last - 1] = min(cols, valid[last - 1] + x - k);
                            }
                            std::copy(valid.begin() + last + 1, valid.end(), valid.begin() + last);
                            std::copy(buffer.begin() + cols * (last + 1), buffer.end(), buffer.begin() + cols * last);
                            std::copy(colors_bg.begin() + cols * (last + 1), colors_bg.end(), colors_bg.begin() + cols * last);
                            std::copy(colors_fg.begin() + cols * (last + 1), colors_fg.end(), colors_fg.begin() + cols * last);
                            valid.resize(valid.size() - 1);
                            buffer.resize(buffer.size() - cols);
                            colors_bg.resize(colors_bg.size() - cols);
                            colors_fg.resize(colors_fg.size() - cols);
                            std::fill(buffer.begin() + (cols * end_y + end_x - k), buffer.begin() + cols * end_y, 0);
                            std::fill(colors_bg.begin() + (cols * end_y + end_x - k), colors_bg.begin() + cols * end_y, color_bg);
                            std::fill(colors_fg.begin() + (cols * end_y + end_x - k), colors_fg.begin() + cols * end_y, color_fg);
                        }
                        else {
                            if (x2 == 0 && ptr_y != last) {
                                valid[ptr_y] = min(cols, valid[ptr_y] + k);
                                valid[last] -= k;
                            }
                            else {
                                valid[last] -= k;
                            }
                            std::fill(buffer.begin() + (cols * end_y + end_x - k), buffer.begin() + cols * end_y + end_x, 0);
                            std::fill(colors_bg.begin() + (cols * end_y + end_x - k), colors_bg.begin() + cols * end_y + end_x, color_bg);
                            std::fill(colors_fg.begin() + (cols * end_y + end_x - k), colors_fg.begin() + cols * end_y + end_x, color_fg);
                        }
                        ptr_ry = line;
                        ptr_rx = valid[line];
                    }
                }
            }
        }
        else if (c == '\r') {
            if (!input_state)
                ptr_x = 0;
        }
        else if (c == '\f') {
            ptr_x = 0;
            ptr_y = 0;
            ptr_mx = -1;
            ptr_my = -1;
            ptr_rx = -1;
            ptr_ry = -1;
            view = 0;
            line = 0;

            valid.push_back(0);
            buffer.resize(cols);
            std::fill(buffer.begin(), buffer.end(), 0);
            colors_bg.resize(cols);
            color_bg = MAKE_RGB(30, 30, 30);
            std::fill(colors_bg.begin(), colors_bg.end(), color_bg);
            colors_fg.resize(cols);
            color_fg = MAKE_RGB(241, 241, 241);
            std::fill(colors_fg.begin(), colors_fg.end(), color_fg);
            color_bg_stack.clear();
            color_bg_stack.push_back(color_bg);
            color_fg_stack.clear();
            color_fg_stack.push_back(color_fg);
        }
        else {
            if (input_state) {
                if (ptr_mx + ptr_my * cols <= ptr_x + ptr_y * cols &&
                    ptr_x + ptr_y * cols <= ptr_rx + ptr_ry * cols) {
                    auto last = ptr_y;
                    for (auto i = last; i <= line; i++) {
                        if (valid[i] != cols) {
                            last = i;
                            break;
                        }
                        last = i;
                    }
                    auto x = valid[last]++;
                    if (x == cols - 1) {
                        line++;
                        valid.resize(valid.size() + 1);
                        buffer.resize(buffer.size() + cols);
                        colors_bg.resize(colors_bg.size() + cols);
                        colors_fg.resize(colors_fg.size() + cols);
                        std::copy_backward(valid.begin() + last + 1, valid.end() - 1, valid.end());
                        std::copy_backward(buffer.begin() + cols * (last + 1), buffer.end() - cols, buffer.end());
                        std::copy_backward(colors_bg.begin() + cols * (last + 1), colors_bg.end() - cols, buffer.end());
                        std::copy_backward(colors_fg.begin() + cols * (last + 1), colors_fg.end() - cols, buffer.end());
                        valid[last + 1] = 0;
                        std::fill(buffer.begin() + cols * (last + 1), buffer.begin() + cols * (last + 2), 0);
                        std::fill(colors_bg.begin() + cols * (last + 1), colors_bg.begin() + cols * (last + 2), color_bg);
                        std::fill(colors_fg.begin() + cols * (last + 1), colors_fg.begin() + cols * (last + 2), color_fg);
                    }
                    auto end_x = x;
                    auto end_y = last;
                    std::copy_backward(buffer.begin() + cols * ptr_y + ptr_x, buffer.begin() + cols * end_y + end_x + 1, buffer.begin() + cols * end_y + end_x + 2);
                    std::copy_backward(colors_bg.begin() + cols * ptr_y + ptr_x, colors_bg.begin() + cols * end_y + end_x + 1, colors_bg.begin() + cols * end_y + end_x + 2);
                    std::copy_backward(colors_fg.begin() + cols * ptr_y + ptr_x, colors_fg.begin() + cols * end_y + end_x + 1, colors_fg.begin() + cols * end_y + end_x + 2);
                    draw_char(c);
                    forward(ptr_x, ptr_y, true);
                    auto view_end = min(view + rows, line + 1);
                    if (ptr_y >= view_end) {
                        view++;
                    }
                    forward(ptr_rx, ptr_ry, true);
                }
            }
            else {
                if (ptr_x != cols - 1) {
                    valid.back() = ptr_x + 1;
                    draw_char(c);
                    forward(ptr_x, ptr_y, true);
                }
                else {
                    valid.back() = cols;
                    draw_char(c);
                    new_line();
                }
            }
        }
    }

    void cjsgui::input_call(int c) {
        auto old = screen_ptr;
        screen_ptr = screen_id;
        input(c);
        screen_ptr = old;
    }

    void cjsgui::new_line() {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& view = scr.view;
        auto& line = scr.line;
        auto& valid = scr.valid;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& input_state = scr.input_state;

        if (input_state) {
            if (ptr_mx + ptr_my * cols <= ptr_x + ptr_y * cols &&
                ptr_x + ptr_y * cols <= ptr_rx + ptr_ry * cols) {
                auto last = ptr_y;
                for (auto i = last; i <= line; i++) {
                    if (valid[i] != cols) {
                        last = i;
                        break;
                    }
                    last = i;
                }
                if (ptr_x > valid[last]) {
                    auto end_x = valid[last];
                    auto end_y = last;
                    auto len = valid[ptr_y] - ptr_x;
                    valid[last] += valid[ptr_y] - ptr_x;
                    valid[ptr_y] = ptr_x;
                    std::copy_backward(buffer.begin() + cols * ptr_y + ptr_x, buffer.begin() + cols * end_y + end_x, buffer.begin() + cols * last + valid[last]);
                    std::copy_backward(colors_bg.begin() + cols * ptr_y + ptr_x, colors_bg.begin() + cols * end_y + end_x, colors_bg.begin() + cols * last + valid[last]);
                    std::copy_backward(colors_fg.begin() + cols * ptr_y + ptr_x, colors_fg.begin() + cols * end_y + end_x, colors_fg.begin() + cols * last + valid[last]);
                    std::fill(buffer.begin() + cols * ptr_y + ptr_x, buffer.begin() + cols * ptr_y + ptr_x + len, 0);
                    std::fill(colors_bg.begin() + cols * ptr_y + ptr_x, colors_bg.begin() + cols * ptr_y + ptr_x + len, color_bg);
                    std::fill(colors_fg.begin() + cols * ptr_y + ptr_x, colors_fg.begin() + cols * ptr_y + ptr_x + len, color_fg);
                    ptr_y++;
                }
                else {
                    line++;
                    view = max(view, line - rows + 1);
                    auto end_x = valid[last];
                    auto end_y = last;
                    auto len = valid[ptr_y] - ptr_x;
                    valid.resize(valid.size() + 1);
                    buffer.resize(buffer.size() + cols);
                    colors_bg.resize(colors_bg.size() + cols);
                    colors_fg.resize(colors_fg.size() + cols);
                    std::copy_backward(valid.begin() + last + 1, valid.end() - 1, valid.end());
                    std::copy_backward(buffer.begin() + cols * (last + 1), buffer.end() - cols, buffer.end());
                    std::copy_backward(colors_bg.begin() + cols * (last + 1), colors_bg.end() - cols, colors_bg.end());
                    std::copy_backward(colors_fg.begin() + cols * (last + 1), colors_fg.end() - cols, colors_fg.end());
                    std::fill(buffer.begin() + cols * (last + 1), buffer.begin() + cols * (last + 2), 0);
                    std::fill(colors_bg.begin() + cols * (last + 1), colors_bg.begin() + cols * (last + 2), color_bg);
                    std::fill(colors_fg.begin() + cols * (last + 1), colors_fg.begin() + cols * (last + 2), color_fg);
                    if (last != ptr_y) {
                        valid[ptr_y] = ptr_x;
                        valid[last + 1] = valid[last] - ptr_x;
                        valid[last] = cols;
                    }
                    else {
                        valid[last + 1] = valid[last] - ptr_x;
                        valid[ptr_y] = ptr_x;
                    }
                    std::copy_backward(buffer.begin() + cols * ptr_y + ptr_x, buffer.begin() + cols * end_y + end_x, buffer.begin() + cols * (last + 1) + valid[last + 1]);
                    std::copy_backward(colors_bg.begin() + cols * ptr_y + ptr_x, colors_bg.begin() + cols * end_y + end_x, colors_bg.begin() + cols * (last + 1) + valid[last + 1]);
                    std::copy_backward(colors_fg.begin() + cols * ptr_y + ptr_x, colors_fg.begin() + cols * end_y + end_x, colors_fg.begin() + cols * (last + 1) + valid[last + 1]);
                    std::fill(buffer.begin() + cols * ptr_y + ptr_x, buffer.begin() + cols * ptr_y + ptr_x + len, 0);
                    std::fill(colors_bg.begin() + cols * ptr_y + ptr_x, colors_bg.begin() + cols * ptr_y + ptr_x + len, color_bg);
                    std::fill(colors_fg.begin() + cols * ptr_y + ptr_x, colors_fg.begin() + cols * ptr_y + ptr_x + len, color_fg);
                    ptr_y++;
                    ptr_x = 0;
                }
                ptr_ry = line;
                ptr_rx = valid[line];
            }
        }
        else {
            valid.back() = ptr_x;
            valid.push_back(0);
            ptr_x = 0;
            ptr_y++;
            line++;
            view = max(view, line - rows + 1);
            buffer.resize(buffer.size() + cols);
            colors_bg.resize(colors_bg.size() + cols);
            colors_fg.resize(colors_fg.size() + cols);
            std::fill(buffer.begin() + cols * line, buffer.end(), 0);
            std::fill(colors_bg.begin() + cols * line, colors_bg.end(), color_bg);
            std::fill(colors_fg.begin() + cols * line, colors_fg.end(), color_fg);
        }
    }

    void cjsgui::draw_char(const char& c) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& size = scr.size;

        buffer[ptr_y * cols + ptr_x] = c;
        colors_bg[ptr_y * cols + ptr_x] = color_bg;
        colors_fg[ptr_y * cols + ptr_x] = color_fg;
    }

    void cjsgui::error(const std::string& str) {
        throw cjs_exception(str);
    }

    void cjsgui::set_cycle(int cycle) {
        if (cycle == 0) {
            cycle_set = false;
            this->cycle = GUI_CYCLES;
        }
        else {
            cycle_set = true;
            this->cycle = cycle;
        }
    }

    void cjsgui::set_ticks(int ticks) {
        this->ticks = ticks;
    }

    void cjsgui::move(bool left) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& valid = scr.valid;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& input_state = scr.input_state;

        if (!input_state)
            return;
        if (left) {
            if (ptr_mx + ptr_my * cols < ptr_x + ptr_y * cols) {
                forward(ptr_x, ptr_y, false);
                if (ptr_x == cols)
                    ptr_x--;
            }
        }
        else {
            if (ptr_x + ptr_y * cols < ptr_rx + ptr_ry * cols) {
                if (ptr_x < valid[ptr_y])
                    forward(ptr_x, ptr_y, true);
            }
        }
    }

    void cjsgui::forward(int& x, int& y, bool forward) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& valid = scr.valid;

        if (forward) {
            if (x == cols - 1) {
                x = 0;
                y++;
            }
            else {
                x++;
            }
        }
        else {
            if (y == 0) {
                if (x != 0) {
                    x--;
                }
            }
            else {
                if (x != 0) {
                    x--;
                }
                else {
                    y--;
                    x = valid[y];
                }
            }
        }
    }

    std::vector<char> cjsgui::input_buffer() const {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& valid = scr.valid;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;

        std::vector<char> v;
        if (ptr_my == ptr_ry) {
            for (auto i = ptr_mx; i < ptr_ry; ++i) {
                v.push_back(buffer[cols * ptr_my + i]);
            }
            return v;
        }
        for (auto i = ptr_mx; i < valid[ptr_my]; ++i) {
            v.push_back(buffer[cols * ptr_my + i]);
        }
        v.push_back('\n');
        for (auto i = ptr_my + 1; i < ptr_ry; i++) {
            for (auto j = 0; j < valid[i]; ++j) {
                v.push_back(buffer[cols * i + j]);
            }
            v.push_back('\n');
        }
        for (auto i = 0; i < ptr_rx; ++i) {
            v.push_back(buffer[cols * ptr_ry + i]);
        }
        return v;
    }

    bool cjsgui::init_screen(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return false;
        if (screens[n])
            return false;
        screens[n] = std::make_unique<screen_t>();
        auto& scr = *screens[n].get();
        auto& input = scr.input;
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& color_bg_stack = scr.color_bg_stack;
        auto& color_fg_stack = scr.color_fg_stack;
        auto& size = scr.size;
        input.id = n;
        screen_ptr = 0;
        put_char('\f');
        screen_ptr = -1;
        if (vm) {
            CString s;
            s.Format(L"初始化屏幕（%d）", n);
            add_stat(s);
        }
        return true;
    }

    bool cjsgui::switch_screen_display(int n)
    {
        if (screen_id == n)
            return true;
        if (n < 0 || n >= (int)screens.size())
            return false;
        if (!screens[n])
            return false;
        screen_id = n;
        global_state.input_s = &screens[n]->input;
        return true;
    }

    int cjsgui::current_screen() const
    {
        return screen_id;
    }

    void cjsgui::screen_ref_add(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return;
        if (!screens[n])
            return;
        screen_ref[n]++;
    }

    void cjsgui::screen_ref_dec(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return;
        if (!screens[n])
            return;
        screen_ref[n]--;
        if (n != 0 && screen_ref[n] <= 0) {
            if (screen_id == n) {
                switch_screen_display(0);
            }
            screens[n].reset(nullptr);
            if (vm) {
                CString s;
                s.Format(L"关闭屏幕（%d）", n);
                add_stat(s);
            }
        }
    }

    cjsgui::global_state_t& cjsgui::get_global()
    {
        return global_state;
    }

    bool cjsgui::switch_screen(int n)
    {
        if (screen_ptr == n)
            return true;
        if (n < 0 || n >= (int)screens.size())
            return false;
        if (!screens[n])
            return false;
        screen_ptr = n;
        global_state.input = &screens[n]->input;
        return true;
    }

    void cjsgui::resize(int r, int c) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& size = scr.size;

        if (r == 0 && c == 0) {
            r = GUI_ROWS;
            c = GUI_COLS;
        }
        auto old_rows = rows;
        auto old_cols = cols;
        rows = max(10, min(r, 60));
        cols = max(20, min(c, 200));
        ATLTRACE("[SYSTEM] GUI  | Resize: from (%d, %d) to (%d, %d)\n", old_rows, old_cols, rows, cols);
        // ...
        ptr_x = min(ptr_x, cols);
        ptr_y = min(ptr_y, rows);
        ptr_mx = min(ptr_mx, cols);
        ptr_my = min(ptr_my, rows);
        ptr_rx = min(ptr_rx, cols);
        ptr_ry = min(ptr_ry, rows);
    }

    CSize cjsgui::get_size() const {
        auto& scr = *screens[screen_ptr].get();
        return { scr.cols * GUI_FONT_W, scr.rows * GUI_FONT_H };
    }

    CString cjsgui::get_disp(types::disp_t t) const
    {
        static TCHAR sz[256];
        std::wstringstream ss;
        switch (t) {
        case D_PS:
            break;
        case D_HTOP:
            break;
        case D_HANDLE: {
            CString s;
            const auto& scr = *screens[screen_id].get();
            s.AppendFormat(L"Screen: %d, Rows: %d, Cols: %d, View: %d, Line: %d, Scroll: %02x\n", screen_id, scr.rows, scr.cols, scr.view, scr.line, scr.scroll_fade);
            s.AppendFormat(L"Ptr: (%d, %d), Left: (%d, %d), Right: (%d, %d)\n", scr.ptr_y, scr.ptr_x, scr.ptr_my, scr.ptr_mx, scr.ptr_ry, scr.ptr_rx);
            for (auto i = max(scr.ptr_y - 2, 0); i <= min(scr.ptr_y + 2, scr.line); i++) {
                s.AppendFormat(L"LINE #%d | V= %2d, ", i, scr.valid[i]);
                for (auto j = 0; j < scr.cols; j++) {
                    if (scr.buffer[i * scr.cols + j])
                        s.AppendFormat(L"%02X", (BYTE)scr.buffer[i * scr.cols + j]);
                    else
                        s.Append(L"  ");
                }
                s.Append(L"\n");
            }
            return s;
        }
            break;
        case D_WINDOW:
            break;
        case D_MEM:
            break;
        case D_STAT: {
            auto& stat = *const_cast<std::list<std::tuple<CString, int>>*>(&stat_s);
            if (!stat_s.empty()) {
                ss << (wchar_t)(L'0' + stat_s.size());
                for (auto i = stat.begin(); i != stat.end();) {
                    ss << std::get<0>(*i).GetBuffer(0) << std::endl;
                    if (std::get<1>(*i) <= 1) {
                        i = stat.erase(i);
                    }
                    else {
                        std::get<1>(*i) = std::get<1>(*i) - 1;
                        i++;
                    }
                }
            }
        }
                   break;
        default:
            break;
        }
        return CString(ss.str().c_str());
    }

    void cjsgui::add_stat(const CString& s, bool show)
    {
        if (show) {
            if (stat_s.size() >= STAT_MAX_N)
                stat_s.pop_front();
            stat_s.push_back({ s, STAT_DELAY_N });
        }
#if REPORT_STAT
        {
            std::ofstream log(REPORT_STAT_FILE, std::ios::app | std::ios::out);
            log << CStringA(s).GetBuffer(0) << std::endl;
        }
#endif
    }

    int cjsgui::new_screen(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return 1;
        if (screens[n])
            return 2;
        init_screen(n);
        return 0;
    }

    int cjsgui::get_frame() const
    {
        if (vm)
            return vm->get_frame();
        return 0;
    }

    void cjsgui::clear_frame()
    {
        if (vm)
            vm->clear_frame();
    }

    void cjsgui::trigger_render()
    {
        if (!global_state.drawing)
            global_state.drawing = true;
    }

    int cjsgui::play_music(const std::string& title, const std::string& ext, const std::vector<char>& data)
    {
        auto& zplay = global_state.zplay;
        if (zplay) {
            stop_music();
            //return 1; // already playing
        }
        zplay = libZPlay::CreateZPlay();
        auto type = libZPlay::sfAutodetect;
        if (ext == "mp3") {
            type = libZPlay::sfMp3;
        }
        global_state.zplay_data = data;
        auto result = zplay->OpenStream(0, 0, global_state.zplay_data.data(), global_state.zplay_data.size(), type);
        if (result == 0) {
            global_state.zplay_data.clear();
            CString str;
            str.Format(L"MUSIC Play Error：'%S'", zplay->GetError());
            cjsgui::singleton().add_stat(str);
            zplay->Release();
            zplay = nullptr;
            return 2;
        }
        global_state.zplay_title = title;
        zplay->Play();
        return 0;
    }

    int cjsgui::stop_music()
    {
        auto& zplay = global_state.zplay;
        if (!zplay) {
            return 1; // none
        }
        global_state.zplay->Stop();
        global_state.zplay->Release();
        global_state.zplay = nullptr;
        global_state.zplay_data.clear();
        global_state.zplay_title.clear();
        return 0;
    }

    int cjsgui::compile(const std::string& path, const std::vector<std::string>& args, const std::vector<std::string>& paths) {
        if (path.empty())
            return -1;
        std::stringstream ss;
        ss << "sys.exec_file(\"" << path << "\", [";
        for (size_t i = 0; i < args.size(); i++) {
            ss << args[i];
            if (i + 1 != args.size()) {
                ss << ", ";
            }
        }
        ss << "]);";
        vm->exec("<entry>", ss.str());
        return 0;
    }

    void cjsgui::input_set(bool valid) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& size = scr.size;

        if (valid) {
            input_state = true;
            ptr_mx = ptr_x;
            ptr_my = ptr_y;
            ptr_rx = ptr_x;
            ptr_ry = ptr_y;
        }
        else {
            input_state = false;
            ptr_mx = -1;
            ptr_my = -1;
            ptr_rx = -1;
            ptr_ry = -1;
        }
        input_ticks = 0;
        input_caret = false;
    }

    const wchar_t* mapVirtKey[] = {
        L"VK_RESERVED",
        L"VK_LBUTTON",
        L"VK_RBUTTON",
        L"VK_CANCEL",
        L"VK_MBUTTON",
        L"VK_XBUTTON1",
        L"VK_XBUTTON2",
        L"VK_RESERVED",
        L"VK_BACK",
        L"VK_TAB",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_CLEAR",
        L"VK_RETURN",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_SHIFT",
        L"VK_CONTROL",
        L"VK_MENU",
        L"VK_PAUSE",
        L"VK_CAPITAL",
        L"VK_HANGUL",
        L"VK_RESERVED",
        L"VK_JUNJA",
        L"VK_FINAL",
        L"VK_KANJI",
        L"VK_RESERVED",
        L"VK_ESCAPE",
        L"VK_CONVERT",
        L"VK_NONCONVERT",
        L"VK_ACCEPT",
        L"VK_MODECHANGE",
        L"VK_SPACE",
        L"VK_PRIOR",
        L"VK_NEXT",
        L"VK_END",
        L"VK_HOME",
        L"VK_LEFT",
        L"VK_UP",
        L"VK_RIGHT",
        L"VK_DOWN",
        L"VK_SELECT",
        L"VK_PRINT",
        L"VK_EXECUTE",
        L"VK_SNAPSHOT",
        L"VK_INSERT",
        L"VK_DELETE",
        L"VK_HELP",
        L"VK_0",
        L"VK_1",
        L"VK_2",
        L"VK_3",
        L"VK_4",
        L"VK_5",
        L"VK_6",
        L"VK_7",
        L"VK_8",
        L"VK_9",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_A",
        L"VK_B",
        L"VK_C",
        L"VK_D",
        L"VK_E",
        L"VK_F",
        L"VK_G",
        L"VK_H",
        L"VK_I",
        L"VK_J",
        L"VK_K",
        L"VK_L",
        L"VK_M",
        L"VK_N",
        L"VK_O",
        L"VK_P",
        L"VK_Q",
        L"VK_R",
        L"VK_S",
        L"VK_T",
        L"VK_U",
        L"VK_V",
        L"VK_W",
        L"VK_X",
        L"VK_Y",
        L"VK_Z",
        L"VK_LWIN",
        L"VK_RWIN",
        L"VK_APPS",
        L"VK_RESERVED",
        L"VK_SLEEP",
        L"VK_NUMPAD0",
        L"VK_NUMPAD1",
        L"VK_NUMPAD2",
        L"VK_NUMPAD3",
        L"VK_NUMPAD4",
        L"VK_NUMPAD5",
        L"VK_NUMPAD6",
        L"VK_NUMPAD7",
        L"VK_NUMPAD8",
        L"VK_NUMPAD9",
        L"VK_MULTIPLY",
        L"VK_ADD",
        L"VK_SEPARATOR",
        L"VK_SUBTRACT",
        L"VK_DECIMAL",
        L"VK_DIVIDE",
        L"VK_F1",
        L"VK_F2",
        L"VK_F3",
        L"VK_F4",
        L"VK_F5",
        L"VK_F6",
        L"VK_F7",
        L"VK_F8",
        L"VK_F9",
        L"VK_F10",
        L"VK_F11",
        L"VK_F12",
        L"VK_F13",
        L"VK_F14",
        L"VK_F15",
        L"VK_F16",
        L"VK_F17",
        L"VK_F18",
        L"VK_F19",
        L"VK_F20",
        L"VK_F21",
        L"VK_F22",
        L"VK_F23",
        L"VK_F24",
        L"VK_NAVIGATION_VIEW",
        L"VK_NAVIGATION_MENU",
        L"VK_NAVIGATION_UP",
        L"VK_NAVIGATION_DOWN",
        L"VK_NAVIGATION_LEFT",
        L"VK_NAVIGATION_RIGHT",
        L"VK_NAVIGATION_ACCEPT",
        L"VK_NAVIGATION_CANCEL",
        L"VK_NUMLOCK",
        L"VK_SCROLL",
        L"VK_OEM_FJ_JISHO",
        L"VK_OEM_FJ_MASSHOU",
        L"VK_OEM_FJ_TOUROKU",
        L"VK_OEM_FJ_LOYA",
        L"VK_OEM_FJ_ROYA",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_LSHIFT",
        L"VK_RSHIFT",
        L"VK_LCONTROL",
        L"VK_RCONTROL",
        L"VK_LMENU",
        L"VK_RMENU",
        L"VK_BROWSER_BACK",
        L"VK_BROWSER_FORWARD",
        L"VK_BROWSER_REFRESH",
        L"VK_BROWSER_STOP",
        L"VK_BROWSER_SEARCH",
        L"VK_BROWSER_FAVORITES",
        L"VK_BROWSER_HOME",
        L"VK_VOLUME_MUTE",
        L"VK_VOLUME_DOWN",
        L"VK_VOLUME_UP",
        L"VK_MEDIA_NEXT_TRACK",
        L"VK_MEDIA_PREV_TRACK",
        L"VK_MEDIA_STOP",
        L"VK_MEDIA_PLAY_PAUSE",
        L"VK_LAUNCH_MAIL",
        L"VK_LAUNCH_MEDIA_SELECT",
        L"VK_LAUNCH_APP1",
        L"VK_LAUNCH_APP2",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_OEM_1 // ';:' for US",
        L"VK_OEM_PLUS // '+' any country",
        L"VK_OEM_COMMA // ',' any country",
        L"VK_OEM_MINUS // '-' any country",
        L"VK_OEM_PERIOD // '.' any country",
        L"VK_OEM_2 // '/?' for US",
        L"VK_OEM_3 // '`~' for US",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_GAMEPAD_A",
        L"VK_GAMEPAD_B",
        L"VK_GAMEPAD_X",
        L"VK_GAMEPAD_Y",
        L"VK_GAMEPAD_RIGHT_SHOULDER",
        L"VK_GAMEPAD_LEFT_SHOULDER",
        L"VK_GAMEPAD_LEFT_TRIGGER",
        L"VK_GAMEPAD_RIGHT_TRIGGER",
        L"VK_GAMEPAD_DPAD_UP",
        L"VK_GAMEPAD_DPAD_DOWN",
        L"VK_GAMEPAD_DPAD_LEFT",
        L"VK_GAMEPAD_DPAD_RIGHT",
        L"VK_GAMEPAD_MENU",
        L"VK_GAMEPAD_VIEW",
        L"VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON",
        L"VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON",
        L"VK_GAMEPAD_LEFT_THUMBSTICK_UP",
        L"VK_GAMEPAD_LEFT_THUMBSTICK_DOWN",
        L"VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT",
        L"VK_GAMEPAD_LEFT_THUMBSTICK_LEFT",
        L"VK_GAMEPAD_RIGHT_THUMBSTICK_UP",
        L"VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN",
        L"VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT",
        L"VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT",
        L"VK_OEM_4",
        L"VK_OEM_5",
        L"VK_OEM_6",
        L"VK_OEM_7",
        L"VK_OEM_8",
        L"VK_RESERVED",
        L"VK_OEM_AX",
        L"VK_OEM_102",
        L"VK_ICO_HELP",
        L"VK_ICO_00",
        L"VK_PROCESSKEY",
        L"VK_ICO_CLEAR",
        L"VK_PACKET",
        L"VK_RESERVED",
        L"VK_OEM_RESET",
        L"VK_OEM_JUMP",
        L"VK_OEM_PA1",
        L"VK_OEM_PA2",
        L"VK_OEM_PA3",
        L"VK_OEM_WSCTRL",
        L"VK_OEM_CUSEL",
        L"VK_OEM_ATTN",
        L"VK_OEM_FINISH",
        L"VK_OEM_COPY",
        L"VK_OEM_AUTO",
        L"VK_OEM_ENLW",
        L"VK_OEM_BACKTAB",
        L"VK_ATTN",
        L"VK_CRSEL",
        L"VK_EXSEL",
        L"VK_EREOF",
        L"VK_PLAY",
        L"VK_ZOOM",
        L"VK_NONAME",
        L"VK_PA1",
        L"VK_OEM_CLEAR",
        L"VK_RESERVED",
    };

    static bool js_key_pressing(int code)
    {
        return (GetKeyState((int)code) & 0xF0) != 0;
    }

    void cjsgui::input(int c) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& view = scr.view;
        auto& line = scr.line;
        auto& valid = scr.valid;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& cmd_state = scr.cmd_state;
        auto& size = scr.size;

        {
            CString str;
            if (c & GUI_SPECIAL_MASK) {
                auto cc = c & 0xffff;
                cc = min(cc, 255);
                str.Format(L"屏幕（%d）键盘输入：%d 0x%x %s", screen_ptr, cc, cc, mapVirtKey[cc]);
                if (c & GUI_SPECIAL_MASK) {
                    auto cc = (c & 0xff) - VK_F1;
                    if (cc >= 0 && cc < (int)screens.size()) {
                        if (switch_screen_display(cc)) {
                            str.AppendFormat(L"，切换到屏幕（%d）成功", cc);
                        }
                        else {
                            str.AppendFormat(L"，切换到屏幕（%d）失败", cc);
                        }
                    }
                }
            }
            else
                str.Format(L"屏幕（%d）键盘输入：%d 0x%x %c", screen_ptr, c, c, isprint(c) ? wchar_t(c) : L'?');
            add_stat(str);
        }
        if (!input_state) {
            if (c & GUI_SPECIAL_MASK) {
                switch (c & 0xff) {
                case VK_UP:
                    view = max(0, view - 1);
                    break;
                case VK_DOWN:
                    view = min(max(line - rows, 0), view + 1);
                    break;
                case VK_HOME:
                    view = 0;
                    break;
                case VK_END:
                    view = max(0, line - rows + 1);
                    break;
                case VK_NEXT: // page down
                    view = min(line - rows + 1, view + rows);
                    break;
                case VK_PRIOR: // page up
                    view = max(0, view - rows);
                    break;
                }
            }
            return;
        }
        if (global_state.input_s->input_single) {
            if (c > 0 && c < 256 && (std::isprint(c) || c == '\r')) {
                if (c == '\r')
                    c = '\n';
                put_char(c);
                ptr_x = ptr_rx;
                ptr_y = ptr_ry;
                global_state.input_s->input_content.push_back(c);
                global_state.input_s->input_read_ptr = 0;
                global_state.input_s->input_success = true;
                global_state.input_s->input_code = 0;
                global_state.input_s->input_single = false;
                input_state = false;
            }
            else {
#if LOG_VM
                {
                    CStringA s; s.Format("[SYSTEM] GUI  | Input invalid single key: %d\n", c);
                    global_state.log_err.push_back(s.GetBuffer(0));
                    cvm::logging(CString(s));
                }
#endif
            }
            return;
        }
        if (c < 0) {
            put_char(c);
            return;
        }
        if (c < 0xffff && c > 0xff) {
            CString s;
            s.AppendChar(c);
            CStringA s2(s);
            put_char(s2[0]);
            put_char(s2[1]);
            return;
        }
        if (!((c & GUI_SPECIAL_MASK) || std::isprint(c) || c == '\b' || c == '\n' || c == '\r' || c == 4 || c == 7 || c == 26 || c == 22)) {
#if LOG_VM
            CStringA s; s.Format("[SYSTEM] GUI  | Input: %d\n", (int)c);
            global_state.log_info.push_back(s.GetBuffer(0));
#endif
            ATLTRACE("[SYSTEM] GUI  | Input: %d\n", (int)c);
            return;
        }
        if (c == '\b') {
            put_char('\b');
            return;
        }
        if (c & GUI_SPECIAL_MASK) {
            char C = (char)-9;
            switch (c & 0xff) {
            case VK_LEFT:
                // C = (char) -12;
                // break;
                move(true);
                return;
            case VK_UP:
            {
                if (!input_state || js_key_pressing(VK_CONTROL)) {
                    view = max(0, view - 1);
                    return;
                }
                else {
                    //C = (char)-10;
                    if (cols * ptr_my + ptr_mx <= cols * (ptr_y - 1) + ptr_x) {
                        ptr_y--;
                        ptr_x = min(ptr_x, valid[ptr_y]);
                        view = min(view, ptr_y);
                    }
                    else if (cols* ptr_my + ptr_mx <= cols * (ptr_y - 1) + cols - 1) {
                        ptr_y--;
                        ptr_x = ptr_mx;
                        view = max(view, ptr_y);
                    }
                    return;
                }
            }
                break;
            case VK_RIGHT:
                // C = (char) -13;
                // break;
                if (input_state) {
                    if (ptr_x == valid[ptr_y] && ptr_ry > ptr_y) {
                        ptr_x = 0;
                        ptr_y++;
                    }
                    else {
                        move(false);
                    }
                }
                else
                    move(false);
                return;
            case VK_DOWN:
            {
                if (!input_state || js_key_pressing(VK_CONTROL)) {
                    view = min(max(line - rows, 0), view + 1);
                    return;
                }
                else {
                    //C = (char)-11;
                    if (cols * ptr_ry + ptr_rx >= cols * (ptr_y + 1) + ptr_x) {
                        ptr_y++;
                        ptr_x = min(ptr_x, valid[ptr_y]);
                        view = max(view, ptr_y - rows);
                    }
                    else if (cols * ptr_ry + ptr_rx >= cols * (ptr_y + 1)) {
                        ptr_y++;
                        ptr_x = ptr_rx;
                        view = max(view, ptr_y - rows);
                    }
                    return;
                }
            }
                break;
            case VK_HOME:
            {
                if (!input_state || js_key_pressing(VK_CONTROL)) {
                    view = 0;
                    return;
                }
                else if (js_key_pressing(VK_MENU)) {
                    ptr_x = ptr_mx;
                    ptr_y = ptr_my;
                }
                else {
                    if (ptr_y > ptr_my) {
                        ptr_x = 0;
                    }
                    else {
                        ptr_x = ptr_mx;
                    }
                }
            }
                return;
            case VK_END:
            {
                if (!input_state || js_key_pressing(VK_CONTROL)) {
                    view = max(0, line - rows + 1);
                    return;
                }
                else if (js_key_pressing(VK_MENU)) {
                    ptr_x = ptr_rx;
                    ptr_y = ptr_ry;
                }
                else {
                    if (ptr_y < ptr_ry) {
                        ptr_x = valid[ptr_y];
                        if (ptr_x == cols)
                            ptr_x--;
                    }
                    else {
                        ptr_x = ptr_rx;
                    }
                }
            }
                return;
            case VK_DELETE: {
                if (input_state &&
                    (ptr_x != line || ptr_y != cols - 1) &&
                    ptr_mx + ptr_my * cols <= ptr_x + ptr_y * cols &&
                    ptr_x + ptr_y * cols < ptr_rx + ptr_ry * cols) {
                    auto cc = buffer[ptr_y * cols + ptr_x];
                    auto x = ptr_x;
                    auto y = ptr_y;
                    auto k = 1;
                    if (cc < 0 && ptr_x + ptr_y * cols + 1 <= ptr_rx + ptr_ry * cols) {
                        WORD wd = (((BYTE)cc) << 8) | ((BYTE)buffer[ptr_y * cols + ptr_x + 1]);
                        if (wd >= 0x8140 && wd <= 0xFEFE) { // GBK
                            k++;
                        }
                    }
                    for (auto i = 0; i < k; i++) {
                        if (ptr_x == valid[ptr_y]) {
                            ptr_x = 0;
                            ptr_y++;
                        }
                        else {
                            ptr_x++;
                        }
                        assert(ptr_x + ptr_y * cols <= ptr_rx + ptr_ry * cols);
                    }
                    put_char('\b');
                }
            }
                return;
            case VK_ESCAPE:
                return;
            case VK_SPACE:
                return;
            case VK_BACK:
                return;
            case VK_NEXT: // page down
                view = min(line - rows + 1, view + rows);
                return;
            case VK_PRIOR: // page up
                view = max(0, view - rows);
                return;
            case VK_RETURN:
                if (js_key_pressing(VK_CONTROL)) {
                    if (!global_state.input_s->input_success) {
                        ptr_x = ptr_rx;
                        ptr_y = ptr_ry;
                        put_char('\n');
                        if (global_state.input_s->input_content.empty())
                            global_state.input_s->input_content = input_buffer();
                        global_state.input_s->input_read_ptr = 0;
                        global_state.input_s->input_success = true;
                        global_state.input_s->input_code = 0;
                        input_state = false;
                    }
                    return;
                }
                else {
                    input('\n');
                    return;
                }
            /*case 0x71: // SHIFT
                return;
            case 0x72: // CTRL
                return;
            case 0x74: // ALT
                return;*/
            case VK_SHIFT: // SHIFT
                return;
            case VK_CONTROL: // CTRL
                return;
            case VK_MENU: // ALT
                return;
            default:
#if LOG_VM
            {
                CStringA s; s.Format("[SYSTEM] GUI  | Input invalid special key: %d\n", c & 0xff);
                global_state.log_err.push_back(s.GetBuffer(0));
                cvm::logging(CString(s));
            }
#endif
            ATLTRACE("[SYSTEM] GUI  | Input invalid special key: %d\n", c & 0xff);
            return;
            }
            global_state.input_s->input_content = input_buffer();
            global_state.input_s->input_read_ptr = 0;
            global_state.input_s->input_success = true;
            global_state.input_s->input_code = C;
            input_state = false;
            auto begin = ptr_mx + ptr_my * cols;
            auto end = ptr_x + ptr_y * cols;
            for (int i = begin; i <= end; ++i) {
                buffer[i] = 0;
                colors_bg[i] = color_bg;
                colors_fg[i] = color_fg;
            }
            ptr_x = ptr_mx;
            ptr_y = ptr_my;
            ptr_rx = ptr_mx;
            ptr_ry = ptr_my;
        }
        else if (c == 22) { // Ctrl+V
            OpenClipboard(window->GetWindowHandle());
            if (IsClipboardFormatAvailable(CF_TEXT))
            {
                HGLOBAL hg = GetClipboardData(CF_TEXT);
                if (hg) {
                    LPCSTR q = (LPCSTR)GlobalLock(hg);
                    if (q != NULL)
                    {
                        CStringA A(q);
                        for (auto i = 0; i < A.GetLength(); i++) {
                            put_char(A[i]);
                        }
                    }
                    GlobalUnlock(hg);
                }
            }
            CloseClipboard();
        }
        else {
            put_char((char)(c & 0xff));
        }
    }

    int cjsgui::reset_cycles() {
        auto c = cycles;
        cycles = 0;
        return c;
    }

    void cjsgui::hit(int n)
    {
        if (vm)
            vm->hit(n);
    }

    int cjsgui::cursor() const
    {
        if (vm)
            return vm->cursor();
        return 1;
    }

    void cjsgui::output() const
    {
        auto& scr = *screens[screen_id].get();
        auto& rows = scr.rows;
        auto& cols = scr.cols;
        auto& line = scr.line;
        auto& valid = scr.valid;
        auto& buffer = scr.buffer;

        if (!vm)
            return;
        OpenClipboard(window->GetWindowHandle());
        std::vector<char> outs;
        for (auto i = 0; i <= line; ++i) {
            auto end = min(valid[i], cols - 1);
            for (auto j = 0; j <= end; ++j) {
                const auto& b = buffer[i * cols + j];
                if (b == 0) {
                    break;
                }
                if (b > 0) {
                    if (std::isprint(b))
                        outs.push_back(b);
                    else
                        outs.push_back(' ');
                }
                else {
                    outs.push_back(b);
                }
            }
            outs.push_back('\r');
            outs.push_back('\n');
        }
        while (outs.size() > 2) {
            if (outs.back() == '\n' && *(outs.rbegin() + 1) == '\r') {
                outs.pop_back();
                outs.pop_back();
            }
            else {
                break;
            }
        }
        outs.push_back('\0');
        HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, outs.size());
        if (hg) {
            auto data = (char*)GlobalLock(hg);
            if (data) {
                CopyMemory(data, outs.data(), outs.size());
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hg);
            }
            GlobalUnlock(hg);
        }
        CloseClipboard();
    }

    void cjsgui::exec_cmd(const std::string& s) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& color_bg_stack = scr.color_bg_stack;
        auto& color_fg_stack = scr.color_fg_stack;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;

        switch (s[0]) {
        case 'B': { // 设置背景色
            color_bg = (uint32_t)std::stoul(s.substr(1), nullptr, 16);
        }
                  break;
        case 'F': { // 设置前景色
            color_fg = (uint32_t)std::stoul(s.substr(1), nullptr, 16);
        }
                  break;
        case 'S': { // 设置
            static int cfg;
            cfg = (uint32_t)std::stoul(s.substr(1), nullptr, 10);
            switch (cfg) {
            case 1: // 保存背景色
                color_bg_stack.push_back(color_bg);
                break;
            case 2: // 保存前景色
                color_fg_stack.push_back(color_fg);
                break;
            case 3: // 恢复背景色
                color_bg = color_bg_stack.back();
                if (color_bg_stack.size() > 1) color_bg_stack.pop_back();
                break;
            case 4: // 恢复前景色
                color_fg = color_fg_stack.back();
                if (color_fg_stack.size() > 1) color_fg_stack.pop_back();
                break;
            default:
                break;
            }
        }
                  break;
        default:
            break;
        }
    }
}
