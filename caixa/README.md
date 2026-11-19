# N64 Arcade — caixa hitbox compacta (V1 final)

240 × 128 × 28 mm · 14 switches MX · teclas redondas Ø24 mm com legenda · Pico interno

**Status (19/11/2026):** impressa no Bambu Lab A1 (PLA), tecla de teste aprovada
no switch, controle montado e fechado. ✅

| Arquivo | Imprimir | Observação |
|---|---|---|
| `n64_arcade_tampa.stl` | 1× | Já vem de cabeça para baixo (topo na mesa). Sem suporte. |
| `n64_arcade_fundo.stl` | 1× | Sem suporte. |
| `n64_arcade_tecla_redonda.stl` | 14× | Topo na mesa, pino para cima. Cores: 4 cinza-escuro (direcionais), 1 verde (B), 1 azul (A), 4 amarelas (C), 3 pretas (L, R, Z), 1 vermelha (Start). |

Sugestão de impressão: PLA ou PETG, 0,2 mm, 3 paredes, 20% de preenchimento.
Para as teclas, 0,12–0,16 mm deixa o topo mais liso.

Projetos já fatiados para o **Bambu Lab A1** (PLA, mesa Textured PEI) estão em
`impressao/`: `tampa_A1.3mf` (3h24, 121 g), `fundo_A1.3mf` (2h13, 79 g),
`tecla_teste_A1.3mf` (10 min, 2 g, sem legenda) e as teclas **com legenda em
baixo relevo**, uma placa por cor: `teclas_cinza_direcionais_A1` (▲▼◀▶),
`teclas_amarelas_C_A1` (C▲ C▼ C◀ C▶), `teclas_pretas_L_R_Z_A1`, `tecla_azul_A_A1`,
`tecla_verde_B_A1`, `tecla_vermelha_START_A1`. Os STL individuais estão em `teclas/`
(regerar com `.\gerar_teclas.ps1`).

## Fixação — sem parafusos
- **Tampa ↔ fundo:** 6 ganchos de encaixe (2 na frente, 2 atrás, 1 em cada lateral).
  Para fechar, alinhe e aperte até ouvir os cliques. Para abrir, puxe o fundo
  para fora da tampa com firmeza (os dentes têm rampa dos dois lados).
- **Pico:** encaixe os 4 furos da placa nos 4 pinos, com o USB virado para a
  parede traseira, e empurre a ponta oposta para baixo até a trava estalar.
  Para tirar, afaste a trava com a unha e levante essa ponta.
- Se o encaixe ficar duro demais ou frouxo, ajuste `snap_p` (dente da tampa)
  ou `latch_lip` (trava do Pico) em 0,2 mm e reimprima só o **fundo**.
- **Pico clone sem furos de fixação:** o berço de 4 pinos foi feito para o Pico
  oficial. Para um clone sem furos, as opções são: cortar os pinos e a trava com
  alicate e prender a placa com fita dupla face de espuma, ou modelar um berço que
  segure pelas bordas (precisa medir comprimento e largura da placa).

## Adesivos
`adesivos/adesivos_N64_Arcade_A4.pdf` — imprimir em 100% (conferir a régua de
50 mm). Contém: alerta do topo (72 × 42 mm), faixa de alerta traseira
(90 × 14 mm), faixas laterais (118 × 16 mm, 2×) e frontal (180 × 16 mm) com o
logo Indião Retro Games. Fonte editável: `adesivos/adesivo_alerta.html`.

## Fiação
`esquema_fiacao.html` — vista de cima e vista espelhada para soldar, lista de
corte com comprimentos, dois GND em corrente (pinos 3 e 28).

## Peças extras (opcionais)
- 4 pés de borracha Ø12 mm (adesivos)
- 1 abraçadeira pequena para prender o cabo N64

## Teste antes de imprimir tudo
1. Imprima **1 tecla** e confira o encaixe no switch. Se ficar frouxa, diminua
   `cross_w`/`cross_t` em 0,05 mm; se não entrar, aumente.
2. Se quiser, imprima só um pedaço da tampa para testar o furo de 14,1 mm.

## Editar
Abra `n64_arcade_hitbox.scad` no OpenSCAD. As posições dos botões estão na
lista `keys` (x, y em mm, centro da caixa = 0,0). Escolha a peça em `part`
(`assembly`, `top`, `bottom`, `keycap`) e exporte com F6 → STL.
