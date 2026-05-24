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
            ("tol", po::value<double>(&tol)->default_value(1e-6), "Tolerance");
        
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) { std::cout << desc << "\n"; return 1; }
    } catch (const po::error& e) {
        std::cerr << "Error: " << e.what() << "\n"; return 1;
    }

    HeatSolver solver(nx, ny);
    solver.initialize();

    // Получаем сырые указатели для OpenACC
    double* d_A = solver.getPtrA();
    double* d_Anew = solver.getPtrAnew();
    const int N = solver.getNx(); // для present()
    const int totalSize = N * N;

    std::cout << "Optimized Jacobi: " << nx << "x" << ny << " mesh\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    double error = 1.0;
    int iter = 0;
    const int CHECK_EVERY = 100; // Проверять ошибку раз в 100 итераций

    // === ЯВНАЯ ОБЛАСТЬ ДАННЫХ: копируем один раз в начале, один раз в конце ===
    #pragma acc data copy(d_A[0:totalSize], d_Anew[0:totalSize])
    {
        while (iter < max_iter) {
            // 99 из 100 итераций: быстрое ядро БЕЗ reduction
            if (iter % CHECK_EVERY != 0) {
                HeatSolver::compute_kernel_fast(nx, ny, d_A, d_Anew);
            } 
            // Каждая 100-я итерация: ядро С reduction для проверки сходимости
            else {
                error = HeatSolver::compute_kernel_with_error(nx, ny, d_A, d_Anew);
                if (error <= tol) break;
                
                // Прогресс в stderr
                if (iter % 1000 == 0) {
                    std::cerr << "[GPU] Iter: " << iter << " | Error: " << error << "\n";
                }
            }
            iter++;
            std::swap(d_A, d_Anew); // Меняем указатели (безопасно внутри acc data)
        }
        
        // Если вышли по max_iter без проверки — сделаем финальную проверку
        if (iter % CHECK_EVERY != 0) {
            error = HeatSolver::compute_kernel_with_error(nx, ny, d_A, d_Anew);
        }
    }
    // === Конец области данных: результат автоматически скопирован на хост ===

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