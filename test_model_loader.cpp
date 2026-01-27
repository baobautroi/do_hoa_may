/*
 * test_model_loader.cpp
 * 
 * Chương trình test đơn giản để kiểm tra ModelLoader
 * Compile: compile_test.bat
 * Run: test_model_loader.exe
 */

#include "ModelLoader.h"
#include <GL/glut.h>
#include <iostream>

ModelLoader testModel;
float rotationAngle = 0.0f;
bool modelLoaded = false;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Setup camera
    gluLookAt(0, 3, 8,    // Camera position
              0, 0, 0,    // Look at point
              0, 1, 0);   // Up vector
    
    // Rotate model for better view
    rotationAngle += 0.5f;
    if (rotationAngle > 360.0f) rotationAngle -= 360.0f;
    
    if (modelLoaded) {
        // Draw the loaded model with rotation
        testModel.render(0, 0, 0, 1.0f, rotationAngle, 0, 1, 0);
    } else {
        // Draw a fallback cube if model failed to load
        glColor3f(1.0f, 0.5f, 0.2f);
        glPushMatrix();
        glRotatef(rotationAngle, 0, 1, 0);
        glutSolidCube(1.5);
        glPopMatrix();
    }
    
    glutSwapBuffers();
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // ~60 FPS
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 27:  // ESC
            exit(0);
            break;
        case 'r':
        case 'R':
            rotationAngle = 0.0f;
            break;
    }
}

void init() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Setup light
    GLfloat lightPos[] = {5.0f, 5.0f, 5.0f, 1.0f};
    GLfloat lightColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    
    // Background color
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    
    // Setup perspective
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0/600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void printHelp() {
    std::cout << "\n====================================\n";
    std::cout << "  Model Loader Test Program\n";
    std::cout << "====================================\n\n";
    std::cout << "Usage:\n";
    std::cout << "  test_model_loader.exe [path_to_model.obj]\n\n";
    std::cout << "Example:\n";
    std::cout << "  test_model_loader.exe models/tree.obj\n\n";
    std::cout << "Controls:\n";
    std::cout << "  ESC - Exit\n";
    std::cout << "  R   - Reset rotation\n\n";
    std::cout << "If no model file is specified, a default cube will be shown.\n";
    std::cout << "====================================\n\n";
}

int main(int argc, char** argv) {
    printHelp();
    
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Model Loader Test - Assimp");
    
    // Initialize OpenGL settings
    init();
    
    // Try to load model if path provided
    if (argc > 1) {
        std::cout << "Attempting to load model: " << argv[1] << "\n\n";
        
        if (testModel.loadModel(argv[1])) {
            modelLoaded = true;
            std::cout << "\n====================================\n";
            std::cout << "Model loaded successfully!\n";
            std::cout << "Meshes: " << testModel.getMeshCount() << "\n";
            
            // Get and print bounds
            float minX, minY, minZ, maxX, maxY, maxZ;
            testModel.getBounds(minX, minY, minZ, maxX, maxY, maxZ);
            std::cout << "Bounds:\n";
            std::cout << "  X: " << minX << " to " << maxX << "\n";
            std::cout << "  Y: " << minY << " to " << maxY << "\n";
            std::cout << "  Z: " << minZ << " to " << maxZ << "\n";
            std::cout << "====================================\n\n";
        } else {
            std::cout << "\n====================================\n";
            std::cout << "Failed to load model!\n";
            std::cout << "Showing fallback cube instead.\n";
            std::cout << "====================================\n\n";
            modelLoaded = false;
        }
    } else {
        std::cout << "No model file specified.\n";
        std::cout << "Showing fallback cube.\n\n";
        modelLoaded = false;
    }
    
    // Setup callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);
    
    // Start main loop
    std::cout << "Starting render loop...\n";
    std::cout << "Close window or press ESC to exit.\n\n";
    glutMainLoop();
    
    return 0;
}

/*
 * HƯỚNG DẪN SỬ DỤNG:
 * 
 * 1. Compile:
 *    compile_test.bat
 * 
 * 2. Chạy với model:
 *    test_model_loader.exe models/tree.obj
 * 
 * 3. Chạy không có model (hiện cube):
 *    test_model_loader.exe
 * 
 * 4. Kiểm tra kết quả:
 *    - Model có xoay tự động không?
 *    - Có hiển thị đúng hình dạng không?
 *    - Console có báo lỗi không?
 * 
 * 5. Nếu model quá lớn/nhỏ:
 *    - Chỉnh số scale trong hàm render()
 *    - Ví dụ: testModel.render(0, 0, 0, 0.5f, ...)  // Scale nhỏ hơn
 *            testModel.render(0, 0, 0, 2.0f, ...)  // Scale lớn hơn
 */
