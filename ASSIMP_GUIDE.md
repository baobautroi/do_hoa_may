# Hướng dẫn cài đặt Assimp cho dự án OpenGL

## Bước 1: Cài đặt Assimp qua MSYS2

Mở MSYS2 MinGW 64-bit và chạy lệnh sau:

```bash
pacman -S mingw-w64-x86_64-assimp
```

Lệnh này sẽ cài đặt:
- Thư viện Assimp
- Các dependencies cần thiết
- Header files

## Bước 2: Kiểm tra cài đặt

Sau khi cài đặt, kiểm tra xem Assimp đã được cài chưa:

```bash
pacman -Qs assimp
```

Bạn sẽ thấy output tương tự:
```
local/mingw-w64-x86_64-assimp 5.x.x-x
    Open Asset Import Library
```

## Bước 3: Compile code với Assimp

### Cách 1: Sử dụng file compile_with_assimp.bat (đã tạo sẵn)

Chỉ cần double-click file `compile_with_assimp.bat`

### Cách 2: Compile thủ công

```bash
g++ pickleball_scene.cpp ModelLoader.cpp -o pickleball_scene.exe \
    -lfreeglut -lopengl32 -lglu32 -lassimp -std=c++11
```

## Bước 4: Cấu trúc thư mục

```
dohoa (2)/
├── pickleball_scene.cpp
├── ModelLoader.h
├── ModelLoader.cpp
├── stb_image.h
├── example_usage.cpp
├── compile_with_assimp.bat
├── RUN_GAME.bat
└── models/              (tạo thư mục này)
    ├── tree.obj        (file model cây)
    ├── paddle.obj      (file model vợt)
    ├── player.obj      (file model nhân vật)
    └── textures/       (nếu có texture)
        ├── tree_texture.png
        ├── paddle_texture.png
        └── player_texture.png
```

## Bước 5: Tìm và tải model 3D miễn phí

### Websites để tải model .obj miễn phí:

1. **Sketchfab** (https://sketchfab.com)
   - Filter: "Downloadable" + "Free"
   - Format: OBJ

2. **Free3D** (https://free3d.com)
   - Nhiều model miễn phí
   - Download format: OBJ

3. **CGTrader Free** (https://www.cgtrader.com/free-3d-models)
   - Chọn "Free" và "OBJ" format

4. **TurboSquid Free** (https://www.turbosquid.com/Search/3D-Models/free)
   - Filter theo "Free" và "OBJ"

### Từ khóa tìm kiếm:

- **Cho cây**: "tree obj", "low poly tree", "cartoon tree"
- **Cho vợt**: "paddle obj", "tennis racket", "pickleball paddle"
- **Cho nhân vật**: "low poly character", "rigged character obj", "person obj"

### Tips khi chọn model:

1. **Chọn "Low Poly"** - chạy nhanh hơn, phù hợp với game
2. **Kiểm tra license** - đảm bảo free for personal use
3. **Kiểm tra format** - phải có .obj hoặc convertible
4. **Size nhỏ** - dưới 50MB là tốt nhất

## Bước 6: Xử lý lỗi thường gặp

### Lỗi: "cannot find -lassimp"

**Giải pháp:**
```bash
# Reinstall assimp
pacman -S mingw-w64-x86_64-assimp
```

### Lỗi: "assimp/Importer.hpp: No such file or directory"

**Giải pháp:**
Kiểm tra đường dẫn include. Với MSYS2, header sẽ ở:
```
C:\msys64\mingw64\include\assimp\
```

Nếu vẫn lỗi, thêm flag:
```bash
-IC:/msys64/mingw64/include
```

### Lỗi: Model không hiển thị

**Giải pháp:**
1. Kiểm tra đường dẫn đến file .obj
2. Kiểm tra scale của model (có thể quá lớn hoặc quá nhỏ)
3. Check console output để xem có lỗi load không

### Model bị đen/tối

**Giải pháp:**
1. Kiểm tra lighting trong OpenGL
2. Đảm bảo normals được tính đúng
3. Thử set màu diffuse sáng hơn

## Bước 7: Test đơn giản

Tạo file `test_model.cpp`:

```cpp
#include "ModelLoader.h"
#include <GL/glut.h>

ModelLoader testModel;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 2, 5, 0, 0, 0, 0, 1, 0);
    
    testModel.render(0, 0, 0, 1.0f);
    
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Test Model Loader");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Load model
    if (!testModel.loadModel("models/tree.obj")) {
        printf("Failed to load model!\n");
        return 1;
    }
    
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
```

Compile:
```bash
g++ test_model.cpp ModelLoader.cpp -o test.exe -lfreeglut -lopengl32 -lglu32 -lassimp -std=c++11
```

## Bước 8: Tối ưu hóa

Để model load nhanh hơn và render mượt hơn:

1. **Giảm polygon count** - dùng Blender để decimate
2. **Nén texture** - resize ảnh về kích thước hợp lý (512x512 hoặc 1024x1024)
3. **Cache models** - load một lần khi khởi động
4. **LOD (Level of Detail)** - dùng model đơn giản hơn khi xa camera

## Links hữu ích

- Assimp Documentation: http://assimp.org/
- OpenGL Tutorial: https://learnopengl.com/
- Model conversion: https://www.blender.org/
- stb_image: https://github.com/nothings/stb

## Support

Nếu gặp vấn đề, kiểm tra:
1. Console output khi compile
2. Console output khi chạy program
3. Đường dẫn đến file model có đúng không
4. MSYS2 MinGW64 đã được add vào PATH chưa
