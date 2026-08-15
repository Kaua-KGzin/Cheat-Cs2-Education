"""Ponto de entrada do bot. A logica real vive no pacote `triggerbot/`.

Mantido como arquivo separado para compatibilidade com `rodar.bat` /
`setup_e_rodar.bat` (`python trigger.py`).
"""
from triggerbot.app import run

if __name__ == "__main__":
    run()
