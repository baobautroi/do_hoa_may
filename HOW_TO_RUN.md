# 🎮 How to Run the Pickleball Scene

## ✨ Quick Start (Easiest Way)

### Method 1: Double-Click the Batch File
1. Open File Explorer
2. Navigate to `d:\dohoa`
3. Double-click `RUN_GAME.bat`
4. The 3D scene will open automatically!

---

## 📝 Manual Methods

### Method 2: PowerShell/Command Prompt
1. Press `Win + R`
2. Type `powershell` and press Enter
3. Run these commands:
```powershell
cd d:\dohoa
.\pickleball_scene.exe
```

### Method 3: Via MSYS2 (Most Reliable)
1. Press `Win + R`
2. Type `cmd` and press Enter
3. Run:
```cmd
C:\msys64\msys2_shell.cmd -mingw64 -defterm -no-start -c "cd /d/dohoa && ./pickleball_scene.exe"
```

---

## 🎮 Controls Once Running

### Time of Day
- **↑ (Arrow Up)** - Make it later in the day (advance time)
- **↓ (Arrow Down)** - Make it earlier in the day (reverse time)

### Camera Movement
- **W** - Zoom IN (get closer)
- **S** - Zoom OUT (get farther)
- **A** - Rotate camera LEFT
- **D** - Rotate camera RIGHT
- **Q** - Move camera UP (higher view)
- **E** - Move camera DOWN (lower view)

### Animation
- **SPACE** - Pause/Resume ball movement
- **ESC** - Exit the program

---

## 🌅 What to Try

1. **Watch the Day-Night Cycle**
   - Press ↑ repeatedly to watch the sun set
   - Notice the street lamps turn on at night!
   - Sky color changes: Blue → Orange → Purple → Dark

2. **Follow the Ball**
   - Watch it bounce between players
   - See physics in action (gravity, bouncing)
   - Press SPACE to pause and examine

3. **Explore the Scene**
   - Use W/S to zoom in and see details
   - Use A/D to rotate around the court
   - Use Q/E to get different viewing angles

---

## ❓ Troubleshooting

**Problem: Window opens then closes immediately**
- Use Method 3 (MSYS2) instead
- The application needs the MSYS2 DLL files

**Problem: "freeglut.dll not found"**
- Run via MSYS2 method (Method 3)
- Or copy `C:\msys64\mingw64\bin\freeglut.dll` to `d:\dohoa`

**Problem: Nothing happens when I press keys**
- Make sure the OpenGL window has focus (click on it)
- Try clicking inside the 3D window first

---

## 🎯 Quick Reference Card

```
╔════════════════════════════════════╗
║   PICKLEBALL SCENE CONTROLS        ║
╠════════════════════════════════════╣
║ TIME       ↑ Later   ↓ Earlier     ║
║ ZOOM       W Closer  S Farther     ║
║ ROTATE     A Left    D Right       ║
║ HEIGHT     Q Up      E Down        ║
║ PAUSE      SPACE                   ║
║ EXIT       ESC                     ║
╚════════════════════════════════════╝
```

---

## 💡 Pro Tips

- **Best view**: Start with default camera, then press ↑ a few times to see sunset
- **Night mode**: Press ↑ many times until sky is dark - street lamps will glow!
- **Action shot**: Zoom in (W W W) and rotate (A or D) while ball is moving
- **Pause feature**: Press SPACE to freeze the ball mid-air for screenshots
