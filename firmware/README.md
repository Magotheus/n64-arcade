# N64 Arcade — firmware V1 (final)

Firmware em C (Pico SDK 2.2.0 + libjoybus v0.9.0) que faz um Raspberry Pi Pico
(RP2040) se apresentar ao Nintendo 64 como um controle original (ID `0x0500`,
sem acessório). Testado e aprovado no console em 19/11/2026.

| Item | Valor |
|---|---|
| Linha DATA | **GP18** (pino físico 24) · GND do cabo no pino 23 · +3,3 V do cabo **isolado** |
| Botões | GP6–GP15 → A, B, Z, L, R, C▲, C▼, C◀, C▶, START (switch entre GPIO e GND, pull-up interno) |
| Direcionais | GP2–GP5 → **stick** (±80 retos, ±70 nas diagonais) **e D-pad** ao mesmo tempo |
| Opostos juntos | se anulam (neutro) |
| Acessório | responde "sem pak" aos comandos 0x02/0x03 |
| Robustez | **realinha o receptor** quando o console religa e em comando inválido (correção da falha "insert a controller") |
| Execução | da RAM (`copy_to_ram`), sem atraso de cache da flash |
| Alimentação | USB (carregador de celular ou PC) |

## Arquivos prontos

| Arquivo | Para |
|---|---|
| `n64_mechanical_pico.uf2` | **Pico comum / clone** — é o que está no controle |
| `n64_mechanical_pico_w.uf2` | Pico W |
| `src/main_v1_backup.c` | Versão anterior à resposta "sem pak" (histórico) |

## Gravar
1. Segure **BOOTSEL**, conecte o USB, solte → aparece a unidade **RPI-RP2**.
2. Copie o `.uf2` certo para ela. O Pico reinicia sozinho.

Atalho: abrir a porta serial do Pico a **1200 baud** reinicia em BOOTSEL sem
apertar o botão (feche painel/monitor antes).

## Testar e diagnosticar (nada para instalar)
| Ferramenta | O que faz |
|---|---|
| `painel_controle.html` (Chrome/Edge) | Controle desenhado ao vivo, checklist 14/14, stick X/Y, indicador **"Console"** (lê `[DIAG]`) |
| `monitor.ps1` | Monitor no terminal: APERTOU/soltou, aviso amarelo de dois botões juntos, acha o Pico sozinho |
| `..\ferramentas\registro_telemetria.ps1` | Grava toda a serial em arquivo com hora (para investigar falhas) |

Só um programa usa a porta serial por vez.

### Linhas que o firmware imprime pela USB
| Linha | Frequência | Conteúdo |
|---|---|---|
| `X=.. Y=.. DPAD=.. \| botões` | quando algo muda | estado dos botões |
| `[N64 Mechanical V1] ...` | 5 s | `joybus_init`, pino DATA, contadores de pak |
| `[DIAG] ...` | 2 s | nível da linha DATA, bytes/s, comandos recebidos (info/ler/reset/outros) |
| `[TEL] ...` | 2 s | inícios, incompletos, eventos perdidos, `ressinc`, `realinha_off` |
| `[EVT] ...` | eventos | cada comando do console + resposta (tudo por 3 s após o console voltar a falar; depois só o anormal) |

## Compilar (opcional)
Ambiente em `%USERPROFILE%\.pico-sdk` (instalado no PC principal e no notebook):
Arm GNU Toolchain 14.2, CMake 4.4.4, Ninja 1.13.2, Git, Pico SDK 2.2.0 com
submódulos, pioasm 2.2.0 e picotool 2.2.0-a4 (release v2.2.0-4 de
`raspberrypi/pico-sdk-tools`).

    .\build.ps1 -Placa pico     # Pico comum / clone
    .\build.ps1                 # Pico W

O build usa pasta curta (`.pico-sdk\build\n64_<placa>`) por causa do limite de
260 caracteres do Windows. A libjoybus v0.9.0 é baixada pelo CMake.

**Atenção:** a API da libjoybus **v0.9.0** difere da documentação online
(branch `main`). A correção de desalinhamento usa `target_rx_timeout` do backend
rp2xxx da v0.9.0 — conferir se continua exportada antes de atualizar a lib.

**OneDrive:** não editar o firmware nas duas máquinas ao mesmo tempo.

## Uso no console
- Ligar: **carregador/USB → controle**, depois **liga o N64**.
- Desligar: **N64 primeiro**, depois o USB.
- Cabo sempre na **porta 1** para jogos de 1 jogador.
