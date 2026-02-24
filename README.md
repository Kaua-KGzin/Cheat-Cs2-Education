# AimLab TriggerBot v3

Projeto educacional para estudar:
- captura de tela em alta frequencia;
- deteccao de cor em HSV com OpenCV;
- automacao de clique via WinAPI (`SendInput`);
- impacto de latencia, ruido e calibracao.

## Aviso de uso

Use este projeto apenas para estudo e testes em ambiente controlado.  
Voce e responsavel por respeitar as regras do jogo/plataforma onde executar.

## O que o bot faz

1. Captura uma area pequena no centro da tela (`CAPTURE_SIZE`).
2. Analisa somente a zona perto da mira (`TRIGGER_ZONE_PX`).
3. Detecta a cor alvo em HSV (com calibracao por `F3`).
4. Se o trigger estiver ligado (`F1`) e houver alvo na zona, envia clique.
5. Limita cliques por segundo (`MAX_CPS`) para evitar spam.

## Requisitos

- Windows 10/11
- Python 3.9+ (recomendado 3.10+)
- `pip` no PATH

Dependencias usadas:
- `mss`
- `numpy`
- `opencv-python`
- `pywin32`

## Arquivos do projeto

- `trigger.py`: logica principal (captura, deteccao, trigger, debug e hotkeys)
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
  - faixa calibrada (`col_lower` / `col_upper`);
  - faixa extra para vermelho alto (`H 165-180`).
- Conta os pixels detectados na zona de disparo.
- Se `total >= MIN_WHITE_PIXELS`, considera alvo valido.
- O clique roda em thread separada, com limite por `MAX_CPS` e delay opcional.

## Configuracoes importantes (`trigger.py`)

| Parametro | Funcao | Efeito pratico |
|---|---|---|
| `CAPTURE_SIZE` | Tamanho da captura em px | Menor = mais rapido; maior = mais contexto |
| `TRIGGER_ZONE_PX` | Raio da zona de disparo | Menor = mais preciso |
| `CLICK_DELAY_MS` | Delay fixo antes do clique | Maior = mais humano, menos instantaneo |
| `CLICK_JITTER_MS` | Delay aleatorio adicional | Reduz padrao de tempo fixo |
| `MAX_CPS` | Limite de cliques por segundo | Evita spam de clique |
| `MIN_WHITE_PIXELS` | Minimo de pixels detectados | Maior = menos falso positivo |

## Perfis de ajuste (exemplos)

### Mais preciso (menos falso positivo)
- `TRIGGER_ZONE_PX = 8`
- `MIN_WHITE_PIXELS = 15` ate `25`
- `CAPTURE_SIZE = 180` ate `220`

### Mais rapido (resposta imediata)
- `CLICK_DELAY_MS = 0`
- `CLICK_JITTER_MS = 0`
- `CAPTURE_SIZE = 160` ate `200`

### Mais estavel (equilibrado)
- `TRIGGER_ZONE_PX = 10` ate `12`
- `MIN_WHITE_PIXELS = 10` ate `14`
- `MAX_CPS = 12` ate `18`

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
- Aumente `MIN_WHITE_PIXELS`.
- Reduza `TRIGGER_ZONE_PX`.
- Recalibre com `F3` em alvo bem iluminado.

### Reacao lenta
- Zere `CLICK_DELAY_MS` e `CLICK_JITTER_MS`.
- Reduza `CAPTURE_SIZE`.
- Feche apps pesados em segundo plano.

### Janela de debug pesada
- Deixe o debug desligado (`F2`) durante uso normal.

## Observacoes finais

- O projeto foi pensado para aprendizado pratico de visao computacional em tempo real.
- A calibracao (`F3`) e essencial quando cor/iluminacao mudam.
- Nao e necessario alterar o codigo para rodar no modo basico.
- Você é livre para usar como quiser, até melhorar.
