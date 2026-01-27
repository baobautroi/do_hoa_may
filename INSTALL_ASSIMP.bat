@echo off
echo ====================================
echo Installing Assimp and Dependencies
echo ====================================
echo.

echo This will install Assimp library for 3D model loading
echo.

C:\msys64\msys2_shell.cmd -mingw64 -defterm -no-start -c "pacman -S --noconfirm mingw-w64-x86_64-assimp"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ====================================
    echo Assimp installed successfully!
    echo ====================================
    echo.
    echo You can now compile with: compile_with_assimp.bat
    echo.
) else (
    echo.
    echo ====================================
    echo Installation failed!
    echo ====================================
    echo.
    echo Please check:
    echo 1. MSYS2 is installed at C:\msys64
    echo 2. Internet connection is working
    echo 3. Run MSYS2 and update: pacman -Syu
    echo.
)

pause
