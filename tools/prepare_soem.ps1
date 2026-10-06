param([string]$Source='')
. (Join-Path $PSScriptRoot 'environment.ps1')
$expected = '88e8ed46efba7dfa7b94d08a512db25a33e3f8d5'
$dest = Join-Path $ProjectRoot 'vendor/SOEM'
if (Test-Path -LiteralPath $dest) { throw 'vendor/SOEM 已存在，不覆盖；修改前保留本地成果。' }
if ($Source) {
    $Source = (Resolve-Path -LiteralPath $Source).Path
    $actual = & git -c "safe.directory=$($Source.Replace('\','/'))" -C $Source rev-parse HEAD
    if ($LASTEXITCODE -ne 0 -or $actual -ne $expected) { throw 'SOEM 源码版本不符，拒绝复制。' }
    New-Item -ItemType Directory -Path $dest | Out-Null
    Get-ChildItem -LiteralPath $Source -Force | Where-Object { $_.Name -ne '.git' } | ForEach-Object {
        Copy-Item -LiteralPath $_.FullName -Destination $dest -Recurse
    }
} else {
    # 独立仓库直接准备固定依赖，不要求存在原软件课程目录。
    New-Item -ItemType Directory -Path (Split-Path -Parent $dest) -Force | Out-Null
    Invoke-Checked 'git' @('clone', '--no-checkout', 'https://github.com/OpenEtherCATsociety/SOEM.git', $dest)
    Invoke-Checked 'git' @('-C', $dest, 'checkout', '--detach', $expected)
    $actual = & git -C $dest rev-parse HEAD
    if ($LASTEXITCODE -ne 0 -or $actual -ne $expected) { throw '下载的 SOEM 版本核对失败。' }
}
[IO.File]::WriteAllText((Join-Path $dest 'PROJECT_SOURCE_COMMIT.txt'), $expected, [Text.Encoding]::UTF8)
Write-Host "已准备固定 SOEM 依赖：$dest（vendor 不进入项目 Git）"
