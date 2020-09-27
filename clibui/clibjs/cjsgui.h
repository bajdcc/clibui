﻿//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CGUI_H
#define CLIBJS_CGUI_H

#include <d2d1.h>
#include <array>
#include <deque>
#include <regex>
#include <ui\window\window.h>
#include <ui\gdi\Gdi.h>
#include <clibjs\JS2D.h>
#include <clibjs\cjstypes.h>
#include <clibjs\cjs.h>
#include <libzplay\libzplay.h>

#define GLOBAL_STATE clib::cjsgui::singleton().get_global()

#define GUI_FONT GLUT_BITMAP_9_BY_15
#define GUI_FONT_W 9
#define GUI_FONT_W_C1 1
#define GUI_FONT_W_C2 (GUI_FONT_W * 2 + 1)
#define GUI_FONT_H 17
#define GUI_FONT_H_1 0
#define GUI_FONT_H_2 17
#define GUI_FONT_H_C1 -2
#define GUI_FONT_H_C2 15
#define GUI_ROWS 30
#define GUI_COLS 84
#define GUI_SIZE (GUI_ROWS * GUI_COLS)
#define GUI_CYCLES 1000
#define GUI_MAX_SPEED 4
#define GUI_TICKS 16
#define GUI_MAX_CYCLE_N 10000
#define GUI_MAX_CYCLE (GUI_MAX_CYCLE_N / GUI_TICKS)
#define GUI_MIN_CYCLE 10
#define GUI_MIN_FPS_RATE 0.6
#define GUI_MAX_FPS_RATE 0.9
#define GUI_CYCLE_STABLE 100
#define GUI_INPUT_CARET 15
#define GUI_SPECIAL_MASK 0x20000
#define GUI_SCREEN_N 4
#define GUI_CACHE_OBJ_MIN 5

namespace clib {

    class cjsgui {
    public:
        cjsgui();
        ~cjsgui() = default;

        cjsgui(const cjsgui&) = delete;
        cjsgui& operator=(const cjsgui&) = delete;

        void draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, const JS2DEngine::BrushBag& brushes, bool paused, decimal fps);
        int compile(const std::string& path, const std::vector<std::string>& args, const std::vector<std::string>& paths);

        void put_string(const std::string& str);
        void put_char(int c);
        void input_call(int c);

        void set_cycle(int cycle);
        void set_ticks(int ticks);
        void resize(int rows, int cols);
        CSize get_size() const;
        void resize_ui();

        void input_set(bool valid);
        void input(int c);
        int reset_cycles();
        void hit(int n);
        void output() const;

        CString get_disp(types::disp_t) const;
        void add_stat(const CString& s, bool show = true);

        int new_screen(int n);

        int get_frame() const;
        void clear_frame();

        void trigger_render();

    public:
        int play_music(const std::string& title, const std::string& ext, const std::vector<char>& data);
        int stop_music();

    private:
        void reset_ips();
        void tick();
        void draw_text(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, const JS2DEngine::BrushBag& brushes, bool cached);
        void draw_window(const CRect& bounds, const JS2DEngine::BrushBag& brushes);

        void init_render_target();

        void new_line();
        inline void draw_char(const char& c);

        void exec_cmd(const std::string& s);

        static void error(const std::string&);
        void move(bool left);
        void forward(int& x, int& y, bool forward);
        std::vector<char> input_buffer() const;

        bool init_screen(int n);
        bool switch_screen_display(int n);

        void init_cubic_bezier();

    public:
        static cjsgui& singleton();

        bool switch_screen(int n);
        int current_screen() const;
        void screen_ref_add(int n);
        void screen_ref_dec(int n);

        void reset();
        void clear_cache();

        bool begin_render();
        void end_render();
        void change_target(std::shared_ptr<Direct2DRenderTarget> renderTarget);

    private:
        std::unique_ptr<clib::cjs> vm;
        class global_input_t {
        public:
            global_input_t() = default;
            std::vector<char> input_content;
            int id{ -1 };
            bool input_success{ false };
            int input_read_ptr{ -1 };
            int input_code{ 0 };
            bool input_single{ false };
        };
        struct cache_bitmap_t {
            using ref = std::shared_ptr<cache_bitmap_t>;
            CComPtr<ID2D1Bitmap> bitmap;
            CComPtr<ID2D1RenderTarget> renderTarget;
            CComPtr<ID2D1BitmapRenderTarget> renderTarget_bitmap;
        };
        struct cache_idx_t {
            int start, end, to;
        };
        struct global_state_t {
            global_input_t* input{ nullptr };
            global_input_t* input_s{ nullptr };
            std::string hostname{ "ccjs" };
            bool gui{ false };
            float gui_blur{ 0.0f };
            decltype(std::chrono::system_clock::now()) now;
#if LOG_VM
            std::vector<string_t> log_info, log_err;
#endif
            MouseInfo mouseInfo;
            KeyInfo keyInfo;
            int window_focus{ -1 };
            int window_hover{ -1 };
            bool is_logging{ false };
            bool reboot{ false };
            bool stop{ false };
            libZPlay::ZPlay* zplay{ nullptr };
            std::vector<char> zplay_data;
            std::string zplay_title;
            CRect bound;
            std::weak_ptr<Direct2DRenderTarget> canvas;
            CComPtr<ID2D1Bitmap> bitmap;
            CComPtr<ID2D1RenderTarget> renderTarget;
            CComPtr<ID2D1BitmapRenderTarget> renderTarget_bitmap;
            CComPtr<ID2D1Bitmap> bitmapConsole;
            CComPtr<ID2D1RenderTarget> renderTargetConsole;
            CComPtr<ID2D1BitmapRenderTarget> renderTarget_bitmapConsole;
            std::vector<cache_idx_t> cached_index;
            std::vector<cache_bitmap_t::ref> cached_bitmap;
            int total_obj{ 0 };
            int cache_obj{ 0 };
            bool drawing{ false };
            bool drawingConsole{ false };
            bool painting{ false };
            bool paintingConsole{ false };
            bool paintingScroll{ false };
            std::vector<jsv_ui::weak_ref> render_queue;
            std::vector<jsv_ui::weak_ref> render_queue_auto;
            std::vector<int> render_queue_level;
            std::vector<jsv_ui::weak_ref> render_queue_bk;
            std::vector<jsv_ui::weak_ref> render_queue_auto_bk;
            jsv_ui::weak_ref ui_focus;
            jsv_ui::weak_ref ui_hover;
            int cursor{ 0 };
            std::list<CStringA> stat_s;
        } global_state;
        class screen_t {
        public:
            screen_t() = default;
            int line{ 0 };
            int view{ 0 };
            std::vector<int> valid;
            std::vector<char> buffer;
            std::vector<uint32_t> colors_bg;
            std::vector<uint32_t> colors_fg;
            std::vector<uint32_t> color_bg_stack;
            std::vector<uint32_t> color_fg_stack;
            int ptr_x{ 0 };
            int ptr_y{ 0 };
            int ptr_mx{ -1 };
            int ptr_my{ -1 };
            int ptr_rx{ -1 };
            int ptr_ry{ -1 };
            int rows{ GUI_ROWS };
            int cols{ GUI_COLS };
            int size{ GUI_SIZE };
            bool input_state{ false };
            int input_ticks{ 0 };
            bool input_caret{ false };
            bool cmd_state{ false };
            std::vector<char> cmd_string;
            std::vector<char> input_delay;
            uint32_t color_bg{ 0 };
            uint32_t color_fg{ 0xffffff };
            int scroll_fade{ 0 };
            CRect scroll_rect;
            int old_line{ 0 };
            int old_view{ 0 };
            global_input_t input;
        };
        std::array<std::unique_ptr<screen_t>, GUI_SCREEN_N> screens;
        std::array<int, GUI_SCREEN_N> screen_ref;
        int screen_id{ -1 };
        int screen_ptr{ -1 };
        bool running{ false };
        bool exited{ false };
        int cycle{ GUI_CYCLES };
        int ticks{ GUI_TICKS };
        int cycles{ 0 };
        int cycle_speed{ 0 };
        int cycle_stable{ 0 };
        bool cycle_set{ false };
        bool entered{ false };
        std::chrono::time_point<std::chrono::system_clock> last_time{ std::chrono::system_clock::now() };
        struct easy_t {
            std::vector<float> easy_in;
            std::vector<float> easy_out;
        } animation;

    public:
        global_state_t& get_global();

    private:
        const std::string pat_path{ R"((/[A-Za-z0-9_]+)+)" };
        std::regex re_path{ pat_path };
        const std::string pat_bin{ R"([A-Za-z0-9_]+)" };
        std::regex re_bin{ pat_bin };
    };
}

#endif //CLIBLISP_CGUI_H
