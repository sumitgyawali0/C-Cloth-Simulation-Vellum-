#include "window_gl.h"
#include <iostream>

// Initialize static members
double WindowGL::scrollY = 0.0;
bool WindowGL::prevMouseButtons[3] = {false, false, false};

WindowGL::WindowGL(int width, int height, const std::string& title) 
    : width(width), height(height) {
    
    // Set error callback
    glfwSetErrorCallback(errorCallback);
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    
    // Make context current
    glfwMakeContextCurrent(window);
    
    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return;
    }
    
    // Configure OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    
    // Set viewport
    glViewport(0, 0, width, height);
}

WindowGL::~WindowGL() {
    if (window) {
        glfwDestroyWindow(window);
    }
}

bool WindowGL::isKeyPressed(int key) const {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void WindowGL::getMousePosition(double& x, double& y) const {
    glfwGetCursorPos(window, &x, &y);
}

bool WindowGL::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

bool WindowGL::isMouseButtonJustPressed(int button) const {
    bool currentState = glfwGetMouseButton(window, button) == GLFW_PRESS;
    bool justPressed = currentState && !prevMouseButtons[button];
    prevMouseButtons[button] = currentState;
    return justPressed;
}

void WindowGL::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void WindowGL::errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void WindowGL::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Update the static scroll value
    scrollY += yoffset;
}

double WindowGL::getScrollY() const {
    return scrollY;
}

void WindowGL::resetScroll() {
    scrollY = 0.0;
}
