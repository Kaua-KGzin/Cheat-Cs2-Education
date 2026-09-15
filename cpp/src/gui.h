#pragma once
#include <windows.h>
#include <string>
#include <commctrl.h>
#include <fstream>
#include <sstream>

#pragma comment(lib, "comctl32.lib")

struct GuiSettings {
    bool esp_on = false;
    bool trigger_on = false;
    bool aimbot_on = false;
    bool anti_recoil_on = false;
    bool headshot_only = false;
    bool radar_hack = false;
    float aim_sensitivity = 4.5f;
    float aim_smooth = 0.35f;
    int aim_fov = 350;
    int aim_deadzone = 10;
    int aim_stick = 400;
    int trigger_delay = 5;
    int max_distance = 2000;
    int bone_selection = 0;
    int anti_recoil_strength = 50;
    int fps = 0;
    int shots = 0;
    int players = 0;
};

namespace gui {

inline GuiSettings settings;
inline HWND hwnd = nullptr;
inline bool running = true;

inline COLORREF bg_base     = RGB(12, 12, 18);
inline COLORREF bg_card     = RGB(18, 18, 28);
inline COLORREF bg_control  = RGB(24, 24, 38);
inline COLORREF bg_hover    = RGB(32, 32, 50);
inline COLORREF accent      = RGB(0, 180, 240);
inline COLORREF accent_soft = RGB(0, 120, 200);
inline COLORREF accent_glow = RGB(0, 140, 220);
inline COLORREF green       = RGB(0, 220, 140);
inline COLORREF green_dim   = RGB(0, 60, 45);
inline COLORREF red         = RGB(255, 65, 85);
inline COLORREF orange      = RGB(255, 160, 40);
inline COLORREF text_bright = RGB(240, 240, 255);
inline COLORREF text_mid    = RGB(160, 160, 190);
inline COLORREF text_dim    = RGB(90, 90, 115);
inline COLORREF border      = RGB(40, 40, 65);
inline COLORREF border_lite = RGB(55, 55, 85);
inline COLORREF status_bg   = RGB(8, 8, 14);

enum CtrlID {
    ID_BTN_ESP = 1001,
    ID_BTN_TRIGGER,
    ID_BTN_AIMBOT,
    ID_BTN_LEGIT,
    ID_BTN_RAGE,
    ID_BTN_SEMI,
    ID_BTN_ANTI_RECOIL,
    ID_BTN_HEADSHOT,
    ID_BTN_RADAR,
    ID_SLIDER_SENS,
    ID_SLIDER_SMOOTH,
    ID_SLIDER_FOV,
    ID_SLIDER_DZ,
    ID_SLIDER_STICK,
    ID_SLIDER_DELAY,
    ID_SLIDER_MAX_DIST,
    ID_SLIDER_BONE,
    ID_SLIDER_RECOIL_STR,
};

inline HFONT hFont = nullptr;
inline HFONT hFontSmall = nullptr;
inline HFONT hFontTitle = nullptr;
inline HFONT hFontBold = nullptr;
inline HFONT hFontMono = nullptr;

inline HBRUSH hbrBg = nullptr;
inline HBRUSH hbrCard = nullptr;
inline HBRUSH hbrControl = nullptr;
inline HBRUSH hbrStatus = nullptr;

inline HWND hBtnEsp = nullptr;
inline HWND hBtnTrigger = nullptr;
inline HWND hBtnAimbot = nullptr;
inline HWND hBtnLegit = nullptr;
inline HWND hBtnRage = nullptr;
inline HWND hBtnSemi = nullptr;
inline HWND hBtnAntiRecoil = nullptr;
inline HWND hBtnHeadshot = nullptr;
inline HWND hBtnRadar = nullptr;
inline HWND hSliderSens = nullptr;
inline HWND hSliderSmooth = nullptr;
inline HWND hSliderFov = nullptr;
inline HWND hSliderDz = nullptr;
inline HWND hSliderStick = nullptr;
inline HWND hSliderDelay = nullptr;
inline HWND hSliderMaxDist = nullptr;
inline HWND hSliderBone = nullptr;
inline HWND hSliderRecoilStr = nullptr;
inline HWND hLabelSens = nullptr;
inline HWND hLabelSmooth = nullptr;
inline HWND hLabelFov = nullptr;
inline HWND hLabelDz = nullptr;
inline HWND hLabelStick = nullptr;
inline HWND hLabelDelay = nullptr;
inline HWND hLabelMaxDist = nullptr;
inline HWND hLabelBone = nullptr;
inline HWND hLabelRecoilStr = nullptr;
inline HWND hLabelFps = nullptr;
inline HWND hLabelPlayers = nullptr;
inline HWND hLabelShots = nullptr;
inline HWND hStatus = nullptr;

inline HWND CreateLabel(HWND parent, const char* text, int x, int y, int w, int h, HFONT font, COLORREF clr = 0) {
    HWND lbl = CreateWindow("STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, w, h, parent, nullptr, nullptr, nullptr);
    if (lbl && font) SendMessage(lbl, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return lbl;
}

inline HWND CreateSectionHeader(HWND parent, const char* text, int x, int y, int w) {
    HWND lbl = CreateWindow("STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, w, 18, parent, nullptr, nullptr, nullptr);
    if (lbl && hFontBold) SendMessage(lbl, WM_SETFONT, reinterpret_cast<WPARAM>(hFontBold), TRUE);
    return lbl;
}

inline HWND CreateSlider(HWND parent, int id, int x, int y, int w, int min_val, int max_val, int val) {
    HWND sl = CreateWindow(TRACKBAR_CLASS, "",
        WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_FIXEDLENGTH,
        x, y, w, 26, parent, reinterpret_cast<HMENU>(static_cast<intptr_t>(id)), nullptr, nullptr);
    SendMessage(sl, TBM_SETRANGE, TRUE, MAKELPARAM(min_val, max_val));
    SendMessage(sl, TBM_SETPOS, TRUE, val);
    SendMessage(sl, TBM_SETTHUMBLENGTH, 12, 0);
    return sl;
}

inline HWND CreateToggle(HWND parent, int id, const char* text, int x, int y, int w, int h) {
    HWND btn = CreateWindow("BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        x, y, w, h, parent, reinterpret_cast<HMENU>(static_cast<intptr_t>(id)), nullptr, nullptr);
    return btn;
}

inline HWND CreatePresetBtn(HWND parent, int id, const char* text, int x, int y, int w, int h) {
    HWND btn = CreateWindow("BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        x, y, w, h, parent, reinterpret_cast<HMENU>(static_cast<intptr_t>(id)), nullptr, nullptr);
    return btn;
}

inline void UpdateLabels() {
    char buf[32];
    sprintf(buf, "%.1f", settings.aim_sensitivity);
    if (hLabelSens) SetWindowTextA(hLabelSens, buf);
    sprintf(buf, "%.2f", settings.aim_smooth);
    if (hLabelSmooth) SetWindowTextA(hLabelSmooth, buf);
    sprintf(buf, "%d", settings.aim_fov);
    if (hLabelFov) SetWindowTextA(hLabelFov, buf);
    sprintf(buf, "%d", settings.aim_deadzone);
    if (hLabelDz) SetWindowTextA(hLabelDz, buf);
    sprintf(buf, "%d", settings.aim_stick);
    if (hLabelStick) SetWindowTextA(hLabelStick, buf);
    sprintf(buf, "%d ms", settings.trigger_delay);
    if (hLabelDelay) SetWindowTextA(hLabelDelay, buf);
    sprintf(buf, "%d", settings.max_distance);
    if (hLabelMaxDist) SetWindowTextA(hLabelMaxDist, buf);
    const char* bones[] = {"Head", "Chest", "Stomach", "Pelvis"};
    int bone_idx = settings.bone_selection;
    if (bone_idx < 0) bone_idx = 0;
    if (bone_idx > 3) bone_idx = 3;
    if (hLabelBone) SetWindowTextA(hLabelBone, bones[bone_idx]);
    sprintf(buf, "%d", settings.anti_recoil_strength);
    if (hLabelRecoilStr) SetWindowTextA(hLabelRecoilStr, buf);
    sprintf(buf, "%d", settings.fps);
    if (hLabelFps) SetWindowTextA(hLabelFps, buf);
    sprintf(buf, "%d", settings.players);
    if (hLabelPlayers) SetWindowTextA(hLabelPlayers, buf);
    sprintf(buf, "%d", settings.shots);
    if (hLabelShots) SetWindowTextA(hLabelShots, buf);
}

inline void UpdateButtonColors() {
    if (hBtnEsp) SetWindowTextA(hBtnEsp, settings.esp_on ? "ESP  ON" : "ESP  OFF");
    if (hBtnTrigger) SetWindowTextA(hBtnTrigger, settings.trigger_on ? "Trigger  ON" : "Trigger  OFF");
    if (hBtnAimbot) SetWindowTextA(hBtnAimbot, settings.aimbot_on ? "Aimbot  ON" : "Aimbot  OFF");
    if (hBtnAntiRecoil) SetWindowTextA(hBtnAntiRecoil, settings.anti_recoil_on ? "Recoil  ON" : "Recoil  OFF");
    if (hBtnHeadshot) SetWindowTextA(hBtnHeadshot, settings.headshot_only ? "HS Only  ON" : "HS Only  OFF");
    if (hBtnRadar) SetWindowTextA(hBtnRadar, settings.radar_hack ? "Radar  ON" : "Radar  OFF");
}

inline void SyncSliders() {
    if (hSliderSens) SendMessage(hSliderSens, TBM_SETPOS, TRUE, static_cast<int>(settings.aim_sensitivity * 10));
    if (hSliderSmooth) SendMessage(hSliderSmooth, TBM_SETPOS, TRUE, static_cast<int>(settings.aim_smooth * 100));
    if (hSliderFov) SendMessage(hSliderFov, TBM_SETPOS, TRUE, settings.aim_fov);
    if (hSliderDz) SendMessage(hSliderDz, TBM_SETPOS, TRUE, settings.aim_deadzone);
    if (hSliderStick) SendMessage(hSliderStick, TBM_SETPOS, TRUE, settings.aim_stick);
    if (hSliderDelay) SendMessage(hSliderDelay, TBM_SETPOS, TRUE, settings.trigger_delay);
    if (hSliderMaxDist) SendMessage(hSliderMaxDist, TBM_SETPOS, TRUE, settings.max_distance);
    if (hSliderBone) SendMessage(hSliderBone, TBM_SETPOS, TRUE, settings.bone_selection);
    if (hSliderRecoilStr) SendMessage(hSliderRecoilStr, TBM_SETPOS, TRUE, settings.anti_recoil_strength);
    UpdateLabels();
}

inline void apply_preset(const char* name) {
    if (strcmp(name, "legit") == 0) {
        settings.aim_sensitivity = 1.2f;
        settings.aim_smooth = 0.08f;
        settings.aim_fov = 40;
        settings.aim_deadzone = 3;
        settings.aim_stick = 70;
        settings.trigger_delay = 90;
        settings.max_distance = 1000;
        settings.bone_selection = 1;
        settings.anti_recoil_strength = 20;
    } else if (strcmp(name, "semi") == 0) {
        settings.aim_sensitivity = 2.5f;
        settings.aim_smooth = 0.18f;
        settings.aim_fov = 150;
        settings.aim_deadzone = 6;
        settings.aim_stick = 200;
        settings.trigger_delay = 30;
        settings.max_distance = 1500;
        settings.bone_selection = 0;
        settings.anti_recoil_strength = 35;
    } else if (strcmp(name, "rage") == 0) {
        settings.aim_sensitivity = 4.5f;
        settings.aim_smooth = 0.35f;
        settings.aim_fov = 350;
        settings.aim_deadzone = 10;
        settings.aim_stick = 400;
        settings.trigger_delay = 5;
        settings.max_distance = 2000;
        settings.bone_selection = 0;
        settings.anti_recoil_strength = 50;
    }
    SyncSliders();
}

inline void DrawToggleBtn(LPDRAWITEMSTRUCT dis) {
    bool is_on = false;
    int id = GetDlgCtrlID(dis->hwndItem);
    if (id == ID_BTN_ESP) is_on = settings.esp_on;
    else if (id == ID_BTN_TRIGGER) is_on = settings.trigger_on;
    else if (id == ID_BTN_AIMBOT) is_on = settings.aimbot_on;
    else if (id == ID_BTN_ANTI_RECOIL) is_on = settings.anti_recoil_on;
    else if (id == ID_BTN_HEADSHOT) is_on = settings.headshot_only;
    else if (id == ID_BTN_RADAR) is_on = settings.radar_hack;

    HDC hdc = dis->hDC;
    RECT rc = dis->rcItem;

    COLORREF bg = is_on ? RGB(0, 35, 30) : bg_control;
    HBRUSH br = CreateSolidBrush(bg);
    FillRect(hdc, &rc, br);
    DeleteObject(br);

    if (is_on) {
        HPEN pen = CreatePen(PS_SOLID, 2, green);
        HPEN old = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
        SelectObject(hdc, old);
        SelectObject(hdc, oldBr);
        DeleteObject(pen);
    } else {
        HPEN pen = CreatePen(PS_SOLID, 1, border);
        HPEN old = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
        SelectObject(hdc, old);
        SelectObject(hdc, oldBr);
        DeleteObject(pen);
    }

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, is_on ? green : text_dim);
    if (hFontBold) SelectObject(hdc, hFontBold);
    const char* txt = "???";
    if (dis->hwndItem == hBtnEsp) txt = is_on ? "ESP  [ON]" : "ESP  [OFF]";
    else if (dis->hwndItem == hBtnTrigger) txt = is_on ? "TRIGGER  [ON]" : "TRIGGER  [OFF]";
    else if (dis->hwndItem == hBtnAimbot) txt = is_on ? "AIMBOT  [ON]" : "AIMBOT  [OFF]";
    else if (dis->hwndItem == hBtnAntiRecoil) txt = is_on ? "RECOIL  [ON]" : "RECOIL  [OFF]";
    else if (dis->hwndItem == hBtnHeadshot) txt = is_on ? "HS ONLY  [ON]" : "HS ONLY  [OFF]";
    else if (dis->hwndItem == hBtnRadar) txt = is_on ? "RADAR  [ON]" : "RADAR  [OFF]";
    DrawTextA(hdc, txt, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

inline void DrawPresetBtn(LPDRAWITEMSTRUCT dis) {
    HDC hdc = dis->hDC;
    RECT rc = dis->rcItem;

    bool hovered = (dis->itemState & ODS_SELECTED);

    COLORREF bg = hovered ? bg_hover : bg_card;
    HBRUSH br = CreateSolidBrush(bg);
    FillRect(hdc, &rc, br);
    DeleteObject(br);

    HPEN pen = CreatePen(PS_SOLID, 1, hovered ? accent_soft : border_lite);
    HPEN old = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
    SelectObject(hdc, old);
    SelectObject(hdc, oldBr);
    DeleteObject(pen);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, hovered ? accent : text_mid);
    if (hFontBold) SelectObject(hdc, hFontBold);
    char txt[32];
    GetWindowText(dis->hwndItem, txt, sizeof(txt));
    DrawTextA(hdc, txt, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

inline void DrawSeparator(HDC hdc, RECT rc) {
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HPEN old = (HPEN)SelectObject(hdc, pen);
    MoveToEx(hdc, rc.left, rc.top + 1, nullptr);
    LineTo(hdc, rc.right, rc.top + 1);
    SelectObject(hdc, old);
    DeleteObject(pen);
}

inline LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_HSCROLL: {
        HWND sl = reinterpret_cast<HWND>(lParam);
        int pos = static_cast<int>(SendMessage(sl, TBM_GETPOS, 0, 0));
        if (sl == hSliderSens) settings.aim_sensitivity = pos / 10.0f;
        else if (sl == hSliderSmooth) settings.aim_smooth = pos / 100.0f;
        else if (sl == hSliderFov) settings.aim_fov = pos;
        else if (sl == hSliderDz) settings.aim_deadzone = pos;
        else if (sl == hSliderStick) settings.aim_stick = pos;
        else if (sl == hSliderDelay) settings.trigger_delay = pos;
        else if (sl == hSliderMaxDist) settings.max_distance = pos;
        else if (sl == hSliderBone) settings.bone_selection = pos;
        else if (sl == hSliderRecoilStr) settings.anti_recoil_strength = pos;
        UpdateLabels();
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_ESP:
            settings.esp_on = !settings.esp_on;
            UpdateButtonColors();
            if (hBtnEsp) InvalidateRect(hBtnEsp, nullptr, TRUE);
            break;
        case ID_BTN_TRIGGER:
            settings.trigger_on = !settings.trigger_on;
            UpdateButtonColors();
            if (hBtnTrigger) InvalidateRect(hBtnTrigger, nullptr, TRUE);
            break;
        case ID_BTN_AIMBOT:
            settings.aimbot_on = !settings.aimbot_on;
            UpdateButtonColors();
            if (hBtnAimbot) InvalidateRect(hBtnAimbot, nullptr, TRUE);
            break;
        case ID_BTN_ANTI_RECOIL:
            settings.anti_recoil_on = !settings.anti_recoil_on;
            UpdateButtonColors();
            if (hBtnAntiRecoil) InvalidateRect(hBtnAntiRecoil, nullptr, TRUE);
            break;
        case ID_BTN_HEADSHOT:
            settings.headshot_only = !settings.headshot_only;
            UpdateButtonColors();
            if (hBtnHeadshot) InvalidateRect(hBtnHeadshot, nullptr, TRUE);
            break;
        case ID_BTN_RADAR:
            settings.radar_hack = !settings.radar_hack;
            UpdateButtonColors();
            if (hBtnRadar) InvalidateRect(hBtnRadar, nullptr, TRUE);
            break;
        case ID_BTN_LEGIT:
            apply_preset("legit");
            break;
        case ID_BTN_SEMI:
            apply_preset("semi");
            break;
        case ID_BTN_RAGE:
            apply_preset("rage");
            break;
        }
        break;
    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT dis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
        int id = GetDlgCtrlID(dis->hwndItem);
        if (id == ID_BTN_ESP || id == ID_BTN_TRIGGER || id == ID_BTN_AIMBOT ||
            id == ID_BTN_ANTI_RECOIL || id == ID_BTN_HEADSHOT || id == ID_BTN_RADAR) {
            if (dis->hwndItem) DrawToggleBtn(dis);
            return TRUE;
        } else if (id == ID_BTN_LEGIT || id == ID_BTN_SEMI || id == ID_BTN_RAGE) {
            if (dis->hwndItem) DrawPresetBtn(dis);
            return TRUE;
        }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        SetTextColor(hdc, text_mid);
        SetBkColor(hdc, bg_base);
        return reinterpret_cast<LRESULT>(hbrBg);
    }
    case WM_CTLCOLORBTN: {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        SetTextColor(hdc, text_bright);
        SetBkColor(hdc, bg_control);
        return reinterpret_cast<LRESULT>(hbrControl);
    }
    case WM_CTLCOLORSCROLLBAR: {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        SetBkColor(hdc, bg_control);
        return reinterpret_cast<LRESULT>(hbrControl);
    }
    case WM_ERASEBKGND: {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, hbrBg);
        return 1;
    }
    case WM_CLOSE:
        running = false;
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

inline bool save_config(const char* path) {
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << "esp_on=" << (settings.esp_on ? 1 : 0) << "\n";
    f << "trigger_on=" << (settings.trigger_on ? 1 : 0) << "\n";
    f << "aimbot_on=" << (settings.aimbot_on ? 1 : 0) << "\n";
    f << "anti_recoil_on=" << (settings.anti_recoil_on ? 1 : 0) << "\n";
    f << "headshot_only=" << (settings.headshot_only ? 1 : 0) << "\n";
    f << "radar_hack=" << (settings.radar_hack ? 1 : 0) << "\n";
    f << "aim_sensitivity=" << settings.aim_sensitivity << "\n";
    f << "aim_smooth=" << settings.aim_smooth << "\n";
    f << "aim_fov=" << settings.aim_fov << "\n";
    f << "aim_deadzone=" << settings.aim_deadzone << "\n";
    f << "aim_stick=" << settings.aim_stick << "\n";
    f << "trigger_delay=" << settings.trigger_delay << "\n";
    f << "max_distance=" << settings.max_distance << "\n";
    f << "bone_selection=" << settings.bone_selection << "\n";
    f << "anti_recoil_strength=" << settings.anti_recoil_strength << "\n";
    f << "fps=" << settings.fps << "\n";
    f << "shots=" << settings.shots << "\n";
    f << "players=" << settings.players << "\n";
    f.close();
    return true;
}

inline bool load_config(const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    char line[256];
    while (f.getline(line, sizeof(line))) {
        char key[64];
        char val[64];
        if (sscanf(line, "%63[^=]=%63s", key, val) == 2) {
            if (strcmp(key, "esp_on") == 0) settings.esp_on = atoi(val) != 0;
            else if (strcmp(key, "trigger_on") == 0) settings.trigger_on = atoi(val) != 0;
            else if (strcmp(key, "aimbot_on") == 0) settings.aimbot_on = atoi(val) != 0;
            else if (strcmp(key, "anti_recoil_on") == 0) settings.anti_recoil_on = atoi(val) != 0;
            else if (strcmp(key, "headshot_only") == 0) settings.headshot_only = atoi(val) != 0;
            else if (strcmp(key, "radar_hack") == 0) settings.radar_hack = atoi(val) != 0;
            else if (strcmp(key, "aim_sensitivity") == 0) settings.aim_sensitivity = static_cast<float>(atof(val));
            else if (strcmp(key, "aim_smooth") == 0) settings.aim_smooth = static_cast<float>(atof(val));
            else if (strcmp(key, "aim_fov") == 0) settings.aim_fov = atoi(val);
            else if (strcmp(key, "aim_deadzone") == 0) settings.aim_deadzone = atoi(val);
            else if (strcmp(key, "aim_stick") == 0) settings.aim_stick = atoi(val);
            else if (strcmp(key, "trigger_delay") == 0) settings.trigger_delay = atoi(val);
            else if (strcmp(key, "max_distance") == 0) settings.max_distance = atoi(val);
            else if (strcmp(key, "bone_selection") == 0) settings.bone_selection = atoi(val);
            else if (strcmp(key, "anti_recoil_strength") == 0) settings.anti_recoil_strength = atoi(val);
        }
    }
    f.close();
    SyncSliders();
    UpdateButtonColors();
    return true;
}

inline bool create() {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    hbrBg = CreateSolidBrush(bg_base);
    hbrCard = CreateSolidBrush(bg_card);
    hbrControl = CreateSolidBrush(bg_control);
    hbrStatus = CreateSolidBrush(status_bg);

    hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    hFontSmall = CreateFont(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    hFontBold = CreateFont(13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    hFontTitle = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    hFontMono = CreateFont(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");

    WNDCLASSEX wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "KGzinRETORNAV2";
    wc.hbrBackground = hbrBg;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassEx(&wc);

    int w = 340, h = 810;
    int sx = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int sy = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    hwnd = CreateWindowEx(0, "KGzinRETORNAV2", "KGzin RETORNAV2",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        sx, sy, w, h, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

    if (!hwnd) return false;

    int y = 14;
    int lw = 90;
    int sw = 115;
    int sx_start = 16;
    int content_w = 308;

    HWND hTitle = CreateLabel(hwnd, "KGzin RETORNAV2", sx_start, y, content_w, 30, hFontTitle, accent);
    y += 34;

    HWND hSep1 = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        sx_start, y, content_w, 1, hwnd, nullptr, nullptr, nullptr);
    y += 12;

    hBtnEsp = CreateToggle(hwnd, ID_BTN_ESP, "ESP  [OFF]", sx_start, y, 98, 32);
    hBtnTrigger = CreateToggle(hwnd, ID_BTN_TRIGGER, "Trigger  [OFF]", sx_start + 104, y, 98, 32);
    hBtnAimbot = CreateToggle(hwnd, ID_BTN_AIMBOT, "Aimbot  [OFF]", sx_start + 208, y, 98, 32);
    y += 40;

    hBtnAntiRecoil = CreateToggle(hwnd, ID_BTN_ANTI_RECOIL, "Recoil  [OFF]", sx_start, y, 98, 32);
    hBtnHeadshot = CreateToggle(hwnd, ID_BTN_HEADSHOT, "HS Only  [OFF]", sx_start + 104, y, 98, 32);
    hBtnRadar = CreateToggle(hwnd, ID_BTN_RADAR, "Radar  [OFF]", sx_start + 208, y, 98, 32);
    y += 42;

    CreateSectionHeader(hwnd, "PRESETS", sx_start, y, 200);
    y += 22;

    hBtnLegit = CreatePresetBtn(hwnd, ID_BTN_LEGIT, "Legit", sx_start, y, 98, 28);
    hBtnSemi = CreatePresetBtn(hwnd, ID_BTN_SEMI, "Semi", sx_start + 104, y, 98, 28);
    hBtnRage = CreatePresetBtn(hwnd, ID_BTN_RAGE, "Rage", sx_start + 208, y, 98, 28);
    y += 40;

    HWND hSep2 = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        sx_start, y, content_w, 1, hwnd, nullptr, nullptr, nullptr);
    y += 12;

    CreateSectionHeader(hwnd, "AIMBOT", sx_start, y, 200);
    y += 24;

    HWND lb;
    lb = CreateLabel(hwnd, "Sensitivity", sx_start, y + 5, lw, 18, hFont);
    hSliderSens = CreateSlider(hwnd, ID_SLIDER_SENS, sx_start + lw + 4, y, sw, 5, 80, 45);
    hLabelSens = CreateLabel(hwnd, "4.5", sx_start + lw + sw + 10, y + 5, 45, 18, hFont, accent);
    y += 32;

    lb = CreateLabel(hwnd, "Smooth", sx_start, y + 5, lw, 18, hFont);
    hSliderSmooth = CreateSlider(hwnd, ID_SLIDER_SMOOTH, sx_start + lw + 4, y, sw, 1, 100, 35);
    hLabelSmooth = CreateLabel(hwnd, "0.35", sx_start + lw + sw + 10, y + 5, 45, 18, hFont, accent);
    y += 32;

    lb = CreateLabel(hwnd, "FOV", sx_start, y + 5, lw, 18, hFont);
    hSliderFov = CreateSlider(hwnd, ID_SLIDER_FOV, sx_start + lw + 4, y, sw, 10, 500, 350);
    hLabelFov = CreateLabel(hwnd, "350", sx_start + lw + sw + 10, y + 5, 45, 18, hFont, accent);
    y += 32;

    lb = CreateLabel(hwnd, "Deadzone", sx_start, y + 5, lw, 18, hFont);
    hSliderDz = CreateSlider(hwnd, ID_SLIDER_DZ, sx_start + lw + 4, y, sw, 0, 50, 10);
    hLabelDz = CreateLabel(hwnd, "10", sx_start + lw + sw + 10, y + 5, 45, 18, hFont, accent);
    y += 32;

    lb = CreateLabel(hwnd, "Stick Radius", sx_start, y + 5, lw, 18, hFont);
    hSliderStick = CreateSlider(hwnd, ID_SLIDER_STICK, sx_start + lw + 4, y, sw, 50, 500, 400);
    hLabelStick = CreateLabel(hwnd, "400", sx_start + lw + sw + 10, y + 5, 45, 18, hFont, accent);
    y += 32;

    lb = CreateLabel(hwnd, "Bone", sx_start, y + 5, lw, 18, hFont);
    hSliderBone = CreateSlider(hwnd, ID_SLIDER_BONE, sx_start + lw + 4, y, sw, 0, 3, 0);
    hLabelBone = CreateLabel(hwnd, "Head", sx_start + lw + sw + 10, y + 5, 55, 18, hFont, accent);
    y += 40;

    HWND hSep3 = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        sx_start, y, content_w, 1, hwnd, nullptr, nullptr, nullptr);
    y += 12;

    CreateSectionHeader(hwnd, "TRIGGER", sx_start, y, 200);
    y += 24;

    lb = CreateLabel(hwnd, "Delay (ms)", sx_start, y + 5, lw, 18, hFont);
    hSliderDelay = CreateSlider(hwnd, ID_SLIDER_DELAY, sx_start + lw + 4, y, sw, 0, 200, 5);
    hLabelDelay = CreateLabel(hwnd, "5 ms", sx_start + lw + sw + 10, y + 5, 50, 18, hFont, accent);
    y += 40;

    HWND hSep4 = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        sx_start, y, content_w, 1, hwnd, nullptr, nullptr, nullptr);
    y += 12;

    CreateSectionHeader(hwnd, "SETTINGS", sx_start, y, 200);
    y += 24;

    lb = CreateLabel(hwnd, "Max Dist", sx_start, y + 5, lw, 18, hFont);
    hSliderMaxDist = CreateSlider(hwnd, ID_SLIDER_MAX_DIST, sx_start + lw + 4, y, sw, 100, 5000, 2000);
    hLabelMaxDist = CreateLabel(hwnd, "2000", sx_start + lw + sw + 10, y + 5, 45, 18, hFont, accent);
    y += 32;

    lb = CreateLabel(hwnd, "Recoil Str", sx_start, y + 5, lw, 18, hFont);
    hSliderRecoilStr = CreateSlider(hwnd, ID_SLIDER_RECOIL_STR, sx_start + lw + 4, y, sw, 0, 100, 50);
    hLabelRecoilStr = CreateLabel(hwnd, "50", sx_start + lw + sw + 10, y + 5, 45, 18, hFont, accent);
    y += 42;

    HWND hSep5 = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        sx_start, y, content_w, 1, hwnd, nullptr, nullptr, nullptr);
    y += 12;

    hLabelFps = CreateLabel(hwnd, "FPS: 0", sx_start, y, 80, 16, hFontMono);
    hLabelPlayers = CreateLabel(hwnd, "Players: 0", sx_start + 85, y, 80, 16, hFontMono);
    hLabelShots = CreateLabel(hwnd, "Shots: 0", sx_start + 180, y, 80, 16, hFontMono);
    y += 20;

    hStatus = CreateLabel(hwnd, "F1 Trigger  F2 ESP  F3 Aim  F4 Recoil  F5 HS  F6 Radar  F7 Save  F8 Load  F9 Exit",
        sx_start, y, content_w + 50, 16, hFontSmall);

    load_config("config.cfg");

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    UpdateLabels();
    UpdateButtonColors();

    return true;
}

inline void pump() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

}
