#pragma once

// Include GLEW before GLM to avoid OpenGL header conflicts
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

// Helper function for grid indexing
inline int idx(int x, int y, int cols) { 
    return y * cols + x; 
}

// Constants
constexpr float GRAVITY = -9.81f;
constexpr float DEFAULT_DAMPING = 0.994f;
constexpr float DEFAULT_SPRING_STIFFNESS = 1500.0f;
constexpr float DEFAULT_CONSTRAINT_ALPHA = 0.5f;
constexpr int DEFAULT_CONSTRAINT_ITERATIONS = 10;
