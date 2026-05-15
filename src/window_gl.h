#pragma once

// Include GLEW before GLFW to avoid OpenGL header conflicts
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

class WindowGL {
public:
    WindowGL(int width, int height, const std::string& title);
    ~WindowGL();
    
    bool isValid() const { return window != nullptr; }
    bool shouldClose() const { return glfwWindowShouldClose(window); }
    void swapBuffers() { glfwSwapBuffers(window); }
    
    // Input handling
    bool isKeyPressed(int key) const;
    void getMousePosition(double& x, double& y) const;
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonJustPressed(int button) const;
    
    // Mouse wheel
    double getScrollY() const;
    void resetScroll();
    
    // Window properties
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    GLFWwindow* getGLFWWindow() const { return window; }

private:
    GLFWwindow* window;
    int width, height;
    
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void errorCallback(int error, const char* description);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // Static scroll tracking (simplified approach)
    static double scrollY;
    
    // Mouse button state tracking
    static bool prevMouseButtons[3];  // Left, Right, Middle
};
