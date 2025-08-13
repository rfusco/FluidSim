#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "simConfig.hpp"

void initSPH(simConfig &config);
void computeDensityAndPressure(simConfig &config);
void computeForces(simConfig &config);
void integrate(simConfig &config);

