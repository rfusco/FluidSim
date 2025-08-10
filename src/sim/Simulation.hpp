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
        void initRenderer();
        void initImGui();

        // main loop helpers
        void update(double dt);
        void stepSimulation();
        void render();
        void renderUI();

        // sim helpers
        void resetSimulation();

        // config & state
        simConfig config_;
        GLFWwindow* window_ = nullptr;

        // timing
        using clock_t = std::chrono::high_resolution_clock;
        clock_t::time_point lastTime_;

        void throwIfWindowNull();
};