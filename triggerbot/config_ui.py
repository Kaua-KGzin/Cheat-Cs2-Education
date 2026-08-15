"""Interface grafica (Tkinter) para editar config.json sem mexer em codigo.

Usa apenas a biblioteca padrao do Python — nenhuma dependencia nova.
"""
from __future__ import annotations

import tkinter as tk
from tkinter import messagebox, ttk
from typing import Callable, Dict, Tuple

from .config import Config

# (chave, rotulo, minimo, maximo, ajuda)
_FIELDS: Tuple[Tuple[str, str, int, int, str], ...] = (
    ("capture_size", "Tamanho da captura (px)", 60, 800, "Menor = mais rapido; maior = mais contexto"),
    ("trigger_zone_px", "Raio da zona de disparo (px)", 1, 200, "Menor = mais preciso"),
    ("click_delay_ms", "Delay antes do clique (ms)", 0, 1000, "0 = instantaneo"),
    ("click_jitter_ms", "Jitter aleatorio (ms)", 0, 1000, "Reduz padrao de tempo fixo"),
    ("max_cps", "Cliques maximos por segundo", 1, 60, "Evita spam de clique"),
    ("min_target_pixels", "Minimo de pixels do alvo", 1, 2000, "Maior = menos falso positivo"),
    ("hue_tolerance", "Tolerancia de matiz (calibracao F3)", 1, 90, "Maior = calibracao mais abrangente"),
)


class ConfigEditor(ttk.Frame):
    def __init__(self, master: tk.Tk, config: Config, on_save: Callable[[Config], None]) -> None:
        super().__init__(master, padding=16)
        self.config_obj = config
        self._on_save = on_save
        self._vars: Dict[str, tk.IntVar] = {}
        self.grid(sticky="nsew")
        self._build_form()

    def _build_form(self) -> None:
        ttk.Label(self, text="AimLab TriggerBot — Configuracao", font=("Segoe UI", 13, "bold")).grid(
            row=0, column=0, columnspan=3, sticky="w", pady=(0, 12)
        )

        for row, (key, label, lo, hi, help_text) in enumerate(_FIELDS, start=1):
            value = getattr(self.config_obj, key)
            var = tk.IntVar(value=value)
            self._vars[key] = var

            ttk.Label(self, text=label).grid(row=row, column=0, sticky="w", pady=4)
            ttk.Spinbox(self, from_=lo, to=hi, textvariable=var, width=8).grid(
                row=row, column=1, sticky="w", padx=(8, 8), pady=4
            )
            ttk.Label(self, text=help_text, foreground="#666666").grid(row=row, column=2, sticky="w", pady=4)

        info_row = len(_FIELDS) + 1
        ttk.Label(
            self,
            text="A cor do alvo (HSV) e calibrada em tempo real com F3 durante o uso do bot,\n"
            "por isso nao aparece aqui.",
            foreground="#666666",
            justify="left",
        ).grid(row=info_row, column=0, columnspan=3, sticky="w", pady=(12, 12))

        button_row = info_row + 1
        buttons = ttk.Frame(self)
        buttons.grid(row=button_row, column=0, columnspan=3, sticky="e")
        ttk.Button(buttons, text="Restaurar padrao", command=self._restore_defaults).pack(side="left", padx=(0, 8))
        ttk.Button(buttons, text="Salvar", command=self._save).pack(side="left")

    def _restore_defaults(self) -> None:
        defaults = Config()
        for key, var in self._vars.items():
            var.set(getattr(defaults, key))

    def _save(self) -> None:
        try:
            values = {key: var.get() for key, var in self._vars.items()}
        except tk.TclError:
            messagebox.showerror("Configuracao invalida", "Todos os campos precisam ser numeros inteiros.")
            return

        for key, label, lo, hi, _ in _FIELDS:
            v = values[key]
            if not (lo <= v <= hi):
                messagebox.showerror(
                    "Configuracao invalida", f"'{label}' deve estar entre {lo} e {hi} (valor atual: {v})."
                )
                return

        for key, value in values.items():
            setattr(self.config_obj, key, value)

        self.config_obj.save()
        self._on_save(self.config_obj)
        messagebox.showinfo("Configuracao salva", "config.json atualizado com sucesso.")


def run() -> None:
    config = Config.load()
    root = tk.Tk()
    root.title("AimLab TriggerBot — Configuracao")
    root.resizable(False, False)
    ConfigEditor(root, config, on_save=lambda _cfg: None)
    root.mainloop()


if __name__ == "__main__":
    run()
