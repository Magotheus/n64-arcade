# Sessão 19/11/2026 (tarde, notebook) — Telemetria e correção da falha intermitente

> **Para o Claude que vai continuar:** o `CONTEXTO_DO_PROJETO.md` já foi
> atualizado com tudo daqui (seções 2, 4, 5, 7 e 10). Este arquivo é o relato
> detalhado. Resultado: **falha intermitente de reconhecimento resolvida e
> validada no Pico clone.**

## 1. Resumo
O Pico **clone** às vezes não era reconhecido pelo N64 ("Please turn the
control deck off and insert a controller"). Foram acrescentados ao firmware
uma **telemetria do protocolo Joybus** e um registro em arquivo. Com isso a
falha foi **capturada**: depois que o console desliga, o receptor do Pico fica
**desalinhado 1 bit** e lê os comandos errado (`0xFF` como `0x7F`, `0x01` como
`0x80`), sem responder. A correção (realinhar o receptor quando o console
religa e quando chega byte inválido) foi gravada no clone e passou em todos os
testes seguintes.

## 2. Linha do tempo
| Hora | O que aconteceu |
|---|---|
| ~14h | Clone (COM7) não reconhecido no Mario Tennis. Contadores provaram que o Pico **recebia** os comandos (~60 leituras/s) mas a resposta não era aceita. |
| — | No PC principal o usuário refez a solda DATA/GND do clone → passou a reconhecer; outra IA gravou firmware com `[DIAG]` e criou `firmware/painel_controle.html`. Pelo OneDrive, isso sobrescreveu edições feitas no notebook (sem perda relevante). |
| 14:45 | Voltou a falhar de forma intermitente. Telemetria criada (seção 3). |
| 14:51–14:55 | Mario Tennis (reset, desliga/liga, troca de fita) e Pokémon Stadium (liga, reset, desliga/liga): todos OK — usados como **padrão de referência**. |
| **14:56:11** | **Falha capturada** após 67 s desligado: primeiro comando lido `0x7F` (era `0xFF`), depois 162 leituras lidas como `0x80` (eram `0x01`) por 2,7 s, nenhuma respondida. Realinhou sozinho às 14:56:14.548. Log: `diagnostico/telemetria/2026-11-19_1449_clone_falha_desalinhamento.txt`. |
| 14:59 | Firmware com a correção gravado no clone. |
| 15:02–15:26 | **5 desliga/liga com a correção** — Mortal Kombat 3 (11 s e 70 s desligado), jogo de luta livre (24 s) e mais dois: `realinha_off=5`, primeiro comando sempre lido certo, **0 inválidos (`ressinc=0`), 0 incompletos**. Usuário confirmou reconhecimento e deu o teste como concluído. Log: `..._1459_clone_correcao_validada.txt`. |

## 3. Telemetria (firmware)
- `[EVT] t=<s.µs> dt=<µs> cmd=<XX> n=<bytes> resp=<hex> [ANTERIOR_INCOMPLETO] [NAO_SUPORTADO]`
  — um evento por comando concluído. Registra **tudo por 3 s** quando o
  console volta a falar após >300 ms de silêncio (linha
  `===== console comecou a falar (silencio de N ms) =====`); fora disso, só o
  anormal. Buffer circular de 512 eventos (interrupção → laço principal).
- `[EVT] linha DATA baixa ha 20 ms: console desligado` /
  `[EVT] linha DATA voltou: console ligado, receptor realinhado`.
- `[DIAG]` (2 s): mesmo formato que o `painel_controle.html` lê;
  `pulsos/s` agora = bytes recebidos/s (a interrupção de borda no pino DATA
  foi **removida** — competia com a resposta Joybus).
- `[TEL]` (2 s): `inicios`, `incompletos`, `eventos_perdidos`, `ressinc`,
  `realinha_off`.
- Programa roda da RAM (`copy_to_ram` no `CMakeLists.txt`).

## 4. Causa e correção
**Causa.** O PIO da libjoybus v0.9.0 (`joybus_target.pio`) monta bytes
contando bordas de descida (`wait 0` → amostra → `wait 1`). Com o console
desligado a linha DATA fica em 0 V: o PIO "lê" um bit 0 e fica parado. Ao
religar, esse bit sobra e todos os bytes saem deslocados. `0xFF` → `0x7F`
(bate exatamente com o capturado). Além disso, em comando inválido a lib
reinicia o PIO no meio do comando, e os bits restantes realimentam o
desalinhamento (`0x80, 0x80, …`).

**Correção (`firmware/src/main.c`).**
1. Vigia no laço principal: DATA baixa por ≥20 ms = console desligado; quando
   volta ALTA e fica parada 2 ms → `realinhar_receptor()`, que chama
   `target_rx_timeout(0, bus)` (função **não-static** do backend rp2xxx da
   v0.9.0, executa `enter_idle_mode`) com `PIO0_IRQ_0` desabilitada.
2. Comando não reconhecido: em vez de erro, o wrapper pede "mais 1 byte"; o
   restante do comando é engolido e o timeout de 100 µs da lib reinicia o PIO
   com a linha já parada.

**Atenção para atualizações futuras da libjoybus:** se a versão mudar, conferir
se `target_rx_timeout` continua exportada e se o receptor ainda tem esse
comportamento.

## 5. Padrão de boot (referência)
| Jogo | O que o console faz ao ligar |
|---|---|
| Mario Tennis | 2× `cmd=00` → leituras 60/s ~0,5 s → pausa 429 ms → jogo; sonda pak (`cmd=03`) |
| Pokémon Stadium | várias pausas de carregamento (2026 ms, 349 ms…), ~10× `cmd=00` |
| Mortal Kombat 3 | começa com `cmd=FF`; depois `cmd=00` ~1×/s o tempo todo |
| Luta livre | 174× `cmd=00` em poucos segundos; 1× `cmd=03` |

Resposta correta a `00`/`FF`: `050000` (controle N64, sem acessório).

## 6. Arquivos
| Arquivo | Mudança |
|---|---|
| `firmware/src/main.c` | Telemetria `[EVT]/[TEL]`, vigia da linha DATA, ressincronização, remoção da interrupção de borda |
| `firmware/CMakeLists.txt` | `pico_set_binary_type(n64_mechanical copy_to_ram)` |
| `firmware/n64_mechanical_pico.uf2` | Versão final — **é a gravada no clone** |
| `firmware/n64_mechanical_pico_w.uf2` | Mesma versão para Pico W (ainda não gravada nele) |
| `ferramentas/registro_telemetria.ps1` | Novo: registro contínuo em arquivo |
| `diagnostico/telemetria/` | Novo: logs da falha e da validação |

## 7. Pendente
- Testes extras opcionais (não bloqueiam): Mario Tennis com desligamento curto
  (1–2 s) e mais Pokémon Stadium com o firmware corrigido.
- Gravar o `.uf2` novo no Pico W se ele voltar a ser usado.
- Seguir a seção 10 do contexto (caixa, documentação, configuração).
