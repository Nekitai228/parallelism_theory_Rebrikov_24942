#include "heat_solver.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    int nx = 128, ny = 128;
    int max_iter = 1000000;
    double tol = 1e-6;

    try {
        po::options_description desc("Heat Equation Solver (Optimized Jacobi OpenACC)");
        desc.add_options()
            ("help,h", "Show help")
            ("nx", po::value<int>(&nx)->default_value(128), "Grid size X")
            ("ny", po::value<int>(&ny)->default_value(128), "Grid size Y")
            ("max-iter", po::value<int>(&max_iter)->default_value(1000000), "Max iterations")
            ("tol", po::value<double>(&tol)->default_value(1e-6), "Tolerance")
        ;
        
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) { std::cout << desc << "\n"; return 1; }
    } catch (const po::error& e) {
        std::cerr << "Error: " << e.what() << "\n"; return 1;
    }

    HeatSolver solver(nx, ny);
    solver.initialize();

    std::cout << "Optimized Jacobi: " << nx << "x" << ny << " mesh\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    double error = 1.0;
    int iter = 0;
    double* d_A = solver.getPtrA();


    const int CHECK_EVERY = 100;
    while (iter < max_iter) {
        error = HeatSolver::compute_kernel(solver.getNx(), solver.getNy(), d_A);
        iter++;
        // std::swap(d_A, d_Anew);
        if (iter % CHECK_EVERY == 0 && error <= tol) break;
    }
    if (error > tol) {
        error = HeatSolver::compute_kernel(solver.getNx(), solver.getNy(), d_A);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "\n=== Results ===\n";
    std::cout << "Iterations: " << iter << "\n";
    std::cout << "Final error: " << std::scientific << error << "\n";
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Time: " << elapsed.count() << " seconds\n";

    if (nx >= 10) solver.printGrid(10, 10);
    if (nx >= 13) solver.printGrid(13, 13);

    return 0;
}