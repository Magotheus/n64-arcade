# Registro continuo da telemetria do Pico (N64 Arcade) em arquivo, com hora em cada linha.
#
# Uso (PowerShell, dentro da pasta ferramentas):
#   powershell -ExecutionPolicy Bypass -File .\registro_telemetria.ps1
#     -> grava em ..\diagnostico\telemetria\AAAA-MM-DD_HHMM.txt
#   powershell -ExecutionPolicy Bypass -File .\registro_telemetria.ps1 -Saida C:\caminho\log.txt
#
# Para ver ao vivo em outra janela:
#   Get-Content -Path <arquivo> -Wait -Tail 40
#
# - Acha o Pico sozinho (USB VID 2E8A), espera a porta ficar livre e reconecta se o USB cair.
# - NAO pode rodar junto com o painel_controle.html / teste_controle.html / monitor.ps1
#   (todos usam a mesma porta). Ctrl+C para parar.
param([string]$Saida)

if (-not $Saida) {
    $pasta = Join-Path $PSScriptRoot '..\diagnostico\telemetria'
    New-Item -ItemType Directory -Force $pasta | Out-Null
    $Saida = Join-Path $pasta ((Get-Date -Format 'yyyy-MM-dd_HHmm') + '.txt')
}
Write-Host "Gravando em: $Saida"
"$(Get-Date -Format 'HH:mm:ss.fff')  registro iniciado, aguardando a porta" | Out-File $Saida -Encoding utf8

while ($true) {
    $d = Get-CimInstance Win32_PnPEntity | Where-Object { $_.DeviceID -match 'VID_2E8A&PID_000A&MI_00' -and $_.Name -match '\((COM\d+)\)' } | Select-Object -First 1
    if (-not $d) { Start-Sleep -Milliseconds 500; continue }
    $null = $d.Name -match '\((COM\d+)\)'; $com = $Matches[1]
    $p = New-Object System.IO.Ports.SerialPort $com, 115200
    $p.DtrEnable = $true; $p.ReadTimeout = 500
    try { $p.Open() } catch { Start-Sleep -Milliseconds 700; continue }
    "$(Get-Date -Format 'HH:mm:ss.fff')  conectado em $com" | Out-File $Saida -Append -Encoding utf8
    Write-Host "Conectado em $com"
    $buf = ''
    try {
        while ($p.IsOpen) {
            $buf += $p.ReadExisting()
            while (($i = $buf.IndexOf("`n")) -ge 0) {
                $l = $buf.Substring(0, $i).Trim(); $buf = $buf.Substring($i + 1)
                if ($l) { "$(Get-Date -Format 'HH:mm:ss.fff')  $l" | Out-File $Saida -Append -Encoding utf8 }
            }
            Start-Sleep -Milliseconds 50
        }
    } catch {
        "$(Get-Date -Format 'HH:mm:ss.fff')  conexao caiu: $($_.Exception.Message)" | Out-File $Saida -Append -Encoding utf8
    } finally { try { $p.Close() } catch {} }
    Start-Sleep -Milliseconds 500
}
