#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "renderer/renderer.hpp"
#include "sphSolver.hpp"
#include "kernel.hpp"
#include "init/initGL.hpp"

extern int windowWidth;
extern int windowHeight;

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
float simDeltaTime = 1 / simFPS; // fixed timestep for simulation
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

int main() {
    // Initialize and create window
    GLFWwindow* window = InitGL();
    if (!window) return -1;

    // Initialize renderer
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
    initSPH(particles, 650, H/2, windowWidth, windowHeight);

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
        std::vector<glm::vec3> colors;
        positions.reserve(particles.size());
        radii.reserve(particles.size());
        for (const auto& p : particles) {
            positions.push_back(p.position);
            radii.push_back(p.rad);

            // Color based on force
            //colors.push_back(glm::mix(glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::dot(p.force, p.force) / 100000000));
            
            // Color based on pressure
            // Determined via max pressure and arbitrary scaling
            colors.push_back(glm::mix(glm::vec3(0, 0, 1), glm::vec3(1, 0, 0),  (599975 + p.p) / 8.5));
        }
        Renderer::RenderFrame(positions, radii, colors);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Performance window
        ImGui::Begin("Performance & Controls");
        ImGui::Text("Program FPS: %f", io.Framerate);
        ImGui::Text("Simulation FPS: %.1f", simFPSDisplay);
        ImGui::Text("Particles: %zu", particles.size());

        //if(ImGui::Button)

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
