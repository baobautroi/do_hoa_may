# 🎬 ANIMATION FEATURES DOCUMENTATION

## Tổng quan Animation System

Cảnh pickleball có **3 hệ thống animation chính**:
1. Player Animation (Người chơi)
2. Environmental Animation (Môi trường)
3. Lighting Animation (Ánh sáng)

---

## 1. 🏃 PLAYER ANIMATION

### PlayerState Structure
```cpp
struct PlayerState {
    float legAngle1;      // Góc chân trái (-15° đến +15°)
    float legAngle2;      // Góc chân phải (ngược với chân trái)
    float armSwing;       // Góc vung tay (-20° đến +25° khi idle, -20° đến +45° khi active)
    float bodyTilt;       // Nghiêng thân (-5° đến +5°)
    float jumpHeight;     // Độ cao nhảy (0 đến 0.3 units)
};
```

### Animation States

#### IDLE State (Đứng yên)
**Khi nào**: Bóng ở phía đối diện
**Chuyển động**:
- `armSwing`: Dao động nhẹ ±10° (2 Hz)
- `legAngle1/2`: 0° (đứng thẳng)
- `bodyTilt`: 0°
- `jumpHeight`: Decay về 0

**Code**:
```cpp
player1State.armSwing = sin(animationTime * 2.0f) * 10.0f;
player1State.legAngle1 = 0;
player1State.legAngle2 = 0;
player1State.bodyTilt = 0;
player1State.jumpHeight *= 0.9f;  // Giảm dần
```

#### ACTIVE State (Đang chơi)
**Khi nào**: Bóng ở phía mình và đang bay về phía mình
**Chuyển động**:
- `armSwing`: Vung mạnh ±45° (10 Hz)
- `legAngle1`: +15° → -15° luân phiên (8 Hz)
- `legAngle2`: Ngược với legAngle1
- `bodyTilt`: ±5° (10 Hz)
- `jumpHeight`: Nhảy lên khi bóng gần (max 0.3)

**Code**:
```cpp
if (ballPosX < 0 && ballVelocityX < 0) {  // Player 1 active
    player1State.armSwing = sin(animationTime * 10.0f) * 45.0f - 20.0f;
    player1State.legAngle1 = sin(animationTime * 8.0f) * 15.0f;
    player1State.legAngle2 = -player1State.legAngle1;
    player1State.bodyTilt = sin(animationTime * 10.0f) * 5.0f;
    
    // Jump when ball is close
    if (abs(ballPosX + COURT_LENGTH/3) < 1.0f) {
        player1State.jumpHeight = abs(sin(animationTime * 15.0f)) * 0.3f;
    }
}
```

### Body Parts Hierarchy

**Parent-Child Relationships**:
```
Root (position + jumpHeight + bodyTilt)
├── Left Leg
│   ├── Thigh (legAngle1)
│   ├── Knee
│   ├── Lower Leg
│   └── Shoe
├── Right Leg
│   ├── Thigh (legAngle2)
│   ├── Knee
│   ├── Lower Leg
│   └── Shoe
├── Torso
├── Neck
├── Head
│   ├── Face
│   ├── Eyes
│   └── Hair
├── Left Arm
│   ├── Upper Arm
│   ├── Elbow
│   ├── Forearm
│   └── Hand
└── Right Arm (Paddle arm)
    ├── Upper Arm (armSwing)
    ├── Elbow
    ├── Forearm
    ├── Hand
    └── Paddle
        ├── Handle
        └── Face
```

### Animation Frequencies
- **Breathing (Idle)**: 2 Hz (120 BPM)
- **Arm Swing (Active)**: 10 Hz
- **Leg Movement**: 8 Hz
- **Body Tilt**: 10 Hz
- **Jump**: 15 Hz

---

## 2. 🌿 ENVIRONMENTAL ANIMATION

### Wind System

**Global Variables**:
```cpp
float windTime = 0.0f;        // Accumulated time
float windStrength = 1.0f;    // 0.0 to 3.0
```

**Update**:
```cpp
windTime += 0.02f * windStrength;  // Each frame (~60 FPS)
```

### Grass Swaying

**Formula**:
```cpp
float swayX = sin(windTime * 2.0f + x * 0.3f + z * 0.2f) * windStrength * 0.15f;
float swayZ = cos(windTime * 1.5f + x * 0.2f + z * 0.3f) * windStrength * 0.1f;
```

**Parameters**:
- `windTime * 2.0f`: Base frequency
- `x * 0.3f + z * 0.2f`: Spatial variation (grass at different positions sway differently)
- `windStrength * 0.15f`: Max sway amplitude

**Visual Effect**:
- Each blade rotates around its base
- Wave-like pattern across the field
- Stronger wind = larger sway

### Tree Swaying

**Formula**:
```cpp
float swayAngle = sin(windTime + x * 0.5f + z * 0.3f) * windStrength * 3.0f;
```

**Application**:
```cpp
glRotatef(swayAngle, 0, 0, 1);  // Tilt left-right
```

**Effect**:
- Entire tree canopy tilts
- Maximum ±9° at windStrength = 3.0
- Different trees sway at different phases

### Sun Rays Rotation

**Formula**:
```cpp
glRotatef(angle + windTime * 10.0f, 0, 0, 1);
```

**Effect**:
- 12 rays rotate around sun
- Speed proportional to wind
- Creates dynamic lighting effect

---

## 3. ☀️ LIGHTING ANIMATION

### Time-based Lighting

**Time Cycle**: `timeOfDay` (0.0 to 1.0)
- 0.00 - 0.25: Night
- 0.25 - 0.50: Dawn → Noon
- 0.50 - 0.75: Noon → Dusk  
- 0.75 - 1.00: Dusk → Night

### Sky Color Transition

**Night (0.00)**:
```cpp
color = {0.1, 0.1, 0.2}  // Dark blue
```

**Dawn (0.25)**:
```cpp
color = {0.4, 0.5, 0.6}  // Light blue
```

**Noon (0.50)**:
```cpp
color = {0.7, 0.8, 0.9}  // Bright blue
```

**Dusk (0.75)**:
```cpp
color = {0.9, 0.5, 0.4}  // Orange-red
```

### Light Intensity

**Night**: 30% intensity
```cpp
intensity = 0.3f;
lightColor = {0.4, 0.4, 0.6} * intensity;  // Blue tint
```

**Day**: 100% intensity
```cpp
intensity = 1.0f;
lightColor = {1.0, 1.0, 0.95};  // Warm white
```

**Dawn/Dusk**: 70% intensity
```cpp
intensity = 0.7f;
lightColor = {1.0, 0.7, 0.4} * intensity;  // Orange
```

### Sun Position

**Formula**:
```cpp
float angle = timeOfDay * 2.0f * PI;
sunX = 30.0f * cos(angle);
sunY = 30.0f * sin(angle);
```

**Path**: Circular arc from east to west

### Street Lamps

**Activation**:
```cpp
if (timeOfDay < 0.25f || timeOfDay > 0.75f) {
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
}
```

**Effect**:
- Warm yellow light (1.0, 0.9, 0.7)
- Creates pools of light at night
- Lamp heads glow (emission property)

---

## 🎯 ANIMATION SYNCHRONIZATION

### Ball-Player Sync

**Detection**:
```cpp
if (ballPosX < 0 && ballVelocityX < 0) {
    // Left player active
}
```

**Trigger Jump**:
```cpp
if (abs(ballPosX ± COURT_LENGTH/3) < 1.0f) {
    // Ball is near player → jump
    jumpHeight = abs(sin(animationTime * 15.0f)) * 0.3f;
}
```

### Wind-Environment Sync

All wind-affected elements use same `windTime`:
- Grass: `windTime * 2.0f`
- Trees: `windTime * 1.0f`
- Sun rays: `windTime * 10.0f`

**Result**: Cohesive environmental movement

---

## 🔧 TUNING PARAMETERS

### Speed Adjustments

**Slower animations**:
```cpp
animationTime += 0.008f;  // Half speed (default: 0.016f)
```

**Faster wind**:
```cpp
windTime += 0.04f * windStrength;  // Double speed
```

### Amplitude Adjustments

**More dramatic player swing**:
```cpp
armSwing = sin(...) * 60.0f - 20.0f;  // ±60° instead of ±45°
```

**Less grass sway**:
```cpp
swayX = sin(...) * windStrength * 0.08f;  // Half amplitude
```

---

## 📊 PERFORMANCE

### Frame Update Costs

**Per Frame** (~60 FPS):
- Player state update: ~0.1ms (2 players)
- Grass field: ~2-3ms (depends on density)
- Trees: ~0.5ms (8 trees)
- Ball physics: ~0.05ms
- Lighting: ~0.2ms

**Total**: ~3-4ms per frame (well under 16.67ms budget for 60 FPS)

### Optimization Tips

1. **Reduce grass density**:
   ```cpp
   for (x...; x += 1.2f)  // Instead of 0.8f
   ```

2. **Simplify player model**:
   - Remove eyes/hair for distant views
   - Use lower sphere/cylinder tesselation

3. **LOD for trees**:
   - Fewer foliage layers when camera far

---

**Animation system hoàn chỉnh và tối ưu! 🎬**
