#include <sphSolver.hpp>

void initSPH(std::vector<Particle>& particles, int numParticles, float radius, float windowWidth, float windowHeight) {
    particles.clear();

    const float spacing = 2 * radius; // Default radius for particles

    // Compute grid size (make it as square as possible)
    int numX = static_cast<int>(std::ceil(std::sqrt(numParticles)));
    int numY = static_cast<int>(std::ceil(numParticles / static_cast<float>(numX)));

    // Compute total width and height of the particle grid
    float gridWidth = (numX - 1) * spacing;
    float gridHeight = (numY - 1) * spacing;
    
    // Compute starting position to center the grid
    float startX = (windowWidth - gridWidth) * 0.5f;
    float startY = (windowHeight - gridHeight) * 0.5f;


    int count = 0;
    for (int y = 0; y < numY && count < numParticles; ++y) {
        for (int x = 0; x < numX && count < numParticles; ++x) {
            float jitter = rand() % 100 / 10.0f; // Random jitter for particle position
            float jitterX = ((rand() / (float)RAND_MAX) - 0.5f) * jitter;
            float jitterY = ((rand() / (float)RAND_MAX) - 0.5f) * jitter;

            glm::vec2 pos = glm::vec2(
                startX + x * spacing + jitterX,
                startY + y * spacing + jitterY
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
                float diff = H2 - r2;
                pi.rho += pj.m * POLY6 * (diff * diff * diff); // was pow(diff, 3) — pow() isn't reliably folded to multiplies by the compiler, and this runs O(n^2) times per step
            }
        }
        pi.p = GAS_CONSTANT * (pi.rho - REST_DENSITY); // Pressure based on density
    }
}

void computeForces(std::vector<Particle>& particles, float H, float G, float MASS, float SPIKY_GRADIENT, float VISCOSITY, float VISCOSITY_LAPLACIAN){
    const float H2 = H * H; // avoids taking sqrt() before knowing a pair is even in range — see below

    for (auto &pi : particles){
        glm::vec2 pForce = glm::vec2(0.0f, 0.0f);
        glm::vec2 vForce = glm::vec2(0.0f, 0.0f);

        for(auto &pj : particles){
            if (&pi == &pj) continue; // Skip self

            glm::vec2 rij = pj.position - pi.position;
            float r2 = glm::dot(rij, rij); // dot product with self == squared norm

            // Checked on r2 now instead of r, so sqrt() below only runs for pairs
            // already known to be in range — previously computed unconditionally
            // for every one of the O(n^2) pairs, including ones far outside H.
            // 1e-12f (~ (1e-6f)^2) guards against coincident particles (possible
            // from initSPH's jitter) producing a 0/0 division in normalizedRij.
            if (r2 > 1e-12f && r2 < H2) {
                float r = sqrt(r2);
                float diff = H - r;
                float diff3 = diff * diff * diff; // was pow(diff, 3)

                // Pressure force
                glm::vec2 normalizedRij = rij / r; // Normalize the vector
                pForce += MASS * (pi.p + pj.p) / (2.0f * pj.rho) * (SPIKY_GRADIENT * diff3) * -normalizedRij;

                // Viscosity force
                vForce += VISCOSITY * pj.m / pj.rho * (pj.velocity - pi.velocity) * (VISCOSITY_LAPLACIAN * diff);
            }
        }
        // Gravity force. pForce/vForce above are left as force-densities (not yet
        // divided by rho) — integrate() applies a single /pi.rho to the combined
        // force. So gForce must be pre-multiplied by rho here, not divided, or
        // gravity's contribution ends up as G*MASS/rho^2 instead of G.
        glm::vec2 gForce = glm::vec2(0.0f, G * pi.rho);

        // Combine forces
        pi.force = pForce + vForce + gForce;
    }
}

void integrate(std::vector<Particle>& particles, float timeStep, float EPSILON, float BOUND_DAMPING, float windowWidth, float windowHeight) {
    float boundaryStiffness = 100.0f;
    for(auto &p : particles){
        // Enforce boundary conditions
        // Left
        if (p.position.x - p.rad - EPSILON < 0)
        {
            p.velocity.x *= -BOUND_DAMPING;
            p.position.x = EPSILON + p.rad;
            p.force.x += -(p.position.x - p.rad) * boundaryStiffness - p.velocity.x * p.m;
        }
        // Right
        if (p.position.x + p.rad + EPSILON > windowWidth)
        {
            p.velocity.x *= -BOUND_DAMPING;
            p.position.x = windowWidth - EPSILON - p.rad;
            p.force.x -= (windowWidth - p.position.x + p.rad) * boundaryStiffness - p.velocity.x * p.m;
        }
        // Bottom
        if (p.position.y - p.rad - EPSILON < 0)
        {
            p.velocity.y *= -BOUND_DAMPING;
            p.position.y = EPSILON + p.rad;
        }
        // Top
        if (p.position.y + p.rad + EPSILON > windowHeight)
        {
            p.velocity.y *= -BOUND_DAMPING;
            p.position.y = windowHeight - EPSILON - p.rad;
        }

        // Euler integration
        p.velocity += p.force / p.rho * timeStep; // Update velocity
        p.position += p.velocity * timeStep; // Update position
    }
}