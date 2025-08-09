#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <simConfig.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow* InitGL(simConfig &config);