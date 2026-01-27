/*
 * ModernRenderer.cpp
 * Implementation of Modern OpenGL Renderer
 */

#include "ModernRenderer.h"
#include <stb_image.h>
#include <iostream>

// ============================================
// Camera Implementation
// ============================================

Camera::Camera(glm::vec3 pos) : 
    position(pos), worldUp(0.0f, 1.0f, 0.0f),
    yaw(-90.0f), pitch(0.0f), fov(45.0f)
{
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::processKeyboard(int direction, float deltaTime) {
    float velocity = 5.0f * deltaTime;
    if (direction == 0) position += front * velocity; // Forward
    if (direction == 1) position -= front * velocity; // Backward
    if (direction == 2) position -= right * velocity; // Left
    if (direction == 3) position += right * velocity; // Right
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
    xoffset *= 0.1f;
    yoffset *= 0.1f;
    
    yaw += xoffset;
    pitch += yoffset;
    
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
    fov -= yoffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 45.0f) fov = 45.0f;
}

// ============================================
// Mesh Implementation
// ============================================

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    // Create interleaved vertex data
    std::vector<float> vertices;
    for (size_t i = 0; i < positions.size(); i++) {
        // Position
        vertices.push_back(positions[i].x);
        vertices.push_back(positions[i].y);
        vertices.push_back(positions[i].z);
        // Normal
        vertices.push_back(normals[i].x);
        vertices.push_back(normals[i].y);
        vertices.push_back(normals[i].z);
        // TexCoords
        vertices.push_back(texCoords[i].x);
        vertices.push_back(texCoords[i].y);
        // Tangent
        vertices.push_back(tangents[i].x);
        vertices.push_back(tangents[i].y);
        vertices.push_back(tangents[i].z);
        // Bitangent
        vertices.push_back(bitangents[i].x);
        vertices.push_back(bitangents[i].y);
        vertices.push_back(bitangents[i].z);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    
    // Vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    // TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    // Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    // Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    
    glBindVertexArray(0);
}

void Mesh::draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// ============================================
// ModernRenderer Implementation
// ============================================

ModernRenderer::ModernRenderer(int width, int height) : 
    screenWidth(width), screenHeight(height),
    depthMapFBO(0), depthMap(0), shadowWidth(2048), shadowHeight(2048),
    skyboxVAO(0), skyboxVBO(0), cubemapTexture(0),
    fogColor(0.7f, 0.8f, 0.9f), fogDensity(0.02f), fogGradient(1.5f),
    camera(nullptr)
{
}

ModernRenderer::~ModernRenderer() {
    if (depthMapFBO) glDeleteFramebuffers(1, &depthMapFBO);
    if (depthMap) glDeleteTextures(1, &depthMap);
    if (skyboxVAO) glDeleteVertexArrays(1, &skyboxVAO);
    if (skyboxVBO) glDeleteBuffers(1, &skyboxVBO);
    if (cubemapTexture) glDeleteTextures(1, &cubemapTexture);
}

bool ModernRenderer::initialize() {
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    // OpenGL configuration
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Load shaders
    blinnPhongShader.compile("modern_renderer/shaders/blinn_phong.vert", 
                             "modern_renderer/shaders/blinn_phong.frag");
    shadowMapShader.compile("modern_renderer/shaders/shadow_map.vert",
                            "modern_renderer/shaders/shadow_map.frag");
    skyboxShader.compile("modern_renderer/shaders/skybox.vert",
                        "modern_renderer/shaders/skybox.frag");
    
    // Setup shadow map
    setupShadowMap(2048);
    
    // Create default camera if none exists
    if (!camera) {
        camera = new Camera(glm::vec3(0.0f, 10.0f, 20.0f));
    }
    
    return true;
}

void ModernRenderer::setupShadowMap(unsigned int resolution) {
    shadowWidth = shadowHeight = resolution;
    
    // Create framebuffer
    glGenFramebuffers(1, &depthMapFBO);
    
    // Create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModernRenderer::renderShadowMap(const std::vector<Mesh*>& meshes) {
    // Calculate light space matrix
    float near_plane = 1.0f, far_plane = 50.0f;
    float orthoSize = 20.0f;
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(
        -dirLight.direction * 20.0f,  // Light position
        glm::vec3(0.0f),              // Look at origin
        glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
    );
    lightSpaceMatrix = lightProjection * lightView;
    
    // Render to shadow map
    shadowMapShader.use();
    shadowMapShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    
    glViewport(0, 0, shadowWidth, shadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    glCullFace(GL_FRONT); // Peter panning fix
    for (auto mesh : meshes) {
        glm::mat4 model = glm::mat4(1.0f); // You should pass actual model matrices
        shadowMapShader.setMat4("model", model);
        mesh->draw();
    }
    glCullFace(GL_BACK);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
}

void ModernRenderer::setupSkybox(const std::vector<std::string>& faces) {
    // Skybox vertices
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };
    
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    cubemapTexture = loadCubemap(faces);
}

void ModernRenderer::renderSkybox() {
    glDepthFunc(GL_LEQUAL);
    skyboxShader.use();
    
    glm::mat4 view = glm::mat4(glm::mat3(camera->getViewMatrix())); // Remove translation
    glm::mat4 projection = glm::perspective(glm::radians(camera->fov), 
        (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    glDepthFunc(GL_LESS);
}

void ModernRenderer::beginFrame() {
    glClearColor(fogColor.x, fogColor.y, fogColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ModernRenderer::renderMesh(Mesh* mesh, const glm::mat4& model) {
    blinnPhongShader.use();
    
    // Set matrices
    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera->fov),
        (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    
    blinnPhongShader.setMat4("model", model);
    blinnPhongShader.setMat4("view", view);
    blinnPhongShader.setMat4("projection", projection);
    blinnPhongShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    
    // Set light
    blinnPhongShader.setVec3("dirLight.direction", dirLight.direction);
    blinnPhongShader.setVec3("dirLight.ambient", dirLight.ambient);
    blinnPhongShader.setVec3("dirLight.diffuse", dirLight.diffuse);
    blinnPhongShader.setVec3("dirLight.specular", dirLight.specular);
    
    // Set material
    blinnPhongShader.setFloat("material.shininess", mesh->material.shininess);
    blinnPhongShader.setBool("material.hasNormalMap", mesh->material.hasNormalMap);
    
    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh->material.diffuseMap);
    blinnPhongShader.setInt("material.diffuseMap", 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh->material.specularMap);
    blinnPhongShader.setInt("material.specularMap", 1);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mesh->material.normalMap);
    blinnPhongShader.setInt("material.normalMap", 2);
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    blinnPhongShader.setInt("shadowMap", 3);
    
    // Set camera and fog
    blinnPhongShader.setVec3("viewPos", camera->position);
    blinnPhongShader.setVec3("fogColor", fogColor);
    blinnPhongShader.setFloat("fogDensity", fogDensity);
    blinnPhongShader.setFloat("fogGradient", fogGradient);
    
    mesh->draw();
}

void ModernRenderer::renderScene(const std::vector<Mesh*>& meshes) {
    // 1. Render shadow map
    renderShadowMap(meshes);
    
    // 2. Render scene
    beginFrame();
    
    // Render skybox first
    if (cubemapTexture) {
        renderSkybox();
    }
    
    // Render all meshes
    for (auto mesh : meshes) {
        glm::mat4 model = glm::mat4(1.0f); // You should manage model transforms
        renderMesh(mesh, model);
    }
}

void ModernRenderer::endFrame() {
    // Swap buffers handled externally
}

void ModernRenderer::setFog(const glm::vec3& color, float density, float gradient) {
    fogColor = color;
    fogDensity = density;
    fogGradient = gradient;
}

// Helper: Calculate tangent space
void ModernRenderer::calculateTangentSpace(Mesh& mesh) {
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        unsigned int i0 = mesh.indices[i];
        unsigned int i1 = mesh.indices[i + 1];
        unsigned int i2 = mesh.indices[i + 2];
        
        glm::vec3 edge1 = mesh.positions[i1] - mesh.positions[i0];
        glm::vec3 edge2 = mesh.positions[i2] - mesh.positions[i0];
        glm::vec2 deltaUV1 = mesh.texCoords[i1] - mesh.texCoords[i0];
        glm::vec2 deltaUV2 = mesh.texCoords[i2] - mesh.texCoords[i0];
        
        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        
        glm::vec3 tangent, bitangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        
        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        
        mesh.tangents[i0] += tangent;
        mesh.tangents[i1] += tangent;
        mesh.tangents[i2] += tangent;
        
        mesh.bitangents[i0] += bitangent;
        mesh.bitangents[i1] += bitangent;
        mesh.bitangents[i2] += bitangent;
    }
    
    // Normalize
    for (auto& t : mesh.tangents) t = glm::normalize(t);
    for (auto& b : mesh.bitangents) b = glm::normalize(b);
}

// Helper: Load texture
GLuint ModernRenderer::loadTexture(const std::string& path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }
    
    return textureID;
}

// Helper: Load cubemap
GLuint ModernRenderer::loadCubemap(const std::vector<std::string>& faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    return textureID;
}
