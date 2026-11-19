// =====================================================================
//  N64 Arcade — hitbox compacto (V1 da caixa)
//  14 switches MX + Raspberry Pi Pico W
//
//  Peças:
//    part = "top"      -> tampa (topo + paredes). Imprimir DE CABEÇA PARA BAIXO.
//    part = "bottom"   -> fundo com suportes do Pico W e da abraçadeira do cabo.
//    part = "assembly" -> montagem com keycaps coloridas (só visualização).
//
//  Fixação SEM PARAFUSOS:
//    - 6 ganchos de encaixe (snap-fit) no fundo travam em canaletas por
//      dentro das paredes da tampa. Fecha com pressão; abre puxando.
//    - Pico: 4 pinos atravessam os furos da placa + 1 trava flexível na
//      ponta oposta ao USB.
// =====================================================================

part = "assembly"; // [assembly, top, bottom, keycap]

$fn = 48;

// ---------------------------------------------------------- Caixa
W      = 240;   // largura  (cabe em mesa de 256 mm)
D      = 128;   // profundidade
H      = 28;    // altura total (fundo + tampa)
R      = 10;    // raio dos cantos
bevel  = 2.5;   // chanfro da borda superior
wall   = 2.5;   // parede
top_t  = 4;     // espessura do topo
bot_t  = 3;     // espessura do fundo
fit    = 0.3;   // folga do encaixe fundo/tampa

// ---------------------------------------------------------- Switches MX
mx_hole   = 14.1;  // furo quadrado (14,1 funcionou na base V2)
plate_t   = 1.5;   // espessura onde a trava do switch prende
mx_pocket = 15.6;  // rebaixo por baixo para as travas

// ---------------------------------------------------------- Layout
// [nome, x, y, cor da keycap]   (origem no centro da caixa, y+ = fundo)
// Inspirado no arcade stick N64 de referência:
// Mão esquerda (dedos): ◄ ▼ ►      | polegar esquerdo: ▲
// Mão direita: coluna B/A, quadrado dos C (◄ ▲ / ▼ ►), coluna L/R
//              polegar direito: Z   | Start no canto superior esquerdo
// Teclas redondas Ø24 mm (estilo fliperama) -> espaçamento ~27 mm.
keys = [
  ["ESQUERDA",   -92,   8, "#333333"],
  ["BAIXO",      -65,  13, "#333333"],
  ["DIREITA",    -38,   6, "#333333"],
  ["CIMA",       -19, -30, "#333333"],

  ["B",           18,   8, "#2E9E44"],
  ["A",           14, -18, "#2456C8"],

  ["C_ESQUERDA",  45,  16, "#F2C200"],
  ["C_CIMA",      72,  16, "#F2C200"],
  ["C_BAIXO",     42, -10, "#F2C200"],
  ["C_DIREITA",   69, -10, "#F2C200"],

  ["L",           99,  12, "#222222"],
  ["R",           96, -14, "#222222"],

  ["Z",           23, -45, "#222222"],

  ["START",      -99,  45, "#D42A2A"],
];

// ---------------------------------------------------------- Tecla redonda MX
cap_d      = 24;    // diâmetro
cap_h      = 6;     // altura do corpo
cap_round  = 1.8;   // arredondamento da borda superior
cap_dish   = 0;     // concavidade no topo (0 = plano; >0 tira o apoio na mesa ao imprimir)
stem_d     = 5.6;   // pino que encaixa no switch
stem_len   = 5;     // comprimento do pino abaixo do corpo
cross_w    = 4.15;  // cruz MX (4,0 nominal + folga de impressão)
cross_t    = 1.35;  // largura do braço da cruz (1,17 nominal + folga)
cross_depth = 4;

// ---------------------------------------------------------- Legendas das teclas
// Gravadas em baixo relevo no topo. Códigos:
//   "A" "B" "Z" "L" "R" "START"
//   "CU" "CD" "CL" "CR"  -> C com seta (cima, baixo, esquerda, direita)
//   "U" "D" "LF" "RT"    -> setas dos direcionais
cap_legend     = "";      // usada por part = "keycap"
legend_depth   = 0.6;     // profundidade da gravação
legend_font    = "Liberation Sans:style=Bold";
legend_letter  = 10;      // tamanho das letras A B Z L R
legend_arrow   = 10;      // tamanho das setas dos direcionais

function legend_for(id) =
  id == "ESQUERDA"   ? "LF" : id == "DIREITA" ? "RT" :
  id == "CIMA"       ? "U"  : id == "BAIXO"   ? "D"  :
  id == "C_CIMA"     ? "CU" : id == "C_BAIXO" ? "CD" :
  id == "C_ESQUERDA" ? "CL" : id == "C_DIREITA" ? "CR" : id;

// ---------------------------------------------------------- Pico W
pico_w = 21;  pico_l = 51;
pico_hole_dx = 11.4;  pico_hole_dy = 47.0;
standoff_h = 4;
pico_cy = D/2 - wall - 1 - pico_l/2;       // USB encostado na parede traseira
usb_w = 12; usb_h = 8;
usb_z = bot_t + standoff_h + 1.6;          // altura do conector micro-USB

// ---------------------------------------------------------- Cabo N64
cable_x = 32;   cable_d = 7;   cable_z = bot_t + 7;

// ---------------------------------------------------------- Encaixe (snap-fit)
// Braço vertical que sai do fundo, encostado por dentro da parede da tampa,
// com um dente que entra numa canaleta da parede. Rampas de ~40° nos dois
// lados do dente: fecha empurrando e abre puxando (sem ferramenta).
snap_w     = 10;    // largura do gancho (ao longo da parede)
snap_t     = 1.6;   // espessura do braço -> flexibilidade (PLA)
snap_h     = 14;    // altura do braço acima do fundo
snap_p     = 1.1;   // quanto o dente sai em direção à parede
snap_tooth = 2.8;   // altura do dente
snap_clear = 0.3;   // folga da canaleta
// [x, y, rotação]: rotação 0 = parede traseira (+y), 180 = frente,
// -90 = lateral direita (+x), 90 = lateral esquerda (-x)
snap_face_y = D/2 - wall - fit;
snap_face_x = W/2 - wall - fit;
snaps = [
  [-70,  snap_face_y,   0], [ 70,  snap_face_y,   0],
  [-70, -snap_face_y, 180], [ 70, -snap_face_y, 180],
  [ snap_face_x, 0, -90],   [-snap_face_x, 0,  90],
];

// ---------------------------------------------------------- Berço do Pico
pico_peg_d   = 1.9;   // pino que passa no furo de 2,1 mm da placa
pico_peg_h   = 1.8;
pico_board_t = 1.0;
latch_w = 5;  latch_t = 1.2;  latch_lip = 0.6;

// ---------------------------------------------------------- Pés
feet = [[ W/2-18, D/2-15], [-W/2+18, D/2-15],
        [ W/2-18,-D/2+15], [-W/2+18,-D/2+15]];
foot_d = 12.5;  foot_depth = 1.0;

// ---------------------------------------------------------- Logo
logo_text = "N64 ARCADE";
logo_pos  = [45, 46];
logo_size = 7;
logo_depth = 0.6;

// =====================================================================
module rrect(w, d, r, h) {
  linear_extrude(h) offset(r = r) square([w - 2*r, d - 2*r], center = true);
}

// Coloca um elemento de encaixe na posição/rotação de cada gancho.
module at_snaps() {
  for (s = snaps) translate([s[0], s[1], 0]) rotate([0, 0, s[2]]) children();
}

// Gancho no referencial local: parede em +y, face externa do braço em y = 0.
module snap_hook() {
  // braço
  translate([-snap_w/2, -snap_t, 0]) cube([snap_w, snap_t, snap_h]);
  // dente com rampa de entrada (topo) e de saída (baixo)
  rotate([90, 0, 90]) translate([0, 0, -snap_w/2]) linear_extrude(snap_w)
    polygon([
      [-0.01,  snap_h - snap_tooth],
      [snap_p, snap_h - snap_tooth + 1.0],
      [snap_p, snap_h - 0.9],
      [-0.01,  snap_h],
    ]);
}

// Canaleta na parede da tampa (mesmo referencial; z = 0 na borda da tampa).
module snap_groove() {
  translate([-(snap_w + 1)/2, -0.01, snap_h - snap_tooth - snap_clear])
    cube([snap_w + 1, fit + snap_p + snap_clear + 0.01, snap_tooth + 2*snap_clear]);
}

module top_shell() {
  shell_h = H - bot_t;
  difference() {
    difference() {
      // casca externa com chanfro no topo
      hull() {
        rrect(W, D, R, shell_h - bevel);
        translate([0,0,shell_h - 0.01])
          rrect(W - 2*bevel, D - 2*bevel, R - bevel, 0.01);
      }
      // oco interno
      translate([0,0,-1])
        rrect(W - 2*wall, D - 2*wall, R - wall, shell_h - top_t + 1);
    }
    // canaletas dos ganchos
    at_snaps() snap_groove();

    // switches: furo passante + rebaixo para as travas
    for (k = keys) translate([k[1], k[2], 0]) {
      translate([0,0,shell_h/2]) cube([mx_hole, mx_hole, shell_h + 2], center = true);
      translate([-mx_pocket/2, -mx_pocket/2, -1])
        cube([mx_pocket, mx_pocket, shell_h - plate_t + 1]);
    }

    // abertura do micro-USB (parede traseira)
    translate([-usb_w/2, D/2 - wall - 1, usb_z - bot_t - usb_h/2])
      cube([usb_w, wall + 2, usb_h]);

    // saída do cabo N64
    translate([cable_x, D/2 + 1, cable_z - bot_t]) rotate([90,0,0])
      cylinder(d = cable_d, h = wall + 2);

    // logo gravado
    translate([logo_pos[0], logo_pos[1], shell_h - logo_depth])
      linear_extrude(logo_depth + 1)
        text(logo_text, size = logo_size, font = "Liberation Sans:style=Bold",
             halign = "center", valign = "center");
  }
}

// Trava do Pico: braço flexível na ponta oposta ao USB, com lábio por cima
// da placa. Nasce num rebaixo do fundo para ter braço mais longo (flexiona mais).
latch_y0 = pico_cy - pico_l/2 - 0.2;           // face do braço encostada na placa
latch_z0 = 1.0;                                 // nasce 1 mm acima da base
board_top = bot_t + standoff_h + pico_board_t;

module pico_latch() {
  translate([-latch_w/2, latch_y0 - latch_t, latch_z0])
    cube([latch_w, latch_t, board_top + 0.15 + 1.0 - latch_z0]);
  // lábio com chanfro de entrada
  translate([latch_w/2, 0, 0]) rotate([0, -90, 0]) linear_extrude(latch_w)
    polygon([
      [board_top + 0.15,       latch_y0 - 0.01],
      [board_top + 0.15,       latch_y0 + latch_lip],
      [board_top + 0.15 + 0.4, latch_y0 + latch_lip],
      [board_top + 0.15 + 1.2, latch_y0 - 0.01],
    ]);
}

module bottom_plate() {
  difference() {
    union() {
      rrect(W, D, R, bot_t);
      // ressalto que encaixa por dentro da tampa
      translate([0,0,bot_t - 0.01]) difference() {
        rrect(W - 2*wall - 2*fit, D - 2*wall - 2*fit, R - wall - fit, 2);
        translate([0,0,-1])
          rrect(W - 2*wall - 2*fit - 3, D - 2*wall - 2*fit - 3, R - wall - fit - 1.5, 4);
      }
      // suportes do Pico com pinos de encaixe
      for (sx = [-1,1], sy = [-1,1])
        translate([sx*pico_hole_dx/2, pico_cy + sy*pico_hole_dy/2, bot_t - 0.01]) {
          cylinder(d = 5, h = standoff_h + 0.01);
          translate([0, 0, standoff_h]) cylinder(d = pico_peg_d, h = pico_peg_h);
          translate([0, 0, standoff_h + pico_peg_h])
            cylinder(d1 = pico_peg_d, d2 = pico_peg_d - 0.6, h = 0.4);
        }
      // ancoragem da abraçadeira do cabo
      translate([cable_x - 6, D/2 - wall - 16, bot_t - 0.01])
        difference() {
          cube([12, 6, 6]);
          translate([-1, 1.5, 1.5]) cube([14, 3, 2.2]);
        }
    }
    // rebaixo em volta da trava do Pico (braço mais longo = mais flexível)
    translate([-latch_w/2 - 1, latch_y0 - latch_t - 1.2, latch_z0])
      cube([latch_w + 2, latch_t + 1.2 + 0.4, bot_t]);
    // rebaixos para pés de borracha
    for (f = feet) translate([f[0], f[1], -1]) cylinder(d = foot_d, h = foot_depth + 1);
  }
  // ganchos (fora do difference para não serem cortados)
  translate([0, 0, bot_t - 0.01]) at_snaps() snap_hook();
  pico_latch();
}

// Tecla redonda estilo fliperama com encaixe MX.
// Origem na base do corpo; o pino desce em -z.
// Seta triangular apontando para "ang" (0 = cima, 90 = esquerda, -90 = direita).
module legend_tri(ang, s) {
  rotate(ang) polygon([[0, s * 0.55], [-s * 0.52, -s * 0.38], [s * 0.52, -s * 0.38]]);
}

module legend2d(l) {
  arrows = [["U", 0], ["D", 180], ["LF", 90], ["RT", -90]];
  c_arrows = [["CU", 0], ["CD", 180], ["CL", 90], ["CR", -90]];
  a  = search([l], arrows)[0];
  ca = search([l], c_arrows)[0];
  if (a != [] && a != undef) legend_tri(arrows[a][1], legend_arrow);
  else if (ca != [] && ca != undef) {
    translate([-2.9, 0]) text("C", size = 8, font = legend_font, halign = "center", valign = "center");
    translate([5.0, 0]) legend_tri(c_arrows[ca][1], 5.2);
  }
  else if (l == "START") text("START", size = 3.5, font = legend_font, halign = "center", valign = "center");
  else if (l != "") text(l, size = legend_letter, font = legend_font, halign = "center", valign = "center");
}

module round_keycap(l = "") {
  difference() {
    union() {
      // corpo com borda superior arredondada
      rotate_extrude($fn = 96) hull() {
        square([cap_d/2, 0.01]);
        square([0.01, cap_h]);
        translate([cap_d/2 - cap_round, cap_h - cap_round]) circle(r = cap_round, $fn = 32);
        translate([cap_d/2 - 0.01, 0]) square([0.01, cap_h - cap_round]);
      }
      // pino
      translate([0,0,-stem_len]) cylinder(d = stem_d, h = stem_len + 0.01);
    }
    // cruz MX
    translate([0,0,-stem_len - 0.01]) linear_extrude(cross_depth) {
      square([cross_w, cross_t], center = true);
      square([cross_t, cross_w], center = true);
    }
    // concavidade suave no topo
    if (cap_dish > 0) {
      dish_r = (pow(cap_d/2 - cap_round, 2) + pow(cap_dish, 2)) / (2*cap_dish);
      translate([0,0,cap_h + dish_r - cap_dish]) sphere(r = dish_r, $fn = 128);
    }
    // legenda em baixo relevo
    if (l != "") translate([0, 0, cap_h - legend_depth])
      linear_extrude(legend_depth + 1) legend2d(l);
  }
}

module keycap(c, l = "") { color(c) round_keycap(l); }

module pico_dummy() {
  color("ForestGreen") translate([-pico_w/2, pico_cy - pico_l/2, bot_t + standoff_h])
    cube([pico_w, pico_l, 1]);
  color("Silver") translate([-4, D/2 - wall - 1 - 5, bot_t + standoff_h + 1])
    cube([8, 5.5, 2.6]);
}

// =====================================================================
if (part == "top") {
  // de cabeça para baixo, pronto para fatiar
  rotate([180,0,0]) translate([0,0,-(H - bot_t)]) top_shell();
} else if (part == "bottom") {
  bottom_plate();
} else if (part == "none") {
  // nada (usado quando este arquivo é incluído por outro)
} else if (part == "check") {
  // Verificação: o que sobra aqui é interferência entre fundo e tampa fechados.
  intersection() { bottom_plate(); translate([0,0,bot_t]) top_shell(); }
} else if (part == "keycap") {
  // de cabeça para baixo (topo na mesa, pino para cima): imprime sem suporte
  rotate([180,0,0]) translate([0,0,-cap_h]) round_keycap(cap_legend);
} else {
  color("#3A3A3A") bottom_plate();
  pico_dummy();
  color("#BFBFBF") translate([0,0,bot_t]) top_shell();
  for (k = keys) translate([k[1], k[2], H + 6.6 + 1]) keycap(k[3], legend_for(k[0]));
}
