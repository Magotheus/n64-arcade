// Prévia das 14 teclas com legenda (só visualização)
include <n64_arcade_hitbox.scad>

legs = [["U","#555555"],["D","#555555"],["LF","#555555"],["RT","#555555"],
        ["CU","#E2B400"],["CD","#E2B400"],["CL","#E2B400"],["CR","#E2B400"],
        ["A","#2456C8"],["B","#2E9E44"],["Z","#333333"],["L","#333333"],["R","#333333"],["START","#D42A2A"]];
for (i = [0 : len(legs) - 1])
  translate([(i % 7) * 30, -floor(i / 7) * 30, 0]) color(legs[i][1]) round_keycap(legs[i][0]);
