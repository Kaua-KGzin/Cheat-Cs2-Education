#pragma once
#include <windows.h>
#include <string>
#include <cmath>
#include <unordered_map>
#include <mutex>

class Overlay {
public:
    HWND hwnd = nullptr;
    int screen_w = 0;
    int screen_h = 0;

    bool create() {
        screen_w = GetSystemMetrics(SM_CXSCREEN);
        screen_h = GetSystemMetrics(SM_CYSCREEN);

        WNDCLASSEX wc{};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = "CS2ESP";
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        RegisterClassEx(&wc);

        hwnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
            "CS2ESP", "CS2 ESP",
            WS_POPUP,
            0, 0, screen_w, screen_h,
            nullptr, nullptr, GetModuleHandle(nullptr), nullptr
        );

        if (!hwnd) return false;

        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        hdc_screen = GetDC(hwnd);
        hdc_mem = CreateCompatibleDC(hdc_screen);
        hbm_mem = CreateCompatibleBitmap(hdc_screen, screen_w, screen_h);
        hbm_old = (HBITMAP)SelectObject(hdc_mem, hbm_mem);

        SelectObject(hdc_mem, GetStockObject(NULL_BRUSH));
        SetBkMode(hdc_mem, TRANSPARENT);

        hbr_bg = CreateSolidBrush(RGB(0, 0, 0));
        hbr_health = CreateSolidBrush(RGB(40, 40, 40));
        cache_gdi_objects();

        return true;
    }

    void begin() {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        RECT r = {0, 0, screen_w, screen_h};
        FillRect(hdc_mem, &r, hbr_bg);
    }

    void end() {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        BitBlt(hdc_screen, 0, 0, screen_w, screen_h, hdc_mem, 0, 0, SRCCOPY);
    }

    void draw_box(int x, int y, int w, int h, COLORREF color, int thickness = 1) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        draw_box_nolock(x, y, w, h, color, thickness);
    }

    void draw_corner_box(int x, int y, int w, int h, COLORREF color, int thickness = 2) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        draw_corner_box_nolock(x, y, w, h, color, thickness);
    }

    void draw_line(int x1, int y1, int x2, int y2, COLORREF color, int thickness = 1) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        draw_line_nolock(x1, y1, x2, y2, color, thickness);
    }

    void draw_text(int x, int y, const std::string& text, COLORREF color, int size = 12) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        draw_text_nolock(x, y, text, color, size);
    }

    void draw_text_centered(int cx, int y, const std::string& text, COLORREF color, int size = 12) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        auto font_it = font_cache_.find(color);
        HFONT font = (font_it != font_cache_.end()) ? font_it->second : get_or_create_font(color, size);
        HGDIOBJ old = SelectObject(hdc_mem, font);
        SetTextColor(hdc_mem, color);
        SetBkMode(hdc_mem, TRANSPARENT);
        SIZE sz;
        GetTextExtentPoint32A(hdc_mem, text.c_str(), static_cast<int>(text.size()), &sz);
        TextOutA(hdc_mem, cx - sz.cx / 2, y, text.c_str(), static_cast<int>(text.size()));
        SelectObject(hdc_mem, old);
    }

    void draw_crosshair(int cx, int cy, int size, COLORREF color) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        draw_line_nolock(cx - size, cy, cx - 4, cy, color, 2);
        draw_line_nolock(cx + 4, cy, cx + size, cy, color, 2);
        draw_line_nolock(cx, cy - size, cx, cy - 4, color, 2);
        draw_line_nolock(cx, cy + 4, cx, cy + size, color, 2);
    }

    void draw_health_bar(int x, int y, int h, int health, int max_h = 100) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        draw_health_bar_nolock(x, y, h, health, max_h);
    }

    void draw_snapline(int cx, int sh, int tx, int ty, COLORREF color) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        draw_line_nolock(cx, sh, tx, ty, color, 1);
    }

    ~Overlay() {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        if (hdc_mem) {
            SelectObject(hdc_mem, hbm_old);
            DeleteObject(hbm_mem);
            DeleteDC(hdc_mem);
        }
        if (hdc_screen) ReleaseDC(hwnd, hdc_screen);
        if (hwnd) DestroyWindow(hwnd);

        for (auto& pair : pen_cache_) DeleteObject(pair.second);
        for (auto& pair : font_cache_) DeleteObject(pair.second);

        if (hbr_bg) DeleteObject(hbr_bg);
        if (hbr_health) DeleteObject(hbr_health);
    }

private:
    std::recursive_mutex mtx_;
    HDC hdc_screen = nullptr;
    HDC hdc_mem = nullptr;
    HBITMAP hbm_mem = nullptr;
    HBITMAP hbm_old = nullptr;

    HBRUSH hbr_bg = nullptr;
    HBRUSH hbr_health = nullptr;

    std::unordered_map<COLORREF, HPEN> pen_cache_;
    std::unordered_map<COLORREF, HFONT> font_cache_;

    HPEN get_or_create_pen(COLORREF color, int width) {
        auto it = pen_cache_.find(color);
        if (it != pen_cache_.end()) return it->second;
        HPEN pen = CreatePen(PS_SOLID, width, color);
        pen_cache_[color] = pen;
        return pen;
    }

    HFONT get_or_create_font(COLORREF color, int size) {
        auto it = font_cache_.find(color);
        if (it != font_cache_.end()) return it->second;
        HFONT font = CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");
        font_cache_[color] = font;
        return font;
    }

    void cache_gdi_objects() {
        COLORREF common_colors[] = {
            RGB(0, 180, 255), RGB(255, 80, 80), RGB(100, 180, 255),
            RGB(0, 220, 100), RGB(180, 220, 0), RGB(255, 160, 0),
            RGB(255, 60, 60), RGB(255, 210, 50), RGB(0, 50, 40),
            RGB(40, 40, 40), RGB(18, 18, 24), RGB(26, 26, 36),
            RGB(34, 34, 48), RGB(0, 160, 255), RGB(0, 100, 180),
            RGB(230, 230, 240), RGB(120, 120, 145), RGB(0, 200, 120),
            RGB(255, 70, 80), RGB(50, 50, 70), RGB(255, 255, 255),
            RGB(255, 255, 0)
        };

        for (auto color : common_colors) {
            get_or_create_pen(color, 2);
            get_or_create_font(color, 12);
        }
    }

    void draw_line_nolock(int x1, int y1, int x2, int y2, COLORREF color, int thickness) {
        HPEN pen = get_or_create_pen(color, thickness);
        HGDIOBJ old = SelectObject(hdc_mem, pen);
        MoveToEx(hdc_mem, x1, y1, nullptr);
        LineTo(hdc_mem, x2, y2);
        SelectObject(hdc_mem, old);
    }

    void draw_box_nolock(int x, int y, int w, int h, COLORREF color, int thickness) {
        HPEN pen = get_or_create_pen(color, thickness);
        HGDIOBJ old = SelectObject(hdc_mem, pen);
        Rectangle(hdc_mem, x, y, x + w, y + h);
        SelectObject(hdc_mem, old);
    }

    void draw_corner_box_nolock(int x, int y, int w, int h, COLORREF color, int thickness) {
        int corner = w / 4;
        if (corner < 4) corner = 4;

        HPEN pen = get_or_create_pen(color, thickness);
        HGDIOBJ old = SelectObject(hdc_mem, pen);

        auto line = [&](int x1, int y1, int x2, int y2) {
            MoveToEx(hdc_mem, x1, y1, nullptr);
            LineTo(hdc_mem, x2, y2);
        };

        int l = x, r = x + w, t = y, b = y + h;

        line(l, t, l + corner, t);
        line(l, t, l, t + corner);

        line(r - corner, t, r, t);
        line(r, t, r, t + corner);

        line(l, b - corner, l, b);
        line(l, b, l + corner, b);

        line(r, b - corner, r, b);
        line(r - corner, b, r, b);

        SelectObject(hdc_mem, old);
    }

    void draw_text_nolock(int x, int y, const std::string& text, COLORREF color, int size) {
        HFONT font = get_or_create_font(color, size);
        HGDIOBJ old = SelectObject(hdc_mem, font);
        SetTextColor(hdc_mem, color);
        SetBkMode(hdc_mem, TRANSPARENT);
        TextOutA(hdc_mem, x, y, text.c_str(), static_cast<int>(text.size()));
        SelectObject(hdc_mem, old);
    }

    void draw_health_bar_nolock(int x, int y, int h, int health, int max_h) {
        int bar_w = 4;
        int fill = h * (health < max_h ? health : max_h) / max_h;
        if (fill < 1) fill = 1;

        HGDIOBJ old_brush = SelectObject(hdc_mem, hbr_health);
        HPEN bg_pen = get_or_create_pen(RGB(40, 40, 40), 1);
        HGDIOBJ old_pen = SelectObject(hdc_mem, bg_pen);
        Rectangle(hdc_mem, x - bar_w - 4, y, x - 4, y + h);
        SelectObject(hdc_mem, old_pen);

        COLORREF color;
        if (health > 75) color = RGB(0, 220, 100);
        else if (health > 50) color = RGB(180, 220, 0);
        else if (health > 25) color = RGB(255, 160, 0);
        else color = RGB(255, 60, 60);

        HPEN pen = get_or_create_pen(color, 1);
        old_pen = SelectObject(hdc_mem, pen);

        int fill_y = y + h - fill;
        Rectangle(hdc_mem, x - bar_w - 4, fill_y, x - 4, y + h);

        SelectObject(hdc_mem, old_pen);
        SelectObject(hdc_mem, old_brush);
    }
};
