# 离线读取固件 ELF；不连接 OpenOCD，不复位、暂停或烧录开发板。
. (Join-Path $PSScriptRoot 'environment.ps1')
$gdbExe = Join-Path $ToolPaths.arm_root 'bin/arm-none-eabi-gdb.exe'
$elfPath = Join-Path $ProjectRoot 'build/firmware-gcc/ethercat_joint.elf'
if (-not (Test-Path -LiteralPath $gdbExe)) { throw '未找到 GNU Arm GDB，请核对 config/tool_paths.json 中的 arm_root。' }
if (-not (Test-Path -LiteralPath $elfPath)) { throw '未找到固件 ELF，请先运行 tools/build.ps1 -Target Firmware。' }
Write-Host '离线 GDB：只载入本地 ELF，不连接开发板。'
Write-Host '看到 (gdb) 后可输入：info line Project_Poll、ptype Joint；输入 quit 退出。'
Push-Location $ProjectRoot
try {
    Invoke-Checked $gdbExe @('-q','-nx','-ex','set pagination off',$elfPath)
} finally { Pop-Location }
