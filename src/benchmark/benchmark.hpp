#pragma once

#include <string>
#include <vector>

/**
 * Particle counts swept during a --benchmark run, smallest to largest.
 *
 * Capped at 2500 while the solver is still brute-force O(n^2) (Stages 0-2) —
 * at that complexity 5000+ particles takes minutes per data point and 20000
 * would take hours (confirmed by an actual Stage 0 run). Raised once the
 * spatial grid (Stage 3) makes larger counts practical to sit through.
 */
extern const std::vector<int> kBenchmarkParticleCounts;

/** Frames run untimed before timing starts at each particle count — lets the
 *  simulation reach representative motion and absorbs first-use driver costs
 *  (shader/state caching) before any frame is measured. */
constexpr int kBenchmarkWarmupFrames = 30;

/** Frames timed and averaged at each particle count. */
constexpr int kBenchmarkSampleFrames = 200;

/**
 * @brief Aggregated full-frame timing for one particle count.
 *
 * "Frame" here means one call to the real per-frame path (physics step +
 * render + ImGui + swap) — see StepAndRenderFrame in main.cpp.
 */
struct FrameStats {
    int particleCount = 0;
    double avgFrameMs = 0.0;
    double minFrameMs = 0.0;
    double maxFrameMs = 0.0;
    double avgFps = 0.0;
};

/**
 * @brief Writes benchmark results to benchmarks/results/<stageLabel>.csv.
 *
 * Creates the benchmarks/results directory if it doesn't exist. One row per
 * FrameStats entry; columns: particle_count,avg_frame_ms,min_frame_ms,max_frame_ms,avg_fps
 *
 * @param stageLabel Name of this benchmark run (e.g. "00_baseline"), used as the CSV filename.
 * @param stats Per-particle-count results to write, in sweep order.
 */
void writeBenchmarkCSV(const std::string& stageLabel, const std::vector<FrameStats>& stats);
