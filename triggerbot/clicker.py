"""Disparo de clique em thread separada, com limite de CPS e jitter opcional.

Usa uma unica thread de trabalho persistente (em vez de criar uma thread
nova a cada clique) para eliminar o overhead de criacao de thread da
latencia de reacao — o que importa quando o objetivo e resposta rapida e
consistente.
"""
from __future__ import annotations

import threading
import time

import numpy as np

from . import winapi


class Clicker:
    def __init__(self, max_cps: float, delay_ms: int = 0, jitter_ms: int = 0) -> None:
        self.max_cps = max_cps
        self.delay_ms = delay_ms
        self.jitter_ms = jitter_ms
        self.shots_fired = 0
        self._last_click = 0.0
        self._pending = threading.Event()
        self._busy = threading.Event()
        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

    def trigger(self) -> None:
        """Pede um clique, respeitando o limite de CPS.

        Ignorado silenciosamente se o limite de CPS ainda nao passou ou se
        ha um clique em andamento — o chamador nao precisa checar nada
        antes de chamar.
        """
        if time.time() - self._last_click < 1.0 / self.max_cps:
            return
        if self._busy.is_set():
            return
        self._pending.set()

    def _loop(self) -> None:
        while True:
            self._pending.wait()
            self._pending.clear()
            self._busy.set()
            try:
                if self.delay_ms > 0 or self.jitter_ms > 0:
                    delay = (self.delay_ms + np.random.randint(0, max(1, self.jitter_ms))) / 1000
                    time.sleep(delay)
                self._last_click = time.time()
                self.shots_fired += 1
                winapi.click()
            finally:
                self._busy.clear()
