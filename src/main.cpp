#include <iostream>
// Include GLEW before GLM
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "window_gl.h"
#include "cloth.h"
#include "renderer.h"

// Convert screen coordinates to world coordinates
glm::vec3 screenToWorld(double mouseX, double mouseY, int screenWidth, int screenHeight, 
                        const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    // Convert screen coordinates to normalized device coordinates (-1 to 1)
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    
    // Create ray in clip space
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
    
    // Transform to eye space
    glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
    rayEye.z = -1.0f;
    rayEye.w = 0.0f;
    
    // Transform to world space
    glm::vec4 rayWorld = glm::inverse(viewMatrix) * rayEye;
    glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));
    
    // Return ray origin (camera position) and direction
    return rayDirection;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    WindowGL window(800, 600, "Cloth Simulation");
    if (!window.isValid()) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    Cloth cloth(20, 20, 0.5f); // 20x20 grid, 0.5 spacing (bigger cloth)
    Renderer renderer;
    
    if (!renderer.initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Camera setup - position camera to see the cloth clearly
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 8.0f);  // Camera behind and above
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);  // Look at center of cloth
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Camera zoom variables
    float cameraDistance = 8.0f;
    const float minDistance = 2.0f;
    const float maxDistance = 20.0f;
    const float zoomSpeed = 0.5f;
    
    // Set initial view matrix
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    renderer.setViewMatrix(view);
    
    // Set projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
    renderer.setProjectionMatrix(projection);
    
    // Set model matrix (position the cloth at center)
    glm::mat4 model = glm::mat4(1.0f);  // No translation, keep at origin
    renderer.setModelMatrix(model);

    double lastTime = glfwGetTime();
    
    std::cout << "Starting cloth simulation..." << std::endl;
    std::cout << "Controls: R=Reset, Space=Toggle mode, W=Wireframe, Up/Down=Iterations" << std::endl;
    std::cout << "Mouse: Left click and drag to grab/move particles, Scroll wheel to zoom" << std::endl;
    
    // Main render loop
    while (!window.shouldClose()) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Clamp delta time for stability
        deltaTime = std::min(deltaTime, 1.0/30.0);
        
        // Handle input
        if (window.isKeyPressed(GLFW_KEY_R)) {
            cloth.reset();
        }
        if (window.isKeyPressed(GLFW_KEY_SPACE)) {
            static int renderMode = 0;
            renderMode = (renderMode + 1) % 2;
            renderer.setRenderMode(renderMode);
        }
        if (window.isKeyPressed(GLFW_KEY_W)) {
            static bool wireframe = false;
            wireframe = !wireframe;
            renderer.setWireframeMode(wireframe);
        }
        if (window.isKeyPressed(GLFW_KEY_UP)) {
            int iterations = cloth.getConstraintIterations() + 1;
            cloth.setConstraintIterations(std::min(iterations, 20));
        }
        if (window.isKeyPressed(GLFW_KEY_DOWN)) {
            int iterations = cloth.getConstraintIterations() - 1;
            cloth.setConstraintIterations(std::max(iterations, 1));
        }
        
        // Handle mouse wheel zoom
        double scrollY = window.getScrollY();
        if (scrollY != 0.0) {
            cameraDistance -= scrollY * zoomSpeed;
            cameraDistance = std::max(minDistance, std::min(maxDistance, cameraDistance));
            
            // Update camera position and view matrix
            cameraPos = glm::vec3(0.0f, 0.0f, cameraDistance);
            view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
            renderer.setViewMatrix(view);
            
            // Reset scroll for next frame
            window.resetScroll();
            
            // Show zoom level
            std::cout << "Camera distance: " << cameraDistance << std::endl;
        }
        
        // Handle mouse interaction with cloth
        static bool wasGrabbing = false;
        
        if (window.isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            // Get mouse position
            double mouseX, mouseY;
            window.getMousePosition(mouseX, mouseY);
            
            // Convert to world ray
            glm::vec3 rayDirection = screenToWorld(mouseX, mouseY, window.getWidth(), window.getHeight(), view, projection);
            
            // Find closest particle
            int particleIndex = cloth.getClosestParticle(cameraPos, rayDirection, 3.0f);
            
            if (particleIndex >= 0) {
                // Calculate world position at cloth plane (z=0)
                float t = -cameraPos.z / rayDirection.z;
                glm::vec3 worldPos = cameraPos + t * rayDirection;
                
                cloth.grabParticle(particleIndex, worldPos);
                wasGrabbing = true;
                std::cout << "Grabbed particle " << particleIndex << std::endl;
            }
        }
        
        if (window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && wasGrabbing) {
            // Update grabbed particle position
            double mouseX, mouseY;
            window.getMousePosition(mouseX, mouseY);
            
            // Convert to world ray
            glm::vec3 rayDirection = screenToWorld(mouseX, mouseY, window.getWidth(), window.getHeight(), view, projection);
            
            // Calculate world position at cloth plane (z=0)
            float t = -cameraPos.z / rayDirection.z;
            glm::vec3 worldPos = cameraPos + t * rayDirection;
            
            cloth.updateGrabbedParticle(worldPos);
        }
        
        if (!window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && wasGrabbing) {
            // Release particle
            cloth.releaseParticle(cloth.getClosestParticle(cameraPos, glm::vec3(0,0,1), 3.0f));
            wasGrabbing = false;
            std::cout << "Released particle" << std::endl;
        }
        
        // Update simulation
        cloth.update(deltaTime);
        
        // Render
        renderer.render(cloth);
        
        // Poll events and swap buffers
        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
