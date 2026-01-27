@echo off
echo ====================================
echo   Pickleball Scene - Starting...
echo ====================================
echo.

REM Check if the executable exists
if not exist "pickleball_scene.exe" (
    echo ERROR: pickleball_scene.exe not found!
    echo Please run compile.bat first.
    echo.
    pause
    exit /b 1
)

echo Starting the 3D Pickleball Scene...
echo.
echo Controls will be shown in the console.
echo Make sure to click on the 3D window to use keyboard controls!
echo.

REM Run via MSYS2 for proper DLL access
C:\msys64\msys2_shell.cmd -mingw64 -defterm -no-start -here -c "./pickleball_scene.exe"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ====================================
    echo   Application closed
    echo ====================================
    echo.
    echo If you experienced issues, check HOW_TO_RUN.md
    echo.
)

pause
