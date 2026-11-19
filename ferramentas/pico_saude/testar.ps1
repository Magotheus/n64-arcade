# Espera um Pico em BOOTSEL, grava o diagnóstico e mostra o relatório.
# Uso: .\testar.ps1      (depois conecte o Pico segurando BOOTSEL)
param([int]$EsperaSegundos = 180)
$uf2 = Join-Path $PSScriptRoot 'pico_saude.uf2'

Write-Host "Aguardando Pico em modo BOOTSEL (unidade RPI-RP2)..."
$fim = (Get-Date).AddSeconds($EsperaSegundos); $v = $null
while ((Get-Date) -lt $fim -and -not $v) { $v = Get-Volume | Where-Object FileSystemLabel -eq 'RPI-RP2'; if (-not $v) { Start-Sleep 1 } }
if (-not $v) { Write-Host "RPI-RP2 nao apareceu."; exit 1 }
Get-Content "$($v.DriveLetter):\INFO_UF2.TXT"
Copy-Item $uf2 "$($v.DriveLetter):\"
Write-Host "Diagnostico gravado. Esperando a porta serial..."

$fim = (Get-Date).AddSeconds(20); $c = $null
while ((Get-Date) -lt $fim -and -not $c) {
    Start-Sleep 1
    $c = Get-CimInstance Win32_PnPEntity | Where-Object { $_.DeviceID -match 'VID_2E8A&PID_000A&MI_00' -and $_.Name -match 'COM\d+' }
}
if (-not $c) { Write-Host "Porta serial nao apareceu (a placa nao iniciou o programa)."; exit 2 }
$null = $c.Name -match '\((COM\d+)\)'; $porta = $Matches[1]
Write-Host "Lendo $porta ..."

function Ler([int]$seg, [string]$enviar) {
    $out = ''
    $fim = (Get-Date).AddSeconds($seg)
    try {
        $p = New-Object System.IO.Ports.SerialPort $porta, 115200
        $p.DtrEnable = $true; $p.ReadTimeout = 500; $p.WriteTimeout = 500
        $p.Open()
        if ($enviar) { Start-Sleep 1; $p.Write($enviar) }
        while ((Get-Date) -lt $fim -and $out -notmatch 'FIM_DO_DIAGNOSTICO') {
            Start-Sleep -Milliseconds 200
            $out += $p.ReadExisting()
        }
        $p.Close()
    } catch { $out += "`n[conexao interrompida: $($_.Exception.Message)]" }
    return $out
}

$r = Ler 30 ''
Write-Host $r
if ($r -notmatch 'FIM_DO_DIAGNOSTICO') {
    Write-Host "`n(sem fim do relatorio - aguardando o watchdog reiniciar a placa...)"
    Start-Sleep 12
    $c = Get-CimInstance Win32_PnPEntity | Where-Object { $_.DeviceID -match 'VID_2E8A&PID_000A&MI_00' -and $_.Name -match 'COM\d+' }
    if ($c) { $null = $c.Name -match '\((COM\d+)\)'; $porta = $Matches[1]; Write-Host (Ler 15 '') }
    else { Write-Host "A placa nao voltou." }
}
