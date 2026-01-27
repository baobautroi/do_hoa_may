# ⚡ FASTEST Installation Method - Using Windows Package Manager (winget)
# This is the simplest way to install everything you need!

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Quick MSYS2 Installation via winget" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Check if winget is available
try {
  $wingetVersion = winget --version
  Write-Host "✓ winget found: $wingetVersion" -ForegroundColor Green
}
catch {
  Write-Host "✗ winget not found. Please use Windows 10/11 with App Installer." -ForegroundColor Red
  Write-Host ""
  Write-Host "Alternative: Run INSTALL_NOW.bat or follow SETUP_GUIDE.md" -ForegroundColor Yellow
  pause
  exit 1
}

Write-Host ""
Write-Host "[1/3] Installing MSYS2..." -ForegroundColor Yellow
winget install --id=MSYS2.MSYS2 -e --accept-source-agreements --accept-package-agreements

if ($LASTEXITCODE -eq 0) {
  Write-Host "✓ MSYS2 installed!" -ForegroundColor Green
}
else {
  Write-Host "⚠ Installation may have failed. Check output above." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "[2/3] Installing GCC and FreeGLUT packages..." -ForegroundColor Yellow
Write-Host "Opening MSYS2 terminal to install packages..." -ForegroundColor Gray

$msys2Path = "C:\msys2\msys2_shell.cmd"
if (Test-Path $msys2Path) {
  $installCmd = "pacman -Syu --noconfirm && pacman -S --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-freeglut && echo 'Installation complete!' && sleep 3"
  Start-Process -FilePath $msys2Path -ArgumentList "-mingw64", "-c", $installCmd -Wait
  Write-Host "✓ Packages installed!" -ForegroundColor Green
}
else {
  Write-Host "⚠ MSYS2 not found at expected location" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "[3/3] Adding to PATH..." -ForegroundColor Yellow
$mingwPath = "C:\msys2\mingw64\bin"
$currentPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($currentPath -notlike "*$mingwPath*") {
  [Environment]::SetEnvironmentVariable("Path", "$currentPath;$mingwPath", "User")
  $env:Path += ";$mingwPath"
  Write-Host "✓ Added to PATH!" -ForegroundColor Green
}
else {
  Write-Host "✓ Already in PATH!" -ForegroundColor Green
}

Write-Host ""
Write-Host "=====================================" -ForegroundColor Green
Write-Host "Installation Complete!" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Close this window" -ForegroundColor White
Write-Host "2. Open a NEW PowerShell/Command Prompt" -ForegroundColor White
Write-Host "3. cd d:\dohoa" -ForegroundColor White
Write-Host "4. Run: compile.bat" -ForegroundColor White
Write-Host ""
pause
