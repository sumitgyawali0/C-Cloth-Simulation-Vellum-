#include "renderer.h"
#include <iostream>

Renderer::Renderer() : wireframeMode(false), renderMode(0) {
    lineVAO = lineVBO = 0;
    triangleVAO = triangleVBO = triangleEBO = triangleNormalVBO = 0;
    
    // Initialize matrices
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
    modelMatrix = glm::mat4(1.0f);
}

Renderer::~Renderer() {
    cleanup();
}

bool Renderer::initialize() {
    std::cout << "Initializing renderer..." << std::endl;
    
    // Create built-in shaders
    const char* vertexShaderSource = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
uniform mat4 model, view, projection;
out vec3 vNormal;
out vec3 vWorldPos;
void main(){
    vec4 wp = model * vec4(aPos,1.0);
    vWorldPos = wp.xyz;
    vNormal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * wp;
}
)";

    const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;
out vec4 FragColor;
uniform vec3 lightPos;
uniform vec3 baseColor;
void main(){
    vec3 N = normalize(vNormal);
    vec3 L = normalize(lightPos - vWorldPos);
    float diff = max(dot(N,L), 0.0);
    vec3 color = 0.15*baseColor + diff*baseColor;
    FragColor = vec4(color, 1.0);
}
)";

    const char* lineVertexShaderSource = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 model, view, projection;
void main(){
    gl_Position = projection * view * model * vec4(aPos,1.0);
}
)";

    const char* lineFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main(){
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);  // Bright green lines
}
)";

    if (!triangleShader.loadFromStrings(vertexShaderSource, fragmentShaderSource)) {
        std::cerr << "Failed to load triangle shader" << std::endl;
        return false;
    }
    
    if (!lineShader.loadFromStrings(lineVertexShaderSource, lineFragmentShaderSource)) {
        std::cerr << "Failed to load line shader" << std::endl;
        return false;
    }
    
    std::cout << "Shaders loaded successfully" << std::endl;
    
    setupBuffers();
    std::cout << "Buffers set up successfully" << std::endl;
    return true;
}

void Renderer::setupBuffers() {
    // Line rendering buffers
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Triangle rendering buffers
    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);
    glGenBuffers(1, &triangleEBO);
    glGenBuffers(1, &triangleNormalVBO);
    
    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, triangleNormalVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleEBO);
}

void Renderer::render(const Cloth& cloth) {
    // Clear with a dark blue background
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    updateBuffers(cloth);
    
    if (renderMode == 0) {
        renderLines(cloth);
    } else {
        renderTriangles(cloth);
    }
}

void Renderer::updateBuffers(const Cloth& cloth) {
    const auto& particles = cloth.getParticles();
    const auto& indices = cloth.getIndices();
    
    // Update positions
    positions.clear();
    positions.reserve(particles.size());
    for (const auto& p : particles) {
        positions.push_back(p.pos);
    }
    
    // Update normals for triangle rendering
    if (renderMode == 1) {
        normals.resize(particles.size(), glm::vec3(0.0f));
        computeNormals(cloth, normals);
    }
    
    // Update line buffer
    if (renderMode == 0) {
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);
    }
    
    // Update triangle buffers
    if (renderMode == 1) {
        glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ARRAY_BUFFER, triangleNormalVBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);
    }
}

void Renderer::computeNormals(const Cloth& cloth, std::vector<glm::vec3>& normalVectors) {
    const auto& particles = cloth.getParticles();
    const auto& indices = cloth.getIndices();
    
    // Reset normals
    std::fill(normalVectors.begin(), normalVectors.end(), glm::vec3(0.0f));
    
    // Compute normals for each triangle
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned a = indices[i], b = indices[i + 1], c = indices[i + 2];
        
        glm::vec3 edge1 = particles[b].pos - particles[a].pos;
        glm::vec3 edge2 = particles[c].pos - particles[a].pos;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        
        normalVectors[a] += normal;
        normalVectors[b] += normal;
        normalVectors[c] += normal;
    }
    
    // Normalize accumulated normals
    for (auto& n : normalVectors) {
        if (glm::length(n) > 0.0f) {
            n = glm::normalize(n);
        }
    }
}

void Renderer::renderLines(const Cloth& cloth) {
    lineShader.use();
    lineShader.setMat4("model", modelMatrix);
    lineShader.setMat4("view", viewMatrix);
    lineShader.setMat4("projection", projectionMatrix);
    
    // Make lines thicker and more visible
    glLineWidth(3.0f);
    
    glBindVertexArray(lineVAO);
    
    // Prepare all line data at once
    const auto& springs = cloth.getSprings();
    const auto& particles = cloth.getParticles();
    
    std::vector<glm::vec3> lineData;
    lineData.reserve(springs.size() * 2);
    
    for (const auto& spring : springs) {
        lineData.push_back(particles[spring.i].pos);
        lineData.push_back(particles[spring.j].pos);
    }
    
    // Update buffer with all line data
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, lineData.size() * sizeof(glm::vec3), lineData.data(), GL_DYNAMIC_DRAW);
    
    // Draw all lines at once
    glDrawArrays(GL_LINES, 0, lineData.size());
}

void Renderer::renderTriangles(const Cloth& cloth) {
    triangleShader.use();
    triangleShader.setMat4("model", modelMatrix);
    triangleShader.setMat4("view", viewMatrix);
    triangleShader.setMat4("projection", projectionMatrix);
    triangleShader.setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
    triangleShader.setVec3("baseColor", glm::vec3(0.8f, 0.8f, 0.8f));
    
    glBindVertexArray(triangleVAO);
    
    if (wireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    glDrawElements(GL_TRIANGLES, cloth.getIndices().size(), GL_UNSIGNED_INT, 0);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::cleanup() {
    if (lineVAO) glDeleteVertexArrays(1, &lineVAO);
    if (lineVBO) glDeleteBuffers(1, &lineVBO);
    if (triangleVAO) glDeleteVertexArrays(1, &triangleVAO);
    if (triangleVBO) glDeleteBuffers(1, &triangleVBO);
    if (triangleEBO) glDeleteBuffers(1, &triangleEBO);
    if (triangleNormalVBO) glDeleteBuffers(1, &triangleNormalVBO);
}

void Renderer::setViewMatrix(const glm::mat4& view) {
    viewMatrix = view;
}

void Renderer::setProjectionMatrix(const glm::mat4& projection) {
    projectionMatrix = projection;
}

void Renderer::setModelMatrix(const glm::mat4& model) {
    modelMatrix = model;
}

void Renderer::setWireframeMode(bool wireframe) {
    wireframeMode = wireframe;
}

void Renderer::setRenderMode(int mode) {
    renderMode = mode;
}
