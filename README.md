# 🎾 Enhanced Pickleball Park Scene

**Cảnh công viên đánh bóng pickleball 3D chân thực với OpenGL/GLUT**

![OpenGL](https://img.shields.io/badge/OpenGL-3D-blue)
![FreeGLUT](https://img.shields.io/badge/FreeGLUT-Library-green)
![C++](https://img.shields.io/badge/C%2B%2B-Programming-orange)

## ✨ Tính năng nổi bật

### 🏃 Nhân vật chi tiết và chân thực
- **Mô hình người chơi đầy đủ**: Đầu, cổ, thân, tay, chân được vẽ riêng biệt
- **Chi tiết cơ thể**: Có mắt, tóc, khớp khuỷu tay, đầu gối, giày dép
- **Quần áo màu sắc**: Áo đỏ, quần xanh, giày trắng
- **Vợt đánh bóng**: Cầm vợt màu xanh dương với cán và mặt vợt chi tiết

### 🎬 Animation mượt mà và tự nhiên
- **Chuyển động chân**: Hai chân di chuyển đan xen khi người chơi hoạt động
- **Swing tay**: Tay vung vợt với góc độ 45° khi đánh bóng
- **Body tilt**: Thân nghiêng theo động tác đánh
- **Nhảy nhẹ**: Người chơi nhảy lên khi tiếp xúc với bóng
- **Idle animation**: Động tác thở nhẹ nhàng khi đứng yên

### � Bối cảnh công viên tuyệt đẹp
- **Cỏ đung đưa theo gió**: Hàng trăm ngọn cỏ xung quanh sân với hiệu ứng gió
- **Màu sắc tự nhiên**: Màu cỏ biến thiên nhẹ để trông chân thực
- **Bãi cỏ rộng**: Vùng cỏ bao quanh cả sân pickleball

### 🌳 Cây cối phong phú
- **Cấu trúc nhiều tầng**: 3 tầng lá với màu sắc khác nhau
- **Lá điểm xuyết**: 5 chùm lá nhỏ xung quanh tán cây
- **Thân cây chi tiết**: Hình trụ taper tự nhiên
- **Đung đưa theo gió**: Tán cây lắc lư theo sức gió
- **8 cây cối**: Đặt xung quanh sân tạo không gian công viên

### ☀️ Ánh sáng mặt trời đẹp mắt
- **Mặt trời có tia sáng**: 12 tia sáng xoay quanh mặt trời
- **Glow effect**: Vòng sáng bao quanh mặt trời
- **Ánh sáng động**: Tia sáng quay theo thời gian
- **Chỉ hiện ban ngày**: Mặt trời chỉ vẽ khi timeOfDay phù hợp

### � Chu kỳ ngày đêm động
- **Bầu trời gradient**: Thay đổi màu theo thời gian (đêm → bình minh → trưa → hoàng hôn)
- **Ánh sáng thích ứng**: Cường độ và màu sắc ánh sáng thay đổi theo giờ
- **Đèn đường tự động**: Bật vào ban đêm, tắt ban ngày

### 🎮 Điều khiển linh hoạt

#### Điều khiển thời gian và môi trường
- **↑ / ↓**: Chỉnh thời gian trong ngày
- **R / F**: Tăng/giảm sức gió (0.0 - 3.0)
- **SPACE**: Tạm dừng/Tiếp tục animation

#### Điều khiển camera
- **W / S**: Zoom in/out (5 - 40 units)
- **A / D**: Xoay camera trái/phải
- **Q / E**: Điều chỉnh độ cao camera (2 - 25 units)

#### Khác
- **ESC**: Thoát chương trình

## 🎨 Chi tiết kỹ thuật

### Cấu trúc Player Animation
```cpp
struct PlayerState {
    float legAngle1, legAngle2;  // Góc chân trái/phải
    float armSwing;               // Góc vung tay
    float bodyTilt;               // Nghiêng thân
    float jumpHeight;             // Độ cao nhảy
};
```

### Hiệu ứng gió
- **windTime**: Thời gian chạy của hiệu ứng gió
- **windStrength**: Cường độ gió (0.0 - 3.0)
- Áp dụng cho: Cỏ, cây cối, tia sáng mặt trời

### Lighting Setup
- **GL_LIGHT0**: Mặt trời/mặt trăng chính
- **GL_LIGHT1, GL_LIGHT2**: Đèn đường (bật ban đêm)
- **Ambient light**: Thay đổi theo thời gian

## 📦 Cài đặt

### Yêu cầu
- **MSYS2** với MinGW-w64
- **FreeGLUT**
- **OpenGL** (thường có sẵn)
- **Windows** (hoặc Linux/macOS với điều chỉnh nhỏ)

### Cài đặt tự động (Windows)
```batch
INSTALL_NOW.bat
```

### Compile thủ công
```batch
compile.bat
```

## 🚀 Chạy chương trình

### Sử dụng script (Khuyến nghị)
```batch
RUN_GAME.bat
```

## 🎯 Sân pickleball chi tiết

- **Kích thước**: 44 feet x 20 feet (theo tiêu chuẩn)
- **Hai bên sân**: Màu xanh lá và đỏ
- **Vạch kẻ trắng**: Boundary lines, center line, service lines
- **Lưới**: Cao 0.914m với lưới dệt chi tiết
- **Trụ lưới**: Hai trụ màu xám ở hai bên

## 🏞️ Các vật thể trong công viên

### Cây cối (8 cây)
- 4 cây ở 4 góc sân
- 4 cây bổ sung tạo không gian công viên
- Mỗi cây có 3 tầng lá và 5 chùm lá nhỏ

### Ghế ngồi (2 cái)
- Chất liệu gỗ màu nâu
- Có lưng tựa và 4 chân
- Đặt hai bên sân

### Đèn đường (2 cái)
- Trụ cao 5m màu xám
- Bóng đèn phát sáng vào ban đêm
- Tạo điểm nhấn cho cảnh đêm

## 🔧 Tùy chỉnh

### Thay đổi màu sắc
Trong code, tìm các dòng `GLfloat xxxColor[]` để thay đổi màu:
- `shirtColor`: Màu áo người chơi
- `pantsColor`: Màu quần
- `grassColor`: Màu cỏ
- `foliageColor`: Màu lá cây

### Điều chỉnh animation speed
Trong hàm `updateBall()`:
```cpp
animationTime += 0.016f;  // Thay đổi giá trị này
```

### Thay đổi số lượng cỏ
Trong hàm `drawGrassField()`:
```cpp
for (float x = -COURT_LENGTH - 8; x < COURT_LENGTH + 8; x += 0.8f)
//                                                              ^ Thay đổi step
```

## 🐛 Xử lý sự cố

### Không compile được
- Kiểm tra MSYS2 đã cài đặt chưa
- Chạy `pacman -S mingw-w64-x86_64-freeglut` trong MSYS2

### Chạy bị lag
- Giảm số lượng grass blades (tăng step value)
- Giảm số cây cối
- Giảm complexity của player model

### Màn hình đen
- Kiểm tra OpenGL drivers
- Thử chạy với quyền Administrator

## 📚 Tài liệu tham khảo

- [OpenGL Documentation](https://www.opengl.org/documentation/)
- [FreeGLUT Documentation](http://freeglut.sourceforge.net/docs/api.php)
- [Pickleball Court Specifications](https://usapickleball.org/)

## 🎓 Học tập

Project này minh họa:
- ✅ 3D modeling cơ bản với primitives
- ✅ Hierarchical transformations (push/pop matrix)
- ✅ Lighting và material properties
- ✅ Animation và time-based updates
- ✅ User input handling
- ✅ Scene composition
- ✅ Wind simulation với sin/cos
- ✅ Day-night cycle implementation

## 👨‍💻 Tác giả

Phát triển như một demonstration project cho OpenGL graphics programming.

---

**Enjoy playing with the Enhanced Pickleball Park Scene! 🎾🌳☀️**
