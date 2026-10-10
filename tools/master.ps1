param(
    [ValidateSet('Help','List','Scan','Inspect','Reset','Demo','LedDemo','Offline')][string]$Action='Help',
    [string]$Interface='', [int]$Cycles=2000, [int]$PeriodUs=10000,
    [string]$Csv='evidence/logs/hardware.csv', [switch]$ResetBeforeDemo
)
. (Join-Path $PSScriptRoot 'environment.ps1')
$OldPath = $env:PATH
Push-Location $ProjectRoot
try {
    if ($Action -eq 'Offline') { Invoke-Checked (Join-Path $ProjectRoot 'build/host/offline_demo.exe') @(); return }
    Use-Npcap
    $arguments = @('--' + $Action.ToLowerInvariant())
    if ($Action -eq 'LedDemo') { $arguments = @('--demo'); if (-not $PSBoundParameters.ContainsKey('Cycles')) { $Cycles=6000 } }
    if ($Action -notin @('Help','List')) {
        if ([string]::IsNullOrWhiteSpace($Interface)) { throw '请使用 -Interface 传入 Windows 有线网卡名称（如“以太网”）或 List 输出中的完整接口名。' }
        # 完整 Npcap 接口名保持兼容；Windows 网卡名按精确名称解析，不自动猜选网卡。
        if (-not $Interface.StartsWith('\Device\NPF_', [System.StringComparison]::OrdinalIgnoreCase)) {
            try { $adapters = @(Get-NetAdapter -ErrorAction Stop | Where-Object { $_.Name -eq $Interface }) }
            catch { throw '无法读取 Windows 网卡名称。请使用 -Action List 列出的完整 Npcap 接口名，或检查当前终端权限。' }
            if ($adapters.Count -ne 1) { throw "未找到唯一的网卡 '$Interface'。请运行 Get-NetAdapter 核对名称，或使用 List 的完整接口名。" }
            $adapter = $adapters[0]
            if ($adapter.Status -ne 'Up') { throw "网卡 '$Interface' 当前状态为 $($adapter.Status)，请先核对开发板供电与网线连接。" }
            $Interface = '\Device\NPF_{' + ([guid]$adapter.InterfaceGuid).ToString().ToUpperInvariant() + '}'
            Write-Host "使用网卡：$($adapter.Name) / $($adapter.InterfaceDescription)"
            Write-Host "Npcap 接口：$Interface"
        }
        $arguments += $Interface
    }
    if ($Action -eq 'LedDemo') { $arguments += '--visual' }
    if ($Action -in @('Demo','LedDemo')) { $arguments += @('--cycles',"$Cycles",'--period-us',"$PeriodUs",'--csv',$Csv) }
    if ($Action -in @('Demo','LedDemo') -and $ResetBeforeDemo) { $arguments += '--reset-start' }
    Invoke-Checked (Join-Path $ProjectRoot 'build/host/joint_master.exe') $arguments
} finally { $env:PATH = $OldPath; Pop-Location }
