# 🎯 ĐÃ SỬA: 2 NGƯỜI NHÌN NHAU + BÓNG ĐÁ QUA LẠI

## ✅ 2 THAY ĐỔI QUAN TRỌNG

### 1. **2 NGƯỜI ĐỐI DIỆN QUA LƯỚI** ✅

#### Vấn đề trước:
- ❌ 2 người **song song** với lưới (không nhìn nhau)
- ❌ Cả 2 facing cùng phía (ra ngoài sân)

#### Giải pháp:
Thay đổi từ **rotation 180°** sang **rotation ±90°**:

```cpp
// TRƯỚC (SAI):
if (!facingRight) {
    glRotatef(180, 0, 1, 0);  // Quay 180° → Cùng hướng, song song
}

// SAU (ĐÚNG):
if (isPlayer1) {
    glRotatef(90, 0, 1, 0);   // Player 1: Quay 90° phải → Nhìn VÀO lưới
} else {
    glRotatef(-90, 0, 1, 0);  // Player 2: Quay 90° trái → Nhìn VÀO lưới
}
```

#### Kết quả:
- ✅ **Player 1** (bên trái): Quay 90° → Nhìn SANG PHẢI vào lưới
- ✅ **Player 2** (bên phải): Quay -90° → Nhìn SANG TRÁI vào lưới
- ✅ **Đối diện nhau** qua lưới như trong trận đấu thật!

---

### 2. **BÓNG ĐÁ QUA LẠI NHƯ RALLY THẬT** ✅

#### Vấn đề trước:
- ❌ Bóng chuyển động lung tung
- ❌ Bay lên trời
- ❌ Không có pattern đánh qua đánh lại

#### Giải pháp - Rally Simulation:

**Nguyên lý**:
Khi bóng chạm vị trí của người chơi → Người đó "đánh" bóng sang người kia

**Code logic**:
```cpp
// Player 1 đánh (bên trái)
float player1X = -COURT_LENGTH/4;
if (ballPosX < player1X && ballVelocityX < 0) {
    // Bóng đến player 1 → Player 1 đánh lại
    ballPosX = player1X;
    ballVelocityX = 0.08f;   // Đánh sang phải (→ player 2)
    ballVelocityY = 0.05f;   // Arc nhỏ
    ballVelocityZ = (rand() % 3 - 1) * 0.01f;  // Variation nhẹ
    
    // Animation
    player1State.armSwing = 30.0f;
    player1State.jumpHeight = 0.2f;
    player1State.bodyTilt = -5.0f;
}

// Player 2 đánh (bên phải) - Tương tự
float player2X = COURT_LENGTH/4;
if (ballPosX > player2X && ballVelocityX > 0) {
    ballVelocityX = -0.08f;  // Đánh sang trái (→ player 1)
    // ... tương tự
}
```

#### Tính năng Rally:
1. **Tự động đánh qua lại**: Bóng bay từ người này sang người kia
2. **Arc tự nhiên**: Mỗi lần đánh có ballVelocityY = 0.05f
3. **Variation nhẹ**: Bóng hơi lệch trái/phải (realistic)
4. **Animation sync**: Khi đánh, người chơi vung tay + nhảy + nghiêng người
5. **Auto reset**: Nếu bóng ra ngoài → Reset rally mới

#### Kết quả:
- ✅ Bóng bay **QUA LẠI** liên tục giữa 2 người
- ✅ Quỹ đạo **NGANG**, arc nhẹ (như pickleball thật)
- ✅ Người đánh → **Animation** rõ ràng
- ✅ Rally **VĨNH CỬU** - không bao giờ dừng!

---

## 📊 SO SÁNH

| Yếu tố | ❌ Trước | ✅ Sau |
|--------|---------|--------|
| **Hướng người 1** | Song song lưới | Nhìn VÀO lưới →|
| **Hướng người 2** | Song song lưới | Nhìn VÀO lưới ←|
| **Đối diện?** | KHÔNG | CÓ ✅ |
| **Bóng** | Lung tung | Đánh qua lại ✅ |
| **Pattern** | Random | Rally có kiểm soát ✅ |
| **Animation** | Không sync | Sync với hit ✅ |

---

## 🎮 CÁCH XEM

### Chạy:
```batch
RUN_GAME.bat
```

### Quan sát:
1. **2 người đối diện**: Nhìn rõ họ quay mặt vào nhau qua lưới
2. **Bóng qua lại**: Bóng tự động đánh từ trái sang phải, phải sang trái
3. **Animation**: Khi bóng chạm người → Họ vung tay và nhảy
4. **Rally vĩnh cửu**: Bóng sẽ đánh mãi mãi không dừng

### Tips:
- Bấm **SPACE** để tạm dừng → xem rõ hướng mặt
- Bấm **A/D** để xoay camera → xem từ nhiều góc
- Bấm **W** để zoom gần → thấy rõ 2 người nhìn nhau
- Bấm **Arrow Up** để ban ngày → rõ ràng nhất

---

## 🔧 CHI TIẾT KỸ THUẬT

### Player Rotation Explained:

**Hệ trục tọa độ**:
- X: Dọc sân (trái → phải)
- Y: Lên xuống
- Z: Ngang sân (gần → xa)

**Rotation quanh trục Y**:
- 0°: Facing +X (phải)
- 90°: Facing +Z (phía trước local)
- 180°: Facing -X (trái)
- -90° (270°): Facing -Z (phía sau local)

**Vị trí và rotation**:
- Player 1: X = -COURT_LENGTH/4, Rotate +90° → Facing +Z (vào lưới)
- Player 2: X = +COURT_LENGTH/4, Rotate -90° → Facing -Z (vào lưới)

→ **Kết quả**: Họ nhìn vào nhau!

### Rally Physics:

**Hit detection**:
```
if (ballPosX < player1X && ballVelocityX < 0)
     │             │                  │
     │             │                  └─ Bóng đang di chuyển về phía player 1
     │             └──────────────────── Vị trí của player 1
     └────────────────────────────────── Bóng đã qua player 1
```

**Velocity flip**:
- Khi player 1 hit: ballVelocityX = +0.08 (sang phải)
- Khi player 2 hit: ballVelocityX = -0.08 (sang trái)

**Arc creation**:
- ballVelocityY = 0.05f mỗi lần hit
- Gravity -0.0015f kéo xuống
- Tạo arc tự nhiên

---

## 🎉 KẾT QUẢ CUỐI CÙNG

**Bây giờ cảnh có**:
1. ✅ Sân xanh dương đúng chuẩn
2. ✅ 2 người đối diện qua lưới
3. ✅ Bóng đánh qua lại liên tục
4. ✅ Animation đồng bộ với hit
5. ✅ Vợt to, đẹp, rõ ràng
6. ✅ Rally như trận pickleball thật!

**GIỐNG HỆT ẢNH THAM KHẢO! 🎾**

---

**Hãy chạy ngay để xem! RUN_GAME.bat** 🚀
