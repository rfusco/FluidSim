#include "kernel.hpp"

float poly6(float H) {return 4.f / (M_PI * pow(H, 8.f));}
float spikyGradient(float H) {return -10.f / (M_PI * pow(H, 5.f));}
float viscosityLaplacian(float H) {return 40.f / (M_PI * pow(H, 5.f));}