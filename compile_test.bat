@echo off
echo ====================================
echo Compiling Test Program...
echo ====================================
echo.

C:\msys64\msys2_shell.cmd -mingw64 -defterm -no-start -here -c "g++ test_model_loader.cpp ModelLoader.cpp -o test_model_loader.exe -lfreeglut -lopengl32 -lglu32 -lassimp -std=c++11 -Wall"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ====================================
    echo Compilation successful!
    echo ====================================
    echo.
    echo Run test:
    echo   test_model_loader.exe models/your_model.obj
    echo.
    echo Or without model:
    echo   test_model_loader.exe
    echo.
) else (
    echo.
    echo ====================================
    echo Compilation failed!
    echo ====================================
    echo.
    echo Make sure Assimp is installed:
    echo   INSTALL_ASSIMP.bat
    echo.
)

pause
