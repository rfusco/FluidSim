#pragma once

#include <vector>

#include "particle.hpp"
#include "kernel.hpp"

struct simConfig{
    // Window
    int windowWidth = 1000;
    int windowHeight = 800;

    // Objects
    std::vector<Particle> particles;

    // Constants
    float H = 16; // Kernel smoothing radius
    float H2 = H * H; // Squared smoothing radius
    float REST_DENSITY = 300.f;  // rest density
    float GAS_CONSTANT = 2000.f; // const for equation of state
    float VISCOSITY = 200.0f;
    float G = 9.81f;

    // Controls
    bool simRunning = false;
    bool useSimFPS = false;
    int numParticles = 400;
    int colorMode = 0;
    float radius = H/2;

    // Precomputed kernel constants
    float POLY6 = poly6(H);
    float SPIKY_GRADIENT = spikyGradient(H);
    float VISCOSITY_LAPLACIAN = viscosityLaplacian(H);

    // Sim parameters
    float EPSILON = H / 100000000;
    float BOUND_DAMPING = 0.5f; // damping factor for boundary collisions
    float simTime = 0.0007;
    float minPressure = std::numeric_limits<float>::max();
    float maxPressure = std::numeric_limits<float>::lowest();

    // Make fps independent from simulation speed
    float simFPS = 60; // target FPS for simulation
    float simDeltaTime = 1 / simFPS; // fixed timestep for simulation
    double lastTime = 0.0;
    double accumulatedTime = 0.0;

    // Simulation FPS measurement
    int simStepsThisSecond = 0;
    float simFPSDisplay = 0.0f;
    double simFPSTimer = 0.0;
};