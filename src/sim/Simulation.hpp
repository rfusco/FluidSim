#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "simConfig.hpp"
#include <chrono>

class Simulation{
    public:
        Simulation();
        ~Simulation();

        // runs main loop until window is closed
        void run();

    private:
        // Init helpers
        void initGLFWAndWindow();

        // main loop helpers
        void update(double dt);
        void stepSimulation();
        void render();
        void renderUI();

        // sim helpers
        void resetSimulation();

        // config & state
        simConfig config;
        GLFWwindow* window = nullptr;

        // timing
        using clock_t = std::chrono::high_resolution_clock;
        clock_t::time_point lastTime;

        void throwIfWindowNull();
};