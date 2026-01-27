/*
 * example_modern_renderer.cpp
 * 
 * Example usage of ModernRenderer for Pickleball Scene
 * Demonstrates how to integrate modern OpenGL 3.3+ rendering
 */

#include "modern_renderer/ModernRenderer.h"
#include "modern_renderer/Shader.h"
#include <GLFW/glfw3.h>
#include <iostream>

// Window dimensions
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Camera
Camera camera(glm::vec3(0.0f, 10.0f, 20.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Pickleball - Modern Renderer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize Modern Renderer
    ModernRenderer renderer(SCR_WIDTH, SCR_HEIGHT);
    if (!renderer.initialize()) {
        std::cout << "Failed to initialize renderer" << std::endl;
        return -1;
    }
    
    renderer.setCamera(&camera);
    
    // Setup directional light (sun)
    DirectionalLight sunLight;
    sunLight.direction = glm::vec3(-0.3f, -1.0f, -0.3f);
    sunLight.ambient = glm::vec3(0.2f, 0.2f, 0.25f);
    sunLight.diffuse = glm::vec3(0.9f, 0.9f, 0.8f);
    sunLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    renderer.setLight(sunLight);
    
    // Setup fog
    renderer.setFog(glm::vec3(0.7f, 0.8f, 0.9f), 0.015f, 1.5f);
    
    // Setup skybox
    std::vector<std::string> skyboxFaces = {
        "textures/skybox/right.jpg",
        "textures/skybox/left.jpg",
        "textures/skybox/top.jpg",
        "textures/skybox/bottom.jpg",
        "textures/skybox/front.jpg",
        "textures/skybox/back.jpg"
    };
    renderer.setupSkybox(skyboxFaces);
    
    // Create sample meshes
    std::vector<Mesh*> sceneMeshes;
    
    // TODO: Load your actual models using Assimp
    // Example:
    // Mesh* courtMesh = loadModelFromFile("models/court.obj");
    // Mesh* treeMesh = loadModelFromFile("models/tree.obj");
    // Mesh* playerMesh = loadModelFromFile("models/player.obj");
    // sceneMeshes.push_back(courtMesh);
    // sceneMeshes.push_back(treeMesh);
    
    std::cout << "=== Modern Pickleball Renderer ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  Scroll - Zoom" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Render scene
        renderer.renderScene(sceneMeshes);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    for (auto mesh : sceneMeshes) {
        delete mesh;
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(0, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(1, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(2, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(3, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll(yoffset);
}
