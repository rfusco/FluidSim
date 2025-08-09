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
#include "simConfig.hpp"

int main() {
    simConfig config;
    // Initialize and create window
    GLFWwindow* window = InitGL(config);
    if (!window) return -1;

    // Initialize renderer
    Renderer::Init();
    Renderer::UpdateProjection(config.windowWidth, config.windowHeight);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize SPH particles
    initSPH(config);

    config.lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - config.lastTime;
        config.lastTime = currentTime;
        config.accumulatedTime += deltaTime;

        if(config.useSimFPS && config.simRunning){
            // Simulation update with fixed timestep
            while (config.accumulatedTime >= config.simDeltaTime) {
                computeDensityAndPressure(config);
                computeForces(config);
                integrate(config);

                config.accumulatedTime -= config.simDeltaTime;

                config.simStepsThisSecond++; // count simulation step
            }

            // Update simulation FPS once per second
            config.simFPSTimer += deltaTime;
            if (config.simFPSTimer >= 1.0) {
                config.simFPSDisplay = (float)config.simStepsThisSecond / config.simFPSTimer;
                config.simStepsThisSecond = 0;
                config.simFPSTimer = 0.0;
            }
        } else if(config.simRunning){
            computeDensityAndPressure(config);
            computeForces(config);
            integrate(config);
        }
        

        // Rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Gather particle data for rendering on GPU
        std::vector<glm::vec2> positions;
        std::vector<float> radii;
        std::vector<float> pressures;
        positions.reserve(config.particles.size());
        radii.reserve(config.particles.size());
        pressures.reserve(config.particles.size());

        
        for (const auto& p : config.particles) {
            positions.push_back(p.position);
            radii.push_back(p.rad);
            pressures.push_back(-p.p);

            config.minPressure = std::min(config.minPressure, -p.p);
            config.maxPressure = std::max(config.maxPressure, -p.p);
        }
        Renderer::RenderFrame(positions, radii, pressures, config.minPressure, config.maxPressure, config.colorMode);
        config.minPressure = std::numeric_limits<float>::max();
        config.maxPressure = std::numeric_limits<float>::min();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Performance window
        ImGui::Begin("Performance & Controls");
        ImGui::Text("Program FPS: %.2f", io.Framerate);
        ImGui::Text("Simulation FPS: %.2f", config.simFPSDisplay);
        ImGui::Text("Particles: %zu", config.particles.size());

        // Controls
        // Start/Stop simulation
        if(ImGui::Button(config.simRunning ? "Stop Simulation" : "Start Simulation")) {
            config.simRunning = !config.simRunning;
        }

        // Reset simulation
        if(ImGui::Button("Reset Simulation")) {
            config.particles.clear();
            initSPH(config);
            config.simRunning = false;
        }

        // Change number of particles
        if(ImGui::SliderInt("Number of Particles", &config.numParticles, 1, 1000)) {
            config.particles.clear();
            initSPH(config);
        }

        // Color mode Selection
        if(ImGui::Combo("Color Mode", &config.colorMode, "Jet\0Heat\0BlueRed\0")) {
            // Update colors based on selected mode
        }


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
