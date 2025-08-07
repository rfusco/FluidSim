#include <chrono>

#include "sphSolver.hpp"
#include "benchmark.hpp"
#include "initGL.hpp"
#include "renderer.hpp"


float benchmarkAveragePerformance(int numParticles, float sampleTime, int numSamples){
    int windowWidth = 1000, windowHeight = 800;
    // Init opengl
    GLFWwindow* window = InitGL();
    if(!window) return -1;

    // Init renderer
    Renderer::Init();
    Renderer::UpdateProjection(windowWidth, windowHeight);

    // Init particles
    std::vector<Particle> particles;
    for(int i = 0; i < numSamples; i++){
        initSPH(particles, numParticles, 8, windowWidth, windowHeight);
    }
}