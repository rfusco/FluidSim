#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

//My things
#include "renderer/renderer.h"

int main() {
    // Init GLFW
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "FluidSim", NULL, NULL);
    //Ensire window creation was successful
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Init GLAD loader
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Resize window when window is resized
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}