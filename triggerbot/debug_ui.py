"""Janela de debug: mostra a mascara HSV, a zona de disparo e um HUD com status."""
from __future__ import annotations

import cv2
import numpy as np

from .detection import ColorRange, build_mask

WINDOW_NAME = "TriggerBot Debug  [F2 = fechar]"


def draw(
    frame_bgr: np.ndarray,
    zone_radius_px: int,
    primary: ColorRange,
    high_red: ColorRange,
    hit: bool,
    active: bool,
    shots_fired: int,
    fps: int,
    max_cps: float,
) -> None:
    hsv = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2HSV)
    mask = build_mask(hsv, primary, high_red)
    vis = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
    h, w = vis.shape[:2]
    cx, cy = w // 2, h // 2
    r = zone_radius_px

    zone_color = (0, 0, 255) if hit else (0, 200, 255)
    cv2.rectangle(vis, (cx - r, cy - r), (cx + r, cy + r), zone_color, 1)
    cv2.drawMarker(vis, (cx, cy), (255, 255, 255), cv2.MARKER_CROSS, 14, 1)

    overlay = vis.copy()
    cv2.rectangle(overlay, (0, 0), (w, 78), (20, 20, 20), -1)
    cv2.addWeighted(overlay, 0.55, vis, 0.45, 0, vis)

    def put(text: str, row: int, color=(200, 200, 200)) -> None:
        cv2.putText(vis, text, (8, 16 + row * 18), cv2.FONT_HERSHEY_SIMPLEX, 0.50, color, 1, cv2.LINE_AA)

    put(f"TRIGGER: {'ON' if active else 'OFF'}", 0, (0, 255, 80) if active else (80, 80, 80))
    put(f"ALVO: {'DETECTADO!' if hit else 'nenhum'}", 1, (0, 0, 255) if hit else (120, 120, 120))
    put(f"Tiros: {shots_fired}   FPS: {fps}", 2)
    put(f"Zona: {zone_radius_px}px  CPS max: {max_cps}", 3)

    cv2.imshow(WINDOW_NAME, vis)
    cv2.waitKey(1)


def close() -> None:
    cv2.destroyAllWindows()
