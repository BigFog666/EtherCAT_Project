param([switch]$Attach)
. (Join-Path $PSScriptRoot 'environment.ps1')
Push-Location $ProjectRoot
try {
    $gdb = Join-Path $ToolPaths.arm_root 'bin/arm-none-eabi-gdb.exe'
    # 默认复位到main；Attach仅暂停当前运行，保留模型和故障现场。
    $CommandFile = if ($Attach) { 'config/attach.gdb' } else { 'config/debug.gdb' }
    Invoke-Checked $gdb @('-x', $CommandFile)
} finally { Pop-Location }
