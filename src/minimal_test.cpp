#include <iostream>

// Include GLEW before any OpenGL headers
#include <GL/glew.h>

// Include GLFW after GLEW
#include <GLFW/glfw3.h>

int main() {
    std::cout << "Testing minimal OpenGL setup..." << std::endl;
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    std::cout << "GLFW initialized successfully" << std::endl;
    
    // Create a simple window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Minimal OpenGL Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    std::cout << "GLFW window created successfully" << std::endl;
    
    // Make context current
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return -1;
    }
    
    std::cout << "GLEW initialized successfully" << std::endl;
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // Test basic OpenGL functionality
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    std::cout << "Basic OpenGL test successful!" << std::endl;
    
    // Clean up
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}

