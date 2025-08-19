#pragma once
#include <glm/glm.hpp>

struct Particle{
    glm::vec2 position, velocity, force;
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // Default color white
    float rho, p, m = 2.5f; // density, pressure, radius, mass

    Particle(float x_, float y_) : 
    position(x_, y_), velocity(0.0f, 0.0f), force(0.0f, 0.0f), rho(1), p(0){}
};