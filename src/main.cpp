#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
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

// Sim parameters
float EPSILON = H / 10000000;
float BOUND_DAMPING = 0.5f; // damping factor for boundary collisions

// Make fps independent from simulation speed
float simFPS = 60.0f; // target FPS for simulation
float simDeltaTime = 1.0f / simFPS; // time step for simulation
double lastTime = 0.0;
double accumulatedTime = 0.0;

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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Optional

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //Initialize SPH particles
    initSPH(particles, 350, 0.05f);

    while (!glfwWindowShouldClose(window)) {
        // Update time
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        accumulatedTime += deltaTime;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Draw all particles
        std::vector<glm::vec2> positions;
        std::vector<float> radii;
        for (const auto& p : particles) {
            positions.push_back(p.position);
            radii.push_back(p.rad);
        }
        Renderer::RenderFrame(positions, radii);

        // Update SPH simulation
        while (accumulatedTime >= simDeltaTime) {
            //computeDensityAndPressure(particles, H, H2, POLY6, GAS_CONSTANT, REST_DENSity);
            computeForces(particles, H, -9.81f, 2.5f, SPIKY_GRADIENT, 0.1f, VISCOSITY_LAPLACIAN);
            integrate(particles, simDeltaTime, EPSILON, BOUND_DAMPING, windowWidth, windowHeight);
            
            // FPS measurement
            accumulatedTime -= simDeltaTime;
        }

        // Show FPS in ImGui
        ImGui::Begin("Performance");
        ImGui::Text("Program FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Renderer::Cleanup();
    glfwTerminate();
    return 0;
}
