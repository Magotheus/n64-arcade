# Gera as 14 teclas com legenda (STL) na pasta teclas\
$ErrorActionPreference = 'Continue'
$o = 'C:\Program Files\OpenSCAD\openscad.com'
$scad = Join-Path $PSScriptRoot 'n64_arcade_hitbox.scad'
$dst = Join-Path $PSScriptRoot 'teclas'
New-Item -ItemType Directory -Force $dst | Out-Null
$teclas = [ordered]@{
  'cinza_cima'='U'; 'cinza_baixo'='D'; 'cinza_esquerda'='LF'; 'cinza_direita'='RT'
  'amarela_C_cima'='CU'; 'amarela_C_baixo'='CD'; 'amarela_C_esquerda'='CL'; 'amarela_C_direita'='CR'
  'azul_A'='A'; 'verde_B'='B'; 'preta_Z'='Z'; 'preta_L'='L'; 'preta_R'='R'; 'vermelha_START'='START'
}
foreach ($n in $teclas.Keys) {
  $l = $teclas[$n]
  & $o -o "$dst\tecla_$n.stl" -D 'part=\"keycap\"' -D "cap_legend=\`"$l\`"" $scad *>&1 | Select-String 'ERROR' | ForEach-Object { "$n : $_" }
  Write-Host "ok $n"
}
