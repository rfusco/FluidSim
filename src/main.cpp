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

int windowWidth = 1000;
int windowHeight = 800;
std::vector<Particle> particles;

// Constants
float H = 16; // Kernel smoothing radius
float H2 = H * H; // Squared smoothing radius
float REST_DENSITY = 300.f;  // rest density
float GAS_CONSTANT = 2000.f; // const for equation of state

// Sim parameters
float EPSILON = H / 100000000;
float BOUND_DAMPING = 0.5f; // damping factor for boundary collisions

float simTime = 0.0007;

// Make fps independent from simulation speed
bool useSimFPS = false;
float simFPS = 60; // target FPS for simulation
float simDeltaTime = 1/simFPS; // fixed timestep for simulation
double lastTime = 0.0;
double accumulatedTime = 0.0;

// Simulation FPS measurement
int simStepsThisSecond = 0;
float simFPSDisplay = 0.0f;
double simFPSTimer = 0.0;

// Precomputed kernel constants
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
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Instanced Circles", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Enable VSync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return nullptr;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return window;
}

int main() {
    GLFWwindow* window = InitGL();
    if (!window) return -1;

    Renderer::Init();
    Renderer::UpdateProjection(windowWidth, windowHeight);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize SPH particles
    initSPH(particles, 550, H/2, windowWidth, windowHeight);

    lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        accumulatedTime += deltaTime;

        if(useSimFPS){
            // Simulation update with fixed timestep
            while (accumulatedTime >= simDeltaTime) {
                computeDensityAndPressure(particles, H, H2, POLY6, GAS_CONSTANT, REST_DENSITY);
                computeForces(particles, H, -9.81f, 2.5f, SPIKY_GRADIENT, 200, VISCOSITY_LAPLACIAN);
                integrate(particles, simTime, EPSILON, BOUND_DAMPING, windowWidth, windowHeight);

                accumulatedTime -= simDeltaTime;

                simStepsThisSecond++; // count simulation step
            }

            // Update simulation FPS once per second
            simFPSTimer += deltaTime;
            if (simFPSTimer >= 1.0) {
                simFPSDisplay = (float)simStepsThisSecond / simFPSTimer;
                simStepsThisSecond = 0;
                simFPSTimer = 0.0;
            }
        } else {
            computeDensityAndPressure(particles, H, H2, POLY6, GAS_CONSTANT, REST_DENSITY);
            computeForces(particles, H, -9.81f, 2.5f, SPIKY_GRADIENT, 200, VISCOSITY_LAPLACIAN);
            integrate(particles, simTime, EPSILON, BOUND_DAMPING, windowWidth, windowHeight);
        }
        

        // Rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        std::vector<glm::vec2> positions;
        std::vector<float> radii;
        positions.reserve(particles.size());
        radii.reserve(particles.size());
        for (const auto& p : particles) {
            positions.push_back(p.position);
            radii.push_back(p.rad);
        }
        Renderer::RenderFrame(positions, radii);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Performance window
        ImGui::Begin("Performance");
        ImGui::Text("Program FPS: %f", io.Framerate);
        ImGui::Text("Simulation FPS: %.1f", simFPSDisplay);
        ImGui::Text("Particles: %zu", particles.size());
        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Renderer::Cleanup();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
