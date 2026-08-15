"""Wrappers finos sobre a WinAPI: clique via SendInput, estado de teclas e tamanho de tela.

So funciona no Windows (usa ctypes.windll). Mantido isolado dos modulos de
deteccao para que estes possam ser testados em qualquer sistema operacional.
"""
from __future__ import annotations

import ctypes

user32 = ctypes.windll.user32
kernel32 = ctypes.windll.kernel32


class _MOUSEINPUT(ctypes.Structure):
    _fields_ = [
        ("dx", ctypes.c_long),
        ("dy", ctypes.c_long),
        ("mouseData", ctypes.c_ulong),
        ("dwFlags", ctypes.c_ulong),
        ("time", ctypes.c_ulong),
        ("dwExtraInfo", ctypes.POINTER(ctypes.c_ulong)),
    ]


class _INPUT_UNION(ctypes.Union):
    _fields_ = [("mi", _MOUSEINPUT)]


class _INPUT(ctypes.Structure):
    _anonymous_ = ("_u",)
    _fields_ = [("type", ctypes.c_ulong), ("_u", _INPUT_UNION)]


_MOUSEEVENTF_LEFTDOWN = 0x0002
_MOUSEEVENTF_LEFTUP = 0x0004
_HIGH_PRIORITY_CLASS = 0x00008000

KEY_TOGGLE = 0x70     # F1
KEY_DEBUG = 0x71      # F2
KEY_CALIBRATE = 0x72  # F3
KEY_EXIT = 0x73       # F4


def click() -> None:
    """Clique esquerdo direto via SendInput — mais rapido que win32api."""
    down = _INPUT(type=0)
    down.mi = _MOUSEINPUT(dwFlags=_MOUSEEVENTF_LEFTDOWN)
    up = _INPUT(type=0)
    up.mi = _MOUSEINPUT(dwFlags=_MOUSEEVENTF_LEFTUP)
    arr = (_INPUT * 2)(down, up)
    user32.SendInput(2, arr, ctypes.sizeof(_INPUT))


def key_down(vk: int) -> bool:
    return bool(user32.GetAsyncKeyState(vk) & 0x8000)


def screen_size() -> tuple[int, int]:
    return user32.GetSystemMetrics(0), user32.GetSystemMetrics(1)


def raise_process_priority() -> None:
    """Aumenta a prioridade do processo para reduzir latencia (best-effort)."""
    try:
        kernel32.SetPriorityClass(kernel32.GetCurrentProcess(), _HIGH_PRIORITY_CLASS)
    except OSError:
        pass
