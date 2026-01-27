# Automatic MSYS2 Installation Script for Pickleball OpenGL Project
# Run this script in PowerShell as Administrator

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "MSYS2 Auto-Installation Script" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

$msys2Installer = "$env:TEMP\msys2-x86_64-latest.exe"
$msys2Path = "C:\msys64"

# Step 1: Download MSYS2
Write-Host "[1/5] Downloading MSYS2..." -ForegroundColor Yellow
try {
    $ProgressPreference = 'SilentlyContinue'
    Invoke-WebRequest -Uri "https://github.com/msys2/msys2-installer/releases/download/nightly-x86_64/msys2-x86_64-latest.exe" -OutFile $msys2Installer
    Write-Host "✓ Download complete!" -ForegroundColor Green
} catch {
    Write-Host "✗ Download failed. Please download manually from https://www.msys2.org/" -ForegroundColor Red
    exit 1
}

# Step 2: Install MSYS2
Write-Host ""
Write-Host "[2/5] Installing MSYS2..." -ForegroundColor Yellow
Write-Host "This will open the installer. Please click through the installation wizard." -ForegroundColor Gray
Write-Host "Use the default installation path: C:\msys64" -ForegroundColor Gray
Start-Process -FilePath $msys2Installer -ArgumentList "install --root $msys2Path --confirm-command" -Wait
Write-Host "✓ MSYS2 installed!" -ForegroundColor Green

# Step 3: Update MSYS2 and install packages
Write-Host ""
Write-Host "[3/5] Installing GCC and FreeGLUT..." -ForegroundColor Yellow
Write-Host "This will open MSYS2 terminal. Please wait for the installation to complete." -ForegroundColor Gray

$installScript = @"
pacman -Syu --noconfirm
pacman -S --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-freeglut
echo ""
echo "Installation complete! Press any key to close..."
read -n 1
"@

$installScript | Out-File -FilePath "$env:TEMP\msys2_install.sh" -Encoding ASCII

Start-Process -FilePath "$msys2Path\msys2_shell.cmd" -ArgumentList "-mingw64 -c `"bash /tmp/msys2_install.sh`"" -Wait

Write-Host "✓ Packages installed!" -ForegroundColor Green

# Step 4: Add to PATH
Write-Host ""
Write-Host "[4/5] Adding MSYS2 to system PATH..." -ForegroundColor Yellow

$mingwBinPath = "$msys2Path\mingw64\bin"
$currentPath = [Environment]::GetEnvironmentVariable("Path", "Machine")

if ($currentPath -notlike "*$mingwBinPath*") {
    try {
        [Environment]::SetEnvironmentVariable("Path", "$currentPath;$mingwBinPath", "Machine")
        $env:Path += ";$mingwBinPath"
        Write-Host "✓ PATH updated!" -ForegroundColor Green
    } catch {
        Write-Host "⚠ Could not update PATH automatically. Please add manually:" -ForegroundColor Yellow
        Write-Host "  $mingwBinPath" -ForegroundColor White
    }
} else {
    Write-Host "✓ Already in PATH!" -ForegroundColor Green
}

# Step 5: Verify installation
Write-Host ""
Write-Host "[5/5] Verifying installation..." -ForegroundColor Yellow
Start-Sleep -Seconds 2

$gccVersion = & "$mingwBinPath\g++.exe" --version 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ GCC installed successfully!" -ForegroundColor Green
    Write-Host $gccVersion[0] -ForegroundColor Gray
} else {
    Write-Host "⚠ GCC not found. You may need to restart your terminal." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Installation Complete!" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Close this window and open a NEW PowerShell window" -ForegroundColor White
Write-Host "2. Navigate to: d:\dohoa" -ForegroundColor White
Write-Host "3. Run: .\compile.bat" -ForegroundColor White
Write-Host ""
Write-Host "Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
