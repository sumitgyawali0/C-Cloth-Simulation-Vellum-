#include "cloth.h"
#include <iostream>

Cloth::Cloth(int cols, int rows, float spacing) 
    : cols(cols), rows(rows), spacing(spacing) {
    initializeGrid();
    createSprings();
    createIndices();
}

Cloth::~Cloth() {
    // Clean up OpenGL buffers if they exist
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo_pos) glDeleteBuffers(1, &vbo_pos);
    if (vbo_norm) glDeleteBuffers(1, &vbo_norm);
    if (ebo) glDeleteBuffers(1, &ebo);
}

void Cloth::initializeGrid() {
    std::cout << "Initializing cloth grid: " << cols << "x" << rows << std::endl;
    particles.clear();
    particles.reserve(cols * rows);
    
            for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                Particle p;
                // Position cloth in front of camera, centered at origin
                p.pos = glm::vec3((x - cols/2) * spacing, (rows/2 - y) * spacing, 0.0f);
                p.prevPos = p.pos;
                // Pin the top corners
                p.fixed = (y == 0 && (x == 0 || x == cols - 1));
                particles.push_back(p);
            }
        }
    std::cout << "Created " << particles.size() << " particles" << std::endl;
}

void Cloth::createSprings() {
    springs.clear();
    springs.reserve(cols * rows * 2); // Approximate capacity for structural springs
    
    auto addSpring = [&](int a, int b) {
        Spring s(a, b, spacing, DEFAULT_SPRING_STIFFNESS);
        springs.push_back(s);
    };
    
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int a = idx(x, y, cols);
            if (x + 1 < cols) addSpring(a, idx(x + 1, y, cols));
            if (y + 1 < rows) addSpring(a, idx(x, y + 1, cols));
        }
    }
    std::cout << "Created " << springs.size() << " springs" << std::endl;
}

void Cloth::createIndices() {
    indices.clear();
    indices.reserve((cols - 1) * (rows - 1) * 6); // 2 triangles per grid cell
    
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            unsigned a = idx(x, y, cols), b = idx(x + 1, y, cols);
            unsigned c = idx(x, y + 1, cols), d = idx(x + 1, y + 1, cols);
            // Two triangles: a-b-c, b-d-c
            indices.insert(indices.end(), {a, b, c, b, d, c});
        }
    }
}

void Cloth::update(float deltaTime) {
    // Use fixed time step for stability
    const float fixedDt = 1.0f / 120.0f;
    int substeps = std::max(1, static_cast<int>(deltaTime / fixedDt));
    
    for (int step = 0; step < substeps; ++step) {
        verletIntegrate(fixedDt);
        satisfySpringConstraints();
    }
}

void Cloth::verletIntegrate(float deltaTime) {
    glm::vec3 gravity(0.0f, GRAVITY, 0.0f);
    
    for (auto& p : particles) {
        if (p.fixed) continue;
        
        glm::vec3 acc = gravity;
        glm::vec3 cur = p.pos;
        glm::vec3 vel = p.pos - p.prevPos;  // implicit velocity
        
        p.pos = p.pos + damping * vel + acc * deltaTime * deltaTime;
        p.prevPos = cur;
    }
}

void Cloth::satisfySpringConstraints() {
    for (int it = 0; it < constraintIterations; ++it) {
        for (auto& s : springs) {
            Particle& A = particles[s.i];
            Particle& B = particles[s.j];
            
            glm::vec3 delta = B.pos - A.pos;
            float dist = glm::length(delta);
            if (dist < 1e-6f) continue;
            
            float diff = (dist - s.restLength) / dist;
            float alpha = constraintAlpha;
            
            if (!A.fixed && !B.fixed) {
                A.pos += alpha * delta * diff * 0.5f;
                B.pos -= alpha * delta * diff * 0.5f;
            } else if (!A.fixed) {
                A.pos += alpha * delta * diff;
            } else if (!B.fixed) {
                B.pos -= alpha * delta * diff;
            }
        }
    }
}

void Cloth::reset() {
    initializeGrid();
}

void Cloth::togglePin(int x, int y) {
    if (x >= 0 && x < cols && y >= 0 && y < rows) {
        int index = idx(x, y, cols);
        particles[index].fixed = !particles[index].fixed;
    }
}

void Cloth::setSpringStiffness(float stiffness) {
    for (auto& spring : springs) {
        spring.k = stiffness;
    }
}

// Mouse interaction methods
int Cloth::getClosestParticle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, float maxDistance) const {
    int closestParticle = -1;
    float closestDistance = maxDistance;
    
    for (size_t i = 0; i < particles.size(); ++i) {
        const auto& particle = particles[i];
        
        // Calculate distance from ray to particle
        glm::vec3 toParticle = particle.pos - rayOrigin;
        float projection = glm::dot(toParticle, rayDirection);
        
        // Closest point on ray to particle
        glm::vec3 closestPoint = rayOrigin + projection * rayDirection;
        float distance = glm::length(particle.pos - closestPoint);
        
        if (distance < closestDistance) {
            closestDistance = distance;
            closestParticle = static_cast<int>(i);
        }
    }
    
    return closestParticle;
}

void Cloth::grabParticle(int particleIndex, const glm::vec3& worldPos) {
    if (particleIndex >= 0 && particleIndex < static_cast<int>(particles.size())) {
        grabbedParticleIndex = particleIndex;
        grabOffset = worldPos - particles[particleIndex].pos;
        // Temporarily fix the particle while grabbed
        particles[particleIndex].fixed = true;
    }
}

void Cloth::releaseParticle(int particleIndex) {
    if (particleIndex >= 0 && particleIndex < static_cast<int>(particles.size())) {
        // Restore original fixed state (only top corners should be fixed)
        int x = particleIndex % cols;
        int y = particleIndex / cols;
        particles[particleIndex].fixed = (y == 0 && (x == 0 || x == cols - 1));
        grabbedParticleIndex = -1;
    }
}

bool Cloth::isParticleGrabbed(int particleIndex) const {
    return particleIndex == grabbedParticleIndex;
}

void Cloth::updateGrabbedParticle(const glm::vec3& worldPos) {
    if (grabbedParticleIndex >= 0 && grabbedParticleIndex < static_cast<int>(particles.size())) {
        particles[grabbedParticleIndex].pos = worldPos - grabOffset;
        particles[grabbedParticleIndex].prevPos = particles[grabbedParticleIndex].pos;
    }
}
