/*
 * example_usage.cpp
 * 
 * Ví dụ cách sử dụng ModelLoader để thay thế các hình vẽ đơn giản
 * bằng model 3D thực tế
 */

#include "ModelLoader.h"
#include <GL/glut.h>

// Khai báo các model loader toàn cục
ModelLoader treeModel;      // Model cây
ModelLoader paddleModel;    // Model vợt
ModelLoader playerModel;    // Model nhân vật

// Khởi tạo - gọi sau khi khởi tạo OpenGL/GLUT
void initModels() {
    // Load model cây (thay thế drawTree)
    if (!treeModel.loadModel("models/tree.obj")) {
        std::cerr << "Failed to load tree model, using fallback rendering" << std::endl;
    }
    
    // Load model vợt pickleball (thay thế vẽ vợt đơn giản)
    if (!paddleModel.loadModel("models/paddle.obj")) {
        std::cerr << "Failed to load paddle model, using fallback rendering" << std::endl;
    }
    
    // Load model nhân vật (thay thế drawPlayer)
    if (!playerModel.loadModel("models/player.obj")) {
        std::cerr << "Failed to load player model, using fallback rendering" << std::endl;
    }
    
    std::cout << "All models loaded successfully!" << std::endl;
}

// Ví dụ vẽ cây sử dụng model thực
void drawTreeWithModel(float x, float z) {
    if (treeModel.getMeshCount() > 0) {
        // Vẽ bằng model 3D
        treeModel.render(x, 0, z, 2.0f);  // x, y, z, scale
    } else {
        // Fallback: vẽ bằng hình học đơn giản (code cũ)
        // drawTree(x, z);  // Hàm cũ của bạn
    }
}

// Ví dụ vẽ vợt với animation
void drawPaddleWithModel(float x, float y, float z, float swingAngle) {
    if (paddleModel.getMeshCount() > 0) {
        // Vẽ vợt với góc xoay (swing animation)
        paddleModel.render(x, y, z, 0.5f, swingAngle, 0, 1, 0);
        // render(x, y, z, scale, rotateAngle, rotateX, rotateY, rotateZ)
    } else {
        // Fallback: vẽ vợt đơn giản
        glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(swingAngle, 0, 1, 0);
        glScalef(0.3f, 0.05f, 0.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}

// Ví dụ vẽ người chơi với model thực
void drawPlayerWithModel(float x, float z, float bodyRotation, float armSwing) {
    if (playerModel.getMeshCount() > 0) {
        glPushMatrix();
        glTranslatef(x, 0, z);
        glRotatef(bodyRotation, 0, 1, 0);
        
        // Vẽ thân người
        playerModel.render(0, 0, 0, 1.0f);
        
        // Vẽ vợt trong tay (với animation)
        drawPaddleWithModel(0.4f, 1.2f, 0.2f, armSwing);
        
        glPopMatrix();
    } else {
        // Fallback: code vẽ người đơn giản cũ
        // drawPlayerSimple(x, z, bodyRotation, armSwing);
    }
}

// Tích hợp vào display callback của bạn
void displayExample() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup camera, lighting...
    
    // Vẽ cây xung quanh sân
    float treePositions[][2] = {
        {-10.0f, -8.0f},
        {-10.0f, 8.0f},
        {10.0f, -8.0f},
        {10.0f, 8.0f}
    };
    
    for (int i = 0; i < 4; i++) {
        drawTreeWithModel(treePositions[i][0], treePositions[i][1]);
    }
    
    // Vẽ người chơi 1
    drawPlayerWithModel(-5.0f, 0.0f, 0.0f, 30.0f);
    
    // Vẽ người chơi 2
    drawPlayerWithModel(5.0f, 0.0f, 180.0f, -20.0f);
    
    glutSwapBuffers();
}

// Main function mẫu
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 800);
    glutCreateWindow("Pickleball with 3D Models");
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Load all 3D models
    initModels();
    
    // Setup callbacks
    glutDisplayFunc(displayExample);
    
    glutMainLoop();
    return 0;
}

/*
 * HƯỚNG DẪN TÍCH HỢP VÀO CODE CỦA BẠN:
 * 
 * 1. Thêm #include "ModelLoader.h" vào đầu file pickleball_scene.cpp
 * 
 * 2. Khai báo các ModelLoader toàn cục:
 *    ModelLoader treeModel;
 *    ModelLoader paddleModel;
 *    ModelLoader playerModel;
 * 
 * 3. Trong hàm main(), sau khi khởi tạo GLUT, gọi:
 *    initModels();
 * 
 * 4. Thay thế các hàm vẽ hiện tại:
 *    - Thay drawTree() bằng drawTreeWithModel()
 *    - Thay phần vẽ vợt bằng drawPaddleWithModel()
 *    - Thay phần vẽ người chơi bằng drawPlayerWithModel()
 * 
 * 5. Chuẩn bị file .obj:
 *    - Tạo thư mục "models" trong cùng thư mục với .exe
 *    - Đặt các file: tree.obj, paddle.obj, player.obj vào đó
 *    - Có thể tải free models từ: 
 *      - sketchfab.com
 *      - free3d.com
 *      - cgtrader.com
 */
