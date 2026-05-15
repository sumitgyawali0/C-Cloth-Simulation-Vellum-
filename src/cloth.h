#pragma once

#include <vector>
#include "math_types.hpp"
#include <GL/glew.h>

struct Particle {
    glm::vec3 pos;
    glm::vec3 prevPos;
    bool fixed = false;
    
    Particle() : pos(0.0f), prevPos(0.0f), fixed(false) {}
    Particle(const glm::vec3& position) : pos(position), prevPos(position), fixed(false) {}
};

struct Spring {
    int i, j;             // indices into particle array
    float restLength;
    float k;              // stiffness
    
    Spring(int a, int b, float length, float stiffness) 
        : i(a), j(b), restLength(length), k(stiffness) {}
};

class Cloth {
public:
    Cloth(int cols, int rows, float spacing);
    ~Cloth();
    
    void update(float deltaTime);
    void reset();
    
    // Getters for rendering
    const std::vector<Particle>& getParticles() const { return particles; }
    const std::vector<Spring>& getSprings() const { return springs; }
    const std::vector<unsigned>& getIndices() const { return indices; }
    int getCols() const { return cols; }
    int getRows() const { return rows; }
    
    // Interactive controls
    void togglePin(int x, int y);
    void setConstraintIterations(int iterations) { constraintIterations = iterations; }
    int getConstraintIterations() const { return constraintIterations; }
    void setDamping(float damp) { damping = damp; }
    void setSpringStiffness(float stiffness);
    
    // Mouse interaction
    int getClosestParticle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, float maxDistance = 2.0f) const;
    void grabParticle(int particleIndex, const glm::vec3& worldPos);
    void releaseParticle(int particleIndex);
    void updateGrabbedParticle(const glm::vec3& worldPos);
    bool isParticleGrabbed(int particleIndex) const;

private:
    void initializeGrid();
    void createSprings();
    void createIndices();
    void verletIntegrate(float deltaTime);
    void satisfySpringConstraints();
    
    int cols, rows;
    float spacing;
    std::vector<Particle> particles;
    std::vector<Spring> springs;
    std::vector<unsigned> indices;
    
    // Simulation parameters
    float damping = DEFAULT_DAMPING;
    int constraintIterations = DEFAULT_CONSTRAINT_ITERATIONS;
    float constraintAlpha = DEFAULT_CONSTRAINT_ALPHA;
    
    // Render buffers (for modern GL path)
    GLuint vao = 0, vbo_pos = 0, vbo_norm = 0, ebo = 0;
    
    // Mouse interaction state
    int grabbedParticleIndex = -1;
    glm::vec3 grabOffset;
};
