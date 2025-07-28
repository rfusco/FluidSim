#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer/renderer.hpp"
#include "sphSolver.hpp"
#include "kernel.hpp"
#include <vector>
#include <iostream>

int windowWidth = 800;
int windowHeight = 600;
std::vector<Particle> particles;

// Constants
float H = 16.0f; // Kernel smoothing radius
float H2 = H * H; // Squared smoothing radius
float REST_DENSity = 300.f;  // rest density
float GAS_CONSTANT = 2000.f; // const for equation of state
float timeStep = 0.0007f; // time step for simulation

// get kernel function values ahead of time to avoid recomputing
const static float POLY6 = poly6(H);
const static float SPIKY_GRADIENT = spikyGradient(H);
const static float VISCOSITY_LAPLACIAN = viscosityLaplacian(H);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    Renderer::UpdateProjection(width, height);
}

// Initialize OpenGL and create a window
GLFWwindow* InitGL(void){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Instanced Circles", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return NULL;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return window;
}

int main() {
    // Initialize GLFW and create a window
    GLFWwindow* window = InitGL();
    if (!window) {
        return -1;
    }

    //Initialize renderer
    Renderer::Init();
    Renderer::UpdateProjection(windowWidth, windowHeight);

    //Initialize SPH particles
    initSPH(particles, 20000, 0.005f);


    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw all particles
        std::vector<glm::vec2> positions;
        std::vector<float> radii;
        for (const auto& p : particles) {
            positions.push_back(p.position);
            radii.push_back(p.rad);
        }
        Renderer::RenderFrame(positions, radii);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Renderer::Cleanup();
    glfwTerminate();
    return 0;
}
