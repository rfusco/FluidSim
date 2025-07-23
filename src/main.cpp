#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer/renderer.hpp"
#include <vector>
#include <iostream>

int windowWidth = 800;
int windowHeight = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    Renderer::UpdateProjection(width, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Instanced Circles", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Renderer::Init();
    Renderer::UpdateProjection(windowWidth, windowHeight);

    std::vector<glm::vec2> positions;
    std::vector<float> radii;

    for (int i = -10; i <= 10; ++i) {
        for (int j = -5; j <= 5; ++j) {
            positions.emplace_back(i * 0.1f, j * 0.1f);
            radii.push_back(0.01f);
        }
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        Renderer::RenderFrame(positions, radii);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Renderer::Cleanup();
    glfwTerminate();
    return 0;
}
