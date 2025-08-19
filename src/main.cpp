// main.cpp
#include "Simulation.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        std::cout << argc;
        Simulation sim;
        sim.run();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
