"""Deteccao de alvo por cor HSV.

Modulo com apenas funcoes puras (numpy/OpenCV) — sem dependencia de captura de
tela ou WinAPI — para permitir testes unitarios com frames sinteticos em
qualquer sistema operacional.
"""
from __future__ import annotations

from typing import NamedTuple, Optional, Tuple

import cv2
import numpy as np

HSV = Tuple[int, int, int]


class ColorRange(NamedTuple):
    lower: np.ndarray
    upper: np.ndarray

    @classmethod
    def from_tuples(cls, lower: HSV, upper: HSV) -> "ColorRange":
        return cls(np.array(lower, dtype=np.uint8), np.array(upper, dtype=np.uint8))


def build_mask(hsv_frame: np.ndarray, primary: ColorRange, high_red: Optional[ColorRange] = None) -> np.ndarray:
    """Combina a mascara da faixa calibrada com a faixa extra de vermelho alto (H 165-180)."""
    mask = cv2.inRange(hsv_frame, primary.lower, primary.upper)
    if high_red is not None:
        mask = cv2.bitwise_or(mask, cv2.inRange(hsv_frame, high_red.lower, high_red.upper))
    return mask


def count_target_pixels(frame_bgr: np.ndarray, primary: ColorRange, high_red: Optional[ColorRange] = None) -> int:
    hsv = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2HSV)
    return int(cv2.countNonZero(build_mask(hsv, primary, high_red)))


def crop_zone(frame_bgr: np.ndarray, zone_radius_px: int) -> np.ndarray:
    """Recorta uma ROI quadrada centrada no frame (assume a mira no centro da captura)."""
    h, w = frame_bgr.shape[:2]
    cx, cy = w // 2, h // 2
    r = zone_radius_px
    y1, y2 = max(0, cy - r), min(h, cy + r)
    x1, x2 = max(0, cx - r), min(w, cx + r)
    return frame_bgr[y1:y2, x1:x2]


def in_trigger_zone(
    frame_bgr: np.ndarray,
    zone_radius_px: int,
    primary: ColorRange,
    high_red: Optional[ColorRange],
    min_pixels: int,
) -> bool:
    """True se ha pixels de alvo suficientes na zona de disparo ao redor do centro do frame."""
    roi = crop_zone(frame_bgr, zone_radius_px)
    if roi.size == 0:
        return False
    return count_target_pixels(roi, primary, high_red) >= min_pixels


def calibrate_hsv(patch_bgr: np.ndarray, hue_tolerance: int = 14) -> Optional[ColorRange]:
    """Deriva uma faixa HSV a partir da mediana dos pixels vibrantes (S/V > 80) do patch.

    Retorna None se nenhum pixel vibrante for encontrado (usuario nao mirou num alvo).
    """
    hsv = cv2.cvtColor(patch_bgr, cv2.COLOR_BGR2HSV).reshape(-1, 3)
    valid = hsv[(hsv[:, 1] > 80) & (hsv[:, 2] > 80)]
    if len(valid) == 0:
        return None

    h, s, v = (int(np.median(valid[:, i])) for i in range(3))
    lower = np.array([max(0, h - hue_tolerance), max(0, s - 70), max(50, v - 90)], dtype=np.uint8)
    upper = np.array([min(180, h + hue_tolerance), 255, 255], dtype=np.uint8)
    return ColorRange(lower, upper)
