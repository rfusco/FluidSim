#include <sphSolver.hpp>

void initSPH(std::vector<Particle>& particles, int numParticles, float radius){
    particles.clear();

    const float jitter = 0.0005f;
    const float spacing = 2 * radius; // Default radius for particles

    // Compute grid size (make it as square as possible)
    int numX = static_cast<int>(std::ceil(std::sqrt(numParticles)));
    int numY = static_cast<int>(std::ceil(numParticles / static_cast<float>(numX)));

    // Compute total width and height of the particle grid
    float gridWidth = (numX - 1) * spacing;
    float gridHeight = (numY - 1) * spacing;

    // Compute offset to center the grid at (0,0)
    float xOffset = -gridWidth / 2.0f;
    float yOffset = -gridHeight / 2.0f;

    int count = 0;
    for (int y = 0; y < numY && count < numParticles; ++y) {
        for (int x = 0; x < numX && count < numParticles; ++x) {
            float jitterX = ((rand() / (float)RAND_MAX) - 0.5f) * jitter;
            float jitterY = ((rand() / (float)RAND_MAX) - 0.5f) * jitter;

            glm::vec2 pos = glm::vec2(
                x * spacing + xOffset + jitterX,
                y * spacing + yOffset + jitterY
            );

            particles.emplace_back(pos.x, pos.y, radius);
            ++count;
        }
    }
}

void computeDensityAndPressure(std::vector<Particle>& particles, float H, float H2, float POLY6, float GAS_CONSTANT, float REST_DENSITY) {
    for (auto &pi : particles){
        pi.rho = 0.0f; // Reset density
        for (auto &pj : particles){
            // Calculate distance from pi to pj
            glm::vec2 rij = pj.position - pi.position;
            float r2 = glm::dot(rij, rij); // dot product with self == squared norm

            // Only particles within the kernel smoothing radius contribute to density
            if (r2 < H2){
                pi.rho += pj.m * POLY6 * pow(H2 - r2, 3);
            }
        }
        pi.p = GAS_CONSTANT * (pi.rho - REST_DENSITY); // Pressure based on density
    }
}

void computeForces(std::vector<Particle>& particles, float H, float G, float MASS, float SPIKY_GRADIENT, float VISCOSITY, float VISCOSITY_LAPLACIAN){
    
    for (auto &pi : particles){
        glm::vec2 pForce = glm::vec2(0.0f, 0.0f);
        glm::vec2 vForce = glm::vec2(0.0f, 0.0f);

        for(auto &pj : particles){
            if (&pi == &pj) continue; // Skip self

            glm::vec2 rij = pj.position - pi.position;
            float r2 = glm::dot(rij, rij); // dot product with self == squared norm
            float r = sqrt(r2);

            if (r < H) {
                // Pressure force
                pForce += MASS * (pi.p + pj.p) / (2.0f * pj.rho) * SPIKY_GRADIENT * pow(H - r, 3) * glm::normalize(rij);

                // Viscosity force
                vForce += VISCOSITY_LAPLACIAN * (pj.velocity - pi.velocity) / pj.rho;
            }
        }
    }
}