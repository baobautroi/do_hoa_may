/*
 * ModelLoader.h
 * A comprehensive model loader using Assimp for OpenGL
 * 
 * Features:
 * - Load .obj, .fbx, .dae and other 3D model formats
 * - Mesh data extraction (vertices, normals, texture coordinates)
 * - Material and texture loading
 * - Multiple mesh support per model
 * - Simple rendering interface
 */

#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glut.h>
#include <vector>
#include <string>
#include <iostream>
#include <map>

// Structure to hold vertex data
struct Vertex {
    float position[3];
    float normal[3];
    float texCoords[2];
};

// Structure to hold texture information
struct Texture {
    GLuint id;
    std::string type;  // diffuse, specular, normal, etc.
    std::string path;
};

// Structure to hold mesh data
struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    
    // Material properties
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float shininess;
    
    // OpenGL buffers (optional, for VBO usage)
    GLuint VAO, VBO, EBO;
    bool useVBO;
    
    Mesh() : VAO(0), VBO(0), EBO(0), useVBO(false), shininess(32.0f) {
        ambient[0] = ambient[1] = ambient[2] = 0.2f; ambient[3] = 1.0f;
        diffuse[0] = diffuse[1] = diffuse[2] = 0.8f; diffuse[3] = 1.0f;
        specular[0] = specular[1] = specular[2] = 1.0f; specular[3] = 1.0f;
    }
};

class ModelLoader {
private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::map<std::string, Texture> texturesLoaded;  // Cache loaded textures
    
    // Process nodes in the scene graph
    void processNode(aiNode* node, const aiScene* scene);
    
    // Process individual mesh
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    
    // Load material textures
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    
    // Load texture from file
    GLuint loadTextureFromFile(const char* path);
    
public:
    ModelLoader();
    ~ModelLoader();
    
    // Load model from file
    bool loadModel(const std::string& path);
    
    // Render the entire model
    void render();
    
    // Render with custom transformations
    void render(float x, float y, float z, float scale = 1.0f, 
                float rotateAngle = 0.0f, float rotateX = 0.0f, 
                float rotateY = 1.0f, float rotateZ = 0.0f);
    
    // Get mesh count
    int getMeshCount() const { return meshes.size(); }
    
    // Clear all loaded data
    void clear();
    
    // Enable/disable textures
    void setUseTextures(bool use);
    
    // Get bounds (useful for auto-scaling)
    void getBounds(float& minX, float& minY, float& minZ, 
                   float& maxX, float& maxY, float& maxZ);
};

#endif // MODEL_LOADER_H
