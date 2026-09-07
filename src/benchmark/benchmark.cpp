#include "benchmark.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

const std::vector<int> kBenchmarkParticleCounts = {100, 250, 500, 1000, 2500};

void writeBenchmarkCSV(const std::string& stageLabel, const std::vector<FrameStats>& stats) {
    // Anchor to the source tree (not the working directory) so results always
    // land in the committed benchmarks/results/, whether run from build/src
    // (required for shaders' relative paths) or anywhere else.
#ifdef FLUIDSIM_SOURCE_DIR
    std::filesystem::path resultsDir = std::filesystem::path(FLUIDSIM_SOURCE_DIR) / "benchmarks" / "results";
#else
    std::filesystem::path resultsDir = std::filesystem::path("benchmarks") / "results";
#endif
    std::filesystem::create_directories(resultsDir);

    std::filesystem::path outPath = resultsDir / (stageLabel + ".csv");
    std::ofstream out(outPath);
    if (!out) {
        std::cerr << "Failed to open " << outPath.string() << " for writing\n";
        return;
    }

    out << "particle_count,avg_frame_ms,min_frame_ms,max_frame_ms,avg_fps\n";
    for (const auto& s : stats) {
        out << s.particleCount << ","
            << s.avgFrameMs << ","
            << s.minFrameMs << ","
            << s.maxFrameMs << ","
            << s.avgFps << "\n";
    }

    std::cout << "Wrote " << stats.size() << " rows to " << outPath.string() << "\n";
}
