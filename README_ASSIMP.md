# Tích hợp Assimp - Load Model 3D vào Pickleball Scene

## 📋 Tổng quan

Bạn đã có sẵn class **ModelLoader** hoàn chỉnh để load các model 3D thực tế (định dạng .obj, .fbx, .dae, v.v.) vào dự án OpenGL sử dụng thư viện Assimp.

## 📁 Cấu trúc Files

```
dohoa (2)/
├── 📄 ModelLoader.h              - Header class ModelLoader
├── 📄 ModelLoader.cpp            - Implementation class ModelLoader  
├── 📄 stb_image.h                - Thư viện load texture
├── 📄 example_usage.cpp          - Ví dụ cách sử dụng
├── 📄 test_model_loader.cpp      - Chương trình test
│
├── 🔧 INSTALL_ASSIMP.bat         - Cài đặt Assimp tự động
├── 🔧 compile_with_assimp.bat    - Compile project với Assimp
├── 🔧 compile_test.bat           - Compile chương trình test
├── 🔧 RUN_GAME.bat               - Chạy game
│
├── 📖 ASSIMP_GUIDE.md            - Hướng dẫn chi tiết
└── 📖 README_ASSIMP.md           - File này
```

## 🚀 Bắt đầu nhanh (Quick Start)

### Bước 1: Cài đặt Assimp

Double-click file:
```
INSTALL_ASSIMP.bat
```

Hoặc chạy thủ công trong MSYS2:
```bash
pacman -S mingw-w64-x86_64-assimp
```

### Bước 2: Test ModelLoader

Compile chương trình test:
```
compile_test.bat
```

Chạy test (hiển thị cube nếu không có model):
```
test_model_loader.exe
```

Hoặc với model cụ thể:
```
test_model_loader.exe models/tree.obj
```

### Bước 3: Tích hợp vào code chính

Xem file `example_usage.cpp` để biết cách sử dụng.

## 🎯 Features của ModelLoader

### ✅ Các tính năng chính:

- ✔️ **Load nhiều format**: .obj, .fbx, .dae, .blend, .3ds, v.v.
- ✔️ **Mesh processing**: Vertices, normals, texture coordinates
- ✔️ **Material support**: Ambient, diffuse, specular, shininess
- ✔️ **Texture loading**: Diffuse maps, specular maps
- ✔️ **Multiple meshes**: Hỗ trợ model có nhiều mesh
- ✔️ **Transform support**: Position, rotation, scale
- ✔️ **Bounds calculation**: Tự động tính kích thước model
- ✔️ **Texture caching**: Tối ưu bộ nhớ khi load nhiều model

### 📝 API chính:

```cpp
ModelLoader model;

// Load model
bool success = model.loadModel("models/tree.obj");

// Render đơn giản
model.render();

// Render với transform
model.render(x, y, z, scale, rotateAngle, rotateX, rotateY, rotateZ);

// Lấy thông tin
int meshCount = model.getMeshCount();
model.getBounds(minX, minY, minZ, maxX, maxY, maxZ);

// Xóa model
model.clear();
```

## 💡 Ví dụ sử dụng trong Pickleball Scene

### 1. Khai báo global:

```cpp
#include "ModelLoader.h"

ModelLoader treeModel;
ModelLoader paddleModel;
ModelLoader playerModel;
```

### 2. Load models trong main():

```cpp
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    // ... setup OpenGL ...
    
    // Load models
    treeModel.loadModel("models/tree.obj");
    paddleModel.loadModel("models/paddle.obj");
    playerModel.loadModel("models/player.obj");
    
    glutMainLoop();
    return 0;
}
```

### 3. Vẽ trong display():

```cpp
void display() {
    // ... setup camera, lighting ...
    
    // Vẽ cây
    treeModel.render(-10, 0, -8, 2.0f);  // x, y, z, scale
    
    // Vẽ người chơi với rotation
    playerModel.render(-5, 0, 0, 1.0f, 180, 0, 1, 0);
    
    // Vẽ vợt với animation
    float swingAngle = sin(animationTime) * 30.0f;
    paddleModel.render(handX, handY, handZ, 0.5f, swingAngle, 0, 1, 0);
    
    glutSwapBuffers();
}
```

## 📥 Tìm model 3D miễn phí

### Websites gợi ý:

1. **Sketchfab** - https://sketchfab.com
   - Filter: Downloadable + Free
   - Format: OBJ
   - Tốt nhất cho models chất lượng cao

2. **Free3D** - https://free3d.com
   - Nhiều model miễn phí
   - Dễ tải, không cần đăng ký

3. **CGTrader** - https://www.cgtrader.com/free-3d-models
   - Professional quality
   - Cần đăng ký để tải

4. **TurboSquid** - https://www.turbosquid.com/Search/3D-Models/free

### Từ khóa tìm kiếm:

| Đối tượng | Keywords |
|-----------|----------|
| 🌳 Cây | `low poly tree obj`, `cartoon tree`, `stylized tree` |
| 🎾 Vợt | `paddle obj`, `tennis racket`, `pickleball paddle` |
| 🧍 Người | `low poly character obj`, `rigged character`, `cartoon person` |

### Tips chọn model:

- ✅ **Low Poly** (< 10,000 triangles) - Chạy mượt
- ✅ **Có texture** - Đẹp hơn
- ✅ **Format OBJ** - Tương thích tốt nhất
- ✅ **License free** - Tránh vấn đề bản quyền
- ❌ Tránh model quá phức tạp (> 100,000 triangles)

## 🛠️ Compile và Run

### Compile with Assimp:

```bash
compile_with_assimp.bat
```

Hoặc thủ công:
```bash
g++ pickleball_scene.cpp ModelLoader.cpp -o pickleball_scene.exe \
    -lfreeglut -lopengl32 -lglu32 -lassimp -std=c++11
```

### Run:

```bash
RUN_GAME.bat
```

hoặc:
```bash
./pickleball_scene.exe
```

## 🐛 Troubleshooting

### Lỗi: "cannot find -lassimp"

**Nguyên nhân**: Chưa cài Assimp

**Giải pháp**:
```bash
INSTALL_ASSIMP.bat
```

### Lỗi: Model không hiển thị

**Kiểm tra**:
1. ✅ Đường dẫn đến .obj file có đúng không?
2. ✅ Model có quá lớn/nhỏ không? (thử điều chỉnh scale)
3. ✅ Console có báo lỗi gì không?

**Thử**:
```cpp
// In console để debug
if (!model.loadModel("models/tree.obj")) {
    printf("FAILED TO LOAD MODEL!\n");
}
printf("Mesh count: %d\n", model.getMeshCount());
```

### Model bị tối/đen

**Nguyên nhân**: Thiếu lighting hoặc normals sai

**Giải pháp**:
```cpp
// Đảm bảo enable lighting
glEnable(GL_LIGHTING);
glEnable(GL_LIGHT0);

// Set ambient light cao hơn
GLfloat ambient[] = {0.5f, 0.5f, 0.5f, 1.0f};
glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
```

### Model bị lộn ngược

**Nguyên nhân**: Coordinate system khác nhau

**Giải pháp**:
```cpp
// Xoay model khi render
model.render(x, y, z, scale, 180, 1, 0, 0);  // Flip by X axis
// hoặc
model.render(x, y, z, scale, 180, 0, 1, 0);  // Flip by Y axis
```

## 📚 Đọc thêm

- **ASSIMP_GUIDE.md** - Hướng dẫn chi tiết
- **example_usage.cpp** - Code mẫu
- **test_model_loader.cpp** - Test program

## 🎨 Workflow đề xuất

1. **Tìm model** trên Sketchfab/Free3D
2. **Download** định dạng OBJ
3. **Test** với `test_model_loader.exe`
4. **Điều chỉnh** scale/rotation nếu cần
5. **Tích hợp** vào code chính
6. **Compile** với `compile_with_assimp.bat`
7. **Chạy** với `RUN_GAME.bat`

## 💪 Next Steps

Sau khi model đã load thành công:

1. **Animation** - Thêm keyframe animation từ .fbx files
2. **Texture** - Thêm normal maps, specular maps
3. **LOD** - Level of Detail cho performance
4. **Physics** - Collision detection với bounding boxes
5. **Optimization** - VBO/VAO cho rendering nhanh hơn

## 🤝 Support

Nếu gặp khó khăn:

1. Kiểm tra console output kỹ
2. Xem file ASSIMP_GUIDE.md
3. Test với test_model_loader.exe trước
4. Đảm bảo MSYS2 và Assimp đã cài đặt đúng

---

**Chúc bạn thành công!** 🎉

Giờ bạn có thể thay thế các cube/sphere đơn giản bằng model 3D thực tế đẹp mắt!
