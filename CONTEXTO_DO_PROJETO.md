# N64 Arcade — contexto do projeto (para continuar em outra máquina)

> **Para o Claude que abrir esta pasta:** este arquivo resume as sessões de
> 18–19/11/2026 (PC original + notebook). Leia-o inteiro antes de agir.
>
> ## ✅ PROJETO V1 CONCLUÍDO (19/11/2026, fim da tarde)
> O usuário declarou a V1 **finalizada e aprovada**: controle hitbox com Pico
> clone, 14 botões, firmware com correção de desalinhamento, caixa impressa e
> fechada, testado e reconhecido no N64. Não há pendência obrigatória. A seção 10
> lista só itens opcionais e o roteiro da V2. Documentação final:
> `README.md` (índice), `documentacao/Dossie_Tecnico_N64_Arcade_V1_Final.docx`,
> este contexto e os relatos `SESSAO_*.md`.
>
> **GitHub (público, CC BY-NC 4.0):** https://github.com/Magotheus/n64-arcade
> — esta pasta é o repositório (`origin/main`). Logo Indião Retro Games: todos
> os direitos reservados. `kit_notebook/` fica fora (.gitignore). Usar o Git em
> uma máquina por vez (a pasta sincroniza pelo OneDrive).
>
> Histórico: a falha intermitente de reconhecimento foi resolvida na tarde de
> 19/11 (receptor do Pico desalinhado 1 bit depois que o console desliga; o
> firmware atual corrige). Relatos: `SESSAO_2026-11-19_TESTE_N64.md` (manhã,
> 1º teste) e `SESSAO_2026-11-19_TARDE_TELEMETRIA.md` (tarde, causa e
> correção). O usuário
> fala português (Brasil), prefere respostas diretas e passo a passo, e faz a
> parte física (solda, impressão, console) — você cuida de software, firmware,
> CAD e validação. Não ligue nada no console sem seguir a seção "Teste no N64".
>
> **Pasta única:** desde 19/11 o projeto vive só aqui (`Área de Trabalho\64 arcade`,
> sincronizada pelo OneDrive); a cópia antiga de `Downloads` foi unificada nesta.
> Resumo da sessão do PC principal: doc "N64 Arcade — Sessão de 18–19/11/2026 no
> PC principal" (https://claude.ai/artifact/YUgHLxfnbEKahyEQHqshdS).

## 1. O que é
Controle de **Nintendo 64 estilo fliperama (hitbox)** com 14 switches mecânicos
MX e um Raspberry Pi **Pico** (RP2040) emulando um controle original via
protocolo **Joybus**. Objetivo: jogar ports da comunidade para N64,
principalmente de fliperama/2D (Comix Zone 64, Final Fight 64, Donut Dodo,
Commander Keen, 64doom etc.). Fase atual **sem analógico**: os 4 direcionais
são switches digitais.

O dossiê original (Word, protótipo) está em `Downloads\Dossie_Tecnico_N64_Mechanical_Controller_V1_Prototipo.docx`
no PC original e foi mantido como histórico. A versão final é
`documentacao/Dossie_Tecnico_N64_Arcade_V1_Final.docx`.

## 2. Estado atual (19/11/2026, ~15h30)

| Item | Estado |
|---|---|
| Firmware N64 (C + Pico SDK 2.2.0 + libjoybus v0.9.0) | ✅ compila (agora também no notebook), grava e inicia (`joybus_init=0`) |
| 14 botões no Pico W (protótipo antigo) | ✅ validados no monitor serial **e no console** |
| Direcionais → stick **e** D-pad ao mesmo tempo | ✅ funcionando no console |
| Cabo N64 (extensão) | ✅ identificado e ligado (GND → pino 23, DATA → pino 24, +3,3 V isolado) |
| **Primeiro teste no console** (Pico W, porta 1, USB do notebook) | ✅ **Mario Tennis, Zelda OoT e Pokémon Stadium reconheceram** |
| Pokémon Stadium | ✅ falhas intermitentes **explicadas e corrigidas** (seção 7, "Desalinhamento do receptor") |
| **Falha intermitente "insert a controller"** | ✅ **causa achada e corrigida em 19/11 tarde** — 5 desliga/liga com a correção (MK3 11 s e 70 s, luta livre 24 s e mais 2), todos limpos, 0 comandos inválidos |
| Telemetria Joybus | ✅ firmware registra cada comando do console + resposta (`[EVT]`, `[TEL]`); `ferramentas/registro_telemetria.ps1` grava em arquivo |
| Firmware com resposta "sem pak" (0x02/0x03) | ✅ gravado no Pico W e no Pico clone |
| Pico **clone** novo (vai para o projeto) | ✅ diagnóstico de saúde 100% · firmware novo gravado · **14 botões soldados e validados no USB em 19/11** (monitor, COM7 no PC principal, sem botão fantasma) · cabo N64 ligado (branco GP18, preto GND pino 23) · **1º teste no console falhou; depois de refazer a solda do DATA/GND, o console reconheceu (19/11)** · firmware atual tem diagnóstico `[DIAG]` (nível da linha DATA, pulsos/s, comandos recebidos) mostrado no indicador "Console" do `painel_controle.html` |
| Fiação dos botões no Pico novo | ✅ feita e validada (14/14 no painel/monitor) |
| Alimentação | ✅ decidido: USB com carregador de celular; +3,3 V do cabo isolado (seção 10, passo 3) |
| Firmware novo no Pico clone | ✅ **versão com telemetria + correção de desalinhamento gravada em 19/11 ~15h (notebook)** — é a do `firmware/n64_mechanical_pico.uf2` atual |
| Firmware no Pico W | ⚠️ ainda com versão antiga; o `n64_mechanical_pico_w.uf2` atual já tem a correção (gravar quando for usá-lo) |
| Caixa impressa (Bambu Lab A1, PLA) | ✅ tampa, fundo e as 14 teclas com legenda impressos; tecla de teste aprovada no switch; controle montado e fechado. O clone não tem furos de fixação (o berço de 4 pinos foi feito para o Pico oficial); a forma de fixação usada na montagem não foi registrada |
| Adesivos | ✅ PDF A4 em tamanho real (`caixa/adesivos/`): alerta de ligar/desligar, faixas laterais/frontal com o logo Indião Retro Games |
| Painel de teste (Web Serial) | ✅ `firmware/painel_controle.html`: controle desenhado, checklist 14/14, indicador "Console" |
| **Status geral** | ✅ **V1 CONCLUÍDA em 19/11/2026** |

## 3. Pinagem (vale para Pico e Pico W — mesma pinagem física)

| Botão | GPIO | Pino físico |
|---|---|---|
| CIMA / BAIXO / ESQUERDA / DIREITA | GP2 / GP3 / GP4 / GP5 | 4 / 5 / 6 / 7 |
| A / B / Z / L | GP6 / GP7 / GP8 / GP9 | 9 / 10 / 11 / 12 |
| R / C_CIMA / C_BAIXO / C_ESQUERDA | GP10 / GP11 / GP12 / GP13 | 14 / 15 / 16 / 17 |
| C_DIREITA / START | GP14 / GP15 | 19 / 20 |
| GND dos botões (mão esquerda / direita) | — | 3 / 28 |
| **DATA do N64** | **GP18** | **24** |
| **GND do cabo N64** | — | **23** |

- Switch entre GPIO e GND, pull-up interno → **pressionado = LOW**.
- Opostos (esq+dir, cima+baixo) se anulam (neutro).
- Stick: ±80 nas direções retas, ±70 por eixo nas diagonais.
- GP23/24/25/29 não usar (no Pico W são do Wi-Fi).

## 4. Arquivos desta pasta

| Caminho | O que é |
|---|---|
| `firmware/n64_mechanical_pico.uf2` | **Firmware pronto para o Pico comum/clone** (o do projeto) |
| `firmware/n64_mechanical_pico_w.uf2` | Mesmo firmware para Pico W |
| `firmware/src/main.c`, `CMakeLists.txt`, `build.ps1` | Fonte e build (`.\build.ps1 -Placa pico` ou sem parâmetro p/ Pico W) |
| `firmware/src/main_v1_backup.c` | `main.c` antes da resposta "sem pak" (V1 original) |
| `firmware/monitor.ps1` | Monitor serial legível: mostra APERTOU/soltou e avisa em amarelo se dois botões aparecem juntos (normal quando você aperta dois de propósito). Mostra em roxo quando o console consulta o acessório (pak). Acha o Pico sozinho. Reconecta se o USB cair. |
| `SESSAO_2026-11-19_TESTE_N64.md` | Relato completo do primeiro teste no console |
| `SESSAO_2026-11-19_TARDE_TELEMETRIA.md` | Relato da tarde: telemetria, causa da falha intermitente e correção |
| `firmware/painel_controle.html` | Painel (Web Serial, Chrome/Edge) feito no PC principal; indicador "Console" lê a linha `[DIAG]` |
| `ferramentas/teste_controle.html` | Página de teste visual mais simples (caixa com teclas nas posições do CAD) |
| `ferramentas/registro_telemetria.ps1` | **Registro contínuo da serial em arquivo com hora** (para ver ao vivo: `Get-Content <arquivo> -Wait -Tail 40`). Não roda junto com painel/monitor (mesma porta) |
| `diagnostico/telemetria/*.txt` | Logs de 19/11: `..._falha_desalinhamento` (a falha capturada às 14:56) e `..._correcao_validada` |
| `ferramentas/pico_saude/` | Diagnóstico de saúde do Pico (`testar.ps1` + `pico_saude.uf2`) |
| `diagnostico/n64_gpio_diagnostic.py` | Script MicroPython original do dossiê |
| `caixa/n64_arcade_hitbox.scad` | CAD paramétrico (OpenSCAD) da caixa e das teclas |
| `caixa/*.stl`, `caixa/impressao/*.3mf` | Peças e projetos já fatiados p/ Bambu A1 (PLA, Textured PEI) |
| `caixa/esquema_fiacao.html` | Esquema de fiação desenhado (abrir no navegador; tem vista espelhada p/ soldar) |
| `kit_notebook/` | Cópia enxuta só do necessário para gravar/testar (redundante com o resto) |

**Para gravar e testar não é preciso instalar nada** — só PowerShell do Windows.
Para **recompilar**: Arm GNU Toolchain 14.2, CMake, Ninja, Git, Pico SDK 2.2.0
com submódulos, e pioasm/picotool de `raspberrypi/pico-sdk-tools` (release
v2.2.0-4), tudo em `%USERPROFILE%\.pico-sdk`. **No notebook isso já está
instalado** (19/11): CMake 4.4.4 e Ninja 1.13.2 via winget; Arm 14.2.Rel1 em
`.pico-sdk\toolchain\14_2_Rel1` (baixado da Arm — o winget só tem a 12.2; não
está no PATH, o `build.ps1` acha sozinho). O build usa pasta de caminho curto
(`.pico-sdk\build\...`) por causa do limite de 260 caracteres do Windows.

## 5. Como gravar e monitorar (no notebook)
1. Segure **BOOTSEL** no Pico, conecte o USB, solte → aparece a unidade **RPI-RP2**.
2. Copie `firmware\n64_mechanical_pico.uf2` para ela. O Pico reinicia sozinho.
3. No PowerShell, dentro da pasta `firmware`:
   ```
   powershell -ExecutionPolicy Bypass -File .\monitor.ps1
   ```
   A cada 5 s o firmware imprime
   `[N64 Mechanical V1] joybus_init=0, DATA=GP18, pak_read=N, pak_write=N`.
   O monitor esconde essa linha e mostra só APERTOU/soltou (e os contadores de
   pak em roxo quando mudam). O firmware atual também imprime `[DIAG]` (2 s),
   `[TEL]` (2 s) e `[EVT]` (eventos Joybus) — para ver/guardar tudo use
   `ferramentas\registro_telemetria.ps1` ou o `firmware\painel_controle.html`.
4. Atalho: abrir a porta serial do Pico a **1200 baud** reinicia em BOOTSEL
   (aparece RPI-RP2) sem apertar o botão. Feche o monitor antes (ele ocupa a porta).

## 6. Teste no N64 — procedimento (✅ feito e aprovado em 19/11 com o Pico W)
1. **Extensão de controle N64** (não cortar o cabo do controle original).
   Identificar GND, DATA e +3,3 V **com multímetro em continuidade**, sem
   confiar em cores (plug e soquete podem estar espelhados).
2. Ligação para o primeiro teste:
   - GND do cabo → **pino 23** do Pico · DATA → **GP18 (pino 24)**
   - **+3,3 V do cabo: isolado, não ligar em nada.**
   - Pico alimentado pelo **USB do notebook** (de preferência notebook na
     bateria, fora da tomada, para evitar laço de terra).
3. Verificado no código da libjoybus v0.9.0: o PIO só **puxa a linha para 0 V
   ou solta** (`side_set pindirs`), nunca força 3,3 V, e não liga pull-up interno
   — o pull-up é do console. Por isso a ligação direta GP18 ↔ DATA é segura.
4. Ordem: **conectar USB no Pico primeiro** → encaixar a extensão na **porta 1**
   → ligar o console. Para desligar: **console primeiro, USB por último**
   (Pico sem energia com a linha DATA em 3,3 V não é bom).
5. Testar com um jogo simples (tela de "press start"), depois os 14 botões.
6. Se o console não reconhecer: conferir GND comum, DATA no pino 24, e se o
   monitor ainda mostra os botões (o firmware continua rodando).

**Alimentação definitiva**: decidida — USB com carregador de celular, +3,3 V do cabo isolado (seção 10, item 3).

## 7. Detalhes técnicos importantes
- **libjoybus v0.9.0 ≠ documentação online** (que segue a `main`). API usada:
  `joybus_rp2xxx_init(&bus, gpio, pio0)`, `joybus_enable(bus)`,
  `joybus_n64_controller_init(&c, JOYBUS_ID_N64_CONTROLLER >> 8)`,
  `joybus_target_register(bus, JOYBUS_TARGET(&c))`; eixos são `uint8_t`
  (complemento de dois). No CMake: `set(JOYBUS_BACKEND_RP2XXX 1)`.
- O Pico novo é **clone**: flash Winbond W25Q128 (**16 MB**), sem divisor de
  VSYS no GP29 (leitura de tensão não vale). Saúde: tudo OK. No PC original
  apareceu como COM7. Uma vez desconectou sozinho — observar conector/cabo.
- **Defeito já resolvido**: fio do C_DIREITA encostando no GND (botão "fantasma"
  junto com L/R). Lição: isolar toda emenda com termo-retrátil.
- **Comandos de acessório (pak)**: a libjoybus v0.9.0 só responde `0xFF`,
  `0x00`, `0x01`. O `main.c` agora intercepta `0x02`/`0x03` (troca
  `n64_controller.base.api` por uma API própria que repassa o resto) e responde
  "sem pak": dados zerados + CRC de dados (polinômio 0x85) **invertido**.
- **Pokémon Stadium**: falhou na 1ª tentativa (firmware V1) e reconheceu depois
  de gravar o firmware novo, **mas os contadores ficaram `pak_read=0,
  pak_write=0`** — o jogo não mandou comandos de pak (status `0x00` já diz "sem
  pak"). Então **a mudança não é a causa comprovada**; a falha pode ter sido
  intermitente. Resposta de pak mantida como robustez. (Mais tarde vários jogos
  — Mario Tennis, luta livre — mandaram `0x03` e o Pico respondeu certo.)
- **Desalinhamento do receptor — CAUSA da falha intermitente (19/11 tarde)**:
  o PIO da libjoybus (`joybus_target.pio`) conta cada borda de descida como 1
  bit. Com o console **desligado a linha DATA fica em 0 V**; o PIO guarda esse
  "bit" e, ao religar, lê tudo deslocado 1 bit. Capturado na telemetria às
  14:56: o `0xFF` de boot chegou como `0x7F` e cada `0x01` como `0x80` por
  2,7 s, sem resposta. Jogos que só procuram o controle ao ligar (Mario
  Tennis) mostram "insert a controller". Agravante: em comando inválido a lib
  reinicia o PIO **no meio** do comando e o resto vira bit sobrando
  (realimenta o erro).
  **Correção no `main.c`**: (1) vigia no laço principal — DATA baixa >20 ms =
  console desligado; quando volta ALTA e fica 2 ms parada, chama
  `target_rx_timeout(0, bus)` (função não-static do backend rp2xxx v0.9.0, que
  executa `enter_idle_mode`) com `PIO0_IRQ_0` desabilitada → receptor zerado
  antes do 1º comando; (2) em comando não reconhecido o wrapper devolve "mais 1
  byte" em vez de erro: o resto do comando é engolido e o timeout de 100 µs da
  lib reinicia o PIO com a linha parada. Contadores `realinha_off` e `ressinc`
  na linha `[TEL]`.
- **Telemetria (`main.c`)**: cada comando concluído vira `[EVT] t=… dt=… cmd=XX
  n=… resp=…` — registra tudo por 3 s quando o console volta a falar após
  >300 ms de silêncio, e fora disso só o anormal (`ANTERIOR_INCOMPLETO`,
  `NAO_SUPORTADO`). `[DIAG]` a cada 2 s mantém o formato que o painel lê, mas
  `pulsos/s` agora = **bytes recebidos/s** (a interrupção de borda no pino DATA
  foi removida por competir com a resposta Joybus). `[TEL]`: `inicios`,
  `incompletos`, `eventos_perdidos`, `ressinc`, `realinha_off`.
- **Firmware roda da RAM** (`pico_set_binary_type(... copy_to_ram)` no
  `CMakeLists.txt`) para eliminar atraso de cache da flash na resposta.
- **Padrão de boot visto na telemetria** (referência para comparar falhas):
  Mario Tennis: 2× `cmd=00` → leituras 60/s → pausa 429 ms → jogo (sonda pak
  com `cmd=03`). Pokémon Stadium: várias pausas de carregamento (2026 ms, 349
  ms), ~10 `cmd=00`. MK3: começa com `cmd=FF` e pergunta `cmd=00` ~1×/s.
  Resposta correta a `00`/`FF`: `050000`.
- **OneDrive**: editar nas duas máquinas ao mesmo tempo faz uma sobrescrever a
  outra (aconteceu em 19/11). Trabalhar em uma máquina por vez.

## 8. Caixa (hitbox compacto) — decisões
- 240 × 128 × 28 mm, cabe na mesa de 256 mm do **Bambu Lab A1**.
- Layout inspirado num arcade stick N64: mão esquerda ◀ ▼ ▶ nos dedos, ▲ no
  polegar; mão direita coluna B (verde) / A (azul), quadrado dos 4 C (amarelos),
  coluna L/R; Z no polegar direito; Start (vermelho) no canto superior esquerdo.
- Teclas **redondas Ø24 mm** com encaixe MX, impressas (topo plano para apoiar
  na mesa; cruz 4,15 × 1,35 mm com folga). Furo do switch 14,1 mm.
- **Sem parafusos**: 6 ganchos snap-fit (fundo → canaletas na tampa) e berço do
  Pico com 4 pinos + 1 trava. Ajustes: `snap_p`, `latch_lip`, `cross_w/cross_t`.
- **Legendas em baixo relevo** (0,6 mm) no topo das teclas: A, B, Z, L, R,
  START, C▲ C▼ C◀ C▶ e as setas ▲ ▼ ◀ ▶ (parâmetro `cap_legend`; os 14 STL
  ficam em `caixa/teclas/`, regerados com `caixa/gerar_teclas.ps1`; prévia em
  `caixa/preview_teclas.png`). Na montagem, ▲ e C▲ apontam para o fundo da caixa.
- Fatiamento validado (Bambu A1, PLA, Textured PEI, em `caixa/impressao/`):
  tampa 3h24/121 g · fundo 2h13/79 g · teclas por cor: cinza direcionais
  26 min/7,8 g · amarelas C 28 min/7,8 g · pretas L R Z 22 min/5,9 g · A, B e
  START ~10 min/2 g cada · tecla de teste sem legenda 10 min/2 g.
- **Teste físico**: tecla de teste (A) aprovada no switch ("ficou excelente");
  tampa, fundo e teclas impressos e o controle montado e fechado.

## 9. Ideias para depois (não fazer agora)
- Software de configuração: remapear botões, perfis por jogo, modo do
  direcional (stick/D-pad/ambos), SOCD, turbo — via página web no Chrome
  (Web Serial) gravando na flash do Pico; troca de perfil por Start+botão.
- Atualizar o dossiê Word (há inconsistências: log do botão A ausente no
  teste 9.2, apêndices fora de ordem, datas).
- **Controle remoto pela USB** (pedido do usuário): o Pico aceitar comandos
  pela serial (ex. `A`, `START`, `CIMA 500ms`) e repassá-los ao console —
  para testes automáticos e medir atraso. Base para o software de configuração.

## 10. ESTADO FINAL E PRÓXIMOS PASSOS (V1 concluída)
Todos os passos da V1 foram concluídos em 19/11/2026. Registro do que foi feito
e do que ficou como opcional:

1. ✅ **Falha intermitente — RESOLVIDA em 19/11 tarde** (seção 7,
   "Desalinhamento do receptor"). Se aparecer "insert a controller" de novo,
   rodar `ferramentas/registro_telemetria.ps1` e procurar `NAO_SUPORTADO` /
   `realinha_off` no arquivo. *Opcional:* gravar o `n64_mechanical_pico_w.uf2`
   novo no Pico W se ele voltar a ser usado.
2. ✅ **14 botões soldados no Pico clone**, validados no painel, cabo N64 ligado
   (branco GP18, preto GND pino 23) e reconhecido no console.
3. ✅ **Alimentação — DECIDIDO em 19/11: USB com carregador de celular.** O
   +3,3 V do cabo (fio vermelho) **fica isolado para sempre**. Regras: o Pico
   tem que estar ligado sempre que o console estiver ligado (carregador antes,
   console depois; desligar o console primeiro); deixar o carregador sempre na
   tomada; não usar a porta USB da TV (desliga em espera). Medido no cabo:
   vermelho = +3,3 V em relação ao preto. Alternativa descartada por ora:
   console → diodo Schottky (1N5817/1N5819/BAT54, faixa para o VSYS pino 39),
   que permitiria cabo único; pilhas também descartadas.
4. ✅ **Caixa** impressa e montada; teclas com legenda; adesivos gerados.
   *Melhoria opcional:* um berço para o Pico clone sem furos (preso pelas
   bordas), que exige medir comprimento e largura da placa.
5. ✅ **Documentação**: `README.md`, este contexto e o dossiê final em
   `documentacao/Dossie_Tecnico_N64_Arcade_V1_Final.docx`.
6. **V2 (ideias, seção 9):** software de configuração (remapear, perfis,
   turbo), controle pela USB, versão sem fio (ESP32 ou Pico W + BlueRetro),
   adaptação para SNES (protocolo de registrador de deslocamento, sinais de
   5 V exigem divisor resistivo).
