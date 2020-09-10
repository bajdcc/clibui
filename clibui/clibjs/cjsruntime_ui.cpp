//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include "cjsruntime.h"
#include "cjsgui.h"
#include <ui\window\Window.h>

namespace clib {

    static LONG obj2num(const js_value::weak_ref& obj) {
        auto o = obj.lock();
        if (o->get_type() != r_number) {
            return 0;
        }
        auto num = JS_NUM(o);
        if (std::isinf(num) || std::isnan(num)) {
            return 0;
        }
        return (LONG)num;
    }

    static float obj2float(const js_value::weak_ref& obj) {
        auto o = obj.lock();
        if (o->get_type() != r_number) {
            return 0;
        }
        auto num = JS_NUM(o);
        if (std::isinf(num) || std::isnan(num)) {
            return 0;
        }
        return (float)num;
    }

    static std::string obj2str(const js_value::weak_ref& obj) {
        auto o = obj.lock();
        if (o->get_type() != r_string) {
            return "";
        }
        return JS_STR(o);
    }

    static void set_location(const js_ui_base::ref &u, const jsv_object::ref& _this, const jsv_object::ref& obj, js_value_new* n) {
        _this->add("left", obj->get("left", n));
        _this->add("top", obj->get("top", n));
        _this->add("width", obj->get("width", n));
        _this->add("height", obj->get("height", n));
    }

    int jsv_ui::get_object_type() const
    {
        return jsv_object::T_UI;
    }

    int jsv_ui::get_element_type() const
    {
        if (element) {
            return element->get_type();
        }
        return -1;
    }

    std::weak_ptr<js_ui_base> jsv_ui::get_element() const
    {
        return element;
    }

    void jsv_ui::add2(const std::string& s, const jsv_object::ref& obj, js_value_new* n)
    {
        add(s, obj->get(s, n));
    }

    bool jsv_ui::init(const jsv_object::ref& obj, js_value_new* n)
    {
        auto v = obj->get("type", n);
        if (v && v->get_type() == r_string) {
            do {
                jsv_object::add("type", v);
                jsv_object::add("length", n->new_number(0));
                auto type = JS_STR(v);
                if (type == "label") {
                    element = std::make_shared<js_ui_label>();
                    break;
                }
                if (type == "rect") {
                    element = std::make_shared<js_ui_rect>();
                    break;
                }
                if (type == "round") {
                    element = std::make_shared<js_ui_round>();
                    break;
                }
                if (type == "qr") {
                    element = std::make_shared<js_ui_qr>();
                    break;
                }
                if (type == "image") {
                    element = std::make_shared<js_ui_image>();
                    break;
                }
                if (type == "svg") {
                    element = std::make_shared<js_ui_svg>();
                    break;
                }
                if (type == "empty") {
                    element = std::make_shared<js_ui_empty>();
                    break;
                }
                if (type == "root") {
                    break;
                }
                return false;
            } while (0);
            auto keys = obj->get_keys();
            for (const auto& k : keys) {
                if (!exists(k)) {
                    add2(k, obj, n);
                }
            }
            return true;
        }
        return false;
    }

    void jsv_ui::render()
    {
        if (!render_state)
            render_state = true;
        if (element)
            element->render();
    }

    void jsv_ui::add(const std::string& s, const js_value::weak_ref& obj)
    {
        if (!obj.lock())
            return;
        if (s == "type")return;
        jsv_object::add(s, obj);
        if (s == "length") {
            cjsgui::singleton().trigger_render();
            return;
        }
        static std::regex num(R"(\d+)", std::regex::ECMAScript | std::regex::optimize);
        if (std::regex_match(s, num)) {
            cjsgui::singleton().trigger_render();
            return;
        }
        if (s == "cursor") {
            auto c = obj2str(obj);
            if (c == "hand") { cursor = Window::CursorType::hand; }
            else if (c == "ibeam") { cursor = Window::CursorType::ibeam; }
            else if (c == "size_left") { cursor = Window::CursorType::size_left; }
            else if (c == "size_top") { cursor = Window::CursorType::size_top; }
            else if (c == "size_topleft") { cursor = Window::CursorType::size_topleft; }
            else if (c == "size_topright") { cursor = Window::CursorType::size_topright; }
            else if (c == "wait") { cursor = Window::CursorType::wait; }
            else cursor = 0;
        }
        if (element) {
            if (s == "left")
                element->left = obj2num(obj);
            else if (s == "top")
                element->top = obj2num(obj);
            else if (s == "width")
                element->width = obj2num(obj);
            else if (s == "height")
                element->height = obj2num(obj);
            else {
                element->add(s, obj.lock());
                return;
            }
            cjsgui::singleton().trigger_render();
        }
    }

    void jsv_ui::remove(const std::string& s)
    {
        if (s == "type")return;
        jsv_object::remove(s);
        if (s == "cursor") {
            cursor = 0;
        }
        if (element) {
            if (s == "left")
                element->left = 0;
            else if (s == "top")
                element->top = 0;
            else if (s == "width")
                element->width = 0;
            else if (s == "height")
                element->height = 0;
            else {
                element->remove(s);
                return;
            }
            cjsgui::singleton().trigger_render();
        }

    }

    void jsv_ui::change_target()
    {
        if (element)
            element->change_target();
    }

    void jsv_ui::set_render(bool s)
    {
        render_state = s;
    }

    bool jsv_ui::is_render() const
    {
        return render_state;
    }

    bool jsv_ui::hit(int x, int y) const
    {
        if (element) {
            return element->hit(x, y);
        }
        return false;
    }

    bool jsv_ui::is_dynamic() const
    {
        if (element) {
            return element->is_dynamic();
        }
        return false;
    }

    int jsv_ui::get_cursor() const
    {
        return cursor;
    }

    void cjsruntime::send_signal(const std::string& s)
    {
        if (!global_ui.signals.empty() && global_ui.signals.back() == s)
            return;
        global_ui.signals.push_back(s);
    }

    void cjsruntime::change_target()
    {
        for (auto& s : global_ui.elements) {
            s->change_target();
        }
    }

    int cjsruntime::get_frame() const
    {
        return global_ui.frames;
    }

    void cjsruntime::clear_frame()
    {
        global_ui.frames = 0;
    }

    void cjsruntime::ui_hit(const js_value::ref& obj, const std::string& n)
    {
        global_ui.hit = obj;
        global_ui.hit_n = n;
        global_ui.signals.push_front("hit");
        eval_ui(true);
        global_ui.hit.reset();
        global_ui.hit_n.clear();
    }

    extern bool js_trans(const js_value::ref& obj);

    bool cjsruntime::hit(int n)
    {
        if (n == WE_MouseEntered || n == WE_GotFocus || n == WE_LostFocus)
            return false;
        if (GLOBAL_STATE.drawing && permanents.state != 0) {
            if (cjsgui::singleton().begin_render()) {
                for (auto& e : global_ui.elements) {
                    e->set_render(false);
                }
                auto result = eval_ui(false);
                cjsgui::singleton().end_render();
                if (result != 0)
                    return false;
            }
        }
        if (n == WE_MouseLeaved) {
            auto& hover = GLOBAL_STATE.ui_hover;
            if (hover.lock()) {
                ui_hit(hover.lock(), "mouseleave");
                GLOBAL_STATE.cursor = 0;
                hover.reset();
            }
            return true;
        }
        if (n == WE_MouseHover) {
            auto& hover = GLOBAL_STATE.ui_hover;
            if (hover.lock()) {
                ui_hit(hover.lock(), "mousehover");
            }
            return true;
        }
        const auto& wnds = GLOBAL_STATE.render_queue_bk;
        if (wnds.empty())
            return false;
        auto mouse_x = GLOBAL_STATE.mouseInfo.pt.x - GLOBAL_STATE.bound.left;
        auto mouse_y = GLOBAL_STATE.mouseInfo.pt.y - GLOBAL_STATE.bound.top;
        for (const auto& i : wnds) {
            auto j = i.lock();
            if (js_trans(j->get("hit")) && j->hit(mouse_x, mouse_y)) {
                switch (n) {
                case WE_LeftButtonDown:
                    ui_hit(j, "leftbuttondown");
                    break;
                case WE_LeftButtonUp:
                    ui_hit(j, "leftbuttonup");
                    break;
                case WE_LeftButtonDoubleClick:
                    ui_hit(j, "leftbuttondoubleclick");
                    break;
                case WE_RightButtonDown:
                    ui_hit(j, "rightbuttondown");
                    break;
                case WE_RightButtonUp:
                    ui_hit(j, "rightbuttonup");
                    break;
                case WE_RightButtonDoubleClick:
                    ui_hit(j, "rightbuttondoubleclick");
                    break;
                case WE_MiddleButtonDown:
                    ui_hit(j, "middlebuttondown");
                    break;
                case WE_MiddleButtonUp:
                    ui_hit(j, "middlebuttonup");
                    break;
                case WE_MiddleButtonDoubleClick:
                    ui_hit(j, "middlebuttondoubleclick");
                    break;
                case WE_HorizontalWheel:
                    ui_hit(j, "horizontalwheel");
                    break;
                case WE_VerticalWheel:
                    ui_hit(j, "verticalwheel");
                    break;
                case WE_MouseMoving:
                    ui_hit(j, "mousemove");
                    break;
                case WE_MouseLeaved:
                    ui_hit(j, "mouseleave");
                    break;
                case WE_MouseHover:
                    ui_hit(j, "mousehover");
                    break;
                default: {
                    std::stringstream ss;
                    ss << "Unknown hit type: " << n;
                    ui_hit(j, ss.str());
                }
                       break;
                }
                if (n == WE_MouseMoving) {
                    auto& hover = GLOBAL_STATE.ui_hover;
                    if (hover.lock()) {
                        if (hover.lock() != j) {
                            ui_hit(hover.lock(), "mouseleave");
                            hover = i;
                            ui_hit(hover.lock(), "mouseenter");
                        }
                    }
                    else {
                        hover = i;
                        ui_hit(hover.lock(), "mouseenter");
                    }
                    GLOBAL_STATE.cursor = hover.lock()->get_cursor();
                }
                else if (n >= WE_LeftButtonDown && n <= WE_MiddleButtonDoubleClick) {
                    auto& focus = GLOBAL_STATE.ui_focus;
                    if (focus.lock()) {
                        if (focus.lock() != j) {
                            ui_hit(focus.lock(), "lostfocus");
                            focus = i;
                            ui_hit(focus.lock(), "gotfocus");
                        }
                    }
                    else {
                        focus = i;
                        ui_hit(focus.lock(), "gotfocus");
                    }
                }
                return true;
            }
        }
        if (n == WE_MouseMoving) {
            auto& hover = GLOBAL_STATE.ui_hover;
            if (hover.lock()) {
                ui_hit(hover.lock(), "mouseleave");
                GLOBAL_STATE.cursor = 0;
                hover.reset();
            }
        }
        else if (n >= WE_LeftButtonDown && n <= WE_MiddleButtonDoubleClick) {
            auto& focus = GLOBAL_STATE.ui_focus;
            if (focus.lock()) {
                ui_hit(focus.lock(), "lostfocus");
                focus.reset();
            }
        }
        else return false;
        return true;
    }

    int cjsruntime::eval_ui(bool signal) {
        if (signal && global_ui.signals.empty()) {
            return 0;
        }
        auto current = current_stack;
        auto size = current_stack ? current_stack->stack.size() : 0;
        auto stack_size = stack.size();
        do {
            if (signal) {
                if (!global_ui.signals.empty()) {
                    auto s = global_ui.signals.front();
                    global_ui.signals.pop_front();
                    std::stringstream ss;
                    ss << "<signal::" << s << ">";
                    auto name = ss.str();
                    if (is_cached(name)) {
                        exec(name, "", false, true);
                        break;
                    }
                    ss.str("");
                    ss << "sys.send_signal(\"" << s << "\");";
                    exec(name, ss.str(), false, true);
                }
            }
            else {
                global_ui.frames++;
                static auto name = std::string("<signal::render>");
                if (is_cached(name)) {
                    exec(name, "", false, true);
                    break;
                }
                static auto code = std::string("sys.send_signal(\"render\");");
                exec(name, code, false, true);
            }
        } while (0);
        auto r = call_internal(false, stack_size);
        current_stack = current;
        if (current_stack)
            current_stack->stack.resize(size);
        return 0;
    }

    // ---------------------- EMPTY ----------------------

    int js_ui_empty::get_type()
    {
        return js_ui_base::empty;
    }

    const char* js_ui_empty::get_type_str() const
    {
        return "empty";
    }

    void js_ui_empty::render()
    {
    }

    void js_ui_empty::clear()
    {
    }

    void js_ui_empty::change_target()
    {
    }

    void js_ui_empty::add(const std::string& s, const js_value::ref& obj)
    {
        cjsgui::singleton().trigger_render();
    }

    void js_ui_empty::remove(const std::string& s)
    {
        cjsgui::singleton().trigger_render();
    }

    bool js_ui_empty::hit(int x, int y) const
    {
        auto bounds = CRect(left, top, left + width, top + height);
        return bounds.PtInRect(CPoint(x, y));
    }

    bool js_ui_empty::is_dynamic() const
    {
        return false;
    }

    // ---------------------- LABEL ----------------------

    js_ui_label::js_ui_label()
    {
        label = cjsrender_label::create();
        change_target();
    }

    int js_ui_label::get_type()
    {
        return js_ui_base::label;
    }

    const char* js_ui_label::get_type_str() const
    {
        return "label";
    }

    void js_ui_label::render()
    {
        auto bounds = CRect(left, top, left + width, top + height);
        label->get_renderer()->render(bounds, GLOBAL_STATE.renderTarget);
    }

    void js_ui_label::clear()
    {
        label->get_renderer()->destroy2();
    }

    void js_ui_label::change_target()
    {
        label->get_renderer()->on_changed();
    }

    void js_ui_label::add(const std::string& s, const js_value::ref& obj)
    {
        if (s == "color") {
            if (obj->get_type() == r_string)
                label->set_color(CColor::Parse(CStringA(JS_STR(obj).c_str())));
            else
                return;
        }
        else if (s == "content") {
            if (obj->get_type() == r_string)
                label->set_text(CString(JS_S(obj)->wstr.c_str()));
            else
                return;
        }
        else if (s == "font") {
            if (obj && !obj->is_primitive()) {
                auto font = JS_O(obj);
                auto f = label->get_font();
                auto o = font->get("family", nullptr);
                if (o && o->get_type() == r_string) {
                    f.fontFamily = CStringA(JS_STR(o).c_str());
                    if (f.fontFamily.IsEmpty()) {
                        f.fontFamily = _T("Microsoft Yahei");
                    }
                }
                o = font->get("size", nullptr);
                if (o && o->get_type() == r_number) {
                    f.size = max(1, min(256, (int)JS_NUM(o)));
                }
                o = font->get("bold", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.bold = JS_BOOL(o);
                }
                o = font->get("italic", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.italic = JS_BOOL(o);
                }
                o = font->get("underline", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.underline = JS_BOOL(o);
                }
                o = font->get("strikeline", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.strikeline = JS_BOOL(o);
                }
                o = font->get("antialias", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.antialias = JS_BOOL(o);
                }
                o = font->get("verticalAntialias", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.verticalAntialias = JS_BOOL(o);
                }
                label->set_font(f);
            }
        }
        else if (s == "align") {
            if (obj->get_type() == r_string) {
                auto a = JS_STR(obj);
                if (a == "left")
                    label->set_horizontal_alignment(cjsrender_label::Alignment::StringAlignmentNear);
                else if (a == "center")
                    label->set_horizontal_alignment(cjsrender_label::Alignment::StringAlignmentCenter);
                else if (a == "right")
                    label->set_horizontal_alignment(cjsrender_label::Alignment::StringAlignmentFar);
                else
                    return;
            }
            else
                return;
        }
        else if (s == "valign") {
            if (obj->get_type() == r_string) {
                auto a = JS_STR(obj);
                if (a == "top")
                    label->set_vertical_alignment(cjsrender_label::Alignment::StringAlignmentNear);
                else if (a == "center")
                    label->set_vertical_alignment(cjsrender_label::Alignment::StringAlignmentCenter);
                else if (a == "bottom")
                    label->set_vertical_alignment(cjsrender_label::Alignment::StringAlignmentFar);
                else
                    return;
            }
            else
                return;
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    void js_ui_label::remove(const std::string& s)
    {
        if (s == "color") {
            label->set_color(CColor());
        }
        else if (s == "content") {
            label->set_text(CString());
        }
        else if (s == "font") {
            Font f;
            f.fontFamily = _T("Microsoft Yahei");
            f.size = 12;
            label->set_font(f);
        }
        else if (s == "align") {
            label->set_horizontal_alignment(cjsrender_label::Alignment::StringAlignmentNear);
        }
        else if (s == "valign") {
            label->set_vertical_alignment(cjsrender_label::Alignment::StringAlignmentNear);
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    bool js_ui_label::hit(int x, int y) const
    {
        auto bounds = CRect(left, top, left + width, top + height);
        return bounds.PtInRect(CPoint(x, y));
    }

    bool js_ui_label::is_dynamic() const
    {
        return false;
    }

    // ---------------------- RECT ----------------------

    js_ui_rect::js_ui_rect()
    {
        rect = cjsrender_rect::create();
        change_target();
    }

    int js_ui_rect::get_type()
    {
        return js_ui_base::rect;
    }

    const char* js_ui_rect::get_type_str() const
    {
        return "rect";
    }

    void js_ui_rect::render()
    {
        auto bounds = CRect(left, top, left + width, top + height);
        rect->get_renderer()->render(bounds, GLOBAL_STATE.renderTarget);
    }

    void js_ui_rect::clear()
    {
        rect->get_renderer()->destroy2();
    }

    void js_ui_rect::change_target()
    {
        rect->get_renderer()->on_changed();
    }

    void js_ui_rect::add(const std::string& s, const js_value::ref& obj)
    {
        if (s == "color") {
            if (obj->get_type() == r_string)
                rect->set_color(CColor::Parse(CStringA(JS_STR(obj).c_str())));
            else
                return;
        }
        else if (s == "fill") {
            if (obj->get_type() == r_boolean)
                rect->set_fill(JS_BOOL(obj));
            else
                return;
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    void js_ui_rect::remove(const std::string& s)
    {
        if (s == "color") {
            rect->set_color(CColor());
        }
        else if (s == "fill") {
            rect->set_fill(true);
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    bool js_ui_rect::hit(int x, int y) const
    {
        auto bounds = CRect(left, top, left + width, top + height);
        return bounds.PtInRect(CPoint(x, y));
    }

    bool js_ui_rect::is_dynamic() const
    {
        return false;
    }

    // ---------------------- ROUND ----------------------

    js_ui_round::js_ui_round()
    {
        round = cjsrender_round::create();
        change_target();
    }

    int js_ui_round::get_type()
    {
        return js_ui_base::round;
    }

    const char* js_ui_round::get_type_str() const
    {
        return "round";
    }

    void js_ui_round::render()
    {
        auto bounds = CRect(left, top, left + width, top + height);
        round->get_renderer()->render(bounds, GLOBAL_STATE.renderTarget);
    }

    void js_ui_round::clear()
    {
        round->get_renderer()->destroy2();
    }

    void js_ui_round::change_target()
    {
        round->get_renderer()->on_changed();
    }

    void js_ui_round::add(const std::string& s, const js_value::ref& obj)
    {
        if (s == "color") {
            if (obj->get_type() == r_string)
                round->set_color(CColor::Parse(CStringA(JS_STR(obj).c_str())));
            else
                return;
        }
        else if (s == "fill") {
            if (obj->get_type() == r_boolean)
                round->set_fill(JS_BOOL(obj));
            else
                return;
        }
        else if (s == "radius") {
            round->set_radius(obj2float(obj));
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    void js_ui_round::remove(const std::string& s)
    {
        if (s == "color") {
            round->set_color(CColor());
        }
        else if (s == "fill") {
            round->set_fill(true);
        }
        else if (s == "radius") {
            round->set_radius(0);
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    bool js_ui_round::hit(int x, int y) const
    {
        auto bounds = CRect(left, top, left + width, top + height);
        return bounds.PtInRect(CPoint(x, y));
    }

    bool js_ui_round::is_dynamic() const
    {
        return false;
    }

    // ---------------------- QR ----------------------

    js_ui_qr::js_ui_qr()
    {
        qr = cjsrender_qr::create();
        change_target();
    }

    int js_ui_qr::get_type()
    {
        return js_ui_base::qr;
    }

    const char* js_ui_qr::get_type_str() const
    {
        return "qr";
    }

    void js_ui_qr::render()
    {
        auto bounds = CRect(left, top, left + width, top + height);
        qr->get_renderer()->render(bounds, GLOBAL_STATE.renderTarget);
    }

    void js_ui_qr::clear()
    {
        qr->get_renderer()->destroy2();
    }

    void js_ui_qr::change_target()
    {
        qr->get_renderer()->on_changed();
    }

    void js_ui_qr::add(const std::string& s, const js_value::ref& obj)
    {
        if (s == "color") {
            if (obj->get_type() == r_string)
                qr->set_color(CColor::Parse(CStringA(JS_STR(obj).c_str())));
            else
                return;
        }
        else if (s == "text") {
            if (obj->get_type() == r_string)
                qr->set_text(JS_STR(obj));
            else
                return;
        }
        else if (s == "background") {
            if (obj->get_type() == r_string)
                qr->set_background(CColor::Parse(CStringA(JS_STR(obj).c_str())));
            else
                return;
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    void js_ui_qr::remove(const std::string& s)
    {
        if (s == "color") {
            qr->set_color(CColor());
        }
        else if (s == "text") {
            qr->set_text("");
        }
        else if (s == "background") {
            qr->set_background(CColor(255, 255, 255));
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    bool js_ui_qr::hit(int x, int y) const
    {
        auto bounds = CRect(left, top, left + width, top + height);
        return bounds.PtInRect(CPoint(x, y));
    }

    bool js_ui_qr::is_dynamic() const
    {
        return false;
    }

    // ---------------------- IMAGE ----------------------

    js_ui_image::js_ui_image()
    {
        image = cjsrender_image::create();
        change_target();
    }

    int js_ui_image::get_type()
    {
        return js_ui_base::image;
    }

    const char* js_ui_image::get_type_str() const
    {
        return "image";
    }

    void js_ui_image::render()
    {
        auto bounds = CRect(left, top, left + width, top + height);
        image->get_renderer()->render(bounds, GLOBAL_STATE.renderTarget);
    }

    void js_ui_image::clear()
    {
        image->get_renderer()->destroy2();
    }

    void js_ui_image::change_target()
    {
        image->get_renderer()->on_changed();
    }

    void js_ui_image::add(const std::string& s, const js_value::ref& obj)
    {
        if (s == "data") {
            if (!obj->is_primitive()) {
                auto buffer = JS_O(obj);
                auto b = buffer->get_buffer();
                image->set_data(b);
            }
            else
                return;
        }
        else if (s == "full") {
            image->set_full(obj->to_bool());
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    void js_ui_image::remove(const std::string& s)
    {
        if (s == "data") {
            std::vector<char> d;
            image->set_data(d);
        }
        else if (s == "full") {
            image->set_full(false);
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    bool js_ui_image::hit(int x, int y) const
    {
        auto bounds = CRect(left, top, left + width, top + height);
        return bounds.PtInRect(CPoint(x, y));
    }

    bool js_ui_image::is_dynamic() const
    {
        return image->get_renderer()->is_dynamic();
    }

    // ---------------------- SVG ----------------------

    js_ui_svg::js_ui_svg()
    {
        svg = cjsrender_svg::create();
        change_target();
    }

    int js_ui_svg::get_type()
    {
        return js_ui_base::svg;
    }

    const char* js_ui_svg::get_type_str() const
    {
        return "svg";
    }

    void js_ui_svg::render()
    {
        auto bounds = CRect(left, top, left + width, top + height);
        svg->get_renderer()->render(bounds, GLOBAL_STATE.renderTarget);
    }

    void js_ui_svg::clear()
    {
        svg->get_renderer()->destroy2();
    }

    void js_ui_svg::change_target()
    {
        svg->get_renderer()->on_changed();
    }

    void js_ui_svg::add(const std::string& s, const js_value::ref& obj)
    {
        if (s == "text") {
            if (obj->get_type() == r_string)
                svg->set_text(JS_STR(obj));
            else
                return;
        }
        else if (s == "background") {
            if (obj->get_type() == r_string)
                svg->set_background(CColor::Parse(CStringA(JS_STR(obj).c_str())));
            else
                return;
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    void js_ui_svg::remove(const std::string& s)
    {
        if (s == "text") {
            svg->set_text("");
        }
        else if (s == "background") {
            svg->set_background(CColor(255, 255, 255));
        }
        else
            return;
        cjsgui::singleton().trigger_render();
    }

    bool js_ui_svg::hit(int x, int y) const
    {
        auto bounds = CRect(left, top, left + width, top + height);
        return bounds.PtInRect(CPoint(x, y));
    }

    bool js_ui_svg::is_dynamic() const
    {
        return false;
    }
}