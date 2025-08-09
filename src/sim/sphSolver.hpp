#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "simConfig.hpp"

// struct Particle{
//     Particle(float x_, float y_, float rad_ = 0.05f) : position(x_, y_), velocity(0.0f, 0.0f), force(0.0f, 0.0f), rho(1), p(0), rad(rad_){}
//     glm::vec2 position, velocity, force;
//     glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // Default color white
//     float rho, p, rad, m = 2.5f; // density, pressure, radius, mass
// };

/**
 * @brief Initializes a new smoothed-particle hydrodynamic simulation in a centered grid.
 * 
 * Creates particles positioned in a square grid with some jitter to create a scattered impact.
 * Particles are placed within bounds of the window dimensions.
 * 
 * @param particles Vector of Particle objects to initialize.
 * @param numParticles Number of particles to create.
 * @param radius Particle radius.
 * @param windowWidth Width of simulation window.
 * @param windowHeight Height of simulation window.
 * 
 * @return void
 */
void initSPH(simConfig &config);

/**
 * @brief Computes density and pressure for each particle.
 * 
 * Calculates the density and pressure for each particle due to all other particles.
 * 
 * @param particles Vector of particles to process.
 * @param H                Kernel smoothing radius.
 * @param H2               Precomputed squared kernel radius.
 * @param POLY6            Precomputed poly6 kernel coefficient.
 * @param GAS_CONSTANT     Gas constant for pressure computation.
 * @param REST_DENSITY     Rest fluid density.
 */
void computeDensityAndPressure(simConfig &config);

/**
 * @brief Computes all forces on each particle.
 *
 * Calculates the pressure force, viscosity force, and gravitational force
 * for each particle based on SPH equations. This does **not** integrate
 * positions — see `integrate()` for time stepping.
 *
 *
 * @param[in,out] particles        Vector of particles.
 * @param H                        Kernel smoothing radius.
 * @param G                        Gravity acceleration.
 * @param MASS                     Mass of each particle.
 * @param SPIKY_GRADIENT           Precomputed spiky gradient coefficient.
 * @param VISCOSITY                Viscosity coefficient.
 * @param VISCOSITY_LAPLACIAN      Precomputed viscosity Laplacian coefficient.
 */
void computeForces(simConfig &config);

/**
 * @brief Integrates particle motion using Euler Method.
 * 
 * Applies forces to update velocities and postions. Also handles boundary conditions.
 * 
 * @param particles Vector of Particle objects to integrate.
 * @param timeStep Simulation delta time.
 * @param EPSILON Small epsilon to prevent sinking into walls.
 * @param BOUND_DAMPING Velocity damping factor for wall collisions.
 * @param windowWidth Width of simulation window.
 * @param windowHeight Height of simulation window.
 * 
 * @return void
 */
void integrate(simConfig &config);

