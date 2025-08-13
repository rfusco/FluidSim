// main.cpp
#include "Simulation.hpp"
#include <iostream>

int main() {
    try {
        Simulation sim;
        sim.run();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
