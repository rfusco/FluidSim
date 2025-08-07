#pragma once

#include <vector>

/**
 * @brief Benchmarks fluid simulation and returns the average frame rate
 * 
 * Creates a new simulation and runs a benchmark for a fixed particle size
 * and returns the average frame rate
 * 
 * @param numParticles The number of particles to benchmark at
 * @param sampleTime The length of the benchmark
 * @param numSamples The number of samples collected
 * @return Average fps (float) 
 */
float benchmarkAveragePerformance(int numParticles, float sampleTime, int numSamples);
//float toCSV