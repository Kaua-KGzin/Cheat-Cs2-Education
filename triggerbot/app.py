"""Loop principal do TriggerBot: teclas de atalho, captura, deteccao e clique."""
from __future__ import annotations

import logging
import time
from pathlib import Path
from typing import Dict

from . import debug_ui, winapi
from .capture import grab_bgr
from .clicker import Clicker
from .config import Config
from .detection import ColorRange, calibrate_hsv, in_trigger_zone

logger = logging.getLogger(__name__)

BANNER = """
+====================================================+
|         AimLab TriggerBot  v4                      |
+------------------------------------------------------+
|  F1  Liga/Desliga trigger                           |
|  F2  Debug (ver o que o bot enxerga)                |
|  F3  Calibrar cor (mire num alvo e aperte F3)        |
|  F4  Sair                                            |
+====================================================+
"""

_DEBOUNCE_SECONDS = 0.20
_LOOP_BUDGET_SECONDS = 0.002  # cap de ~500 fps, bem acima de qualquer monitor
_ERROR_BACKOFF_SECONDS = 0.5


class _KeyDebouncer:
    def __init__(self) -> None:
        self._last_press: Dict[int, float] = {}

    def pressed(self, vk: int) -> bool:
        now = time.time()
        if winapi.key_down(vk) and now - self._last_press.get(vk, 0) > _DEBOUNCE_SECONDS:
            self._last_press[vk] = now
            return True
        return False


class TriggerBotApp:
    def __init__(self, config: Config) -> None:
        self.config = config
        self.active = False
        self.show_debug = False
        self.primary = ColorRange.from_tuples(config.hsv_lower, config.hsv_upper)
        self.high_red = ColorRange.from_tuples(config.high_red_lower, config.high_red_upper)
        self.clicker = Clicker(config.max_cps, config.click_delay_ms, config.click_jitter_ms)
        self._keys = _KeyDebouncer()
        self._fps_counter = 0
        self._fps_display = 0
        self._fps_timer = time.time()

    def calibrate(self, sct, cx: int, cy: int) -> None:
        region = {"left": cx - 5, "top": cy - 5, "width": 11, "height": 11}
        patch = grab_bgr(sct, region)
        result = calibrate_hsv(patch, self.config.hue_tolerance)
        if result is None:
            print("  [CALIBRAR] Nenhuma cor vibrante encontrada -- aponte para um alvo!")
            return
        self.primary = result
        print(f"  [CALIBRADO] lower={result.lower.tolist()}  upper={result.upper.tolist()}")

    def _update_fps(self) -> None:
        self._fps_counter += 1
        now = time.time()
        if now - self._fps_timer >= 1.0:
            self._fps_display = self._fps_counter
            self._fps_counter = 0
            self._fps_timer = now

    def _handle_keys(self, sct, cx: int, cy: int, prev_debug: bool) -> bool:
        if self._keys.pressed(winapi.KEY_TOGGLE):
            self.active = not self.active
            print(f"  Trigger: {'ON' if self.active else 'OFF'}")

        if self._keys.pressed(winapi.KEY_DEBUG):
            self.show_debug = not self.show_debug
            print(f"  Debug: {'ON' if self.show_debug else 'OFF'}")

        if self._keys.pressed(winapi.KEY_CALIBRATE):
            self.calibrate(sct, cx, cy)

        if prev_debug and not self.show_debug:
            debug_ui.close()
        return self.show_debug

    def run(self) -> None:
        import mss

        winapi.raise_process_priority()
        sw, sh = winapi.screen_size()
        cx, cy = sw // 2, sh // 2
        half = self.config.capture_size // 2
        region = {
            "left": cx - half,
            "top": cy - half,
            "width": self.config.capture_size,
            "height": self.config.capture_size,
        }

        print(BANNER)
        print("  DICA: Abra o Aim Lab, entre num treino e aperte")
        print("  F2 para ver o debug, depois F3 mirando no alvo.\n")

        prev_debug = False

        with mss.mss() as sct:
            while True:
                t0 = time.perf_counter()
                try:
                    if self._keys.pressed(winapi.KEY_EXIT):
                        print("Saindo.")
                        break

                    prev_debug = self._handle_keys(sct, cx, cy, prev_debug)

                    frame = grab_bgr(sct, region)
                    hit = in_trigger_zone(
                        frame,
                        self.config.trigger_zone_px,
                        self.primary,
                        self.high_red,
                        self.config.min_target_pixels,
                    )

                    if self.active and hit:
                        self.clicker.trigger()

                    self._update_fps()

                    if self.show_debug:
                        debug_ui.draw(
                            frame,
                            self.config.trigger_zone_px,
                            self.primary,
                            self.high_red,
                            hit,
                            self.active,
                            self.clicker.shots_fired,
                            self._fps_display,
                            self.config.max_cps,
                        )
                except Exception:
                    logger.exception("Erro no loop principal; tentando continuar em %.1fs.", _ERROR_BACKOFF_SECONDS)
                    time.sleep(_ERROR_BACKOFF_SECONDS)
                    continue

                elapsed = time.perf_counter() - t0
                wait = max(0.0, _LOOP_BUDGET_SECONDS - elapsed)
                if wait:
                    time.sleep(wait)

        debug_ui.close()


def run() -> None:
    log_path = Path(__file__).resolve().parent.parent / "triggerbot.log"
    logging.basicConfig(
        filename=str(log_path),
        level=logging.WARNING,
        format="%(asctime)s %(levelname)s %(name)s: %(message)s",
    )
    config = Config.load()
    TriggerBotApp(config).run()
