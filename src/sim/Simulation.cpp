#include "Simulation.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#include "renderer/renderer.hpp"
#include "sphSolver.hpp"
#include "kernel.hpp"
#include "simConfig.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

Simulation::Simulation(){
    // Init GLFW, GL, renderer, ImGui
    initGLFWAndWindow();
    Renderer::Init();
    Renderer::UpdateProjection(config.windowWidth, config.windowHeight);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    resetSimulation();
    lastTime = clock_t::now();
}

Simulation::~Simulation(){
    // cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // cleanup renderer
    Renderer::Cleanup();

    // cleanup GLFW
    if(window){
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void Simulation::run(){
    throwIfWindowNull();

    while(!glfwWindowShouldClose(window)){
        auto now = clock_t::now();
        std::chrono::duration<double> elapsed = now - lastTime;
        double dt = elapsed.count();
        lastTime = now;

        update(dt);

        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Simulation::initGLFWAndWindow(){
    if(!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(config.windowWidth, config.windowHeight, "Instanced Circles", NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Disable VSync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height){
        auto sim = static_cast<Simulation*>(glfwGetWindowUserPointer(w));
        if (!sim) return;
        sim->config.windowWidth = width;
        sim->config.windowHeight = height;
        glViewport(0,0,width,height);
        Renderer::UpdateProjection(width, height);
    });
}

void Simulation::update(double dt){
    // accumulate time
    config.accumulatedTime += dt;

    if(config.useSimFPS && config.simRunning){
        // fixed time step update
        while(config.accumulatedTime >= config.simDeltaTime){
            stepSimulation();
            config.accumulatedTime -= config.simDeltaTime;
            config.simStepsThisSecond++;
        }

        config.simFPSTimer += dt;
        if (config.simFPSTimer >= 1.0) {
                config.simFPSDisplay = static_cast<float>(config.simStepsThisSecond) / static_cast<float>(config.simFPSTimer);
                config.simStepsThisSecond = 0;
                config.simFPSTimer = 0.0;
            }
    } else if (config.simRunning){
        stepSimulation();
    }
}

void Simulation::stepSimulation(){
    computeDensityAndPressure(config);
    computeForces(config);
    integrate(config);
}

void Simulation::render(){
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
        radii.push_back(config.radius);
        pressures.push_back(-p.p);

        config.minPressure = std::min(config.minPressure, -p.p);
        config.maxPressure = std::max(config.maxPressure, -p.p);
    }
    // Render to GPU
    Renderer::RenderFrame(positions, radii, pressures, config.minPressure, config.maxPressure, config.colorMode);
    
    // Reset mins and maxs
    config.minPressure = std::numeric_limits<float>::max();
    config.maxPressure = std::numeric_limits<float>::min();

    renderUI();
}

void Simulation::renderUI(){
    // Start ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Performance window
        ImGui::Begin("Performance & Controls");
        ImGui::Text("Program FPS: %.2f", ImGui::GetIO().Framerate);
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
}

void Simulation::resetSimulation(){
    config.particles.clear();
    initSPH(config);
    config.minPressure = std::numeric_limits<float>::max();
    config.maxPressure = std::numeric_limits<float>::lowest();
    config.simFPSDisplay = 0.0f;
    config.simStepsThisSecond = 0;
    config.simFPSTimer = 0.0;
    config.accumulatedTime = 0.0;
}

void Simulation::throwIfWindowNull(){
    if(!window) throw std::runtime_error("GLFW window is null");
}
