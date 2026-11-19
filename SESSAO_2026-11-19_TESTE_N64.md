# Sessão 19/11/2026 (madrugada) — Primeiro teste no console N64

> **Para o Claude que vai atualizar a documentação:** este arquivo registra o
> que foi feito numa sessão no **notebook**, depois do `CONTEXTO_DO_PROJETO.md`.
> Use-o para atualizar o `CONTEXTO_DO_PROJETO.md` (seções 2, 4, 6 e 7) e o
> dossiê Word. Tudo que está marcado como **não comprovado** deve continuar
> assim na documentação.

## 1. Resumo em uma linha
**O controle funcionou no N64 de verdade** (porta 1, Pico W com os 14 botões):
Mario Tennis, Zelda e Pokémon Stadium reconheceram o controle. O Pokémon Stadium
falhou na primeira tentativa e funcionou depois de regravar o firmware, mas
**a causa da falha não foi comprovada** (ver seção 4).

## 2. Montagem usada no teste
- Placa: **Pico W** (protótipo antigo, 14 botões já soldados), no USB do
  notebook (COM6 neste notebook).
- Extensão de controle N64: **GND → pino 23**, **DATA → GP18 (pino 24)**,
  **+3,3 V isolado** (não ligado). Fios identificados pelo usuário com
  multímetro.
- Ordem seguida: USB no Pico primeiro → extensão na porta 1 → liga o console.
  Desligar: console → extensão → USB.
- Antes do console, o firmware respondeu na serial
  `joybus_init=0, DATA=GP18` (OK).

## 3. Resultados por jogo
| Jogo | Resultado | Observações |
|---|---|---|
| Mario Tennis | ✅ reconheceu | START, direcionais e botões responderam no jogo |
| The Legend of Zelda (Ocarina of Time) | ✅ reconheceu | — |
| Pokémon Stadium — 1ª tentativa (firmware V1 original) | ❌ não reconheceu o controle | — |
| Pokémon Stadium — 2ª tentativa (firmware novo, ver seção 4) | ✅ reconheceu | A, B, L, R, C e START usados no jogo; R segurado + botões C funcionou |

Os **14 botões** foram apertados um a um no monitor serial antes de ligar o
console (todos limpos, sem botão fantasma) e depois **testados no console:
todos funcionando** (confirmado pelo usuário).

## 4. Pokémon Stadium — investigação e mudança no firmware
**Hipótese inicial:** a libjoybus **v0.9.0** só responde aos comandos Joybus
`0xFF` (reset), `0x00` (identify) e `0x01` (read). Os comandos de acessório
`0x02` (pak read) e `0x03` (pak write) ficam **sem resposta** (conferido no
código: `src/target/n64_controller.c` da tag v0.9.0). A suspeita era que o
Pokémon Stadium procurava o Transfer Pak e, sem resposta, desistia do controle.

**Mudança feita** (`firmware/src/main.c`):
- O firmware intercepta o `byte_received` do target N64
  (`n64_controller.base.api` trocado por uma API própria) e responde:
  - `0x02` (3 bytes recebidos) → 32 bytes `0x00` + CRC dos dados **invertido**
    (= "nenhum pak inserido", como um controle original sem acessório).
  - `0x03` (35 bytes recebidos) → 1 byte: CRC dos 32 bytes **invertido**.
  - Demais comandos → repassados para a libjoybus.
  - CRC de dados do pak: polinômio `0x85` (mesmo algoritmo do libdragon).
- O aviso periódico (a cada 5 s) agora é:
  `[N64 Mechanical V1] joybus_init=0, DATA=GP18, pak_read=N, pak_write=N`
- O status do controle continua `05 00 00` (byte de status `0x00`).

**Resultado — importante:** com o firmware novo o Pokémon Stadium reconheceu,
**mas os contadores ficaram `pak_read=0, pak_write=0` a sessão inteira**. Ou
seja, o jogo **não mandou** os comandos de acessório (faz sentido: o status
`0x00` já diz ao jogo que não há pak). **Conclusão: a mudança não foi a causa
comprovada do conserto.** A falha da 1ª tentativa pode ter sido intermitente
(contato do cartucho, ordem de energização, Pico/console num estado ruim etc.).
A resposta de pak foi **mantida no firmware como robustez** (é inofensiva e
deixa o controle mais fiel ao original).

**Pendente para confirmar:** desligar e ligar o console várias vezes com o
Pokémon Stadium (e com outros jogos) para ver se a falha de reconhecimento
volta. Se voltar, anotar: jogo, mensagem na tela, se o monitor serial
continuava mostrando os botões, e os contadores pak_read/pak_write.

## 5. Arquivos alterados nesta sessão
| Arquivo | O que mudou |
|---|---|
| `firmware/src/main.c` | Resposta "sem pak" para `0x02`/`0x03` + contadores na linha periódica |
| `firmware/src/main_v1_backup.c` | **Novo** — cópia do `main.c` antes da mudança |
| `firmware/n64_mechanical_pico_w.uf2` | Recompilado com a mudança (**gravado no Pico W** e testado no console) |
| `firmware/n64_mechanical_pico.uf2` | Recompilado com a mudança (Pico comum/clone — **ainda não gravado no clone**) |
| `firmware/monitor.ps1` | Mostra em roxo "console consultou acessorio (leituras/escritas): N/N" quando os contadores mudam |
| `firmware/build.ps1` | Acha sozinho o Arm GNU Toolchain em `%USERPROFILE%\.pico-sdk\toolchain\<pasta>\bin` |

> Os `.uf2` antigos (V1 sem a resposta de pak) foram sobrescritos. Para
> voltar à V1 basta compilar com o `main_v1_backup.c` no lugar do `main.c`.

## 6. Ambiente de compilação instalado no notebook
Tudo em `%USERPROFILE%\.pico-sdk` (o mesmo layout que o `build.ps1` espera):
| Item | Versão | Como |
|---|---|---|
| Arm GNU Toolchain | 14.2.Rel1 | zip de developer.arm.com → `toolchain\14_2_Rel1` (não está no PATH do sistema; o `build.ps1` adiciona) |
| CMake | 4.4.4 | `winget install Kitware.CMake` |
| Ninja | 1.13.2 | `winget install Ninja-build.Ninja` |
| Pico SDK | 2.2.0 (+ submódulos) | `git clone --branch 2.2.0` → `sdk\2.2.0` |
| pioasm / picotool | 2.2.0 / 2.2.0-a4 | `raspberrypi/pico-sdk-tools` release v2.2.0-4 → `tools\2.2.0`, `picotool\2.2.0-a4` |

Compilar: na pasta `firmware`, `.\build.ps1` (Pico W) ou `.\build.ps1 -Placa pico`.
Observação: o winget só oferece o Arm Toolchain 12.2 (MPACBTI), por isso a
14.2 foi baixada direto da Arm.

**Dica de gravação:** abrir a porta serial do Pico a **1200 baud** faz o
firmware reiniciar em modo BOOTSEL (unidade RPI-RP2) sem apertar o botão
(recurso padrão do `pico_stdio_usb`).

## 7. Observações e pendências
- **Avisos amarelos no monitor** ("ao mesmo tempo") apareceram várias vezes
  durante os jogos. Quase todos são combinações intencionais (R segurado + C
  no Pokémon Stadium, diagonais, direcional + A) ou botões C apertados em
  sequência rápida. **O usuário confirmou que todos os 14 botões foram
  testados no console e funcionam certinho** — não há indício de fio
  encostando.
- Repetir o teste de reconhecimento no Pokémon Stadium (ver seção 4).
- Gravar o firmware novo no **Pico clone** quando ele receber os botões.
- Ideia levantada pelo usuário (para depois): **controle remoto pela USB** —
  o Pico aceitar comandos pela serial (ex. `A`, `START`, `CIMA 500ms`) e
  repassá-los ao console, para testes automáticos e medição de atraso. Exige
  alterar o firmware; combina com o software de configuração via Web Serial
  já previsto na seção 9 do contexto.
- Próxima etapa do projeto continua: alimentação definitiva pelo console
  (medir antes; fase 9 do dossiê).
