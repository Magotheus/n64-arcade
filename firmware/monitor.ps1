# Monitor serial ao vivo do N64 Mechanical Controller (versao legivel).
# Mostra APERTOU / SOLTOU para cada botao, em vez do estado bruto.
# Uso: .\monitor.ps1 [-Porta COM7] [-Bruto]     (Ctrl+C para sair; sem -Porta acha o Pico sozinho)
param([string]$Porta = '', [switch]$Bruto)

# Sem -Porta: procura sozinho a porta serial do Pico (USB VID 2E8A).
if (-not $Porta) {
    Write-Host "Procurando o Pico..." -ForegroundColor DarkGray
    $fim = (Get-Date).AddSeconds(60)
    while (-not $Porta -and (Get-Date) -lt $fim) {
        $d = Get-CimInstance Win32_PnPEntity | Where-Object { $_.DeviceID -match 'VID_2E8A&PID_000A&MI_00' -and $_.Name -match '\((COM\d+)\)' } | Select-Object -First 1
        if ($d) { $null = $d.Name -match '\((COM\d+)\)'; $Porta = $Matches[1] } else { Start-Sleep 1 }
    }
    if (-not $Porta) { Write-Host "Pico nao encontrado. Ele esta conectado e com o firmware N64 gravado?" -ForegroundColor Red; exit 1 }
    Write-Host "Pico encontrado em $Porta" -ForegroundColor Green
}

$nomesDir = @{ '^' = 'CIMA'; 'v' = 'BAIXO'; '<' = 'ESQUERDA'; '>' = 'DIREITA' }

function Ler-Estado([string]$linha) {
    # Formato: "X=   0 Y=   0 DPAD=^> | A B"
    $set = New-Object System.Collections.Generic.HashSet[string]
    if ($linha -notmatch 'DPAD=(\S*)\s*\|(.*)$') { return $null }
    foreach ($c in $Matches[1].ToCharArray()) { [void]$set.Add($nomesDir["$c"]) }
    foreach ($n in ($Matches[2].Trim() -split '\s+')) { if ($n) { [void]$set.Add($n) } }
    return ,$set
}

function Abrir-Porta {
    $avisou = $false
    while ($true) {
        $np = New-Object System.IO.Ports.SerialPort $Porta, 115200
        $np.DtrEnable = $true
        try { $np.Open(); return $np } catch {
            if (-not $avisou) {
                Write-Host "Aguardando $Porta (USB desconectado ou porta em uso pelo Thonny)..." -ForegroundColor DarkYellow
                $avisou = $true
            }
            Start-Sleep -Milliseconds 500
        }
    }
}

$p = Abrir-Porta
Write-Host "Monitorando $Porta - aperte UM botao por vez (Ctrl+C para sair)" -ForegroundColor Green
Write-Host "Se aparecer em AMARELO '(ao mesmo tempo: ...)' sem voce apertar dois botoes, ha fio encostando." -ForegroundColor DarkGray

$anterior = New-Object System.Collections.Generic.HashSet[string]
$buf = ''
try {
    while ($true) {
        try { $buf += $p.ReadExisting() } catch {
            Write-Host "--- USB desconectado ---" -ForegroundColor DarkYellow
            try { $p.Close() } catch {}
            $p = Abrir-Porta
            Write-Host "--- reconectado: aperte os botoes ---" -ForegroundColor Green
            $anterior = New-Object System.Collections.Generic.HashSet[string]
            $buf = ''
            continue
        }
        while ($buf.Contains("`n")) {
            $i = $buf.IndexOf("`n")
            $linha = $buf.Substring(0, $i).Trim()
            $buf = $buf.Substring($i + 1)
            if ($Bruto) { Write-Host $linha; continue }
            if ($linha -like '`[N64*') {   # aviso periodico de "vivo"
                # Firmware novo informa quantos comandos de acessorio (pak) o console mandou
                if ($linha -match 'pak_read=(\d+), pak_write=(\d+)') {
                    $pak = "$($Matches[1])/$($Matches[2])"
                    if ($pak -ne $script:pakAnterior) {
                        $script:pakAnterior = $pak
                        Write-Host "$((Get-Date).ToString('HH:mm:ss'))  console consultou acessorio (leituras/escritas): $pak" -ForegroundColor Magenta
                    }
                }
                continue
            }
            $atual = Ler-Estado $linha
            if ($null -eq $atual) { continue }
            $hora = (Get-Date).ToString('HH:mm:ss')
            foreach ($b in $atual) { if (-not $anterior.Contains($b)) {
                Write-Host "$hora  APERTOU  $b" -ForegroundColor Cyan } }
            foreach ($b in $anterior) { if (-not $atual.Contains($b)) {
                Write-Host "$hora  soltou   $b" -ForegroundColor DarkGray } }
            if ($atual.Count -ge 2) {
                Write-Host "          (ao mesmo tempo: $(@($atual) -join ' + '))" -ForegroundColor Yellow
            }
            $anterior = $atual
        }
        Start-Sleep -Milliseconds 20
    }
} finally { $p.Close() }
