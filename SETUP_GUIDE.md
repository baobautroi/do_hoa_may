# Quick Setup Guide for Windows

## 🎯 You have 3 options to compile this OpenGL project:

---

## ✅ Option 1: MSYS2 with MinGW (RECOMMENDED - Easiest)

### Step 1: Install MSYS2
1. Download MSYS2 from: **https://www.msys2.org/**
2. Run the installer (msys2-x86_64-xxxxxxxx.exe)
3. Follow the installation wizard (use default location: C:\msys64)

### Step 2: Install required packages
Open **MSYS2 MINGW64** (search in Start menu) and run:
```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-freeglut
```

### Step 3: Add to PATH
Add this to your Windows PATH environment variable:
```
C:\msys64\mingw64\bin
```

To add to PATH:
1. Search "Environment Variables" in Windows Start
2. Click "Environment Variables"
3. Under "System variables", find "Path" and click "Edit"
4. Click "New" and add: `C:\msys64\mingw64\bin`
5. Click OK on all dialogs
6. **Restart your terminal/PowerShell**

### Step 4: Compile and run
```bash
cd d:\dohoa
g++ pickleball_scene.cpp -o pickleball_scene.exe -lfreeglut -lopengl32 -lglu32
.\pickleball_scene.exe
```

---

## ✅ Option 2: Visual Studio (If you have it installed)

### Step 1: Check if you have Visual Studio
Open **Developer Command Prompt for VS** or **x64 Native Tools Command Prompt**

### Step 2: Install FreeGLUT
Download prebuilt binaries from:
**https://www.transmissionzero.co.uk/software/freeglut-devel/**

Extract and copy:
- `bin\x64\freeglut.dll` → `d:\dohoa\`
- Remember the paths to `include\` and `lib\` folders

### Step 3: Compile
```bash
cl pickleball_scene.cpp /I"path\to\freeglut\include" /link /LIBPATH:"path\to\freeglut\lib\x64" freeglut.lib opengl32.lib glu32.lib
```

---

## ✅ Option 3: Standalone MinGW-w64

### Download standalone MinGW:
**https://github.com/niXman/mingw-builds-binaries/releases**

Download: `x86_64-xx.x.x-release-posix-seh-ucrt-rt_vxx-revx.7z`

Extract to `C:\mingw64` and add `C:\mingw64\bin` to PATH.

Then install FreeGLUT manually from:
**https://www.transmissionzero.co.uk/software/freeglut-devel/**

---

## 🚀 Quick Test After Installation

After installing using any method above, test with:
```powershell
g++ --version
```

If you see version info, you're ready to compile!

---

## ❓ Which option should I choose?

- **New to C++**: Use **Option 1 (MSYS2)** - everything is automated
- **Have Visual Studio**: Use **Option 2**
- **Want minimal install**: Use **Option 3**

---

## 📞 Need Help?

After installing, just run:
```bash
compile.bat
```

Or manually:
```bash
g++ pickleball_scene.cpp -o pickleball_scene.exe -lfreeglut -lopengl32 -lglu32
```
