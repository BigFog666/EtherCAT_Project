param([switch]$Help)
if ($Help) {
    Write-Host '用法：.\tools\reset.ps1'
    Write-Host '先结束主站和OpenOCD/GDB。仅复位MCU并运行现有固件，不下载，不写EEPROM。'
    Write-Host '复位会清除RAM故障现场；想查看当前错误码时，先用debug.ps1 -Attach。'
    return
}
. (Join-Path $PSScriptRoot 'environment.ps1')
Push-Location $ProjectRoot
try {
    # MCU重新初始化与CiA402故障复位是两种操作；此处会重新执行main。
    Write-Host '复位MCU并重新执行现有固件；RAM故障现场将被清除。'
    $ResetArguments = @('-f','interface/stlink.cfg','-f','target/stm32f4x.cfg','-c','adapter speed 1000','-c','init; reset run; shutdown')
    Invoke-Checked $ToolPaths.openocd $ResetArguments
} finally { Pop-Location }
