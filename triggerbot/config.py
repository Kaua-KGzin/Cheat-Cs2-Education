"""Configuracao do bot: carregada de config.json na raiz do projeto, com defaults embutidos.

Editar config.json (em vez de constantes no codigo) e a forma recomendada de ajustar o bot.
"""
from __future__ import annotations

import json
import logging
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Tuple

logger = logging.getLogger(__name__)

HSV = Tuple[int, int, int]

DEFAULT_CONFIG_PATH = Path(__file__).resolve().parent.parent / "config.json"


@dataclass
class Config:
    # Tamanho da janela de captura ao redor do centro da tela (pixels).
    # Menor = mais rapido. Recomendado: 160-300.
    capture_size: int = 200

    # Zona de disparo: raio em pixels ao redor do crosshair. Menor = mais preciso.
    trigger_zone_px: int = 12

    # Delay antes de clicar (ms). 0 = instantaneo.
    click_delay_ms: int = 0

    # Delay aleatorio adicional para humanizar (ms).
    click_jitter_ms: int = 15

    # Maximo de cliques por segundo (evita spam).
    max_cps: int = 18

    # Minimo de pixels de alvo na zona para considerar deteccao valida (anti-ruido).
    min_target_pixels: int = 8

    # Faixa HSV calibrada para a cor do alvo. Ajustada automaticamente com F3.
    hsv_lower: HSV = (0, 160, 160)
    hsv_upper: HSV = (22, 255, 255)

    # Faixa extra fixa para vermelho alto (H 165-180), que "quebra" no espaco HSV.
    high_red_lower: HSV = (165, 150, 100)
    high_red_upper: HSV = (180, 255, 255)

    # Tolerancia de matiz (H) usada pela calibracao (F3) ao redor da mediana detectada.
    hue_tolerance: int = 14

    @classmethod
    def load(cls, path: Path = DEFAULT_CONFIG_PATH) -> "Config":
        """Carrega config.json; cria o arquivo com defaults se ele nao existir."""
        if not path.exists():
            cfg = cls()
            cfg.save(path)
            return cfg

        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            logger.warning("Falha ao ler %s (%s); usando configuracao padrao.", path, exc)
            return cls()

        defaults = asdict(cls())
        unknown = set(data) - set(defaults)
        if unknown:
            logger.warning("Ignorando chaves desconhecidas em %s: %s", path, sorted(unknown))

        merged = {**defaults, **{k: v for k, v in data.items() if k in defaults}}
        merged = {k: (tuple(v) if isinstance(v, list) else v) for k, v in merged.items()}
        return cls(**merged)

    def save(self, path: Path = DEFAULT_CONFIG_PATH) -> None:
        path.write_text(json.dumps(asdict(self), indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
