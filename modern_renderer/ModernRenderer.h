/*
 * ModernRenderer.h
 * Modern OpenGL 3.3+ Renderer with PBR-like materials, shadows, and skybox
 */

#ifndef MODERN_RENDERER_H
#define MODERN_RENDERER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include <vector>
#include <string>

// Material structure
struct Material {
    GLuint diffuseMap;
    GLuint specularMap;
    GLuint normalMap;
    float shininess;
    bool hasNormalMap;
    
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    
    Material() : diffuseMap(0), specularMap(0), normalMap(0), 
                 shininess(32.0f), hasNormalMap(false),
                 ambient(0.2f), diffuse(0.8f), specular(1.0f) {}
};

// Mesh structure
struct Mesh {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    std::vector<unsigned int> indices;
    
    GLuint VAO, VBO, EBO;
    Material material;
    
    Mesh() : VAO(0), VBO(0), EBO(0) {}
    
    void setupMesh();
    void draw();
};

// Directional Light
struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    
    DirectionalLight() : 
        direction(-0.3f, -1.0f, -0.3f),
        ambient(0.2f, 0.2f, 0.2f),
        diffuse(0.8f, 0.8f, 0.8f),
        specular(1.0f, 1.0f, 1.0f) {}
};

// Camera
class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    
    float yaw;
    float pitch;
    float fov;
    
    Camera(glm::vec3 pos = glm::vec3(0.0f, 5.0f, 20.0f));
    
    glm::mat4 getViewMatrix();
    void processKeyboard(int direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset);
    void processMouseScroll(float yoffset);
    
private:
    void updateCameraVectors();
};

// Modern Renderer Class
class ModernRenderer {
public:
    ModernRenderer(int width, int height);
    ~ModernRenderer();
    
    // Initialization
    bool initialize();
    
    // Shadow mapping
    void setupShadowMap(unsigned int resolution = 2048);
    void renderShadowMap(const std::vector<Mesh*>& meshes);
    
    // Skybox
    void setupSkybox(const std::vector<std::string>& faces);
    void renderSkybox();
    
    // Main rendering
    void beginFrame();
    void renderMesh(Mesh* mesh, const glm::mat4& model);
    void renderScene(const std::vector<Mesh*>& meshes);
    void endFrame();
    
    // Getters/Setters
    void setCamera(Camera* cam) { camera = cam; }
    void setLight(const DirectionalLight& light) { dirLight = light; }
    void setFog(const glm::vec3& color, float density, float gradient);
    
    Camera* getCamera() { return camera; }
    
private:
    int screenWidth, screenHeight;
    
    // Shaders
    Shader blinnPhongShader;
    Shader shadowMapShader;
    Shader skyboxShader;
    
    // Shadow mapping
    GLuint depthMapFBO;
    GLuint depthMap;
    unsigned int shadowWidth, shadowHeight;
    glm::mat4 lightSpaceMatrix;
    
    // Skybox
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;
    
    // Lighting
    DirectionalLight dirLight;
    
    // Fog
    glm::vec3 fogColor;
    float fogDensity;
    float fogGradient;
    
    // Camera
    Camera* camera;
    
    // Helper functions
    void calculateTangentSpace(Mesh& mesh);
    GLuint loadTexture(const std::string& path);
    GLuint loadCubemap(const std::vector<std::string>& faces);
};

#endif // MODERN_RENDERER_H
