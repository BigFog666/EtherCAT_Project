param([Parameter(Mandatory=$true, Position=0)][string]$Csv)
. (Join-Path $PSScriptRoot 'environment.ps1')
# 用已有日志做离线统计，不访问网卡或开发板。
Push-Location $ProjectRoot
try {
    if (-not (Test-Path -LiteralPath $Csv -PathType Leaf)) { throw "未找到日志文件：$Csv。请复制真实的 evidence/logs 文件名。" }
    Invoke-Checked $ToolPaths.python @('-X','utf8',(Join-Path $ProjectRoot 'tools/analyze_log.py'),$Csv)
} finally { Pop-Location }
