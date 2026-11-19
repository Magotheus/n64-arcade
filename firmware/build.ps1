# Compila o firmware e copia o .uf2 para esta pasta.
# Uso (PowerShell, dentro da pasta firmware):
#   .\build.ps1                 -> Pico W   -> n64_mechanical_pico_w.uf2
#   .\build.ps1 -Placa pico     -> Pico comum (inclui clones) -> n64_mechanical_pico.uf2
param([ValidateSet('pico_w','pico')][string]$Placa = 'pico_w')

$ErrorActionPreference = 'Continue'  # o CMake escreve avisos no stderr; checamos $LASTEXITCODE
$env:Path = [Environment]::GetEnvironmentVariable('Path','Machine') + ';' + [Environment]::GetEnvironmentVariable('Path','User')
$pico = "$env:USERPROFILE\.pico-sdk"
$env:PICO_SDK_PATH = "$pico\sdk\2.2.0"
# Arm GNU Toolchain extraido em .pico-sdk\toolchain (se nao estiver no PATH do sistema)
$arm = Get-ChildItem "$pico\toolchain" -Directory -ErrorAction SilentlyContinue | Select-Object -First 1
if ($arm -and (Test-Path "$($arm.FullName)\bin\arm-none-eabi-gcc.exe")) { $env:Path = "$($arm.FullName)\bin;" + $env:Path }
# Pasta de build com caminho curto (evita o limite de 260 caracteres do Windows)
$bd = "$pico\build\n64_$Placa"

cmake -G Ninja -B $bd -S $PSScriptRoot `
    "-DPICO_BOARD=$Placa" `
    "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" `
    "-Dpicotool_DIR=$pico\picotool\2.2.0-a4\picotool" `
    "-Dpioasm_DIR=$pico\tools\2.2.0\pioasm"
if ($LASTEXITCODE -ne 0) { throw "Falha ao configurar o CMake" }

cmake --build $bd
if ($LASTEXITCODE -ne 0) { throw "Falha na compilacao" }

$saida = "$PSScriptRoot\n64_mechanical_$Placa.uf2"
Copy-Item "$bd\n64_mechanical.uf2" $saida -Force
Write-Host "OK: $saida"
