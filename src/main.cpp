#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <chrono>
#include <limits>
#include <iostream>
#include <algorithm>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "renderer/renderer.hpp"
#include "sphSolver.hpp"
#include "kernel.hpp"
#include "init/initGL.hpp"
#include "benchmark.hpp"

extern int windowWidth;
extern int windowHeight;

// Controls
bool simRunning = false;
bool useSimFPS = false;
int numParticles = 400;
int colorMode = 0;

extern std::vector<Particle> particles;

// Constants
float H = 16; // Kernel smoothing radius
float H2 = H * H; // Squared smoothing radius
float REST_DENSITY = 300.f;  // rest density
float GAS_CONSTANT = 2000.f; // const for equation of state

// Precomputed kernel constants
const static float POLY6 = poly6(H);
const static float SPIKY_GRADIENT = spikyGradient(H);
const static float VISCOSITY_LAPLACIAN = viscosityLaplacian(H);

// Sim parameters
float EPSILON = H / 100000000;
float BOUND_DAMPING = 0.5f; // damping factor for boundary collisions

float simTime = 0.0007;

// Make fps independent from simulation speed
float simFPS = 60; // target FPS for simulation
float simDeltaTime = 1 / simFPS; // fixed timestep for simulation
double lastTime = 0.0;
double accumulatedTime = 0.0;

// Simulation FPS measurement
int simStepsThisSecond = 0;
float simFPSDisplay = 0.0f;
double simFPSTimer = 0.0;

float minPressure = std::numeric_limits<float>::max();
float maxPressure = std::numeric_limits<float>::lowest();

// Runs exactly one frame: physics step (respecting simRunning/useSimFPS),
// particle-data gather, render, ImGui UI, and buffer swap. This is the single
// per-frame code path shared by interactive use and --benchmark mode, so
// benchmark numbers can never silently diverge from what the user sees.
void StepAndRenderFrame(GLFWwindow* window, ImGuiIO& io) {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    accumulatedTime += deltaTime;

    if (useSimFPS && simRunning) {
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
    } else if (simRunning) {
        computeDensityAndPressure(particles, H, H2, POLY6, GAS_CONSTANT, REST_DENSITY);
        computeForces(particles, H, -9.81f, 2.5f, SPIKY_GRADIENT, 200, VISCOSITY_LAPLACIAN);
        integrate(particles, simTime, EPSILON, BOUND_DAMPING, windowWidth, windowHeight);
    }


    // Rendering
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Gather particle data for rendering on GPU
    std::vector<glm::vec2> positions;
    std::vector<float> radii;
    std::vector<float> pressures;
    positions.reserve(particles.size());
    radii.reserve(particles.size());
    pressures.reserve(particles.size());


    for (const auto& p : particles) {
        positions.push_back(p.position);
        radii.push_back(p.rad);
        pressures.push_back(-p.p);

        minPressure = std::min(minPressure, -p.p);
        maxPressure = std::max(maxPressure, -p.p);
    }
    Renderer::RenderFrame(positions, radii, pressures, minPressure, maxPressure, colorMode);
    minPressure = std::numeric_limits<float>::max();
    maxPressure = std::numeric_limits<float>::lowest();

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Performance window
    ImGui::Begin("Performance & Controls");
    ImGui::Text("Program FPS: %.2f", io.Framerate);
    ImGui::Text("Simulation FPS: %.2f", simFPSDisplay);
    ImGui::Text("Particles: %zu", particles.size());

    // Controls
    // Start/Stop simulation
    if (ImGui::Button(simRunning ? "Stop Simulation" : "Start Simulation")) {
        simRunning = !simRunning;
    }

    // Reset simulation
    if (ImGui::Button("Reset Simulation")) {
        particles.clear();
        initSPH(particles, numParticles, H / 2, windowWidth, windowHeight);
        simRunning = false;
    }

    // Change number of particles
    if (ImGui::SliderInt("Number of Particles", &numParticles, 1, 1000)) {
        particles.clear();
        initSPH(particles, numParticles, H / 2, windowWidth, windowHeight);
    }

    // Color mode Selection
    if (ImGui::Combo("Color Mode", &colorMode, "Jet\0Heat\0BlueRed\0")) {
        // Update colors based on selected mode
    }


    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
}

// Drives fluidSim.exe --benchmark [stageLabel]: sweeps particle counts, running
// warmup + timed frames through the exact same StepAndRenderFrame used
// interactively, then writes benchmarks/results/<stageLabel>.csv.
void RunBenchmarkMode(GLFWwindow* window, ImGuiIO& io, const std::string& stageLabel) {
    // One physics step per rendered frame, uncapped (vsync is already off by
    // default in InitGL), so numbers reflect real CPU+GPU+driver cost.
    simRunning = true;
    useSimFPS = false;

    std::vector<FrameStats> allStats;
    allStats.reserve(kBenchmarkParticleCounts.size());

    for (int count : kBenchmarkParticleCounts) {
        if (glfwWindowShouldClose(window)) {
            std::cerr << "Window closed during benchmark; aborting sweep early.\n";
            break;
        }

        numParticles = count;
        particles.clear();
        initSPH(particles, numParticles, H / 2, windowWidth, windowHeight);

        // Warmup (untimed): let the sim reach representative motion and
        // absorb first-use GL/driver costs before timing starts.
        for (int i = 0; i < kBenchmarkWarmupFrames; ++i) {
            StepAndRenderFrame(window, io);
        }

        double totalMs = 0.0;
        double minMs = std::numeric_limits<double>::max();
        double maxMs = std::numeric_limits<double>::lowest();
        for (int i = 0; i < kBenchmarkSampleFrames; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            StepAndRenderFrame(window, io);
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();
            totalMs += ms;
            minMs = std::min(minMs, ms);
            maxMs = std::max(maxMs, ms);
        }

        FrameStats stats;
        stats.particleCount = count;
        stats.avgFrameMs = totalMs / kBenchmarkSampleFrames;
        stats.minFrameMs = minMs;
        stats.maxFrameMs = maxMs;
        stats.avgFps = stats.avgFrameMs > 0.0 ? 1000.0 / stats.avgFrameMs : 0.0;

        std::cout << "particles=" << count
                   << " avg=" << stats.avgFrameMs << "ms"
                   << " min=" << stats.minFrameMs << "ms"
                   << " max=" << stats.maxFrameMs << "ms"
                   << " (" << stats.avgFps << " fps)\n";

        allStats.push_back(stats);
    }

    writeBenchmarkCSV(stageLabel, allStats);
}

int main(int argc, char** argv) {
    bool runBenchmark = false;
    std::string benchmarkStageLabel = "benchmark";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--benchmark") {
            runBenchmark = true;
            if (i + 1 < argc) {
                benchmarkStageLabel = argv[++i];
            }
        }
    }

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
    initSPH(particles, numParticles, H/2, windowWidth, windowHeight);

    lastTime = glfwGetTime();

    if (runBenchmark) {
        RunBenchmarkMode(window, io, benchmarkStageLabel);
    } else {
        while (!glfwWindowShouldClose(window)) {
            StepAndRenderFrame(window, io);
        }
    }

    Renderer::Cleanup();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
