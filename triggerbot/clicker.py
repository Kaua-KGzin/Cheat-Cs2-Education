"""Disparo de clique em thread separada, com limite de CPS e jitter opcional."""
from __future__ import annotations

import threading
import time
from typing import Optional

import numpy as np

from . import winapi


class Clicker:
    def __init__(self, max_cps: float, delay_ms: int = 0, jitter_ms: int = 0) -> None:
        self.max_cps = max_cps
        self.delay_ms = delay_ms
        self.jitter_ms = jitter_ms
        self.shots_fired = 0
        self._last_click = 0.0
        self._thread: Optional[threading.Thread] = None

    def trigger(self) -> None:
        """Dispara um clique em background, respeitando o limite de CPS.

        Ignorado silenciosamente se o limite de CPS ainda nao passou ou se ha
        um clique em andamento — o chamador nao precisa checar nada antes.
        """
        if time.time() - self._last_click < 1.0 / self.max_cps:
            return
        if self._thread and self._thread.is_alive():
            return
        self._thread = threading.Thread(target=self._worker, daemon=True)
        self._thread.start()

    def _worker(self) -> None:
        if self.delay_ms > 0 or self.jitter_ms > 0:
            delay = (self.delay_ms + np.random.randint(0, max(1, self.jitter_ms))) / 1000
            time.sleep(delay)
        self._last_click = time.time()
        self.shots_fired += 1
        winapi.click()
