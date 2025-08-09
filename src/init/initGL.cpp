#include "init/initGL.hpp"
#include "renderer/renderer.hpp"
#include "sphSolver.hpp"
#include "simConfig.hpp"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    auto *config = static_cast<simConfig*>(glfwGetWindowUserPointer(window));
    if(config){
        config->windowWidth = width;
        config->windowHeight = height;
    }
    glViewport(0, 0, width, height);
    Renderer::UpdateProjection(width, height);
}

// Initialize OpenGL and create a window
GLFWwindow* InitGL(simConfig &config){
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(config.windowWidth, config.windowHeight, "Instanced Circles", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Disable VSync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return nullptr;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSetWindowUserPointer(window, &config);

    return window;
}