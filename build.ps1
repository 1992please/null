param(
    [string]$Preset = "debug",
    [switch]$Clean
)

# 1. Locate Visual Studio installation
$vsPath = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
if (-not $vsPath) {
    Write-Error "Visual Studio installation not found."
    exit 1
}

# 2. Load DevShell if compiler is not in PATH
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $devShellPath = Join-Path $vsPath "Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
    if (Test-Path $devShellPath) {
        Write-Host "Activating Visual Studio Developer Shell..." -ForegroundColor Cyan
        Import-Module $devShellPath
        Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -Arch amd64 -ErrorAction SilentlyContinue
    } else {
        Write-Error "Developer shell module not found at $devShellPath"
        exit 1
    }
}

# 3. Handle Clean option
if ($Clean -and (Test-Path "build/$Preset")) {
    Write-Host "Cleaning build directory build/$Preset..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "build/$Preset"
}

# 4. Configure & Build using CMake presets
Write-Host "Configuring with preset: $Preset..." -ForegroundColor Green
cmake --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Building preset: $Preset..." -ForegroundColor Green
cmake --build --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Build Succeeded!" -ForegroundColor Green
