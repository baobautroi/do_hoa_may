# Kế hoạch Nâng cấp Đồ họa Pickleball Scene

## Tổng quan
Nâng cấp từ OpenGL cổ điển sang đồ họa chất lượng cao hơn với 5 tính năng chính.

## 1. ✅ Phong/Blinn-Phong Shading (Dễ - Ưu tiên cao)
**Mục tiêu:** Tạo hiệu ứng ánh sáng phản chiếu, mảng sáng/tối rõ ràng

**Implementation:**
- Bật GL_LIGHTING và GL_LIGHT0
- Set up material properties (ambient, diffuse, specular, shininess)
- Set up light position (theo mặt trời)
- Enable GL_COLOR_MATERIAL để màu vẫn hoạt động
- Đảm bảo tất cả object có normal vectors

**Code cần thêm:**
```cpp
// Enable lighting
glEnable(GL_LIGHTING);
glEnable(GL_LIGHT0);
glEnable(GL_COLOR_MATERIAL);
glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

// Set light position
GLfloat lightPos[] = {sunX, sunY, sunZ, 1.0f};
glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

// Set material properties
GLfloat specular[] = {0.3f, 0.3f, 0.3f, 1.0f};
GLfloat shininess[] = {32.0f};
glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
```

## 2. ⚠️ Shadow Mapping (Khó - OpenGL 1.x không hỗ trợ tốt)
**Vấn đề:** Cần FBO (Frame Buffer Objects) và shaders - không có trong OpenGL cổ điển

**Giải pháp thay thế:**
- **Fake shadows:** Vẽ ellipse đen mờ dưới chân người chơi/cây
- **Projected shadows:** Dùng stencil buffer để project shadow lên mặt đất
- **Upgrade lên Modern OpenGL:** Cần viết lại toàn bộ với shaders

**Recommended:** Fake shadows đơn giản
```cpp
void drawSimpleShadow(float x, float z, float radius) {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f); // Bóng mờ
    
    glPushMatrix();
    glTranslatef(x, 0.01f, z);
    glRotatef(-90, 1, 0, 0);
    
    // Vẽ disk
    GLUquadricObj *quadric = gluNewQuadric();
    gluDisk(quadric, 0, radius, 20, 1);
    gluDeleteQuadric(quadric);
    
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}
```

## 3. ✅ Textures (Trung bình - Khả thi)
**Mục tiêu:** Thêm texture cho cỏ, mặt sân, lưới

**Cần:**
- Thư viện load ảnh: SOIL (Simple OpenGL Image Library) hoặc STB_IMAGE
- Texture files: grass.jpg, court_surface.jpg

**Code:**
```cpp
GLuint loadTexture(const char* filename) {
    // Sử dụng SOIL hoặc STB_IMAGE
    // Return texture ID
}

// Khi vẽ
glEnable(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, grassTexture);
glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(...);
    glTexCoord2f(1, 0); glVertex3f(...);
    glTexCoord2f(1, 1); glVertex3f(...);
    glTexCoord2f(0, 1); glVertex3f(...);
glEnd();
```

## 4. ✅ Skybox (Trung bình - Khả thi)
**Mục tiêu:** Thay background màu trơn bằng skybox

**Cần:**
- 6 ảnh skybox (top, bottom, left, right, front, back)
- Hoặc dùng sphere mapping đơn giản hơn

**Code:**
```cpp
void drawSkybox() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Vẽ cube lớn bao quanh scene với texture
    // hoặc vẽ sphere với gradient color
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}
```

## 5. ✅ Anti-aliasing MSAA (Dễ - Chỉ cần config)
**Mục tiêu:** Làm mịn các cạnh

**Code:**
```cpp
// Trong main(), trước glutCreateWindow()
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);

// Hoặc enable sau khi init
glEnable(GL_MULTISAMPLE);
glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

// Enable line smoothing
glEnable(GL_LINE_SMOOTH);
glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
```

## Thứ tự triển khai đề xuất:
1. **MSAA** (5 phút) - Chỉ cần thêm vài dòng
2. **Phong Shading** (30 phút) - Cải thiện visual rõ rệt
3. **Simple Shadows** (1 giờ) - Fake shadows dưới chân objects
4. **Skybox** (1 giờ) - Gradient sphere hoặc texture cube
5. **Textures** (2 giờ) - Cần setup SOIL/STB_IMAGE

## Lưu ý:
- OpenGL cổ điển (1.x/2.x) có giới hạn
- Để shadow mapping thực sự cần Modern OpenGL 3.3+ với shaders
- Có thể migrate dần sang Modern OpenGL nếu muốn đồ họa AAA
