"""
╔══════════════════════════════════════════════════╗
║         AimLab TriggerBot  v3                    ║
║──────────────────────────────────────────────────║
║  F1  → Liga/Desliga trigger                      ║
║  F2  → Abre janela de debug                      ║
║  F3  → Calibrar cor (mire num alvo e aperte)     ║
║  F4  → Sair                                      ║
╚══════════════════════════════════════════════════╝
"""

import ctypes, threading, time, sys
import numpy as np
import cv2
import mss

# ══════════════════════════════════════════════════
#  CONFIGURACOES — ajuste conforme necessario
# ══════════════════════════════════════════════════

# Tamanho da janela de captura ao redor do centro da tela (pixels)
# Menor = mais rapido. Recomendado: 160–300
CAPTURE_SIZE = 200

# Zona de disparo: raio em pixels ao redor do crosshair.
# Se alvo estiver dentro, clica. Menor = mais preciso.
TRIGGER_ZONE_PX = 12

# Delay antes de clicar (ms). 0 = instantaneo.
CLICK_DELAY_MS = 0

# Delay aleatorio adicional para humanizar (ms)
CLICK_JITTER_MS = 15

# Maximo de cliques por segundo (evita spam)
MAX_CPS = 18

# Minimo de pixels brancos na zona para considerar alvo real (anti-ruido)
MIN_WHITE_PIXELS = 8

# ── Cor dos alvos (HSV) ──────────────────────────
# Aim Lab usa alvos laranjas brilhantes por padrao.
# Use F3 para calibrar automaticamente.
COLOR_LOWER = np.array([0,   160, 160], dtype=np.uint8)
COLOR_UPPER = np.array([22,  255, 255], dtype=np.uint8)

# ══════════════════════════════════════════════════

# ── WinAPI setup ────────────────────────────────
user32   = ctypes.windll.user32
kernel32 = ctypes.windll.kernel32

class MOUSEINPUT(ctypes.Structure):
    _fields_ = [
        ("dx",          ctypes.c_long),
        ("dy",          ctypes.c_long),
        ("mouseData",   ctypes.c_ulong),
        ("dwFlags",     ctypes.c_ulong),
        ("time",        ctypes.c_ulong),
        ("dwExtraInfo", ctypes.POINTER(ctypes.c_ulong)),
    ]

class _INPUT_UNION(ctypes.Union):
    _fields_ = [("mi", MOUSEINPUT)]

class INPUT(ctypes.Structure):
    _anonymous_ = ("_u",)
    _fields_    = [("type", ctypes.c_ulong), ("_u", _INPUT_UNION)]

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP   = 0x0004

def _raw_click():
    """Clique direto via SendInput — mais rapido que win32api."""
    down      = INPUT(type=0)
    down.mi   = MOUSEINPUT(dwFlags=MOUSEEVENTF_LEFTDOWN)
    up        = INPUT(type=0)
    up.mi     = MOUSEINPUT(dwFlags=MOUSEEVENTF_LEFTUP)
    arr       = (INPUT * 2)(down, up)
    user32.SendInput(2, arr, ctypes.sizeof(INPUT))

def key_down(vk: int) -> bool:
    return bool(user32.GetAsyncKeyState(vk) & 0x8000)

def screen_wh():
    return user32.GetSystemMetrics(0), user32.GetSystemMetrics(1)

# ── Estado global ────────────────────────────────
active       = False
show_debug   = False
last_click   = 0.0
click_thread = None
shots_fired  = 0
shots_hit    = 0    # aproximado: cada trigger = potencial hit
fps_counter  = 0
fps_display  = 0
fps_timer    = time.time()

col_lower = COLOR_LOWER.copy()
col_upper = COLOR_UPPER.copy()

_dbnc     = {}
DBNC_T    = 0.20

KEY_TOGGLE    = 0x70   # F1
KEY_DEBUG     = 0x71   # F2
KEY_CALIBRATE = 0x72   # F3
KEY_EXIT      = 0x73   # F4

# ── Teclas com debounce ──────────────────────────
def pressed(vk):
    now = time.time()
    if key_down(vk) and now - _dbnc.get(vk, 0) > DBNC_T:
        _dbnc[vk] = now
        return True
    return False

# ── Calibracao ──────────────────────────────────
def calibrate(sct, cx, cy):
    """Captura 11x11 pixels ao redor do centro e deriva o range HSV."""
    global col_lower, col_upper
    r = {"left": cx-5, "top": cy-5, "width": 11, "height": 11}
    raw   = sct.grab(r)
    patch = np.frombuffer(raw.bgra, dtype=np.uint8).reshape(11, 11, 4)[:, :, :3]
    hsv   = cv2.cvtColor(patch, cv2.COLOR_BGR2HSV).reshape(-1, 3)

    # Ignora pixels muito escuros ou pouco saturados (fundo)
    valid = hsv[(hsv[:, 1] > 80) & (hsv[:, 2] > 80)]
    if len(valid) == 0:
        print("  [CALIBRAR] Nenhuma cor vibrante encontrada — aponte para um alvo!")
        return

    H = int(np.median(valid[:, 0]))
    S = int(np.median(valid[:, 1]))
    V = int(np.median(valid[:, 2]))

    hr = 14   # tolerancia de hue
    col_lower = np.array([max(0,   H-hr), max(0,  S-70), max(50, V-90)], dtype=np.uint8)
    col_upper = np.array([min(180, H+hr), 255,            255          ], dtype=np.uint8)
    print(f"  [CALIBRADO] HSV median=({H},{S},{V})  lower={col_lower}  upper={col_upper}")

# ── Deteccao de alvo ─────────────────────────────
def in_trigger_zone(frame: np.ndarray) -> bool:
    """
    Verifica se existe cor de alvo dentro da zona de disparo.
    Opera diretamente em uma ROI minima — muito rapido.
    """
    h, w = frame.shape[:2]
    cx,  cy  = w // 2, h // 2
    r        = TRIGGER_ZONE_PX

    # ROI quadrada ao redor do centro
    y1 = max(0,  cy - r)
    y2 = min(h,  cy + r)
    x1 = max(0,  cx - r)
    x2 = min(w,  cx + r)
    roi = frame[y1:y2, x1:x2]

    if roi.size == 0:
        return False

    hsv  = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, col_lower, col_upper)

    # Segundo range para vermelho alto (H 165-180)
    mask2 = cv2.inRange(hsv,
                        np.array([165, 150, 100], dtype=np.uint8),
                        np.array([180, 255, 255], dtype=np.uint8))
    total = int(cv2.countNonZero(mask)) + int(cv2.countNonZero(mask2))

    return total >= MIN_WHITE_PIXELS

def full_mask(frame: np.ndarray) -> np.ndarray:
    """Mascara HSV completa do frame — usada so no debug."""
    hsv  = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    m1   = cv2.inRange(hsv, col_lower, col_upper)
    m2   = cv2.inRange(hsv,
                       np.array([165, 150, 100], dtype=np.uint8),
                       np.array([180, 255, 255], dtype=np.uint8))
    return cv2.bitwise_or(m1, m2)

# ── Click em thread separada ─────────────────────
def do_click():
    global last_click, shots_fired, click_thread

    if time.time() - last_click < 1.0 / MAX_CPS:
        return
    if click_thread and click_thread.is_alive():
        return

    def _worker():
        global last_click, shots_fired
        if CLICK_DELAY_MS > 0 or CLICK_JITTER_MS > 0:
            delay = (CLICK_DELAY_MS + np.random.randint(0, max(1, CLICK_JITTER_MS))) / 1000
            time.sleep(delay)
        last_click  = time.time()
        shots_fired += 1
        _raw_click()

    click_thread = threading.Thread(target=_worker, daemon=True)
    click_thread.start()

# ── Janela de debug ──────────────────────────────
WIN = "TriggerBot Debug  [F2 = fechar]"

def draw_debug(frame: np.ndarray, hit: bool, fps: int):
    mask = full_mask(frame)
    vis  = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
    h, w = vis.shape[:2]
    cx,  cy  = w // 2, h // 2
    r        = TRIGGER_ZONE_PX

    # Zona de disparo
    color_zone = (0, 0, 255) if hit else (0, 200, 255)
    cv2.rectangle(vis, (cx-r, cy-r), (cx+r, cy+r), color_zone, 1)
    cv2.drawMarker(vis, (cx, cy), (255, 255, 255), cv2.MARKER_CROSS, 14, 1)

    # HUD — caixa semi-transparente no topo
    overlay = vis.copy()
    cv2.rectangle(overlay, (0, 0), (w, 78), (20, 20, 20), -1)
    cv2.addWeighted(overlay, 0.55, vis, 0.45, 0, vis)

    def put(txt, row, color=(200, 200, 200)):
        cv2.putText(vis, txt, (8, 16 + row * 18),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.50, color, 1, cv2.LINE_AA)

    trig_col = (0, 255, 80) if active else (80, 80, 80)
    put(f"TRIGGER: {'ON' if active else 'OFF'}",         0, trig_col)
    put(f"ALVO: {'DETECTADO!' if hit else 'nenhum'}",   1, (0,0,255) if hit else (120,120,120))
    put(f"Tiros: {shots_fired}   FPS: {fps}",           2)
    put(f"Zona: {TRIGGER_ZONE_PX}px  CPS max: {MAX_CPS}", 3)

    cv2.imshow(WIN, vis)
    cv2.waitKey(1)

# ══════════════════════════════════════════════════
#   LOOP PRINCIPAL
# ══════════════════════════════════════════════════
def main():
    global active, show_debug, fps_counter, fps_display, fps_timer

    sw, sh = screen_wh()
    cx,  cy  = sw // 2, sh // 2
    half     = CAPTURE_SIZE // 2
    region   = {"left": cx - half, "top": cy - half,
                "width": CAPTURE_SIZE, "height": CAPTURE_SIZE}

    print("╔══════════════════════════════════════════════════╗")
    print("║         AimLab TriggerBot  v3                    ║")
    print("╠══════════════════════════════════════════════════╣")
    print("║  F1  Liga/Desliga trigger                        ║")
    print("║  F2  Debug (ver o que o bot enxerga)             ║")
    print("║  F3  Calibrar cor (mire num alvo e aperte F3)    ║")
    print("║  F4  Sair                                        ║")
    print("╚══════════════════════════════════════════════════╝\n")
    print("  DICA: Abra o Aim Lab, entre num treino e aperte")
    print("  F2 para ver o debug, depois F3 mirando no alvo.\n")

    prev_debug = False

    with mss.mss() as sct:
        while True:
            t0 = time.perf_counter()

            # ── Teclas ──────────────────────────
            if pressed(KEY_EXIT):
                print("Saindo.")
                break

            if pressed(KEY_TOGGLE):
                active = not active
                print(f"  Trigger: {'ON ✓' if active else 'OFF'}")

            if pressed(KEY_DEBUG):
                show_debug = not show_debug
                print(f"  Debug: {'ON' if show_debug else 'OFF'}")

            if pressed(KEY_CALIBRATE):
                calibrate(sct, cx, cy)

            # Fecha janela se debug foi desligado
            if prev_debug and not show_debug:
                cv2.destroyAllWindows()
            prev_debug = show_debug

            # ── Captura ─────────────────────────
            raw   = sct.grab(region)
            frame = np.frombuffer(raw.bgra, dtype=np.uint8)
            frame = frame.reshape((CAPTURE_SIZE, CAPTURE_SIZE, 4))[:, :, :3]

            # ── Deteccao ────────────────────────
            hit = in_trigger_zone(frame)

            # ── Disparo ─────────────────────────
            if active and hit:
                do_click()

            # ── Debug ───────────────────────────
            fps_counter += 1
            now = time.time()
            if now - fps_timer >= 1.0:
                fps_display = fps_counter
                fps_counter = 0
                fps_timer   = now

            if show_debug:
                draw_debug(frame, hit, fps_display)

            # ── ~500fps cap (bem acima de qualquer monitor) ──
            elapsed = time.perf_counter() - t0
            wait    = max(0.0, 0.002 - elapsed)
            if wait:
                time.sleep(wait)

    cv2.destroyAllWindows()


if __name__ == "__main__":
    # Aumenta prioridade do processo para latencia menor
    try:
        kernel32.SetPriorityClass(kernel32.GetCurrentProcess(), 0x00008000)  # HIGH_PRIORITY
    except Exception:
        pass
    main()
