@echo off
echo ====================================
echo Compiling with Assimp Support...
echo ====================================
echo.

REM Compile with Assimp library
C:\msys64\msys2_shell.cmd -mingw64 -defterm -no-start -here -c "g++ pickleball_scene.cpp ModelLoader.cpp -o pickleball_scene.exe -lfreeglut -lopengl32 -lglu32 -lassimp -std=c++11 -Wall"

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
    echo 1. MSYS2 installed at C:\msys64
    echo 2. Assimp installed: pacman -S mingw-w64-x86_64-assimp
    echo 3. FreeGLUT installed: pacman -S mingw-w64-x86_64-freeglut
    echo.
    echo Run INSTALL_ASSIMP.bat to install dependencies
    echo.
)

pause
