"""Testes da logica de deteccao (triggerbot.detection) com frames sinteticos.

Nao dependem de captura de tela nem de WinAPI, entao rodam em qualquer SO:
    pip install numpy opencv-python pytest
    pytest
"""
import numpy as np

from triggerbot.detection import ColorRange, calibrate_hsv, in_trigger_zone

ORANGE_BGR = (0, 140, 255)  # laranja vibrante (cor padrao de alvo no Aim Lab)
GRAY_BGR = (60, 60, 60)     # fundo neutro, pouco saturado

DEFAULT_RANGE = ColorRange.from_tuples((0, 160, 160), (22, 255, 255))
HIGH_RED = ColorRange.from_tuples((165, 150, 100), (180, 255, 255))


def make_frame(size: int, fg_bgr, fg_radius: int, bg_bgr=GRAY_BGR) -> np.ndarray:
    """Frame com um circulo de cor `fg_bgr` centrado sobre um fundo `bg_bgr`."""
    frame = np.full((size, size, 3), bg_bgr, dtype=np.uint8)
    cx = cy = size // 2
    y, x = np.ogrid[:size, :size]
    circle = (x - cx) ** 2 + (y - cy) ** 2 <= fg_radius ** 2
    frame[circle] = fg_bgr
    return frame


def test_detects_target_in_zone():
    frame = make_frame(size=40, fg_bgr=ORANGE_BGR, fg_radius=6)
    assert in_trigger_zone(frame, zone_radius_px=12, primary=DEFAULT_RANGE, high_red=HIGH_RED, min_pixels=8)


def test_no_false_positive_on_background_only():
    frame = make_frame(size=40, fg_bgr=ORANGE_BGR, fg_radius=0)
    assert not in_trigger_zone(frame, zone_radius_px=12, primary=DEFAULT_RANGE, high_red=HIGH_RED, min_pixels=8)


def test_target_outside_zone_is_ignored():
    size = 60
    frame = np.full((size, size, 3), GRAY_BGR, dtype=np.uint8)
    frame[0:8, 0:8] = ORANGE_BGR  # canto da captura, longe do centro/crosshair
    assert not in_trigger_zone(frame, zone_radius_px=10, primary=DEFAULT_RANGE, high_red=HIGH_RED, min_pixels=4)


def test_min_pixels_threshold_filters_noise():
    frame = make_frame(size=40, fg_bgr=ORANGE_BGR, fg_radius=1)  # poucos pixels de alvo
    assert not in_trigger_zone(frame, zone_radius_px=12, primary=DEFAULT_RANGE, high_red=HIGH_RED, min_pixels=50)


def test_calibrate_returns_range_for_vibrant_patch():
    patch = np.full((11, 11, 3), ORANGE_BGR, dtype=np.uint8)
    result = calibrate_hsv(patch)
    assert result is not None
    assert (result.lower <= result.upper).all()


def test_calibrate_returns_none_for_dull_patch():
    patch = np.full((11, 11, 3), GRAY_BGR, dtype=np.uint8)
    assert calibrate_hsv(patch) is None
