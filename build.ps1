param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",

    [ValidateSet("x64", "x86")]
    [string]$Platform = "x64",

    [string]$CoreClrPath = "",
    [string]$CorlibDir = ""
)

$ErrorActionPreference = "Stop"

function Resolve-MSBuild {
    $msbuildCmd = Get-Command msbuild -ErrorAction SilentlyContinue
    if ($msbuildCmd) {
        return $msbuildCmd.Source
    }

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $installPath = & $vswhere -latest -requires Microsoft.Component.MSBuild -property installationPath
        if ($installPath) {
            $candidate = Join-Path $installPath "MSBuild\Current\Bin\MSBuild.exe"
            if (Test-Path $candidate) {
                return $candidate
            }
        }
    }

    throw "MSBuild not found. Run in Developer PowerShell for VS or install Visual Studio Build Tools."
}

function Resolve-CoreClr {
    $runtimeRoot = Join-Path $env:ProgramFiles "dotnet\shared\Microsoft.NETCore.App"
    if (-not (Test-Path $runtimeRoot)) {
        throw "Cannot auto-detect CoreCLR: .NET runtime folder not found: $runtimeRoot"
    }

    $runtimeDirs = Get-ChildItem -Path $runtimeRoot -Directory | Where-Object {
        $_.Name -like "6.*"
    } | Sort-Object { [version]$_.Name } -Descending

    if (-not $runtimeDirs -or $runtimeDirs.Count -eq 0) {
        throw "Cannot auto-detect CoreCLR: no Microsoft.NETCore.App 6.x runtime installed."
    }

    $selectedDir = $runtimeDirs[0].FullName
    $selectedCoreClr = Join-Path $selectedDir "coreclr.dll"
    if (-not (Test-Path $selectedCoreClr)) {
        throw "Cannot auto-detect CoreCLR: coreclr.dll not found in $selectedDir"
    }

    return @{
        CoreClrPath = $selectedCoreClr
        CorlibDir   = $selectedDir
    }
}

function Update-Il2CppConfig {
    param(
        [string]$ConfigPath,
        [string]$NewCoreClrPath,
        [string]$NewCorlibDir
    )

    $content = Get-Content -Path $ConfigPath -Raw
    $content = [regex]::Replace($content, "(?m)^coreclr_path=.*$", "coreclr_path=$NewCoreClrPath")
    $content = [regex]::Replace($content, "(?m)^corlib_dir=.*$", "corlib_dir=$NewCorlibDir")
    Set-Content -Path $ConfigPath -Value $content -Encoding UTF8
}

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$slnPath = Join-Path $root "Lizard.sln"
$bootstrapProj = Join-Path $root "DoorstopBootstrap\DoorstopBootstrap.csproj"
$releaseDir = Join-Path $root "Release"
$resWinHttp = Join-Path $root "res\winhttp.dll"

if (-not (Test-Path $slnPath)) {
    throw "Solution not found: $slnPath"
}
if (-not (Test-Path $bootstrapProj)) {
    throw "Bootstrap project not found: $bootstrapProj"
}

$msbuild = Resolve-MSBuild

Write-Host "==> Building Lizard (native) [$Configuration|$Platform]"
& $msbuild $slnPath /t:Lizard /p:Configuration=$Configuration /p:Platform=$Platform
if ($LASTEXITCODE -ne 0) {
    throw "Lizard build failed with exit code $LASTEXITCODE"
}

Write-Host "==> Building DoorstopBootstrap (managed) [$Configuration]"
dotnet build $bootstrapProj -c $Configuration
if ($LASTEXITCODE -ne 0) {
    throw "DoorstopBootstrap build failed with exit code $LASTEXITCODE"
}

Write-Host "==> Preparing root Release directory"
New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null

$nativeDll = Join-Path $root "$Platform\$Configuration\Lizard.dll"
$managedDll = Join-Path $root "DoorstopBootstrap\bin\$Configuration\net6.0\Doorstop.dll"
$doorstopCfg = Join-Path $root "DoorstopBootstrap\doorstop_config.ini"

if (-not (Test-Path $nativeDll)) {
    throw "Native output not found: $nativeDll"
}
if (-not (Test-Path $managedDll)) {
    throw "Managed output not found: $managedDll"
}
if (-not (Test-Path $doorstopCfg)) {
    throw "Doorstop config not found: $doorstopCfg"
}

Copy-Item $nativeDll -Destination (Join-Path $releaseDir "Lizard.dll") -Force
Copy-Item $managedDll -Destination (Join-Path $releaseDir "Doorstop.dll") -Force
Copy-Item $doorstopCfg -Destination (Join-Path $releaseDir "doorstop_config.ini") -Force

if (-not (Test-Path $resWinHttp)) {
    throw "winhttp.dll not found: $resWinHttp"
}
Copy-Item $resWinHttp -Destination (Join-Path $releaseDir "winhttp.dll") -Force
Write-Host "==> Copied winhttp.dll from: $resWinHttp"

$releaseCfg = Join-Path $releaseDir "doorstop_config.ini"
if ([string]::IsNullOrWhiteSpace($CoreClrPath) -or [string]::IsNullOrWhiteSpace($CorlibDir)) {
    $detected = Resolve-CoreClr
    $CoreClrPath = $detected.CoreClrPath
    $CorlibDir = $detected.CorlibDir
    Write-Host "==> Auto-detected CoreCLR: $CoreClrPath"
}

Update-Il2CppConfig -ConfigPath $releaseCfg -NewCoreClrPath $CoreClrPath -NewCorlibDir $CorlibDir
Write-Host "==> Updated [Il2Cpp] config in: $releaseCfg"

Write-Host "==> Build completed successfully. Artifacts are in: $releaseDir"
