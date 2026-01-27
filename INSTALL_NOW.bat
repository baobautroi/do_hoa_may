@echo off
echo =====================================
echo MSYS2 Automated Installation
echo =====================================
echo.
echo This script will:
echo   1. Download MSYS2
echo   2. Install it to C:\msys64
echo   3. Install GCC compiler and FreeGLUT
echo   4. Add to system PATH
echo.
echo This requires Administrator privileges.
echo.
pause

echo.
echo Running installation script...
echo.

PowerShell -ExecutionPolicy Bypass -File "%~dp0auto_install.ps1"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo =====================================
    echo Installation successful!
    echo =====================================
    echo.
    echo Please restart this window and run compile.bat
    echo.
) else (
    echo.
    echo =====================================
    echo Installation failed!
    echo =====================================
    echo.
    echo Please try manual installation or check SETUP_GUIDE.md
    echo.
)

pause
