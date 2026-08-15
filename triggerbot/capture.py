"""Wrapper fino sobre mss para capturar regioes da tela como arrays BGR."""
from __future__ import annotations

import numpy as np


def grab_bgr(sct, region: dict) -> np.ndarray:
    """Captura a regiao informada e retorna um array HxWx3 em BGR (descarta o alpha)."""
    raw = sct.grab(region)
    frame = np.frombuffer(raw.bgra, dtype=np.uint8)
    return frame.reshape((region["height"], region["width"], 4))[:, :, :3]
