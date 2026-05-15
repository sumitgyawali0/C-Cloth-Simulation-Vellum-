#pragma once

#include "cloth.h"
#include "shader.h"
#include <GL/glew.h>

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool initialize();
    void render(const Cloth& cloth);
    void cleanup();
    
    // Camera controls
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& projection);
    void setModelMatrix(const glm::mat4& model);
    
    // Rendering modes
    void setWireframeMode(bool wireframe);
    void setRenderMode(int mode); // 0=lines, 1=triangles

private:
    void setupBuffers();
    void updateBuffers(const Cloth& cloth);
    void computeNormals(const Cloth& cloth, std::vector<glm::vec3>& normals);
    void renderLines(const Cloth& cloth);
    void renderTriangles(const Cloth& cloth);
    
    Shader lineShader, triangleShader;
    
    // OpenGL buffers
    GLuint lineVAO, lineVBO;
    GLuint triangleVAO, triangleVBO, triangleEBO, triangleNormalVBO;
    
    // Matrices
    glm::mat4 viewMatrix, projectionMatrix, modelMatrix;
    
    // Rendering state
    bool wireframeMode;
    int renderMode;
    
    // Buffer data
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
};
