#include <sphSolver.hpp>

void initSPH(simConfig *config) {
    config->particles.clear();
    config->numParticles = config->numParticles;

    const float spacing = 2 * config->radius; // Default radius for particles

    // Compute grid size (make it as square as possible)
    int numX = static_cast<int>(std::ceil(std::sqrt(config->numParticles)));
    int numY = static_cast<int>(std::ceil(config->numParticles / static_cast<float>(numX)));

    // Compute total width and height of the particle grid
    float gridWidth = (numX - 1) * spacing;
    float gridHeight = (numY - 1) * spacing;
    
    // Compute starting position to center the grid
    float startX = (config->windowWidth - gridWidth) * 0.5f;
    float startY = (config->windowHeight - gridHeight) * 0.5f;


    int count = 0;
    for (int y = 0; y < numY && count < config->numParticles; ++y) {
        for (int x = 0; x < numX && count < config->numParticles; ++x) {
            float jitter = rand() % 100 / 10.0f; // Random jitter for particle position
            float jitterX = ((rand() / (float)RAND_MAX) - 0.5f) * jitter;
            float jitterY = ((rand() / (float)RAND_MAX) - 0.5f) * jitter;

            glm::vec2 pos = glm::vec2(
                startX + x * spacing + jitterX,
                startY + y * spacing + jitterY
            );

            config->particles.emplace_back(pos.x, pos.y, config->radius);
            ++count;
        }
    }
}

void computeDensityAndPressure(simConfig *config) {
    for (auto &pi : config->particles){
        pi.rho = 0.0f; // Reset density
        for (auto &pj : config->particles){
            // Calculate distance from pi to pj
            glm::vec2 rij = pj.position - pi.position;
            float r2 = glm::dot(rij, rij); // dot product with self == squared norm

            // Only particles within the kernel smoothing radius contribute to density
            if (r2 < config->H2){
                pi.rho += pj.m *config->POLY6 * pow(config->H2 - r2, 3);
            }
        }
        pi.p = config->GAS_CONSTANT * (pi.rho - config->REST_DENSITY); // Pressure based on density
    }
}

void computeForces(simConfig *config){
    
    for (auto &pi : config->particles){
        glm::vec2 pForce = glm::vec2(0.0f, 0.0f);
        glm::vec2 vForce = glm::vec2(0.0f, 0.0f);

        for(auto &pj : config->particles){
            if (&pi == &pj) continue; // Skip self

            glm::vec2 rij = pj.position - pi.position;
            float r2 = glm::dot(rij, rij); // dot product with self == squared norm
            float r = sqrt(r2);

            if (r < config->H) {
                // Pressure force
                glm::vec2 normalizedRij = rij / r; // Normalize the vector
                pForce += pi.m * (pi.p + pj.p) / (2.0f * pj.rho) * static_cast<float>(config->SPIKY_GRADIENT * pow(config->H - r, 3)) * -normalizedRij;

                // Viscosity force
                vForce += config->VISCOSITY * pj.m / pj.rho * (pj.velocity - pi.velocity) * static_cast<float>(config->VISCOSITY_LAPLACIAN * (config->H - r));
            }
        }
        // Gravity force
        glm::vec2 gForce = glm::vec2(0.0f, -config->G * pi.m / pi.rho);

        // Combine forces
        pi.force = pForce + vForce + gForce;
    }
}

void integrate(simConfig *config) {
    float boundaryStiffness = 100.0f;
    for(auto &p : config->particles){
        // Enforce boundary conditions
        // Left
        if (p.position.x - p.rad - config->EPSILON < 0)
        {
            p.velocity.x *= -config->BOUND_DAMPING;
            p.position.x = config->EPSILON + p.rad;
            p.force.x += -(p.position.x - p.rad) * boundaryStiffness - p.velocity.x * p.m;
        }
        // Right
        if (p.position.x + p.rad + config->EPSILON > config->windowWidth)
        {
            p.velocity.x *= -config->BOUND_DAMPING;
            p.position.x = config->windowWidth - config->EPSILON - p.rad;
            p.force.x -= (config->windowWidth - p.position.x + p.rad) * boundaryStiffness - p.velocity.x * p.m;
        }
        // Bottom
        if (p.position.y - p.rad - config->EPSILON < 0)
        {
            p.velocity.y *= -config->BOUND_DAMPING;
            p.position.y = config->EPSILON + p.rad;
        }
        // Top
        if (p.position.y + p.rad + config->EPSILON > config->windowHeight)
        {
            p.velocity.y *= -config->BOUND_DAMPING;
            p.position.y = config->windowHeight - config->EPSILON - p.rad;
        }

        // Euler integration
        p.velocity += p.force / p.rho * config->simTime; // Update velocity
        p.position += p.velocity * config->simTime; // Update position
    }
}