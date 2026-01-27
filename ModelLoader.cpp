/*
 * ModelLoader.cpp
 * Implementation of the ModelLoader class
 */

#include "ModelLoader.h"
#include <cstring>

// For image loading - we'll use a simple BMP loader or you can use SOIL/stb_image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  // You'll need to download this header

ModelLoader::ModelLoader() {
}

ModelLoader::~ModelLoader() {
    clear();
}

bool ModelLoader::loadModel(const std::string& path) {
    // Clear any previously loaded model
    clear();
    
    // Create an Assimp importer
    Assimp::Importer importer;
    
    // Read the file with post-processing options
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate |           // Convert polygons to triangles
        aiProcess_FlipUVs |               // Flip texture coordinates
        aiProcess_GenNormals |            // Generate normals if missing
        aiProcess_CalcTangentSpace |      // Calculate tangents for normal mapping
        aiProcess_JoinIdenticalVertices | // Optimize vertex count
        aiProcess_SortByPType             // Sort by primitive type
    );
    
    // Check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return false;
    }
    
    // Extract directory from path
    directory = path.substr(0, path.find_last_of('/'));
    if (directory == path) {
        directory = path.substr(0, path.find_last_of('\\'));
    }
    
    // Process all nodes starting from root
    processNode(scene->mRootNode, scene);
    
    std::cout << "Model loaded successfully: " << path << std::endl;
    std::cout << "Meshes: " << meshes.size() << std::endl;
    
    return true;
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene) {
    // Process all meshes in current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    
    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene) {
    Mesh resultMesh;
    
    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        
        // Position
        vertex.position[0] = mesh->mVertices[i].x;
        vertex.position[1] = mesh->mVertices[i].y;
        vertex.position[2] = mesh->mVertices[i].z;
        
        // Normals
        if (mesh->HasNormals()) {
            vertex.normal[0] = mesh->mNormals[i].x;
            vertex.normal[1] = mesh->mNormals[i].y;
            vertex.normal[2] = mesh->mNormals[i].z;
        } else {
            vertex.normal[0] = 0.0f;
            vertex.normal[1] = 1.0f;
            vertex.normal[2] = 0.0f;
        }
        
        // Texture coordinates (only first set)
        if (mesh->mTextureCoords[0]) {
            vertex.texCoords[0] = mesh->mTextureCoords[0][i].x;
            vertex.texCoords[1] = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.texCoords[0] = 0.0f;
            vertex.texCoords[1] = 0.0f;
        }
        
        resultMesh.vertices.push_back(vertex);
    }
    
    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            resultMesh.indices.push_back(face.mIndices[j]);
        }
    }
    
    // Process materials
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        // Load diffuse textures
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, 
            aiTextureType_DIFFUSE, "texture_diffuse");
        resultMesh.textures.insert(resultMesh.textures.end(), 
            diffuseMaps.begin(), diffuseMaps.end());
        
        // Load specular textures
        std::vector<Texture> specularMaps = loadMaterialTextures(material, 
            aiTextureType_SPECULAR, "texture_specular");
        resultMesh.textures.insert(resultMesh.textures.end(), 
            specularMaps.begin(), specularMaps.end());
        
        // Get material colors
        aiColor4D ambient, diffuse, specular;
        float shininess;
        
        if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient) == AI_SUCCESS) {
            resultMesh.ambient[0] = ambient.r;
            resultMesh.ambient[1] = ambient.g;
            resultMesh.ambient[2] = ambient.b;
            resultMesh.ambient[3] = ambient.a;
        }
        
        if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == AI_SUCCESS) {
            resultMesh.diffuse[0] = diffuse.r;
            resultMesh.diffuse[1] = diffuse.g;
            resultMesh.diffuse[2] = diffuse.b;
            resultMesh.diffuse[3] = diffuse.a;
        }
        
        if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular) == AI_SUCCESS) {
            resultMesh.specular[0] = specular.r;
            resultMesh.specular[1] = specular.g;
            resultMesh.specular[2] = specular.b;
            resultMesh.specular[3] = specular.a;
        }
        
        if (aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess) == AI_SUCCESS) {
            resultMesh.shininess = shininess;
        }
    }
    
    return resultMesh;
}

std::vector<Texture> ModelLoader::loadMaterialTextures(aiMaterial* mat, 
    aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;
    
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        
        // Check if texture was already loaded
        std::string texPath = std::string(str.C_Str());
        
        if (texturesLoaded.find(texPath) != texturesLoaded.end()) {
            // Already loaded, reuse it
            textures.push_back(texturesLoaded[texPath]);
        } else {
            // Load new texture
            Texture texture;
            std::string fullPath = directory + "/" + texPath;
            texture.id = loadTextureFromFile(fullPath.c_str());
            texture.type = typeName;
            texture.path = texPath;
            
            textures.push_back(texture);
            texturesLoaded[texPath] = texture;  // Cache it
        }
    }
    
    return textures;
}

GLuint ModelLoader::loadTextureFromFile(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        // Use gluBuild2DMipmaps for OpenGL 1.x compatibility (GLUT-friendly)
        gluBuild2DMipmaps(GL_TEXTURE_2D, format == GL_RGBA ? 4 : 3, width, height, format, GL_UNSIGNED_BYTE, data);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
        std::cout << "Texture loaded: " << path << std::endl;
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }
    
    return textureID;
}

void ModelLoader::render() {
    for (unsigned int i = 0; i < meshes.size(); i++) {
        Mesh& mesh = meshes[i];
        
        // Set material properties
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mesh.ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mesh.diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mesh.specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mesh.shininess);
        
        // Bind textures if available
        if (!mesh.textures.empty()) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, mesh.textures[0].id);
        }
        
        // Render mesh using immediate mode (compatible with GLUT)
        glBegin(GL_TRIANGLES);
        for (unsigned int j = 0; j < mesh.indices.size(); j++) {
            Vertex& v = mesh.vertices[mesh.indices[j]];
            
            glNormal3fv(v.normal);
            glTexCoord2fv(v.texCoords);
            glVertex3fv(v.position);
        }
        glEnd();
        
        if (!mesh.textures.empty()) {
            glDisable(GL_TEXTURE_2D);
        }
    }
}

void ModelLoader::render(float x, float y, float z, float scale, 
                         float rotateAngle, float rotateX, float rotateY, float rotateZ) {
    glPushMatrix();
    
    // Apply transformations
    glTranslatef(x, y, z);
    glRotatef(rotateAngle, rotateX, rotateY, rotateZ);
    glScalef(scale, scale, scale);
    
    // Render
    render();
    
    glPopMatrix();
}

void ModelLoader::getBounds(float& minX, float& minY, float& minZ, 
                            float& maxX, float& maxY, float& maxZ) {
    if (meshes.empty()) {
        minX = minY = minZ = maxX = maxY = maxZ = 0.0f;
        return;
    }
    
    // Initialize with first vertex
    minX = maxX = meshes[0].vertices[0].position[0];
    minY = maxY = meshes[0].vertices[0].position[1];
    minZ = maxZ = meshes[0].vertices[0].position[2];
    
    // Find min/max across all vertices
    for (const Mesh& mesh : meshes) {
        for (const Vertex& v : mesh.vertices) {
            minX = std::min(minX, v.position[0]);
            minY = std::min(minY, v.position[1]);
            minZ = std::min(minZ, v.position[2]);
            maxX = std::max(maxX, v.position[0]);
            maxY = std::max(maxY, v.position[1]);
            maxZ = std::max(maxZ, v.position[2]);
        }
    }
}

void ModelLoader::clear() {
    // Delete textures
    for (auto& pair : texturesLoaded) {
        glDeleteTextures(1, &pair.second.id);
    }
    texturesLoaded.clear();
    
    // Clear meshes
    meshes.clear();
}

void ModelLoader::setUseTextures(bool use) {
    // This could be implemented to toggle texture rendering
    // For now, textures are always used if available
}
