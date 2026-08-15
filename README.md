# AimLab TriggerBot v4

Projeto educacional para estudar:
- captura de tela em alta frequencia;
- deteccao de cor em HSV com OpenCV;
- automacao de clique via WinAPI (`SendInput`);
- impacto de latencia, ruido e calibracao.

## Aviso de uso

Feito para uso com o **Aim Lab** (treino solo, sem partidas competitivas contra
outros jogadores). Nao use este bot em jogos multiplayer competitivos — a
maioria viola os termos de servico e pode resultar em banimento.

Uso apenas para estudo e testes em ambiente controlado. Qualquer uso ou
modificacao do codigo fora desse escopo e de total responsabilidade de quem
executa/altera o projeto.

## O que o bot faz

1. Captura uma area pequena no centro da tela (`capture_size`).
2. Analisa somente a zona perto da mira (`trigger_zone_px`).
3. Detecta a cor alvo em HSV (com calibracao por `F3`).
4. Se o trigger estiver ligado (`F1`) e houver alvo na zona, envia clique.
5. Limita cliques por segundo (`max_cps`) para evitar spam.

## Requisitos

- Windows 10/11
- Python 3.9+ (recomendado 3.10+)
- `pip` no PATH

Dependencias usadas (ver `requirements.txt`):
- `mss`
- `numpy`
- `opencv-python`
- `pywin32`

## Arquivos do projeto

- `trigger.py`: ponto de entrada (`python trigger.py`) — mantido por compatibilidade com os `.bat`
- `triggerbot/`: pacote com a logica real, dividida por responsabilidade
  - `config.py`: carrega/gera `config.json`
  - `detection.py`: deteccao de cor HSV (funcoes puras, testadas em `tests/`)
  - `winapi.py`: clique via `SendInput`, leitura de teclas e tamanho de tela
  - `clicker.py`: disparo de clique em thread separada, com limite de CPS
  - `capture.py`: captura de tela via `mss`
  - `debug_ui.py`: janela de debug (mascara HSV + HUD)
  - `app.py`: loop principal, hotkeys e tratamento de erros
- `config.json`: todos os parametros ajustaveis (ver secao abaixo) — gerado automaticamente na primeira execucao se nao existir
- `configurar.py` / `configurar.bat`: interface grafica (Tkinter) para editar `config.json` sem editar texto a mao
- `tests/`: testes automatizados da deteccao (`pytest`)
- `requirements.txt` / `requirements-dev.txt`: dependencias de execucao / desenvolvimento
- `setup_e_rodar.bat`: instala dependencias e inicia
- `rodar.bat`: inicia diretamente (para uso diario)

## Instalacao e execucao

### Primeira vez

Execute:

```bat
setup_e_rodar.bat
```

Esse script:
1. verifica se o Python esta instalado;
2. atualiza o `pip`;
3. instala as dependencias;
4. inicia o bot.

### Proximas vezes

Execute:

```bat
rodar.bat
```

### Ajustar configuracoes pela interface grafica

Em vez de editar `config.json` manualmente, execute:

```bat
configurar.bat
```

Isso abre uma janela (Tkinter, ja incluso no Python) com um campo numerico
para cada parametro da secao [Configuracoes](#configuracoes-configjson),
com validacao de faixa e um botao para restaurar os padroes. A cor do alvo
continua sendo calibrada em tempo real com `F3` durante o uso do bot.

## Teclas de atalho

| Tecla | Acao |
|---|---|
| `F1` | Liga/Desliga o trigger |
| `F2` | Abre/fecha a janela de debug |
| `F3` | Calibra a cor no centro da mira |
| `F4` | Encerra o bot |

## Passo a passo recomendado

1. Abra o Aim Lab e entre em uma sessao de treino.
2. Execute `rodar.bat`.
3. Pressione `F2` para abrir o debug.
4. Mire no centro de um alvo.
5. Pressione `F3` para calibrar a cor.
6. Confirme no debug: a zona central deve marcar pixels detectados.
7. Pressione `F1` para ativar o trigger.
8. Pressione `F4` para sair.

## Como funciona (resumo tecnico)

- A captura e feita com `mss` numa regiao fixa no centro da tela.
- O frame e convertido para HSV.
- O bot cria uma mascara de cor (`inRange`) usando:
  - faixa calibrada (`hsv_lower` / `hsv_upper`);
  - faixa extra para vermelho alto (`high_red_lower` / `high_red_upper`, H 165-180).
- Conta os pixels detectados na zona de disparo.
- Se o total >= `min_target_pixels`, considera alvo valido.
- O clique roda em thread separada, com limite por `max_cps` e delay opcional.
- Erros inesperados no loop (ex.: falha momentanea de captura) sao logados em
  `triggerbot.log` e o bot tenta continuar, em vez de encerrar.

## Configuracoes (`config.json`)

O arquivo `config.json` e criado automaticamente na primeira execucao (com os
valores padrao abaixo) e pode ser editado sem tocar no codigo:

| Parametro | Funcao | Efeito pratico |
|---|---|---|
| `capture_size` | Tamanho da captura em px | Menor = mais rapido; maior = mais contexto |
| `trigger_zone_px` | Raio da zona de disparo | Menor = mais preciso |
| `click_delay_ms` | Delay fixo antes do clique | Maior = mais humano, menos instantaneo |
| `click_jitter_ms` | Delay aleatorio adicional | Reduz padrao de tempo fixo |
| `max_cps` | Limite de cliques por segundo | Evita spam de clique |
| `min_target_pixels` | Minimo de pixels detectados | Maior = menos falso positivo |
| `hsv_lower` / `hsv_upper` | Faixa HSV calibrada | Ajustada automaticamente com `F3` |
| `high_red_lower` / `high_red_upper` | Faixa extra de vermelho alto | Raramente precisa mudar |
| `hue_tolerance` | Tolerancia de matiz na calibracao | Maior = calibracao mais abrangente, menos precisa |

## Perfis de ajuste (exemplos)

### Mais preciso (menos falso positivo)
- `trigger_zone_px: 8`
- `min_target_pixels: 15` ate `25`
- `capture_size: 180` ate `220`

### Mais rapido (resposta imediata)
- `click_delay_ms: 0`
- `click_jitter_ms: 0`
- `capture_size: 160` ate `200`

### Mais estavel (equilibrado)
- `trigger_zone_px: 10` ate `12`
- `min_target_pixels: 10` ate `14`
- `max_cps: 12` ate `18`

## Testes

A logica de deteccao (`triggerbot/detection.py`) e testada com frames
sinteticos, sem depender de captura de tela real ou da WinAPI — roda em
qualquer sistema operacional:

```bash
pip install -r requirements-dev.txt   # numpy, opencv-python, pytest (+ deps de execucao)
pytest
```

## Solucao de problemas

### "Python nao encontrado"
- Instale Python em: `https://www.python.org/downloads/`
- Durante a instalacao, marque `Add Python to PATH`.

### Debug nao mostra alvo
- Abra o debug com `F2`.
- Mire em um alvo visivel.
- Pressione `F3` para recalibrar.
- Verifique brilho/contraste/saturacao do jogo.

### Disparando sem alvo (falso positivo)
- Aumente `min_target_pixels`.
- Reduza `trigger_zone_px`.
- Recalibre com `F3` em alvo bem iluminado.

### Reacao lenta
- Zere `click_delay_ms` e `click_jitter_ms`.
- Reduza `capture_size`.
- Feche apps pesados em segundo plano.

### Janela de debug pesada
- Deixe o debug desligado (`F2`) durante uso normal.

### Erros inesperados
- Consulte `triggerbot.log`, gerado na raiz do projeto com o traceback de
  qualquer erro ocorrido durante o loop principal.

## Observacoes finais

- O projeto foi pensado para aprendizado pratico de visao computacional em tempo real.
- A calibracao (`F3`) e essencial quando cor/iluminacao mudam.
- Nao e necessario alterar o codigo para rodar no modo basico — edite `config.json`.
- Você é livre para usar e modificar como quiser; qualquer uso fora do escopo
  descrito acima (treino solo no Aim Lab) e de sua responsabilidade.
