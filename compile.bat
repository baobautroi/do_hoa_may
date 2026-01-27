@echo off
echo ====================================
echo Compiling Pickleball Scene...
echo ====================================

REM Compile with g++ via MSYS2 MinGW
C:\msys64\msys2_shell.cmd -mingw64 -defterm -no-start -here -c "g++ pickleball_scene.cpp -o pickleball_scene.exe -lfreeglut -lopengl32 -lglu32"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ====================================
    echo Compilation successful!
    echo ====================================
    echo.
    echo You can now run RUN_GAME.bat
    echo.
) else (
    echo.
    echo ====================================
    echo Compilation failed!
    echo ====================================
    echo.
    echo Please make sure you have:
    echo 1. MinGW-w64 installed
    echo 2. FreeGLUT libraries installed
    echo 3. OpenGL libraries available
    echo.
    pause
)
