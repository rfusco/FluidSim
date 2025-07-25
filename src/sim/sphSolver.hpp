#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Particle{
    Particle(float x_, float y_, float rad_ = 0.05f) : position(x_, y_), velocity(0.0f), force(0.0f), rho(0), p(0), rad(rad_){}
    glm::vec2 position, velocity, force;
    float rho, p, rad, m = 2.5f; // density, pressure, radius, mass
};

void initSPH(std::vector<Particle>& particles, int numParticles, float radius);
void integrate(void);
/*
For every particle, loop through all other particles and sum their density contributions.
*/
void computeDensityAndPressure(std::vector<Particle>& particles, float H, float H2, float POLY6, float GAS_CONSTANT, float REST_DENSITY);
void computeForces(std::vector<Particle>& particles, float H, float G, float MASS, float SPIKY_GRADIENT, float VISCOSITY, float VISCOSITY_LAPLACIAN);
void update(void);
